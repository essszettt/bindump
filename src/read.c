/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: read.c                                                             |
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
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>

#include "libzxn.h"
#include "bindump.h"
#include "read.h"

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

/*============================================================================*/
/*                               Strukturen                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Typ-Definitionen                             */
/*============================================================================*/

/*============================================================================*/
/*                               Prototypen                                   */
/*============================================================================*/
/*
Read a data frame from logical memory
@param pRead Pointer to the read buffer
@return negativ values on errors; positive values are length of read data
*/
static int readFrame_logical(readbuffer_t* pRead);

/*!
Read a data frame from physical memory
@param pRead Pointer to the read buffer
@return negativ values on errors; positive values are length of read data
*/
static int readFrame_physical(readbuffer_t* pRead);

/*!
Read a data frame from a file
@param pRead Pointer to the read buffer
@param pFile Pointer to the file info
@return negativ values on errors; positive values are length of read data
*/
static int readFrame_file(readbuffer_t* pRead, fileinfo_t* pFile);

/*============================================================================*/
/*                               Klassen                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementierung                              */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* readFrame()                                                                */
/*----------------------------------------------------------------------------*/
int readFrame(dumpmode_t eMode, fileinfo_t* pFile, readbuffer_t* pRead)
{
  int iReturn = EINVAL;

  if ((0 != pFile) && (0 != pRead))
  {
    switch (eMode)
    {
      case DUMP_NONE:
        iReturn = -1 * ERANGE;
        break;

      case DUMP_LOGICAL:
        iReturn = readFrame_logical(pRead);
        break;

      case DUMP_PHYSICAL:
        iReturn = readFrame_physical(pRead);
        break;

      case DUMP_FILE:
        iReturn = readFrame_file(pRead, pFile);
        break;
    }

    iReturn = (0 <= iReturn ? EOK : -1 * iReturn);
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* readFrame_logical()                                                        */
/*----------------------------------------------------------------------------*/
static int readFrame_logical(readbuffer_t* pRead)
{
  int iReturn = 0;

  const uint8_t* pSrc = zxn_memmap(pRead->uiAddr);

  for (uint8_t i = 0; i < pRead->uiStride; ++i)
  {
    if (between_uint32(pRead->uiAddr + i, pRead->uiLower, pRead->uiUpper, 1))
    {
      pRead->uiData[i] = pSrc[i];
      ++iReturn;
    }
    else
    {
      pRead->uiData[i] = 0;
    }
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* readFrame_physical()                                                       */
/*----------------------------------------------------------------------------*/
static int readFrame_physical(readbuffer_t* pRead)
{
  int iReturn = 0;

  const uint8_t* pSrc = zxn_memmap(pRead->uiAddr);

  /* TODO */

  for (uint8_t i = 0; i < pRead->uiStride; ++i)
  {
    if (between_uint32(pRead->uiAddr + i, pRead->uiLower, pRead->uiUpper, 1))
    {
      pRead->uiData[i] = pSrc[i];
      ++iReturn;
    }
    else
    {
      pRead->uiData[i] = 0;
    }
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* readFrame_file()                                                           */
/*----------------------------------------------------------------------------*/
static int readFrame_file(readbuffer_t* pRead, fileinfo_t* pFile)
{
  int iReturn = 0;

  if (INV_FILE_HND != pFile->hFile)
  {
    for (uint8_t i = 0; i < pRead->uiStride; ++i)
    {
      if (between_uint32(pRead->uiAddr + i, pRead->uiLower, pRead->uiUpper, 1))
      {
        if (UINT32_C(-1) != esx_f_seek(pFile->hFile, pRead->uiAddr + i, ESX_SEEK_SET))
        {
          if (1 == esx_f_read(pFile->hFile, &pRead->uiData[i], 1))
          {
            ++iReturn;
          }
          else
          {
            iReturn = -1 * EBADF;
            break;
          }
        }
        else
        {
          iReturn = -1 * EBADF;
          break;
        }

        ++iReturn;
      }
      else
      {
        pRead->uiData[i] = 0;
      }
    }
  }
  else
  {
    iReturn = -1 * ESTAT;
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
