/* Multiple files need access to these typedefs, and there are conflicts with redefinitions. */

//to stop these being redefined, more trouble having these than they're worth
#ifndef DUMBVECS
typedef float vector3[3];
typedef float vector2[2];
#define DUMBVECS
#endif

