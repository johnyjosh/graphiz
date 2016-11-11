#ifndef OP_DEF_H
#define OP_DEF_H
/* was forced to stick this seperate from ucc.h, because ucc.h is included in ucc.l and the BEGIN/END here
 * causes problems with the lexer */

enum {APPLY,TRANSLATE,ROTATE,SCALE,SHEAR,BEGIN, END, SWEEP, APPLY_MAT, SET_MOTION,\
		SET_ORIGIN,SET_CENTRE,\
		FLIP, MAT_INIT, MAT_MERGE, \
		STITCH, PRINT,READ,CLRSCR,NEW,SET_TEXTURE, SET_TEXTURE_FACTOR,SET_TEXTURE_DIR,\
		MUL,ADD,SUB,NEG,DIV,CALL,MOV,RET,WHILE,FOR,DO,IF,IF_ELSE};
		  
#endif
