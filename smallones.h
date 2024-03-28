/* smallones.h - header file for smallOnes.c 
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


#ifndef __SMALLONES_H__ /* include only onec */
#define __SMALLONES_H__

/************************************  includes              ************************/


/************************************  macros                 ***********************/

/* there are areas in some of the game pathmaps that don't connect
 * to any other areas, but have smallOnes set. These are like islands.
 * Bot's can't get to these areas and if, by chance, one should end up in one
 * of these areas, there it stay whether this is set or not.
 * I don't see the need to set this but the ea games pick theirs this way.
 */

#define HAS_ISLANDS TRUE

/* this is just for reference:
 * hasLower & hasRight in the smallOnes structure
 * are actually bitmaped. each bit points the different
 * smallOnes points to points in the tile below or to the
 * right of this one according to this bitmap
 */
#define A1TO1 1 << 0
#define A1TO2 1 << 1
#define A1TO3 1 << 2
#define A1TO4 1 << 3
#define A2TO1 1 << 4
#define A2TO2 1 << 5
#define A2TO3 1 << 6
#define A2TO4 1 << 7
#define A3TO1 1 << 8
#define A3TO2 1 << 9
#define A3TO3 1 << 10
#define A3TO4 1 << 11
#define A4TO1 1 << 12
#define A4TO2 1 << 13
#define A4TO3 1 << 14
#define A4TO4 1 << 15

#define DEF_OFF   48
#define SO_LEN    16
#define BYTE_LEN  8
#define BOAT_MASK_LEN 128


/************************************  structures and enums  ***************************/

typedef enum _edgeDirection
  {
    EDGE_NONE  = 0,
    EDGE_RIGHT = 1 << 0,
    EDGE_DOWN  = 1 << 1,
    EDGE_LEFT  = 1 << 2,
    EDGE_UP    = 1 << 3
  } edgeDirection;

/************************************  prototypes             **************************/

pathfindingmap  *genSmallOnes       ( pathfindingmap *map );
void             findAreas          ( pathfindingmap *map, int offset );
void             addSegment         ( tileArea **area, int dimX, int dimY, 
				      int row, int begin, int end, int contigious );
tileArea        *mergeAreas         ( tileArea **area, tileArea **merge );
tileArea        *sortAreas          ( tileArea *area, int bySize );
void             addSmallOnes       ( pathfindingmap *map, tileArea *area, int offset );
void             setPoint           ( pathfindingmap *map, 
				      int offset, int level, int col, int row );
int              linesConnect       ( lineList *line, int row, int begin, int end );

tileArea        *appendNewArea      ( tileArea **area, int dimX, int dimY );
tileArea        *appendLine         ( tileArea *area, int row, int begin, int end );

void             centerPoint        ( unsigned char *img, int *col, int *row );
void             loadSmallOnes      ( pathfindingmap *map );

int              insertLineItem     ( lineList **list, int row, int begin, int end );
int              cmpLines           ( lineList *list1, lineList *list2 );
lineList        *preInsertLine      ( lineList **list1, lineList **list2 );
lineList        *postInsertLine     ( lineList **list1, lineList **list2 );
void             setBits            ( unsigned char *dst, int begin, int end, 
				      int bufSize, int setHi );

#endif /* __SMALLONES_H__ */
