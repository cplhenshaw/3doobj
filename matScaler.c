#include "matNames.h"	//static array containg all material naems
#define TOTAL_MAT_COUNT 943
#include "matSize.h"	//static array containing dimensions of above

#include "modl.h"
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include "checkedMem.h"



/* Scale the texture vertices from absolute pixel values to values between 0 and 1 for the .obj format, and back again. NOTE: must be undone when read back in. A direction flag of 0 will scale to the .obj specification while a direction flag of 1 will scale back to the Grim specification. */
void scaleTexVerts(MODL *model, int directionFlag)
{
    //should search for material values upfront rather than EVERY SINGLE TIME
    float *matWidths = checked_malloc(sizeof(float) * model->numMaterials);
    float *matHeights = checked_malloc(sizeof(float) * model->numMaterials);

    for(int i=0; i<model->numMaterials; i++)
    {
	char *mat = model->materialNames[i];
	for(int j=0; j<TOTAL_MAT_COUNT; j++)
	{
	    if(strcmp(mat, matList[j]) == 0)
	    {
		//use this index into the matSize array (defined in matSize.h)
		matWidths[i] = matSize[j][0];
		matHeights[i] = matSize[j][1];
	    }
	}
    }
    //the matWidths and matHeights arrays can now be index by the material index to find it's dimensions
   
    //for every mesh
    for(int i=0; i<model->numMeshes; i++)
    {
	MESH *mesh = model->meshes[i];
	//remember as we scale each texture vertex
	int *isScaled = checked_calloc(mesh->numTexVertices, sizeof(int));

	//for each face in this mesh
	for(int j=0; j<mesh->numFaces; j++)
	{
	    FACE *face = mesh->faces[j];
	    if(face->hasMaterial == 0) continue;    //skip face if no material
	    
	    //scale all the texture vertices for this face accordingly
	    //(if they have not been scaled already)
	    for(int k=0; k<face->numVertices; k++)
	    {
		int texVI = face->texVertexIndices[k];
		//if it isnt already scaled
		if(!isScaled[texVI])
		{
			//scale it and remember
		    if(directionFlag == 0)
		    {
			 //going to .obj
			 mesh->texVertices[texVI][0] /= matWidths[face->materialIndex];
			 mesh->texVertices[texVI][1] /= matHeights[face->materialIndex];
		    }
		    else
		    {
			//coming back from .obj
			mesh->texVertices[texVI][0] *= matWidths[face->materialIndex];
			mesh->texVertices[texVI][1] *= matHeights[face->materialIndex];
	
		    }
		    isScaled[texVI] = 1;
		}
	    }

	}

	//quick check that we scaled all the texture vertices
	for(int j=0; j<mesh->numTexVertices; j++)
	{
	    if(isScaled[j] != 1)
	    {
		fprintf(stderr, "Texture vertice %d was never scaled in scaleTexVerts()\n", j);	
	    }
	}
    }

    return;
}

