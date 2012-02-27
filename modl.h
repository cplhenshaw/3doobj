/* GOAL: To read in the .3do model files associated with Grim Fandango in their entirety, and be able to write back to that format, allowing editing of the models. As some of the data's purpose is unknown it must be read in from the original file (not discarded) so that it can be written back after any changes have been made.

The writeup of the model format at wiki.multimedia.cx/index.php?title=MODL was used. */
#include <stddef.h> //lets us use size_t in this header (also why not needed in other files that include this. bad?)

//shitstorm of redefintion bvullshut
#include "vector.h"

/* A structure to hold each FACE in the .3do file */
typedef struct 
{

	//face id
	int faceID;
	//face type
	int faceType;
	//geometry mode
	int geometryMode;
	//lighting mode
	int lightingMode;
	//texture mode
	int textureMode;
	//number of vertices
	int numVertices;
	//unknown 4 bytes
	int unknown1;
	//has texture?
	int hasTexture;
	//has material?
	int hasMaterial;
	//unknown vector3
	vector3 unknown2;
	//extra light
	float extraLight;
	//unknown vector3
	vector3 unknown3;
	//face normal
	vector3 faceNormal;
	//mesh vertex indices, an array of int's with size <numVertices> 
	int *vertexIndices;
	//texture vertex indices, an array with size <numVertices>
	int *texVertexIndices;	//only if <hasTexture>
	//material index
	int materialIndex;	//only if <hasMaterial>
} FACE;



/* A structure to hold each MESH in the .3do file. */
typedef struct 
{

	//name of the mesh, 32 byte string
	char *meshName;
	//4 bytes
	int unknown1;
	//geometry mode, 4 bytes
	int geometryMode;
	//lighting mode
	int lightingMode;
	//texture mode
	int textureMode;
	//number of mesh vertices
	int numVertices;
	//number of texture vertices
	int numTexVertices;
	//number of faces
	int numFaces;
	//mesh vertex data, an array of vector3's with size <numVertices>
	vector3 *vertices;
	//texture vertex data, an array of vector2's with size <numTexVertices>
	vector2 *texVertices;
	//extra light data, an array of floats, size <numVertices>
	float *lightData;
	//unknown, 4 bytes per vertice, i.e array of size <numVertices>
	int *unknown2;
	//face data, an array of pointers to FACE structures, size <numFaces>
	FACE **faces;
	//vertex normals, an array of vector3's with size <numVertices>
	vector3 *normals;
	//has shadow
	int hasShadow;
	//unknown 4 bytes
	int unknown3;
	//mesh radius, should be float??
	float meshRadius;
	//unknown vector3
	vector3 unknown4;
	//unknown vector3
	vector3 unknown5;

} MESH;





/* A structure to hold each NODE in the .3do file. */
typedef struct 
{

	//name of the node, a 64 byte string
	char *name;
	//flags, 4 bytes
	int flags;
	//uknown 4 bytes
	int unknown1;
	//type
	int type;
	//mesh id
	int meshID;
	//depth
	int depth;
	//has parent?
	int hasParent;
	//number of children
	int numChildren;
	//has children?
	int hasChildren;
	//has sibling?
	int hasSibling;
	//pivot, the point about which this node rotates?
	vector3 pivot;
	//position of this node
	vector3 position;
	//pitch of this node
	float pitch;
	//yaw of this node
	float yaw;
	//roll of this node
	float roll;
	//unknown 48 bytes
	int unknown2[12];
	//parentid
	int parentID;
	//childID;
	int childID;
	//siblingID
	int siblingID;

} NODE;

/* The main structure which the entire .3do file will be read into. */
typedef struct 
{

	/* HEADER */

	//4 character format code
	char fourcc[4];
	//number of materials in this model.
	int numMaterials;
	//an array of 32 byte strings, each holding the name of a material.
	char **materialNames;
	//name of the model, 32 byte string
	char *modelName;

	/* GEOSET */
	
	//4 bytes
	int unknown1;
	//number of geosets
	int numGeosets;
	//number of meshes in this geoset
	int numMeshes;
	//an array of pointers to MESH structures
	MESH **meshes;

	/* NODES */

	//4 bytes
	int unknown2;
	//number of nodes in the model
	int numNodes;
	//an array of pointers to NODE structures.
	NODE **nodes;

	/* FOOTER */
	
	//model radius
	float modelRadius;
	//insertion offset
	vector3 insertionOffset;
	//appears to be 36 more bytes unaccounted for by the writeup mentioned in 
	//the top comments
	//looks like it could be another vector3 or 3 floats
	vector3 unknown3;
	//that leaves 24 bytes of stuff? just store them in here
	int unknown4[6];

} MODL; 

/* The "createXXXX()" functions need to be used outside of modl.c, but freeMODL() itself calls the other free functions, and so only freeMODL() needs to be used outside of modl.c */

/*Allocates memory for a new MODL structure and sets pointer fields to NULL */
MODL *createMODL();

/*Allocates memory for a new MESH structure and set pointer fields to NULL */
MESH *createMESH();

/*Allocate memory for a new FACE structure and set pointer fields to NULL */
FACE *createFACE();

/*Allocate memory for a new NODE structure and set pointer fields to NULL */
NODE *createNODE();

/*Free the memory associated with a MODL structure.*/
void freeMODL( MODL *model );

/* Following not needed by "client" as are called themselves during freeMODL */

/*Free the memory associated with a MESH structure.*/
//void freeMESH( MESH *mesh );

/*Free the memory associated with a FACE structure.*/
//this one is needed in update3do.c
void freeFACE( FACE *face );

/*Free the memory associated with a NODE structure.*/
//void freeNODE( NODE *node );


