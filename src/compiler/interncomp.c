#include <assert.h>
#include <stdlib.h>

#include "interncomp.h"


int bc_len(bytecode* bc) {
  assert(bc!=(bytecode*)0);
  int len=0;
  while(bc->code!=BC_NONO&&bc->code!=BC_END) {
    bc++;
    len++;
    if(len>10000) {
      internalCompileError("Non null terminated bytecode string");
      exit(4);
    }
  } 
  return len;
}


void expr_ensureLit(expression* exp) {
  if(exp->isLiteral) {
    exp->val.bcode = expr_toBc(exp);
    exp->isLiteral = false;
  }
}

void expr_autocast(bool toFloat,expression *e) {

  if(toFloat == e->isFloat) {
    // no cast needed

  } else if(e->isLiteral) {
    if( toFloat && !e->isFloat) {
      e->val.fl = (float)e->val.in;
      e->isFloat=true;
    }
    else {
      e->val.in = (int)e->val.fl;
      e->isFloat=false;
    } 
  } else {
    int len;
    len = bc_len(e->val.bcode);
    e->val.bcode = realloc(e->val.bcode, sizeof(bytecode)*(len+2));
    e->val.bcode[len].code = (toFloat && !e->isFloat)?BC_INFL:BC_FLIN;
    e->val.bcode[len+1].code = BC_NONO;
    e->isFloat = toFloat;
  }
}

/*
 * Note: doesnt free anything
 */
bytecode* expr_toBc(expression *exp) {
  if(exp->isLiteral) {
    bytecode* bc = malloc(sizeof(bytecode)*2);
    if(bc==NULL) internalCompileError("Out of memory");
    bc[0].code=BC_CPUSH;
    
    intfloat tmp;
    if(exp->isFloat) {
	tmp.f = exp->val.fl;
    }
    else {
      tmp.i = exp->val.in;
    }
    bc[0].a1 = com_addConst(ccompart , tmp); 
    bc[1].code=BC_NONO;
    return bc;
  }
  else {
    if(exp->val.bcode==0) internalCompileError("Error in expr_toBc");
    return exp->val.bcode;
  }
}


void expr_free(expression* expr) {
  if(!expr->isLiteral) {
    assert(expr->val.bcode != (bytecode*)0);      
    free(expr->val.bcode);
  }
  free(expr);
}