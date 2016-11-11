#include <GL/glut.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <matrix3d.h>
#include <poly.h>
#include <common.h>
#include <builder.h>
#include <gl_helper.h>

#define FP (float*)

//defines the colour of each vertex
#define MYALPHA 1
static Color colordat[8]=
	{
		{0.5,0  ,0  ,MYALPHA}, //dark red
		{1  ,1  ,0.3,MYALPHA}, //yellow
		{1  ,0  ,0  ,MYALPHA}, //red
		{0.5,1  ,0.2,MYALPHA}, //dull yellow??
		{1  ,1  ,0  ,MYALPHA}, //yellow
		{0.9,0.5,0  ,MYALPHA}, //orange
		{1  ,0.9,0.1,MYALPHA}, //yellow
		{1  ,0  ,0  ,MYALPHA}, //red
	};

static GLfloat light_pos[]= {500,0,0,1};  // The last parameter =1 => positional light

static Point tex_coords[]={0,1,0, 0,0,0, 1,0,0, 1,1,0};   // we are using Point as a 2d point

static GLuint *texids=NULL;
static long ntex=0; // number of textures

static Point *verts;
static Quad  *faces;
static Point *line;
static Object *objects;

static long *pnv=0,*pnf=0,*pnb;

enum {INPUT,DISPLAY,DISPLAY_ONLY,CHANGING};
static int mode=CHANGING;

static bool show_normals=false;
static int render_style=GL_QUADS;  /*(p44) GL_POINTS,GL_LINES,GL_LINE_STRIP,GL_LINE_LOOP,GL_QUADS, */

static Vector rot;
static Vector trans;
static Vector velocity;

static int mouseX=0;
static int mouseY=0;
static int mouseState=0;
static int mouseButton=0;

static long save_nv=0;
static long save_N=0;

static long line_handle;

//static long winW=1140,winH=836;
static long winW=400,winH=400;
static void render(void);
static void glutMotion(int x, int y);
static void glutMouse(int btn, int state, int x, int y);

static void glutKeyboard(unsigned char ch, int x, int y);
static void glutPressKey(int key,int x,int y);
static void glutReleaseKey(int key,int x,int y);

static void glutResize(int w, int h);

static void glutDie();

FILE *ifp=NULL;

static void glutDie();

/* Initially I applied new_mat on the prev. points, but due to round-off errors you get skewed results.
 * So now I apply new_mat to the original points each time. This is the only way to do it. If you try
 * rotating m by 360/N each time instead of reinitializing and rotating by (i+1)*360/N, you will get
 * wierd output
 */
/* i starts from 1 */
static Matrix3D* new_mat(Matrix3D *m, long i, long N)
{
		  int r=1;
		  InitMatrix3D(m);
		  Rotate(m,0,(i)*360./N,0);
		  //Translate(m,0,0,-i*100000);
		  return m;
}

static void recalc_normals();
static void set_mode_display(long N)
{
		  Matrix3D m;
		  int i;
		  mode=CHANGING;
		  line_end(line_handle);
		  save_nv=*pnv;
		  save_N=N;
		  line_sweep(line_handle,N,&new_mat,1);
		  recalc_normals();
		  if (ntex)
		  	for (i=0;i<*pnf;i++)
					 faces[i].tid=0;
		  glutIdleFunc(render);
//		  glMatrixMode(GL_PROJECTION);
//		  glPopMatrix();
//		  glLoadIdentity();
//		  gluPerspective(60.0,winW/winH,1.0,20.0);  // fov, aspect, near, far
//  	  glMatrixMode(GL_MODELVIEW);
		  mode=DISPLAY;
}

/* before init_window(), either call set_mode_input(), or set_mode_displayonly().
 * set_mode_input(), allows the user to first feed in the outline of the object
 * which is then passed to a volume sweeping function to generate the object.
 * set_mode_displayonly() should be used if this is being used only as frontend.
 * ie: you already have the data set - and you want to feed it to this.
 */

void goto_origin()
{
	trans.x=trans.y=trans.z=rot.x=rot.y=rot.z=0;	  
	velocity.x=velocity.y=velocity.z=0;
   glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void set_mode_input()
{
	mode=CHANGING;
	builder_reset();
	goto_origin();
	line_handle=line_begin();
	glutIdleFunc(NULL);
	glutPostRedisplay();
//	glPushMatrix();
//	glMatrixMode(GL_PROJECTION);
//		glLoadIdentity();
//		gluOrtho2D(-10000,+10000,-10000,+10000);
//	glMatrixMode(GL_MODELVIEW);
	mode=INPUT;
}

void set_mode_display_only()
{
		  mode=DISPLAY_ONLY;
		  glutIdleFunc(render);
}


static void recalc_normals()
{
		  	int i;
			for (i=0;i<*pnf;i++)
					  calcNormal(faces+i);
}

#define MAXLINE 200
/* remember that if mode=INPUT, then nv,nf,nb are most likely 0 */
void gl_run(Point *iverts, long *nv, Quad *ifaces, long *nf, Object *iobjects, long *nb)
{
	verts=iverts;
	faces=ifaces;
	objects=iobjects;
	pnv=nv;
	pnf=nf;
	pnb=nb;

	line=malloc(sizeof(Point)*MAXLINE);
	recalc_normals();
//	gl_enable_lights();
	glutMainLoop();
}

void gl_load_textures(char **itex,int n)
{
		  int i;
		  assert(n>0);  // if somebody is calling this with n=0, he probably didnt expect it to be 0
		  texids=malloc(n*sizeof(int));
		  for (i=0;i<n;i++) {
					 texids[i]=gl_loadBMP(itex[i]);
					 if (texids[i]==0) {  // We can't continue because some faces are pointing to this texture.
								fprintf(stderr,"Bailing out.\n");
								glutDie();
					 }
		  }
		  ntex=n;
		  
		  //ntex=gl_load_AllBMPs(&texids);
		  glEnable(GL_TEXTURE_2D);
		  glEnable(GL_BLEND);
		  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
}

void gl_init(int* argc,char **argv)
{
	glutInit(argc,argv);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(winW,winH);
	glutInitWindowPosition(1,1);
	glutCreateWindow("cube");

	glutDisplayFunc(render);	
//	glutIdleFunc(render);

	glutMouseFunc(glutMouse);
//	glutPassiveMotionFunc(glutMotion);
	glutMotionFunc(glutMotion);
//	glutEntryFunc((*func)(int state));

	glutKeyboardFunc(glutKeyboard);

	glutIgnoreKeyRepeat(0);
	glutSpecialFunc(glutPressKey);
	glutSpecialUpFunc(glutReleaseKey);

	glutReshapeFunc(glutResize);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_RESCALE_NORMAL);
	//glEnable(GL_NORMALIZE);

//	glPointSize(3);
//	glLineWidth(3);
//	glLineStipple(1,0x3f07);
//	glEnable(GL_LINE_STIPPLE);
}



void drawNormal(float *p1, float *p2, float *vector);

void inline gl_draw_quads_wireframe(Quad *f)
{
		  long b,v;
		  if (show_normals) drawNormal(FP(f->p[0]),FP(f->p[2]),FP(&f->normal));
		  glBegin(GL_LINE_LOOP);
		  glNormal3fv(FP&(f->normal)); //sets the current normal to this quad's normal
		  for (b=0;b<4;b++) {
//					 glColor3fv(FP&colordat[f->v[b]%7]);
					 glVertex3fv(FP(f->p[b]));
		  }
		  glEnd();
}

void inline gl_draw_quads_solid(Quad *f)
{
		  long b,v;
		  if (f->tid!=-1) glBindTexture(GL_TEXTURE_2D,texids[f->tid]);
		  glBegin(GL_QUADS);
		  glNormal3fv(FP&f->normal); //sets the current normal to this quad's normal
		  for (b=0;b<4;b++) {
					 if (f->tid!=-1) glTexCoord2f(tex_coords[b].x*f->tex_factor_x,tex_coords[b].y*f->tex_factor_y);
					 glColor3fv(FP&colordat[f->v[b]%7]);
					 glVertex3fv(FP(f->p[b]));
		  }
		  glEnd();
}

void inline gl_draw_triangles_wireframe(Quad *f)
{
		  long b,v;
					 glBegin(GL_LINE_STRIP);
					 glNormal3fv(FP&(f->normal)); //sets the current normal to this quad's normal
					 for (b=2;b!=1;b=(b+1)%4) {
//								glColor3fv(FP&colordat[f->v[b]%7]);
								glVertex3fv(FP(f->p[b]));
					 }
					 for (b=2;b>=0;b--) {
//								glColor3fv(FP&colordat[f->v[b]%7]);
								glVertex3fv(FP(f->p[b]));
					 }
					 glEnd();
}

/* warning: Don't put // style comments within a #define. Remember that this is actually a single line. */
					 //if (ntex) glTexCoord2f(tex_coords[b].x,tex_coords[b].y); 
					 
#define drawLine(x1,y1,z1,x2,y2,z2)	glBegin(GL_LINES); glVertex3f(x1,y1,z1); glVertex3f(x2,y2,z2); glEnd();

void print_vector(float *v)
{
		  int i;
		  for (i=0;i<3;i++) printf("%f:",v[i]);
		  printf("\n");
}

void drawNormal(float *p1, float* p2,float *vector)
{
	float length=10;
	float x,y,z;
	x=(p1[0]+p2[0])/2;
	y=(p1[1]+p2[1])/2;
	z=(p1[2]+p2[2])/2;
	drawLine(x,y,z,x+length*vector[0],y+length*vector[1],z+length*vector[2]);
}

#define TRIANGLE_CORNER(b) \
					 if (f->tid!=-1) glTexCoord2f(tex_coords[b].x*f->tex_factor_x,tex_coords[b].y*f->tex_factor_y);\
					 /*glColor3fv(FP&colordat[b%7]);*/\
					 glColor3f(1,1,1); \
					 glVertex3fv(FP(f->p[b]));

void inline gl_draw_triangles_solid(Quad *f)
{
		  long b,v;
		  if (f->tid!=-1) glBindTexture(GL_TEXTURE_2D,texids[f->tid]); 
		  if (show_normals) drawNormal(FP(f->p[0]),FP(f->p[2]),FP(&f->normal)); 
		  glBegin(GL_TRIANGLE_FAN);
		  glNormal3fv(FP(&f->normal)); //sets the current normal to this quad's normal

		  TRIANGLE_CORNER(0);
		  TRIANGLE_CORNER(1);
		  TRIANGLE_CORNER(2);
		  TRIANGLE_CORNER(3);

		  glEnd();
}

#define SP_FACTOR .2
#define LOOKSPEED .5
#define DECAY .002
//#define drawLine(x1,y1,z1,x2,y2,z2)

#define FRICTION .05

#define BRAKE(x,factor)	if (velocity.x>factor) velocity.x-=factor;\
									else if (velocity.x<-factor) velocity.x+=factor;\
									if (velocity.x>-factor&& velocity.x<factor) velocity.x=0;
static void render(void)
{
	static float rotateBy=.2;
	long v;
	long a,b,m;
	long i,j;
	Object *o;


	if (mode==CHANGING) return;
	if (mode==INPUT) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor3f(1,1,1);
		glBegin(GL_LINE_STRIP);
		for (a=0;a<*pnv;a++) {
				  glVertex3fv(FP&line[a]);
		}
		
		glEnd();
		glutSwapBuffers();
		return;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	gl_my_translate(trans.x,trans.y,trans.z);
	BRAKE(x,FRICTION); BRAKE(y,FRICTION); BRAKE(z,FRICTION);
	trans.x=velocity.x*SP_FACTOR;
	trans.y=velocity.y*SP_FACTOR;
	trans.z=velocity.z*SP_FACTOR;


	glPushMatrix();
	gl_my_rotate(rot.x, 1, 0, 0);
	glLightfv(GL_LIGHT0,GL_POSITION,light_pos);
	if (show_normals) drawLine(light_pos[0],light_pos[1],light_pos[2],0,0,0);

	/*
	drawLine(-100,0,0,100,0,0);
	drawLine(0,-100,0,0,100,0);
	drawLine(0,0,-100,0,0,100);
	*/
	
	if (*pnb) {
			  Matrix3D m;
			  for (a=0;a<*pnb;a++) {
						 o=objects+a;					 
						 if (o->motion_flag) {
							//MatrixPrint(&o->motion_m);
							InitMatrix3D(&m);
						   if (o->origin_flag)  Translate(&m,-o->origin->x,-o->origin->y,-o->origin->z);
							//printf("m (origin)\n");
							//MatrixPrint(&m);

							MergeMatrix(&m,o->motion_m.Matrix);
							//printf("m (rotate)\n");
							//MatrixPrint(&m);

						 	if (o->origin_flag)  Translate(&m,+o->origin->x,+o->origin->y,+o->origin->z);
							//printf("m (restore)\n");
							//MatrixPrint(&m);

							for (i=0;i<o->nv;i++)   // Note that the centre is part of this list, (the origin may be)
								Transform(&m,verts+o->v_start+i);

						 	for (i=0;i<o->nf;i++)
								calcNormal(faces+o->f_start+i);
						 }
			  }
	}

	if (render_style==GL_QUADS) {
			  for (b=0;b<*pnf;b++) 
						 gl_draw_triangles_solid(faces+b);
			  //gl_draw_quads_solid(faces+b);
	} else {  /* This is to draw the wire frame */
			  for (a=0;a<*pnf;a++) 
						 //gl_draw_triangles_wireframe(faces+a);
						gl_draw_quads_wireframe(faces+a);
	}

	glPopMatrix();
	glutSwapBuffers();
}


/* Note that if you disable this, nothing will come on the screen */
static void glutResize(int w, int h)   // note: this is also called once at the start
{	
		  if (!h)
					 return;

		  printf("%d,%d\n",w,h);
		  winW = w;
		  winH = h;

		  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		  glViewport(0, 0, winW, winH);

		  //GL_PROJECTION GL_MODELVIEW and GL_TEXTURE
		  glMatrixMode(GL_PROJECTION);
		  glLoadIdentity();
		  gluPerspective(45, (double) winW / winH, 1, 4096);

		  glMatrixMode(GL_TEXTURE);
		  glLoadIdentity();
		  //   glScalef(4,4,0);

		  glMatrixMode(GL_MODELVIEW);
		  //	glLoadIdentity();
		  glutPostRedisplay();
}


#define SP_CONST 10
static void glutPressKey(int key,int x,int y)
{
		  /*printf("%d %d %d\n",key,x,y);*/
		  if (mode==DISPLAY || mode==DISPLAY_ONLY) {
					 switch (key) 
					 {
								case GLUT_KEY_UP: trans.z=SP_CONST*SP_FACTOR;
														break;
								case GLUT_KEY_DOWN: trans.z=-SP_CONST*SP_FACTOR;
														  break;
								case GLUT_KEY_LEFT: trans.x=SP_CONST*SP_FACTOR;
														  break;
								case GLUT_KEY_RIGHT: trans.x=-SP_CONST*SP_FACTOR;
															break;
								case GLUT_KEY_PAGE_UP: velocity.y--;
														break;
								case GLUT_KEY_PAGE_DOWN: velocity.y++;
														  break;
								case GLUT_KEY_HOME: goto_origin(); break;
					 }
		  }
}	

static void glutReleaseKey(int key, int x, int y)
{
		  /*printf("%d %d %d\n",key,x,y);*/
		  if (mode==DISPLAY || mode==DISPLAY_ONLY) {
					 switch(key)
					 {
								case GLUT_KEY_UP: 
								case GLUT_KEY_DOWN: //APPLY_FRICTION(trans.z);
														  break;
								case GLUT_KEY_LEFT: 
								case GLUT_KEY_RIGHT: //APPLY_FRICTION(trans.x);
															break;
					 }
		  }
}	


static void save_file()
{
		  int i,maxy,miny,middley;
		  char buf[256];
		  static int file_num=0;
		  char fname[40]="object";
		  char fname_ext[]="dat";
		  char* templfile="object.template";
		  FILE *fp,*ifp;

		  char key[]="<DATA_POINTS>";
		  file_num++;

		  ifp=fopen("object.template","r");
		  if (!ifp) {
					 printf("Could'nt open %s\n",templfile);
					 printf("Can't save any data. Press any key to continue.\n");
					 getchar();
					 return;
		  }

		  sprintf(buf,"%s%02d.%s",fname,file_num,fname_ext);
		  fp=fopen(buf,"w");
		  if (!fp) {
					 printf("Could'nt open object.dat\n");
					 printf("You data will not be saved. Press any key to continue..\n");
					 getchar();
					 fclose(fp);
					 return;
		  }
		 
		  maxy=miny=verts[0].y;
		  for (i=1;i<save_nv;i++) {
				if (verts[i].y>maxy) maxy=verts[i].y;
				if (verts[i].y<miny) miny=verts[i].y;
		  }
		  middley=(maxy+miny)/2;
		  while (fgets(buf,256,ifp)) {
					 if (strstr(buf,"<DATA_POINTS>")) {
								for (i=0;i<save_nv;i++) 
										  fprintf(fp,"\tnew Point (%f,%f,%f);\n",verts[i].x,verts[i].y-middley,verts[i].z);
								//fprintf(fp,"\tn=%d;\n",save_N);
					 } else {
								fputs(buf,fp);
					 }
		  }
		  fclose(fp);
		  fclose(ifp);
}

#define BRAKE_FACTOR 8
#define MAX_VEL		1000

#define INCR(x) if (velocity.x<MAX_VEL) velocity.x++;
#define DECR(x) if (velocity.x>-MAX_VEL) velocity.x--;

static void glutKeyboard(unsigned char ch, int x, int y)
{
		  static char num[50];
		  static int top=0;
		  int N;
		  if (mode==INPUT) {
					 switch (ch) {
								case 13: num[top]=0; top=0;
											 N=atoi(num); 
											 if (N<1 || N>100) N=40;
											 if (*pnv<=1) break;
											 set_mode_display(N);
											 break;
								case 'r': set_mode_input();
											 break;
								case 'u': point_undo_new();
											 glutPostRedisplay();
											 break;
								case 27: glutDie();
								default:  if (ch>='0' && ch<='9') 
														num[top++]=ch;
											 else top=0;
					 }
		  } else if (mode==DISPLAY || mode==DISPLAY_ONLY) {
					 switch (ch) {
								case ' ': velocity.y=0; break;
								case 'w': INCR(z);
											 break;
								case 's': DECR(z);
											 break;
								case 'a': INCR(x);
											 break;
								case 'd': DECR(x);
											 break;
								case '1': //velocity.x=velocity.y=velocity.z=0;
											 BRAKE(x,BRAKE_FACTOR); BRAKE(y,BRAKE_FACTOR); BRAKE(z,BRAKE_FACTOR);
											 break;
								case 'q': gl_my_rotate(-1,0,1,0);
											 break;
								case 'e': gl_my_rotate(1,0,1,0);
											 break;
								case 't': if (render_style==GL_QUADS) render_style=GL_LINE_LOOP;
														else render_style=GL_QUADS;
											 break;
								case 'n': show_normals=!show_normals;break;
								case 27: glutDie();			 
											break;
					 }
					 if (mode==DISPLAY) {
								switch (ch) {
										  case 13: save_file();
													  set_mode_input();
													  break;
										  case 'r': set_mode_input();
														break;
								}
					 }
		  }
}

/*
	int glutGetModifiers() will return one of the following
	GLUT_ACTIVE_SHIFT, GLUT_ACTIVE_CTRL, GLUT_ACTIVE_ALT
	xor(caps,shift)
 */	


static void glutMouse(int btn, int bstate, int x, int y)
{
		  GLint viewport[4];
		  GLdouble mvmatrix[16], projmatrix[16];
		  GLint realy;
		  GLdouble wx,wy,wz;

		  int mod=glutGetModifiers();
		  if (mode==INPUT) {
					 if (bstate == GLUT_DOWN && btn==GLUT_LEFT_BUTTON) {
								glGetIntegerv(GL_VIEWPORT, viewport);
								glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
								glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
								realy=viewport[3] - (GLint) y -1;
								gluUnProject((GLdouble)x, (GLdouble)realy, 0.1,\
													 mvmatrix, projmatrix, viewport, &wx, &wy, &wz);

								//wx=x;wy=y;wz=0;
								line[*pnv].x=(float)wx; line[*pnv].y=(float)wy; 
								line[*pnv].z=wz;
								point_new((winW-x),(winH/2-y),0);
								glutPostRedisplay();
					 }
		  }else if (mode==DISPLAY || mode==DISPLAY_ONLY) {
					 if (bstate == GLUT_DOWN) {
								if (btn==GLUT_LEFT_BUTTON) {
										  mouseState = bstate;
										  mouseButton = btn;
										  mouseX = x;
										  mouseY = y;
								}
								if (btn==GLUT_RIGHT_BUTTON) velocity.y+=8;
								else if (btn==GLUT_MIDDLE_BUTTON) velocity.y-=8;
					 }
					 else { 
								mouseState = 0;
					 } 
		  }
}



static void glutMotion(int x, int y)
{
		  if (mode==DISPLAY || mode==DISPLAY_ONLY) {
					 if (mouseState == GLUT_DOWN) {
								if (mouseButton == GLUT_LEFT_BUTTON) {
										  rot.x+=(y-mouseY)*LOOKSPEED;
										  //	 rot.y=(x-mouseX)*LOOKSPEED;
										  gl_my_rotate((x-mouseX)*LOOKSPEED, 0, 1, 0);
					 					  mouseX = x;
										  mouseY = y;
								} 
					 }
		  }
}

static void glutDie()
{
		  //free (verts);
		  //free (faces);
		  exit(0);
}
