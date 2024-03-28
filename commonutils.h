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

#ifndef __COMMONUTILS_H__
#define __COMMONUTILS_H__

/************************************  includes              ************************/

#include "memory.h"

/************************************  prototypes             ***********************/

void           setMapIO          ( mapIOData *ioSrc, mapIOData *ioDst, int level );
char          *fullPath          ( pathfindingmap *map );
int            openFile          ( pathfindingmap *map, const char *mode );
void           shutdown          ( int err, char *format, ... );
void           debug             ( debugFlag level, char *format, ... );
int            fileSize          ( pathfindingmap *map );
char          *fileName          ( char *path );
char          *strToUpper        ( char *str );
int            strCaseCmp        ( char *dst, char *src, int n );
int            isFile            ( char *str );
int            isDir             ( char *str );
char          *fullName          ( char *path, int type, int vehicle, int level );
jobList       *addJob            ( jobList **list, jobList *curJob );
void           addJobs           ( void );
jobList       *addVehicle        ( char *filename, int vtype, int level );
int            isPathmapFile     ( char *path, int *type, int *level );
#endif /*  __COMMONUTILS_H__ */
