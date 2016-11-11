#include <GL/glut.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>

/* Use 'convert' in unix to convert any jpg to bmp */
#include <bmploader.h>
#include <common.h>
#include <gl_helper.h>

void gl_enable_lights()
{
GLfloat light_diffuse[] = {1,1,1,1};
GLfloat light_specular[]= {1,1,1};
GLfloat light_ambient[] = {0,0,.1,1};

GLfloat mat_ambient[]= {1,1,1,1};
GLfloat mat_diffuse[]= {0,0,.31,1};
float mat_specular[] = {1,1,1,1};
float mat_shine[] 	= {4}; // 0-128
GLfloat ambient[]		= {.5,.5,.5,1};  // The general ambient light.

	glEnable(GL_LIGHTING);			        //enables lighting
	glEnable(GL_LIGHT0);				//enables a light

	glEnable(GL_COLOR_MATERIAL);	 		//causes lighting model to take glColor() calls
	glColorMaterial(GL_FRONT,GL_DIFFUSE);    // p 207

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);//sets lighting to one-sided
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE); //or GL_TRUE, FALSE is default - thus infinite view point is default
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR); // wether Specular is calculated seperately
						/* GL_SEPERATE_SPECULAR_COLOR */


							//into account

				/* GL_AMBIENT, GL_SPECULAR, GL_DIFFUSE, */

	glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);
	glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);
	glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
	glMaterialfv(GL_FRONT,GL_SHININESS,mat_shine);   // takes a value from 0 to 128.0
//	glMaterialfv(GL_FRONT,GL_EMISSION,mat_ambient);

//	glLightf(GL_LIGHT0,GL_SPOT_CUTOFF, 20);

	glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);	//updates the light's diffuse colour
	glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);//updates the light's specular colour
	glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);	//updates the light's ambient colour
			   /*GL_POSITION, GL_SPOT_DIRECTION,GL_SPOT_EXPONENT,GL_SPOT_CUTOFF,
			   	GL_CONSTANT_ATTENUATION, GL_QUADRATIC_ATTENUATION */


//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA,GL_DST_COLOR); // p 233
//	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // p 233
//	glBlendFunc(GL_SRC_ALPHA,GL_DST_ALPHA); // p 233
	

	glShadeModel(GL_FLAT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


#define MAXTEX 100
int gl_load_AllBMPs(GLuint **texids)
{
		  int ntex=0;
		  int i;
		  *texids=(GLuint*)malloc(MAXTEX*sizeof(GLuint));
		  for (i=0;i<MAXTEX;i++) (*texids)[i]=0;
	//	  loadBMP("../img/armada.bmp");
		  if (i=gl_loadBMP("../../img3/rotunda1.bmp")) *texids[ntex++]=i;
		  glEnable(GL_TEXTURE_2D);
		  glEnable(GL_BLEND);
		  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		  return ntex;
}

long gl_loadBMP(char *fname)
{
		  GLuint tid;
		 // if (ntex==MAXTEX) {printf("cannot load anymore\n");return;}
		  if (load2DTextureBMP(fname,&tid,GL_RGB)!=LOAD_TEXTUREBMP_SUCCESS) {
					 printf("Error loading texture from %s\n",fname);
					 return 0;
		  }	
		  printf("loaded texture from %s (tid=%d)\n",fname,tid);
		  return tid;
}


void gl_show_mat()
{
    float matrix[16];

    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    printf("% 0.3f% 0.3f% 0.3f% 0.3f\n",
           matrix[0],matrix[4],matrix[8],matrix[12]);
    printf("% 0.3f% 0.3f% 0.3f% 0.3f\n",
           matrix[1],matrix[5],matrix[9],matrix[13]);
    printf("% 0.3f% 0.3f% 0.3f% 0.3f\n",
           matrix[2],matrix[6],matrix[10],matrix[14]);
    printf("% 0.3f% 0.3f% 0.3f% 0.3f\n",
           matrix[3],matrix[7],matrix[11],matrix[15]);
	   printf("\n");
}

void gl_my_translate(float x, float y, float z)
{
    float mvmatrix[16];

    glGetFloatv(GL_MODELVIEW_MATRIX, mvmatrix);
    glLoadIdentity();
    glTranslatef(x,y,z);
    glMultMatrixf(mvmatrix);
}

void gl_my_rotate(float angle, float x, float y, float z)
{
    float mvmatrix[16];

    glGetFloatv(GL_MODELVIEW_MATRIX, mvmatrix);
    glLoadIdentity();
    glRotatef(angle,x,y,z);
    glMultMatrixf(mvmatrix);
}

