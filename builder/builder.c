#include <poly.h>

#include <stdio.h>

#include <builder.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static Point *verts;
static Quad  *faces;
static Object	*objects;
static GenObject	*gen_objects;

static struct TexInfo *ti=NULL;
static char* tex_dir=NULL;
static char* texlist[MAXTEX];
static long ntex=0;

static long  nv=0,nf=0,nb=0,ngb=0; /* next free slot in the verts/faces/objects array */
static long maxv, maxf, maxb,maxgb;
int bdebug=0;

//		  frontend_display(faces,nf,verts,nv); 

//		frontend_set_tex(itex,n);

long builder_texlist_install(char *s)
{
	int i;
	char buf[250];
	char *inp;

	if ( !tex_dir || (s[0]=='.' && s[1]=='/') || s[0]=='/') inp=s;
	else {
			  	strcpy(buf,tex_dir);
				strcat(buf,"/");
				strcat(buf,s);
				inp=buf;
	}
	for (i=0;i<ntex;i++)  // have to do binary search
			  	if (!strcmp(inp,texlist[i])) return i;
	if (ntex==MAXTEX) {fprintf(stderr,"Too many textures\n");exit(0);}
	texlist[ntex++]=strdup(inp);
	
	return i;
}

struct TexInfo* builder_new_texinfo(long *tidlist, long max, float x_factor, float y_factor)
{
	// from now on all quads will use this ti
	ti=malloc(sizeof(struct TexInfo));
	ti->tid_list=malloc(sizeof(long)*max);
	memcpy(ti->tid_list,tidlist,sizeof(long)*max);
	ti->tex_x=x_factor;
	ti->tex_y=y_factor;
	ti->max=max;
	ti->pos=0;
	return ti;
}

void builder_set_texinfo(struct TexInfo *t)
{
		  	ti=t;
}

void builder_set_tex_dir(char *dir)
{
		  tex_dir=dir;
}

void builder_set_tex_factor(float x,float y)
{
		  if (!ti) {fprintf(stderr,"ti is NULL.\n"); exit(0);}
		  ti->tex_x=x; ti->tex_y=y;
}

void builder_alloc_mem(long f)
{
	long v=f*4; /* you can have a maximum of f*4 vertices */
	long b=f/4;
	long gb=f/4;
	if (b==0) b=10;
	if (gb==0) gb=10;
	verts=(Point*)malloc(v*sizeof(Point));
	faces=(Quad*) malloc(f*sizeof(Quad));
	objects=(Object*)malloc(b*sizeof(Object));  /* only for objects */
	gen_objects=(GenObject*)malloc(gb*sizeof(Object));  /* stores lines and matrices*/
	maxv=v;
	maxf=f;
	maxb=b;
	maxgb=gb;
}

void builder_reset()
{
	nv=nf=nb=ngb=0;
}

Point *builder_get_verts(long *pv)	{if (pv) *pv=nv;return verts;}
Quad  *builder_get_faces(long *pf)	{if (pf) *pf=nf;return faces;}
Object *builder_get_objects(long *pb) {if (pb) *pb=nb;return objects;}

long	*builder_get_nv() {return &nv;}
long  *builder_get_nf() {return &nf;}
long  *builder_get_nb() {return &nb;}
long	*builder_get_maxv() {return &maxv;}
long  *builder_get_maxf() {return &maxf;}
long  *builder_get_maxb() {return &maxb;}

char **builder_get_texlist() { return texlist; }
long	 builder_get_ntex()    { return ntex; }


void quad_display(long h)
{
	printf(":Quad[%d](%d,%d,%d,%d):",h,faces[h].v[0],faces[h].v[1],faces[h].v[2],faces[h].v[3]);
}

void object_display(long h)
{
	printf(":Object[%d](f%d:%d, v%d:%d):",h,objects[h].f_start, objects[h].nf, objects[h].v_start, objects[h].nv);
}

void point_display(long h)
{
	printf(":Point[%d](%f,%f,%f):",h,verts[h].x,verts[h].y,verts[h].z);
}

void point_undo_new()
{
	if (nv)	  nv--;
}

long point_new(float x,float y,float z)
{
	if (nv==maxv) { fprintf(stderr,"Too many points.\n"); exit(0); }
	verts[nv].x=x;
	verts[nv].y=y;
	verts[nv].z=z;
	if (bdebug) fprintf(stderr,"New Point[%d](%f,%f,%f)\n",nv,x,y,z);
	nv++;
	return nv-1;
}
			  
long point_copy(long h)
{
	return point_new(verts[h].x,verts[h].y,verts[h].z);
}

long point_transform(long h, Matrix3D* m)
{
  Transform(m,&verts[h]);
}

static void quad_set_texture(long h,long t, float factor_x, float factor_y)
{
	if (bdebug) fprintf(stderr,"Setting faces[%d].tid=%d factorx=%f factor_y=%f\n",h,t,factor_x,factor_y);
	faces[h].tid=t;
	faces[h].tex_factor_x=factor_x;
	faces[h].tex_factor_y=factor_y;
}

long quad_new(long h0,long h1, long h2, long h3)
{
	if (nf==maxf) { fprintf(stderr,"Too many quads.\n"); exit(0); }
	if (bdebug) fprintf(stderr,"New Quad[%d]=(%d,%d,%d,%d)\n",nf,h0,h1,h2,h3);
	faces[nf].p[0]=verts+h0;
	faces[nf].v[0]=h0;
	faces[nf].p[1]=verts+h1;
	faces[nf].v[1]=h1;
	faces[nf].p[2]=verts+h2;
	faces[nf].v[2]=h2;
	faces[nf].p[3]=verts+h3;
	faces[nf].v[3]=h3;
	//calcNormal(faces+nf); /*No point doing it now. Have to do it anyway after all transformations are done */
	if (ti) {
			  quad_set_texture(nf,ti->tid_list[ti->pos],ti->tex_x,ti->tex_y);
			  ti->pos=(ti->pos+1)%ti->max;
	} else {
			  quad_set_texture(nf,-1,1,1);
	}
	nf++;
	return nf-1;
}	


long quad_copy(long h)
{
	Quad *o=&faces[h];
	return quad_new(point_copy(o->v[0]),point_copy(o->v[1]),point_copy(o->v[2]),point_copy(o->v[3]));
}

void quad_flip(long h)
{
	int i;
	Point *p; long t;
	Quad *o=&faces[h];
	for (i=0;i<2;i++) {
			  p=o->p[i];
			  t=o->v[i];
			  o->p[i]=o->p[3-i];
			  o->v[i]=o->v[3-i];

			  o->p[3-i]=p;
			  o->v[3-i]=t;
	}		  
}

void object_set_centre(long h, float x, float y, float z)
{
	if (bdebug) fprintf(stderr,"objects[%d].setCentre(%f,%f,%f)\n",h,x,y,z);
	initPoint(objects[h].centre,x,y,z);
}

// Note: it is very important to allocate a point from the verts array.
// Only then will an operation on a parent object update the origin properly.
void object_set_origin(long h, float x, float y, float z)
{
	long p;
	if (bdebug) fprintf(stderr,"objects[%d].setOrigin(%f,%f,%f)\n",h,x,y,z);
	if (objects[h].origin_flag)  
			  initPoint(objects[h].origin,x,y,z);
	else {
			  p=point_new(x,y,z);
			  objects[h].origin=verts+p;  
			  objects[h].origin_flag=1;
	}
}

void object_set_origin_rel(long h,long src)
{   
	objects[h].origin=objects[src].centre;
}

void object_rotate(long handle, float x, float y, float z)
{
	if (bdebug) fprintf(stderr,"objects[%d].rotate(%f,%f,%f)\n",handle,x,y,z);
	Rotate(&objects[handle].m,x,y,z);
	object_apply(handle);
}

void object_translate(long handle, float x, float y, float z)
{
	if (bdebug) fprintf(stderr,"objects[%d].translate(%f,%f,%f)\n",handle,x,y,z);
	Translate(&objects[handle].m,x,y,z);
	object_apply(handle);
}

void object_scale(long handle, float x, float y, float z)
{
	if (bdebug) fprintf(stderr,"objects[%d].scale(%f,%f,%f)\n",handle,x,y,z);
	Scale(&objects[handle].m,x,y,z);
	object_apply(handle);
}

void object_shear(long handle, float x, float y)
{
	if (bdebug) fprintf(stderr,"objects[%d].shear(%f,%f)\n",handle,x,y);
	Shear(&objects[handle].m,x,y);
	object_apply(handle);
}

long object_begin()
{
	if (nb==maxb) { fprintf(stderr,"Too many objects.\n"); exit(0); }
	if (bdebug) fprintf(stderr,"New Object[%d]\n",nb);
	objects[nb].f_start=nf;
	objects[nb].v_start=nv;
	InitMatrix3D(&objects[nb].m);
	InitMatrix3D(&objects[nb].motion_m);
	objects[nb].motion_flag=0;
	objects[nb].origin_flag=0;
	/* create another point (the centre) as part of this object */
	objects[nb].centre=verts+point_new(0,0,0);  
	/* the origin is NULL */
	objects[nb].origin=NULL;

	nb++;
	return nb-1;
}

void object_end(long handle)
{
	Object *o=&objects[handle];
	o->nf=nf-o->f_start;
	o->nv=nv-o->v_start;
	if (bdebug) fprintf(stderr,"objects[%d].end f_start=%d,nf=%d, v_start=%d, nv=%d\n",handle,o->f_start,o->nf,o->v_start,o->nv);
}	

long object_copy(long src)
{
	int i,j;
	long newh;
	newh=object_begin();
	for (i=0;i<objects[src].nf;i++)
			  quad_copy(objects[src].f_start+i);
	object_end(newh);
	return newh;
}

void object_apply(long handle)
{
	int i;
	Object *o=&objects[handle];
	if (bdebug) fprintf(stderr,"objects[%d].apply\n",handle);
	for (i=o->v_start;i<o->v_start+o->nv;i++)
		Transform(&o->m,verts+i);
	/*printf("(initial)%f %f %f\n",o->origin.x,o->origin.y,o->origin.z);
	Transform(&o->m,&o->origin);
	printf("(after  )%f %f %f\n",o->origin.x,o->origin.y,o->origin.z);
	*/
	InitMatrix3D(&o->m);
}

void object_mat_apply(long handle, long math)
{
	int i;
	Object *o=&objects[handle];
	if (bdebug) fprintf(stderr,"objects[%d].applyMat(%d)\n",handle,math);
	for (i=0;i<o->nv;i++)
			  Transform(&gen_objects[math].m,verts+o->v_start+i);
}

void object_motion_apply(long h, long math)
{
	int i;
	InitMatrix3D(&objects[h].motion_m);
	MergeMatrix(&objects[h].motion_m,gen_objects[math].m.Matrix);
	objects[h].motion_flag=1;
}


long line_begin()
{
	if (bdebug) fprintf(stderr,"New Line[%d]\n",ngb);
	if (ngb==maxgb) { fprintf(stderr,"Too many objects.\n"); exit(0); }
	gen_objects[ngb].v_start=nv;
	InitMatrix3D(&gen_objects[ngb].m);
	ngb++;
	return ngb-1;
}

void line_end(long handle)
{
	GenObject *o=&gen_objects[handle];
	o->nv=nv-o->v_start;
	if (bdebug) fprintf(stderr,"lines[%d].end v_start=%d, nv=%d\n",handle,o->v_start,o->nv);
}

void line_rotate(long handle, float x, float y, float z)
{
	if (bdebug) fprintf(stderr,"lines[%d].rotate(%f,%f,%f)\n",handle,x,y,z);
	Rotate(&gen_objects[handle].m,x,y,z);
	line_apply(handle);
}

void line_translate(long handle, float x, float y, float z)
{
	if (bdebug) fprintf(stderr,"lines[%d].translate(%f,%f,%f)\n",handle,x,y,z);
	Translate(&gen_objects[handle].m,x,y,z);
	line_apply(handle);
}

void line_scale(long handle, float x, float y, float z)
{
	if (bdebug) fprintf(stderr,"line[%d].scale(%f,%f,%f)\n",handle,x,y,z);
	Scale(&gen_objects[handle].m,x,y,z);
	line_apply(handle);
}

void line_shear(long handle, float x, float y)
{
	if (bdebug) fprintf(stderr,"line[%d].shear(%f,%f)\n",handle,x,y);
	Shear(&gen_objects[handle].m,x,y);
	line_apply(handle);
}

void line_apply(long handle)
{
	int i;
	GenObject *o=&gen_objects[handle];
	if (bdebug) fprintf(stderr,"lines[%d].apply\n",handle);
	for (i=0;i<o->nv;i++)
			  Transform(&o->m,verts+o->v_start+i);
	InitMatrix3D(&o->m);
}

void line_mat_apply(long handle, long math)
{
	int i;
	GenObject *o=&gen_objects[handle];
	if (bdebug) fprintf(stderr,"lines[%d].applyMat(%d)\n",handle,math);
	for (i=0;i<o->nv;i++)
			  Transform(&gen_objects[math].m,verts+o->v_start+i);
}

long line_copy(long handle)
{
   int i;
	long h1,newl;
	GenObject *o=&gen_objects[handle];
	newl=line_begin();
	h1=o->v_start;
	for (i=0;i<o->nv;i++) 
	  point_copy(h1+i);
	line_end(newl);
	return newl;
}

void line_stitch(long line1, long line2)
{
 long M1=gen_objects[line1].nv;
 long M2=gen_objects[line2].nv;  /* Do for the shorter line, ignore the rest of the points */
 long h1,h2,M;
 long k;
 if (M1<M2) M=M1;
 else M=M2;
 h1=gen_objects[line1].v_start;
 h2=gen_objects[line2].v_start;
 for (k=0;k<M-1;k++) {     // construct the faces
	  quad_new(h1,h1+1,h2+1,h2);
	  h1++;h2++;
 }
}

/* This just creates the required vertices and faces. Its the users resposibility to have done 
 * a object.begin() before he created the gene. (the gene is included in this object). 
 * func() can do any operation on the matrix and return a matrix that is then applied to
 * the original points, to generate the next set.
 */
void line_sweep(long handle, long N, Matrix3D *func(Matrix3D *,long i, long N), int closed)
{
 GenObject* o = &gen_objects[handle];
 long M=o->nv;
 long h1,h2,save_h2,h_gene;
 long i,k;
 Matrix3D m;
 h_gene=o->v_start;

 h1=h_gene;
 InitMatrix3D(&m);
 for (i=0;i<N;i++) {
			if (i==N-1) {
					  if (closed) h2=o->v_start; 			/* the last step - merge with original points */
						else  break;
			} else { 				
					  h2=point_copy(h_gene);
					  for (k=1;k<M;k++)
								 point_copy(h_gene+k);
					  for (k=0;k<M;k++) 
								 point_transform(h2+k,func(&m,i+1,N));
			}

			save_h2=h2;
			for (k=0;k<M-1;k++) {     // construct the faces
					  quad_new(h1,h1+1,h2+1,h2);
					  h1++;h2++;
			}
			h1=save_h2;
 }
}

long matrix_new()
{
	if (ngb==maxgb) { fprintf(stderr,"Too many matrices.\n"); exit(0); }
	if (bdebug) fprintf(stderr,"New Matrix[%d]\n",ngb);
	InitMatrix3D(&gen_objects[ngb].m);
	ngb++;
	return ngb-1;
}

long matrix_copy(long src)
{
	long h;
	h=matrix_new();
	gen_objects[h].m=gen_objects[src].m;
	return h;
}

void matrix_init(long h)
{
	InitMatrix3D(&gen_objects[h].m);
}

void matrix_translate(long h,float x,float y,float z)
{
	Translate(&gen_objects[h].m,x,y,z);
}

void matrix_rotate(long h,float x,float y,float z)
{
	Rotate(&gen_objects[h].m,x,y,z);
}

void matrix_scale(long h, float x, float y, float z)
{
	Scale(&gen_objects[h].m,x,y,z);
}

void matrix_shear(long h, float x, float y)
{
	Shear(&gen_objects[h].m,x,y);
}

void matrix_merge(long h, long src)
{
	MergeMatrix(&gen_objects[h].m,gen_objects[src].m.Matrix);
}

