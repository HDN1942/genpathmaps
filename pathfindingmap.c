/* pathfindingImage.c - functions used for pathfind data manipulation
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
#include "image.h"
#include "textfile.h"

/************************************  global variables      ************************/

extern char *baseName[];
extern char *inputType[];

/************************************  functions             ************************/

pathfindingmap *
getMap ( pathfindingmap **maps, mapIOData *src, mapIOData *dst )
{
  int type;
  int level;

  pathfindingmap *map = NULL;

  if ( !maps )
    shutdown ( EF_INFO_MISSING, "Function pathfindingmaps passed bad map list\n" );

  /* search list first - might already be open */
  if ( *maps ) {
    /* if the destination image exists return it */
    if ( dst && ( map = findMap ( *maps, *dst ))) {
      type = findLn2 ( IMGTYPES(dst->type));
      debug ( DBG_NOTICE,
	      "Retrieving %s %s level %d map\n",
	      baseName[dst->vehicle], inputType[type], dst->level );
      if ( !dst ) return map;
    }
    /* if the source image exists, use it */
    map = findMap ( *maps, *src );
  }

  /* if map doesn't exist, create one */
  if ( !map ) {
    type = findLn2 ( IMGTYPES(src->type));
    debug ( DBG_NOTICE,
	    "New %s %s level %d map\n",
	    baseName[src->vehicle], inputType[type], src->level );

    /* allocate pathfindingmap structure - and keep track of them*/
    if (!( map = (pathfindingmap *) calloc ( sizeof ( pathfindingmap ), 1 )))
      shutdown ( EF_MALLOC, "Memory allocation error creating pathfindingmap\n" );

    /* these are known - came with the call */
    copyIO ( &(map->io) , *src );

    /* should we try to read a file */
    if ( src && src->path  && ( src->type & FTF_READ )) {
      type = findLn2 ( IMGTYPES(src->type));
      debug ( DBG_NOTICE,
	      "Loading %s %s level %01d file from: %s\n",
	      baseName[src->vehicle], inputType[type], src->level, src->path );
      /* try to load the map */
      if ( loadFile ( map )) {
	/* that worked, link it to the others */
	linkMaps ( maps, map );
	src->type    &= ~FTF_READ;
	map->io.type &= ~FTF_READ;
      } else {
	/* map didn't load - free map structure and return empty handed */
	free ( map );
	return NULL;
      }
    } else linkMaps ( maps, map );
  }

  if ( dst ) {
    /* this map needs conversion */
    switch ( IMGTYPES(dst->type) )
      {
      case FTF_NONE:
	break;
      case FTF_MAP:
	while ( map->io.level != dst->level ) {
	  debug ( DBG_NOTICE,
		  "Compressing %s map from level %01d to %01d\n",
		  baseName[dst->vehicle], map->io.level, dst->level );
	  map = compressMap ( getMap ( maps, dst, NULL ));
	}
	break;
      case FTF_INFO:
	if ( !( map->io.type & FTF_INFO )) {
	  map = findInfo ( map );
	  debug ( DBG_NOTICE,
		  "Created %s Info map\n",
		  baseName[dst->vehicle]);
	}
	break;
      case FTF_SO:
	if ( !( map->io.type & ( FTF_SO | FTF_TXT ))) {
	  map = genSmallOnes ( map );
	  debug ( DBG_NOTICE,
		  "Created %s SmallOnes map\n",
		  baseName[dst->vehicle]);
	}
	break;
      case FTF_TXT:
      case FTF_TXT | FTF_SO:
	if ( map->io.type & FTF_MAP ) {
	  map = genSmallOnes ( map );
	  debug ( DBG_NOTICE,
		  "Created %s SmallOnes map\n",
		  baseName[dst->vehicle]);
	}
	break;
      default:
	shutdown ( EF_BAD_DATA, "Too many output types passed to function getMap\n" );
      }

    level = map->io.level;
    copyIO ( &(map->io), *dst );
    map->io.level = level;
    if ( map && (dst->type & FTF_WRITE )) {
      type = findLn2 ( IMGTYPES(dst->type));
      debug ( DBG_NOTICE,
	      "Writing %s %s level %d file to %s\n\n",
	      baseName[dst->vehicle], inputType[type], dst->level, dst->path );
      /* set the destination path and output map */
      writeMap ( map );

      /* 8 bit maps are not created, just printed from existing maps */
      if ( dst->type & FTF_IMG && !( dst->type & FTF_MAP ))
	copyIO ( &(map->io), *src );
      map->io.type &= ~FTF_WRITE;
    }
  }

  return map;
} /* end getMap */

void
writeMap ( pathfindingmap *map )
{
  int buf;
  int i;
  int comp;
  mapFileHeader header;

  /* make sure there is a place to write too */
  if ( ! map || ! map->io.path ) return;

  /* open the file for writing */
  if ( !( openFile ( map, WRITE_MODE ))) return;

  /* output either an 8 bit or compressed map */
  if ( map->io.type & FTF_IMG ) writeImageFile ( map );
  else {
    switch ( IMGTYPES(map->io.type))
      {
      case FTF_MAP:
      case FTF_INFO:

	fillHeader ( map, &header );
	if ( !fwrite ( &header, sizeof ( mapFileHeader ), 1, map->fp ))
	  shutdown ( EF_FILE_WRITE, "Error writing header to pathinfing file\n" );

	if ( header.dataOffset == 2 ) {
	  buf = 0;
	  if ( !fwrite ( &buf, 4, 1, map->fp ))
	    shutdown ( EF_FILE_WRITE, "Error writing header to pathinfing file\n" );
	  buf = -1;
	  if ( !fwrite ( &buf, 4, 1, map->fp ))
	    shutdown ( EF_FILE_WRITE, "Error writing header to pathinfing file\n" );
	  comp = TRUE;
	} else comp = FALSE;

	for ( i = 0; i < map->tiles; i++ ) {
	  if ( !fwrite ( &(map->tile[i].flag), 4, 1, map->fp ))
	    shutdown ( EF_FILE_WRITE, "Error writing record delimiter to pathinfing file\n" );
	  if ( !comp || ( map->tile[i].flag == TDT_MIXED )) {
	    if ( !fwrite ( map->tile[i].bits, map->bytesPerTile, 1, map->fp ))
	      shutdown ( EF_FILE_WRITE, "Error writing tile record to pathinfing file\n" );

	  }
	}

	break;
      case FTF_SO:
	if ( !map || !map->so )
	  shutdown ( EF_DATA_MISSING, "Function writeSmallOnes pass bad parameters\n" );

	/* write output - 2 int for the header, then 16 * number of tiles for data */
	if ( ! fwrite ( &(map->tilesPerCol), 4, 1, map->fp ))
	  shutdown ( EF_FILE_WRITE,
		     "Error writing header to %s smallOnes text file for writing\n",
		     baseName[map->io.vehicle] );
	if ( ! fwrite ( &(map->tilesPerRow), 4, 1, map->fp ))
	  shutdown ( EF_FILE_WRITE,
		     "Error writing header to %s smallOnes text file for writing\n",
		     baseName[map->io.vehicle] );
	if ( !fwrite ( map->so, sizeof ( smallOnesData ) * map->tiles, 1, map->fp ))
	  shutdown ( EF_FILE_WRITE,
		     "Error writing record to %s smallOnes text file for writing\n",
		     baseName[map->io.vehicle] );
	break;

      case FTF_TXT:
      case FTF_TXT | FTF_SO:
	writeSmallOnesText ( map );
	break;
      }
  }

  /* done with the file - close it */
  fclose ( map->fp );
  map->fp = NULL;
} /* end writeMap */

int
loadFile ( pathfindingmap *map )
{
  if ( ( map->io.type & FTF_GRID ) && !IMGTYPES( map->io.type) ) {
    initGridMap8Bit ( map );
    return TRUE;
  }
  if ( !openFile ( map, READ_MODE )) return FALSE;

  if ( map->io.type & FTF_IMG ) {
    switch ( IMGTYPES(map->io.type))
      {
      case FTF_MAP:
	loadImage ( map );
	break;
      default:
	shutdown ( EF_NOT_SUPPORTED, 
		   "This file is not supported for input\n" );
      }
  } else {
    switch ( IMGTYPES(map->io.type))
      {
      case FTF_MAP:
      case FTF_INFO:
	loadMapFile (map);
	break;
      case FTF_SO:
	loadSmallOnes ( map );
	break;
      case FTF_TXT:
	loadSmallOnesText ( map );
	break;
      }
  }

  /* done with the file - close it */
  fclose ( map->fp );
  map->fp = NULL;

  return TRUE;
} /* end loadFile */

void
loadMapFile ( pathfindingmap *map )
{
  char *filename;
  int i;
  mapFileHeader header;

  filename = fullPath ( map );

  /* all of these share the header and record types */
  /* load header information or die */

  /* read pathfinding file header */
  if ( !fread ( &header, sizeof ( mapFileHeader ), 1, map->fp ))
      shutdown ( EF_FILE_READ,
		 "Error reading map header from: %s\n", filename );

  /* check the file header - make sure that's what it really is */
  if ((  header.ln2TilesPerRow != header.ln2TilesPerCol ) ||
      (  header.ln2TilesPerRow > 8  ) ||
      (  header.ln2TileRes  < 6  ) ||
      (  header.ln2TileRes  > 12 ) ||
      (( header.isInfo     != 1 ) && ( header.isInfo     != 0 )) ||
      (( header.dataOffset != 0 ) && ( header.dataOffset != 2 ))) {
      shutdown ( EF_BAD_DATA,
		 "Is this really a path map file?: %s\n...Doesn't look like one.\n\n",
		 filename );
  }

  /* set global variables */
  map->tilesPerRow  = 1 << header.ln2TilesPerRow;
  map->tilesPerCol  = 1 << header.ln2TilesPerCol;
  map->tiles        = map->tilesPerRow * map->tilesPerCol;

  map->rowsPerTile  = 1 << ( header.ln2TileRes - header.compLevel );
  map->bytesPerRow  = map->rowsPerTile >> ( 3 - header.isInfo );
  map->bytesPerTile = map->rowsPerTile * map->bytesPerRow;
  map->res          = 1 << ( header.ln2TilesPerRow + header.ln2TileRes -
			     header.compLevel * !header.isInfo );


  /* move file pointer to data area, if needed */
  if ( header.dataOffset )
    if ( fseek ( map->fp, header.dataOffset * 4, SEEK_CUR ) == -1 )
      shutdown ( EF_FILE_READ,
		 "Error seeking data in file: %s\n", filename );

  /* create array of tile data records - all set to zero */
  if ( !( map->tile = (tileData *) calloc ( sizeof ( tileData ) * map->tiles, 1 )))
    shutdown ( EF_MALLOC,
	       "Error creating tile data buffer for: %s\n", filename );

  /* load file information into the array */
  for ( i = 0; i < map->tiles; i++ ) {
    if ( !fread ( &( map->tile[i].flag ), 4, 1, map->fp ))
      shutdown ( EF_FILE_READ,
		 "Error reading tile data flag in file: %s\n", filename );
    switch ( map->tile[i].flag )
      {
      case TDT_DOGO:
      case TDT_NOGO:
	break;
      case TDT_MIXED:
	/* data to follow - create record and fill it from file */
	if ( !( map->tile[i].bits = ( unsigned char *) calloc ( map->bytesPerTile, 1 )))
	  shutdown ( EF_MALLOC,
		     "Error creating tile data array for: %s\n", filename );
	if ( !fread ( map->tile[i].bits, map->bytesPerTile, 1, map->fp ))
	  shutdown ( EF_FILE_READ,
		     "Error reading tile data in file: %s\n", filename );
	break;
      default:
	/* not good - unknown flag type */
	shutdown ( EF_BAD_DATA, "Bad tile data flag in file: %s\n", filename );
      }
  }
  free ( filename );
} /* end loadMapFile */

pathfindingmap *
findInfo ( pathfindingmap *map )
{
  pathfindingmap *soMap;
  pathfindingmap *infoMap;
  pathfindingmap *srcMap;
  tileImageData  *img   = NULL;
  tileData       *actTile = NULL;
  tileData       *srcTile = NULL;

  mapIOData       io    = {0};

  int tile, row, bits, level;
  int srcByteOff, srcBitOff;
  int byteOff, bitOff;
  int pixSize;
  int i, j, k, l;
  int isDoGo;
  int sum;

  copyIO ( &io, map->io );

  /* we need these maps :       */
  /* smallOnes for this vehicle */
  io.type = IMGFLAG(io.type) | FTF_SO;
  io.level = 0;
  srcMap = map;
  soMap = getMap ( &map, &(srcMap->io), &io );
  /* */
  /* create empty info map */
  io.type = IMGFLAG(io.type ) | FTF_INFO;
  io.level = ISSEA(io.vehicle) ? 3 : 1;

  infoMap = getMap ( &map, &io, NULL );
  infoMap->tilesPerRow = infoMap->tilesPerCol = srcMap->tilesPerRow;
  infoMap->tiles = srcMap->tiles;
  infoMap->rowsPerTile = ( srcMap->rowsPerTile >> ( io.level ));
  infoMap->bytesPerRow = ( srcMap->bytesPerRow >> ( io.level - 1 ));
  infoMap->bytesPerTile = infoMap->rowsPerTile * infoMap->bytesPerRow;
  infoMap->res = srcMap->res;
  pixSize = 1 << infoMap->io.level;

  if ( !( infoMap->tile = (tileData *) calloc ( sizeof ( tileData ) * infoMap->tiles, 1 )))
    shutdown ( EF_MALLOC,
	       "Error allocating info tile buffer array in function findInfo\n" );
  for ( tile = 0; tile < infoMap->tiles; tile++ ) {
    actTile = &(infoMap->tile[tile]);
    srcTile = &(soMap->tile[tile]);
    if ( srcTile->flag != TDT_MIXED ) {
      actTile->flag = srcTile->flag;
      continue;
    }

    actTile->flag = TDT_MIXED;
    img = &(soMap->img[tile]);

    if ( ! ( actTile->bits = (unsigned char *) malloc ( infoMap->bytesPerTile )))
      shutdown ( EF_MALLOC,
		 "Error allocating info tile buffer in function findInfo\n" );

    memset ( actTile->bits, 0xff, infoMap->bytesPerTile );

    for ( row = 0;
	  row < TILE_DIM;
	  row += pixSize ) {
      for ( bits = 0;
	    bits < soMap->bytesPerRow * 8;
	    bits += pixSize ) {
	for ( level = 0; level < 4; level++ ) {

	  if ( ! img->pt[level] ) break;
	  
	  for ( i = 0; i < 2; i++ ) {
	    for ( j = 0; j < 2; j++ ) {
	      for ( k = 0; k < pixSize / 2; k++ ) {
		isDoGo = TRUE;
		for ( l = 0; l < pixSize / 2; l++ ) {
		  srcByteOff = (( row + i * pixSize / 2 + k ) * soMap->bytesPerRow + 
				( bits + j * pixSize / 2 + l ) / 8 );
		  srcBitOff = ( bits + j * pixSize / 2 + l ) % 8;
		  if ( img->pt[ level ][ srcByteOff ] & ( 1 << srcBitOff )) {
		    isDoGo = FALSE;
		    break;
		  }
		}
		if ( !isDoGo ) break;

	      } /* end k loop */
	      if ( isDoGo ) {
		byteOff = (( row / ( pixSize )) * infoMap->bytesPerRow +
			   ( bits / ( 1 << (infoMap->io.level ))) / 4 );
		
		bitOff  = ( bits / ( 1 << ( infoMap->io.level ))) % 4;
		actTile->bits[byteOff] &= ~ (( 3 - level ) << ( bitOff * 2 ));
		break;
	      }

	      if ( isDoGo ) break;
	    } /* end j loop */
	    if ( isDoGo ) break;
	  } /* end i loop */
	} /* end level loop */
      } /* end bit loop */
    } /* end row loop */
    sum = 0;
    for ( i = 0; i < infoMap->bytesPerTile; i++ ) {
      sum += actTile->bits[i];
    }
    if ( !sum ) {
      actTile->flag = TDT_DOGO;
      free ( actTile->bits );
      actTile->bits = NULL;
    } else if ( sum == infoMap->bytesPerTile * 255 ) {
      actTile->flag = TDT_NOGO;
      free ( actTile->bits );
      actTile->bits = NULL;
    }
  } /* end tile loop */
  return infoMap;
} /* end findInfo */

void
initGridMap8Bit ( pathfindingmap *map )
{
  pathfindingmap *m;

  /* check for previous maps - steal data from there */
  if ( !map ) return;

  m = map;

  /* rewind map list */
  while ( m->prev ) m = m->prev;

  while ( m ) if ( !m->io.level && ( m->io.type & ( FTF_MAP | FTF_SO ))) break;

  if ( m ) {
    /* col and row are the same */
    map->tilesPerCol  = map->tilesPerRow = m->tilesPerRow;
    map->tiles        = m->tilesPerRow * m->tilesPerCol;

    /* fixed tile dims for smallOnes */
    map->rowsPerTile  = m->rowsPerTile;
    map->bytesPerRow  = m->bytesPerRow;
    map->bytesPerTile = m->bytesPerTile;
    map->res          = m->res;
  } else {
    /* set to defaults for a medium map */
    /* col and row are the same */
    map->tilesPerCol  = map->tilesPerRow = TILE_DIM;
    map->tiles        = map->tilesPerRow * map->tilesPerCol;

    /* fixed tile dims for smallOnes */
    map->rowsPerTile  = TILE_DIM;
    map->bytesPerRow  = ROW_BYTES;
    map->bytesPerTile = TILE_BYTES;
    map->res          = MD_MAP_RES;
  }

  map->io.level        = 0;
  map->io.vehicle      = VT_NONE;
  map->io.path         = m->io.path;
}


pathfindingmap *
compressMap ( pathfindingmap *map )
{
  pathfindingmap *src;
  mapIOData       ioIn     = {0};
  mapIOData       ioOut    = {0};

  if ( ! map )
    shutdown ( EF_DATA_MISSING, "Function prepTileData passed NULL map list\n" );

  copyIO ( &ioIn,  map->io );
  copyIO ( &ioOut, map->io );

  ioIn.level = ioIn.level - 1;
  src = getMap ( &map, &ioIn, NULL );
  if ( !src )
    shutdown ( EF_INFO_MISSING, "Failed to locate previos map in compressMap\n" );

  /* set maps vars */
  map->tilesPerCol  = map->tilesPerRow = src->tilesPerRow / 2;
  map->tiles        = map->tilesPerCol * map->tilesPerRow;
  map->rowsPerTile  = TILE_DIM;
  map->bytesPerRow  = ROW_BYTES;
  map->bytesPerTile = TILE_BYTES;
  map->res          = map->tilesPerRow * TILE_DIM;

  /* create tile data buffer array */
  if ( !( map->tile = (tileData *) calloc ( sizeof ( tileData ) * map->tiles, 1 )))
    shutdown ( EF_MALLOC,
	       "Error creating map tile data buffer array in function compressMap\n" );

  compressTiles ( map, src->tile );
  return map;
} /* end prepTileData */

void
compressTiles ( pathfindingmap *map, tileData *tile )
{
  unsigned char *tileBuf = NULL;
  tileData      *oldTile = NULL;

  int bit, tileRow, col, row;
  int curTile;
  int rowByte;
  int compRow, compCol;
  int hasNoGo, hasDoGo;

  int oldRowOff;
  int oldTileOff;
  int oldByteOff;

  /* compress data one level
   *
   */
  for ( row = 0; row < map->tilesPerCol; row++ ) {
    for ( col = 0; col < map->tilesPerRow; col++ ) {

      /* set current tile */
      curTile = row * map->tilesPerRow + col;
      oldTileOff = row * 2 * map->tilesPerRow * 2 + col * 2;

      /* do we really need to scan the bits? */
      if (( tile[ oldTileOff     ].flag == TDT_DOGO ) &&
	  ( tile[ oldTileOff + 1 ].flag == TDT_DOGO ) &&
	  ( tile[ oldTileOff + map->tilesPerRow * 2     ].flag == TDT_DOGO ) &&
	  ( tile[ oldTileOff + map->tilesPerRow * 2 + 1 ].flag == TDT_DOGO )) {

	map->tile[curTile].flag = TDT_DOGO;

      } else if (( tile[ oldTileOff     ].flag == TDT_NOGO ) &&
		 ( tile[ oldTileOff + 1 ].flag == TDT_NOGO ) &&
		 ( tile[ oldTileOff + map->tilesPerRow * 2     ].flag == TDT_NOGO ) &&
		 ( tile[ oldTileOff + map->tilesPerRow * 2 + 1 ].flag == TDT_NOGO )) {

	map->tile[curTile].flag = TDT_NOGO;

      } else {

	/* looks like we do */
	if ( tileBuf )
	  memset ( tileBuf, 0, TILE_BYTES );
	else  	/* create buffer and set bytes to 0 */
	  if ( !( tileBuf = (unsigned char *) calloc ( TILE_BYTES, 1 )))
	  shutdown ( EF_MALLOC,
		     "Error creating local tile data buffer in function compressMap\n" );

	/* assume nothing */
	hasNoGo = hasDoGo = FALSE;

	/* check and set every bit in the new tile */
	for ( tileRow = 0; tileRow < TILE_DIM; tileRow++ ) {
	  for ( rowByte = 0; rowByte < ROW_BYTES; rowByte++ ) {

	    oldTileOff = (( row * map->tilesPerRow * 2 + col ) * 2 +
			   ( tileRow * 2 / TILE_DIM ) * map->tilesPerRow * 2 +
			   ( rowByte * 2 / 8 ));

	    oldTile = &( tile[ oldTileOff ]);

	    if ( oldTile->flag != TDT_MIXED ) {
	      if ( oldTile->flag == TDT_NOGO ) tileBuf[tileRow * ROW_BYTES + rowByte] = 0xff;
	      continue;
	    }

	    oldRowOff = ( tileRow * 2 )%TILE_DIM * ROW_BYTES;

	    for ( bit = 0; bit < 8; bit++ ) {
	      oldByteOff = oldRowOff + ( rowByte * 2 )%ROW_BYTES + bit/4;

	      for ( compRow = 0; compRow < 2; compRow++ )
		for (compCol = 0; compCol < 2; compCol++ )

		  if ( oldTile->bits[ oldByteOff + compRow * ROW_BYTES ] &
			 ( 1 << ((bit*2 + compCol)%8 ) )) {
		    tileBuf[ tileRow * ROW_BYTES + rowByte ] |= ( 1 << bit );
		    hasNoGo = TRUE;
		  } else hasDoGo = TRUE;

	    } /* end bit loop */
	  } /* end rowByte loop */
	} /* end tileRow loop */
	/* attach to map */
	if ( hasDoGo && hasNoGo ) {
	  map->tile[ curTile ].flag = TDT_MIXED;
	  map->tile[ curTile ].bits = tileBuf;
	  tileBuf = NULL;
	} else
	  map->tile[ curTile ].flag = ( hasDoGo ) ? TDT_DOGO : TDT_NOGO;
      }
    } /* end col loop  */
  } /* end row loop    */
} /* end compressTiles */

void
fillHeader ( pathfindingmap *map, mapFileHeader *header )
{
  header->ln2TilesPerRow = findLn2 ( map->tilesPerRow );
  header->ln2TilesPerCol = findLn2 ( map->tilesPerCol );
  header->isInfo         = ( map->io.type & FTF_INFO ) ? TRUE : FALSE;
  header->compLevel      = map->io.level;
  header->ln2TileRes     = map->io.level + findLn2( map->rowsPerTile );
  header->dataOffset     = ( map->io.type & FTF_ALT ) ? 0 : 2;
} /* end mapFileHeader */

int
findLn2 ( int p )
{
  int i = 0;
  while (( p >>= 1 ) > 0 ) i++;
  return i;

} /* end findLn2 */

void
linkMaps ( pathfindingmap **maps, pathfindingmap *map )
{
  /* empty map structure requested */
  if ( !(*maps) )
    *maps = map;
  else {
    while ( (*maps)->next ) *maps = (*maps)->next;
    (*maps)->next = map;
    map->prev   = *maps;
  }
} /* end linkMaps */

pathfindingmap *
findMap ( pathfindingmap *maps, mapIOData src )
{
  pathfindingmap *map;
  map = maps;
  int dstType, srcType;

  while ( map->prev ) map = map->prev;
  while ( map ) {
    dstType = ( map->io.type & (FTF_IMG-1) );
    srcType = ( src.type & (FTF_IMG-1) );

    if (( dstType         == srcType     ) &&
	( map->io.vehicle == src.vehicle ) &&
	( map->io.level   == src.level   )) {
      return map;
    }

    map = map->next;
  }
  return NULL;
} /* end findMap */


void
copyIO ( mapIOData *dst, mapIOData src )
{
  dst->path    = src.path;
  dst->type    = src.type;
  dst->vehicle = src.vehicle;
  dst->level   = src.level;
  dst->bits    = src.bits;
}
