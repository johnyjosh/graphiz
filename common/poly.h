#ifndef _POLY_H
#define _POLY_H

typedef struct {
	float x,y,z;
} Point;

typedef struct {
	float r,g,b,a;
} Color;

typedef Point Vector;

typedef struct {
	Point* p[4];
	long v[4];
	Vector normal;
	long tid;			// set to the texture id to be used to draw this quad
	float	tex_factor_x;  // set to the factor to be multiplied to the tex coordinates.
	float tex_factor_y; // So if tex_factor_x is 2, the texture is tiled into 2 parts on this face.
} Quad;

void initColor(Color *p,float r,float g,float b);
void initPoint(Point *,float,float,float);
void assignVert(Quad* f,Point* vert,long A,long B,long C,long D);
void calcNormal(Quad *p);  // calc normal

#endif
