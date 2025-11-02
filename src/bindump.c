/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: bindump.c                                                          |
| project:  ZX Spectrum Next - BINDUMP                                         |
| author:   Stefan Zell                                                        |
| date:     11/01/2025                                                         |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| description:                                                                 |
|                                                                              |
| Application to dump binary content of memory and files                       |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| Copyright (c) 11/01/2025 STZ Engineering                                     |
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>
#include <arch/zxn/sysvar.h>

#include "libzxn.h"
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
extern appstate_t g_tState;

/*============================================================================*/
/*                               Strukturen                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Typ-Definitionen                             */
/*============================================================================*/

/*============================================================================*/
/*                               Prototypen                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Klassen                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementierung                              */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* readLine()                                                                 */
/*----------------------------------------------------------------------------*/
int readLine(dumpmode_t eMode, readbuffer_t* pRead)
{
  int iReturn = EOK;

  if (0 != pRead)
  {
    switch (eMode)
    {
      case DUMP_LOGICAL:
        if (0 != pRead->pBuffer)
        {
          memcpy(pRead->pBuffer, zxn_memmap(pRead->uiAddr), pRead->uiStride);
          iReturn = pRead->uiStride;
        }
        else
        {
          iReturn = -1 * ENOMEM;
        }
        break;

      case DUMP_PHYSICAL:
        iReturn = -1 * ESTAT;
        break;

      case DUMP_FILE:
        iReturn = -1 * ESTAT;
        break;
    }
  }
  else
  {
    iReturn = -1 * EINVAL;
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* renderScreenLine()                                                         */
/*----------------------------------------------------------------------------*/
int renderScreenLine(readbuffer_t* pBuffer)
{
  int iReturn = EINVAL;
  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* renderFileLine()                                                           */
/*----------------------------------------------------------------------------*/
int renderFileLine(readbuffer_t* pBuffer)
{
  int iReturn = EINVAL;
  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* saveLine()                                                                 */
/*----------------------------------------------------------------------------*/
int saveLine(readbuffer_t* pBuffer, fileinfo_t* pFile)
{
  int iReturn = EINVAL;
  return iReturn;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
