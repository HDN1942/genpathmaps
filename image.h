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

#ifndef __IMAGE_H__
#define __IMAGE_H__


/************************************  includes              ************************/

/************************************  macros                 ***********************/

#define NUM_OFF_X 2
#define NUM_OFF_Y 2


/************************************  structures and enums  ************************/
/************************************  prototypes             ***********************/

void loadImage        ( pathfindingmap *map );
void loadBmpHeader    ( pathfindingmap *map );
void writeImageFile   ( pathfindingmap *map );
void writeBmpHeader   ( pathfindingmap *map );
void plotImageRow     ( pathfindingmap *map, int row );
void prepImageBuf     ( pathfindingmap *map, int row );
void plotSmallOnesRow ( pathfindingmap *map, int row );
void plotPoint        ( pathfindingmap *map, int tile, int col, int row, int value );
void drawRectangle    ( pathfindingmap *map, 
			int tile1, int col1, int row1,
			int tile2, int col2, int row2, int value );
void drawLine         ( pathfindingmap *map,
			int tile1, int col1, int row1,
			int tile2, int col2, int row2, int value );
void readPixels       ( pathfindingmap *map, int inBits );
void setRowPixel      ( pathfindingmap *map, int offset, int value );
void fillColorMap     ( pathfindingmap *map, rgbQuad *bmpColors );

#endif /* __IMAGE_H__ */
