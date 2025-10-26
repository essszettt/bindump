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
  Backup: Current speed of Z80
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
  g_tState.eAction       = ACTION_NONE;
  g_tState.bQuiet        = false;
  g_tState.bForce        = false;
  g_tState.iExitCode     = EOK;
  g_tState.uiCpuSpeed    = ZXN_READ_REG(REG_TURBO_MODE) & 0x03;

  ZXN_WRITE_REG(REG_TURBO_MODE, RTM_28MHZ);

  g_tState.bInitialized  = true;
}


/*----------------------------------------------------------------------------*/
/* _destruct()                                                                */
/*----------------------------------------------------------------------------*/
void _destruct(void)
{
  if (g_tState.bInitialized)
  {
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

  g_tState.eAction = ACTION_NONE;

  int i = 1;

  while (i < argc)
  {
    const char* acArg = argv[i];

    if ('-' == acArg[0])
    {
      if ((0 == stricmp(acArg, "-h")) /* || (0 == stricmp(acArg, "--help")) */)
      {
        g_tState.eAction = ACTION_HELP;
      }
      else if ((0 == strcmp(acArg, "-v")) /* || (0 == stricmp(acArg, "--version")) */)
      {
        g_tState.eAction = ACTION_INFO;
      }
      else if ((0 == strcmp(acArg, "-q")) /* || (0 == stricmp(acArg, "--quiet")) */)
      {
        g_tState.bQuiet = true;
      }
      else if ((0 == strcmp(acArg, "-f")) /* || (0 == stricmp(acArg, "--force")) */)
      {
        g_tState.bForce = true;
      }
     #if 0
      else if ((0 == stricmp(acArg, "-c")) || (0 == stricmp(acArg, "--count")))
      {
        if ((i + 1) < argc)
        {
          g_tState.uiCount = strtoul(argv[i + 1], 0, 0);
          ++i;
        }
      }
     #endif
      else
      {
        fprintf(stderr, "unknown option: %s\n", acArg);
        iReturn = EINVAL;
        break;
      }
    }
    else
    {
      /* snprintf(g_tState.bmpfile.acPathName, sizeof(g_tState.bmpfile.acPathName), "%s", acArg); */
    }

    ++i;
  }

  if (ACTION_NONE == g_tState.eAction)
  {
   #if 1
    g_tState.eAction = ACTION_DUMP;
   #else
    if (0 < strnlen(g_tState.bmpfile.acPathName, sizeof(g_tState.bmpfile.acPathName)))
    {
      g_tState.eAction = ACTION_SHOT;
    }
    else if ('\0' == g_tState.bmpfile.acPathName[0])
    {
      g_tState.eAction = ACTION_SHOT;
    }
   #endif
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* showHelp()                                                                 */
/*----------------------------------------------------------------------------*/
int showHelp(void)
{
  unsigned char acAppName[0x10];
  strncpy(acAppName, VER_INTERNALNAME_STR, sizeof(acAppName));
  strupr(acAppName);

  printf("%s\n\n", VER_FILEDESCRIPTION_STR);

  printf("%s [-f file][-l][-p][-o offset][-s size][-r][-q][-h][-v] file\n\n", acAppName);
  //      0.........1.........2.........3.
  printf(" file        pathname of file\n");
  printf(" -f[ile]     read from file\n");
  printf(" -l[ogical]  read logical mem.\n");
  printf(" -p[hysical] read physical mem.\n");
  printf(" -o[ffset]   offset to read from\n");
  printf(" -s[ize]     length to read from\n");
  printf(" -[fo]r[ce]  force overwrite\n");
  printf(" -q[uiet]    print no messages\n");
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
  unsigned char acAppName[0x10];
  strncpy(acAppName, VER_INTERNALNAME_STR, sizeof(acAppName));
  strupr(acAppName);

  printf("%s " VER_LEGALCOPYRIGHT_STR "\n", acAppName);
  //      0.........1.........2.........3.
  printf(" Version %s\n", VER_FILEVERSION_STR);
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

