/* commonutils.c - various utility functions
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

/************************************  includes              *********************/

#include "common.h"
#include "commonutils.h"

/************************************  global variables      *********************/

extern userData data;
extern char *baseName[];

/************************************  functions             *********************/

void
setMapIO ( mapIOData *ioSrc,  mapIOData *ioDst, int level )
{
    /* set input file type */
    switch ( data.readflag )
      {
      case RF_NONE:
	shutdown ( EF_DATA_MISSING, "Error in main: no input type specified\n" );
      case RF_RAW:
	ioSrc->type = ( FTF_RAW | FTF_IMG | FTF_MAP | FTF_READ );
	break;
      case RF_BMP:
	ioSrc->type = ( FTF_IMG | FTF_MAP | FTF_READ );
	break;
      case RF_MAP:
	ioSrc->type = ( FTF_MAP | FTF_READ );
	break;
      case RF_SO:
	ioSrc->type = ( FTF_SO | FTF_READ );
	break;
      case RF_INFO:
	ioSrc->type = ( FTF_INFO | FTF_READ );
	break;
      case RF_TXT:
	ioSrc->type = ( FTF_TXT | FTF_READ );
	break;
      }
    ioSrc->path  = data.inpath;
    ioDst->path  = data.outpath;
    ioDst->level = ioSrc->level = level;
    ioDst->type  = data.writeflag;

    ioDst->bits  = (ioDst->type & FTF_RAW ) ? 8 : ((ioDst->type & FTF_MAP ) ? 1 : 4 );

}

char *
fullPath ( pathfindingmap *map )
{
  return fullName ( map->io.path, 
		    map->io.type, 
		    map->io.vehicle, 
		    map->io.level );
}

int
openFile ( pathfindingmap *map, const char *mode )
{
  char *filename;
  int isRead;
  int ret = FALSE;

  isRead = !strcmp ( mode, READ_MODE );

  filename = isRead ? map->io.path : fullPath ( map );

  debug ( DBG_INFO, "Opening file for %s: %s\n",
	  ( isRead ? "reading" : "writing" ), filename );

  if ( !( map->fp = fopen (( filename), mode )))
    debug ( DBG_ERR, "Error opening file for %s: %s\n",
	    ( isRead ? "reading" : "writing" ), filename );
  else ret = TRUE;

  if ( !isRead ) free (filename);
  return ret;
}

void
shutdown ( int err, char *format, ... )
{
  /* use starg macros to supply arguments to the print function */
  va_list ap;
  va_start ( ap, format );

  /* check level of shutdown and report */
  if ( format ) {
    if (err) debug ( DBG_ERR, format, ap );
    else debug ( DBG_INFO, format, ap );
  }

  va_end(ap);
  freeAll ();
  exit (err);
} /* end shutdown */

void
debug ( debugFlag level, char *format, ... )
{
  va_list ap;
  if ( level <= data.debug ) {
    va_start ( ap, format );
	vprintf ( format, ap );
    va_end(ap);
  }
} /* end debug */

int
fileSize ( pathfindingmap *map )
{
  int filesize;
  int curPointer;

  if ( !map->fp )
    return -1;
  /* save current file pointer */
  curPointer = ftell ( map->fp );

    /* set pointer to file end */
  if ( fseek ( map->fp, 0, SEEK_END ) != 0 )
    return -1;

  /* how far is that exactly? */
  filesize = ftell ( map->fp );
  /* reset the file pointer */
  fseek ( map->fp, curPointer, SEEK_SET );
  /* return file size */
  return filesize;
}

char *
fileName ( char *path )
{
  char *p;
  if (( p = strrchr ( path, PATHSEP )) == NULL )
    p = path;
  else p++;
  return p;
}

char *
strToUpper ( char *str )
{
  int i;
  for ( i = 0; i < (int) strlen ( str ); i++ ) str[i] = toupper ( str[i] );
  return str;
}

int
strCaseCmp ( char *dst, char *src, int n )
{
  int len;
  int i;
  int diff;
  len = MIN(MIN( (int) strlen( dst ), (int) strlen( src )),n);
  for ( i = 0; i < len; i++ ) {
    diff = toupper( dst[i] ) - toupper( src[i] );
    if ( diff ) return !!diff;
  }
  if ( len == n ) return 0;
  return !!(( strlen( dst ) - strlen( src )));
}

int
isFile ( char *str )
{
  FILE *fp;

  if ( !( fp = fopen ( str, READ_MODE ))) return FALSE;
  else fclose ( fp );

  return TRUE;
}

int
isDir ( char *str )
{
  char buffer[ BUF_SIZE ];
  
  getcwd ( buffer, BUF_SIZE );

  if ( chdir ( str ) == - 1 ) return FALSE;
  else chdir ( buffer );
  return TRUE;
  
}


char *
fullName ( char *path, int type, int vehicle, int level )
{
  char buffer[BUF_SIZE];
  char *ret;
  char *ext;
  memset ( buffer, 0, sizeof (buffer));

  if ( type & FTF_IMG ) {
    ext = ( type & FTF_RAW ) ? FILE_8BIT_RAW_EXT : FILE_BMP_EXT;
    
    switch ( type  & ( FTF_IMG - 1 ))
      {
      case FTF_NONE:
	if ( type & FTF_GRID ) {
	  if ( type & FTF_NUMBERS )
	    sprintf ( buffer, FILENAME_NUM_GRID, path, PATHSEP );
	  else
	    sprintf ( buffer, FILENAME_GRID, path, PATHSEP );
	}
	break;
      case FTF_MAP:
	sprintf ( buffer, FILENAME_IMG_MAP,
		  path, PATHSEP, baseName[vehicle], vehicle, level, ext );
	break;
      case FTF_INFO:
	sprintf ( buffer, FILENAME_IMG_INFO,
		  path, PATHSEP, baseName[vehicle], ext );
	break;
      case FTF_SO:
	sprintf ( buffer, FILENAME_IMG_SO,
		  path, PATHSEP, baseName[vehicle], ext );
	break;
      case FTF_TXT:
      case FTF_TXT | FTF_SO:
	sprintf ( buffer, FILENAME_TXT,
		  path, PATHSEP, baseName[vehicle] );
	break;
      }
  } else {
    switch ( type  & ( FTF_IMG - 1 ))
      {
      case FTF_MAP:
	sprintf ( buffer, FILENAME_MAP_RAW,
		  path, PATHSEP, baseName[vehicle], vehicle, level );
	break;
      case FTF_INFO:
	sprintf ( buffer, FILENAME_INFO_RAW,
		  path, PATHSEP, baseName[vehicle]);
	break;
      case FTF_SO:
	sprintf ( buffer, FILENAME_SO_RAW,
		  path, PATHSEP, baseName[vehicle] );
	break;
      case FTF_TXT:
      case FTF_TXT | FTF_SO:
	sprintf ( buffer, FILENAME_TXT,
		  path, PATHSEP, baseName[vehicle] );
	break;
      }
  }
  if ( !( ret = (char *) malloc ( strlen ( buffer )+1)))
    shutdown ( EF_MALLOC,
	       "Memory error creating full pathname: \n\t%s\n",
	       buffer );
  strncpy ( ret, buffer, strlen ( buffer )+1);
  return ret;

}

jobList *
addJob ( jobList **list, jobList *curJob )
{
  jobList *job;

  job  = newJob ( list );

  if ( !curJob ) return job;

  job->in.path     = ( curJob->in.path  ) ? dupString ( curJob->in.path  ) : NULL ;
  job->out.path    = ( curJob->out.path ) ? dupString ( curJob->out.path ) : NULL ;
  job->in.type     = curJob->in.type;
  job->out.type    = curJob->out.type;
  job->out.vehicle = job->in.vehicle = curJob->in.vehicle;
  job->in.level    = curJob->in.level;
  job->out.level   = curJob->out.level;
  return job;

}

void
addJobs ( void )
{
  int level;
  int vtype;

  if ( !data.inpath || !data.outpath ) 
    shutdown ( EF_DATA_MISSING, "Insufficient arguments\n");

  if ( !isDir (data.outpath ))
    shutdown ( EF_DATA_MISSING, "Output directory malformed\n");


  if ( isPathmapFile ( data.inpath, &vtype, &level ) && isDir ( data.outpath )) 
    addVehicle ( data.inpath, vtype, level );
  else
    shutdown ( EF_DATA_MISSING, "input file  malformed\n");

}

jobList *
addVehicle ( char *filename, int vtype, int level )
{
  jobList job;

  char buffer[ BUF_SIZE ];
  /* char *p; */
  int maxLevel;
  int mtype;
  int i;

  initJob ( &job );
  job.in.path = dupString ( filename );
  job.in.level = level;
  job.out.vehicle = job.in.vehicle = vtype;
  maxLevel = ( INRANGE ( job.out.vehicle, VT_BOAT, VT_LANDINGCRAFT )) ? 5 : 2;

  switch ( data.readflag )
    {
    case RF_NONE:
      shutdown ( EF_DATA_MISSING, "No read file type found.\n" );
    case RF_BMP:
      job.in.type = FTF_IMG | FTF_MAP;
      break;
    case RF_RAW:
      job.in.type = FTF_IMG | FTF_RAW | FTF_MAP;
      break;
    case RF_MAP:
      job.in.type = FTF_MAP;
      break;
    case RF_SO:
      job.in.type = job.out.type =FTF_SO;
      job.in.level = 0;
      break;
    case RF_INFO:
      job.in.type = job.out.type = FTF_INFO;
      break;
    case RF_TXT:
      job.in.type = FTF_TXT;
      break;
    }
  job.in.type |= FTF_READ;
  strncpy ( buffer, filename, BUF_SIZE );

  if ( job.in.type & FTF_MAP ) {
    if ( job.in.level == 0 ) {
      for ( mtype = FTF_MAP; mtype <= FTF_TXT; mtype <<= 1 ) {
	if ( mtype & data.writeflag ) {
	  job.out.type = ( IMGFLAG( data.writeflag ) | mtype | FTF_WRITE );
	  if ( mtype & FTF_INFO )
	    job.out.level = ( INRANGE( job.out.vehicle, VT_BOAT, VT_LANDINGCRAFT ) ? 3 : 1);
	  else job.out.level = 0;
	  job.out.path = data.outpath;
	  addJob ( &(data.jobs), &job );
	  if ( !IMGFLAG( data.writeflag ) && (job.out.type & FTF_MAP ))	
	    for ( i = 1; i <= maxLevel; i++ ) {
	      job.out.level = i;
	    addJob ( &(data.jobs), &job );
	    }
	}
      }
    } else {
      job.out.type = ( IMGFLAG( data.writeflag ) | FTF_MAP | FTF_WRITE );
      job.out.level = job.in.level;
      job.out.path = data.outpath;
      addJob ( &(data.jobs), &job );

    }
  } else if ( job.in.type & FTF_TXT ) {
    job.out.level = 0;
    job.out.type = FTF_SO | FTF_WRITE | IMGFLAG ( data.writeflag );
    job.out.path = data.outpath;
    addJob ( &(data.jobs), &job );
  } else {
    if ( job.in.type & FTF_INFO )
      job.out.level = ( INRANGE( job.out.vehicle, VT_BOAT, VT_LANDINGCRAFT ) ? 3 : 1);
    else job.out.level = 0;
    job.out.type |= FTF_WRITE | IMGTYPES ( data.writeflag ) | IMGFLAG ( data.writeflag );
    job.out.path = data.outpath;
    addJob ( &(data.jobs), &job );
    
  }
  return data.jobs;
}

int
isPathmapFile ( char *path, int *vtype, int *level )
{
  char buffer[ BUF_SIZE ];
  char *p;
  char scanVt[13] = {0};
  int scanLvl;
  char scanExt[4];
  int i, j;

  if ( !path || !strlen (path) || !isFile ( path )) return FALSE;

  if ( !( p = strrchr ( path, PATHSEP ))) p = path;
  else p++;

  for ( i = 0; i <= strlen ( p ); i++ ) buffer[i] = (char) toupper ( p[i] );

  *vtype = -1;

  for ( i = VT_TANK; i <= VT_AMPHIBIUS; i++ ) {
    for ( j = 0; j <= strlen ( baseName[i] ) + 1; j++ ) 
      scanVt[j] = (char) toupper ( baseName[i][j] );
    if ( strncmp ( buffer, scanVt, strlen ( scanVt )) == 0 ) {
      *vtype = i;
      *level = scanLvl;
      p = buffer + strlen ( baseName[i] );
      break;
    }
  }
  if ( *vtype < 0 )
    return FALSE;

  if ( sscanf ( p, "%*01dLEVEL%01dMAP.%s", &scanLvl, scanExt) == 2 ) {
    if ( !strncmp ( scanExt, "RAW", 3 )) {
      data.readflag = RF_MAP;
      if ( !IMGFLAG( data.writeflag )) data.writeflag |= FTF_IMG;
    } else if ( !strncmp ( scanExt, "BMP", 3 )) {
      data.readflag = RF_BMP;
    }
    if ( !IMGTYPES( data.writeflag )) data.writeflag |= FTF_ALL_MAPS;
    *level = scanLvl;
    return TRUE;

  } else 
    if ( sscanf ( p, "%*01dLEVEL%01dMAP8BIT.%s", &scanLvl, scanExt) == 2 ) {
      if ( strncmp ( scanExt, "RAW", 3 )) return FALSE;
      data.readflag = RF_RAW;
      if ( !IMGTYPES( data.writeflag )) data.writeflag = FTF_ALL_MAPS;
      *level = scanLvl;
      return TRUE;
      
    } else if ( !strncmp ( p, "INFO.RAW", 8 )) {
      data.readflag = RF_INFO;
      data.writeflag = FTF_INFO_IMG;
      *level = (( *vtype == 2 ) || ( *vtype == 3 )) ? 3 : 1;
      return TRUE;
      
    } else if ( sscanf ( p, ".%s", scanExt )) {
      if ( !strncmp ( scanExt, "RAW", 3 )) {
	data.readflag = RF_SO;
	if ( !( data.writeflag & ( FTF_TXT | FTF_IMG ))) 
	  data.writeflag |= FTF_DIAG_IMG | FTF_SO;
      } else if ( !strncmp ( scanExt, "TXT", 3 )) {
	data.readflag = RF_TXT;
      }
      scanLvl = 0;
      return TRUE;
    }
  
  return FALSE;
}
