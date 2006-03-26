#include "extern/vm.h"
#include "extern/compart.h"
#include "extern/compslerr.h"
#include "extern/compsl.h"

#include "intern/panic.h"
#include "intern/vm.h"
#include "intern/vars.h"
#include "intern/bytecode.h"

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <stdbool.h>

#define VM_FLOAT_EPSILON 0.01f

void runCubbyhole(compart *com, int id)
{
static void *jmptbl[] = 
{
	&&NOOP,
//stack operations with local vars
	&&PUSH,
 	&&APUSH,
 	&&CPUSH,
 	&&POP, 
 	&&APOP,
 	&&DPOP,
 	&&SWAP,
 // native function call
 	&&CALL, 
 // integer operations
 	&&ADD, 
 	&&SUB, 
 	&&MUL, 
 	&&DIV, 
 	&&CMP,
 // float operations 
 	&&FADD, 
 	&&FSUB, 
 	&&FMUL,
 	&&FDIV, 	
 	&&FCMP,
 // jumps 
 	&&JMP, 	
 	&&JMPL, 	
 	&&JMPE, 	
 	&&JMPG, 	
 	&&JMLE, 	
 	&&JMNE,
 	&&JMGE,
 //type conversion
 	&&FLIN, 
 	&&INFL,
 //mod
 	&&MOD, 
 	&&FMOD,
 //global vars
 	&&GPSH,
 	&&GAPS,
 	&&GPOP,
 	&&GAPP,
 //boolean 
 	&&AND,
 	&&OR,
 	&&NOT,
 //bitwise
 	&&BAND,
 	&&BOR,
 	&&BXOR,
 	&&BNOT,
 //misc
 	&&END,
 	&&HLT,
 	&&DBG
 };
	intfloat stack[VM_STACK_SIZE];
	intfloat *sp = stack; // stack pointer
	intfloat tmp;
	
	var *lvs = com->vt.vars;
	var *lcs = com->ct.vars;
	var *gvs = com->vm->vt.vars;
	
	bytecode *pc= (bytecode *)(com->cubbys[id].code) - 1; // init program counter
	
	bool less, equal, greater; // comparison flags
	less = equal = greater = false;
	
	TOP: 
		pc++;
		goto *jmptbl[pc->code]; // highly unreabable, but it gets the bytecode,  and jumps to the correct instruction
	NOOP:
		goto TOP;
	PUSH:
		*sp = lvs[pc->a1].v;
		sp++;
		goto TOP;
 	APUSH:
 		//TODO: writeme
 		goto UNIMP;
 	CPUSH:
 		*sp = lcs[pc->a1].v;
 		sp++;
		goto TOP;
 	POP:
 		sp--;
 		lvs[pc->a1].v = *sp;
		goto TOP;
 	APOP:
 		//TODO: writeme
 		goto UNIMP;
 	DPOP:
 		sp--;
 		goto TOP;
 	SWAP://TODO: should this be an addressed thing? (ie: can swap an arbitrary pair of stack elements)
 		tmp = *(sp - 1);
 		*(sp - 1) = *(sp - 2);
 		*(sp - 2) = tmp;
 		goto TOP;
 	CALL: 
 	//TODO: writeme
 		goto UNIMP;
 	ADD:
 		sp--;
 		(sp - 1)->i = (sp - 1)->i + sp->i;
 		goto TOP;
 	SUB:
 		sp--;	
 		(sp - 1)->i = (sp - 1)->i - sp->i;
 		goto TOP;
 	MUL:
	 	sp--;
 		(sp - 1)->i = (sp - 1)->i * sp->i;
 		goto TOP;
 	DIV:
 		sp--;	
 		(sp - 1)->i = (sp - 1)->i / sp->i;
 		goto TOP;
 	CMP:
 	// TODO: see if this can be done faster
 		sp-=2;
 		tmp.i = sp->i - (sp+1)->i;
 		less = tmp.i < 0;
 		equal = !tmp.i;
 		greater = tmp.i > 0;
 		goto TOP;
 	FADD:
	 	sp--;
 		(sp - 1)->f = (sp - 1)->f + sp->f;
 		goto TOP;
 	FSUB:
	 	sp--;
 		(sp - 1)->f = (sp - 1)->f - sp->f;
 		goto TOP; 
 	FMUL:
 		sp--;
 		(sp - 1)->f = (sp - 1)->f * sp->f;
 		goto TOP; 
 	FDIV:
 		sp--;
 		(sp - 1)->f = (sp - 1)->f / sp->f;
 		goto TOP; 
 	FCMP:
	 	sp-=2;
 		tmp.f = sp->f - (sp + 1)->f;
 		less = tmp.f < 0;
 		equal = fabs(tmp.f) < VM_FLOAT_EPSILON; // compensate for float inaccuracys
 		greater = tmp.f > 0;
 		goto TOP;
 	JMP:
 		pc += pc->sa - 1; // compensate for pc++ at top
 		goto TOP;
 	JMPL:
 		if(less) pc += pc->sa - 1;// compensate for pc++ at top
 		goto TOP;	
 	JMPE:
	 	if(equal) pc += pc->sa - 1;// compensate for pc++ at top
 		goto TOP; 	
 	JMPG:
 		if(greater) pc += pc->sa - 1;// compensate for pc++ at top
 		goto TOP;
 	JMLE:
 		if(less || equal) pc += pc->sa - 1;// compensate for pc++ at top
 		goto TOP;
 	JMNE:
 		if(!equal) pc += pc->sa - 1;// compensate for pc++ at top
 		goto TOP; 	
 	JMGE:
 		if(greater || equal) pc += pc->sa - 1;// compensate for pc++ at top
 		goto TOP;
 	FLIN: 
 		sp->i = (int)(sp->f);
 		goto TOP;
 	INFL:
 		sp->f = (float)(sp->i);
 		goto TOP;
 	MOD:
 		sp--;	
 		(sp - 1)->i = (sp - 1)->i % sp->i;
 		goto TOP;
 	FMOD:
 		sp--;	
 		(sp - 1)->f = (sp - 1)->f - sp->f * (int)((sp - 1)->f / sp->f); // a ? ((int)(a / b)) * b
 		goto TOP;
 	GPSH:
 		*sp = gvs[pc->a1].v;
		sp++;
		goto TOP;
 	GAPS:
 		//TODO: writeme
 		goto UNIMP;
 	GPOP:
 		sp--;
 		gvs[pc->a1].v = *sp;
		goto TOP;
 	GAPP:
 //TODO: writeme
 		goto UNIMP;
 		
 //begin of boolean + bitwise opers
 
 //boolean 
 	AND:
 		sp--;	
 		(sp - 1)->i = (sp - 1)->i && sp->i;
 		goto TOP;
 	OR:
 		sp--;	
 		(sp - 1)->i = (sp - 1)->i || sp->i;
 		goto TOP;
 	NOT:
 		(sp - 1)->i = !((sp - 1)->i);
 		goto TOP;
 //bitwise
 	BAND:
 		sp--;	
 		(sp - 1)->i = (sp - 1)->i & sp->i;
 		goto TOP;
 	BOR:
 		sp--;	
 		(sp - 1)->i = (sp - 1)->i | sp->i;
 		goto TOP;
 	BXOR:
 		sp--;	
 		(sp - 1)->i = (sp - 1)->i ^ sp->i;
 		goto TOP;
 	BNOT: 
 		(sp - 1)->i = ~((sp - 1)->i);
 		goto TOP;
 //end of boolean + bitwise opers
 	HLT:
 	
 	END:
 		return;
 	DBG: // dump some state info
 #ifdef DEBUG
 	{
 		var *t;
 		intfloat *st;
 		
 		fprintf(stderr, "Program Counter: %X", (unsigned int)((bytecode *)(com->cubbys[id].code) - pc));
 		
 		fprintf(stderr, "Compare Flags (e,l,g): %s %s %s\n",
 			(equal?"true":"false"),
 			(less?"true":"false"),
 			(greater?"true":"false"));
 		fprintf(stderr, "Stack Pointer: %X\n", (unsigned int)(sp - stack)); // what does this do? index of sp?
 		fprintf(stderr, "Stack: \n");
 		st = stack;
 		for(unsigned int i = 0; i < VM_STACK_SIZE; i++, st++)
	 		fprintf(stderr,"\t%X   %i %f\n", i, (int)(st->i), (st->f)); 

 		fprintf(stderr, "Local variables:\n");
 		t = lvs;
 		for(unsigned int i = 0 ; i < COMPART_MAX_VARS; i++, t++)
 			if(t->size <= 0) 
 				fprintf(stderr, "\t%X:   %i %f\n", i, (int)(t->v.i), t->v.f);
 		
 		fprintf(stderr, "Global Vars:\n");
 		t = gvs;
 		for(unsigned int i = 0 ; i < VM_MAX_GVARS; i++, t++)
 			if(t->size <= 0) 
 				fprintf(stderr, "\t%X:   %i %f\n",i, (int)(t->v.i), t->v.f);
 	}
 #endif
 		goto TOP;
 	UNIMP:
 	
 	panic("unimplemented instruction");
}
