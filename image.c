/* image.c - functions used for bitmap image input and output
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


/************************************  includes              ************************/
#include "common.h"
#include "commonutils.h"

#include "image.h"
#include "smallones.h"
#include "pathfindingmap.h"

/************************************  global variables      ************************/

unsigned char gridNumbers [10][3] = {
  { 0x96, 0x99, 0x69 },
  { 0x22, 0x22, 0x22 },
  { 0x96, 0x61, 0xF8 },
  { 0x96, 0x12, 0x69 },
  { 0x31, 0xF5, 0x11 },
  { 0x8F, 0x1E, 0x69 },
  { 0x86, 0x9E, 0x69 },
  { 0x1F, 0x42, 0x44 },
  { 0x96, 0x96, 0x69 },
  { 0x96, 0x79, 0x61 }
};

rgbQuad defaultColors[16] = {
  { 0x00, 0x00, 0x00, 0x00 },

  { 0x60, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x60, 0x00 },
  { 0x00, 0x40, 0x40, 0x00 },
  { 0x40, 0x00, 0x40, 0x00 },
  { 0x20, 0x20, 0x20, 0x00 },

  { 0x20, 0xa0, 0xa0, 0x00 },
  { 0xf0, 0xf0, 0x80, 0x00 },
  { 0x00, 0x00, 0xa0, 0x00 },
  { 0xa0, 0x00, 0x00, 0x00 },
  { 0x00, 0xa0, 0x00, 0x00 },
  { 0x80, 0xf0, 0xf0, 0x00 },
  { 0x80, 0x80, 0xf0, 0x00 },
  { 0xf0, 0x80, 0x80, 0x00 },
  { 0x80, 0xf0, 0x80, 0x00 },
  { 0xff, 0xff, 0xff, 0x00 }

};

unsigned char colors[16] = {
  0,                        /* DoGo                                                   */
  23,                       /* info1                                                  */
  47,                       /* info2                                                  */
  71,                       /* info3                                                  */
  127,                      /* level 0 line                                           */
  143,                      /* level 1 line                                           */
  159,                      /* level 2 line                                           */
  175,                      /* level 3 line                                           */
  207,                      /* level 0                                                */
  219,                      /* level 1                                                */
  231,                      /* level 2                                                */
  243,                      /* level 3                                                */
  31,                       /* grid                                                   */
  255,                      /* special (currently used for unknowns)                  */
  255,                      /* special (currently used for unknowns)                  */
  255                       /* NoGo                                                   */
};



extern char *baseName[];

/************************************  functions             ************************/

void
loadImage ( pathfindingmap *map )
{
  if ( map->io.type & FTF_RAW ) {
    /* only accept the 3 allowed map sizes (are there more?) */
    switch ( fileSize ( map ))
      {
      case SM_MAP_SIZE:
	map->res = SM_MAP_RES;
	break;
      case MD_MAP_SIZE:
	map->res = MD_MAP_RES;
	break;
      case LG_MAP_SIZE:
	map->res = LG_MAP_RES;
	break;
      case XL_MAP_SIZE:
	map->res = XL_MAP_RES;
	break;
      default:
	shutdown ( EF_FILE_SIZE,
		   "Bad file size for 8 bit %s map.\n",
		   baseName[map->io.vehicle] );
      }
    readPixels ( map, 8 );
  } else {
    loadBmpHeader ( map );
    readPixels ( map, map->io.bits );
  }


} /* end loadMap8Bit */

void
loadBmpHeader ( pathfindingmap *map )
{
  bidHeader      header;
  bidInfoHeader  infoHeader;
  int numColors;
  rgbQuad color;

    /* read pathfinding file header */
  if ( !fread ( &header, sizeof ( bidHeader ), 1, map->fp ))
      shutdown ( EF_FILE_READ,
		 "Error reading %s bitmap header\n", baseName[map->io.vehicle] );

  if ( header.sig != DIB_SIGNATURE )
    shutdown ( EF_BAD_FILE,
	       "%s Bitmap file has bad signature\n",
	       baseName[map->io.vehicle] );

  if ( (int) header.fileSize != fileSize ( map ) )
    shutdown ( EF_BAD_FILE,
	       "%s bitmap file size does not match header filesize\n",
	       baseName[map->io.vehicle] );

  if ( !fread ( &infoHeader, sizeof ( bidInfoHeader ), 1, map->fp ))
      shutdown ( EF_FILE_READ,
		 "Error reading %s bitmap info header\n", baseName[map->io.vehicle] );

  if ( infoHeader.biSize != 40 )
    shutdown ( EF_BAD_DATA,
	       "Unexpected bitmap info header size for %s file.\n",
	       baseName[map->io.vehicle] );

  if (( infoHeader.biWidth != infoHeader.biHeight ) ||
      !(( infoHeader.biWidth == SM_MAP ) ||
	( infoHeader.biWidth == MD_MAP ) ||
	( infoHeader.biWidth == XL_MAP ) ||
	( infoHeader.biWidth == LG_MAP )))
    shutdown ( EF_BAD_DATA,
	       "%s bitmap file has the wrong dimentions\n",
	       baseName[map->io.vehicle] );

  if ( infoHeader.biCompression )
    shutdown ( EF_BAD_DATA,
	       "%s bitmap file compression is unsupported.\n",
	       baseName[map->io.vehicle] );

  numColors = ( header.offBits - sizeof ( bidHeader ) - sizeof ( bidInfoHeader )) / 4;

  if ( infoHeader.biClrUsed ) {
    if ( numColors != (int) infoHeader.biClrUsed )
      shutdown ( EF_BAD_DATA,
		 "%s bitmap file allotted colors mismatch.\n",
		 baseName[map->io.vehicle] );
  } else 
    infoHeader.biClrUsed = numColors;

  map->io.bits = findLn2 ( numColors );

  if ( !fread ( &color, sizeof ( rgbQuad ), 1, map->fp ))
    shutdown ( EF_BAD_DATA,
	       "Error reading color table in image file.\n",
	       baseName[map->io.vehicle] );
  if ( color.rgbRed ) map->io.inverted = TRUE;

  /* set file pointer to data */
  fseek ( map->fp, header.offBits, SEEK_SET );

  if ( infoHeader.biBitCount >= 24 )
    shutdown ( EF_BAD_DATA,
	       "%s bitmap file must be 1 or 8 bit\n",
	       baseName[map->io.vehicle] );

  map->io.bits = infoHeader.biBitCount;
  map->res = infoHeader.biWidth;
} /* end loadBmpHeader */

void
writeImageFile ( pathfindingmap *map )
{
  int row;
  int bufSize;
  int mapDim;
  int tileDim;
  int mult;

  if ( map->io.type & FTF_RAW )  map->io.bits = 8;
  else if ( map->io.type & FTF_MAP )
    map->io.bits = ( map->io.type & FTF_PREP ) ? 4 : 1;
  else if ( map->io.type & ( FTF_SO | FTF_INFO ))
    map->io.bits = 4;

  mult    = (( map->io.type & FTF_INFO ) ? 1 : ( 1 << map->io.level ));
  mapDim  = map->res * mult;
  tileDim = TILE_DIM * mult;
  bufSize = ( mapDim * tileDim * map->io.bits ) / 8;

  /* create output buffer - image width x tile length */
  if ((map->buf  = (unsigned char *) malloc ( bufSize )) == NULL )
    shutdown ( EF_MALLOC, "Error creating output buffer\n" );

  /* if not outputing raw, write bitmap header */
  if ( !( map->io.type & FTF_RAW )) writeBmpHeader ( map );

  /* process the file one tile row at a time */

  /* zero the row of tiles */
  for ( row=0; row < map->tilesPerCol; row += 1 ) {

    memset ( map->buf, colors [ GP_DOGO ], bufSize );

    if (!( map->io.type & FTF_MAP )) prepImageBuf ( map, row );

    switch ( IMGTYPES(map->io.type))
      {
      case FTF_SO:
	if ( map->io.type & FTF_PREP )
	  plotImageRow ( map, row );
	else
	  plotSmallOnesRow ( map, row );
	break;

      case FTF_INFO:
      case FTF_MAP:
	plotImageRow ( map, row );
	break;
      case FTF_NONE:
	if ( map->io.type & FTF_GRID ) break;
      default:
	shutdown ( EF_BAD_DATA, "Function writeImageFile passed bad map type\n");
      }

    /* write this row of tiles to plot file */
    if ( !fwrite (map->buf, bufSize, 1, map->fp ))
      shutdown ( EF_FILE_WRITE, "Error writing to plot image: %s8bit.raw\n",
		 baseName[map->io.vehicle] );

  }
  free ( map->buf );
  map->buf = NULL;
}

void
writeBmpHeader ( pathfindingmap *map )
{

  bidHeader      header = { 0 };

  bidInfoHeader  infoHeader = { 0 };
  rgbQuad        bmpColors[256];


  int res;
  int numColors;
  int i;

  numColors =  1 << map->io.bits;

  header.sig = DIB_SIGNATURE;
  header.offBits = ( sizeof ( bidHeader ) +
		     sizeof ( bidInfoHeader ) +
		     sizeof ( rgbQuad ) * ( 1 << map->io.bits ));

  res = map->res << (( map->io.type & FTF_INFO ) ? 0 : map->io.level);
  header.fileSize = header.offBits + res * res * map->io.bits / 8;


  if ( ! fwrite ( &header, sizeof (bidHeader), 1, map->fp ))
    shutdown ( EF_FILE_WRITE,
	       "Error writing %s bitmap header\n",
	       baseName[ map->io.vehicle ] );

  infoHeader.biSize          = 40;
  infoHeader.biWidth         = infoHeader.biHeight = res;
  infoHeader.biPlanes        = 1;
  infoHeader.biBitCount      = map->io.bits;
  infoHeader.biXPelsPerMeter = infoHeader.biYPelsPerMeter = DIB_PELSPERMETER;
  infoHeader.biClrUsed       = infoHeader.biClrImportant = numColors;

  if ( ! fwrite ( &infoHeader, sizeof (bidInfoHeader), 1, map->fp ))
    shutdown ( EF_FILE_WRITE,
	       "Error writing %s bitmap header\n",
	       baseName[ map->io.vehicle ] );

  if ( map->io.bits == 1 ) {
    map->io.type|= FTF_GRAY;
    colors[0] = 0; colors[1] = 1;
  }
  if ( map->io.bits == 4 ) for ( i = 0; i < 16; i++ ) colors[i] = i;

  if ( map->io.type & FTF_GRAY ) for ( i = 0; i < numColors; i++ ) {
    bmpColors[i].rgbBlue = bmpColors[i].rgbGreen = bmpColors[i].rgbRed =
      i * 255 / ( numColors - 1 ) ;
    bmpColors[i].rgbReserved = 0;
  }

  if ( ! fwrite ( (( map->io.type & FTF_GRAY ) ? bmpColors : defaultColors ),
		  sizeof (rgbQuad) * numColors, 1, map->fp ))
    shutdown ( EF_FILE_WRITE,
	       "Error writing %s bitmap color map\n",
	       baseName[ map->io.vehicle ] );
} /* end writeBmpHeader */

void
plotImageRow ( pathfindingmap *map, int row )
{
  int offset, byteOff;
  int bytesPerRow, rowsPerTile;
  int color;
  int col;
  int tileRow, rowByte, bit;
  int recX1, recX2, recY1, recY2;
  int colorIndex;
  int srcInc, dstInc;
  int tileDim;
  int mult;
  int noGoColor;

  rowsPerTile = map->rowsPerTile;
  bytesPerRow = map->bytesPerRow;

  srcInc = ( map->io.type & FTF_INFO ) ? 2 : 1;
  dstInc = 1 << map->io.level;
  mult    = (( map->io.type & FTF_INFO ) ? 1 : ( 1 << map->io.level ));
  tileDim = TILE_DIM * mult;
  noGoColor = ( map->io.bits == 8 ) ? 15 : ( 1 << map->io.bits ) - 1;
  for ( col = 0; col < map->tilesPerRow; col++ ) {
    offset = row * map->tilesPerCol + col;

    if ( map->tile ) {
      switch ( map->tile[offset].flag )
	{
	case TDT_DOGO:
	  if ( map->io.type & FTF_INFO )
	    drawRectangle ( map,
			    col, 0, 0,
			    col, tileDim - 1, tileDim - 1,
			    colors[ GP_INFO0 ] );

	  continue;
	case TDT_NOGO:
	  drawRectangle ( map,
			  col, 0, 0,
			  col, tileDim - 1, tileDim - 1,
			  colors[ noGoColor ] );
	  break;
	case TDT_MIXED:
	  for ( tileRow = 0; tileRow < rowsPerTile; tileRow++ )

	    for ( rowByte = 0; rowByte < bytesPerRow; rowByte++ ) {
	      for ( bit = 0; bit < 8; bit += srcInc ) {

		byteOff = tileRow * bytesPerRow + rowByte;

		if ( map->io.type & FTF_INFO ) {

		  /* set the info region color */
		  colorIndex = (( map->tile[offset].bits[byteOff]  >> bit ) & 3 );
		  color = colors [ (( colorIndex < 3 ) ? colorIndex + 1: noGoColor )];

		} else {
		  /* set the  region color */
		  color = ( map->tile[offset].bits[byteOff] & ( 1 << bit )) ?
		    colors [ noGoColor ] : colors [ GP_DOGO ];
		}
		if ( color ) {

		  recX1 = ((rowByte * 8 + bit) * dstInc) / srcInc ;
		  recX2 = ((rowByte * 8 + bit + srcInc) * dstInc) / srcInc - 1;

		  recY1 = (tileRow*srcInc * dstInc) / srcInc;
		  recY2 = ((tileRow*srcInc + srcInc) * dstInc / srcInc) - 1;

		  if (( recX1 == recX2 ) && ( recY1 == recY2 ))
		    plotPoint ( map, col, recX1, recY1, color );
		  else
		    drawRectangle ( map,
				    col, recX1, recY1,
				    col, recX2, recY2, color );
		}
	      }
	    }

      }
    }
  } /* end col loop */
} /* end plot8BitRow */

void
prepImageBuf ( pathfindingmap *map, int row )
{
  unsigned char buffer [25];
  int addGrid;
  int i, j;
  int highNibble, offbit;
  int numByte;
  int offset;
  int rowOff;
  int col;

  if (!( map->io.type & ( FTF_GRID | FTF_NUMBERS )) ||
      ( map->io.type & FTF_MAP )) return;

  for ( col = 0; col < map->tilesPerRow; col++ ) {
    addGrid = ((( row & 1 ) && ( col & 1 )) || (!( row & 1 ) && !(col & 1 )));

    if ( map->io.type & FTF_GRID ) {
      /* draw checkerboard grid */
      if (addGrid)
	drawRectangle ( map,
			col, 0, 0,
			col, TILE_DIM - 1, TILE_DIM - 1,
			colors[ GP_GRID ] );

    }
    if ( map->io.type & FTF_NUMBERS ) {
      memset ( buffer, 0, sizeof ( buffer ) );

      /* fill buffer */
      for ( i = 0; i < 6; i++ ) {

	/* draw dash */
	if ( i == 3 ) for ( j = 0; j < 4; j++ )
	  buffer[10+j] = colors[ GP_SPECIAL ];

	for ( j = 0; j < 4; j++ ) {

	  highNibble = i % 2;
	  numByte    = i >> 1;
	  offbit     = 1 << (highNibble * 4 + 3-j );

	  /* draw col's 10's place */
	  if ( gridNumbers[ col/10 ][ numByte ] & offbit )
	    buffer[j] = colors[ GP_SPECIAL ];
	  /* draw col's 1's place */
	  if ( gridNumbers[ col%10 ][ numByte ] & offbit )
	    buffer[j+5] = colors[ GP_SPECIAL ];

	  /* draw row's 10's place */
	  if ( gridNumbers[ row/10 ][ numByte ] & offbit )
	    buffer[j+15] = colors[ GP_SPECIAL ];
	  /* draw row's 1's place */
	  if ( gridNumbers[ row%10 ][ numByte ] & offbit )
	    buffer[j+20] = colors[ GP_SPECIAL ];
	}
	rowOff = ( map->io.type & FTF_RAW ) ? i + NUM_OFF_Y : TILE_DIM - i - NUM_OFF_Y - 1;
	offset     = rowOff * map->res + col * TILE_DIM + NUM_OFF_X;

	for ( j = 0; j < sizeof ( buffer ); j++ )
	  if ( buffer[j] ) setRowPixel ( map, offset + j, buffer[j] );

	memset ( buffer, 0, sizeof( buffer ));
      }
    }
  }
}

void
plotSmallOnesRow ( pathfindingmap *map, int row )
{
  smallOnesData *small;
  smallOnesData *target;
  int offset;
  int i, j;
  int col;


  /* step thru tiles */
  for ( col=0; col < map->tilesPerRow; col++ ) {

    offset = row * map->tilesPerRow + col;
    small = &(map->so[offset]);

    /* i have no idea what these fields are for - mark them anyway */
    if (small->na1 > 0 ) {
      drawLine ( map,
		 col, 0, 0,
		 col, TILE_DIM - 1, TILE_DIM - 1,
		 colors[ GP_SPECIAL ] );
      drawLine ( map,
		 col, TILE_DIM - 1, 0,
		 col, 0, TILE_DIM - 1,
		 colors[ GP_SPECIAL ] );
    }
    if (small->na2 > 0 ) {
      drawLine ( map,
		 col, 31, 0,
		 col, 31, TILE_DIM - 1,
		 colors[ GP_SPECIAL ] );
      drawLine ( map,
		 col, 0, 31,
		 col, TILE_DIM - 1, 31,
		 colors[ GP_SPECIAL ] );
    }
    if (small->na3 > 0 ) {
      drawRectangle ( map,
		      col, 24, 24,
		      col, 39, 39,
		      colors[ GP_SPECIAL ] );
    }

    /* check for each point */
    for ( i= 0; i < 4; i++ ) {
      /* if not set, break now */
      if (( small->active & ( 1 << ( i + 4 ))) == 0 ) continue;

      if ( map->io.type & FTF_LINES ) {
	/* connect the dots with lines */
	if ( row > 0 ) {
	  /* check previous row for a connection */
	  target = &(map->so[offset-map->tilesPerRow]);
	  for ( j = 0; j < 4; j++ ) {
	    /* if not set, break now */
	    if ( target->hasLower & ( 1 << ( j * 4 + i ))) {
	      drawLine ( map,
			 col, small->pt[i][0], small->pt[i][1],
			 col-map->tilesPerRow, target->pt[j][0], target->pt[j][1],
			 colors[GP_LEVEL0_LINE + j] );
	    }
	  }
	}
	if ( col < map->tilesPerRow - 1 ) {
	  /* check this row for connection */
	  target = &(map->so[offset+1]);
	  for ( j = 0; j < 4; j++ ) {
	    /* if not set, break now */
	    if ( small->hasRight & ( 1 << ( i * 4 + j ))) {
	      drawLine ( map,
			 col, small->pt[i][0], small->pt[i][1],
			 col+1, target->pt[j][0], target->pt[j][1],
			 colors[GP_LEVEL0_LINE + i] );
	    }
	  }
	}
	if ( row < map->tilesPerRow - 1 ) {
	  /* check next row for a connection */
	  target = &(map->so[offset+map->tilesPerRow]);
	  for ( j = 0; j < 4; j++ ) {
	    /* if not set, break now */
	    if ( small->hasLower & ( 1 << ( i * 4 + j ))) {
	      drawLine ( map,
			 col, small->pt[i][0], small->pt[i][1],
			 col+map->tilesPerRow, target->pt[j][0], target->pt[j][1],
			 colors[GP_LEVEL0_LINE + i] );
	    }
	  }
	}
      }
      if (( small->pt[i][0] >= TILE_DIM ) && ( small->pt[i][1] >= TILE_DIM ))
	drawRectangle ( map,
			col,
			MAX( small->pt[i][0]%TILE_DIM - 2, 0 ),
			MAX( small->pt[i][1]%TILE_DIM - 2, 0 ),
			col,
			MIN( small->pt[i][0]%TILE_DIM + 2, TILE_DIM - 1 ),
			MIN( small->pt[i][1]%TILE_DIM + 2, TILE_DIM - 1 ),
			colors[ GP_LEVEL0_PT + i] );
      else
	drawRectangle ( map,
			col,
			MAX( small->pt[i][0]%TILE_DIM - 1, 0 ),
			MAX( small->pt[i][1]%TILE_DIM - 1, 0 ),
			col,
			MIN( small->pt[i][0]%TILE_DIM + 1, TILE_DIM - 1 ),
			MIN( small->pt[i][1]%TILE_DIM + 1, TILE_DIM - 1 ),
			colors[ GP_LEVEL0_PT + i ] );
    }
  }
}

void
plotPoint ( pathfindingmap *map, int tile, int col, int row, int value )
{
  int offset;
  int tileDim;
  int mapDim;
  int mult;

  if ( !map )
    shutdown ( EF_PASSED_NULL, "plotPoint passed NULL pointer to buffer\n" );

  mult = ( 1 << map->io.level );
  tileDim = TILE_DIM * mult;
  mapDim = map->res * mult;
  /* calc offset into buffer */
  offset = ( map->tilesPerRow * row + tile ) * tileDim  + col;

  /* make sure we're on the same planet */
  if ((offset < 0 ) ||  ( offset >= mapDim * tileDim ))
    shutdown ( EF_MEM_OVERRUN, "plotPoint tried to write past buffer boundry\n" );

  /* put value into point */

  setRowPixel ( map, offset, value );
}

void
drawRectangle ( pathfindingmap *map,
		int tile1, int col1, int row1,
		int tile2, int col2, int row2,
		int value )
{
  int i, j;
  int o1, o2;
  int temp;
  int mult;
  int tileDim, mapRes, bufSize;


  if ( !map || !map->buf )
    shutdown ( EF_PASSED_NULL, "plotRectangle passed NULL pointer to image\n" );

  mult = ( map->io.type & FTF_INFO ) ? 1 : (1 << map->io.level);
  tileDim = TILE_DIM * mult;
  mapRes = map->res * mult;
  bufSize = mapRes * tileDim;

  /* fix the rectangle point order, if needed */
  if ( tile1 > tile2 ) {
    temp = tile1; tile1 = tile2; tile2 = temp;
    temp = col1; col1 = col2; col2 = temp;
    temp = row1; row1 = row2; row2 = temp;
  }
  if ( row1 > row2 ) {
    temp = row1; row1 = row2; row2 = temp;
  }

  o1 = ( map->tilesPerRow * row1 + tile1 ) * tileDim + col1;
  o2 = ( map->tilesPerRow * row2 + tile2 ) * tileDim + col2;

  if (( o1 < 0 ) || ( o2 < 0 ) ||
      ( o1 >= bufSize ) || ( o2 >= bufSize ))
    shutdown ( EF_MEM_OVERRUN, "drawRectangle tried to write past buffer boundry\n" );


  for ( i = row1; i <= row2; i++ )
    for ( j = (tile1 * tileDim) + col1; j <= (tile2 * tileDim) + col2; j++ )
      setRowPixel ( map, i * mapRes + j, value );

} /* end drawRectangle */

void
drawLine ( pathfindingmap *map,
	   int tile1, int col1, int row1,
	   int tile2, int col2, int row2,
	   int value )
{
  int i, j;
  int c1, c2;
  int cb, ce;
  int rb, re;
  int cd, rd;
  int rise, run;

  if ( !map || !map->buf )
    shutdown ( EF_PASSED_NULL, "drawLine: passed NULL pointer to buffer\n" );

  if (( tile1 < 0 ) || ( tile1 >= map->tilesPerRow ))
    shutdown ( EF_MEM_OVERRUN, "drawLine: tile out of range\n");

  /* this is only a guess, but some tiles are out of range for some strange reason */
  col1 %= TILE_DIM;
  col2 %= TILE_DIM;
  row1 %= TILE_DIM;
  row2 %= TILE_DIM;

  if ( tile2 < 0 ) {
    tile2 += map->tilesPerRow;
    row2 -= TILE_DIM;
  } else if ( tile2 >= map->tilesPerRow ) {
    tile2 -= map->tilesPerRow;
    row2 += TILE_DIM;
  }

  c1 = tile1 * TILE_DIM + col1;
  c2 = tile2 * TILE_DIM + col2;
  cd = ( c1 < c2 ) ? 1 : -1;
  rd = ( row1 < row2 ) ? 1 : -1;

  rise = row2 - row1;
  run = c2 - c1;

  cb = ce = rb = re = 0;

  if ( ABS(run) > ABS(rise) ) {
    for ( i = row1; i * rd <= row2 * rd; i += rd ) {

      /* make sure we're still in the buffer */
      if ((  i < 0 ) || ( i >= TILE_DIM )) break;

      /* FIXME: this has got to be wrong... */
      if ( rise ) {
	ce = ( 2 * (i - row1) + rd) * run / (2 * rise);
	if (( 2 * ( i - row1 ) + rd ) * run > 2 * rise ) ce += cd;
      } else ce = run;

      for ( j = cb; j*cd <= ce*cd; j += cd ) {
	if (( j + c1 ) * cd > ( c2 * cd )) {
	  i = row2;
	  break;
	}
	setRowPixel ( map, i * map->res + c1 + j, value );
      }

      cb = ce;
    }
  } else {
    for ( i = c1; i * cd <= c2 * cd; i += cd ) {

      if ( run ) {
	re = ( 2 * ( i - c1 ) + cd ) * rise / ( 2 * run );
	if (( 2 * (i - c1) + cd ) * rise > 2 * run ) re += rd;
      } else re = rise;

      for ( j = rb; j*rd <= re*rd; j += rd ) {
	/* make sure we're still in the buffer */
	if (( row1 + j < 0 ) || ( row1 + j >= TILE_DIM )) {
	  i = c2;
	  break;
	}
	setRowPixel ( map, (row1 + j) * map->res + i, value );
      }
      rb = re;
    }
  }
} /*end drawLine */

void
readPixels ( pathfindingmap *map, int inBits )
{
  unsigned char tileBuf[TILE_BYTES];
  int i, j, k;
  int tileRow, tileCol;
  int hasNoGo, hasDoGo;
  int byteOff;
  int curTile;
  int bufSize;
  int bufRowBytes;
  int curPix;
  int curByte;
  int mask;
  int *p;

  /* how many TILE_DIM (64 byte) blocks per row */
  map->tilesPerCol = map->tilesPerRow = map->res / TILE_DIM;
  map->tiles = map->tilesPerRow * map->tilesPerCol;
  map->rowsPerTile = TILE_DIM;
  map->bytesPerRow = TILE_DIM / 8;
  map->bytesPerTile = TILE_DIM * TILE_DIM / 8;

  bufRowBytes = ( map->res * inBits ) >> 3;
  bufSize = ( bufRowBytes ) * TILE_DIM;

  /* create input buffer - image width x tilement length */
  if (!(map->buf = (unsigned char *) malloc ( bufSize )))
    shutdown ( EF_MALLOC,
	       "Error creating %s input image buffer for.\n",
	       baseName[map->io.vehicle] );

  /* create tile data record array */
  if ( !( map->tile = (tileData *) calloc ( sizeof (tileData) * map->tiles, 1 )))
    shutdown ( EF_MALLOC,
	       "Error creating %s tile data array buffer for.\n",
	       baseName[map->io.vehicle] );

  /* loop thru each row of tiles */
  for ( tileRow = 0; tileRow < map->tilesPerCol; tileRow++ ) {
    /* read map->tilePerRow tiles into buf */
    if ( !fread ( map->buf, bufSize, 1, map->fp ))
      shutdown ( EF_FILE_READ,
		 "Error reading from %s input image file.\n",
		 baseName[map->io.vehicle] );


    if ( map->io.inverted ) {
      p = (int *) map->buf;
      for ( i = 0; i < bufSize; i += sizeof ( int ) ) {
	*p = ~(*p);
	p++;
      }
    }
    /* scan input buffer one tile at a time */
    for ( tileCol = 0; tileCol < map->tilesPerRow; tileCol++ ) {
      curTile = tileRow * map->tilesPerCol + tileCol;

      /* zero the buffer */
      memset ( tileBuf, 0, TILE_BYTES );

      hasNoGo = hasDoGo = FALSE;

      /* loop thru each row in tile */
      for ( i = 0; i < TILE_DIM; i++ ) {

	/* pack each tile row */
	for ( j = 0; j < ROW_BYTES; j++ ) {

	  byteOff = i * map->bytesPerRow + j;
	  curPix = i * map->res + tileCol * TILE_DIM + j * 8;

	  for ( k = 0; k < 8; k++ ) { /* check each pixel */
	    curByte = (( curPix + k ) * inBits ) / 8;
	    mask = (( 1 << inBits ) - 1) << ( inBits * (( 7 - k ) / inBits ));
	    if ( map->buf[ curByte ] & mask ) {
	      hasNoGo = TRUE;
	      tileBuf[ byteOff ] |= ( 1 << k );
	    } else hasDoGo = TRUE;
	  }
	} /* end j loop */
      } /* end i loop */
      if ( !hasDoGo || !hasNoGo ) {
	/* mark tiles with all DoGo's or NoGo's - don't copy data */
	if ( hasDoGo ) {
	  map->tile[curTile].flag = TDT_DOGO;
	} else if ( hasNoGo ) {
	  map->tile[curTile].flag = TDT_NOGO;
	} else
	  shutdown ( EF_BAD_DATA,
		     "Tile has neither DoGo's or NoGo's - this should never happen\n" );
      } else {
	/* this one has mixed data - no choice must copy */
	map->tile[curTile].flag = TDT_MIXED;
	map->tile[curTile].bits = (unsigned char *) malloc ( TILE_BYTES);
	memcpy ( map->tile[curTile].bits, tileBuf, TILE_BYTES );
      }
    } /* end for tileCol loop */
  } /* end for tileRow loop */

  /* clean up a little */
  free ( map->buf );
  map->buf = NULL;
}

void
setRowPixel ( pathfindingmap *map, int offset, int value )
{
  int byteOff;
  int bitOff;
  int pixPerByte;
  int mask;

  if ( map->io.bits == 8 ) map->buf[offset] = value;
  else {
    byteOff = (offset * map->io.bits ) / 8;
    pixPerByte = 8 / map->io.bits;
    bitOff = ( map->io.type & FTF_RAW ) ?
      ( offset % pixPerByte ) : pixPerByte - ( offset % pixPerByte ) - 1;
    mask = ((( 1 << map->io.bits ) - 1) << ( bitOff * map->io.bits ));
    map->buf[byteOff] &= ~mask;
    map->buf[byteOff] |= ( value << ( bitOff * map->io.bits ));
  }
}

void
fillColorMap ( pathfindingmap *map, rgbQuad *bmpColors )
{
  int i;
  int numColors;

  numColors = 1 << map->io.bits;
  /* FIXME: this is dummied out for now, put some colors in here */
  for ( i = 0; i < numColors; i++ ) {
    bmpColors[i].rgbBlue = bmpColors[i].rgbGreen = bmpColors[i].rgbRed =
      i * 255 / ( numColors - 1 );
    bmpColors[i].rgbReserved = 0;
  }

  if ( map->io.bits == 1 ) colors[ GP_NOGO ] = 1;
  if ( map->io.bits == 4 ) for ( i = 0; i < 16; i++ ) colors[i] = i;
}

