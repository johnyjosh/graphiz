#ifndef __ESDEFS_H__
#define __ESDEFS_H__

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FALSE  0
#define TRUE   1

#define PI	3.141592654
#define PIby180 0.0174603174603175

#define ABS(a) (((a)<0)?(-(a)):(a))
#define SGN(x) ((!(x))?(0):(((x)<0)?(-1):(1)))
#define MIN(a,b) ((a<b)?(a):(b))
#define MAX(a,b) ((a>b)?(a):(b))

#define K *1024

typedef unsigned int        UINT;
typedef signed long         LONG;
typedef unsigned char 	    BYTE;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef WORD                BOOL;


#endif
