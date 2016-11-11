#include <stdio.h>

#include <cube.h>
#include <poly.h>
#include <matrix3d.h>

// a=height,width  d=depth (z direction)
long getCubeV()
{
 return 8;
}

long getCubeF()
{
 return 6;
}

static void initgenCube(Point* vert,long *v,Quad faces[],long *f,long a,long d,Matrix3D *m, int dir);
void initCube(Point* vert,long *v,Quad faces[],long *f,long a,long d,Matrix3D *m)
{
 initgenCube(vert,v,faces,f,a,d,m,0);
}

void initRoom(Point* vert,long *v,Quad faces[],long *f,long a,long d,Matrix3D *m)
{
	initgenCube(vert,v,faces,f,a,d,m,1);
}

// a general cube - inward facing (room) or outward facing (cube)
static void initgenCube(Point* vert,long *v,Quad faces[],long *f,long a,long d,Matrix3D *m, int dir)
// create a cube centered at the origin
{

	float aby2,dby2;
	float x=0,y=0,z=0;         // create cube at origin

	long i;
	long nv=0,nf=0;
	aby2=(float)a/2.;
        dby2=(float)d/2.;

	if (v) {nv=*v;*v+=8;}
	if (f) {nf=*f;*f+=6;}

	initPoint(&vert[nv+0],x-aby2,  y+aby2,  z+dby2  );
	initPoint(&vert[nv+1],x+aby2,  y+aby2,  z+dby2  );
	initPoint(&vert[nv+2],x+aby2,  y+aby2,  z-dby2  );
	initPoint(&vert[nv+3],x-aby2,  y+aby2,  z-dby2  );
	initPoint(&vert[nv+4],x-aby2,  y-aby2,  z+dby2  );
	initPoint(&vert[nv+5],x+aby2,  y-aby2,  z+dby2  );
	initPoint(&vert[nv+6],x+aby2,  y-aby2,  z-dby2  );
	initPoint(&vert[nv+7],x-aby2,  y-aby2,  z-dby2  );

// Note the faces should be assigned vertices in a clock wise
// fashion in order to preserve their direction. (normal to the face)

if (dir==0) {
	assignVert(&faces[nf+0],vert, nv+0,nv+1,nv+2,nv+3);
	assignVert(&faces[nf+1],vert, nv+0,nv+3,nv+7,nv+4);
	assignVert(&faces[nf+2],vert, nv+3,nv+2,nv+6,nv+7);
	assignVert(&faces[nf+3],vert, nv+2,nv+1,nv+5,nv+6);
	assignVert(&faces[nf+4],vert, nv+0,nv+4,nv+5,nv+1);
	assignVert(&faces[nf+5],vert, nv+4,nv+7,nv+6,nv+5);
} else {
	assignVert(&faces[nf+0],vert, nv+0,nv+3,nv+2,nv+1);
	assignVert(&faces[nf+1],vert, nv+0,nv+4,nv+7,nv+3);
	assignVert(&faces[nf+2],vert, nv+3,nv+7,nv+6,nv+2);
	assignVert(&faces[nf+3],vert, nv+2,nv+6,nv+5,nv+1);
	assignVert(&faces[nf+4],vert, nv+0,nv+1,nv+5,nv+4);
	assignVert(&faces[nf+5],vert, nv+4,nv+5,nv+6,nv+7);
}

	if (m)
		transformObject(m,vert+nv,8,faces+nf,6);

}
