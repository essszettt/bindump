/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: main.c                                                             |
| project:  ZX Spectrum Next - BINDUMP                                         |
| author:   Stefan Zell                                                        |
| date:     10/26/2025                                                         |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| description:                                                                 |
|                                                                              |
| Application to dump binary content of memory and files                       |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| Copyright (c) 10/26/2025 STZ Engineering                                     |
|                                                                              |
| This software is provided  "as is",  without warranty of any kind, express   |
| or implied. In no event shall STZ or its contributors be held liable for any |
| direct, indirect, incidental, special or consequential damages arising out   |
| of the use of or inability to use this software.                             |
|                                                                              |
| Permission is granted to anyone  to use this  software for any purpose,      |
| including commercial applications,  and to alter it and redistribute it      |
| freely, subject to the following restrictions:                               |
|                                                                              |
| 1. Redistributions of source code must retain the above copyright            |
|    notice, definition, disclaimer, and this list of conditions.              |
|                                                                              |
| 2. Redistributions in binary form must reproduce the above copyright         |
|    notice, definition, disclaimer, and this list of conditions in            |
|    documentation and/or other materials provided with the distribution.      |
|                                                                          ;-) |
+-----------------------------------------------------------------------------*/

/*============================================================================*/
/*                               Includes                                     */
/*============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <input.h>
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>
#include <arch/zxn/sysvar.h>

#include "libzxn.h"
#include "bindump.h"
#include "read.h"
#include "render.h"
#include "save.h"
#include "version.h"

/*============================================================================*/
/*                               Defines                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Namespaces                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Konstanten                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Variablen                                    */
/*============================================================================*/

/*!
All global data of the application is saved in this structure
*/
static appstate_t g_tState;

/*============================================================================*/
/*                               Strukturen                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Typ-Definitionen                             */
/*============================================================================*/

/*============================================================================*/
/*                               Prototypen                                   */
/*============================================================================*/

/*!
Diese Funktion wird einmalig beim Start der Anwendung zur Initialisierung aller
benoetigten Ressourcen aufgerufen.
*/
void _construct(void);

/*!
Diese Funktion wird einmalig beim Beenden der Anwendung zur Freigabe aller
belegten Ressourcen aufgerufen.
*/
void _destruct(void);

/*!
Diese Funktion interpretiert alle Argumente, die der Anwendung uebergeben
wurden.
*/
int parseArguments(int argc, char* argv[]);

/*!
Ausgabe der Hilfe dieser Anwendung.
*/
int showHelp(void);

/*!
Ausgabe der Versionsinformation dieser Anwendung.
*/
int showInfo(void);

/*!
This function dump data (to file, to screen, ...)
*/
int dump(void);

/*!
This function dump data to a file
*/
int dumpQuiet(void);

/*!
This function dump data interactively to screen
*/
int dumpInteractive(void);

/*============================================================================*/
/*                               Klassen                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementierung                              */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* _construct()                                                               */
/*----------------------------------------------------------------------------*/
void _construct(void)
{
  memset(&g_tState, 0, sizeof(g_tState));

  if (!g_tState.bInitialized)
  {
    g_tState.eAction        = ACTION_NONE;
    g_tState.bQuiet         = false;
    g_tState.bForce         = false;
    g_tState.eMode          = DUMP_NONE;
    g_tState.tRdFile.hFile  = INV_FILE_HND;
    g_tState.tWrFile.hFile  = INV_FILE_HND;
    g_tState.uiCpuSpeed     = ZXN_READ_REG(REG_TURBO_MODE) & 0x03;
    g_tState.tScreen.uiCols = 32;
    g_tState.tScreen.uiRows = 22;
    g_tState.iExitCode      = EOK;

    ZXN_WRITE_REG(REG_TURBO_MODE, RTM_28MHZ);

    g_tState.bInitialized = true;
  }

  /* Detect current test resolution */
  struct esx_mode tMode;
  memset(&tMode, 0, sizeof(tMode));

  if (0 == esx_ide_mode_get(&tMode))
  {
    g_tState.tScreen.uiCols = tMode.cols;
    g_tState.tScreen.uiRows = tMode.rows;

    if ((1 == tMode.mode8.layer) && (2 == tMode.mode8.submode))
    {
      g_tState.tScreen.uiCols = (512 / tMode.width); /* 64 | 85 ? */
    }

    DBGPRINTF("_construct() - textres: %u/%u\n", g_tState.tScreen.uiCols, g_tState.tScreen.uiRows);
  }
}


/*----------------------------------------------------------------------------*/
/* _destruct()                                                                */
/*----------------------------------------------------------------------------*/
void _destruct(void)
{
  if (g_tState.bInitialized)
  {
    if (INV_FILE_HND != g_tState.tWrFile.hFile)
    {
      (void) esx_f_close(g_tState.tWrFile.hFile);
      g_tState.tWrFile.hFile = INV_FILE_HND;
    }

    if (INV_FILE_HND != g_tState.tRdFile.hFile)
    {
      (void) esx_f_close(g_tState.tRdFile.hFile);
      g_tState.tRdFile.hFile = INV_FILE_HND;
    }

    ZXN_WRITE_REG(REG_TURBO_MODE, g_tState.uiCpuSpeed);
  }
}


/*----------------------------------------------------------------------------*/
/* main()                                                                     */
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
  _construct();
  atexit(_destruct);

  if (EOK == (g_tState.iExitCode = parseArguments(argc, argv)))
  {
    switch (g_tState.eAction)
    {
      case ACTION_NONE:
        g_tState.iExitCode = EOK;
        break;

      case ACTION_INFO:
        g_tState.iExitCode = showInfo();
        break;

      case ACTION_HELP:
        g_tState.iExitCode = showHelp();
        break;

      case ACTION_DUMP:
        g_tState.iExitCode = dump();
        break;
    }
  }

  return (int) (EOK == g_tState.iExitCode ? 0 : zxn_strerror(g_tState.iExitCode));
}


/*----------------------------------------------------------------------------*/
/* parseArguments()                                                           */
/*----------------------------------------------------------------------------*/
int parseArguments(int argc, char* argv[])
{
  int iReturn = EOK;

  /* Defaults */
  g_tState.eAction  = ACTION_NONE;
  g_tState.bQuiet   = false;
  g_tState.bForce   = false;
  g_tState.eMode    = DUMP_NONE;
  g_tState.uiOffset = 0;
  g_tState.uiSize   = 0;
  g_tState.tRdFile.acPathName[0] = '\0';
  g_tState.tWrFile.acPathName[0] = '\0';

  int i = 1;
  while (i < argc)
  {
    const char_t* acArg = /* strrstrip(strstrip(argv[i])) */ argv[i];

    if ('-' == acArg[0]) /* Options */
    {
      if ((0 == strcmp(acArg, "-h")) || (0 == stricmp(acArg, "--help")))
      {
        g_tState.eAction = ACTION_HELP;
      }
      else if ((0 == strcmp(acArg, "-v")) || (0 == stricmp(acArg, "--version")))
      {
        g_tState.eAction = ACTION_INFO;
      }
      else if ((0 == strcmp(acArg, "-q")) || (0 == stricmp(acArg, "--quiet")))
      {
        g_tState.bQuiet = true;
      }
      else if ((0 == strcmp(acArg, "-r")) || (0 == stricmp(acArg, "--force")))
      {
        g_tState.bForce = true;
      }
      else if ((0 == strcmp(acArg, "-l")) || (0 == stricmp(acArg, "--logical")))
      {
        if (DUMP_NONE == g_tState.eMode)
        {
          g_tState.eMode = DUMP_LOGICAL;
        }
        else
        {
          fprintf(stderr, "options -l/-p/-f are mutually exclusive\n");
          iReturn = EINVAL;
          break;
        }
      }
      else if ((0 == strcmp(acArg, "-p")) || (0 == stricmp(acArg, "--physical")))
      {
        if (DUMP_NONE == g_tState.eMode)
        {
          g_tState.eMode = DUMP_PHYSICAL;
        }
        else
        {
          fprintf(stderr, "options -l/-p/-f are mutually exclusive\n");
          iReturn = EINVAL;
          break;
        }
      }
      else if ((0 == strcmp(acArg, "-f")) || (0 == stricmp(acArg, "--file")))
      {
        if (DUMP_NONE == g_tState.eMode)
        {
          if ((i + 1) < argc)
          {
            snprintf(g_tState.tRdFile.acPathName, sizeof(g_tState.tRdFile.acPathName), "%s", argv[++i]);
            zxn_normalizepath(g_tState.tRdFile.acPathName);
          }
          else
          {
            fprintf(stderr, "option %s requires a path argument\n", acArg);
            iReturn = EINVAL;
            break;
          }
        }
        else
        {
          fprintf(stderr, "options -l/-p/-f are mutually exclusive\n");
          iReturn = EINVAL;
          break;
        }
      }
      else if ((0 == strcmp(acArg, "-o")) || (0 == stricmp(acArg, "--offset")))
      {
        if ((i + 1) < argc)
        {
          g_tState.uiOffset = strtoul(argv[++i], 0, 0);
        }
        else
        {
          fprintf(stderr, "option %s requires a value\n", acArg);
          iReturn = EINVAL;
          break;
        }
      }
      else if ((0 == strcmp(acArg, "-s")) || (0 == stricmp(acArg, "--size")))
      {
        if ((i + 1) < argc)
        {
          g_tState.uiSize = strtoul(argv[++i], 0, 0);
        }
        else
        {
          fprintf(stderr, "option %s requires a value\n", acArg);
          iReturn = EINVAL;
          break;
        }
      }
      else
      {
        fprintf(stderr, "unknown option: %s\n", acArg);
        iReturn = EINVAL;
        break;
      }
    }
    else /* Arguments */
    {
      if ('\0' == g_tState.tWrFile.acPathName[0])
      {
        snprintf(g_tState.tWrFile.acPathName, sizeof(g_tState.tWrFile.acPathName), "%s", acArg);
        zxn_normalizepath(g_tState.tWrFile.acPathName);
      }
      else
      {
        fprintf(stderr, "unexpected extra argument: %s\n", acArg);
        iReturn = EINVAL;
        break;
      }
    }

    ++i;
  }

  DBGPRINTF("parseArgs() - mode   = %s\n", g_tState.bQuiet ? "quiet" : "interactive");
  DBGPRINTF("parseArgs() - dump   = %d\n", g_tState.eMode);
  DBGPRINTF("parseArgs() - offset = 0x%08lX\n", (unsigned long) g_tState.uiOffset);
  DBGPRINTF("parseArgs() - size   = 0x%08lX\n", (unsigned long) g_tState.uiSize);
  DBGPRINTF("parseArgs() - ifile  = %s\n", g_tState.tRdFile.acPathName);
  DBGPRINTF("parseArgs() - ofile  = %s\n", g_tState.tWrFile.acPathName);

  if (EOK == iReturn)
  {
    if (ACTION_NONE == g_tState.eAction)
    {
      g_tState.eAction = ACTION_DUMP;
    }
  }

  /* Plausibility checks */
  if (EOK == iReturn)
  {
    if ((ACTION_DUMP == g_tState.eAction))
    {
      if (DUMP_NONE == g_tState.eMode)
      {
        fprintf(stderr, "no dump mode specified\n");
        iReturn = EDOM;
      }
      else if (!g_tState.bQuiet && ('\0' != g_tState.tWrFile.acPathName[0]))
      {
        fprintf(stderr, "no dump to file in interactive mode\n");
        iReturn = EDOM;
      }
      else if (g_tState.bQuiet && ('\0' == g_tState.tWrFile.acPathName[0]))
      {
        fprintf(stderr, "output file required in quiet mode\n");
        iReturn = EDOM;
      }
    }
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* showHelp()                                                                 */
/*----------------------------------------------------------------------------*/
int showHelp(void)
{
  char_t acAppName[0x10];
  strncpy(acAppName, VER_INTERNALNAME_STR, sizeof(acAppName));
  strupr(acAppName);

  printf("%s\n\n", VER_FILEDESCRIPTION_STR);

  printf("%s [-f ifile][-l][-p][-o offset][-s size][-r][-q][-h][-v] ofile\n\n", acAppName);
  //      0.........1.........2.........3.
  printf("  ofile      pathname out-file\n");
  printf(" -f[ile]     read from file\n");
  printf("  ifile      pathname in-file\n");
  printf(" -l[ogical]  read logical mem.\n");
  printf(" -p[hysical] read physical mem.\n");
  printf(" -o[ffset]   offset to read from\n");
  printf(" -s[ize]     length to read\n");
  printf(" -[fo]r[ce]  force overwrite\n");
  printf(" -q[uiet]    no screen output\n");
  printf(" -h[elp]     print this help\n");
  printf(" -v[ersion]  print version info\n");

  /*
  bindump -p -o 0x21000 -s 0x100 -q c:/home/tmp/dump.bin
  bindump -l -o 0x0000 -s 0x4000 -q c:/home/tmp/dump.bin
  bindump -f c:/dot/ls -o 0x0000 -s 0x10000 -q c:/home/tmp/dump.bin
  bindump -p -o 0x21000,0x100 -q -r c:/home/tmp/dump.bin
  */

  return EOK;
}


/*----------------------------------------------------------------------------*/
/* showInfo()                                                                 */
/*----------------------------------------------------------------------------*/
int showInfo(void)
{
  char_t acBuffer[0x10];
  uint16_t uiVersion;

  strncpy(acBuffer, VER_INTERNALNAME_STR, sizeof(acBuffer));
  strupr(acBuffer);

  printf("%s " VER_LEGALCOPYRIGHT_STR "\n", acBuffer);

  if (ESX_DOSVERSION_NEXTOS_48K != (uiVersion = esx_m_dosversion()))
  {
    snprintf(acBuffer, sizeof(acBuffer), "NextOS %u.%02u",
             ESX_DOSVERSION_NEXTOS_MAJOR(uiVersion),
             ESX_DOSVERSION_NEXTOS_MINOR(uiVersion));
  }
  else
  {
    strncpy(acBuffer, "48K mode", sizeof(acBuffer));
  }

  //      0.........1.........2.........3.
  printf(" Version %s (%s)\n", VER_FILEVERSION_STR, acBuffer);
  printf(" Stefan Zell (info@diezells.de)\n");

  return EOK;
}


/*----------------------------------------------------------------------------*/
/* dump()                                                                     */
/*----------------------------------------------------------------------------*/
int dump(void)
{
  int iReturn = EOK;

  /* Detect stride to walk through memory/file */
  if (EOK == iReturn)
  {
    if (85 <= g_tState.tScreen.uiCols)
    {
      g_tState.tRead.uiStride = 16;
    }
    else if (64 <= g_tState.tScreen.uiCols)
    {
      g_tState.tRead.uiStride = 16;
    }
    else
    {
      g_tState.tRead.uiStride = 8;
    }
  }

  /* Calculate bounds of the region to read */
  if (EOK == iReturn)
  {
    uint32_t uiStrideMask = ~(((uint32_t) g_tState.tRead.uiStride) - UINT32_C(1));

    g_tState.tRead.uiLower = g_tState.uiOffset;
    g_tState.tRead.uiUpper = g_tState.uiOffset + g_tState.uiSize;
    g_tState.tRead.uiBegin = g_tState.tRead.uiLower & uiStrideMask;
    g_tState.tRead.uiAddr  = g_tState.tRead.uiBegin;
    g_tState.tRead.uiEnd   = (g_tState.tRead.uiUpper + (g_tState.tRead.uiStride - 1)) & uiStrideMask;

    DBGPRINTF("dump() - stride = 0x%02X\n", g_tState.tRead.uiStride);
    DBGPRINTF("dump() - outer  = 0x%06lX-0x%06lX\n", g_tState.tRead.uiBegin, g_tState.tRead.uiEnd);
    DBGPRINTF("dump() - inner  = 0x%06lX-0x%06lX\n", g_tState.tRead.uiLower, g_tState.tRead.uiUpper);
  }

  /* Open input file */
  if (EOK == iReturn)
  {
    if (DUMP_FILE == g_tState.eMode)
    {
      if ('\0' != g_tState.tRdFile.acPathName[0])
      {
        uint8_t uiResult = 0;
        struct esx_stat tStat;
        memset(&tStat, 0, sizeof(tStat));

        if (0 == (uiResult = esx_f_stat(g_tState.tRdFile.acPathName, &tStat)))
        {
          if (INV_FILE_HND == (g_tState.tRdFile.hFile = esx_f_open(g_tState.tRdFile.acPathName, ESX_MODE_READ | ESX_MODE_OPEN_EXIST)))
          {
            iReturn = EBADF;
          }
          else
          {
            if ((g_tState.uiOffset + g_tState.uiSize) > tStat.size)
            {

            }
          }
        }
        else
        {
          fprintf(stderr, "dumpData() - esx_f_stat(%s) = %u\n", g_tState.tRdFile.acPathName, uiResult);
          iReturn = EBADF;
        }
      }
    }
  }

  /* Open output file */
  if (EOK == iReturn)
  {
    if (!g_tState.bQuiet)
    {
      if ('\0' != g_tState.tWrFile.acPathName[0])
      {
        if (EOK == iReturn)
        {
          /* Is argument a directory ? */
          if (INV_FILE_HND != (g_tState.tWrFile.hFile = esx_f_opendir(g_tState.tWrFile.acPathName)))
          {
            uint16_t uiIdx = 0;
            char_t acPathName[ESX_PATHNAME_MAX];

            esx_f_closedir(g_tState.tWrFile.hFile);
            g_tState.tWrFile.hFile = INV_FILE_HND;

            while (uiIdx < 0xFFFF)
            {
              snprintf(acPathName, sizeof(acPathName),
                      "%s" ESX_DIR_SEP VER_INTERNALNAME_STR "-%u.txt",
                      g_tState.tWrFile.acPathName,
                      uiIdx);

              if (INV_FILE_HND == (g_tState.tWrFile.hFile = esx_f_open(acPathName, ESXDOS_MODE_R | ESXDOS_MODE_OE)))
              {
                snprintf(g_tState.tWrFile.acPathName, sizeof(g_tState.tWrFile.acPathName), "%s", acPathName);
                break;  /* filename found */
              }
              else
              {
                esx_f_close(g_tState.tWrFile.hFile);
                g_tState.tWrFile.hFile = INV_FILE_HND;
              }

              ++uiIdx;
            }

            if (0xFFFF == uiIdx)
            {
              iReturn = ERANGE; /* Error */
            }
          }
          else /* Argument is a file ... */
          {
            g_tState.tWrFile.hFile = esx_f_open(g_tState.tWrFile.acPathName, ESXDOS_MODE_R | ESXDOS_MODE_OE);

            if (INV_FILE_HND != g_tState.tWrFile.hFile)
            {
              esx_f_close(g_tState.tWrFile.hFile);
              g_tState.tWrFile.hFile = INV_FILE_HND;

              if (g_tState.bForce)
              {
                esx_f_unlink(g_tState.tWrFile.acPathName);
              }
              else
              {
                iReturn = EBADF; /* Error: File exists */
              }
            }
          }
        }

        if (EOK == iReturn)
        {
          g_tState.tWrFile.hFile = esx_f_open(g_tState.tWrFile.acPathName, ESXDOS_MODE_W | ESXDOS_MODE_CN);

          if (INV_FILE_HND == g_tState.tWrFile.hFile)
          {
            iReturn = EACCES; /* Error */
          }
        }
      }
      else
      {
        fprintf(stderr, "no output file specified\n");
        iReturn = EINVAL;
      }
    }
  }

  /* Execute the dump */
  if (EOK == iReturn)
  {
    if (g_tState.bQuiet)
    {
      iReturn = dumpQuiet();
    }
    else
    {
      iReturn = dumpInteractive();
    }
  }

  /* Close open files */
  if (INV_FILE_HND != g_tState.tRdFile.hFile)
  {
    esx_f_close(g_tState.tRdFile.hFile);
    g_tState.tRdFile.hFile = INV_FILE_HND;

    if (EOK != iReturn)
    {
      esx_f_unlink(g_tState.tRdFile.acPathName);
    }
  }

  if (INV_FILE_HND != g_tState.tWrFile.hFile)
  {
    esx_f_close(g_tState.tWrFile.hFile);
    g_tState.tWrFile.hFile = INV_FILE_HND;
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* dumpQuiet()                                                                */
/*----------------------------------------------------------------------------*/
int dumpQuiet(void)
{
  int iReturn = EOK;

  /* Walk through the region */
  if (EOK == iReturn)
  {
    int iResult = EOK;

    while (g_tState.tRead.uiAddr < g_tState.tRead.uiEnd)
    {
      if (0 < (iResult = readFrame(g_tState.eMode, &g_tState.tRdFile , &g_tState.tRead)))
      {
        renderLine(g_tState.eMode, &g_tState.tScreen, &g_tState.tRead, &g_tState.tRender);

        if (INV_FILE_HND != g_tState.tWrFile.hFile)
        {
          saveFrame(g_tState.eMode, &g_tState.tRead, &g_tState.tWrFile);
        }

        g_tState.tRead.uiAddr += ((uint32_t) g_tState.tRead.uiStride);
      }
      else
      {
        iReturn -1 * iResult;
        break;
      }
    }
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* dumpInteractive()                                                          */
/*----------------------------------------------------------------------------*/
int dumpInteractive(void)
{
  int iReturn = ENOTSUP;
  return iReturn;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

