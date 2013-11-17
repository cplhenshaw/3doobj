/* The main file for the first executable.  This part converts a .3do file into a .obj model which can then be edited.  The resulting .obj can be "merged" back into the original .3do using the second executable. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "modl.h"
#include "read3do.h"
#include "writeObj.h"

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
	printf("3DO-OBJ by cplhenshaw: Converts 3DO files to OBJ files for modelling.\n\n");
	printf("Usage: '%s manny.3do manny.obj\n\n", argv[0]);
	printf("Also accept an optional third argument to specify texture image format in the .mtl file:\n");
	printf("E.g.: '%s manny.3do manny.obj .jpg'\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    //read in the .3do file to a MODL structure
    MODL *m = read3do(argv[1]);
    if(m == NULL)
    {
	fprintf(stderr, "Failed to read in .3do file %s\n", argv[1]);
	exit(EXIT_FAILURE);
    }

    //write out the structure to a .obj file
    printObj(m, argv[2]);
    
    //determine a .mtl name (i.e manny.obj will have manny.mtl) 
    char mtlFilename[32];
    strncpy(mtlFilename, argv[2], 32);
    char *c = mtlFilename;
    while(*c++ != '.');
    strncpy(c, "mtl", 4);   //one more space for null byte
	
    if(argc == 4)
    {
	//if a third command line argument given use it as the image format for the .mtl
	printMtl(m, mtlFilename, argv[3]);
    }
    else
    {
        //default to .png 
	printMtl(m, mtlFilename, ".png"); 	
    }


    //free memory associated with the MODL structure
    freeMODL(m);

    exit(EXIT_SUCCESS);
}
