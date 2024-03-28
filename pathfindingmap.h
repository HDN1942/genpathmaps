/* pathfinding.h - header file for pathfinging support functions 
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


#ifndef __PATHFINDINGMAP_H__ /* only include once */
#define __PATHFINDINGMAP_H__

/************************************  includes              ************************/

/************************************  prototypes             **************************/

pathfindingmap *getMap          ( pathfindingmap **maps, mapIOData *src, mapIOData *dst );
void            writeMap        ( pathfindingmap *map ) ;
int             loadFile        ( pathfindingmap *map );
void            loadMapFile     ( pathfindingmap *map );
pathfindingmap *findInfo        ( pathfindingmap *map );
void            initGridMap8Bit ( pathfindingmap *map );
pathfindingmap *compressMap     ( pathfindingmap *map );
void            compressTiles   ( pathfindingmap *map, tileData *tile );
void            fillHeader      ( pathfindingmap *map, mapFileHeader *header );
int             findLn2         ( int p );
void            linkMaps        ( pathfindingmap **maps, pathfindingmap *map );
pathfindingmap *findMap         ( pathfindingmap *maps, mapIOData src );
void            copyIO          ( mapIOData *dst, mapIOData src );

#endif /* __PATHFINDINGMAP_H__ */
