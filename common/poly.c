#include <poly.h>
#include <matrix3d.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>


void initPoint(Point *p,float x,float y,float z)
{
	p->x=x;
	p->y=y;
	p->z=z;
}

void initColor(Color *p,float r,float g,float b)
{
	p->r=r;
	p->g=g;
	p->b=b;
}
void calcNormal(Quad *p)  // calc normal
{
	float x1,y1,z1,x2,y2,z2,x3,y3,z3;
	float a,b,c;
	double dist;
	int i=0;


			  x1=p->p[i]->x;
			  y1=p->p[i]->y;
			  z1=p->p[i]->z;
			  x2=p->p[i+1]->x;
			  y2=p->p[i+1]->y;
			  z2=p->p[i+1]->z;
			  x3=p->p[i+2]->x;
			  y3=p->p[i+2]->y;
			  z3=p->p[i+2]->z;

			  a=y1*(z2-z3) + y2*(z3-z1) + y3*(z1-z2);
			  b=z1*(x2-x3) + z2*(x3-x1) + z3*(x1-x2);
			  c=x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2);

			  dist= sqrt ( a*a + b*b + c*c );
			  if (dist!=0) {
						 initPoint(&(p->normal),a/dist,b/dist,c/dist);
						 return;
			  }
	fprintf(stderr,"This is not a polygon. Cannot calculate normal.\n");
	exit(0);
}

void assignVert(Quad* f,Point* vert,long A,long B,long C,long D)
{
   f->p[0]=&vert[A];
   f->p[1]=&vert[B];
   f->p[2]=&vert[C];
   f->p[3]=&vert[D];
   
   f->v[0]=A;
   f->v[1]=B;
   f->v[2]=C;
   f->v[3]=D;

   calcNormal(f);
}

void transformObject(Matrix3D *m,Point verts[],long v,Quad faces[],long f)
{
		long i;
		for (i=0;i<v;i++)  {
			Transform(m,&verts[i]);
		}
		for (i=0;i<f;i++) {
			Transform(m,&faces[i].normal);
		}
}
