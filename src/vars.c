#include "intern/gen.h"
#include "extern/var.h"
#include "intern/vars.h"
#include <inttypes.h>
#include <malloc.h>
#include <string.h>

void varTableCreate(varTable *vt, uint16_t size)//FIXME uh what did I put this here for
{
	vt->cnt = 0;
	vt->symbols = malloc(sizeof(struct SYMTABLE_T) * size);
	vt->vars = malloc(sizeof(var) * size);
}

void varTableDestroy(varTable *vt)
{
	int i;
	for(i = 0; i < vt->cnt; i++)
	{
		free(vt->symbols[i].name);// free the name string
		if(vt->symbols[i].typeflags & IS_ARRAY)
		{
			free(vt->vars[i].p);// free the array
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
		if(strcmp(vt->symbols[i].name, name) == 0)// is this the one?
			return i;
	}
	
	return -1; //didn't find it
}

var *addVar(varTable *vt, uint8_t typeflags, const char *name)
{	
		vt->vars[vt->cnt].size = -1;
		vt->vars[vt->cnt].v.i = 0;
		
		vt->symbols[vt->cnt].id = vt->cnt;
		vt->symbols[vt->cnt].name = malloc(sizeof(char)*(strlen(name) + 1));
		strcpy(vt->symbols[vt->cnt].name, name);
		vt->symbols[vt->cnt].typeflags = typeflags;
		
		return &(vt->vars[vt->cnt++]);
}
