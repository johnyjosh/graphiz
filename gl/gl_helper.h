#ifndef GL_HELPER_H
#define GL_HELPER_H

#include <GL/glut.h>

void gl_enable_lights();
int gl_load_AllBMPs(GLuint **texids);
void gl_show_mat();
void gl_my_translate(float x, float y, float z);
void gl_my_rotate(float angle, float x, float y, float z);
long gl_loadBMP(char *fname);

#endif
