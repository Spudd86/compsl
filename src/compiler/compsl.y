 	


%parse-param { char const *file_name };
%initial-action
{	
	//printf("Parsing file: %s",file_name);
};


%{
#include <stdio.h>
#include <string.h>
//#include "node.h"

#define YYERROR_VERBOSE 1

extern FILE *yyin;

int goparse(char* fn) {
	yyin = fopen( fn, "r" );
	yyrestart(yyin);
	return yyparse(fn);
}

void yyerror(const char *fn, const char *msg) {
    fprintf(stderr,"> In file \"%s\"\n  Error: \"%s\"\n  Line Num: %i\n",fn,msg,-1);//yylloc.first_line);
	exit(2);
}
 
int yywrap() {
        return 1;
} 
  
//main() {
//        yyparse();
//        printf("Parsing successful\n");
//} 

%}

%union {
	int ival;
	float fval;
	char* sval;
//	node* nv;
}

%token INT_LIT FLOAT_LIT CUBBY GLOBAL DECLARE INT BOOL FLOAT TRUE FALSE IF ELSEIF ELSE WHILE BREAK RETURN IDENTIFIER SEMI COMA OPENB CLOSEB OPENP CLOSEP PLUS MINUS MULT DIV MOD ISEQ ISNEQ ASSIGN ISGEQ ISLEQ ISGT ISLT NOT AND OR;

%type <nv> file header_stuff do_declare cubbys decls block stmts stmt control expression;
%type <fval> FLOAT_LIT;
%type <ival> INT_LIT;
%type <sval> IDENTIFIER;

%locations

%%
file:
		header_stuff do_declare cubbys
        ;

header_stuff: 
		;
		
do_declare:
		DECLARE OPENB decls CLOSEB;

cubbys:
		cubby cubbys | ;
cubby:
		CUBBY IDENTIFIER block {
			//printf("Cubby declared %i\n",yyget_lineno()); //doesnt get line no. dunno what that gets
		};
block:
		OPENB stmts CLOSEB | stmt SEMI;

stmts:
		stmt SEMI stmts 	|
		control stmts | ;
stmt:
		expression | RETURN | BREAK
		;
		
expression:
		OPENP expression CLOSEP;
		|
		math
		|
		IDENTIFIER OPENP expression CLOSEP
		|
		IDENTIFIER OPENP CLOSEP
		|
		retable
		|
		IDENTIFIER ASSIGN expression
		|
		cast expression
		;
		

cast: 
		OPENP INT CLOSEP 
		| 
		OPENP FLOAT CLOSEP
		;
		
math:
		expression PLUS expression | 
		expression MINUS expression;
		
		
retable:
		IDENTIFIER 
		| 
		FLOAT_LIT{ //printf("%f\n",yylval.fval);
			} 
		| 
		INT_LIT { //printf("%i\n",yylval.ival); 
			}
		;


control: 
		IF OPENP expression CLOSEP block else
		|
		WHILE OPENP expression CLOSEP block
		;

else:
		ELSEIF OPENP expression CLOSEP block else
		|
		ELSE block
		|
		;
		

decls:
		decl SEMI decls | ;
			
decl:	
		modifiers INT ident_list
		|
		modifiers FLOAT ident_list 
		;
		
modifiers:
		GLOBAL | ;

		
ident_list:
		IDENTIFIER more_ident_list;
more_ident_list:
		COMA IDENTIFIER more_ident_list
		|
		;		
				
%%
