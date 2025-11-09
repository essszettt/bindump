/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: save.c                                                             |
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
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>

#include "libzxn.h"
#include "bindump.h"
#include "save.h"

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
/*!
This function saves hex dump line to a file.
@param pRender Pointer to the render buffer
@param pFile Pointer to the file info
@return EOK = no error
*/
static int saveFrame_hex(const renderbuffer_t* pRender, const fileinfo_t* pFile);

/*!
This function saves a raw data block to a file.
@param pRead Pointer to the read buffer
@param pFile Pointer to the file info
@return EOK = no error
*/
static int saveFrame_raw(const readbuffer_t* pRead, const fileinfo_t* pFile);

/*============================================================================*/
/*                               Klassen                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementierung                              */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* saveFrame()                                                                */
/*----------------------------------------------------------------------------*/
int saveFrame(
  const readbuffer_t* pRead,
  const renderbuffer_t* pRender,
  const fileinfo_t* pFile)
{
  int iReturn = EOK;

  if (0 != pRender)
  {
    iReturn = saveFrame_hex(pRender, pFile);
  }
  else
  {
    iReturn = saveFrame_raw(pRead, pFile);
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* saveFrame_hex()                                                            */
/*----------------------------------------------------------------------------*/
static int saveFrame_hex(const renderbuffer_t* pRender, const fileinfo_t* pFile)
{
  int iReturn = EOK;

  if ((0 != pRender) && (0 != pFile))
  {
    if (INV_FILE_HND != pFile->hFile)
    {
      if (EOK == iReturn)
      {
        if (pRender->uiLen != esx_f_write(pFile->hFile, pRender->acData, pRender->uiLen))
        {
          iReturn = EBADF;
        }
      }

      if (EOK == iReturn)
      {
        if (1 != esx_f_write(pFile->hFile, "\n", 1))
        {
          iReturn = EBADF;
        }
      }
    }
  }
  else
  {
    iReturn = EINVAL;
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* saveFrame_raw()                                                            */
/*----------------------------------------------------------------------------*/
static int saveFrame_raw(const readbuffer_t* pRead, const fileinfo_t* pFile)
{
  int iReturn = EOK;

  if ((0 != pRead) && (0 != pFile))
  {
    if (INV_FILE_HND != pFile->hFile)
    {
      uint8_t uiFrom;
      uint8_t uiTo;
      uint8_t uiLen;

      uiFrom = (pRead->uiAddr < pRead->uiLower ? pRead->uiLower - pRead->uiAddr  : 0);
      uiTo   = ((pRead->uiAddr + pRead->uiStride) >= pRead->uiUpper ?
                (pRead->uiAddr + pRead->uiStride) - pRead->uiUpper :
                pRead->uiStride);
      uiLen  = uiTo - uiFrom;

      if (uiLen != esx_f_write(pFile->hFile, &pRead->uiData[uiFrom], uiLen))
      {
        iReturn = EBADF;
      }
    }
  }
  else
  {
    iReturn = EINVAL;
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
