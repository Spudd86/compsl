// $Id$

/*
    CompSL scripting language 
    Copyright (C) 2007  Thomas Jones & John Peberdy

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/ 	



%parse-param { char const *file_name };
%initial-action
{	
	//printf("Parsing file: %s",file_name);
};


%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "node.h"
#include "interncomp.h"
#include "compsl.tab.h"
#include "../extern/compsl.h"
#include "../intern/gen.h"
#include "../intern/compartment.h"
#include "../intern/builtins.h"

#define YYERROR_VERBOSE 1
  
  // NOTE: ANY INCLUDES HERE SHOULD ALSO GO IN compsl.l probably  
  
extern FILE *yyin;
  
int goparse(const char *fn, compart *com) {
	int ret;
	lineNo=1;
	
	DPRINTF("\n\n>> STARTING PARSE - %s\n",fn);
	ccompart = com;
	
	sprt=malloc(1024 * sizeof(char));
	ret = yyparse(fn);
	DPRINTF(">> DONE PARSE\n\n");
	
	free(sprt); sprt = NULL;
	return ret;
}
  
void yyerror(const char *fn, const char *msg) {
    fprintf(stderr,"> In file \"%s\"\n  Error: \"%s\"\n  Line Num: %i\n",fn,msg,lineNo);
    return; 
}

int yywrap(void) {return 1;}
///////////////////////////////////////////////////////////////////////
// END FUNC DECLS

%}

%union {
	int ival;
  	float fval;
	char *sval;
	bool bval;
	expression *expr;
	list *xlist;
	bytecode *bc;
}

%token <fval> FLOAT_LIT;
%token <ival> INT_LIT; 
%token <sval> IDENTIFIER;
%token CUBBY CUBBY2 GLOBAL INT FLOAT IF ELSE WHILE BREAK SEMI COMA OPENB CLOSEB OPENP CLOSEP DECLARE CLOSES OPENS CONTINUE;

// regular operators
%token BOR BAND SHFTL SHFTR PLUS MINUS MULT DIV MOD NOT AND OR;

%token PLUSPLUS MINUSMINUS;

// comparison, 
%token ISEQ ISNEQ ISGEQ ISLEQ ISGT ISLT;

// assign
%token ASSIGN PLUSEQ MINUSEQ MULTEQ DIVEQ MODEQ BANDEQ BOREQ; 

%type <expr> expression math retable var;
%type <ival> cast array_length assignop;
%type <sval> cubby_id;
%type <bval> global_modifier intfloat_keyword;
%type <bc> stmt block ife elsee control;
%type <xlist> paramlist stmts moreparamlist ident_list more_ident_list;
%type <void> file header_stuff do_declare cubbys cubby decl decls cubby_keyword;

%locations

%%
file: header_stuff do_declare cubbys;

header_stuff: {};
		
do_declare: {DPRINTF("Doing declare\n");}
		DECLARE OPENB decls CLOSEB {
		  DPRINTF("Done declare\n");
		}

cubbys:
		cubby cubbys {} | {};
cubby:	
		cubby_id block {
		  com_addCubby(ccompart, $2, $1);
		  free($1);
#ifdef COMP_STACKCHECK
			int rs;
			remUselessDUPs($2, bc_len($2), ccompart->vm, ccompart);
			rs = stackcheck($2, bc_len($2), ccompart->vm, ccompart);
			DPRINTF("Ran stackcheck(), result: %i\n",rs);
			if(rs!=0) 
			  internalCompileError("Unmatched bytecode!");
#endif
		}
cubby_id:
		cubby_keyword IDENTIFIER {
			DPRINTF("> Cubby: %s\n",$2);
			// MEM: Identifier free'd in 'cubby'
			$$=$2;
		}

cubby_keyword:
		CUBBY {} | CUBBY2 {};

		
block:
		OPENB stmts CLOSEB {
			DPRINTF("Parsed block with %i statements\n",$2->length);

			llist *cur=$2->head;
			bytecode *mcode;
			int len=0, cpos=0;
			
			// Get the length
			while($2->length>0 && cur) {
			  assert(cur->obj!=NULL); 
			  len+=bc_len(cur->obj);
			  cur=cur->next;
			}
			len+=1; // for the BC_END
			
			mcode = calloc(len+1,sizeof(bytecode)); 
			assert(mcode!=NULL);
			
			cur=$2->head;
			
			// Copy in
			while($2->length>0 && cur) {
				int curlen=bc_len(cur->obj);
				memcpy(&mcode[cpos], cur->obj, curlen*sizeof(bytecode));
				free(cur->obj); cur->obj = NULL;
				cpos+=curlen;
				cur=cur->next;
			}

			// Terminate
			mcode[cpos].code = BC_END;
			cpos++;

			assert(cpos==len);
			list_free($2);
			$$=mcode;
		}
		|
		stmt SEMI {
			$$=$1;
		}
		|
		SEMI {
		  bytecode *r = calloc(1,sizeof(bytecode));
		  r[0].code=BC_NONO;
		  $$ = r;
		};


stmts:
		stmts stmt SEMI {
			list_addToBack($1,$2);
			$$=$1;
		}
		|
		stmts control {
			list_addToBack($1,$2);
			$$=$1;
		}
		| 
		{
		  // This is before the first element of the stmts list		     
			$$=list_new();
		};
stmt:
		expression {
		  if($1 == NULL) {
		    compileError("Invalid expression");
		    YYABORT;
		  }
			bytecode *code = expr_toBc($1);

			if(code==(bytecode*)0) {
			  compileError("Bad bytecode");
			  YYABORT;
			}
			int l = bc_len(code);
			DPRINTF("Statement of %i bytecodes\n",l);
			
			code = realloc(code, sizeof(bytecode)*(l+2));
			if(code == NULL) internalCompileError("Out of Memory");
			$1->isLiteral = true; // Don't free bytecode
			expr_free($1);

			code[l].code = BC_DPOP;
			code[l+1].code = BC_NONO;
			$$=code;

			$$ = remUselessDUPs($$, l+1, ccompart->vm, ccompart);

#ifdef COMP_STACKCHECK
			int rs;
			rs = stackcheck($$, bc_len($$), ccompart->vm, ccompart);
			DPRINTF("Ran stackcheck() on stmt, result: %i\n",rs);
			if(rs!=0) 
			  internalCompileError("Unmatched bytecode in stmt!");
#endif
		}
        | 
        BREAK {
		  DPRINTF("Break statement\n");
		  bytecode *code = calloc(2, sizeof(bytecode));
		  code[0].code = BC_NOOP; //JMP 0 = break, tobe parsed in block
		  code[0].a = BREAK_NOOP_IDEN; 
		  code[1].code = BC_NONO;
		  $$=code;
        }
        |
        CONTINUE {
        	DPRINTF("Continue statement\n");
			bytecode *code = calloc(2, sizeof(bytecode));
			code[0].code = BC_NOOP; //NOOP 0 = continue, tobe parsed in block
			code[0].a = CONTINUE_NOOP_IDEN; 
			code[1].code = BC_NONO;        	
			$$=code;
        };
		
expression:
	OPENP expression CLOSEP {
	  $$ = $2;
	}
|
	math {
	  $$ = $1;
	}
|
	IDENTIFIER OPENP paramlist CLOSEP { 		// FUNCTION CALL
	  $$ = function_call($1, $3);
	  free($1);
	  if(NULL == $$) 
	    YYABORT;
	}
|
	retable {
	  $$ = $1;
	}
|
	IDENTIFIER ASSIGN expression {
	  $$ = assignVar($1, $3);
	  free($1);
	  if(NULL == $$) 
	    YYABORT;
	  
	}
|
	IDENTIFIER assignop expression {

	  expression *expr = readVar($1);
	  if(NULL == expr) 
	    YYABORT;

	  expr=bin_op($2,expr,$3);
	  if(NULL == expr) 
	    YYABORT;

	  $$ = assignVar($1, expr);
	  if(NULL == $$) 
	    YYABORT;
	  free($1);
	}
|
	cast expression { 
	  expr_autocast($1==FLOAT,$2);
	  $$=$2;
	}

assignop: PLUSEQ {$$=PLUS;} | MINUSEQ {$$=MINUS;} | MULTEQ {$$=MULT;} | DIVEQ {$$=DIV;} 
| MODEQ {$$=MOD;} | BANDEQ {$$=BAND;} | BOREQ {$$=BOR;} 
		
		
paramlist:
		moreparamlist {
			$$=$1
		}
		|
		{
			$$= list_new();
		}
		;

moreparamlist:	
		expression COMA moreparamlist {
			list_addToFront($3,$1);
			$$=$3;
		}
		| expression {
			list *lst = list_new();
			list_addToFront(lst,$1);
			$$=lst;
		}

		;

cast: 
		OPENP INT CLOSEP {
			$$ = INT; 
		}
		| 
		OPENP FLOAT CLOSEP {
			$$ = FLOAT;
		}
		;

		
////////////////////////////////////////////////////////////
// MATH		
////////////////////////////////////////////////////////////

/*
You need to watch out for order of operation - sometimes 
it doesn't matter like if you on your shirt before your 
pants - but sometimes it does like if you put on your pants
before you underwear, then you become superman instead 
of a man - Chang PMath331 
*/

%nonassoc     PLUSEQ MINUSEQ DIVEQ MULTEQ MODEQ BANDEQ BOREQ;
%left         OR;
%left         AND;
%left         BOR;
%left         BAND;
%nonassoc     ISEQ ISNEQ;
%nonassoc     ISGEQ ISLEQ ISGT ISLT;
%left         SHFTR SHFTL;
%left         PLUS MINUS;
%left         MULT DIV MOD;
%left         NOT; //unary ops
		

math:
expression PLUS expression {   $$=bin_op(PLUS,$1,$3);}
|
expression MINUS expression {   $$=bin_op(MINUS,$1,$3);}
|
expression MULT expression {   $$=bin_op(MULT,$1,$3);}
|
expression DIV expression {   $$=bin_op(DIV,$1,$3);}
|
expression MOD expression {   $$=bin_op(MOD,$1,$3);}
|
expression OR expression {   $$=bin_op(OR,$1,$3);}
|
expression AND expression {   $$=bin_op(AND,$1,$3);}
|
expression ISEQ expression {   $$=bin_op(ISEQ,$1,$3);}
|
expression ISNEQ expression {   $$=bin_op(ISNEQ,$1,$3);}
|
expression ISGEQ expression {   $$=bin_op(ISGEQ,$1,$3);}
|
expression ISGT expression {   $$=bin_op(ISGT,$1,$3);}
|
expression ISLEQ expression {   $$=bin_op(ISLEQ,$1,$3);}
|
expression ISLT expression {   $$=bin_op(ISLT,$1,$3);}
|
expression BOR expression {   $$=bin_op(BOR,$1,$3);}
|
expression BAND expression {   $$=bin_op(BAND,$1,$3);}
|
expression SHFTL expression {   $$=bin_op(SHFTL,$1,$3);}
|
expression SHFTR expression {   $$=bin_op(SHFTR,$1,$3);}
|
NOT expression {
  expr_ensureLit($2);
    
  int len = bc_len($2->val.bcode);
  $2->val.bcode = realloc($2->val.bcode, (len+2)*sizeof(bytecode));
  $2->val.bcode[len].code = BC_NOT;
  $2->val.bcode[len+1].code = BC_NONO;
  $$ = $2;
}
|
MINUS expression {
  expression *e = calloc(1, sizeof(expression));
  e->isLiteral = true;
  e->isFloat = false;
  e->val.in = 0;
  $$=bin_op(MINUS,e,$2);
}
;
		
retable:
var {
  $$ = $1;
}
| 
FLOAT_LIT { 
  expression *a = malloc(sizeof(expression));
  a->isFloat=true;
  a->isLiteral=true;
  a->val.fl=$1;
  $$ = a;
} 
| 
INT_LIT { 
  expression *a = malloc(sizeof(expression));
  a->isFloat=false;
  a->isLiteral=true;
  a->val.in=$1;
  $$ = a;
}
;

var:
IDENTIFIER {
  $$ = readVar($1);
  free($1);
  if(NULL == $$) {
    YYABORT;
  }
}
|
PLUSPLUS IDENTIFIER {
  $$ = incVar($2, true, false);
  free($2);
  if(NULL == $$) {
    YYABORT;
  }
}
|
IDENTIFIER PLUSPLUS {
  $$ = incVar($1, true, true);
  free($1);
  if(NULL == $$) {
    YYABORT;
  }
}
|
MINUSMINUS IDENTIFIER {
  $$ = incVar($2, false, false);
  free($2);
  if(NULL == $$) {
    YYABORT;
  }
}
|
IDENTIFIER MINUSMINUS {
  $$ = incVar($1, false, true);
  free($1);
  if(NULL == $$) {
    YYABORT;
  }
}
;

control: 
		ife
		|
		WHILE OPENP expression CLOSEP block {
		  $$ = ctrlWhile($3,$5);
		  if(NULL == $$) {
		    YYABORT;
		  }
		};

ife: 
		IF OPENP expression CLOSEP block elsee {
		  $$ = ctrlIf($3,$5,$6);
		  if(NULL == $$) {
		    YYABORT;
		  }
		} 

elsee:
		ELSE ife { $$ = $2; }
		|
		ELSE block { $$ = $2; }
		| 
		{ $$ = (bytecode*)0; }
		;
		
decls:
		decl SEMI decls {} | {};

decl:	
		global_modifier intfloat_keyword ident_list {
		  bool isGlobal = $1;
		  bool isFloat = $2;
		  bool stat;
		  ipair* p;
		  char* iden;

		  DPRINTF("Doing decl");
		  
		  while((p = (ipair*)list_popFromFront($3))) {
		    iden = p->a;

		    if(p->b<0) {
		      if(isGlobal){
			if(isFloat) {
			  stat = vm_addFloat(ccompart->vm,iden);
			} else {
			  stat = vm_addInt(ccompart->vm,iden);
			}
		      } else {
			if(isFloat) {
			  stat = com_addFloat(ccompart,iden);
			} else {
			  stat = com_addInt(ccompart,iden);
			}
		      }
		    } else{
		      varTable *vt;
		      if(isGlobal)
			vt = &ccompart->vm->vt;
		      else 
			vt = &ccompart->vt;
		      
		      // At some point this can be moved to a seperate function
		      var *tmp=addVar(vt,((isFloat)?FLOAT_VAR:INT_VAR) | IS_ARRAY,iden);
		      tmp->size = p->b;
		      tmp->p = calloc(p->b, sizeof(intfloat));
		      if(NULL==tmp->p) {
			compileError("Out of memory when declaring array :(");
			YYERROR;
		      }
		      stat = true; 
		    }
		    
		    if(!stat) {
		      sprintf(sprt, "Declaration of variable: \"%s\" failed, global=%i, float=%i",iden, isGlobal, isFloat);
		      compileWarning(sprt);
		    }
		    free(iden);
		    free(p);
		  }
		  DPRINTF(" - Done declare\n");
		  list_free($3);
			
		};
		
		
intfloat_keyword:
		INT  { 
		  DPRINTF("  Declaration is an int\n");
			$$=false; 
		}
		| 
		FLOAT { 
		  DPRINTF("  Declaration is a float\n");
			$$=true; 
		};
		
global_modifier:
		GLOBAL { 
		  DPRINTF("  Declaration is a global\n");
			$$=1; 
		}
		|  {
		  DPRINTF("  Declaration isn't a global\n");
			$$=0;
		};

array_length:
		OPENS expression CLOSES {
		  DPRINTF("  Declaration is array\n");
			if(!$2->isLiteral) {
				compileError("Array declaration with non-literal length");
				YYERROR;
			}
			expr_autocast(false,$2);
			$$ = $2->val.in;
			if($$<0) {
				sprintf(sprt, "Array declared with length %i",$$);
				compileError(sprt);
				YYERROR;
			}
			free($2);
		}
		| 
		{
		  DPRINTF("  Declaration isn't array\n");
		  $$=-1;
		}

		
ident_list:
		more_ident_list {
		  DPRINTF("  Declaration of %i variables\n",$1->length);
		  $$=$1;
		}
		|
		{
		  DPRINTF("  Declaration of 0 variables\n");
			$$= list_new();
		}
		;

more_ident_list:	
		IDENTIFIER array_length COMA more_ident_list {
		  ipair * p = calloc(1, sizeof(ipair));

		  p->a = $1;
		  p->b = $2;

		  list_addToFront($4,p);

		  // MEM: Identifier is free'd in 'decl'
		  $$=$4;
		}
		| IDENTIFIER array_length {
		  list *lst = list_new();
		  ipair * p = calloc(1, sizeof(ipair));

		  p->a = $1;
		  p->b = $2;
		  list_addToFront(lst,p);

		  // MEM: Identifier is free'd in 'decl'
		  $$=lst;
		}
		;		
				
%%
