#ifndef BUILDER_H
#define BUILDER_H
#include <matrix3d.h>

/* An object just has the index into the quad array at which this object starts and the index at
 * which this ends. This helps name a group of quads that form an object (like a cube).
 * Once thus named, this set of quads can be manipulated (translated/rotated/etc) as a whole.
 */
typedef struct {
		  long f_start,nf,v_start,nv;
		  Matrix3D m;
		  Matrix3D motion_m; int motion_flag;
		  Point* origin; int origin_flag;
		  Point* centre;
} Object;		 // This is only for objects. We want a tight loop in the renderer - so we avoid a mixed array of objects,
					 // and maitain a single array for objects.

typedef struct {
		  long v_start,nv;
		  Matrix3D m;
} GenObject;	// This could be a matrix or a line.	  

#define MAXTEX	100 

struct TexInfo {
	int* tid_list;
	int pos;
	int max;
	float tex_x,tex_y;
};

long builder_texlist_install(char *s);
struct TexInfo* builder_new_texinfo(long *tidlist, long max, float x_factor, float y_factor);
void builder_set_texinfo(struct TexInfo *t);
void builder_set_tex_dir(char *dir);
void builder_set_tex_factor(float x,float y);
char **builder_get_texlist();
long	 builder_get_ntex()   ;

void builder_display();
void builder_alloc_mem(long f);
void builder_reset();

Point *builder_get_verts(long *pv);
Quad  *builder_get_faces(long *pf);
Object *builder_get_objects(long *pb);
long	*builder_get_nv();
long  *builder_get_nf();
long  *builder_get_nb(); 
long	*builder_get_maxv();
long  *builder_get_maxf();
long  *builder_get_maxb();

long point_new(float x, float y, float z);
long point_copy(long h);
long point_transform(long h, Matrix3D* m);

long quad_new(long a, long b, long c, long d);
long quad_copy(long h);
void quad_flip(long h);

void object_set_origin_rel(long h,long src);
void object_set_centre(long h, float x, float y, float z);
void object_set_origin(long h, float x, float y, float z);
long object_copy(long src);
long object_begin();
void object_end(long);
 void object_translate(long h, float x, float y, float z);
 void object_rotate(long h, float x, float y, float z);
 void object_scale(long h, float x, float y, float z);
 void object_shear(long h, float x, float y);
void object_apply(long handle);
void object_mat_apply(long handle, long math);
void object_motion_apply(long handle, long math);

long line_begin();
void line_end(long);
 void line_translate(long h, float x, float y, float z);
 void line_rotate(long h, float x, float y, float z);
 void line_scale(long h, float x, float y, float z);
 void line_shear(long h, float x, float y);
void line_apply(long handle);
void line_mat_apply(long handle, long math);
void line_sweep(long handle, long N, Matrix3D *func(Matrix3D *,long,long), int closed);

long matrix_new();
long matrix_copy(long src);
void matrix_init(long h);
 void matrix_translate(long h,float x,float y,float z);
 void matrix_rotate(long h,float x,float y,float z);
 void matrix_scale(long h, float x, float y, float z);
 void matrix_shear(long h, float x, float y);
void matrix_merge(long h, long src);
#endif
