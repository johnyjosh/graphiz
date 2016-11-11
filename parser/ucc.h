#ifndef UCC_H
#define UCC_H

#include <stdio.h>
#include <matrix3d.h>
#include <builder.h>


#define FOR_EACH_INF(s) for (;s;s=s->next)
#define FOR_EACH(s,i,N) for (i=0;s && i<N;s=s->next,i++)
#define FOR_EACH_2(s1,s2) for (i=0;s1 && s2; s1=s1->next, s2=s2->next)


#define INCLUDEDIR "/usr/include/"

enum {DONTINSTALL} ;
enum {UINT,VAR,STRING};

#define GLOBAL			0		/* slot 0 in the scopelist array is for globals */

struct SymbolTabElm;
struct FuntionTabElm;
struct Instruction;
typedef struct SymbolTabElm 	SELM;
typedef struct FunctionTabElm FELM;
typedef struct Instruction    IELM;

typedef union {
		int	v_int;
		float v_float;
		char	v_char;
		long 	g_handle;  /* used as index into the appropriate array - verts/faces/etc */
		char* p_char;
} Value;

#define RESERVED	 	  (1)
#define BUILTIN_FN 	  (1<<1)
#define BUILTIN_METHOD (1<<2)

/* The builtin methods/fn are caught in this way. They are of the same type as any identifier, but their attr
 * is set appropriately in install_id_3(). In the parser, the code for handling a function call does special
 * case checking based on attr to see if this is a builtin call. If so it just does a geninstr() using the
 * opcode in the 'which' field.
 */
 
struct SymbolTabElm {
	char *name;
	int  attr;         /* reserved/builtin method/builtin fn */
	int  which;			 /* holds the opcode for a builtin method/fn */
	int  type;         /* Symbol type */     
/*  For variables */
	int  vartype;   
	int  ispointer;
	int  arraysz;  
	SELM* fn;      	/* Which function does this belong to */
	int   scope;    
	Value val;
/* for function declarations */
	FELM* felm;
};
/* Why do we need this list? Why not just use a common array for parsing lists? If you use a global array to build
 * up lists, then you can't parse - fn_call(a,sum(4,5),5); because it involves a second list while parsing the first
 * list. */

/* I could have just added a next field in SELM itself and avoided defining this list structure, 
 * but the problem is that then you can have a symbol appearing in only 1 list at a time. I can't
 * think of why this should be a problem, but just being safe. */

struct SymbolTabElm_list;
typedef struct SymbolTabElm_list SYM_list; 
struct SymbolTabElm_list { 
		 SELM *elm;
		 SYM_list *next;
};

struct Instruction {
	int opcode;          /*  MUL,ADD... */
	SELM *op1,*op2;
	SELM *res;
	SYM_list *list;  /* This is the actual parameter list, if this
									   * instruction is a call.
										* This could also be the rhs of a dynamic assignment
										* like: o1=(q1,q2,q3,q4,q6);
										*/
	unsigned int blockstart,blockend;  
	unsigned int blockend2;       
	int check;    
	/* debug info */
	int lineno;
	char *fname;
};


struct FunctionTabElm {
/* For function definitions */
	SYM_list* formalparams;  /* the formal parameters */
	int  scopeno;  
	SELM ret;
	IELM* instrlist;
	int  instrcnt;
	struct TexInfo* ti;
	char* tex_dir;  // Unless you start with a './' or '/' this directory is prefixed to the texture paths 
};

char *get_cur_filename();
int get_lineno();
int dispmsg(char *);
int yyerror(char *);
void start_new_block();
void end_current_block();
SELM* get_globalfn();
void start_local_scope(SELM *);
void end_local_scope(int);
void init_parser();
void show_info(IELM *ie);
void show_all_tables();
void initialize_scope_list();
SELM* search_tables(int scope, char *idstr);
SELM* install_id(char *idstr,int type);
SELM* symtab_lookup(char *idstr,int type);
//SELM* get_symtab_entry(int,int);
void err_exit(char *);
//void start_exec(SELM*,int,int);
void execute();

#endif
