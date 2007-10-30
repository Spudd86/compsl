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


#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "compiler/interncomp.h"

COMPSL_INTERN COMPSL_NONNULL expression *function_call(const char* name, list *params) {
  expression *ex = malloc(sizeof(expression));
  if(ex == NULL) internalCompileError("Out of Memory");
  bytecode *mcode=NULL;
  ex->isFloat = false;
  int lenBc, curBc;
  
  bytecode callcode;
  int numParams=0;
  uint8_t *paramFlags = NULL;
  bool freeParamFlags = false;
  bool found=false;
  
  llist *curParam;	  
  
  // Built in funcs
  for(int i=0;i<builtins_len;i++) {
    if(strcmp(name,builtins[i].name)==0) {
      DPRINTF("Function \"%s\" found is a built in function\n",name);
      bool isFloat = builtins[i].isFloat;
      callcode.code = builtins[i].code;
      callcode.a1 =0;
      numParams = builtins[i].ac; 
      ex->isFloat = builtins[i].isFloat;
      paramFlags = malloc(sizeof(uint8_t)*numParams);
      if(paramFlags == NULL) internalCompileError("Out of Memory");
      freeParamFlags = true;
      for(int q=0;q<numParams;q++)
	paramFlags[q] = ((isFloat)?FLOAT_VAR:0);
      found = true;
      break;
    }
  }
  
  // Native function
  if(!found) {
    symbolinfo symbol = searchSym(name,ccompart);
    nativeFN *funk = NULL;
    if(symbol.id<0) {
      sprintf(sprt, "Function %s does not exist",name);
      compileError(sprt);
      free(ex);
      return NULL;
    } else if(symbol.isvar) {
      sprintf(sprt,"Variable %s used as a function call",name);
      compileError(sprt);
      free(ex);
      return NULL;
    }
    funk = &ccompart->vm->natives[symbol.id];
    paramFlags = funk->paramFlags;
    ex->isFloat = funk->retFloat;
    numParams = funk->numParam;
    callcode.code = BC_CALL;
    callcode.a1 = symbol.id;
    found = true;
    
    DPRINTF("Function \"%s\" found with id %i, function=%p\n",name,symbol.id, funk->func);
    assert(strcmp(name,funk->name)==0); 
    assert(funk->func!=0);
  }	    
  
  assert(found);
  assert(paramFlags!=0 || numParams==0);
  
  
  // Check no. arguments
  if(params->length != numParams) {
    sprintf(sprt, "Function %s has %i parameters but %i found",name, numParams, params->length);
    compileError(sprt);
    return 0;
  }
  
  // 1 for callcode, 1 for BC_NONO
  lenBc = 2;
  
  // Ready the parameters and count the size of the bytecodes
  curParam = params->head;
  for(int i=0; i< numParams;i++) {
    expression* ce = (expression*)curParam->obj;
    expr_autocast(paramFlags[i] & FLOAT_VAR, ce);
    
    if(paramFlags[i] & IS_ARRAY) {
      sprintf(sprt, "Function %s has parameters %i as an array, however array wasn't found",name, i);
      compileError(sprt);
      free(ex);
      return NULL;
    }
    
    if(ce->isLiteral) {
      ce->val.bcode = expr_toBc(ce);
      ce->isLiteral = false;
    }
    lenBc+=bc_len(ce->val.bcode);
    curParam = curParam->next;
    DPRINTF("  Parameter %i has bytecode length %i, new total is %i\n",i, bc_len(ce->val.bcode), lenBc);
  }
  
  mcode = calloc(lenBc, sizeof(bytecode));
  if(mcode == NULL) internalCompileError("Out of Memory");
  curBc = 0;
  
  // Copy the parameters into mcode, last parameter first
  for(int i=numParams-1;i>=0;i--) {
    expression* ce = (expression*)list_get(params,i); // inneficient
    int clen = bc_len(ce->val.bcode);
    memcpy(&mcode[curBc],ce->val.bcode,sizeof(bytecode)*clen);
    expr_free(ce);
    curBc+=clen;
  }
  
  mcode[curBc] = callcode;
  curBc++;
  mcode[curBc].code = BC_NONO;
  curBc++;
  assert(curBc==lenBc);
  
  ex->isLiteral = false;
  ex->val.bcode = mcode;
  
  if(freeParamFlags) free(paramFlags);
  list_free(params);
  // params contents already free'd in copy loop
  
  DPRINTF("Function call completed with length %i with isFloat=%i\n", bc_len(ex->val.bcode), ex->isFloat);
  
  return ex;
}
