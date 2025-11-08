/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: render.c                                                           |
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
#include "render.h"

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
This function renders a dataframe in 85 column mode.
@param uiIdx Current index in the render buffer
@param pRead Pointer to the read buffer
@param pRender Pointer to the render buffer
@return Current index in the render buffer
*/
static uint16_t renderLine_85(
  uint16_t uiIdx,
  const readbuffer_t* pRead,
  renderbuffer_t* pRender);

/*!
This function renders a dataframe in 64 column mode.
@param uiIdx Current index in the render buffer
@param pRead Pointer to the read buffer
@param pRender Pointer to the render buffer
@return Current index in the render buffer
*/
static uint16_t renderLine_64(
  uint16_t uiIdx,
  const readbuffer_t* pRead,
  renderbuffer_t* pRender);

/*!
This function renders a dataframe in 32 column mode.
@param uiIdx Current index in the render buffer
@param pRead Pointer to the read buffer
@param pRender Pointer to the render buffer
@return Current index in the render buffer
*/
static uint16_t renderLine_32(
  uint16_t uiIdx,
  const readbuffer_t* pRead,
  renderbuffer_t* pRender);

/*============================================================================*/
/*                               Klassen                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementierung                              */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* renderLine()                                                               */
/*----------------------------------------------------------------------------*/
int renderLine(
  dumpmode_t eMode,
  const screeninfo_t* pScreen,
  const readbuffer_t* pRead,
  renderbuffer_t* pRender)
{
  int iReturn = EINVAL;

  memset(pRender->acData, ' ', pRead->uiStride);
  pRender->acData[pRead->uiStride] = '\0';

  if ((0 != pScreen) && (0 != pRead) && (0 != pRender))
  {
    char_t* acIdx = &pRender->acData[0];

    byte2hex((pRead->uiAddr >> 16) & 0xFF, acIdx); acIdx += 2;
    byte2hex((pRead->uiAddr >>  8) & 0xFF, acIdx); acIdx += 2;
    byte2hex((pRead->uiAddr >>  0) & 0xFF, acIdx); acIdx += 2;

    if (85 <= pScreen->uiCols) /* 85 x 24 */
    {
      acIdx += renderLine_85(acIdx - pRender->acData, pRead, pRender);
    }
    else if (64 <= pScreen->uiCols) /* 64 x 24 */
    {
      acIdx += renderLine_64(acIdx - pRender->acData, pRead, pRender);
    }
    else /* 32 x 24 */
    {
      acIdx += renderLine_32(acIdx - pRender->acData, pRead, pRender);
    }

    *acIdx++ = '\0';
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* renderLine_85()                                                            */
/*----------------------------------------------------------------------------*/
static uint16_t renderLine_85(
  uint16_t uiIdx,
  const readbuffer_t* pRead,
  renderbuffer_t* pRender)
{
  uint16_t uiReturn = 0;

  if ((0 != pRead) && (0 != pRender))
  {
    char_t* acIdx = &pRender->acData[uiIdx];

    *acIdx++ = '|';

    for (uint8_t i = 0; i < pRead->uiStride; ++i)
    {
      ++acIdx; /* ' ' */

      if (between_uint32(pRead->uiAddr + i, pRead->uiLower, pRead->uiUpper, 1))
      {
        byte2hex(pRead->uiData[i], acIdx); 
      }
      acIdx += 2;
    }

    *acIdx++ = '|';
    ++acIdx; /* ' ' */

    for (uint8_t i = 0; i < pRead->uiStride; ++i)
    {
      if (between_uint32(pRead->uiAddr + i, pRead->uiLower, pRead->uiUpper, 1))
      {
        *acIdx = (between_uint8(pRead->uiData[i], ' ', '\xA4') ? pRead->uiData[i] : '.');
      }
      ++acIdx;
    }

    *acIdx++ =  '|';

    uiReturn = acIdx - pRender->acData;
  }

  return uiReturn;
}


/*----------------------------------------------------------------------------*/
/* renderLine_64()                                                            */
/*----------------------------------------------------------------------------*/
static uint16_t renderLine_64(
  uint16_t uiIdx,
  const readbuffer_t* pRead,
  renderbuffer_t* pRender)
{
  uint16_t uiReturn = 0;

  if ((0 != pRead) && (0 != pRender))
  {
    char_t* acIdx = &pRender->acData[uiIdx];

    *acIdx++ = '|';

    for (uint8_t i = 0; i < pRead->uiStride; i += 2)
    {
      if (between_uint32(pRead->uiAddr + i, pRead->uiLower, pRead->uiUpper, 1))
      {
        byte2hex(pRead->uiData[i], acIdx);
      }
      acIdx += 2;

      if (between_uint32(pRead->uiAddr + i + 1, pRead->uiLower, pRead->uiUpper, 1))
      {
        byte2hex(pRead->uiData[i + 1], acIdx);
      }
      acIdx += 2;

      ++acIdx;
    }

    *acIdx++ = '|';

    for (uint8_t i = 0; i < pRead->uiStride; ++i)
    {
      if (between_uint32(pRead->uiAddr + i + 1, pRead->uiLower, pRead->uiUpper, 1))
      {
        *acIdx = (between_uint8(pRead->uiData[i], ' ', '\xA4') ? pRead->uiData[i] : '.');
      }
      ++acIdx;
    }

    uiReturn = acIdx - pRender->acData;
  }

  return uiReturn;
}


/*----------------------------------------------------------------------------*/
/* renderLine_32()                                                            */
/*----------------------------------------------------------------------------*/
static uint16_t renderLine_32(
  uint16_t uiIdx,
  const readbuffer_t* pRead,
  renderbuffer_t* pRender)
{
  uint16_t uiReturn = 0;

  if ((0 != pRead) && (0 != pRender))
  {
    char_t* acIdx = &pRender->acData[uiIdx];

    *acIdx++ = '|';

    for (uint8_t i = 0; i < pRead->uiStride; ++i)
    {
      if (between_uint32(pRead->uiAddr + i + 1, pRead->uiLower, pRead->uiUpper, 1))
      {
        byte2hex(pRead->uiData[i], acIdx);
      }
      acIdx += 2;
    }

    *acIdx++ = '|';

    for (uint8_t i = 0; i < pRead->uiStride; ++i)
    {
      if (between_uint32(pRead->uiAddr + i + 1, pRead->uiLower, pRead->uiUpper, 1))
      {
        *acIdx = (between_uint8(pRead->uiData[i], ' ', '\xA4') ? pRead->uiData[i] : '.');
      }
      ++acIdx;
    }

    uiReturn = acIdx - pRender->acData;
  }

  return uiReturn;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
