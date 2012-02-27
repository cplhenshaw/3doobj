/*Contains functions for creating and managing a MODL structure, then freeing the associated memory. */

#include "modl.h"
#include "checkedMem.h"

#include <stdio.h>
#include <stdlib.h> 

/* Create a new MODL structure ready to be read into*/ 
MODL *createMODL() 
{
	
	MODL *model = checked_malloc( sizeof(MODL) ); 

	//set all pointer values to NULL for safety
	model->materialNames = NULL;
	model->modelName = NULL;
	model->meshes = NULL;
	model->nodes = NULL;

	return model;
}

/* Create a new MESH structure ready to be read into */
MESH *createMESH()
{
	MESH *mesh = checked_malloc( sizeof(MESH) );

	//set all pointer values to NULL for safety
	mesh->meshName = NULL;
	mesh->vertices = NULL;
	mesh->texVertices = NULL;
	mesh->lightData = NULL;
	mesh->unknown2 = NULL;
	mesh->faces = NULL;
	mesh->normals = NULL;

	return mesh;
}

/* Create a new FACE structure ready to be read into */
FACE *createFACE()
{
	FACE *face = checked_malloc( sizeof(FACE) );

	//set all pointer values to NULL for safety
	face->vertexIndices = NULL;
	face->texVertexIndices = NULL;

	return face;
}

/* Create a new NODE structure ready to be read into */
NODE *createNODE()
{
	NODE *node = checked_malloc( sizeof(NODE) );

	//set all pointer fields to NULL for safety
	node->name = NULL;

	return node;
}

/*Free the memory associated with a NODE structure. */
void freeNODE( NODE *node )
{
	//if already freed, exit early
	if( node == NULL ) return;

	//free the node name (allocated by strndup)
	free(node->name);

	//free the memory allocated to the NODE structure
	free(node);

	return;
}

/* Free the memory associated with a FACE structure. */
void freeFACE( FACE *face )
{
	//if already freed, exit early
	if( face == NULL ) return;

	//free a couple of dynamic arrays
	free(face->vertexIndices);
	free(face->texVertexIndices);

	//free the memory allocated to the FACE structure itself
	free(face);

	return;
}

/* Free the memory associated with a MESH structure */
void freeMESH( MESH *mesh )
{
	//if already freed, exit early
	if( mesh == NULL ) return;

	free(mesh->meshName);	//allocate by strndup

	//free some dynamic arrays
	free(mesh->vertices);
	free(mesh->texVertices);
	free(mesh->lightData);
	free(mesh->unknown2);

	if( mesh->faces != NULL )
	{
		//free each of the FACE structures
		for(int i=0; i < mesh->numFaces; i++)
		{
			freeFACE(mesh->faces[i]);
		}
		//free the memory allocated to the array itself
		free(mesh->faces);
	}

	free(mesh->normals);

	//finally free the memory allocated to the MESH structure itself
	free(mesh);

	return;
}

/* Free the memory associated with a MODL structure. */
void freeMODL( MODL *model )
{
	//if already freed, exit early
	if( model == NULL )
	{
		printf("ALREADY FREED THIS MODL");
		return; 
	}
	if( model->materialNames != NULL )
	{
		//free each of the material names (allocate with strndup)
		for(int i=0; i < model->numMaterials; i++)
		{
			free(model->materialNames[i]);
		}
		//free the memory allocated to the array itself
		free(model->materialNames);
	}

	free(model->modelName);	//allocated by strndup
	
	if( model->meshes != NULL )
	{
		//free each of the MESH structures
		for(int i=0; i < model->numMeshes; i++)
		{
			freeMESH(model->meshes[i]);
		}
		//free the memory allocated to the array itself
		free(model->meshes);
	}
	

	if( model->nodes != NULL )
	{
		//free each of the NODE structures
		for(int i=0; i < model->numNodes; i++)
		{
			freeNODE(model->nodes[i]);
		}
		//free the memory allocated to the array itself
		free(model->nodes);
	}
	
	//finally free the memory allocated to the structure itself
	free(model);

	return;
}
