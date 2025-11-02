/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: bindump.h                                                          |
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

#if !defined(__BINDUMP_H__)
  #define __BINDUMP_H__

/*============================================================================*/
/*                               Includes                                     */
/*============================================================================*/

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
/*!
Enumeration/list of all actions the application can execute
*/
typedef enum _action
{
  ACTION_NONE = 0,
  ACTION_HELP,
  ACTION_INFO,
  ACTION_DUMP
} action_t;

/*!
This enumeration lists all possible data sources the application is able to dump
*/
typedef enum _dumpmode
{
  DUMP_NONE = 0,
  DUMP_LOGICAL,
  DUMP_PHYSICAL,
  DUMP_FILE
} dumpmode_t;

/*!
All required information to read data from source
*/
typedef struct _readbuffer
{
  /*!
  Stride to used while passing the source
  */
  uint8_t uiStride;

  /*!
  First byte of the region to read, aligned with stride
  */
  uint32_t uiBegin;

  /*!
  Lower bound of the region to read
  */
  uint32_t uiLower;

  /*!
  Current address of the read data
  */
  uint32_t uiAddr;

  /*!
  Upper bound of the region to read
  */
  uint32_t uiUpper;

  /*!
  Last byte of the region to read, aligned with stride
  */
  uint32_t uiEnd;

  /*!
  Buffer for the read data (dynamically allocated)
  */
  uint8_t* pBuffer;
} readbuffer_t;

/*!
File information of the input file
*/
typedef struct _fileinfo
{
  char_t acPathName[ESX_PATHNAME_MAX];
  uint8_t hFile;
} fileinfo_t;

/*!
Information of the current text-screen-settings  
*/
typedef struct _screeninfo
{
  uint8_t uiCols;
  uint8_t uiRows;
} screeninfo_t;

/*!
In dieser Struktur werden alle globalen Daten der Anwendung gespeichert.
*/
typedef struct _appstate
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
  fileinfo_t tRdFile;

  /*!
  File information of the output file
  */
  fileinfo_t tWrFile;

  /*!
  Backup: Current speed of Z80N
  */
  uint8_t uiCpuSpeed;

  /*!
  Information of the current text-screen-settings  
  */
  screeninfo_t tScreen;

  /*!
  All required information to read data from source
  */
  readbuffer_t read;

  /*!
  Exitcode of the application, that is handovered to BASIC
  */
  int iExitCode;
} appstate_t;

/*============================================================================*/
/*                               Prototypen                                   */
/*============================================================================*/
/*!
*/
int readLine(dumpmode_t eMode, readbuffer_t* pBuffer);

/*!
*/
int renderScreenLine(readbuffer_t* pBuffer);

/*!
*/
int renderFileLine(readbuffer_t* pBuffer);

/*!
*/
int saveLine(readbuffer_t* pBuffer, fileinfo_t* pFile);

/*============================================================================*/
/*                               Klassen                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementierung                              */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

#endif /* __BINDUMP_H__ */
