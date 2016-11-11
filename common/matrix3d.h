#ifndef _MATRIX3D_H
#define _MATRIX3D_H

#include <poly.h>

typedef struct {
  double Matrix [ 4 ] [ 4 ];
  double RMatrix [ 4 ] [ 4 ];
  int Xr, Zr, Yr;
  double XTrans, YTrans, ZTrans;
} Matrix3D;

void initMath (void);
void InitMat(double Matrix[4][4]);
void InitMatrix3D(Matrix3D *p);
void MatrixPrint(Matrix3D *m);
void MergeMatrix (Matrix3D *m, double NewMatrix [ 4 ] [ 4 ] );
void MergeMatrices ( double Dest [ 4 ] [ 4 ], double Source [ 4 ] [ 4 ] );
void Rotate (Matrix3D *m, float Xa, float Ya, float Za );
void Translate (Matrix3D *m, double Xt, double Yt, double Zt );
void Scale (Matrix3D *m, double Xs, double Ys, double Zs );
void Shear ( Matrix3D *m,double Xs, double Ys );
void Transform (Matrix3D *m, Point *V );
void TransformQuad(Matrix3D *m,Quad *p);
void transformObject(Matrix3D *m,Point verts[],long v,Quad faces[],long f);

#endif
