#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <builder.h>
#include <common.h>
#include <assert.h>
#include <ucc.h>
#include "y.tab.h"
#include <stdio.h>

#include <op_defs.h>


#define MAXCALLDEPTH 30

#define MAXSYMBOLS   1000	/* Number of symbols in each scope. Note that every literal is allocated a temporary, and
										every operation result is first stored in a temporary */
#define MAXSCOPELIST 50    /* each declared function has its own scope. Everything in scopelist[0] is the global scope */

#define MAXINSTRS    4000	/* The maximum number of instructions totally */

#define MAXGLBSYMS	200

#define MAXLOCALSYMS 5000

extern SELM* curfn;
extern int ydebug;
extern int in_declaration;

static char opname[][40]={"APPLY","TRANSLATE","ROTATE","SCALE", "SHEAR","BEGIN","END","SWEEP","APPLY_MAT","SET_MOTION",\
		  		"SET_ORIGIN","SET_CENTRE",\
		  		"FLIP","MAT_INIT","MAT_MERGE",\
		  		"STITCH","PRINT","READ","CLRSCR","NEW","SET_TEXTURE","SET_TEXTURE_FACTOR","SET_TEXTURE_DIR",\
		  		"MUL","ADD","SUB","NEG","DIV","CALL","MOV","RET","WHILE","FOR","DO","IF","IF_ELSE"};

static char * to_string(int opcode)
{
		  return opname[opcode];
}

int* symtabtop;  /* This stores the current top of each symbol table */
static SELM* *scopelist;   /* Array of symbol tables */ 

static int localslist_top=0;
static SELM* localslist;   /* All local symbols are created in this (we can do this because after the parser finishes with
										one local scope, it doesnt have to come back to it. But it has to keep coming back to the
										global scope for every external declaration/fn definition */

int instrlist_top=0;

IELM* instrlist;

#define GLOBAL 0
/* Every variable needs to have its scope set to the correct scope so that we know
 * which function it was declared in */
int currentscope=0;			   /* currentscope is set to scopetop in init_new_scope() */
int scopetop=0;               /* Keeps track of the number of scopes we have seen */
int varsoffset=0;				   /* The variables are installed after the predefined 
											identifiers (like 'int','<','>', etc). varsoffset
											keeps track of where the variables start in the
											global symbol table. Used only for displaying info. */
SELM *mainptr;						/* This points to the 'main' function */
static char errmsg[250];

static int i;


/* This is the stack for passing parameters to functions */
static SELM fncallstack[MAXCALLDEPTH];
static int fncalltop=0;   /* The stack top */

/* globalfn is contains the global scope. */
static SELM globalfn;  
SELM* get_globalfn()
{
	return &globalfn;
}

#define PRN(s) s?s->name:"NULL"

void start_local_scope(SELM *curfn)
{
	currentscope=scopetop;
	symtabtop[currentscope]=0;
	if (ydebug) fprintf(stderr,"Initialising new scope: localslist_top=%d, instrlist_top=%d\n",localslist_top,instrlist_top);
	scopelist[currentscope]=localslist+localslist_top;
	if (localslist_top>=MAXLOCALSYMS) err_exit("Run out of local scope tables!");
	scopetop++;

	curfn->felm=(FELM*)malloc(sizeof(FELM));
	curfn->felm->instrlist=instrlist+instrlist_top;
}

void end_local_scope(int ninsns)
{
	localslist_top+=symtabtop[currentscope];
	instrlist_top+=ninsns;
}

static struct SymbolTabElm * install_id_4(char *idstr,int type,int attr, int which);
static struct SymbolTabElm * install_id_3(char *idstr,int type,int attr);
static void init_global_scope()
{
	currentscope=GLOBAL;
	symtabtop[GLOBAL]=0;
	scopelist[GLOBAL]=(SELM*)malloc(MAXGLBSYMS*sizeof(SELM));
	scopetop++;
	/* Reserved words are present only in global scope. */

	/* all the inbuilt types */
	install_id_3("int",   T_int,   RESERVED);
	install_id_3("float", T_float, RESERVED);
	install_id_3("Point", T_point, RESERVED);
	install_id_3("Quad",  T_quad,  RESERVED);
	install_id_3("Line",  T_line,  RESERVED);
	install_id_3("Object",T_object,RESERVED);
	install_id_3("Matrix",T_matrix,RESERVED);
   install_id_3("char",  T_char,  RESERVED);
   install_id_3("void",  T_void,  RESERVED);
	install_id_3("string",T_string,RESERVED);	

	/* inbuilt methods */
   install_id_4("begin",    Identifier, RESERVED | BUILTIN_METHOD, BEGIN);
   install_id_4("end",      Identifier, RESERVED | BUILTIN_METHOD, END  );
   install_id_4("translate",Identifier, RESERVED | BUILTIN_METHOD, TRANSLATE);
   install_id_4("rotate",   Identifier, RESERVED | BUILTIN_METHOD, ROTATE);
   install_id_4("scale",    Identifier, RESERVED | BUILTIN_METHOD, SCALE);
   install_id_4("shear",    Identifier, RESERVED | BUILTIN_METHOD, SHEAR);
   install_id_4("merge",    Identifier, RESERVED | BUILTIN_METHOD, MAT_MERGE);
   install_id_4("apply",	 Identifier, RESERVED | BUILTIN_METHOD, APPLY);
   install_id_4("sweep",    Identifier, RESERVED | BUILTIN_METHOD, SWEEP);
   install_id_4("applyMat", Identifier, RESERVED | BUILTIN_METHOD, APPLY_MAT);
   install_id_4("setMotion", Identifier, RESERVED | BUILTIN_METHOD, SET_MOTION);
   install_id_4("setOrigin", Identifier, RESERVED | BUILTIN_METHOD, SET_ORIGIN);
   install_id_4("setCentre", Identifier, RESERVED | BUILTIN_METHOD, SET_CENTRE);
//   install_id_4("new",      Identifier, RESERVED | BUILTIN_METHOD, NEW  );

   install_id_4("flip",	 Identifier, RESERVED | BUILTIN_METHOD, FLIP);
   install_id_4("init",	 Identifier, RESERVED | BUILTIN_METHOD, MAT_INIT);

	/* inbuilt functions */
   install_id_4("printf",   	Identifier, RESERVED | BUILTIN_FN,     PRINT);
   install_id_4("read",     	Identifier, RESERVED | BUILTIN_FN,     READ);
	install_id_4("clrscr",	 	Identifier, RESERVED | BUILTIN_FN,     CLRSCR);
   install_id_4("stitch",   	Identifier, RESERVED | BUILTIN_FN,     STITCH);
   install_id_4("setTexture", Identifier, RESERVED | BUILTIN_FN, 		SET_TEXTURE);
   install_id_4("setTextureDir",  Identifier, RESERVED | BUILTIN_FN, 	SET_TEXTURE_DIR);
   install_id_4("setTexFactor", Identifier, RESERVED | BUILTIN_FN, 	SET_TEXTURE_FACTOR);

	/* keywords */
   install_id_3("new",  	K_new	,  RESERVED);
	install_id_3("if",      K_if,    RESERVED);
	install_id_3("else",    K_else,  RESERVED);
	install_id_3("while",   K_while, RESERVED);
	install_id_3("do",	   K_do,    RESERVED);
   install_id_3("for",     K_for,   RESERVED);
   install_id_3("return",  K_return,RESERVED);


   install_id_3("=",K_as,RESERVED);      /* assignment operator */
   install_id_3("==",K_eq,RESERVED);
   install_id_3(">",K_gt,RESERVED);
   install_id_3(">=",K_ge,RESERVED);
   install_id_3("<",K_lt,RESERVED);
   install_id_3("<=",K_le,RESERVED);
   install_id_3("<>",K_ne,RESERVED);
	install_id_3("!=",K_ne,RESERVED);
}

void init_parser()
{
	scopelist=malloc(sizeof(SELM*)*MAXSCOPELIST);
	if (!scopelist) yyerror("Not enough memory\n");
	symtabtop=malloc(sizeof(int)*MAXSCOPELIST);
	if (!symtabtop) yyerror("Not enough memory\n");
	instrlist=malloc(sizeof(IELM)*MAXINSTRS);
	if (!instrlist) yyerror("Not enough memory\n");
	localslist=malloc(sizeof(SELM)*MAXLOCALSYMS);
	if (!localslist) yyerror("Not enough memory\n");
	scopetop=0;
	currentscope=0; /* GLOBAL */
	for (i=0;i<MAXSCOPELIST;i++) scopelist[i]=NULL;
	init_global_scope();
	varsoffset=symtabtop[GLOBAL];
	curfn=get_globalfn();
	curfn->name=(char*)strdup("global");
	curfn->felm=(FELM*)malloc(sizeof(FELM));	
	curfn->felm->scopeno=0;  
}


static SELM* install_id_4(char *idstr,int type,int attr, int which)
{
		  	SELM *s;
			s=install_id(idstr,type);
			s->attr=attr;
			s->which=which;
			return s;
}

static SELM* install_id_3(char *idstr,int type,int attr)
{
		  SELM *s;
		  s=install_id(idstr,type);
		  s->attr=attr;
		  return s;
}


// called directly to make temporaries in ucc.y
SELM* install_id(char *idstr,int type)
{
	int symtt=symtabtop[currentscope];
	SELM* table=scopelist[currentscope];
	SELM* elm=NULL;
	elm=&table[symtt];
	symtt++;
	if (symtt>=MAXSYMBOLS) err_exit("Symbol table overflow\n");
	symtabtop[currentscope]=symtt;

	elm->name=(char*)strdup(idstr);
	elm->type=type;
	elm->fn=NULL;
	elm->felm=NULL;
	elm->attr=elm->which=0;
	return elm;
}

SELM* search_tables(int scope, char *idstr)
{
	int symtt=symtabtop[scope];
	SELM* table=scopelist[scope];
	int i;
	if (symtt==0) return NULL;
	for (i=0;i<symtt;i++) 	
		if (!strcmp(idstr,table[i].name)) {
				  return &table[i];
				  break;
		}			
	return NULL;
}

SELM* symtab_lookup(char *idstr,int type)
{
	SELM* elm=NULL;
	elm=search_tables(GLOBAL,idstr);
	/* Found in global scope.
	 * in_declaration is set to true, at the start of a declaration list (wether local/global/params/fnnames)
	 * This is done at the 'type' production. So when yacc sees any type, it just sets in_declaration to true.
	 * This means that you are using type for any other purpose you better have another rule for type. 
	 * In the earlier method, we just added an entry in the symbol table for every identifier.
	 * (Since we can't say if this is a declaration of a new local identifier that shadows a global, or a use of
	 * a global). The reason I changed it from the above to the current (using in_declaration), is because using the
	 * above required you to make sure that the symbol table entry you are pointing to, is the correct global one and
	 * not the invalid local one (in case you are accessing a global variable).
	 * The third option would be to delete the local reference thus created by the lexer after verifying that it is
	 * a global one in yacc and setting your pointer to the correct global entry (this whole thing would be 
	 * pretty inefficient - every reference to a global variable would have to be deleted - especially since the 
	 * symbol table is an array).
	 */

	if (elm) {
 				/* Check if this item was already declared, and if we are declaring it again.
				 * Since a function definition starts with a type, in_declaration would be true. So if
				 * a function is defined a second time this would catch it.
				 */
			   if ( (elm->type==Identifier) && in_declaration) {
			  		sprintf(errmsg,"%s - duplicate declaration.\n",idstr);
					yyerror(errmsg);
				}			
				/* If we are in GLOBAL scope or if this is a reserved item or if this is not a declaration, return elm */
				if ( (currentscope==GLOBAL) || (elm->attr & RESERVED) || !in_declaration) return elm;  
				/* If this is a declaration, then we are declaring a local variable that shadows a global.*/
	}

	elm=search_tables(currentscope,idstr);
	/* type cant be funtion identifier - no functions can be defined in local scope */
	if ( type==Identifier && elm && in_declaration) {
			  	sprintf(errmsg,"%s - duplicate declaration.\n",idstr);
				yyerror(errmsg);
	}			
	if (elm) return elm;  
	/* this item hasn't been declared */
	if (!in_declaration) {
			  	sprintf(errmsg,"%s - not declared.\n",idstr);
				yyerror(errmsg);
	}			
	return install_id(idstr,type);
}

SELM* get_symtab_entry(int sc,int no)
{
	return &scopelist[sc][no];
}

void err_exit(char *s)
{
	fprintf(stderr,"%s\n",s);
	exit(1);
}

void show_all_tables()
{
	int i,j;
	for (i=0;i<scopetop;i++) 	
		if (i==GLOBAL) j=varsoffset;  /* identifiers start at varsoffset in global symbol table */
		else j=0; 
		for (;j<symtabtop[i];j++) { 
			fprintf(stderr,"%d:%d %s",i,j,scopelist[i][j].name);
			if (scopelist[i][j].fn)
			  fprintf(stderr,"-->%s",scopelist[i][j].fn->name);
			else 
			  fprintf(stderr,"-->Undeclared");
			
			fprintf(stderr,"\n");
		}
}

static int do_compare(IELM* ie)
{
	float t;
	int i,j;
	int fl;
	i=ie->op1->val.v_int;
	j=ie->op2->val.v_int;

	
		switch (ie->res->type) {
		 case K_gt: fl=(i>j);
						break;	
		 case K_lt: fl=(i<j);
						break;	
		 case K_eq: fl=i==j;
						break;	
		 case K_ne: fl=i!=j;
						break;	
		 case K_le: fl=i<=j;
						break;	
		 case K_ge: fl=i>=j;
		 	break;	
		}
	if (ydebug)	fprintf(stderr,"%d %s %d --> %d\n", i,ie->res->name, j,fl);
	return fl;
}

static void print_str(char *s)
{
	while (*s) 
		if (*s=='\\') {
			s++;
			switch(*s) {
				case 'n':putchar('\n');break;
				case 'a':putchar('\a');break;
				case 't':putchar('\t');break;
				case '\\':putchar('\\'); break;
				default:putchar('\\');putchar(*s);
			}
			s++;
		} else putchar(*s++);
}

void show_info(IELM *ie)
{
  SYM_list *list=ie->list;
  fprintf(stderr,"%10s:%3d: %5s %5s,%5s-->%5s",ie->fname,ie->lineno,to_string(ie->opcode),PRN(ie->op1),PRN(ie->op2),PRN(ie->res));
  if (list) {
		 fprintf(stderr,"   :list:-");
		 for (;list;list=list->next) 
		 	fprintf(stderr,"%5s:",list->elm->name);
  }
} 

int type_check(SELM *s1, SELM* s2)
{
	if (s1->vartype==T_dynamic) s1->vartype=s2->vartype;
	if (s2->vartype==T_dynamic) s2->vartype=s1->vartype;
	if ( (s1->vartype==T_float && s2->vartype==T_int) || \
						 (s1->vartype==T_int	&& s2->vartype==T_float))
			  return 0;
	if (s1->vartype!=s2->vartype) return 1;
	return 0;
}

static void myerror(IELM *ie,char *s)
{
	if (ie) show_info(ie);
	printf("%s\n",s);

	
	exit(0);
}

static float get_float(SELM *s)
{
  	if (s->vartype==T_float) return s->val.v_float;
	else if (s->vartype==T_int) return (float)s->val.v_int;
	else myerror(NULL,"ERROR:Float or Int expected\n"); 
}

static void assign_val(SELM *s, float t)
{
	if (s->vartype==T_float)
	  	s->val.v_float=t;
	if (s->vartype==T_int)
	  	s->val.v_int=(int)t;
}

static void copy_value(SELM *src, SELM *dst)
{
		  		type_check(src,dst);
				dst->val=src->val;
				if (src->vartype==T_float || src->vartype==T_int)
						  assign_val(dst,get_float(src));
}

static Matrix3D *new_mat(Matrix3D *m, long i, long N)
{
		  InitMatrix3D(m);
			  	Rotate(m,0,i*360/N,0);
				//Translate(m,0,0,-10);
	return m;
}

static long tid_list[MAXTEX];
static void start_exec(SELM* fn,int start,int end)
{
	int ip,fl,N;
	IELM* ie;SELM *s,*s2;
	SYM_list *list,*list2;
	char tmp[80];
	float e[10];
	float t;

	for (ip=start;ip<=end;ip++) {
		ie=&fn->felm->instrlist[ip];
		if (ydebug) {fprintf(stderr,"%5d:",ip),show_info(ie);fprintf(stderr,"\n");}
		
		list=ie->list; s=ie->op1; /* Lets keep these as default */
		i=0;							  /* Just in case we forget. */
		switch(ie->opcode) {
			case CALL:
				fl=0;
				if (fn==ie->op1)  /* recursive call? */
					fl=1;
				
				if (fl) { /* Save into stack, the previous formalparams, if recursive.
							  * We detect only if a function calls itself. 
						  	  * Because of doing it this way, we can't have mutual recursion.*/
					list=ie->op1->felm->formalparams;
					N=MAXCALLDEPTH-fncalltop;
					FOR_EACH(list,i,N)
				     fncallstack[fncalltop++]=*list->elm;
				  	if (list) myerror(ie,"ERROR:Stack overflow\n");
				}
				list=ie->op1->felm->formalparams;
				list2=ie->list;
				FOR_EACH_2(list,list2) { /* copy into target fn's formal param list */
					copy_value(list2->elm,list->elm);
				}
				s->felm->ti=fn->felm->ti;
				s->felm->tex_dir=fn->felm->tex_dir;
				builder_set_texinfo(s->felm->ti);
				builder_set_tex_dir(s->felm->tex_dir);

				start_exec(ie->op1,0,ie->op1->felm->instrcnt-1);
				builder_set_texinfo(fn->felm->ti);
				builder_set_tex_dir(fn->felm->tex_dir);
				if (fl) {
					list=ie->op1->felm->formalparams;
					FOR_EACH(list,i,N) 
				     *list->elm=fncallstack[fncalltop--];
				}
				list=ie->op1->felm->formalparams;
				list2=ie->list;
				FOR_EACH_2(list,list2) /* Pass by reference is handled by copy return */
					if (list->elm->ispointer) *list2->elm=*list->elm;
				break;
			case RET:
				//	if (fn->felm->ti) free(fn->felm->ti);
					copy_value(ie->op1,ie->res);return;
			case MUL:
				t=get_float(ie->op1)*get_float(ie->op2);
				assign_val(ie->res,t);
				break;
			case ADD:
				t=get_float(ie->op1)+get_float(ie->op2);
				assign_val(ie->res,t);
				break;
			case SUB:
				t=get_float(ie->op1)-get_float(ie->op2);
				assign_val(ie->res,t);
				break;
			case NEG:
				t=-get_float(ie->op1);
				assign_val(ie->res,t);
				break;
			case DIV:	
				t=get_float(ie->op1)/get_float(ie->op2);
				assign_val(ie->res,t);
				break;
			case MOV:
				copy_value(ie->op1,ie->res);
				break;
			case NEW:
				s=ie->op1;
				i=0;
				list=ie->list;
				if (!list && s->vartype!=T_matrix) myerror(ie,"ERROR:No parameters to new()\n");
				switch (s->vartype) {
						  case T_point:
									 if (list->elm->vartype==T_point) {
												s->val.g_handle=point_copy(list->elm->val.g_handle);
									 } else {
												FOR_EACH(list,i,3) 
														  e[i]=get_float(list->elm);
												if (list || i<3) 	
														  myerror(ie,"ERROR:(Point) p=new(float x, y, z);\n");
												s->val.g_handle=point_new(e[0],e[1],e[2]);
									 } 
									 break;
						  case T_quad:
									 if (list->elm->vartype==T_quad) {
												s->val.g_handle=quad_copy(list->elm->val.g_handle);
									 } else {
												FOR_EACH(list,i,4)  
														  e[i]=list->elm->val.g_handle;
									 			if (list || i<4) myerror(ie,"ERROR:(Quad) q=new((Point)p1, p2, p3, p4);\n");
												s->val.g_handle=quad_new(e[0],e[1],e[2],e[3]);
									 }
									 break;
							case T_line:
									 if (list->elm->vartype==T_line) 
												s->val.g_handle=line_copy(list->elm->val.g_handle);
									 else myerror(ie,"ERROR:(Line) l=new (l1);\n");
									 break;
							case T_object:
									 if (list->elm->vartype==T_object)
												s->val.g_handle=object_copy(list->elm->val.g_handle);
									 else myerror(ie,"ERROR:(Object) o=new (o2);\n");
									 break;
							case T_matrix:
									 if (!list) s->val.g_handle=matrix_new();
									 else if (list->elm->vartype==T_matrix)
												s->val.g_handle=matrix_copy(list->elm->val.g_handle);
									 else myerror(ie,"ERROR: (Matrix) m=new (); or new (Matrix);\n");
									 break;
						  default:
									 myerror(ie,"ERROR:New operates only on Point and Quad");
				}
				break;

			case SET_TEXTURE: {
					long i=0;
					if (!list) {
							 if (fn->felm->ti) free(fn->felm->ti);
							 fn->felm->ti=NULL;
							 builder_set_texinfo(NULL);
							 break;
					}
					if (fn->felm->ti) free(fn->felm->ti);
					FOR_EACH(list,i,MAXTEX) 
						tid_list[i]=builder_texlist_install(list->elm->val.p_char);
					if (list) myerror(ie,"ERROR: setTexture(...); Exceeded buffer size.\n");
					fn->felm->ti=builder_new_texinfo(tid_list,i,1,1);
					break;
									}
			case SET_TEXTURE_FACTOR:
					FOR_EACH(list,i,2)
							  e[i]=get_float(list->elm);
					if (list || i<2) myerror(ie,"ERROR: setTexFactor(float,float);\n");
					if (!fn->felm->ti) myerror(ie,"ERROR: setTexFactor called without a setTexture call.\n");
					builder_set_tex_factor(e[0],e[1]); // the builder's current texinfo should be fn->felm->ti
					break;
			case SET_TEXTURE_DIR:
					if (!list || list->next) myerror(ie,"ERROR: setTexDir(String);\n");
					fn->felm->tex_dir=list->elm->val.p_char;
					builder_set_tex_dir(fn->felm->tex_dir);
					break;


			case FLIP: 
					if (!ie->op1->vartype==T_quad) myerror(ie,"ERROR:quad.flip()\n");
					quad_flip(ie->op1->val.g_handle);
					break;

			case BEGIN:
					if (ie->op1->vartype==T_object)
							  ie->op1->val.g_handle=object_begin();
					else if (ie->op1->vartype==T_line)
							  ie->op1->val.g_handle=line_begin();
					else myerror(ie,"ERROR:Object.begin() or Line.begin()\n");

					break;
			case END:
					if (ie->op1->vartype==T_object)
							  object_end(ie->op1->val.g_handle);
					else if (ie->op1->vartype==T_line)
							  line_end(ie->op1->val.g_handle);
					else myerror(ie,"ERROR:Object.end() or Line.end()\n");

					break;
			case APPLY:
					object_apply(ie->op1->val.g_handle);
					break;
			case APPLY_MAT:    /* Matrix m;   Object b;  b.applyMat(m); */
					list=ie->list;
					if (!list || list->elm->vartype!=T_matrix) 
							  myerror(ie,"ERROR:(Object/Line)b.applyMat(*m*);\n");
					if (ie->op1->vartype==T_object)
							  object_mat_apply(ie->op1->val.g_handle,list->elm->val.g_handle);
					else if (ie->op1->vartype==T_line)
							  line_mat_apply(ie->op1->val.g_handle,list->elm->val.g_handle);
					else myerror(ie,"ERROR:*(Object/Line)*b.applyMat(m);\n");

					break;

			case SET_CENTRE:
					FOR_EACH(list,i,3) 
							  e[i]=get_float(list->elm);
					if (list || i<3) myerror(ie,"ERROR:(Object) p.setCentre(float1,float2,float3);\n");
					object_set_centre(s->val.g_handle,e[0],e[1],e[2]);
					break;


			case SET_ORIGIN:
					if (list->elm->vartype==T_object) {
							if (list->next) myerror(ie,"ERROR:(Object) p.setOrigin(Object);\n");
							object_set_origin_rel(s->val.g_handle,list->elm->val.g_handle);
							break;
					}
					FOR_EACH(list,i,3) 
							  e[i]=get_float(list->elm);
					if (list || i<3) myerror(ie,"ERROR:(Object) p.setOrigin(float1,float2,float3);\n");
					object_set_origin(s->val.g_handle,e[0],e[1],e[2]);
					break;

			case SET_MOTION:
					list=ie->list;
					if (!list || list->elm->vartype!=T_matrix) 
							  myerror(ie,"ERROR:(Object)b.setMotion(*m*);\n");
					if (ie->op1->vartype==T_object)
							  object_motion_apply(ie->op1->val.g_handle,list->elm->val.g_handle);
					else myerror(ie,"ERROR:*(Object)*b.setMotion(m);\n");
					break;

			case MAT_INIT:
					if (ie->op1->vartype!=T_matrix) 
							  myerror(ie,"ERROR: (Matrix)m.init();\n");
					matrix_init(ie->op1->val.g_handle);
					break;
			
			case SHEAR:
					i=0;
					s=ie->op1;
					list=ie->list;
					if (!list) myerror(ie,"ERROR:Line/Object/Matrix p=shear(float1,float2);\n");

					FOR_EACH(list,i,2)
							  e[i]=get_float(list->elm);
					if (list || i<2) myerror(ie,"ERROR:Line/Object/Matrix p=shear(float1,float2);\n");
					if (s->vartype==T_object) object_shear(s->val.g_handle,e[0],e[1]);
					else if (s->vartype==T_line) line_shear(s->val.g_handle,e[0],e[1]);
					else if (s->vartype==T_matrix) matrix_shear(s->val.g_handle,e[0],e[1]);
					else myerror(ie,"ERROR:You can't apply this operation to this type\n");
					break;
							  
			case MAT_MERGE:
					list=ie->list;
					if (!list || list->elm->vartype!=T_matrix) myerror(ie,"ERROR:(Matrix) m.merge(matrix2);\n");
					matrix_merge(ie->op1->val.g_handle,list->elm->val.g_handle);
					break;

			case TRANSLATE:
			case ROTATE:
			case SCALE:
					i=0;
					list=ie->list;
					if (!list) myerror(ie,"ERROR:Line/Object/Matrix p=rotate/translate/scale(float1,float2,float3);\n");

					FOR_EACH(list,i,3) 
							  e[i]=get_float(list->elm);
					if (list || i<3) myerror(ie,"ERROR:Line/Object/Matrix p=rotate/translate/scale(float1,float2,float3);\n");
							  
					s=ie->op1;
					switch (ie->opcode) {
							  case TRANSLATE:
									if (s->vartype==T_object) object_translate(s->val.g_handle,e[0],e[1],e[2]);
									else if (s->vartype==T_line) line_translate(s->val.g_handle,e[0],e[1],e[2]);
									else if (s->vartype==T_matrix) matrix_translate(s->val.g_handle,e[0],e[1],e[2]);
									else myerror(ie,"ERROR:You can't apply this operation to this type\n");
									break;
							  case ROTATE:
									if (s->vartype==T_object) object_rotate(s->val.g_handle,e[0],e[1],e[2]);
									else if (s->vartype==T_line) line_rotate(s->val.g_handle,e[0],e[1],e[2]);
									else if (s->vartype==T_matrix) matrix_rotate(s->val.g_handle,e[0],e[1],e[2]);
									else myerror(ie,"ERROR:You can't apply this operation to this type\n");
									break;
							  case SCALE:
									if (s->vartype==T_object) object_scale(s->val.g_handle,e[0],e[1],e[2]);
									else if (s->vartype==T_line) line_scale(s->val.g_handle,e[0],e[1],e[2]);
									else if (s->vartype==T_matrix) matrix_scale(s->val.g_handle,e[0],e[1],e[2]);
									else myerror(ie,"ERROR:You can't apply this operation to this type\n");
									break;
					}
					break;

					

			case SWEEP:
					if (ie->op1->vartype!=T_line) myerror(ie,"ERROR:*line*.sweep(N);\n");
					if (!ie->list || ie->list->next) myerror(ie,"ERROR:sweep(*N*);\n");
					line_sweep(ie->op1->val.g_handle,(long)get_float(ie->list->elm),new_mat,1);
					break;

			case STITCH: {
					SELM *s[2];
					i=0;
					list=ie->list;
					if (!list) myerror(ie,"ERROR:stitch(line,line);\n");
					FOR_EACH(list,i,2) {
							  if (list->elm->vartype!=T_line) myerror(ie,"ERROR:stitch(*line*,*line*);\n");
							  s[i]=list->elm;
					}
					if (list || i!=2) myerror(ie,"ERROR:stitch(line,line);\n");
					line_stitch(s[0]->val.g_handle,s[1]->val.g_handle);
			}
					

			case CLRSCR: system("clear");break;
			case PRINT:
					 list=ie->list;
					 FOR_EACH_INF(list) {
							s=list->elm;
				    switch (s->vartype) {
						  case T_string:
									print_str(s->val.p_char);
									 break;
						  case T_char:			 
				  					 printf("%c",s->val.v_char);
									 break;
						  case T_int:			 
				  					 printf("%d",s->val.v_int);
									 break;
						  case T_float:			 
				  					 printf("%f",s->val.v_float);
									 break;
						  case T_point:
									 point_display(s->val.g_handle);
						  case T_quad:
									 quad_display(s->val.g_handle);
						  case T_object:
									 object_display(s->val.g_handle);
									 break;
				  }					 
				}
				break;
			case READ:
				  list=ie->list;
				  FOR_EACH_INF(list) {
							 s=list->elm;
				  		if (s->vartype==T_string) 
						{
								  fgets(tmp,80,stdin);
								  tmp[strlen(tmp)-1]=0;	
								  if (s->val.p_char) free(s->val.p_char);
								  s->val.p_char=(char*)strdup(tmp);	
						}
						if (s->vartype==T_char) scanf("%c",&s->val.v_char);
						else if (s->vartype==T_float)
								  scanf("%f",&s->val.v_float);
						else if (s->vartype==T_int)
								  scanf("%d",&s->val.v_float);
						else printf("Read of this type not supported\n");
				  }
				break;
			case WHILE:	
				while (1) {
				if (do_compare(ie))
			  	  start_exec(fn,ie->blockstart,ie->blockend);
				else {
					ip=ie->blockend;
					break;
				     }
 				}
				break;
			case DO:	
				while (1) {
			  	  start_exec(fn,ie->blockstart,ie->blockend);
				  if (!do_compare(ie)) {
					ip=ie->blockend;
					break;
				     }
 				}
				break;
			case FOR:
					for (;do_compare(ie);) 
							  start_exec(fn,ie->blockstart,ie->blockend);
				break;
			case IF:
				if (do_compare(ie)) 
				  start_exec(fn,ie->blockstart,ie->blockend);
				 ip=ie->blockend;
				 break; 
			case IF_ELSE:
				if (do_compare(ie)) 
				  start_exec(fn,ie->blockstart,ie->blockend);
				else 
				  start_exec(fn,ie->blockend+2,ie->blockend2);
				 ip=ie->blockend2;
				 break; 

		}
	}
}

void execute()
{	
	int i,j;
	if (!mainptr) err_exit("No main()!");
	fprintf(stderr,"Instructions = %d\n",mainptr->felm->instrcnt);
	start_exec(mainptr,0,mainptr->felm->instrcnt-1);
}


void display_data()
{
	int argc=1;
	char* args[]={"dummy"};
	Point *verts; Quad *faces;
	Object *objects;
	long* pnv,*pnf,*pnb;
	long ntex;
	verts=builder_get_verts(NULL);
   faces=builder_get_faces(NULL);
	objects=builder_get_objects(NULL);
   pnv=builder_get_nv();
   pnf=builder_get_nf();
	pnb=builder_get_nb();
	gl_init(&argc,args);
	set_mode_display_only();
	ntex=builder_get_ntex();
	if (ntex) gl_load_textures(builder_get_texlist(),ntex);
	gl_enable_lights();
	gl_run(verts,pnv,faces,pnf,objects,pnb);
}
