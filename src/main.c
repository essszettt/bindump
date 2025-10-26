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

#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>
#include <arch/zxn/sysvar.h>

#include "libzxn.h"

#include "version.h"
#include "bindump.h"

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
In dieser Struktur werden alle globalen Daten der Anwendung gespeichert.
*/
static struct _state
{
  /*!
  If this flag is set, then this structure is initialized
  */
  bool bInitialized;

  /*!
  Action to execute (help, version, sreenshot, ...)
  */
  action_t eAction;

  /*!
  If this flag is set, no messages are printed to the console while creating a
  screenshot.
  */
  bool bQuiet;

  /*!
  If this flag is set, existing output files are overwritten
  */
  bool bForce;

  /*!
  Datasource: Logical memory, physical memory, file
  */
  dumpmode_t eMode;

  /*!
  Startoffset of the data to be dumped
  */
  uint32_t uiOffset;

  /*!
  Length of the data to be dumped
  */
  uint32_t uiSize;

  /*!
  File information of the input file
  */
  struct _ifile
  {
    char_t acPathName[ESX_PATHNAME_MAX];
    uint8_t hFile;
  } ifile;

  /*!
  File information of the output file
  */
  struct _ofile
  {
    char_t acPathName[ESX_PATHNAME_MAX];
    uint8_t hFile;
  } ofile;

  /*!
  Backup: Current speed of Z80N
  */
  uint8_t uiCpuSpeed;

  /*!
  Exitcode of the application, that is handovered to BASIC
  */
  int iExitCode;

} g_tState;

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
int dumpData(void);

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
  g_tState.eAction     = ACTION_NONE;
  g_tState.bQuiet      = false;
  g_tState.bForce      = false;
  g_tState.eMode       = DUMP_NONE;
  g_tState.uiOffset    = 0;
  g_tState.uiSize      = 0;
  g_tState.ifile.acPathName[0] = '\0';
  g_tState.ifile.hFile = INV_FILE_HND;
  g_tState.ofile.acPathName[0] = '\0';
  g_tState.ofile.hFile = INV_FILE_HND;
  g_tState.iExitCode   = EOK;
  g_tState.uiCpuSpeed  = ZXN_READ_REG(REG_TURBO_MODE) & 0x03;

  ZXN_WRITE_REG(REG_TURBO_MODE, RTM_28MHZ);

  g_tState.bInitialized = true;
}


/*----------------------------------------------------------------------------*/
/* _destruct()                                                                */
/*----------------------------------------------------------------------------*/
void _destruct(void)
{
  if (g_tState.bInitialized)
  {
    if (INV_FILE_HND != g_tState.ofile.hFile)
    {
      (void) esx_f_close(g_tState.ofile.hFile);
      g_tState.ofile.hFile = INV_FILE_HND;
    }

    if (INV_FILE_HND != g_tState.ifile.hFile)
    {
      (void) esx_f_close(g_tState.ifile.hFile);
      g_tState.ifile.hFile = INV_FILE_HND;
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
        g_tState.iExitCode = dumpData();
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
  g_tState.ifile.acPathName[0] = '\0';
  g_tState.ofile.acPathName[0] = '\0';

  int i = 1;
  while (i < argc)
  {
    const char_t* acArg = argv[i];

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
            snprintf(g_tState.ifile.acPathName, sizeof(g_tState.ifile.acPathName), "%s", argv[++i]);
            zxn_normalizepath(g_tState.ifile.acPathName);
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
      if ('\0' == g_tState.ofile.acPathName[0])
      {
        snprintf(g_tState.ofile.acPathName, sizeof(g_tState.ofile.acPathName), "%s", acArg);
        zxn_normalizepath(g_tState.ofile.acPathName);
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

  DBGPRINTF("parseArgs() - mode  = %s\n", g_tState.bQuiet ? "quiet" : "interactive");
  DBGPRINTF("parseArgs() - dump  = %d\n", g_tState.eMode);
  DBGPRINTF("parseArgs() - offset=0x%08lX\n", (unsigned long) g_tState.uiOffset);
  DBGPRINTF("parseArgs() - size  =0x%08lX\n", (unsigned long) g_tState.uiSize);
  DBGPRINTF("parseArgs() - ifile =%s\n", g_tState.ifile.acPathName);
  DBGPRINTF("parseArgs() - ofile =%s\n", g_tState.ofile.acPathName);

  /* Plausibility checks */
  if (EOK == iReturn)
  {
    if (DUMP_NONE == g_tState.eMode)
    {
      fprintf(stderr, "no dump mode specified\n");
      iReturn = EDOM;
    }
    else if (!g_tState.bQuiet && ('\0' != g_tState.ofile.acPathName[0]))
    {
      fprintf(stderr, "no dump to file in interactive mode\n");
      iReturn = EDOM;
    }
    else if (g_tState.bQuiet && ('\0' == g_tState.ofile.acPathName[0]))
    {
      fprintf(stderr, "output file required in quiet mode\n");
      iReturn = EDOM;
    }
  }

  if (EOK == iReturn)
  {
    if (ACTION_NONE == g_tState.eAction)
    {
      g_tState.eAction = ACTION_DUMP;
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
  uint16_t uiOsVersion = esx_m_dosversion();

  char_t acAppName[0x10];
  strncpy(acAppName, VER_INTERNALNAME_STR, sizeof(acAppName));
  strupr(acAppName);

  printf("%s " VER_LEGALCOPYRIGHT_STR "\n", acAppName);

  //      0.........1.........2.........3.
  printf(" Version %s (NextOS %d.%02d)\n",
         VER_FILEVERSION_STR,
         ESX_DOSVERSION_NEXTOS_MAJOR(uiOsVersion),
         ESX_DOSVERSION_NEXTOS_MINOR(uiOsVersion));
  printf(" Stefan Zell (info@diezells.de)\n");

  return EOK;
}


/*----------------------------------------------------------------------------*/
/* dumpData()                                                                 */
/*----------------------------------------------------------------------------*/
int dumpData(void)
{
  int iReturn = EOK;
  return iReturn;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

