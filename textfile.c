/* textfile.c - text file support functions
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
#include "textfile.h"
#include "pathfindingmap.h"
#include "smallones.h"

/************************************  global variables      *********************/

extern char *baseName[];

/************************************  functions             *********************/

void
loadSmallOnesText ( pathfindingmap *map )
{
  char *bp;

  int col1, row1, level1, x1, y1;
  int col2, row2, level2, x2, y2;
  char buffer[BUF_SIZE] = {0};
  char flag1 = 0;
  char flag2 = 0;

  int offset;
  int headerLoaded;

  /* something for later - appending info */
  headerLoaded = FALSE;

  while ( fgets ( buffer, BUF_SIZE, map->fp )) {

    /* remove any comments, end of line or carrage returns */
    if (( bp = strchr ( buffer, '\012' )) != NULL ) *bp = 0;
    if ( bp == buffer ) continue;
    if (( bp = strchr ( buffer, '\015' )) != NULL ) *bp = 0;
    if ( bp == buffer ) continue;
    if (( bp = strchr ( buffer, '#' )) != NULL ) *bp = 0;
    if ( bp == buffer ) continue;

    flag1 = flag2 = 0;

    /* check to is if this is the only input */
    if ( ! headerLoaded ) {

      /* scan for map variables */

      /* get image resolution */
      if ( strncmp ( buffer, READ_IMG_RES, 10 ) == 0 ) {
	if ( !sscanf ( buffer, READ_IMG_RES, &(map->res) ))
	  shutdown ( EF_BAD_DATA, 
		     "Error reading file header in %s smallOnes text file.\n", 
		     baseName[map->io.vehicle] );
	
	/* number of tiles per row */
      } else if ( strncmp ( buffer, READ_TILE_RES, 10 ) == 0 ) {
	if ( !sscanf ( buffer, READ_TILE_RES, &(map->tilesPerRow) ))
	  shutdown ( EF_BAD_DATA, 
		     "Error reading file header in %s smallOnes text file.\ns", 
		     baseName[map->io.vehicle] );
	
	/* total tiles per image */
      } else  if ( strncmp ( buffer, READ_TILE_COUNT, 12 ) == 0 ) {
	if ( !sscanf ( buffer, READ_TILE_COUNT, &(map->tiles)))
	  shutdown ( EF_BAD_DATA, 
		     "Error reading file header in %s smallOnes text file.\n", 
		     baseName[map->io.vehicle] );
	
      } else if (( strncmp ( buffer, READ_POINT_SET, 17 ) == 0 ) ||
		 ( strncmp ( buffer, READ_TILE_LINK, 5  ) == 0 )) {
	/* getting tile lines before header - haven't created smallOnes buffer yet... */
	shutdown ( EF_INFO_MISSING, "Text file header missing, aborting...\n" );
      }

      /* got all the header yet? */
      if ( map->res && map->tilesPerRow && map->tiles ) {
	  
	/* make sure the data isn't mangled */
	if (( map->res   != map->tilesPerRow * TILE_DIM ) ||
	    ( map->tiles != map->tilesPerRow * map->tilesPerRow ))
	  shutdown ( EF_BAD_DATA, 
		     "Malformed header data in %s smallOnes text file.\n", 
		     baseName[map->io.vehicle] );
	  
	/* mark header as read */
	headerLoaded      = TRUE;
	  
	/* set a few pathfinding data variables */
	map->io.level        = 0;
	map->tilesPerCol  = map->tilesPerRow;
	map->rowsPerTile  = TILE_DIM;
	map->bytesPerRow  = TILE_DIM / 8;
	map->bytesPerTile = map->rowsPerTile * map->bytesPerRow;
	  
	/* create smallOnes buffer (less header) 16 byte x total tiles */
	if ( !( map->so = calloc ( sizeof ( smallOnesData ) * map->tiles, 1)))
	  shutdown ( EF_MALLOC, 
		     "Error creating %s smallOnes buffer.\n", 
		     baseName[map->io.vehicle] );
      } 

    } else if ( strncmp ( buffer, READ_POINT_SET, 9 ) == 0 ) {
      if ( sscanf ( buffer,
		    READ_POINT_SET,
		    &col1, &row1, &level1, &x1, &y1, &flag1 ) < 5 )
	shutdown ( EF_INFO_MISSING, 
		   "Error scanning text in %s smallOnesfile.\n", 
		   baseName[map->io.vehicle] );

      /* set offset to first point */
      offset = row1 * map->tilesPerRow + col1;

      /* add point to smallOnes buffer - not linked... */
      setPoint ( map, offset, level1, x1, y1 );


    } else if ( strncmp ( buffer, READ_TILE_LINK, 5 ) == 0 ) {

      /* we found something to work with */
      if ( sscanf ( buffer,
		    READ_TILE_LINK_2,
		    &col1, &row1, &level1, &x1, &y1,
		    &col2, &row2, &level2, &x2, &y2, &flag1) < 10 )
	if ( sscanf ( buffer,
		      READ_TILE_LINK,
		      &col1, &row1, &level1, &x1, &y1, &flag1,
		      &col2, &row2, &level2, &x2, &y2, &flag2 ) < 11 )
	  shutdown ( EF_BAD_DATA, 
		     "Error scanning point link in %s smallOnesfile.\n", 
		     baseName[map->io.vehicle] );
      
      /* set offset */
      offset = row1 * map->tilesPerRow + col1;

      if (( flag1 == 'P' ) || ( flag1 == 'p' )) {
	/* add first point to smallOnes buffer w/ bit 6 set*/
	setPoint ( map, offset, level1, x1+TILE_DIM, y1+TILE_DIM );
      } else {
	/* add first point to smallOnes buffer */
	setPoint ( map, offset, level1, x1, y1 );
      }

      /* link points */
      if ( col1 == col2 ) map->so[offset].hasLower |= (1 << (level1 * 4 + level2 ));
      if ( row1 == row2 ) map->so[offset].hasRight |= (1 << (level1 * 4 + level2 ));

      /* set offset to second point */
      offset = row2 * map->tilesPerRow + col2;

      if (( flag2 == 'P' ) || ( flag2 == 'p' )) {
	/* add first point to smallOnes buffer w/ bit 6 set*/
	setPoint ( map, offset, level2, x2+TILE_DIM, y2+TILE_DIM );
      } else {
	/* add first point to smallOnes buffer */
	setPoint ( map, offset, level2, x2, y2 );
      }

    } else if ( strncmp ( buffer, READ_TILE_MARK, 12 ) == 0 ) {
      /* yet another unknown - some files mark the tile->na1 field set to 0x10
       * this happens at most once in any file and it's outside the allowed
       * path area. So I want to be able to too.
       */
      if ( sscanf ( buffer,
		    READ_TILE_MARK,
		    &col1, &row1 ) < 2 )
	shutdown ( EF_INFO_MISSING, "Error scanning text line: %s", buffer );

      /* set tile to smallOne record in buffer and mark it */
      map->so[row1 * map->tilesPerRow + col1].na1 = ACT_OFF;
    }
  }
} /* end loadSmallOnesText */

void
writeSmallOnesText ( pathfindingmap *map )
{
  int i;
  int level1, level2;
  char *flag1, *flag2;
  smallOnesData *so1;
  smallOnesData *so2;

  /* open output file */
  if ( !( openFile( map, WRITE_MODE ))) 
    shutdown ( EF_FILE_OPEN, "Error openning %s smallOnes text file for writing\n", 
	       baseName[map->io.vehicle] );


  /* print 8 bit pathmap file info */
  fprintf ( map->fp, WRITE_IMG_RES, map->res, map->res, LINEFEED );
  fprintf ( map->fp, WRITE_TILE_RES, map->tilesPerRow, map->tilesPerCol, LINEFEED );
  fprintf ( map->fp, WRITE_TILE_COUNT, map->tiles, LINEFEED );

  /* scan fields for smallOnes */
  for ( i = 0; i < map->tiles; i++ ) {

    /* first pass - just show set points */
    so1 = &(map->so[i]);

    if ( so1->active ) {
      for ( level1 = 0; level1 < 4; level1++ ) {
	if ( so1->active & ( 1 << ( level1 + 4 ))) {
	  fprintf ( map->fp,
		    WRITE_POINT_SET,
		    i % map->tilesPerRow, i/map->tilesPerRow, level1,
		    so1->pt[level1][0], so1->pt[level1][1], LINEFEED );

	}
      }
    }
    if ( so1->na1 ) {
      /* i have no idea what this field is for yet */
      fprintf ( map->fp, WRITE_TILE_MARK,
		i % map->tilesPerRow, i / map->tilesPerRow, LINEFEED );
    }
  }
  /* scan fields for smallOnes */
  for ( i = 0; i < map->tiles; i++ ) {
    /* second pass - show which points are linked to which other points */
    so1 = &(map->so[i]);
    if ( so1->active ) {
      /* we got us a live one */
      /* check each level */
      for ( level1 = 0; level1 < 4; level1++ ) {
	if ( so1->active & ( 1 << ( level1 + 4 ))) {

	  /* this point is active - check agains smallone below and to the right for links */
	  for ( level2 = 0; level2 < 4; level2++ ) {
	    if ( so1->hasLower & ( 1 << ( level1 * 4 + level2 ))) {
	      /* point connected to one below */
	      so2 = &(map->so[ i + map->tilesPerRow ]);

	      flag1 = (so1->pt[level1][0] > TILE_DIM ) ? "P" : " ";
	      flag2 = (so2->pt[level1][0] > TILE_DIM ) ? "P" : " ";

	      fprintf ( map->fp,
			WRITE_TILE_LINK,
			i%map->tilesPerRow, 
			i/map->tilesPerRow, 
			level1,
			so1->pt[level1][0]%TILE_DIM, 
			so1->pt[level1][1]%TILE_DIM,
			flag1, 
			i%map->tilesPerRow, 
			i/map->tilesPerRow+1, 
			level2,
			so2->pt[level2][0]%TILE_DIM, 
			so2->pt[level2][1]%TILE_DIM,
			flag2, 
			LINEFEED );
	    }
	    if ( so1->hasRight & ( 1 << ( level1 * 4 + level2 ))) {
	      so2 = &(map->so[ i + 1 ]);

	      flag1 = (so1->pt[level1][0] > TILE_DIM ) ? "P" : " ";
	      flag2 = (so2->pt[level1][0] > TILE_DIM ) ? "P" : " ";

	      fprintf ( map->fp,
			WRITE_TILE_LINK,
			i%map->tilesPerRow, 
			i/map->tilesPerRow, 
			level1,
			so1->pt[level1][0]%TILE_DIM, 
			so1->pt[level1][1]%TILE_DIM,
			flag2, 
			i%map->tilesPerRow+1, 
			i/map->tilesPerRow, 
			level2,
			so2->pt[level2][0]%TILE_DIM, 
			so2->pt[level2][1]%TILE_DIM,
			flag2, 
			LINEFEED );
	    }
	  }
	}
      }
    }
  }
} /* end writeSmallOnesText */

