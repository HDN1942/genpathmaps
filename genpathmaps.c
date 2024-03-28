/* genPathmaps.c - the main file source file
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
#include "pathfindingmap.h"
#include "smallones.h"
#include "textfile.h"

/************************************  prototypes             ***********************/

void     parseArgs    ( int argc, char *argv[] );
void     usage        ( const char *name );
void     addJobs      ( void );
int      addInputFile ( jobList **list, char *path, int single );
jobList *addJob       ( jobList **list, jobList *job );

/************************************  global variables      *********************/

userData data = {
  NULL,
  NULL,
  NULL,
  RF_NONE,
  FTF_NONE,
  NULL,
  DBG_WARN
};

int freeInpath  = FALSE;

unsigned char *baseName[] = {
  "Tank",
  "Infantry",
  "Boat",
  "LandingCraft",
  "Car",
  "Heli",
  "Amphibius"
};

char *inputType[] = {
  "Pathfinding",
  "Info",
  "SmallOnes",
  "Text"
};

char *ext[] = {
  FILE_BMP_EXT,
  FILE_8BIT_RAW_EXT,
  FILE_RAW_EXT,
  FILE_TXT_EXT
};

/************************************  entry point           *********************/

int
main (int argc, char *argv[])
{
  jobList *job;

  debug ( DBG_NOTICE , "Starting %s\n", fileName ( argv[0] ));

  /* parse out command line arguments */
  parseArgs ( argc, argv );
  addJobs ();

  /* any files to process? */
  if ( !data.jobs )
    shutdown ( EF_NO_JOBS, "No input files were found.\n");

  while ( data.jobs ) {
    job = data.jobs;
    getMap ( &(data.maps), &(job->in), &(job->out) );
    data.jobs = data.jobs->next;
    freeJob ( job );
  }

  /* get more coffee */
  shutdown ( EF_NONE, "" );

  /* shutup gcc */
  return 0;
} /* end main */

/************************************  functions             *********************/

void
parseArgs ( int argc, char *argv[] )
{
  char argType;
  int i = 0;
  char *name;

  if (( name = strrchr ( argv[0], PATHSEP )) == NULL ) name = argv[0];
  else name++;

  while ( ++i < argc ) {

    if (( strlen (argv[i]) == 2 ) && ( argv[i][0] == COMSEP )) {
      argType = argv[i][1];
      switch (argType)
	{
	  /* parameters -s and -f change input from 8 bit raw images to
	   * smallOnes file and text input. both output a smallOnes and smallOnes plot
	   * file.
	   */
	case 'M':
	  /* output map file */
	  data.writeflag |= FTF_MAP;
	  break;
	case 'S':
	  /* output smallOnes file */
	  data.writeflag |= FTF_SO;
	  break;
	case 'I':
	  /* output info file */
	  data.writeflag |= FTF_INFO;
	  break;
	case 'T':
	  /* this turns off output */
	  data.writeflag |= FTF_TXT;
	  break;
	case '8':
	  /* output 8 bit raw image */
	  data.writeflag |= FTF_RAW | FTF_IMG;
	  break;
	case 'R':
	  /* output 8 bit raw image */
	  data.writeflag |= FTF_RAW;
	  break;
	case 'B':
	  /* output  raw image */
	  data.writeflag |= FTF_IMG;
	  break;
	case 'P':
	  /* output preprocessed images */
	  data.writeflag |= ( FTF_IMG | FTF_PREP );
	  break;
	case 'D':
	  /* write smallOnes plot raw file with lines and grid */
	  data.writeflag |= FTF_DIAG_IMG;
	  break;
	case 'G':
	  /* include grid, used in conjunction with the -p option */
	  data.writeflag |= FTF_GRID;
	  break;
	case 'L':
	  /* include grid, used in conjunction with the -p option */
	  data.writeflag |= FTF_LINES;
	  break;
	case 'N':
	  /*  create 8 bit numbered grid image*/
	  data.writeflag |= FTF_NUMBERS;
	  break;
	case 'A':
	  /* use alternate compression method for info file */
	  data.writeflag |= FTF_ALT;
	  break;
	case 'v':
	  data.debug++;
	  break;
	case 'V':
	  printf ( "%s Version %d.%d-%s\n",
		   name,
		   MAJOR_VERSION,
		   MINOR_VERSION,
		   SUB_VERSION );
	  exit (0);
	case 'h':
	case '?':
	  /* print usage */
	  usage ( name );
	  break;
	default:
	  printf ( "Unknown parameter: %c\n", argType );
	  exit (0);
	}
    } else {
      if ( data.inpath == NULL ) {
		data.inpath = dupString (argv[i]);

      } else if ( data.outpath == NULL ) {
	  data.outpath = dupString(argv[i]);

      } else {
	printf ( "What do I do with this: %s\n", argv[i] );
	exit (0);
      }
    }
  }
} /* end parseArgs */

void
usage ( const char *name )
{
  printf ( "\nUsage: %s [options] <source file> <destination path>\n\n", name );
  printf ( "%s accepts 3 types of files for input. Any of the compressed\n", name );
  printf ( "game raw files can be used to create either bmp bitmaps or 8 bit\n" );
  printf ( "raw images. For bmp bitmaps and 8 bit raw images, %s only accepts\n", name );
  printf ( "level 0 search maps for input.\n" );
  printf ( "Bmp bitmaps are constained to the size of the original search map,\n\n" );
  printf ( "while 8 bit raw images are not.\n" );
  printf ( "Since the game only creates leves 2-5 for sea vessels, the compressed\n" );
  printf ( "game raw files are best converted to bmp images for editing. The level 2\n" );
  printf ( "bmp bitmap can be renamed to level 0 and used to recreate the compressed" );
  printf ( "game raw files.\n\n" );
  printf ( "Options are:\n\n" );


  printf ( "     %cM = output all search map files\n",                       COMSEP );
  printf ( "     %cS = output smallOnes file\n",                             COMSEP );
  printf ( "     %cI = output info file\n",                                  COMSEP );
  printf ( "     %cT = output text file (smallOnes points)\n",               COMSEP );
  printf ( "     %c8 = output is 8 bit raw image file\n",                    COMSEP );
  printf ( "     %cB = output is a bitmap image\n\n",                        COMSEP );

  printf ( "     %cD = output diagnostic images\n",                          COMSEP );
  printf ( "     %cG = include grid in diagnostic images\n",                 COMSEP );
  printf ( "     %cN = include grid numbers in diagnostic image\n",          COMSEP );
  printf ( "     %cL = connect points in smallOnes diagnostic image\n\n",    COMSEP );

  printf ( "     %cA = use alternat compression method\n", COMSEP );
  printf ( "     %cv = increase output verbosity\n", COMSEP );
  printf ( "     %cV = print version number and quit\n", COMSEP );
  printf ( "     %ch or %c\?  = display this help\n\n", COMSEP, COMSEP );

  printf ( "Examples:\n" );
  printf ( "     %s %csome_path%cInfantryInfo.raw %coutput\n\n", 
	   name, PATHSEP, PATHSEP, PATHSEP );
  printf ( "The Infantry info file is converted to a bmp bitmap for viewing.\n" );
  printf ( "No options are needed. The default is to output bmp's from compressed raw\n\n" );
  printf ( "     %s %csome_path%cInfantry1Level0Map.raw %coutput\n\n", 
	   name, PATHSEP, PATHSEP, PATHSEP );
  printf ( "If no options are chosen, the default is to convert the level 0 search\n" );
  printf ( "map to a bmp, as well as create images of the info and smallones file.\n" );
  printf ( "If the %cD option is used with this the Info and smallOnes images will\n", 
	   PATHSEP );
  printf ( "also contain a numbered grid and lines are drawn connecting the smallOnes\n" );
  printf ( "points.\n\n" );
  printf ( "     %s %csome_path%cInfantry1Level0Map.bmp %coutput\n\n", 
	   name, PATHSEP, PATHSEP, PATHSEP );
  printf ( "All of the compressed game pathfinding files are created and output to\n" );
  printf ( "the output directory. 8 bit raw images produce the same result.\n\n" );

  exit(0);
} /* end usage */

