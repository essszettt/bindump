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
/* readFrame()                                                                */
/*----------------------------------------------------------------------------*/
int readFrame(dumpmode_t eMode, fileinfo_t* pFile, readbuffer_t* pRead)
{
  int iReturn = EOK;

  if ((0 != pFile) && (0 != pRead))
  {
    switch (eMode)
    {
      case DUMP_LOGICAL:
        memcpy(pRead->uiData, zxn_memmap(pRead->uiAddr), pRead->uiStride);
        iReturn = pRead->uiStride;
        break;

      case DUMP_PHYSICAL:
        iReturn = -1 * ESTAT;
        break;

      case DUMP_FILE:
        if (INV_FILE_HND != pFile->hFile)
        {
          if (0 == esx_f_seek(pFile->hFile, pRead->uiAddr, ESX_SEEK_SET))
          {
            iReturn = esx_f_read(pFile->hFile, pRead->uiData, pRead->uiStride);
          }
          else
          {
            iReturn = -1 * EBADF;
          }
        }
        else
        {
          iReturn = -1 * ESTAT;
        }
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
/* renderLine()                                                               */
/*----------------------------------------------------------------------------*/
int renderLine(
  dumpmode_t eMode,
  screeninfo_t* pScreen,
  readbuffer_t* pRead,
  renderbuffer_t* pRender)
{
  int iReturn = EINVAL;

  if ((0 != pScreen) && (0 != pRead) && (0 != pRender))
  {
    char_t* acIdx = &pRender->acData[0];

    if (85 <= pScreen->uiCols) /* 85 x 24 */
    {
      byte2hex((pRead->uiAddr >> 16) & 0xFF, acIdx); acIdx += 2;
      byte2hex((pRead->uiAddr >>  8) & 0xFF, acIdx); acIdx += 2;
      byte2hex((pRead->uiAddr >>  0) & 0xFF, acIdx); acIdx += 2;

      *acIdx++ = '|';

      for (uint8_t i = 0; i < pRead->uiStride; ++i)
      {
        *acIdx++ = ' ';
        byte2hex(pRead->uiData[i], acIdx); acIdx += 2;
      }

      *acIdx++ = '|';
      *acIdx++ = ' ';

      for (uint8_t i = 0; i < pRead->uiStride; ++i)
      {
        *acIdx++ = (between_uint8(pRead->uiData[i], ' ', '\xA4') ? pRead->uiData[i] : '.');
      }

      *acIdx++ =  '|';
      *acIdx++ = '\0';
    }
    else if (64 <= pScreen->uiCols) /* 64 x 24 */
    {
      byte2hex((pRead->uiAddr >> 16) & 0xFF, acIdx); acIdx += 2;
      byte2hex((pRead->uiAddr >>  8) & 0xFF, acIdx); acIdx += 2;
      byte2hex((pRead->uiAddr >>  0) & 0xFF, acIdx); acIdx += 2;

      *acIdx++ = '|';

      for (uint8_t i = 0; i < pRead->uiStride; i += 2)
      {
        byte2hex(pRead->uiData[    i], acIdx); acIdx += 2;
        byte2hex(pRead->uiData[i + 1], acIdx); acIdx += 2;

        *acIdx++ = ' ';
      }

      *acIdx++ = '|';

      for (uint8_t i = 0; i < pRead->uiStride; ++i)
      {
        *acIdx++ = (between_uint8(pRead->uiData[i], ' ', '\xA4') ? pRead->uiData[i] : '.');
      }

      *acIdx++ = '\0';
    }
    else /* 32 x 24 */
    {
      byte2hex((pRead->uiAddr >> 16) & 0xFF, acIdx); acIdx += 2;
      byte2hex((pRead->uiAddr >>  8) & 0xFF, acIdx); acIdx += 2;
      byte2hex((pRead->uiAddr >>  0) & 0xFF, acIdx); acIdx += 2;

       *acIdx++ = '|';

      for (uint8_t i = 0; i < pRead->uiStride; ++i)
      {
        byte2hex(pRead->uiData[i], acIdx); acIdx += 2;
      }

      *acIdx++ = '|';

      for (uint8_t i = 0; i < pRead->uiStride; ++i)
      {
        *acIdx++ = (between_uint8(pRead->uiData[i], ' ', '\xA4') ? pRead->uiData[i] : '.');
      }

      *acIdx++ = '\0';
    }
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* saveFrame()                                                                */
/*----------------------------------------------------------------------------*/
int saveFrame(dumpmode_t eMode, readbuffer_t* pRead, fileinfo_t* pFile)
{
  int iReturn = EINVAL;

  if ((0 != pRead) && (0 != pFile))
  {
    
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* nibble2hex()                                                               */
/*----------------------------------------------------------------------------*/
char_t nibble2hex(uint8_t uiValue)
{
  static const char_t g_acHexDigits[] = "0123456789ABCDEF";
  return g_acHexDigits[uiValue & 0x0F];
}


/*----------------------------------------------------------------------------*/
/* byte2hex()                                                                 */
/*----------------------------------------------------------------------------*/
int byte2hex(uint8_t uiByte, char_t* acHex)
{
  if (0 != acHex)
  {
    acHex[0] = nibble2hex((uiByte >> 4) & 0x0F);
    acHex[1] = nibble2hex(uiByte & 0x0F);
    return EOK;
  }

  return EINVAL;
}



/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
