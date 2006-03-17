#include "intern/gen.h"
#include "extern/var.h"
#include "intern/vars.h"
#include <inttypes.h>
#include <malloc.h>
#include <string.h>

void varTableCreate(varTable *vt, uint16_t size)//FIXME 
{
	vt->cnt = 0;
	vt->symbols = malloc(sizeof(struct SYMTABLE_T)*size);
	vt->vars = malloc(sizeof(vt->vars)*size);
}

void varTableDestroy(varTable *vt)
{
	int i;
	for(i = 0; i < vt->cnt; i++)
	{
		free(vt->symbols[i].name);
		if(vt->symbols[i].typeflags & IS_ARRAY)
		{
			free(vt->vars[i].p);//TODO find out if this works properly with pointers to float
		}
	}
	
	free(vt->symbols);
	free(vt->vars);
}

int16_t findVar(varTable *vt, const char *name)
{
	int i;
	for(i = 0; i < vt->cnt; i++)
	{
		if(strcmp(vt->symbols[i].name, name) == 0)
			return i;
	}
	
	return -1;
}

var *addVar(varTable *vt, uint8_t typeflags, const char *name)
{	
		vt->vars[vt->cnt].size = -1;
		vt->vars[vt->cnt].v.i = 0;
		
		vt->symbols[vt->cnt].id = vt->cnt;
		vt->symbols[vt->cnt].name = malloc(sizeof(char)*(strlen(name) + 1));
		strcpy(vt->symbols[vt->cnt].name, name);//TODO: make sure the params are in correct order...
		vt->symbols[vt->cnt].typeflags = typeflags;
		
		return &(vt->vars[vt->cnt++]);
}
