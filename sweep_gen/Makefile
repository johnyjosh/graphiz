CC=gcc
OPTIONS=-g
INCLUDE=-I../builder/ -I. -I../frontend -I../common -I../gl -I.. -I/usr/X11R6/include
LIBDIRS = -L/usr/X11R6/lib -L/uf6/gfx/lib/linux -L/home/jg9h/lib
LIBS    = -lglut -lGLU -lGL -lXmu -lXext -lXi -lX11 -lm -lgltexfonts

SRCS=../builder/builder.c ../common/poly.c ../gl/bmploader.c ../common/matrix3d.c ../gl/gl_helper.c ../gl/gl_input.c sweep.c

OBJS=$(SRCS:.c=.o)

all: sweep 


sweep: $(SRCS)
	$(CC) $(SRCS) $(INCLUDE) $(OPTIONS) -lfl $(LIBS) $(LIBDIRS) -o sweep 

clean:
	rm -f sweep
	rm -f *.dat
