#ifndef _CUBE_H
#define _CUBE_H
#include "poly.h"
#include "matrix3d.h"
/* a - height, width
   d - depth
*/

long getCubeV();
long getCubeF();
void initCube(Point verts[],long *v,Quad faces[],long *f,long a,long d,Matrix3D *m);
void initRoom(Point* vert,long *v,Quad faces[],long *f,long a,long d,Matrix3D *m);
#endif
