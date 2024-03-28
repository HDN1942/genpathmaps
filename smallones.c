/* smallOnes.c - support function for smallOnes files
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
#include "pathfindingmap.h"
#include "smallones.h"
#include "textfile.h"

/************************************  global variables      ************************/

unsigned char circleArr[128] =
  {
    128, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    126, 126, 126, 126, 126, 126, 126, 125, 125, 125, 125, 125, 124, 124, 124, 124,
    123, 123, 123, 123, 122, 122, 122, 121, 121, 121, 120, 120, 120, 119, 119, 119,
    118, 118, 117, 117, 116, 116, 116, 115, 115, 114, 114, 113, 113, 112, 111, 111,
    110, 110, 109, 109, 108, 107, 107, 106, 105, 105, 104, 103, 102, 102, 101, 100,
     99,  99,  98,  97,  96,  95,  94,  93,  92,  91,  91,  90,  88,  87,  86,  85,
     84,  83,  82,  81,  79,  78,  77,  75,  74,  73,  71,  70,  68,  67,  65,  63,
     61,  60,  58,  56,  54,  51,  49,  47,  44,  41,  38,  35,  31,  27,  22,  15,
  };

extern char *baseName[];

/************************************  functions             ************************/

pathfindingmap *
genSmallOnes ( pathfindingmap *map )
{
  pathfindingmap *soMap;
  pathfindingmap *srcMap;
  mapIOData       io;
  tileData       *tile;
  tileImageData  *img;

  int tileRow, tileCol;
  int curTile;

  /* make sure everyting is here, shutdown if it is not the case that:   */
  if ( !(     map                        && /* there are maps              */
	      ( map->io.type & FTF_MAP ) && /* it's a type we can convert  */
	      ( map->io.level == 0 )     &&
	      map->tile ))                  /* and it is not compressed    */
    shutdown ( EF_INFO_MISSING,
	       "Function genSmallOnes passed incomplete or incorrect data\n" );

  /* fill in info */
  srcMap     = map;
  io.path    = NULL;
  io.type    = FTF_SO;
  io.level   = 0;
  io.vehicle = srcMap->io.vehicle;

  /* get the map */
  if ( !( soMap = getMap ( &map, &io, NULL )))
    shutdown ( EF_MALLOC, "Error creating pathfinding map in function genSmallOnes\n" );


  /* col and row are the same */
  soMap->tilesPerCol  = soMap->tilesPerRow = srcMap->tilesPerRow;
  soMap->tiles        = soMap->tilesPerCol * soMap->tilesPerRow;
  soMap->rowsPerTile  = srcMap->rowsPerTile;
  soMap->bytesPerRow  = srcMap->bytesPerRow;
  soMap->bytesPerTile = srcMap->bytesPerTile;
  soMap->res          = srcMap->res;

    /* allocate smallOnes buffer */
  if (!( soMap->so =
	 (smallOnesData *) calloc ( sizeof(smallOnesData) * soMap->tiles, 1 )))
    shutdown ( EF_MALLOC,
	       "Memory allocation error creating smallOnesData buffer\n" );

  if (!( soMap->img =
	 (tileImageData *) calloc ( sizeof ( tileImageData ) * soMap->tiles, 1 )))
    shutdown ( EF_MALLOC,
	       "Memory allocation error creating tile image buffer\n" );

  /* adjust tiles for each map */
  soMap->tile = copyTiles( srcMap );

  /* simplify things */
  tile = soMap->tile;
  img  = soMap->img;

  for ( tileRow = 0; tileRow < soMap->tilesPerRow; tileRow++ ) {
    for ( tileCol = 0; tileCol < soMap->tilesPerCol; tileCol++ ) {
      /* get offset into tile and so arrays */
      curTile = tileRow * soMap->tilesPerCol + tileCol;

      if ( tile[ curTile ].flag == TDT_NOGO ) {
	/* smallOne is already zero'ed */
	continue;
      } else if ( tile[ curTile ].flag == TDT_DOGO ) {

	/* allocate tile image (all zero's) */
	if (!( img[ curTile ].pt[0] = (unsigned char *) calloc ( TILE_BYTES, 1 )))
	  shutdown ( EF_MALLOC, "Memory allocation error creating tile image\n" );

	setPoint ( soMap, curTile, 0, DEF_OFF, DEF_OFF );

      } else if ( tile[ curTile ].flag == TDT_MIXED ) {
	/* this tile has both NoGo's and DoGo's - find contiguos areas */
	findAreas ( soMap, curTile );

      } /*end MIXED */
    } /* end tileCol */
  } /* end tileRow loop */

  return soMap;
} /* end genSmallOnes */

void
findAreas ( pathfindingmap *map, int offset )
{
  tileData      *tile;
  tileArea      *area =  NULL;

  int  row, rowByte, bit, end;
  int  begin = -1;
  int  isDoGo;
  char curByte;

  /* simplify things */
  tile = map->tile;

  if ( tile[ offset ].flag != TDT_MIXED ) return;

  /* scan each row in tile */
  for ( row = 0; row < map->rowsPerTile; row++ ) {

    isDoGo = FALSE;
    /* scan each byte in row */
    for ( rowByte = 0; rowByte < map->bytesPerRow; rowByte++ ) {

      /* load current byte - negate the bits for simplicity: DoGo's are now 1*/
      curByte = ~(tile[ offset ].bits[ row * map->bytesPerRow + rowByte ]);

      /* and each bit in the byte */
      for ( bit = 0; bit < 8; bit++ ) {
	end = rowByte * 8 + bit - 1;
	if ( !isDoGo && ( curByte & ( 1 << bit ))) {

	  /* first DoGo in line */
	  isDoGo = TRUE;
	  begin = rowByte * 8 + bit;

	} else if ( isDoGo && !( curByte & ( 1 << bit ))) {

	  /* ran out Dogo's add line segment */
	  addSegment ( &area, TILE_DIM, TILE_DIM, row, begin, end, TRUE );
	  isDoGo = FALSE;
	}
      } /* end bit */
    } /* end rowByte */
    /* ran out of bits - add segment, if needed */
    if ( isDoGo ) addSegment ( &area, TILE_DIM, TILE_DIM, row, begin, TILE_DIM - 1, TRUE );

  } /* end row */

  /* areas should exist - there were some DoGo's earlier */
  if ( area ) {
    /* sort the areas by size */
    area = sortAreas ( area, TRUE);

    /* set smallOnes */
    addSmallOnes ( map, area, offset );

    /* clean up */
    freeAreaList ( &area );
  }
} /* end findAreas */

void
addSegment (tileArea **area, int dimX, int dimY, int row, int begin, int end, int contigious )
{
  tileArea *curArea  = NULL;
  tileArea *merged   = NULL;
  lineList *curLines = NULL;

  if ( ! area )
    shutdown ( EF_DATA_MISSING, "Function addSegment pass NULL area list\n" );

  if ( *area ) {
    /* rewind the area list to the beginning */
    while ( (*area)->prev ) *area = (*area)->prev;
    curArea = *area;
  } else  curArea = appendNewArea ( area, dimX, dimY );

  if ( !contigious ) /* if not seperating into contigious areas, just add line */
    appendLine ( curArea, row, begin, end );
  else {
    do {
      if ( !curArea->lines ) {
	/* add line and save area as merged */
	merged = appendLine ( curArea, row, begin, end );
      } else {

	/* set current line */
	curLines = curArea->lines;
	/* and rewind to first line in list */
	while ( curLines->prev ) curLines = curLines->prev;

	/* search each line or one that connects */
	do {
	  if ( linesConnect ( curLines, row, begin, end )) {
	    if ( merged ) {
	      /* line already merged into an area - use merged area */
	      merged = mergeAreas ( &merged, &curArea );
	    } else {
	      /* attach line to this area and save area as merged */
	      merged = appendLine ( curArea, row, begin, end );
	    }
	    break;
	  }

	  /* nothing yet? keep looking */
	  curLines = curLines->next;

	} while ( curLines );
      }

      /* check every area */
      if ( curArea->next ) curArea = curArea->next;
	  else break;
    } while ( curArea );

    /* if no merged area, still need to add line */
    if ( !merged ) {
      curArea = appendNewArea ( area, dimX, dimY );
      appendLine ( curArea, row, begin, end );
    }
  }
} /* end addSegment */

tileArea *
mergeAreas ( tileArea **area, tileArea **merge )
{
  tileArea *src;
  tileArea *dst;
  lineList *lines;

  if ( !(*area) || !(*merge) )
    shutdown ( EF_DATA_MISSING, "Function mergeAreas pass NULL area list\n" );

  dst = *area;
  while ( dst->prev ) dst = dst->prev;
  while ( dst && ( dst != *area ) && ( dst != *merge )) dst = dst->next;

  src = ( dst == *area ) ? *merge : *area ;

  /* adjust area parameters */
  if ( src->top    < dst->top    ) dst->top    = src->top;
  if ( src->left   < dst->left   ) dst->left   = src->left;
  if ( src->bottom > dst->bottom ) dst->bottom = src->bottom;
  if ( src->right  > dst->right  ) dst->right  = src->right;

  /* unhook merge area from list */
  if ( src->prev ) src->prev->next = src->next;
  if ( src->next ) src->next->prev = src->prev;

  lines = src->lines;
  while ( lines->prev ) lines = lines->prev;

  while ( lines ) {
    src->size += insertLineItem ( &(dst->lines), lines->row, lines->begin, lines->end );
    lines = lines->next;
  }

  /* Well now, that was fun... free merge tile area */
  freeLineList ( &(src->lines) );
  free ( src );

  return dst;
} /* end mergeArea */

tileArea *
sortAreas ( tileArea *area, int bySize )
{
  int max, nextSize;
  int hasEdge = TRUE;;

  tileArea *curArea;
  tileArea *nextArea;
  tileArea *biggerArea;
  tileArea temp;

  if ( ! area ) return area;

  /* rewind area list */
  while ( area->prev ) area = area->prev;

  /* set current area */
  curArea = area;

  /* place areas with the most surface area first in list */
  while ( curArea ) {

    if ( bySize ) {
      hasEdge = (( curArea->top == 0 ) || ( curArea->left == 0 ) ||
		 ( curArea->bottom == curArea->dimX - 1 ) ||
		 ( curArea->right  == curArea->dimY - 1 ));
      /* biggest size so far */
      max      = curArea->size + hasEdge * ( curArea->dimX * curArea->dimY );
    } else
      max = ( curArea->bottom - curArea->top ) * ( curArea->right - curArea->left );

    /* temp holds the largest so far*/
    biggerArea = curArea;

    /* area to compare */
    nextArea = curArea->next;

    /* HAS_ISLANDS: EA Games keeps areas that connect to nothing - islands
     * Bot's can neither get there or leave. seems a waist of time to keep
     * them, but... until I know otherwise these point stay.
     */
    while ( nextArea ) {
      if ( bySize ) {
	hasEdge = (( nextArea->top == 0 ) || ( nextArea->left == 0 ) ||
		   ( nextArea->bottom == nextArea->dimX - 1 ) ||
		   ( nextArea->right  == nextArea->dimY - 1 ));
	nextSize = nextArea->size + hasEdge * ( nextArea->dimX * nextArea->dimY );
      } else
	nextSize = ( nextArea->bottom - nextArea->top ) * ( nextArea->right - nextArea->left );

	/* compare next area to the current one */
      if (( nextSize > max ) && ( HAS_ISLANDS || hasEdge )) {
	/* this one is bigger - keep track of it and it's size */
	biggerArea = nextArea;
	max  = nextSize;
      }
      /* check each area in list */
      nextArea = nextArea->next;
    }

    if ( biggerArea != curArea ) {
      /* switch data in curArea with the large area */
      memcpy ( &temp,   curArea,    sizeof (tileArea ) - sizeof ( tileArea * ) * 2 );
      memcpy ( curArea, biggerArea, sizeof (tileArea ) - sizeof ( tileArea * ) * 2 );
      memcpy ( biggerArea,  &temp,  sizeof (tileArea ) - sizeof ( tileArea * ) * 2 );
    }
    /* search for the next largest */
    curArea = curArea->next;
  }
  /* rewind area list and return it */
  while ( area->prev ) area = area->prev;
  return area;
} /* end sortAreas */

void
addSmallOnes ( pathfindingmap *map, tileArea *area, int offset )
{
  tileArea      *curArea;
  lineList      *lines;
  tileImageData *img;

  int level;
  int bit, bitBegin, bitEnd;
  int byteOff, bitOff;
  int col, row, colWt, rowWt;
  int i;
  int found;
  int maxOff;
  int bytesPerRow;

  /* set useful vars */
  curArea     = area;
  bytesPerRow = map->bytesPerRow;
  img         = &(map->img[offset]);

  /* check each level */
  for ( level = 0; level < 4; level++ ) {

    /* we must have lines to work with - should never be NULL */
    if (( lines = curArea->lines ) == NULL )
      shutdown ( EF_DATA_MISSING,
		 "Function addSmallOnes found NULL line list\n" );

    /* now attempt to find the smallones... */

    /* haven't found anything yet */
    found = FALSE;
    while ( ! found ) {

      if ( !img->pt[level] &&
	   !( img->pt[level] = (unsigned char *) malloc ( TILE_BYTES )))
	shutdown ( EF_MALLOC,
		   "Error allocating tile image buffer in function addSmallOnes\n" );

      /* set tile image to all DoGos */
      memset ( img->pt[level], 0xff, TILE_BYTES );

      /* 'paint' the image onto image buffer */
      /* rewind list  */
      while ( lines->prev ) lines = lines->prev;

      colWt = rowWt = 0;
      while ( lines ) {
	/* find buffer offsets in bits */
	bitBegin = lines->row * TILE_DIM + lines->begin;
	bitEnd   = lines->row * TILE_DIM + lines->end;

	/* keep track of area densities */
	rowWt   += ( lines->end - lines->begin + 1 ) * ( lines->row + 1 );
	for ( bit = bitBegin; bit <= bitEnd; bit++ ) colWt += ( bit%TILE_DIM + 1 );

	setBits ( &(img->pt[ level ][ lines->row * ROW_BYTES ]),
		  lines->begin, lines->end, ROW_BYTES, FALSE );

	/* draw each line in list - get next line */
	if ( lines->next ) lines = lines->next;
	else break;
      } /* end lines */

      /* divide each by area size*/
      col = CLAMP ( colWt / (curArea->size)-1, 0, TILE_DIM - 1 );
      row = CLAMP ( rowWt / (curArea->size)-1, 0, TILE_DIM - 1 );

      /* convert to bitwise offsets */
      byteOff = row * bytesPerRow + col/8;
      bitOff  = col%8;

      if (!( img->pt[level][byteOff] & ( 1 << bitOff ))) {
	/* found it right off - just luck ... */
	if ( curArea->size > 16 ) centerPoint ( img->pt[level], &col, &row );
	found = TRUE;
      } else {
	/* look for one */
	maxOff = MAX ( MAX ( row, TILE_DIM - row - 1 ), MAX ( col, TILE_DIM - col - 1 ));

	for ( i = 1; i < maxOff; i++ ) {
	  /* look right */
	  if (( col + i ) < TILE_DIM ) {
	    byteOff = row * bytesPerRow + (col + i)/8;
	    bitOff  = (col + i)%8;
	    if ( !(img->pt[level][byteOff] & ( 1 << bitOff ))) {
	      col += i;
	      if ( curArea->size > 16 ) centerPoint ( img->pt[level], &col, &row );
	      found = TRUE;
	      break;
	    }
	  }
	  /* look down */
	  if (( row + i ) < TILE_DIM ) {
	    byteOff = ( row + i ) * bytesPerRow + col/8;
	    bitOff  = col%8;
	    if ( !(img->pt[level][byteOff] & ( 1 << bitOff ))) {
	      row += i;
	      if ( curArea->size > 16 ) centerPoint ( img->pt[level], &col, &row );
	      found = TRUE;
	      break;
	    }
	  }
	  /* look left */
	  if (( col - i ) >= 0 ) {
	    byteOff = row * bytesPerRow + (col - i)/8;
	    bitOff  = (col - i)%8;
	    if ( !(img->pt[level][byteOff] & ( 1 << bitOff ))) {
	      col -= i;
	      if ( curArea->size > 16 ) centerPoint ( img->pt[level], &col, &row );
	      found = TRUE;
	      break;
	    }
	  }
	  /* look down */
	  if (( row + i ) >= 0 ) {
	    byteOff = ( row - i ) * bytesPerRow + col/8;
	    bitOff  = col%8;
	    if ( !(img->pt[level][byteOff] & ( 1 << bitOff ))) {
	      row -= i;
	      if ( curArea->size > 16 ) centerPoint ( img->pt[level], &col, &row );
	      found = TRUE;
	      break;
	    }
	  }
	}
      }
      /* all done looking. if found, mark it */
      if ( found ) setPoint ( map, offset, level, col, row );

      /* get next area */
      curArea = curArea->next;

      /* if we run out of areas, quit */
      if ( !curArea ) break;

    } /* end while */

    /* if we run out of areas, really quit */
    if ( !curArea ) break;
  } /* end level loop */
} /* end addSmallOnes */

void
setPoint ( pathfindingmap *map, int offset, int level, int col, int row )
{
  smallOnesData *so;
  int rowByte;
  int byteOff;
  int prevTile = 0;
  int i;
  int hasEdge;

  /* smallOne structure must exist */
  if ( map == NULL )
    shutdown ( EF_INFO_MISSING,
	       "function setPoint was passed a null map list\n" );

  /* for simplicity only */
  so = map->so;

  /* set waypoint for this level and mark it as active */
  so[offset].pt[level][0] = col;
  so[offset].pt[level][1] = row;
  so[offset].active |= ( 1 << (4+level));

  /* if map->img is not set - input comes from text file - let it link the points */
  if ( ! map->img ) return;

  /* check for tiles above that may connect to this one */
  if ( offset > map->tilesPerRow - 1 ) {

    /* set prev to same col, prev row */
    prevTile = offset - map->tilesPerRow;

    /* does previos tile have any active smallOnes? */
    if ( so[prevTile].active ) {

      /* yes, check each smallOnes point */
      for ( i = 0; i < 4; i++ ) if ( so[prevTile].active & ( ACT_OFF << i )) {

	hasEdge = FALSE;
	for ( rowByte = 0; ( rowByte < map->bytesPerRow ) && !hasEdge; rowByte++ ) {
	  /* check each byte ... */
	    /*  .. and each bit
	     * since each DoGo is 0, negate the bytes first then & (and) them
	     * both together
	     */
	    if ( ~( map->img[ prevTile ].pt[i][ map->bytesPerRow * 63 + rowByte ] ) &
		 ~( map->img[ offset   ].pt[level][ rowByte ] ) & 0xff ) {
	      /* these areas connect */
	      hasEdge = TRUE;
	      break;
	    }
	} /* end rowByte loop */
	if ( hasEdge ) {
	  /* connect tile above to this one */
	  so[prevTile].hasLower |= (1 << (level+4*i));
	}
      } /* end i loop */
    }
  }
  /* check if the tile before this one connects */
  if ( ( offset % map->tilesPerCol ) > 0 ) {

    /* set previous tile */
    prevTile = offset - 1;

    /* does previos tile have any active smallOnes? */
    if ( so[prevTile].active ) {

      /* yes, check each smallOnes point */
      for ( i = 0; i < 4; i++ ) if ( so[prevTile].active & ( ACT_OFF << i )) {

	/* assume the worse */
	hasEdge = FALSE;

	/* check shared edge */
	for ( row = 0; ( row < map->rowsPerTile ) && !hasEdge; row++ ) {

	  /* compare last bit in previous area with first bit in current area
	   * for each row
	   */
	  byteOff = (row+1) * map->bytesPerRow - 1;
	  if ( !( map->img[ prevTile ].pt[i][ byteOff ] & ( 1 << 7)) &&
	       !( map->img[ offset   ].pt[level][ row * map->bytesPerRow ] & 1 )) {
	    /* these areas connect */
	    hasEdge = TRUE;
	    break;
	  }

	} /* end row loop */
	if ( hasEdge ) {
	  /* tile share edge - mark previous tile */
	  so[prevTile].hasRight |= (1 << (level+4*i));
	}
      } /* end i loop */
    } /* end active */
  } /* end ( offset % map->tilesPerCol ) */
} /* end setPoint */

int
linesConnect ( lineList *line, int row, int begin, int end )
{
  if ( !line ) return FALSE;
  if ( ABS ( line->row - row ) > 1 ) return FALSE;
  return BORDER ( begin, end, line->begin, line->end );
} /* end linesConnect */


tileArea *
appendNewArea ( tileArea **area, int dimX, int dimY )
{
  tileArea *newarea;

  /* create a new tileArea structure */
  newarea = newArea( dimX, dimY);

  if ( !area )
    shutdown ( EF_DATA_MISSING, "Function appendnewArae pass NULL area list\n" );

  if ( *area ) {
    /* add this to existing area list */
    while ( (*area)->next ) *area = (*area)->next;
    (*area)->next = newarea;
    newarea->prev = *area;
  } else {
    /* no existing list, make this it */
    *area = newarea;
  }

  /* return the new area */
  return newarea;
} /* end appendNewArea */

tileArea *
appendLine ( tileArea *area, int row, int begin, int end )
{
  if ( ! area )
    shutdown ( EF_DATA_MISSING, "Function appendLine passed NULL area list\n" );

  area->size += insertLineItem ( &(area->lines), row, begin, end );

  /* adjust area parameters and return it */
  if ( row   < area->top    ) area->top    = row;
  if ( row   > area->bottom ) area->bottom = row;
  if ( begin < area->left   ) area->left   = begin;
  if ( end   > area->right  ) area->right  = end;

  return area;

} /* end appendLine */

void
centerPoint ( unsigned char *bits, int *col, int *row )
{
  int c, r;
  int cMax, cMin, rMax, rMin;
  int cMaxOff, cMinOff, rMaxOff, rMinOff;
  int goUp, goDown, goLeft, goRight;
  int byteOff, bitOff;

  c = *col;
  r = *row;

  for ( cMaxOff = 0, cMax = c; cMax < TILE_DIM; cMax++, cMaxOff++ ) {
    byteOff = r * ROW_BYTES + cMax/8;
    bitOff  = cMax%8;
    if ( !(bits[byteOff] & ( 1 << bitOff )) && ( cMax < TILE_DIM - 1 )) {
      byteOff = r * ROW_BYTES + (cMax + 1)/8;
      bitOff  = (cMax + 1)%8;
      if ((bits[byteOff] & ( 1 << bitOff ))) break;
    } else break;
  }
  for ( cMinOff = 0, cMin = c; cMin >= 0; cMin--, cMinOff++ ) {
    byteOff = r * ROW_BYTES + cMin/8;
    bitOff  = cMin%8;
    if ( !(bits[byteOff] & ( 1 << bitOff )) && ( cMin > 0 )) {
      byteOff = r * ROW_BYTES + (cMin - 1)/8;
      bitOff  = (cMin - 1)%8;
      if ((bits[byteOff] & ( 1 << bitOff ))) break;
    } else break;
  }
  for ( rMaxOff = 0, rMax = r; rMax < TILE_DIM; rMax++, rMaxOff++ ) {
    byteOff = rMax * ROW_BYTES + c/8;
    bitOff  = c%8;
    if (( !(bits[byteOff] & ( 1 << bitOff ))) && ( rMax < TILE_DIM - 1 )) {
      byteOff = ( rMax + 1 ) * ROW_BYTES + c/8;
      bitOff  = c%8;
      if ((bits[byteOff] & ( 1 << bitOff ))) break;
    } else break;
  }
  for ( rMinOff = 0, rMin = r; rMin >= 0; rMin--, rMinOff++ ) {
    byteOff = rMin * ROW_BYTES + c/8;
    bitOff  = c%8;
    if (( !(bits[byteOff] & ( 1 << bitOff ))) && ( rMin > 0 )) {
      byteOff = ( rMin - 1 ) * ROW_BYTES + c/8;
      bitOff  = c%8;
      if ((bits[byteOff] & ( 1 << bitOff ))) break;
    } else break;
  }
  goLeft  = (( cMaxOff < 2 ) && ( cMin >= 2 ));
  goRight = (( cMinOff < 2 ) && ( cMax >= 2 ));

  goUp    = (( rMaxOff < 2 ) && ( rMin >= 2 ));
  goDown  = (( rMinOff < 2 ) && ( rMax >= 2 ));

  if (( goLeft || goRight ) && ( ABS(( rMax + rMin ) / 2 - r ) > 2 )) {
    r = ( rMax + rMin ) / 2;
  } else
    if (( goUp || goDown ) && ( ABS(( cMax + cMin ) / 2 - c ) > 2 )) {
    c = ( cMax + cMin ) / 2;
  } else {
    *row = r;
    *col = c;
    return;
  }

  /* go for broke - try again and again and ... */
  centerPoint ( bits, &c, &r );
  *row = r;
  *col = c;
} /* end centerPoint */

void
loadSmallOnes ( pathfindingmap *map )
{

  /* smallOnes header is made up of two long integers with tiles per col, row */
  if ( !( fread ( &(map->tilesPerRow), 4, 1, map->fp )))
    shutdown ( EF_FILE_READ,
	       "Error reading %s smallOnes header.\n",
	       baseName[map->io.vehicle] );

  /* col and row are the same */
  map->tilesPerCol = map->tilesPerRow;
  map->tiles       = map->tilesPerRow * map->tilesPerCol;

  /* fixed tile dims for smallOnes */
  map->rowsPerTile  = TILE_DIM;
  map->bytesPerRow  = TILE_DIM / 8;
  map->bytesPerTile = map->rowsPerTile * map->bytesPerRow;
  map->res          = map->tilesPerRow * TILE_DIM;

  /* get input file size */
  if ( fseek ( map->fp, 0, SEEK_END ) != 0 )
    shutdown ( EF_FILE_SEEK,
	       "Error seeking end of smallOnes %s file.\n", baseName[map->io.vehicle] );

  if (( ftell ( map->fp ) - 8 ) != (int) ( map->tiles * sizeof ( smallOnesData )) )
    shutdown ( EF_BAD_FILE,
	       "Wrong sized smallOnes %s file.\n", baseName[map->io.vehicle] );

  if ( fseek ( map->fp, 8, SEEK_SET ) != 0 )
    shutdown ( EF_FILE_SEEK,
	       "Error seeking data in smallOnes %s file.\n", baseName[map->io.vehicle] );

  if ( !( map->so = ( smallOnesData * ) malloc ( sizeof ( smallOnesData ) * map->tiles )))
    shutdown ( EF_MALLOC,
	       "Error creating %s tile data array.\n", baseName[map->io.vehicle] );

  if ( !fread ( map->so, sizeof ( smallOnesData ) * map->tiles, 1, map->fp ))
    shutdown ( EF_FILE_READ,
	       "Error reading %s smallOnes data.\n", baseName[map->io.vehicle] );
} /* end loadSmallOnes */

int
insertLineItem ( lineList **list, int row, int begin, int end )
{
  lineList *newLine;
  int retCount;
  if ( !( newLine = (lineList *) calloc ( sizeof (lineList), 1 )))
    shutdown ( EF_MALLOC, "Error allocating line item in function calcNoGos\n" );
  newLine->row   = row;
  newLine->begin = begin;
  newLine->end   = end;

  retCount = end - begin + 1;
  if ( *list ) {
    /* scan froward thru the lines first */
    while (( cmpLines ( *list, newLine )) < 0 ) {
      if ( (*list)->next ) *list = (*list)->next;
      else {
	/* ran out of lines - append and return list */
	postInsertLine ( list, &newLine );
	return retCount;
      }
    }
    /* scan backwards now */
    while ( (*list)->prev &&
	    (( cmpLines ( (*list)->prev, newLine ) >= 0 ))) {
      *list = (*list)->prev;
    }
    /* if not connected to current line - append line and return list */
    if (( cmpLines ( *list, newLine ) > 0 ) &&
	(( !(*list)->prev ) || ( cmpLines ((*list)->prev, newLine ) < 0 ))) {
      preInsertLine ( list, &newLine );
      return retCount;
    }
    *list = postInsertLine ( list, &newLine );
    while ( (*list)->next && ( !cmpLines ( *list, (*list)->next ))) {
      retCount = 0;
      /* adjust endpoints */
      if ( (*list)->begin > (*list)->next->begin ) {
	retCount += (*list)->begin - (*list)->next->begin;
	(*list)->begin = (*list)->next->begin;
      }
      if ( (*list)->end < (*list)->next->end ) {
	retCount += (*list)->next->end - (*list)->end;
	(*list)->end = (*list)->next->end;
      }
      /* unhook & free line */
      newLine = (*list)->next;
      if ( newLine->next ) newLine->next->prev = *list;
      (*list)->next = newLine->next;
      free ( newLine );

      if ( (*list)->next ) *list = (*list)->next;
      else break;
    }
  } else {
    *list = newLine;
  }
  return retCount;
}

int
cmpLines ( lineList *list1, lineList *list2 )
{
  if (( list1->row > list2->row ) ||
      (( list1->row == list2->row ) && ( list1->begin > list2->end + 1 )))
    return 1;
  if (( list1->row < list2->row ) ||
      (( list1->row == list2->row ) && ( list1->end < list2->begin - 1 )))
    return -1;
  return 0;
}

lineList *
preInsertLine ( lineList **list1, lineList **list2 )
{
  if ( (*list1)->prev ) {
    (*list1)->prev->next = *list2;
    (*list2)->prev = (*list1)->prev;
  }
  (*list1)->prev = *list2;
  (*list2)->next = *list1;
  return *list2;
}

lineList *
postInsertLine ( lineList **list1, lineList **list2 )
{
  if ( (*list1)->next ) {
    (*list1)->next->prev = *list2;
    (*list2)->next = (*list1)->next;
  }
  (*list1)->next = *list2;
  (*list2)->prev = *list1;
  return *list1;
}

void
setBits ( unsigned char *dst, int begin, int end, int bufSize, int setHi )
{
  unsigned char fullByte;
  int bytes, bitBegin, bitEnd, mask, offset;
  int byteBegin, byteEnd;
  int clipBegin, clipEnd;

  fullByte = ( setHi ? 0xff : 0 );

  /* fill whole buffer? */
  if (( end - begin + 1 ) == ( bufSize * 8 )) {
    /* fill the row */
    memset ( dst, fullByte, bufSize );
    return;
  }

  byteBegin = begin / 8;
  byteEnd   = end   / 8;
  bitBegin  = begin % 8;
  bitEnd    = end   % 8;
  offset    = byteBegin;

  if ( byteBegin == byteEnd ) {
    /* segment begins and ends in the same byte */
    mask   = ( 1 << ( bitEnd + 1 )) - 1;
    mask = (( mask >>  bitBegin ) <<  bitBegin );

    dst[ offset ] = setHi ? (dst[ offset ] | mask) : ( dst[ offset ] & ~mask );
    return;
  }

  clipBegin = ( bitBegin > 0 );
  clipEnd   = ( bitEnd < 7 );
  bytes     = byteEnd - byteBegin + 1;

  if ( clipBegin ) {
    mask = ~(( 1 << bitBegin ) - 1);
    dst[offset] = setHi ? ( dst[offset] | mask ) : ( dst[offset] & ~mask );
    offset++; bytes--;
  }
  if ( clipEnd ) {
    mask = ( 1 << ( bitEnd + 1 )) - 1;
    dst[byteEnd] = setHi ? ( dst[byteEnd] | mask ) : ( dst[byteEnd] & ~mask );
    bytes--;
  }
  if ( bytes ) memset ( &(dst[offset]), fullByte, bytes );
}
