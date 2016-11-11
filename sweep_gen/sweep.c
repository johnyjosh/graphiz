
#include <poly.h>
#include <common.h>

#include <builder.h>
#include <gl_input.h>

#define FP (float*)

static	Point *verts=NULL;
static	Point *line=NULL;
static	Quad  *faces=NULL;
static   Object *objects=NULL;
static	long *pnv,*pnf,*pnb;
static 	char* texlist[]={"../inp/tex/pa012.bmp"};
int main()
{
	int argc=1;
	int i;
	char* args[]={"dummy"};

	builder_alloc_mem(5000);
	verts=builder_get_verts(NULL);
	faces=builder_get_faces(NULL);
	objects=builder_get_objects(NULL);
	pnv=builder_get_nv();
	pnf=builder_get_nf();
	pnb=builder_get_nb();

	gl_init(&argc,args);
	set_mode_input();
	gl_load_textures(texlist,1);
	gl_enable_lights();
	gl_run(verts,pnv,faces,pnf,objects,pnb);
}
