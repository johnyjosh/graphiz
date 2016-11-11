%{
#include <ucc.h>
#include <builder.h>
#include "y.tab.h"
#include <string.h>
#define INCLMAX      10
#define MAXPATH      80


int yyparse();
int yyerror(char *s);
int ldebug=0;

YY_BUFFER_STATE include_stack[INCLMAX];
int lineno[INCLMAX];
char fname[INCLMAX][MAXPATH];


static int inclptr=0;
static char *s;
static FILE *f;
enum {STANDARD,USERDEFINED};
void handle_include_files(int);

%}

%option noyywrap
letter [a-zA-Z_]
digit  [0-9]

%x COMMENT COMMENT2

%%
[ \t]+
"/*" {BEGIN(COMMENT);}
"//" {BEGIN(COMMENT2);}
<COMMENT2>\n {BEGIN(0);lineno[inclptr]++;}
<COMMENT2>.
<COMMENT>"*/" { BEGIN(0);  }
<COMMENT>.

{letter}({letter}|{digit})* {
	yylval.symptr=symtab_lookup(yytext,Identifier);
	if (ldebug) fprintf(stderr,"ll.id: %s\n",yytext);
	return yylval.symptr->type;
}
{digit}+	{ 
	yylval.l_int=strtol(yytext,0,10);
	if (ldebug) fprintf(stderr,"ll.l_int: %d\n",yylval.l_int);
	return Uint;
}

{digit}*"."{digit}+ {
	yylval.l_float=atof(yytext);
	if (ldebug) fprintf(stderr,"ll.l_float: %f\n",yylval.l_float);
	return Float;
}

[=<>]+ {
	yylval.symptr=symtab_lookup(yytext,DONTINSTALL);
	if (ldebug) fprintf(stderr,"ll.special: %s\n",yytext);
	if (yylval.symptr==NULL) yyerror("Unrecognised symbols\n");
	else return yylval.symptr->type;
}

\"[^\0\n\"]*\"  { 
	yylval.str=strdup(yytext+1);yylval.str[strlen(yytext)-2]=0;
	if (ldebug) fprintf(stderr,"ll.str: %s\n",yytext); 
	return String;
}

\'.\' {
	if (ldebug) fprintf(stderr,"ll.char: %s\n",yytext[1]); 
	yylval.ch=yytext[1];
	return Char;
}

"#include"[ \t]+\"[^ \t\n]+\" { 
	if (ldebug) fprintf(stderr,"ll.include: %s\n",yytext); 
	handle_include_files(USERDEFINED);
}
"#include"[ \t]+<[^ \t\n]+> { 
	if (ldebug) fprintf(stderr,"ll.include: %s\n",yytext); 
	handle_include_files(STANDARD);
}


<<EOF>>  {  
		if (ldebug) fprintf(stderr,"<<EOF>>\n");
		if (--inclptr < 0) {
		yyterminate();
		} else {
			yy_delete_buffer(YY_CURRENT_BUFFER);
			yy_switch_to_buffer(include_stack[inclptr]);
/*			fprintf(stderr,"<<back>>\n");*/
		}
	}

<COMMENT,INITIAL>\n lineno[inclptr]++;

.  {	
	    return yytext[0];
	}
%%



char *get_cur_filename()
{
	return fname[inclptr];
}

int get_lineno()
{
	return lineno[inclptr];
}

int yyerror(char *s)
{
	fprintf(stderr,"%s:%d %s\n",fname[inclptr],lineno[inclptr],s);
	exit(1);
}

int dispmsg(char *s)
{
	fprintf(stderr,"%s:%d %s",fname[inclptr],lineno[inclptr],s);
	return 1;
}


main(int argc,char * argv[])
{
	lineno[0]=1;inclptr=0;
	if (argc<2) err_exit("syntax : ucc <filename>\n");
	f=fopen(argv[1],"r");
	if (f==NULL) err_exit("unable to open source file\n");
	
	yyin=f;
	strcpy(fname[0],argv[1]);
	init_parser();
	builder_alloc_mem(50000);
   yyparse();
	show_all_tables();
	initMath();
	fprintf(stderr,"---------Executing---------\n");
	execute();
	display_data();
}


void handle_include_files(int e)
{
/*		fprintf(stderr,"<<#include>>\n");*/
		if ((inclptr+1) >= INCLMAX) {
			yyerror("Include files too deeply nested\n");
			exit(1);
		}
		include_stack[inclptr]=YY_CURRENT_BUFFER;
                s=yytext; 
			
		fname[inclptr+1][0]=0;

		if (e==STANDARD) {
			while (*s++!='<');
			strcat(fname[inclptr+1],INCLUDEDIR);
		}
		else 
			while (*s++!='"');
		strcat(fname[inclptr+1],s);
		s=fname[inclptr+1];
		s[strlen(s)-1]=0;
		if (!(f=fopen(s,"r"))) {
			fprintf(stderr,"unable to open %s\n",s);
		}
		else {
			inclptr++;
			lineno[inclptr]=1;	
			yyin=f;
/*			fprintf(stderr,"<<Now working on %s>>\n",s);*/
			yy_switch_to_buffer(yy_create_buffer(yyin,YY_BUF_SIZE));
		}
}
