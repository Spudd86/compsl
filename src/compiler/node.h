#include <stdbool.h>
#include "../intern/bytecode.h"
#include "../extern/vm.h"
#include "../extern/compart.h"

#include "../intern/debug.h"

#ifndef NODE_H_
#define NODE_H_

#define DPRINTF(...) COMP_DPRINTF(__VA_ARGS__) 


#define CSL_FLOAT 2;
#define CSL_INT 1;


typedef struct _node_t
{
	char* foo;
	int f;
	
} node;


typedef struct _expression_t {
	bool isLiteral;
	bool isFloat;	
	union { 
		int in; 
		float fl; 
		bytecode* bcode; 
	} val;
} expression;

typedef struct llist {
	void *obj;
	void *next; //TODO: make llist type
} llist;

typedef struct _list_t {
	int length;
	llist *head;
} list;


int bc_len(bytecode *);
bytecode* expr_toBc(expression *exp);
void expr_free(expression* exp);

list* list_new(void);
void* list_get(list *, int);
void list_free(list *);
void list_addToFront(list *, void *);
void* list_popFromFront(list *);

void internalCompileError(const char* str);
void compileError(const char *str);
void autocast(bool toFloat,expression *e);


extern char *sprt;


#endif 
