CC=gcc
OPTIONS=-g
INCLUDE=-I../builder/ -I. -I../common -I../gl -I.. -I/usr/X11R6/include -I../sweep_gen
LIBDIRS = -L/usr/X11R6/lib -L/uf6/gfx/lib/linux -L/home/jg9h/lib
LIBS    = -lglut -lGLU -lGL -lXmu -lXext -lXi -lX11 -lm -lgltexfonts

SRCS=ucchelper.c ../gl/gl_input.c ../gl/gl_helper.c ../builder/builder.c ../common/poly.c ../gl/bmploader.c ../common/matrix3d.c y.tab.c lex.yy.c
OBJS=$(SRCS:.c=.o)

all: ucc


y.tab.c: ucc.y
	yacc  -d ucc.y
lex.yy.c: ucc.l
	flex ucc.l

ucc : $(SRCS)
	$(CC) $(SRCS) $(INCLUDE) $(OPTIONS) -lfl $(LIBS) $(LIBDIRS) -o ucc
	rm lex.yy.c y.tab.c y.tab.h

clean:
	-rm -f ucc

depend:
	make
