#ifndef GL_INPUT_H
#define GL_INPUT_H


void set_mode_input();
void set_mode_display_only();
void gl_run(Point *iverts, long *nv, Quad *ifaces, long *nf, Object *objects, long *nbf);
void gl_init(int* argc,char **argv);

#endif
