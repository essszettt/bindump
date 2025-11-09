/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: render.h                                                           |
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

#if !defined(__RENDER_H__)
  #define __RENDER_H__

/*============================================================================*/
/*                               Includes                                     */
/*============================================================================*/

/*============================================================================*/
/*                               Defines                                      */
/*============================================================================*/
/*!
First character to show in ASCII-dump
*/
#define cFIRST_CHAR ' ' 

/*!
Last character to show in ASCII-dump
*/
#define cLAST_CHAR '\x8F' 

/*!
Character used as separator between columns
*/
#define cSEPERATOR_CHAR '|'

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
This function renders a dataframe in to a visual representation (hex dump).
@param pScreen Pointer to the screen parameters
@param pRead Pointer to the read buffer
@param pRender Pointer to the render buffer
@return EOK = no error
*/
int renderFrame(
  const screeninfo_t* pScreen,
  const readbuffer_t* pRead,
  renderbuffer_t* pRender);

/*============================================================================*/
/*                               Klassen                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementierung                              */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

#endif /* __RENDER_H__ */
