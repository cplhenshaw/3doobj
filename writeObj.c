/* Functions that given a MODL structure (defined in modl.h) write the appropriate data to a Wavefront .obj file */

#include "modl.h"
#include <stdio.h>
#include <stdlib.h>
#include "matScaler.h"


/* Writes a MESH structure to a file stream as part of a .obj file.*/
void printMesh( MODL *model, MESH *mesh, float offset[3], FILE *ofp )
{
    //make each mesh a separate group
    //NOTE: writing with "g groups", not o groups
    fprintf(ofp, "g %s\n\n", mesh->meshName);
    
    //write out the vertices, adding the correct offset
    for(int i=0; i < mesh->numVertices; i++)
    {
	//add the correct offset to each coordinate of the vertex
	float v[3];
	for(int j=0; j<3; j++) v[j] = mesh->vertices[i][j] + offset[j];

	fprintf(ofp, "v %f %f %f\n", v[0], v[1], v[2]);
    } 
    fprintf(ofp, "\n");
    
    //write out the texture vertices
    for(int i=0; i < mesh->numTexVertices; i++)
    {
	float *vt = mesh->texVertices[i];
	
	//NOTE: Writing out the texture vertices here is tied to how they must be read back in within readObj.c  Whatever happens here must be "undone" when reading back in after editing
	fprintf(ofp, "vt %f %f\n", vt[0], -vt[1]); 
    }
    fprintf(ofp, "\n");


    //write out the vertex normals
    for(int i=0; i < mesh->numVertices; i++)
    {
	float *vn = mesh->normals[i];
	fprintf(ofp, "vn %f %f %f\n", vn[0], vn[1], vn[2]);
    }
    fprintf(ofp, "\n");

    //these will be used to store the offset to add to all vertex indices
    //NOTE: .3do indexes from 0, .obj indexes from 1, intialise offsets with 1
    //NOTE: vertices and vertex normals are BOTH indexed with the same value
    static int vertexIndexOffset = 1;
    static int texVertexIndexOffset = 1;

    //remember the previous material index
    int prevMatIndex = -1;
    //PRINT THE FACES
    for(int i=0; i < mesh->numFaces; i++)
    {
	FACE *face = mesh->faces[i];
	//determine the material to use
	if(face->hasMaterial != 0 && face->materialIndex != prevMatIndex)
	{
	    //update index and declare new material in .obj file
	    fprintf(ofp, "usemtl ");
	    char *m = model->materialNames[face->materialIndex];
	    
	/*
	    //replace the .mat with some other format for whatever texture 
	    //may not use this, see replacement directly below
	    int c = 0;
	    do {fprintf(ofp, "%c", m[c]);} while(m[c++] != '.');
	    fprintf(ofp, "gif\n");
	*/
	    //print "whatever.mat" as the material name, if eventually do a .mtl as will this can remain the name and the texture specified within the .mtl  Then when reading back in can just use the material name directly to determine which .mat to use
	    fprintf(ofp, "%s\n", m);
	    prevMatIndex = face->materialIndex;
	}


	//format "f v/vt/vn v/vt/vn ..." a triplet of indices for each vertex
	fprintf(ofp, "f ");
	//print the indices for each vertex
	for(int j=0; j < face->numVertices; j++)
	{
	    //calculate the indice triplets  
	    int vi = face->vertexIndices[j] + vertexIndexOffset;
	    int tvi = face->texVertexIndices[j] + texVertexIndexOffset;
	    fprintf(ofp, "%d/%d/%d ", vi, tvi, vi);
	}
	fprintf(ofp, "\n");
    }

    fprintf(ofp, "\n");

    //update the index offsets
    vertexIndexOffset += mesh->numVertices;
    texVertexIndexOffset += mesh->numTexVertices;

    //all for now
    return;

}

/* Recursively print a node hierarchy to a file stream in the .obj format */
void printNode(MODL *model, NODE *node, float parentOffset[3], FILE *ofp)
{
    //add this nodes offset to it's parent (accumulating as we recurse)
    float nodeOffset[3];
    for(int i=0; i<3; i++) nodeOffset[i] = parentOffset[i] + node->position[i];

    //the mesh if further offset by a pivot offset (and then should be rotated but not done here yet)
    float meshOffset[3];
    for(int i=0; i<3; i++) meshOffset[i] = nodeOffset[i] + node->pivot[i];

    //draw the mesh for this node if it has one
    if(node->meshID != -1)
    {
	printMesh(model, model->meshes[node->meshID], meshOffset, ofp);	
    }

    //recurse and print the child nodes if it has any
    if(node->hasChildren != 0)
    {
	//just recurses to the first child which will then itself recurse to 
	//any remaining siblings, see next block down
	printNode(model, model->nodes[node->childID], nodeOffset, ofp); 
    }

    //recurse and print the current node's siblings if it has any
    if(node->hasSibling != 0)
    {
	//NOTE: siblings all share the same original parent offset
	printNode(model, model->nodes[node->siblingID], parentOffset, ofp);
    }
}


/* Accepts a MODL structure previously filled by read3d0() and the name of the file to write to. */ 
void printObj( MODL *model, char *filename )
{
    if(model == NULL)
    {
	fprintf(stderr, "printObj() called with null MODL*\n");
	return;
    }
    if(filename == NULL)
    {
	fprintf(stderr, "printObj() called with null filename.\n");
	return;
    }

    //SCALE THE TEXTURE VERTICES TO THE .OBJ format (0 - 1)
    scaleTexVerts(model, 0);   //MUST UNDO WHEN READING BACK In


    //open the file for writing and ensure success
    FILE *ofp = fopen(filename, "w");
    if(ofp == NULL)
    {
	fprintf(stderr, "Could not open %s for writing.\n", filename);
	return;
    }

    
    //print the nodes recursively by starting with the first
    if(model->numNodes != 0)
    {
	float startingOffset[3] = {0.0, 0.0, 0.0};
	printNode(model, model->nodes[0], startingOffset, ofp);

    }
    
    //close the file (ideally check to ensure it closed properly)
    fclose(ofp);
    return; 
	
		
}

/* Accepts a MODL structure previously filled by read3do() and the name of the file to write to.  It then produces a .mtl file to accompany the .obj file.  Each material name will be for example "m_eye.mat" and it will then specify a texture for that material (perhaps "m_eye.gif" pr whatever format is passed in via the imFormat paramter */
void printMtl( MODL *model, char *filename, char *imFormat )
{
    if(model == NULL)
    {
	fprintf(stderr, "printMtl() called with null MODL*\n");
	return;
    }
    if(filename == NULL)
    {
	fprintf(stderr, "printMtl() called with null filename.\n");
	return;
    }

    
    //open the file for writing and ensure success
    FILE *ofp = fopen(filename, "w");
    if(ofp == NULL)
    {
	fprintf(stderr, "Could not open %s for writing.\n", filename);
	return;
    }

    fprintf(ofp, "# Material Count: %d\n", model->numMaterials);

    //for each material in the MODL create a new material in the .mtl file
    for(int i=0; i < model->numMaterials; i++)
    {
	fprintf(ofp, "newmtl %s\n", model->materialNames[i]);
	fprintf(ofp, "map_Kd ");
	//swap the .mat for .gif (or whatever is needed)
	char *t = model->materialNames[i];
	while(*t != '.')
	{
	    fprintf(ofp, "%c", *t);
	    t++;
	}
	fprintf(ofp, "%s\n", imFormat);

	//i.e	newmtl m_eye.mat
	//	map_Ka m_eye.gif
    }

    //close the file and return
    fclose(ofp);
    return;
}
