/* Given a MODL structure (defined in modl.h) write it's contents to a binary .3do file as required for the game Grim Fandango. */

#include "modl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Delegates to fwrite, checks for failure, reports and terminates*/
size_t checked_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t numWritten = fwrite(ptr, size, nmemb, stream);
	if( numWritten != nmemb )
	{
		fprintf(stderr, "fwrite() failed to write all bytes.\n");
		exit(EXIT_FAILURE);
	}

	//pass on the return value
	return numWritten;
}

/* Write an integer to a filestream. */
void writeInt( int value, FILE *stream )
{
	checked_fwrite(&value, 4, 1, stream);
}

/* Write a float to a filestream. */
void writeFloat( float value, FILE *stream )
{
	checked_fwrite(&value, 4, 1, stream);
}

/* Write a string to a file stream, as a block of 32 characters, remainder padded with 0's */
void writeString32( char *string, FILE *stream )
{
	int length = strlen(string);
	if( length > 32 )
	{
		fprintf(stderr, "writeString32() passed %d chars\n", length);
		exit(EXIT_FAILURE);
	}
	//write the string
	checked_fwrite(string, 1, length, stream);
	//write 0's for remainder of the block
	int padSize = 32 - length;
	char zero = '\0';	
	for(int i=0; i< padSize; i++)
	{
		checked_fwrite(&zero, 1, 1, stream);
	}

	return;
}

/* Write a string toa file stream as a block of 64 characters, string + null terminator + remainder padded with 0xCC */
void writeString64( char *string, FILE *stream )
{
	int length = strlen(string);
	if( length > 64 )
	{
		fprintf(stderr, "writeString64() passed %d chars\n", length);
		exit(EXIT_FAILURE);
	}
	//write the string WITH the 0 terminator
	checked_fwrite(string, 1, length+1, stream);
	//write 0xCC for remainder of block
	int padSize = 64 - (length + 1);
	unsigned char pad = 0xCC;	//without unsigned was overflowing
	for(int i=0; i < padSize; i++)
	{
		checked_fwrite(&pad, 1, 1, stream);
	}

	return;
}

/* Write a FACE structure to a file stream (.3do file) */
void writeFace( FACE *face, FILE *ofp )
{
	writeInt(face->faceID, ofp);
	writeInt(face->faceType, ofp);
	writeInt(face->geometryMode, ofp);
	writeInt(face->lightingMode, ofp);
	writeInt(face->textureMode, ofp);
	writeInt(face->numVertices, ofp);
	writeInt(face->unknown1, ofp);
	writeInt(face->hasTexture, ofp);
	writeInt(face->hasMaterial, ofp);
	checked_fwrite(face->unknown2, 4, 3, ofp);
	writeFloat(face->extraLight, ofp);
	checked_fwrite(face->unknown3, 4, 3, ofp);
	checked_fwrite(face->faceNormal, 4, 3, ofp);
	
	//write the mesh vertex indices
	for(int i=0; i < face->numVertices; i++)
	{
		writeInt(face->vertexIndices[i], ofp);
	}

	//write the texture vertexIndices (only if <hasTexture>)
	if( face->hasTexture != 0 )
	{
		for(int i=0; i < face->numVertices; i++)
		{
			writeInt(face->texVertexIndices[i], ofp);
		}
	}

	//write the materialIndex only if <hasMaterial>
	if( face->hasMaterial != 0 )
	{
		writeInt(face->materialIndex, ofp);
	}

	return;
}

/* Write a MESH structure to a file stream (.3do file) */
void writeMesh( MESH *mesh, FILE *ofp )
{
	writeString32(mesh->meshName, ofp);
	writeInt(mesh->unknown1, ofp);
	writeInt(mesh->geometryMode, ofp);
	writeInt(mesh->lightingMode, ofp);
	writeInt(mesh->textureMode, ofp);
	writeInt(mesh->numVertices, ofp);
	writeInt(mesh->numTexVertices, ofp);
	writeInt(mesh->numFaces, ofp);

	//write the mesh vertex data, (an array of vector3's)
	for(int i=0; i < mesh->numVertices; i++)
	{
		checked_fwrite(mesh->vertices[i], 4, 3, ofp);	
	}
	//write the texture vertex data (an array of vector2's)
	for(int i=0; i < mesh->numTexVertices; i++)
	{
		checked_fwrite(mesh->texVertices[i], 4, 2, ofp);
	}
	//write the extra light data (an array of floats)
	for(int i=0; i < mesh->numVertices; i++)
	{
		writeFloat(mesh->lightData[i], ofp);
	}	
	//write unknown2 data, 4 bytes per vertice
	for(int i=0; i < mesh->numVertices; i++)
	{
		writeInt(mesh->unknown2[i], ofp);
	}
	//write the face data
	for(int i=0; i < mesh->numFaces; i++)
	{
		writeFace(mesh->faces[i], ofp);
	}
	//write the vertex normals
	for(int i=0; i < mesh->numVertices; i++)
	{
		checked_fwrite(mesh->normals[i], 4, 3, ofp);
	}

	writeInt(mesh->hasShadow, ofp);
	writeInt(mesh->unknown3, ofp);
	writeFloat(mesh->meshRadius, ofp);
	checked_fwrite(mesh->unknown4, 4, 3, ofp);
	checked_fwrite(mesh->unknown5, 4, 3, ofp);

	return;
}

/* Write a NODE structure to a file stream (.3do file ) */
void writeNode( NODE *node, FILE *ofp )
{
	//write the name as a block of 64 chars
	writeString64(node->name, ofp);
	writeInt(node->flags, ofp);
	writeInt(node->unknown1, ofp);
	writeInt(node->type, ofp);
	writeInt(node->meshID, ofp);
	writeInt(node->depth, ofp);
	writeInt(node->hasParent, ofp);
	writeInt(node->numChildren, ofp);
	writeInt(node->hasChildren, ofp);
	writeInt(node->hasSibling, ofp);
	checked_fwrite(node->pivot, 4, 3, ofp);
	checked_fwrite(node->position, 4, 3, ofp);
	writeFloat(node->pitch, ofp);
	writeFloat(node->yaw, ofp);
	writeFloat(node->roll, ofp);
	checked_fwrite(node->unknown2, 4, 12, ofp);
	if(node->hasParent != 0)
		writeInt(node->parentID, ofp);
	if(node->hasChildren != 0)
		writeInt(node->childID, ofp);
	if(node->hasSibling != 0)
		writeInt(node->siblingID, ofp);

	return;
}

/* Write a MODL structure as a binary .3do file with name <filename> */
void write3do( MODL *model, char *filename )
{
	//open the file for writing, check success
	FILE *ofp = fopen(filename, "wb");
	if( ofp == NULL )
	{
		fprintf(stderr, "Could not open %s for writing.\n", filename);
		exit(EXIT_FAILURE);
	}

	/* BEGIN WRITING THE FILE */	

	/* HEADER SECTION*/

	checked_fwrite(model->fourcc, 1, 4, ofp);
	writeInt(model->numMaterials, ofp);
	//write each material name as a block of 32 chars
	for(int i=0; i < model->numMaterials; i++)
	{
		writeString32(model->materialNames[i], ofp);
	}
	writeString32(model->modelName, ofp);

	/* GEOSET SECTION */

	writeInt(model->unknown1, ofp);
	writeInt(model->numGeosets, ofp);
	writeInt(model->numMeshes, ofp);
	//write the meshes
	for(int i=0; i < model->numMeshes; i++)
	{
		writeMesh(model->meshes[i], ofp);	
	}

	/* NODES SECTION */
	
	writeInt(model->unknown2, ofp);
	writeInt(model->numNodes, ofp);
	//write the nodes
	for(int i=0; i < model->numNodes; i++)
	{
		writeNode(model->nodes[i], ofp);
	}

	/* FOOTER SECTION */

	writeFloat(model->modelRadius, ofp);
	checked_fwrite(model->insertionOffset, 4, 3, ofp);
	checked_fwrite(model->unknown3, 4, 3, ofp);
	checked_fwrite(model->unknown4, 4, 6, ofp);

	//close the file pointer (forgot this :/)
	//ideally also check that it closed correctly
	fclose(ofp);

	return;
}
