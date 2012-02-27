/* Accept a MODL structure and an OBJ structure, and where the name of a group in the OBJ file resembles a MESH in the MODL structure, update the MODL structure with that information.   Allows an edited .obj model to "update"  a .3do file. */

#include "objStructs.h"	
#include "modl.h"
#include "checkedMem.h"
#include "matScaler.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//for sqrt
#include <math.h>

//need to adjust the normals data such that it can be index by the
//vertex index, not by an individual index
//do this in readOBJ, not here

void setFaceDefaults(MESH *mesh, FACE *face)
{
    face->faceType = 0;	//all faces will be "normal"?? NOTE: 2 = translucent (needed for transparency in some faces??
    face->geometryMode = mesh->geometryMode;
    face->lightingMode = mesh->lightingMode;
    face->textureMode = mesh->textureMode;
    face->unknown1 = 0;
    face->hasTexture = 1;
    face->hasMaterial = 1;  //assuming any face we need to change will have texture and mat?
    float *a = face->unknown2;
    float *b = face->unknown3;
    float *c = face->faceNormal;   //dont think face normals are included in the .obj file maybe calculate from the vertex normals? 
    for(int i=0; i<3; i++) 
    {
	a[i] = b[i] = c[i] = 0;
    }
    face->extraLight = 0.0;
    face->materialIndex = 0;	//set to 0 here, and override later if a material is specified for this face

    return;
}

/* Quick function to set the normal for a given face (different from the vertex normals)
 *
 * Weird lighting effects on the model ingame, not sure if it ignores the vertex normals, so made
 * this function to see if it uses the face normal instead. */
void setFaceNormal(FACE *face, float x, float y, float z)
{   
    face->faceNormal[0] = x;
    face->faceNormal[1] = y;
    face->faceNormal[2] = z;

    return;
}

//update a MESH structure with the info from a GROUP structure
void updateMesh(MODL *model, MESH *mesh, GROUP *group, float meshOffset[3])
{
//update the vertice array
    
    //replace the mesh vertice array with the group vertice array
    mesh->numVertices = group->numVertices;
    free(mesh->vertices);   //we had allocated memory for this
    mesh->vertices = group->vertices;
    //sever the pointer from the GROUP structure (so the memory isnt interfered with)
    group->vertices = NULL;   
    
    //subtract the meshOffset from all vertice coordinates
    for(int i=0; i<mesh->numVertices; i++)
    {
	float *v = mesh->vertices[i];
	for(int j=0; j<3; j++)
	{
	    v[j] -= meshOffset[j];
	}
    }

    //update the texture vertice array
    mesh->numTexVertices = group->numTexVertices;
    free(mesh->texVertices);
    mesh->texVertices = group->texVertices;
    group->texVertices = NULL;
    
    //update the normals array
    //reallocate the array to the correct size
    mesh->normals = checked_realloc(mesh->normals, sizeof(vector3)*mesh->numVertices);
    //remember as we update each normal (while updating the FACE's)
    int *isUpdated = checked_calloc(mesh->numVertices, sizeof(int));

    //update the face array
    //free the old FACE structures
    for(int i=0; i<mesh->numFaces; i++)
    {
	freeFACE(mesh->faces[i]);
    }
    //update the size of the face array
    mesh->numFaces = group->numFaces;
    mesh->faces = checked_realloc(mesh->faces, sizeof(FACE *)*mesh->numFaces);
    //allocate new FACE structures
    for(int i=0; i<mesh->numFaces; i++)
    {
	mesh->faces[i] = createFACE();
	mesh->faces[i]->faceID = i; //guessing just face index
	//fill in some default values for most of it
	setFaceDefaults(mesh, mesh->faces[i]);
    }

    //UPDATE EACH FACE
    for(int i=0; i<mesh->numFaces; i++)
    {
	FACE *f = mesh->faces[i];
	OBJFACE *of = group->faces[i];

	f->numVertices = of->numVertices;
	f->vertexIndices = checked_malloc(sizeof(int)*f->numVertices);
	f->texVertexIndices = checked_malloc(sizeof(int)*f->numVertices);
	for(int j=0; j<f->numVertices; j++)
	{   
	    //NOTE: .obj indices start from 1, so must subtract 1 from each type of index
	    f->vertexIndices[j] = of->indices[j][0] - 1;
	    f->texVertexIndices[j] = of->indices[j][1] - 1;
	    

	    //normals not needed in each FACE but must update array in the outer MESH	   
	    int normalIndex = of->indices[j][2] - 1;
	    //if the normal for this vertex has not been updated yet
	    if(isUpdated[f->vertexIndices[j]] != 1)
	    {
		//destination and source of the normal data
		float *dn = mesh->normals[f->vertexIndices[j]];
		float *sn = group->normals[normalIndex]; 
		
		//float *sn = mesh->vertices[f->vertexIndices[j]];
		
		//copy it over
		for(int k=0; k<3; k++) dn[k] = sn[k];

		/*
		float x = sn[0];
		float y = sn[1];
		float z = sn[2];

		float mag = sqrt(x*x + y*y + z*z);
		x = x/mag;
		y = y/mag;
		z = z/mag;

		dn[0] = x;
		dn[1] = y;
		dn[2] = z;
		*/

		isUpdated[f->vertexIndices[j]] = 1;
	    }
	}

	/* Determine which material to use based on the name specified in the .obj file */
	if(f->hasMaterial != 0)
	{
	    //check for an exact matching name
	    int found = 0;
	    for(int i=0; i < model->numMaterials; i++)
	    {
		//if the .obj material name matches one in the MODL
		if(strcmp(model->materialNames[i], of->materialName) == 0)
		{
		    //use this material and stop searching
		    f->materialIndex = i;
		    found = 1;
		    break;

		}
	    }
	    //if we dont find a material with that exact name,
	    //look for one which has the significant part of it as a substring
	    //i.e if there is a texture gl_chest.mat and gl_chest appears in the material name use it 
	    if(found == 0)
	    {
		    char buffer[33];
		    for(int i=0; i < model->numMaterials; i++)
		    {
			//find the significant part of this material name
			strncpy(buffer, model->materialNames[i], 33);
			//locate the first fullstop in the name and terminate the string there
			char *dot = strchr(buffer, '.');
			if(dot != NULL)
			{
			    *dot = '\0';
			}
			
			//if the .obj material name has one of the materials as a substring
		    	if(strstr(of->materialName, buffer) != NULL)
			{
				f->materialIndex = i;
				found = 1;
				break;
			}
		    }
	    }

	    //if still not found, report taht we are using the deafult
	    if(found == 0)
	    {
	    	fprintf(stderr, "Could not find a corresponding material for %s\n", of->materialName);
	    }
	}
    }
    //FINISHED updating faces

    //in the above process the normals array should have been filled out as they were specified
    //just alert if this is not quite working
    for(int i=0; i<mesh->numVertices; i++)
    {
	//if this normal was never updated during the process
	if(isUpdated[i] != 1)
	{   
	    printf("%d\n", isUpdated[i]);
	    fprintf(stderr, "The normal for vertex %d was NOT updated in updateMESH()\n", i);
	}
    }
    //no longer need this
    free(isUpdated);
    
    //resize the extra light data and unknown2 arrays appropriately   
    mesh->lightData = checked_calloc(mesh->numVertices, sizeof(float));	//all 0.0 is default
    mesh->unknown2 = checked_calloc(mesh->numVertices, sizeof(int)); //dont know what this does, or even what type it should be, set to 0 with calloc and see what happens

    return;
}

//recursively step trhough node hierarchy, updating meshes
void updateMeshes(MODL *model, OBJ *obj, NODE *node, float parentOffset[3])
{
    //accumulate this node's offset to pass further on
    float nodeOffset[3];
    for(int i=0; i<3; i++) nodeOffset[i] = parentOffset[i] + node->position[i];
    //may not be needed, add the further mesh offset
    float meshOffset[3];
    for(int i=0; i<3; i++) meshOffset[i] = nodeOffset[i] + node->pivot[i];

    //if this has a mesh
    if(node->meshID != -1)
    {
	//if the OBJ structure has an equivalent, update the mesh
	MESH *mesh = model->meshes[node->meshID];
	int found = 0;
	for(int i=0; i<obj->numGroups; i++)
	{
	    GROUP *group = obj->groups[i];
	    //if the group name has the mesh name as a substring
	    if(strstr(group->groupName, mesh->meshName) != NULL)
	    {
		//update
		updateMesh(model, mesh, group, meshOffset);
		found = 1;
		//stop searching
		break;
	    }
	}
	if(found == 0) fprintf(stderr, "Found no group corresponding to %s\n", mesh->meshName);
    }

    //recurse on any child nodes (if it has any)
    if(node->hasChildren != 0)
    {
	//just recurse on the first child, which itself will recurse on any siblings (see below)
	updateMeshes(model, obj, model->nodes[node->childID], nodeOffset);
    }

    //recurse on any sibling nodes (if it has any)
    if(node->hasSibling != 0)
    {
	//siblings share the same offset of their parents
	updateMeshes(model, obj, model->nodes[node->siblingID], parentOffset);
    }

    return;
}


void update3do(MODL *model, OBJ *obj)
{
    if(model == NULL)
    {
	fprintf(stderr, "update3do() passed NULL MODL structure\n");
	return;
    }
    if(obj == NULL)
    {
	fprintf(stderr, "update3do() passed NULL OBJ structure\n");
	return;
    }

    //need to recursively step through the node hierarchy, maintaining an
    //offset to subtract from each vertices
    //(reverse of writeObj)
    if(model->numNodes != 0)
    {
	float startingOffset[3] = {0.0, 0.0, 0.0};
	//start it off with the first node
	updateMeshes(model, obj, model->nodes[0], startingOffset);
    }

    //scale all the textures back from the .obj specification
    scaleTexVerts(model, 1); 

    return;
}
