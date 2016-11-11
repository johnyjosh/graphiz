%{
#include <ucc.h>
#include <op_defs.h>

#include <string.h>
#include <assert.h>
#include <common.h>


#define MAXBLOCKDEPTH 40

int ydebug=0;

extern int currentscope;
extern SELM* mainptr;
extern IELM* instrlist;
extern int instrlist_top;

int in_declaration;

SELM* curfn=NULL;    /* to maintain current scope */

static int tvarcnt=0;

static unsigned int blockstack[MAXBLOCKDEPTH];
static int   blocktop;

static SELM *symptr;
static SYM_list *t_symlist;
static IELM *iptr;
static int ispointer;
static int i,max;
static int oldip;
static char errstr[90];

void init_fields(FELM*);
void free_all(SYM_list *head);
int yylex();
%}

%union {
IELM *ielm;
SELM *symptr;
SYM_list *symlist;
SELM *con[3];
char *str;
int  l_int;
char ch;
float l_float;
}


%token <l_float> Float
%token <l_int>  Uint 
%token <str>    String
%token <ch>     Char

%token <symptr> Identifier T_int T_char T_void T_string T_point T_quad T_object T_line T_dynamic T_float T_matrix
%token <symptr> K_if K_else K_while K_do K_new
%token <symptr> K_for K_return 
%token <symptr> K_as K_eq K_gt K_ge K_lt K_le K_ne 
/*%token <symptr> K_begin K_end K_printf K_read K_clear K_translate K_rotate*/
  
%nonassoc IFX
%nonassoc K_else

%left '-' '+'
%left '*' '/'
%nonassoc UMINUS	/* nonassoc just names a precedence. Now you can use UMINUS to assign precedence */

%type <con>    conditional
%type <symptr> type type1 onevardecl rel_op
%type <symptr> expression assignment fncall methodcall rhs functiondef newobject
%type <ielm>	for_assignment
%type <symlist> idlist formalparameterlist actualparams 

%%
programstmnts: 
	    programstmnts extstatement  {currentscope=0;curfn=get_globalfn();} /* Global scope - curfn is set to globalfn */
	    |extstatement  {currentscope=0;curfn=get_globalfn();}

extstatement:
	 functiondefinition {
			if (ydebug) dispmsg("function definition\n");
	}
	 | vardeclaration {if (ydebug) dispmsg("ext variable declaration\n"); } /* to catch variables declared outside any fns 
	 																				 * Such variables would have fn set to globalfn */

vardeclarations: vardeclaration  {if (ydebug) dispmsg("internal var declaration\n");} 
		| vardeclarations  vardeclaration {if (ydebug) dispmsg("internal var declarations\n");}

vardeclaration: type idlist ';' {
		if (ydebug) fprintf(stderr,"curfn=%s",curfn->name);
		t_symlist=$2;
		FOR_EACH_INF(t_symlist) {
			if (ydebug) fprintf(stderr,":%s:",t_symlist->elm->name);
			t_symlist->elm->vartype=$1->type;
			t_symlist->elm->fn=curfn;
			t_symlist->elm->scope=currentscope;
		}
		free_all($2);
		if (ydebug) fprintf(stderr,"\n");
		in_declaration=false;
		}

idlist: onevardecl {
				$$=new_symlist($1);
				}
		| idlist ',' onevardecl  {
				$$=add_symlist($1,$3);
				}

onevardecl: Identifier { 
			$1->arraysz=0;
			$1->ispointer=false; 
			$$=$1;
		  }

functiondefinition:  functiondef functionbody {end_local_scope($1->felm->instrcnt);}

functiondef:	type  Identifier  '(' {
				if (ydebug) fprintf(stderr,"New scope init\n");
				start_local_scope($2); 						/* by doing this, the parameters appear to be in */
			} formalparameterlist')' {         /*	the same scope as locals 							 */    
				in_declaration=false;
				if (ydebug) fprintf(stderr,"%s(",$2->name);		
				set_curfn_info($2,reverse_symlist($5),$1);
				if (ydebug) fprintf(stderr,")\n");
				if (!strcmp($2->name,"main")) mainptr=$2;
				$$=$2;
			}

functionbody: '{'vardeclarations manystatements'}' 
		|'{' manystatements '}'
	

formalparameterlist: 
		  type Identifier {
					$2->vartype=$1->type;
					$2->ispointer=false;
					$$=new_symlist($2);
				} 

		  | formalparameterlist ',' type Identifier {
					$4->vartype=$3->type;
					$4->ispointer=false;
					$$=add_symlist($1,$4);
			}
			| {$$=NULL;}





statement:
	 assignment  ';'  { if (ydebug) dispmsg("assignment\n");}
	 |expression ';'  { if (ydebug) dispmsg("expression\n");}
	 |fncall     ';'  { if (ydebug) dispmsg("function call\n");}
	 |newobject ';'
	 |methodcall ';'  { if (ydebug) dispmsg("method call\n"); }
	 |K_return rhs ';' {geninstr(RET,$2,$2,&curfn->felm->ret,0);}
	 |ifstmt  
	 |whileloop  {if (ydebug) dispmsg("while {} \n"); }
	 |doloop ';' {if (ydebug) dispmsg("do while \n"); }
	 |forloop    {if (ydebug) dispmsg("for loop \n"); }
	 ;
	 
	 

for_assignment:
	Identifier '+' '+' {
		symptr=new_tmp();
		symptr->val.v_float=1;
		$$=(IELM*)malloc(sizeof(IELM));
		set_instr($$,ADD,$1,symptr,$1,NULL);
	}
	|Identifier K_as Identifier '+' Uint {
		symptr=new_tmp();
		symptr->val.v_float=$5;
		$$=(IELM*)malloc(sizeof(IELM));
		set_instr($$,ADD,$3,symptr,$1,NULL);
	}
	|Identifier K_as Identifier '+' Identifier {
		$$=(IELM*)malloc(sizeof(IELM));
		set_instr($$,ADD,$3,$5,$1,NULL);
	}
	;

assignment:
	Identifier K_as rhs { 	
		
		if (!$3) yyerror("You can't do an assignment. This fn/method has got nothing to return.\n");
		//if ($3->attr & BUILTIN_METHOD) yyerror("This is a method. You can't use it as a function.\n");
		if (type_check($3,$1)) yyerror("Type mismatch\n");
		geninstr(MOV,$3,NULL,$1,0); 
		$$=$1;
	}
	/*| Identifier '+' '+' {   - gives me shift/reduce conflict. How to do autoincr?
		geninstr(AUTOINCR,$1,0,0,0);
		$$=$1;
	}*/
	;


rhs:
	expression
	| fncall
	| newobject
	;

	 
expression: 
	expression '+' expression {
			$$=new_tmp();
			geninstr(ADD,$1,$3,$$,0);
		}
	|expression '-' expression {
			$$=new_tmp();
			geninstr(SUB,$1,$3,$$,0);
		}
	|expression '/' expression {
			$$=new_tmp();
			geninstr(DIV,$1,$3,$$,0);
		} 
	|expression '*' expression { 
			$$=new_tmp();
			geninstr(MUL,$1,$3,$$,0);
		}
	| '-' %prec UMINUS expression	{
			$$=new_tmp();
			geninstr(NEG,$2,NULL,$$,0);
		}
	| '+' %prec UMINUS expression	{
			$$=$2;
		}
	|'(' expression ')' { $$=$2;}
	| Uint    {
		$$=new_tmp();
		$$->vartype=T_int;
		$$->val.v_int=$1;
	}
	| Float {
		$$=new_tmp();
		$$->vartype=T_float;
		$$->val.v_float=$1;
	}
	| String  {
		$$=new_tmp();
		$$->vartype=T_string;
		$$->val.p_char=(char*)strdup($1);
	 }
	| Char  {
		$$=new_tmp();
		$$->vartype=T_char;
		$$->val.v_char=$1;
	 }
   | Identifier
	;
	
methodcall: Identifier '.' fncall {
						  if ($3->attr & BUILTIN_METHOD) { /* weed out the builtin methods*/
									 geninstr($3->which,$1,NULL,NULL,reverse_symlist(t_symlist));
						  } else {
						  		sprintf(errstr,"%s not a built-in method\n",$3->name);
								yyerror(errstr);
						  }
				}

newobject: K_new type1 '(' actualparams ')' {
			   		$$=new_tmp();
						$$->vartype=$2->type;
						geninstr(NEW,$$,NULL,NULL,reverse_symlist($4));
			  }

fncall: Identifier '(' actualparams ')'  {
			if ($1->attr & BUILTIN_METHOD) {
				/* handled above */
				$$=$1;
				t_symlist=$3;
			} else if ($1->attr & BUILTIN_FN) {		 /* weed out the builtin calls */
			   //$$=new_tmp();
				//$$->vartype=T_dynamic;  /* gets set in the assignment that HAS to be there */
				$$=NULL; /* currently no inbuilt functions return anything */
				geninstr($1->which, NULL,NULL, NULL,reverse_symlist($3)); 
			} else {
			  $3=reverse_symlist($3);
			  check_fn_paramlist($1,$3);
			  geninstr(CALL,$1,NULL,NULL,$3);
			  $$=&($1->felm->ret);
			}
	 }

actualparams: expression {$$=new_symlist($1);}
		| actualparams ',' expression {
			$$=add_symlist($1,$3);	            
		}
		| {$$=NULL;}
		;
		
codeblock:  '{' {startnewblock();}  manystatements '}' 
		| {startnewblock();} statement 
manystatements: manystatements statement  
		    | statement  

ifstmt: K_if '(' conditional ')'  codeblock K_else  codeblock {
		if (ydebug) dispmsg("if-else\n");
		geninstr(IF_ELSE,$3[0],$3[2],$3[1],0);
		endcurrentblock();
		endcurrentblock();
	} 
        |K_if '(' conditional ')' codeblock %prec IFX {
		if (ydebug) dispmsg("if\n");
		geninstr(IF,$3[0],$3[2],$3[1],0);
		endcurrentblock();
	}

whileloop: K_while '(' conditional ')'   codeblock {
		geninstr(WHILE,$3[0],$3[2],$3[1],0);
		endcurrentblock();
	    }
doloop: K_do   codeblock  K_while '(' conditional ')' {
		geninstr(DO,$5[0],$5[2],$5[1],0);
		endcurrentblock();
	    }	

forloop: K_for '(' assignment ';' conditional ';' for_assignment ')' codeblock {
		/*fprintf(stderr,"cmp %s %s %s\n",$5[0]->name, $5[2]->name, $5[1]->name);*/
		gen_for_instr($7,$5[0],$5[2],$5[1]);
		endcurrentblock();
		}

conditional:   expression rel_op expression {  $$[0]=$1;$$[1]=$2;$$[2]=$3;}

rel_op:		K_eq
			|	K_gt
			|	K_ge
			|	K_lt
			|	K_le
			|	K_ne


type: type1 {in_declaration=true;$$=$1;} /* Use type1 if you are not installing any symbols.
														  So for eg., don't use type1 when declaring new variables because
														  you will be allowed to install symbols, only if
														  in_declaration is true.*/
type1:  
	T_int  
	| T_float
	| T_char
	| T_void
	| T_string
	| T_point
	| T_quad
	| T_object
	| T_line
	| T_matrix
	
%%


/* Initialises a new function on the currentscope. 
*/

void set_curfn_info(SELM* p,SYM_list* params,SELM* ret)
{
		SYM_list *formal_params;
		curfn=p;
		init_fields(curfn->felm);
		curfn->felm->scopeno=currentscope;  /* One of the scopetable entries is allocated to this function */
		curfn->felm->ret.vartype=ret->type;
		curfn->felm->ret.ispointer=false;

		curfn->fn=get_globalfn();
		curfn->scope=GLOBAL; 			/* Every function is in the global scope */
		
		curfn->felm->formalparams=params;

		curfn->felm->ti=NULL;
		curfn->felm->tex_dir=NULL;

		if (!params) if (ydebug) fprintf(stderr,"void");
		for (;params;params=params->next) {
			if (params->elm->ispointer) 
				if (ydebug) fprintf(stderr,"&");
			if (ydebug) fprintf(stderr,"%s ",params->elm->name);
			params->elm->fn=p; params->elm->scope=currentscope;
		}
}

void init_fields(FELM* f)
{
	f->instrcnt=0;
	f->ret.name=(char*)strdup("ret");
}

/* Sets symptr to the correct symboltable entry. 
	If this is a local variable, symptr would be same as p, but if this is a global, then p
	would be pointing to the local table, while symptr would be pointing to the right global entry.
	Note that install_id creates a new entry for any identifier
	in the currentscope regardless of wether this identifier is present in
	the global scope.
*/	

int check_if_declared(SELM* p)
{
	int reterr=0; symptr=p;
	if (p->fn) return true;  /*p->fn is set to NULL by install_id, when a symbol is first installed in 
										the currentscope.  yacc would set it to curfn only if it was declared 
										properly in the currentscope. If this was declared in the global scope,
										p->fn would still be null, but the global version of this variable
										would have p->fn=globalfn.
									  */

	/* This means that a local declaration is not present. So it must 
	 * be declared globally
	 */
	if (symptr=search_tables(GLOBAL,p->name)) 
			  return true;
	else {
			  /* No it is not declared global, flag error */
			  symptr=NULL;
			  sprintf(errstr,"%s not declared\n",p->name);
			  yyerror(errstr);
			  return false;
	}		
}

void check_fn_paramlist(SELM* e, SYM_list *actual)
{
		  int i=0;
		  SYM_list *formal=e->felm->formalparams;
		  FOR_EACH_2(formal,actual) {
					 if (type_check(formal->elm,actual->elm)) {
								sprintf(errstr,"Type mismatch in parameter %i\n",i);
								yyerror(errstr);
					 }
		  }
		  if (formal || actual) {
		  		sprintf(errstr,"There is a difference in the number of arguments passed.\n");
		  		yyerror(errstr);
		  }
}


void set_instr(IELM *iptr,int opcode, SELM * op1, SELM* op2, SELM* res, SYM_list *list)
{
		  iptr->opcode=opcode;
		  iptr->op1=op1;iptr->op2=op2;
		  iptr->res=res;
	/*	  if (iptr->list) free_all(iptr->list); */
		  iptr->list=list;
		  iptr->lineno=get_lineno();
		  iptr->fname=get_cur_filename();
}		  

void gen_for_instr(IELM* incr, SELM *op1, SELM *op2, SELM *res)
{
   int ino;
	ino=blockstack[blocktop-1];
	iptr=&curfn->felm->instrlist[ino];
	set_instr(iptr,FOR,op1,op2,res,NULL);

	iptr->blockstart=ino+1;
	curfn->felm->instrlist[curfn->felm->instrcnt]=*incr; /* Insert incr here */
	if (ydebug) {
	fprintf(stderr,"The incr instr: "); show_info(&curfn->felm->instrlist[curfn->felm->instrcnt]);
	fprintf(stderr,"\n");
	}
	iptr->blockend=curfn->felm->instrcnt;

	curfn->felm->instrcnt++;
}

void geninstr(int opcode,SELM* op1,SELM* op2,SELM* res, SYM_list* list)
{
		  int ino;

		  if (opcode==WHILE || opcode==DO || opcode==IF)  { 
					 ino=blockstack[blocktop-1];
					 iptr=&curfn->felm->instrlist[ino];
					 iptr->blockstart=ino+1;
					 iptr->blockend=curfn->felm->instrcnt-1;
		  } else if (opcode==IF_ELSE) {
					 ino=blockstack[blocktop-2];
					 iptr=&curfn->felm->instrlist[ino];
					 iptr->blockstart=ino+1;
					 iptr->blockend=blockstack[blocktop-1]-1;
					 iptr->blockend2=curfn->felm->instrcnt-1;
		  } else {
					 ino=curfn->felm->instrcnt;
					 iptr=&curfn->felm->instrlist[ino];
					 curfn->felm->instrcnt++;
		  }

/*		  if (ydebug) fprintf(stderr,"gen: %s %d\n",curfn->name,ino);*/

		  if (opcode==CLRSCR && list) 
					 if (ydebug) dispmsg("Too many parameters to clrscr()!");

		  set_instr(iptr,opcode,op1,op2,res,list);

		  if (ydebug) {
		  	show_info(iptr);
		  	fprintf(stderr,"\n");	
		  }
}

void reset_tmp()
{
	tvarcnt=0;
}
SELM* new_tmp()
{
	char tmpvar[20];SELM *t;
	sprintf(tmpvar,".r%d",tvarcnt++);
	t=install_id(tmpvar,Identifier);
	t->fn=curfn;
	t->scope=currentscope;
	t->vartype=T_float;
	t->ispointer=false;
	return t;
}


void startnewblock()  /* This is used only to mark the start and end instructions, no scoping */
{
	if (blocktop>=MAXBLOCKDEPTH) err_exit("Too much nesting of blocks!");
	   if (ydebug) fprintf(stderr,"startnewblock():blockstack[%d]=%d\n",blocktop,curfn->felm->instrcnt);
	   blockstack[blocktop]=curfn->felm->instrcnt;
	   curfn->felm->instrcnt++; 
	   blocktop++;
}

void endcurrentblock()
{
	if (ydebug) fprintf(stderr,"endblock\n");
	--blocktop;
}		
	
SYM_list* new_symlist(SELM *elm)
{
	SYM_list* t;
	t=malloc(sizeof(SYM_list)); 
	t->elm=elm;t->next=NULL; 
	return t;
}

SYM_list* add_symlist(SYM_list *head, SELM *elm)
{
	SYM_list* t;
	t=malloc(sizeof(SYM_list)); 
	t->elm=elm;
	t->next=head; 
	return t;
}

SYM_list* reverse_symlist(SYM_list *head)
{
	SYM_list *save,*prev=NULL;
	while (head) {
		save=head->next;
		head->next=prev;
		prev=head; 
		head=save;
	}	
	return prev;
}	
		

void free_all(SYM_list *head)
{
	SYM_list *t=head;
	while (head) {t=head; head=head->next; free(t);}
}	
