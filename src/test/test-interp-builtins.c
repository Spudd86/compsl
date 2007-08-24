#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <float.h>
#include <math.h>

#include "../intern/bytecode.h"
#include "../extern/compsl.h"
#include "../intern/vars.h"


/*******************************************************
 * Tester for the builtin function instructions
 *******************************************************/

int main()
{
	puts("Running interpreter builtins tests");
	
	VM *vm = createVM();
	compart *com = createComp(vm);
	bool ret = 0;
	
	/************************ Begin Tests *************************************/

	{ //ABS
        bytecode code[] = 
        { 
            {.code = BC_CPUSH, { {.a1 =0, .a2 = 0} }}, 
            {.code = BC_CPUSH, { {.a1 =1, .a2 = 0} }},
            {.code = BC_ABS },
            {.code = BC_POP, {{.a1 = 1, .a2 = 0}}},
            {.code = BC_ABS },
            {.code = BC_POP, {{.a1 = 0, .a2 = 0}}},
            {.code = BC_END}
        };
        
        com->cons[0].size =-1;
        com->cons[0].v.i = -1;
        com->cons[1].size =-1;
        com->cons[1].v.i = 2;
        
        com->vt.vars[0].size =-1;
        com->vt.vars[0].v.i = 0;
        com->vt.vars[1].size =-1;
        com->vt.vars[1].v.i = 0;
        
        com->cubbys[0].code = code;
        
        runCubbyhole(com, 0);
        
        if(com->vt.vars[0].v.i == 1 &&  com->vt.vars[1].v.i == 2)
            printf("ABS: PASS!\n");
        else
            printf("ABS: FAIL!\n");
    }
	
	/************************** End Tests *************************************/
	com->cubbys[0].code = NULL;
	
	destroyComp(com);
	destroyVM(vm);
	return ret; //return 0 else make's unhappy
}
