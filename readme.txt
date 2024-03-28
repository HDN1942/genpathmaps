Genpathmaps is a conversion and diagnostic tool for Battlefield pathfinding files. 

Installing:

Extract the executable and batch files to any folder you desire. The batch files 
expect the executable to be in c:\genpathmaps. If you place it somewhere else, 
edit the batch files and change the line:

    set genmap="c:\genpathmaps\genpathmaps.exe"

If the batch files are placed on the desktop, you can drag and drop all the 
pathfinding files that need converting onto them at once. The converted files are 
placed in seperate subfolders (bmp, raw and 8bit) in the output directory. 
The default set in the batch files is this line:

    set basedir="c:\genpathmaps\pathfinding"

Change this to point to any folder you want to use for output. The "" is only
needed if there are spaces in the path.

Once you have the batch files set up the way you want, place them, or shortcuts
to them some place handy. Then highlight the files you want to process in 
windows explorer and drag them onto the batch file icon.

Note: 
    Level 2 search maps can be used for boats and landing craft with the batch
    files. All bmp bitmaps are constrain to the actual map size, so all they
    need is to be renamed to level 0 maps. The batch files do this automaticaly
    when converting to bmp bitmaps.

    If using 8 bit raw files, resize the boat and landing craft level 2 files in
    an image editor to the actual size before using. I suggest ignoring the 8 bit
    raw files and convert to/from the game generated raw files and bmp bitmaps
    instead.

Command Line usage:

genpathmaps [options] <source file> <destination path>

genpathmaps accepts 3 types of files for input. Any of the compressed
game raw files can be used to create either bmp bitmaps or 8 bit
raw images. For bmp bitmaps and 8 bit raw images, genpathmaps only accepts
level 0 search maps for input.
Bmp bitmaps are constained to the size of the original search map,

while 8 bit raw images are not.
Since the game only creates leves 2-5 for sea vessels, the compressed
game raw files are best converted to bmp images for editing. The level 2
bmp bitmap can be renamed to level 0 and used to recreate the compressedgame raw files.

Options are:

     /M = output all search map files
     /S = output smallOnes file
     /I = output info file
     /T = output text file (smallOnes points)
     /8 = output is 8 bit raw image file
     /B = output is a bitmap image

     /D = output diagnostic images
     /G = include grid in diagnostic images
     /N = include grid numbers in diagnostic image
     /L = connect points in smallOnes diagnostic image

     /A = use alternat compression method
     /v = increase output verbosity
     /V = print version number and quit
     /h or /?  = display this help

Examples:
     genpathmaps \some_path\InfantryInfo.raw \output

The Infantry info file is converted to a bmp bitmap for viewing.
No options are needed. The default is to output bmp's from compressed raw

     genpathmaps \some_path\Infantry1Level0Map.raw \output

If no options are chosen, the default is to convert the level 0 search
map to a bmp, as well as create images of the info and smallones file.
If the /D option is used with this the Info and smallOnes images will
also contain a numbered grid and lines are drawn connecting the smallOnes
points.

     genpathmaps \some_path\Infantry1Level0Map.bmp \output

All of the compressed game pathfinding files are created and output to
the output directory. 8 bit raw images produce the same result.

Note:
8 bit raw pathfinding images are flipped verticaly. The bmp images are not. 

If you use bmp bitmaps to generate pathfinding files, use 1 bit images. 

