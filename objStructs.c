/* Create and free the structures define in objStructs.h */
#include "objStructs.h"
#include "checkedMem.h"

//the size to begin the array's in these structures
#define INIT_ARR_SIZE 16;


OBJ *createOBJ()
{
    OBJ *obj = checked_malloc(sizeof(OBJ));
    obj->numGroups = 0;
    obj->groupSize = INIT_ARR_SIZE;
    obj->groups = checked_malloc(sizeof(GROUP *) * obj->groupSize);

    return obj;	
}

void growGroups(OBJ *obj)
{
    //double the space of the groups array
    obj->groupSize *= 2;
    obj->groups = checked_realloc(obj->groups, sizeof(GROUP *) * obj->groupSize);
    return;
}

GROUP *createGROUP()
{
    GROUP *group = checked_malloc(sizeof(GROUP));
	
    group->groupName = NULL;
    group->numVertices = 0;
    group->numTexVertices = 0;
    group->numNormals = 0;
    group->numFaces = 0;
    group->vertSize = INIT_ARR_SIZE;
    group->texVertSize = INIT_ARR_SIZE;
    group->normSize = INIT_ARR_SIZE;
    group->faceSize = INIT_ARR_SIZE;

    group->vertices = checked_malloc(sizeof(vector3) * group->vertSize);
    group->texVertices = checked_malloc(sizeof(vector2) * group->texVertSize);
    group->normals = checked_malloc(sizeof(vector3) * group->normSize);
    group->faces = checked_malloc(sizeof(OBJFACE *) * group->faceSize);
    return group;
}

void growVertices(GROUP *group)
{
    //double the space of the vertices array
    group->vertSize *= 2;
    group->vertices = checked_realloc(group->vertices, sizeof(vector3) * group->vertSize);
    return;
}

void growTexVertices(GROUP *group)
{
    group->texVertSize *= 2;
    group->texVertices = checked_realloc(group->texVertices, sizeof(vector2) * group->texVertSize);
    return;
}

void growNormals(GROUP *group)
{
    group->normSize *= 2;
    group->normals = checked_realloc(group->normals, sizeof(vector3) * group->normSize);
    return;
}

void growFaces(GROUP *group)
{
    group->faceSize *= 2;
    group->faces = checked_realloc(group->faces, sizeof(OBJFACE *) * group->faceSize);
    return;
}

OBJFACE *createOBJFACE()
{
    OBJFACE *objface = checked_malloc(sizeof(OBJFACE));
    objface->numVertices = 0;
    objface->indices = checked_malloc(sizeof(indexTriplet) * MAX_VERTS_PER_FACE);

    return objface;
}



