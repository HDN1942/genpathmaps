/* genpathmaps.h - genPathmaps header file
 *
 * This file is part of genPathmaps - a utility for Battlefield 1942
 *
 * Copyright 2004 William Murphy
 *
 * Author: William Murphy - glyph@intergate.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __GENPATHMAPS_H__ /* only include once */
#define __GENPATHMAPS_H__

/************************************  includes              ************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#ifdef DEBUG
  #include <dmalloc.h>
#endif


#ifdef IS_UNIX
  #include <unistd.h>
#else
  #include <direct.h>
#endif

/************************************  macros                 ***********************/

#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define SUB_VERSION "a"

#define TRUE  1
#define FALSE 0

#define TILE_DIM 64
#define ROW_BYTES 8
#define TILE_BYTES TILE_DIM * ROW_BYTES
#define NUM_TYPES 5
#define BUF_SIZE  256

#define SM_MAP_RES 1024
#define MD_MAP_RES 2048
#define LG_MAP_RES 4096
#define XL_MAP_RES 8192

#define SM_MAP_SIZE 1024 * 1024
#define MD_MAP_SIZE 2048 * 2048
#define LG_MAP_SIZE 4096 * 4096
#define XL_MAP_SIZE 8192 * 8192

#define VT_TYPES 5
#define MAX_LEVEL 5
#define MAP_TYPES 3

#ifdef IS_UNIX
  #define COMSEP '-'
  #define PATHSEP '/'
  #define LINEFEED "\n"
  #define COMMENTTAG "#"
#else
  #define COMSEP '/'
  #define PATHSEP '\\'
  #define LINEFEED "\r\n"
  #define COMMENTTAG ";"
#endif

#define READ_MODE  "rb"
#define WRITE_MODE "wb"

#define BI_RGB 0
#define BI_RLE8 1
#define BI_RLE4 2
#define BI_BITFIELDS 3

#define DIB_SIGNATURE 0x4D42 /* "BM" */
#define DIB_PALVERSION 0x0300
#define DIB_PELSPERMETER 0xec3
/* #define DIB_PELSPERMETER 0xb12 */

#define PUT_2B(array,offset,value)  \
	(array[offset]   = (char) ((value) & 0xFF), \
	 array[offset+1] = (char) (((value) >> 8) & 0xFF))

#define PUT_4B(array,offset,value)  \
	(array[offset]   = (char) ((value) & 0xFF), \
	 array[offset+1] = (char) (((value) >> 8) & 0xFF), \
	 array[offset+2] = (char) (((value) >> 16) & 0xFF), \
	 array[offset+3] = (char) (((value) >> 24) & 0xFF))

#define FILENAME_MAP_RAW    "%s%c%s%dLevel%dMap.raw"
#define FILENAME_SO_RAW     "%s%c%s.raw"
#define FILENAME_INFO_RAW   "%s%c%sInfo.raw"
#define FILENAME_IMG_MAP    "%s%c%s%dLevel%dMap%s"
#define FILENAME_IMG_SO     "%s%c%s%s"
#define FILENAME_IMG_INFO   "%s%c%sInfo%s"
#define FILENAME_TXT        "%s%c%s.txt"
#define FILENAME_GRID       "%s%cGrid.raw"
#define FILENAME_NUM_GRID   "%s%cNumberedGrid.raw"   

#define FILENAME_MAP        "%s%c%s%dLevel%dMap%s"
#define FILENAME_SO         "%s%c%s%s"
#define FILENAME_INFO       "%s%c%sInfo%s"
#define FILE_8BIT_RAW_EXT   "8Bit.raw"
#define FILE_RAW_EXT        ".raw"
#define FILE_BMP_EXT        ".bmp"
#define FILE_TXT_EXT        ".txt"

/* some useful macros */
#define ABS(a) (( 1 + (a) >= 1 ) ? (a) : 0 - (a) )
#define MAX(a,b) (((a) > (b)) ? (a) : (b) )
#define MIN(a,b) (((a) < (b)) ? (a) : (b) )
#undef CLAMP
#define CLAMP(a, low, high) (((a) < (low)) ? (low) : (((a) > (high)) ? (high) : (a)))
#define INRANGE(a,b,c) (( (a) >= (b) ) && ( (a) <= (c) ))
#define BETWEEN(a,b,c) (( a > b ) && ( a < c ))
#define CONNECTED(a,b,c,d) (( (b) >= (c) ) && ( (a) <= (d) ))
#define BORDER(a,b,c,d) (( (b) >= (c) - 1 ) && ( (a) <= (d) + 1 ))
#define ISIMG(a) (( (a) & FTF_IMG ) ? 1 : 0 )
#define ISSEA(a) (( (a) == VT_BOAT ) || ( (a) == VT_LANDINGCRAFT ))
#define IMGTYPES(a) ( (a) & ( FTF_IMG - 1 ))
#define IMGFLAG(a) ( (a) & ~( FTF_IMG - 1 ))

/************************************  structures and enums  ************************/

typedef enum _errFlag
  {
    EF_NONE = 0,
    EF_USAGE,
    EF_NO_JOBS,
    EF_MALLOC,
    EF_FILE_OPEN,
    EF_FILE_SIZE,
    EF_FILE_SEEK,
    EF_FILE_READ,
    EF_FILE_WRITE,
    EF_FILE_NAME,
    EF_NOT_FOUND,
    EF_DATA_MISSING,
    EF_INFO_MISSING,
    EF_MEM_OVERRUN,
    EF_LOOP_TIMEOUT,
    EF_PASSED_NULL,
    EF_BAD_FILE,
    EF_BAD_DATA,
    EF_NOT_SUPPORTED,
    EF_FUNCTION_ERR,
    EF_UNKNOWN_ERROR,
  } errFlag;

typedef enum _debugFlag
  {
    DBG_ERR = 0,
    DBG_WARN,
    DBG_NOTICE,
    DBG_INFO,
    DBG_DEBUG
  } debugFlag;


typedef enum _readFlag
  {
    RF_NONE = 0,
    RF_BMP,
    RF_RAW,
    RF_MAP,
    RF_SO,
    RF_INFO,
    RF_TXT

  } readFlag;

typedef enum _fileTypeFlag
  {
    FTF_NONE    = 0,        /* some helpful numbers used in debugging */
    FTF_MAP     = 1 << 0,   /* 1                                      */
    FTF_INFO    = 1 << 1,   /* 2                                      */
    FTF_SO      = 1 << 2,   /* 4                                      */
    FTF_TXT     = 1 << 3,   /* 8                                      */
    FTF_IMG     = 1 << 4,   /* 16                                     */
    FTF_RAW     = 1 << 5,   /* 32                                     */
    FTF_GRID    = 1 << 6,   /* 64                                     */
    FTF_NUMBERS = 1 << 7,   /* 128                                    */
    FTF_LINES   = 1 << 8,   /* 256                                    */
    FTF_ALT     = 1 << 9,   /* 512                                    */
    FTF_READ    = 1 << 10,  /* 1024                                   */
    FTF_WRITE   = 1 << 11,  /* 2048                                   */

    FTF_GRAY    = 1 << 12,  /* 4096                                   */
    FTF_PREP    = 1 << 13,
    FTF_BATCH   = 1 << 14,
  } fileTypeFlag;	      

#define FTF_ALL_MAPS ( FTF_MAP | FTF_SO | FTF_INFO )
#define FTF_DIAG_IMG ( FTF_IMG | FTF_LINES | FTF_GRID | FTF_NUMBERS )
#define FTF_INFO_IMG ( FTF_IMG | FTF_GRID )

typedef enum _pal4Bit
  {
    GP_DOGO   = 0,
    GP_INFO0,
    GP_INFO1,
    GP_INFO2,
    GP_INFO3,
    GP_GRID,
    GP_SPECIAL,
    GP_LEVEL0_LINE,
    GP_LEVEL1_LINE,
    GP_LEVEL2_LINE,
    GP_LEVEL3_LINE,
    GP_LEVEL0_PT,
    GP_LEVEL1_PT,
    GP_LEVEL2_PT,
    GP_LEVEL3_PT,
    GP_NOGO
  } pal4Bit;

typedef enum _mapRes
  {
    SM_MAP = 1024,
    MD_MAP = 2048,
    LG_MAP = 4096,
    XL_MAP = 8192
  } mapRes;

typedef enum _compressionType
  {
    CT_NONE = 0,
    CT_MAP,
    CT_INFO
  } compressionType;

typedef enum _vehicleType
  {
    VT_NONE = -1,
    VT_TANK = 0,
    VT_INFANTRY,
    VT_BOAT,
    VT_LANDINGCRAFT,
    VT_CAR,
    VT_HELI,
    VT_AMPHIBIUS
    
  } vehicleType;

typedef enum _tileDataType
  {
    TDT_MIXED = -1,
    TDT_DOGO = 0,
    TDT_NOGO = 1

  } tileDataType;

typedef struct _bidHeader
{
  unsigned short sig;
  unsigned int   fileSize;
  unsigned short res1;
  unsigned short res2;
  unsigned int   offBits;
} __attribute__ ((packed)) bidHeader;

typedef struct _bidInfoHeader
{
  unsigned int   biSize;
  int            biWidth;
  int            biHeight;
  unsigned short biPlanes;
  unsigned short biBitCount;
  unsigned int   biCompression;
  unsigned int   biSizeImage;
  int            biXPelsPerMeter;
  int            biYPelsPerMeter;
  unsigned int   biClrUsed;
  unsigned int   biClrImportant;
  
} bidInfoHeader;

typedef struct _rgbQuad 
{
  unsigned char rgbBlue;
  unsigned char rgbGreen;
  unsigned char rgbRed;
  unsigned char rgbReserved;
}  __attribute__ ((packed)) rgbQuad;

typedef struct _smallOnesData
{
  unsigned short hasLower;
  unsigned short hasRight;
  unsigned char pt[4][2];
  unsigned char active;
  unsigned char na1;
  unsigned char na2;
  unsigned char na3;

}  __attribute__ ((packed)) smallOnesData;

typedef struct _mapFileHeader
{
  int  ln2TilesPerRow;
  int  ln2TilesPerCol;
  int  ln2TileRes;
  int  compLevel;
  int  isInfo;
  int  dataOffset;

}  __attribute__ ((packed)) mapFileHeader;

typedef struct _lineList
{
  int row;
  int begin;
  int end;
  struct _lineList *prev;
  struct _lineList *next;
} lineList;

typedef struct _tileArea
{
  int               dimX;
  int               dimY;
  int  		    top;    
  int  		    bottom;
  int  		    left;
  int  		    right;
  int  		    size;
  struct _lineList *lines;
  struct _tileArea *prev;
  struct _tileArea *next;
} tileArea;

typedef struct _mapIOData
{
  char        *path;
  fileTypeFlag type;
  vehicleType  vehicle;
  int          level;
  int          bits;
  int          inverted;
} mapIOData;

typedef struct _jobList
{
  struct _mapIOData  in;
  struct _mapIOData  out;
  struct _jobList   *next;
} jobList;

typedef struct _tileData
{
  tileDataType             flag;
  unsigned char           *bits;

} tileData;

typedef struct _tileImageData
{
  unsigned char * pt[4];
} tileImageData;

typedef struct _pathfindingmap
{
  FILE                    *fp;
  struct _mapIOData        io;

  int                      res;
  int                      tiles;
  int                      tilesPerRow;
  int                      tilesPerCol;
  int                      rowsPerTile;
  int                      bytesPerRow;
  int                      bytesPerTile;

  struct _tileData        *tile;
  struct _smallOnesData   *so;
  struct _tileImageData   *img;
  unsigned char           *bmp;
  unsigned char           *buf;

  struct _pathfindingmap  *prev;
  struct _pathfindingmap  *next;
 } pathfindingmap;

typedef struct _userData
{
  char                   *inpath;
  char                   *outpath;
  
  jobList                *jobs;

  readFlag                readflag;
  fileTypeFlag            writeflag;
  struct _pathfindingmap *maps;

  debugFlag               debug;
} userData;


#endif /* __GENPATHMAPS_H__ */

