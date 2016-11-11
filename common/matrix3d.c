#include <matrix3d.h>
#include <esdefs.h>
#include <poly.h>

#include <math.h>

#define DEGREECOUNT 1024

#define COS(a) cos(a) //( CosTable [a & (DEGREECOUNT - 1)] )
#define SIN(a) sin(a) //( SinTable [a & (DEGREECOUNT - 1)] )

float CosTable [ DEGREECOUNT ];
float SinTable [ DEGREECOUNT ];


void initMath (void)
   {
   long double unit = ( long double ) ( PI * 2.0F ) /
		      ( long double ) DEGREECOUNT;
   long double degree;
   unsigned short n;

   for ( n = 0; n < DEGREECOUNT; n++ )
       {
		degree = ( long double ) n;
		CosTable [n] = (float) cos ( unit * degree ) ;
		SinTable [n] = (float) sin ( unit * degree ) ;
       }
   }

void InitMat(double Matrix[4][4])
   {
   Matrix[0][0] = 1;  Matrix[0][1] = 0;  Matrix[0][2] = 0;  Matrix[0][3] = 0;
   Matrix[1][0] = 0;  Matrix[1][1] = 1;  Matrix[1][2] = 0;  Matrix[1][3] = 0;
   Matrix[2][0] = 0;  Matrix[2][1] = 0;  Matrix[2][2] = 1;  Matrix[2][3] = 0;
   Matrix[3][0] = 0;  Matrix[3][1] = 0;  Matrix[3][2] = 0;  Matrix[3][3] = 1;
   }

void InitMatrix3D(Matrix3D *p)
{
	InitMat(p->Matrix);
}


void MatrixPrint(Matrix3D *m)
{
		  int i,j;
		  for (i=0;i<4;i++) {
			 for (j=0;j<4;j++) 
						printf("%8.4f ",m->Matrix[i][j]);
			 printf("\n");
		  }
		  printf("\n");
}

void MergeMatrix (Matrix3D *m, double NewMatrix [ 4 ] [ 4 ] )
   {
   // Multiply NewMatirx by Matrix; store result in TempMatrix
   short unsigned int i,j;
   double TempMatrix [ 4 ] [ 4 ];
	for ( i = 0; i < 4; i++)
		 for ( j = 0; j < 4; j++)
			  TempMatrix[i][j] = (m->Matrix[i][0] * NewMatrix[0][j])
			    + (m->Matrix[i][1] * NewMatrix[1][j])
			    + (m->Matrix[i][2] * NewMatrix[2][j])
			    + (m->Matrix[i][3] * NewMatrix[3][j]);
   // Copy TempMatrix to Matrix:
   for (i = 0; i < 4; i++)
       {
       m->Matrix[i][0] = TempMatrix[i][0];
       m->Matrix[i][1] = TempMatrix[i][1];
       m->Matrix[i][2] = TempMatrix[i][2];
       m->Matrix[i][3] = TempMatrix[i][3];
       }
   }

void MergeMatrices ( double Dest [ 4 ] [ 4 ], double Source [ 4 ] [ 4 ] )
   {
   // Multiply Source by Dest; store result in Temp:
   short unsigned int i,j;
   double Temp [ 4 ] [ 4 ];
	for (  i = 0; i < 4; i++ )
		 for (  j = 0; j < 4; j++ )
	   {
			  Temp [ i ] [ j ] = ( Source [ i ] [ 0 ] * Dest [ 0 ] [ j ] )
			    + ( Source [ i ] [ 1 ] * Dest [ 1 ] [ j ] )
			    + ( Source [ i ] [ 2 ] * Dest [ 2 ] [ j ] )
			    + ( Source [ i ] [ 3 ] * Dest [ 3 ] [ j ] );
	   }
   // Copy Temp to Dest:
   for (i = 0; i < 4; i++)
       {
       Dest [ i ] [ 0 ] = Temp [ i ] [ 0 ];
       Dest [ i ] [ 1 ] = Temp [ i ] [ 1 ];
       Dest [ i ] [ 2 ] = Temp [ i ] [ 2 ];
       Dest [ i ] [ 3 ] = Temp [ i ] [ 3 ];
       }
   }

void  Rotate (Matrix3D *m, float Xa, float Ya, float Za )
   {
   // Generate 3D rotation matrix:
   double Rmat [ 4 ] [ 4 ];
   double Xth=Xa,Yth=Ya,Zth=Za;

   Xth = (double)Xa*PIby180; Yth = (double)Ya*PIby180; Zth = (double)Za*PIby180;

   m->Xr = (int)Xth; m->Yr =(int) Yth; m->Zr = (int)Zth;
   InitMat ( m->RMatrix );

   // Initialize Z rotation matrix - Note: we perform Z
   // rotation first to align the 3D Z axis with the 2D Z axis.
   Rmat[0][0]=COS(Zth);  Rmat[0][1]=SIN(Zth);  Rmat[0][2]=0;    Rmat[0][3]=0;
   Rmat[1][0]=-SIN(Zth); Rmat[1][1]=COS(Zth);  Rmat[1][2]=0;    Rmat[1][3]=0;
   Rmat[2][0]=0;        Rmat[2][1]=0;        Rmat[2][2]=1;    Rmat[2][3]=0;
   Rmat[3][0]=0;        Rmat[3][1]=0;        Rmat[3][2]=0;    Rmat[3][3]=1;

   // Merge matrix with master matrix:
   MergeMatrices ( m->RMatrix, Rmat );

   // Initialize X rotation matrix:
   Rmat[0][0]=1;  Rmat[0][1]=0;        Rmat[0][2]=0;       Rmat[0][3]=0;
   Rmat[1][0]=0;  Rmat[1][1]=COS(Xth);  Rmat[1][2]=SIN(Xth); Rmat[1][3]=0;
   Rmat[2][0]=0;  Rmat[2][1]=-SIN(Xth); Rmat[2][2]=COS(Xth); Rmat[2][3]=0;
   Rmat[3][0]=0;  Rmat[3][1]=0;        Rmat[3][2]=0;       Rmat[3][3]=1;

   // Merge matrix with master matrix:
   MergeMatrices ( m->RMatrix, Rmat );

   // Initialize Y rotation matrix:
   Rmat[0][0]=COS(Yth); Rmat[0][1]=0;   Rmat[0][2]=-SIN(Yth); Rmat[0][3]=0;
   Rmat[1][0]=0;       Rmat[1][1]=1;   Rmat[1][2]=0;        Rmat[1][3]=0;
   Rmat[2][0]=SIN(Yth); Rmat[2][1]=0;   Rmat[2][2]=COS(Yth);  Rmat[2][3]=0;
   Rmat[3][0]=0;       Rmat[3][1]=0;   Rmat[3][2]=0;        Rmat[3][3]=1;

   // Merge matrix with master matrix:
   MergeMatrices ( m->RMatrix, Rmat );

   MergeMatrix (m, m->RMatrix );
   }

void Translate (Matrix3D *m, double Xt, double Yt, double Zt )
   {
   // Create 3D translation matrix:

   double Tmat [ 4 ] [ 4 ];

   // Save translation values:
//   Xt=Xt*BHEIGHT/SBHEIGHT; Yt=Yt*BHEIGHT/SBHEIGHT;

   m->XTrans = Xt; m->YTrans = Yt; m->ZTrans = Zt;

   // Initialize translation matrix:
   Tmat[0][0]=1;  Tmat[0][1]=0;  Tmat[0][2]=0;  Tmat[0][3]=0;
   Tmat[1][0]=0;  Tmat[1][1]=1;  Tmat[1][2]=0;  Tmat[1][3]=0;
   Tmat[2][0]=0;  Tmat[2][1]=0;  Tmat[2][2]=1;  Tmat[2][3]=0;
   Tmat[3][0]=Xt; Tmat[3][1]=Yt; Tmat[3][2]=Zt; Tmat[3][3]=1;

   // Merge matrix with master matrix:
   MergeMatrix (m,Tmat );
   }

void  Scale (Matrix3D *m, double Xs, double Ys, double Zs )
   {
   // Create 3D scaling matrix:
   double Smat [ 4 ] [ 4 ];
//   Xs=Xs*BHEIGHT/SBHEIGHT; Ys=Ys*BHEIGHT/SBHEIGHT; Zs=Zs*BHEIGHT/SBHEIGHT;
   // Initialize scaling matrix:
   Smat[0][0] = Xs; Smat[0][1] = 0;  Smat[0][2] = 0;  Smat[0][3] = 0;
   Smat[1][0] = 0;  Smat[1][1] = Ys; Smat[1][2] = 0;  Smat[1][3] = 0;
   Smat[2][0] = 0;  Smat[2][1] = 0;  Smat[2][2] = Zs; Smat[2][3] = 0;
   Smat[3][0] = 0;  Smat[3][1] = 0;  Smat[3][2] = 0;  Smat[3][3] = 1;

   // Merge matrix with master matrix:
   MergeMatrix (m, Smat );
   }

void  Shear ( Matrix3D *m,double Xs, double Ys )
  {
   // Create 3D shearing matrix:

   double Smat [ 4 ] [ 4 ];
//   Xs=Xs*BHEIGHT/SBHEIGHT; Ys=Ys*BHEIGHT/SBHEIGHT;
   // Initialize shearing matrix:
   Smat[0][0] = 1;  Smat[0][1] = 0;  Smat[0][2] = Xs;  Smat[0][3] = 0;
   Smat[1][0] = 0;  Smat[1][1] = 1;  Smat[1][2] = Ys;  Smat[1][3] = 0;
   Smat[2][0] = 0;  Smat[2][1] = 0;  Smat[2][2] = 1;   Smat[2][3] = 0;
   Smat[3][0] = 0;  Smat[3][1] = 0;  Smat[3][2] = 0;   Smat[3][3] = 1;

   // Merge matrix with master matrix:
   MergeMatrix (m,Smat );
   }

// Function designed to transform a vertex using the master
// matrix:
void Transform (Matrix3D* m, Point *V )
   {
   // Initialize temporary variables:
   double Lx = V->x;
   double Ly = V->y;
   double Lz = V->z;

   // Transform vertex by master matrix:
   V->x = ( (   Lx * m->Matrix [ 0 ][ 0 ]) )
	  + ( ( Ly * m->Matrix [ 1 ][ 0 ]) )
	  + ( ( Lz * m->Matrix [ 2 ][ 0 ]) )
	  + m->Matrix [ 3 ][ 0 ];

   V->y = (   ( Lx * m->Matrix [ 0 ][ 1 ]) )
	  + ( ( Ly * m->Matrix [ 1 ][ 1 ]) )
	  + ( ( Lz * m->Matrix [ 2 ][ 1 ]) )
	  + m->Matrix [ 3 ][ 1 ];

   V->z = (   ( Lx * m->Matrix [ 0 ][ 2 ]) )
	  + ( ( Ly * m->Matrix [ 1 ][ 2 ]) )
	  + ( ( Lz * m->Matrix [ 2 ][ 2 ]) )
	  + m->Matrix [ 3 ][ 2 ];
   }

void TransformQuad(Matrix3D *m,Quad * p)
{
  int i;
  for (i=0;i<4;i++) Transform(m,p->p[i]);
  Transform(m,&p->normal);
}

