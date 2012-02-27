/* Contains functions for reading a .3do model file into a MODL structure (see modl.h and modl.c) */

#include "modl.h" //lets us use structures 
#include "read3do.h"
#include "checkedMem.h" //checked memory allocators

#include <stdio.h>
#include <stdlib.h>
#include <string.h> //for strndup, may need to declare it as well, also strncmp

//extern char *strndup(const char *s, size_t n);
//windows couldn't find strndup so use this instead
char *mystrndup(const char *s, size_t n)
{
	//allocate memory (n chars plus a terminating byte)
	char *r = checked_malloc(sizeof(char) * (n+1));
	strncpy(r, s, n);
	//should be terminated within the .3do file but just in case
	r[n] = '\0';

	return r;
}


/* Delegates to fread but checks for failure, reports and terminates. */
size_t checked_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t numRead = fread(ptr, size, nmemb, stream);
	if( numRead != nmemb )
	{
		if( feof(stream) ) 
		{
			fprintf(stderr, "fread() hit end of file unexpectedly.\n");
		}
		else 
			fprintf(stderr, "fread() failed to read all bytes.\n");

		exit(EXIT_FAILURE);
	}
	//pass on return value of fread for transparency
	return numRead;
}

/* Read and allocate memory for a string from a file.  When rewriting to a 3do how will short strings be handled, '\0' for remaining bytes? */
char *readString( FILE *ifp, size_t numBytes )
{
	char buffer[numBytes];
	checked_fread(buffer, 1, numBytes, ifp);
	
	char *s = mystrndup(buffer, numBytes);
	if( s == NULL )
	{
		fprintf(stderr, "strndup() failed.\n");
		exit(EXIT_FAILURE);
	}

	return s;
}

/* Read a single 4 byte integer from the file and return the value. */
int readInt(FILE *ifp)
{
	int result;
	checked_fread(&result, 4, 1, ifp);

	return result;
}

float readFloat(FILE *ifp)
{
	float result;
	checked_fread(&result, 4, 1, ifp);
	
	return result;
}

/* Creates a new FACE structure and reads the face section of a .3do file into it. */
FACE *readFace( FILE *ifp )
{
	//create a new FACE structure to return
	FACE *face = createFACE();

	//fill it in
	face->faceID = readInt(ifp);
	face->faceType = readInt(ifp);
	face->geometryMode = readInt(ifp);
	face->lightingMode = readInt(ifp);
	face->textureMode = readInt(ifp);
	face->numVertices = readInt(ifp);
	face->unknown1 = readInt(ifp);
	face->hasTexture = readInt(ifp);
	face->hasMaterial = readInt(ifp);
	//read 3 floats into a vector3
	checked_fread(face->unknown2, 4, 3, ifp);
	face->extraLight = readFloat(ifp);
	checked_fread(face->unknown3, 4, 3, ifp);
	checked_fread(face->faceNormal, 4, 3, ifp);

	//allocate memory for the vertex indices (could there be none?) and texture vertex indices
	if( face->numVertices != 0 )
	{
		face->vertexIndices = checked_malloc( sizeof(int) * face->numVertices );
		for(int i=0; i < face->numVertices; i++)
		{
			face->vertexIndices[i] = readInt(ifp);
		}

	}

	//allocate memory for the texture vertex indices (must have a texture for this)
	if( face->numVertices != 0 && face->hasTexture != 0 )
	{
		face->texVertexIndices = checked_malloc(sizeof(int) * face->numVertices);
		for(int i=0; i < face->numVertices; i++)
		{
			face->texVertexIndices[i] = readInt(ifp);
		}
	}
	
	//if <hasMaterial> there will be a material index
	if( face->hasMaterial != 0 )
	{
		face->materialIndex = readInt(ifp);
	}
	
	//done, return the pointer the structure
	return face;
}

/* Creates a new MESH structure and read the mesh section of a .3do into it. */
MESH *readMesh( FILE *ifp )
{
	//create a new MESH structure to return
	MESH *mesh = createMESH();

	//fill it in
	mesh->meshName = readString(ifp, 32);
	mesh->unknown1 = readInt(ifp);
	mesh->geometryMode = readInt(ifp);
	mesh->lightingMode = readInt(ifp);
	mesh->textureMode = readInt(ifp);
	mesh->numVertices = readInt(ifp);
	mesh->numTexVertices = readInt(ifp);
	mesh->numFaces = readInt(ifp);

	//allocate memory for the mesh vertex data array (unless no vertices)
	if( mesh->numVertices != 0 )
	{
		mesh->vertices = checked_malloc( sizeof(vector3) * mesh->numVertices );
		//read in the data
		for(int i=0; i < mesh->numVertices; i++)
		{
			//read 3 floats into a vector3
			checked_fread(mesh->vertices[i], 4, 3, ifp);
		}
	}

	//allocate memory for the texture vertex data array (unless no texture vertices)
	if( mesh->numTexVertices != 0 )
	{
		mesh->texVertices = checked_malloc( sizeof(vector2) * mesh->numTexVertices );
		//read in the data
		for(int i=0; i < mesh->numTexVertices; i++)
		{
			//read 2 flots into a vector2
			checked_fread(mesh->texVertices[i], 4, 2, ifp);
		}
	}

	//allocate memory for the extra light data array (unless no vertices)
	if( mesh->numVertices != 0 )
	{
		mesh->lightData = checked_malloc( sizeof(float) * mesh->numVertices );
		for(int i=0; i < mesh->numVertices; i++)
		{
			//read in 1 float
			mesh->lightData[i] = readFloat(ifp);
		}
	}

	//allocate memory for unknown data, 4 bytes per vertice
	if( mesh->numVertices != 0 )
	{
		mesh->unknown2 = checked_malloc( sizeof(int) * mesh->numVertices );
		for(int i=0; i < mesh->numVertices; i++)
		{
			mesh->unknown2[i] = readInt(ifp);
		}
	}

	//allocate memory for the face data (unless no faces)
	if( mesh->numFaces != 0 )
	{
		mesh->faces = checked_malloc( sizeof(FACE *) * mesh->numFaces );
		for(int i=0; i < mesh->numFaces; i++)
		{
			mesh->faces[i] = readFace(ifp);
		}

	}

	//allocate memory for the vertex normals
	if( mesh->numVertices != 0 )
	{
		mesh->normals = checked_malloc( sizeof(vector3) * mesh->numVertices );
		for(int i=0; i < mesh->numVertices; i++)
		{
			//read 3 floats into a vector3
			checked_fread(mesh->normals[i], 4, 3, ifp);
		}
	}

	mesh->hasShadow = readInt(ifp);
	mesh->unknown3 = readInt(ifp);
	mesh->meshRadius = readFloat(ifp);
	checked_fread(mesh->unknown4, 4, 3, ifp);
	checked_fread(mesh->unknown5, 4, 3, ifp);

	//all done, return the pointer to this MESH structure
	return mesh;
}


/* Creates a new NODE structure and reads the node section of a .3do file into it. */
NODE *readNode( FILE *ifp )
{
	//create a new node structure to return
	NODE *node = createNODE();

	//fill it
	node->name = readString(ifp, 64);
	node->flags = readInt(ifp);
	node->unknown1 = readInt(ifp);
	node->type = readInt(ifp);
	node->meshID = readInt(ifp);
	node->depth = readInt(ifp);
	node->hasParent = readInt(ifp);
	node->numChildren = readInt(ifp);
	node->hasChildren = readInt(ifp);
	node->hasSibling = readInt(ifp);
	//read some vector3's
	checked_fread(node->pivot, 4, 3, ifp);
	checked_fread(node->position, 4, 3, ifp);
	//read some floats
	node->pitch = readFloat(ifp);
	node->yaw = readFloat(ifp);
	node->roll = readFloat(ifp);
	//read 48 unknown bytes
	checked_fread(node->unknown2, 4, 12, ifp);
	if( node->hasParent != 0 ) 
		node->parentID = readInt(ifp);
	if( node->hasChildren != 0 )
		node->childID = readInt(ifp);
	if( node->hasSibling != 0 )
		node->siblingID = readInt(ifp);

	//all done, return a pointer to this NODE structure
	return node;
}


/* Reads the .3do file given as an argument into a MODL structure and returns a pointer to it.  If the process fails it returns NULL. */
MODL *read3do( char *filename )
{
	//open the file for reading and ensure success
	//note b not needed in linux, but caused fread to hit eof early on windows
	FILE *ifp = fopen( filename, "rb" );
	if( ifp == NULL )
	{
		printf("File %s could not be opened.\n", filename);
		return NULL;
	}

	//create a new MODL structure to return
	MODL *model = createMODL();

	/* HEADER */

	//read in the fourcc code and check it is the right type of file
	checked_fread(model->fourcc, 1, 4, ifp);
	if( strncmp(model->fourcc, "LDOM", 4) != 0 )
	{
		fprintf(stderr, "%s is not a binary .3do file.\n", filename);
		return NULL;
	}
	
	model->numMaterials = readInt(ifp);

	//allocate memory for the array of character pointers, and read material names
	model->materialNames = checked_malloc( sizeof(char *) * model->numMaterials);
	for(int i=0; i < model->numMaterials; i++)
	{
		model->materialNames[i] = readString(ifp, 32);
	}

	model->modelName = readString(ifp, 32);

	/* GEOSET */

	model->unknown1 = readInt(ifp);
	model->numGeosets = readInt(ifp);
	//normally only one geoset, alert if different
	if(model->numGeosets != 1)
	{
		fprintf(stderr, "More than one geoset in this file!\n");
		fprintf(stderr, "Probably won't work!\n");
	}
	model->numMeshes = readInt(ifp);

	//allocate memory for the array of MESH pointers, and read them in
	model->meshes = checked_malloc( sizeof(MESH *) * model->numMeshes);
	for(int i=0; i < model->numMeshes; i++)
	{
		model->meshes[i] = readMesh(ifp);
	}

	/* NODES */

	model->unknown2 = readInt(ifp);
	model->numNodes = readInt(ifp);

	//allocate memory for the array of NODE pointers, and read them in
	model->nodes = checked_malloc( sizeof(NODE *) * model->numNodes );
	for(int i=0; i < model->numNodes; i++)
	{
		model->nodes[i] = readNode(ifp);
	}

	/* FOOTER */
	
	//read a float
	model->modelRadius = readFloat(ifp);
	//read a vector3
	checked_fread(model->insertionOffset, 4, 3, ifp);
	//36 bytes left, looks like another vector3, then 24 bytes of other stuff? alot zeros, could be a padded string or maybe whole file is padded?
	checked_fread(model->unknown3, 4, 3, ifp);

	checked_fread(model->unknown4, 4, 6, ifp);

	/*	
	//Was used to see if anything left in the file.
	char remainder[1000];
	size_t numRead = fread(remainder, 1, 1000, ifp);
	printf("Had %d more bytes\n", (int)numRead);
	for(int i=0; i<numRead; i++) putchar(remainder[i]);
	*/
	

	//all done, close file and return a pointer to this MODL structure
	fclose(ifp);

	return model;
}
