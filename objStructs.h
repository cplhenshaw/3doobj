//the most number of vertices we will see in a single face
#define MAX_VERTS_PER_FACE 8

#include "vector.h"
typedef int indexTriplet[3];

typedef struct
{
    int numVertices;
    indexTriplet *indices;
	//need to add a material index of some sort here, and update it while read in the .obj file	
    char *materialName; 
} OBJFACE;


typedef struct
{
    //name of the vertice group
    char *groupName;
    //actual number of vertices in the array
    int numVertices;
    //number of vertices we have allocated space for
    int vertSize;
    //array of vertices
    vector3 *vertices;
    //actual number of texture vertices in the array
    int numTexVertices;
    //number of texture vertices we have allocated space for
    int texVertSize;
    //the array of texture vertices
    vector2 *texVertices;
    //actual number of normals in the array
    int numNormals;
    //number of normals we have allocated space for
    int normSize;
    //array of vertex normal
    vector3 *normals;
    //actual number of faces in the array
    int numFaces;
    //number of faces we have allocate space for
    int faceSize;
    //array of OBJFACE structures
    OBJFACE **faces;

    
} GROUP;


typedef struct
{
    int numGroups;
    //number of groups we have allocated space for
    int groupSize;
    GROUP **groups;
} OBJ;

OBJ *createOBJ();
void growGroups(OBJ *obj);

GROUP *createGROUP();
void growVertices(GROUP *group);
void growTexVertices(GROUP *group);
void growNormals(GROUP *group);
void growFaces(GROUP *group);

OBJFACE *createOBJFACE();

