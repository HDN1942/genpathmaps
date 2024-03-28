/* memory.c - memory allocation support functions
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

extern int freeInpath;
extern int freeOutpath;
extern userData data;
extern char *baseName[];

/************************************  functions             ************************/

void
freeAll ( void )
{
  jobList *job;

  /* I do hope that's it... */
  if ( data.inpath ) {
    free ( data.inpath );
    data.inpath = NULL;
  }
  if ( data.outpath ) {
    free ( data.outpath );
    data.outpath = NULL;
  }
  if ( data.maps )
    while ( data.maps )
      data.maps = freeMap ( &(data.maps) );

  if ( data.jobs )
    while ( data.jobs ) {
      job = data.jobs;
      data.jobs = data.jobs->next;
      freeJob ( job );
  }

} /* end freeAll */

jobList *
newJob ( jobList **list )
{
  jobList *job;
  jobList *tmp;

  if ( !( job = ( jobList *) malloc ( sizeof ( jobList ))))
    shutdown ( EF_MALLOC, "Error creating new job entry\n" );

  initJob ( job );

  if ( !(*list) ) *list = job;
  else {
    tmp = *list;
    while ( tmp->next ) tmp = tmp->next;
    tmp->next = job;
  }
  return job;
}

void
initJob ( jobList *job )
{
  job->in.path = NULL;
  job->in.type = FTF_NONE;
  job->in.level = -1;
  job->in.vehicle = VT_NONE;
  job->in.bits = 0;
  job->out.path = NULL;
  job->out.type = FTF_NONE;
  job->out.level = -1;
  job->out.vehicle = VT_NONE;
  job->out.bits = 0;
  job->next = NULL;
}

void
freeJob ( jobList *job )
{
  free ( job->in.path );
  free ( job->out.path );
  free ( job );
}


pathfindingmap *
freeMap ( pathfindingmap **map )
{
  int i, j;
  pathfindingmap *retMap = NULL;

  if ( !map || !*map ) return NULL;
  /* set return area, if there is one */
  if ( (*map)->next ) retMap = (*map)->next;
  else if ( (*map)->prev ) retMap = (*map)->prev;


  /* close file if open */
  if ( (*map)->fp ) fclose ( (*map)->fp );

  /* tiles may have buffers attached - free those first */
  if ( (*map)->tile ) {
    for (i = 0; i < (*map)->tiles; i++ )
      if ( (*map)->tile[i].bits )
	free ( (*map)->tile[i].bits );
    free ( (*map)->tile );
  }
  /* free smallOnes buffer, if needed */
  if ( (*map)->so ) free ( (*map)->so );
  /* tile image data could have buffers attached
   * - free them, then the image data buffer */
  if ( (*map)->img ) {
    for ( i = 0; i < (*map)->tiles; i++ ) {
      for ( j = 0; j < 4; j++ ) {
	if ( (*map)->img->pt[j] ) free ( (*map)->img->pt[j] );
      }
    }
    free ( (*map)->img );
  }
  /* free 8 bit in/out buffer */
  if ( (*map)->buf ) free ( (*map)->buf );
  /* free 8 bit in/out buffer */
  if ( (*map)->bmp ) free ( (*map)->bmp );

  /* unlink and free map */
  if ( (*map)->prev ) (*map)->prev->next = (*map)->next;
  if ( (*map)->next ) (*map)->next->prev = (*map)->prev;

  /* free map */
  free ( (*map) );
/*  (*map) = retMap; */

  /* return map list, if there is one */
  return (NULL);
}

tileArea *
newArea ( int dimX, int dimY )
{
  tileArea *area;


  /* create area or die trying */
  if (( area = (tileArea*) malloc ( sizeof ( tileArea ))) == NULL )
    shutdown (EF_MALLOC, "Memory allocation error creating tileArea...\n");

  /* initialize the structure and return it */
  area->dimX    = area->left   = dimX;
  area->dimY    = area->top    = dimY;
  area->bottom  = area->right  = -1;
  area->next    = area->prev   = NULL;
  area->lines   = NULL;
  area->size    = 0;

  return area;
} /* end newArea */

lineList *
newLine (int row, int begin, int end )
{
  lineList *line;

  /* create new line list item */
  if (( line = (lineList *) malloc ( sizeof ( lineList ))) == NULL )
    shutdown ( EF_MALLOC, "Memory allocation error creating new lineList item...\n");

  line->row   = row;
  line->begin = begin;
  line->end   = end;
  line->prev = line->next = NULL;

  return line;
} /* newLine */

void
freeAreaList (tileArea **area)
{
  tileArea  *nextArea;


  if ( *area == NULL ) return;

  /* rewind the list */
  while ( (*area)->prev ) (*area) = (*area)->prev;

  /* continue until all areas are free */
  while ( *area ) {
    /* save the next one, we're going to kill this one */
    nextArea = (*area)->next;
    freeLineList ( &((*area)->lines));

    /* free this one and take a look at the next */
    free ( *area );
    *area = nextArea;
  }
} /* end freeAreaList */

void
freeLineList ( lineList **lines )
{
  lineList *tmp;

  if ( !*lines ) return;

  while ( (*lines)->prev ) *lines = (*lines)->prev;
  while ( *lines ) {
    tmp = *lines;
    *lines = (*lines)->next;
    free ( tmp );
  }
}

void
freeTiles ( pathfindingmap *map, tileData **tile )
{
  int i;
  for ( i = 0; i < map->tiles; i++ )
    if ( (*tile)[i].flag == TDT_MIXED ) free ( (*tile)[i].bits );
  free (*tile);
  *tile = NULL;
}


tileData *
copyTiles ( pathfindingmap *map )
{
  tileData *tile = NULL;
  int i;
  if ( !(map->tile) ) return NULL;

  /* create tile data buffer array */
  if ( !( tile = (tileData *) calloc ( sizeof ( tileData ) * map->tiles, 1 )))
    shutdown ( EF_MALLOC,
	       "Error creating map tile data buffer array in function compressMap\n" );

  for ( i = 0; i < map->tiles; i++ ) {
    tile[i].flag = map->tile[i].flag;
    if ( map->tile[i].flag == TDT_MIXED ) {
      if ( !( tile[i].bits = (unsigned char *) malloc ( map->bytesPerTile )))
	shutdown ( EF_MALLOC,
		   "Error creating tile data array in function copyTiles\n"  );
      memcpy ( tile[i].bits, map->tile[i].bits, map->bytesPerTile );
    }
  }
  return tile;
}

char *
dupString ( char *str )
{
  char *p;

  if ( !( p = (char *) calloc ( strlen(str) + 1,1 )))
    shutdown ( EF_MALLOC, "Error duplicating string.\n" );
  strncpy ( p, str, strlen(str) );
  return p;

}



/* end memory.c */
