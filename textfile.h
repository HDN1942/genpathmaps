/* text.h - header for text file support 
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

/************************************  macros                 ***********************/

#define WRITE_IMG_RES    "Image res:        %d x %d%s"
#define READ_IMG_RES     "Image res: %d x %*d"

#define WRITE_TILE_RES   "Image tile res:   %d x %d%s"
#define READ_TILE_RES    "Image tile res: %d x %*d"

#define WRITE_TILE_COUNT "Total tiles:      %d%s"
#define READ_TILE_COUNT  "Total tiles: %d"

#define WRITE_POINT_SET  "Set point - tile: %02dx%02d:%d pt: %02dx%02d%s"
#define READ_POINT_SET   "Set point - tile: %dx%d:%d pt: %dx%d %c"

#define WRITE_TILE_MARK  "Mark mystery tile: %02dx%02d%s"
#define READ_TILE_MARK   "Mark mystery tile: %dx%d"

#define WRITE_TILE_LINK  "tile: %02dx%02d:%d pt: %02dx%02d %s to %02dx%02d:%d pt: %02dx%02d %s%s"
#define READ_TILE_LINK   "tile: %dx%d:%d pt: %dx%d %c to %dx%d:%d pt: %dx%d %c"
#define READ_TILE_LINK_2 "tile: %dx%d:%d pt: %dx%d to %dx%d:%d pt: %dx%d %c"

#define ACT_OFF   0x10


/************************************  prototypes             **************************/
#ifndef __TEXTFILE_H__
#define __TEXTFILE_H__

void             loadSmallOnesText  ( pathfindingmap *map );
void             writeSmallOnesText ( pathfindingmap *map );

#endif /* __TEXTFILE_H__ */
