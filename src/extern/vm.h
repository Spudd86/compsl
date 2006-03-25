/**************************************
 * 
 * EXTERNAL VM HEADER
 * 
 **************************************/

#ifndef VM_H_
#define VM_H_

#include <malloc.h>
#include <inttypes.h>
#include <stdbool.h>

#include "var.h"
#include "compslerr.h"

#define VM_STACK_SIZE 256
#define VM_MAX_GVARS 256
#define VM_NATIVEFN_INIT_SIZE 64

typedef struct _nativeFN_t
{
        char *name; // name of this function
        var (*func)(var *); // pointer the function to call
        
        var *params; // the list of paramaters to pass to it
        uint8_t *paramFlags; //the type flags of each paramater
        uint8_t numParam; // how many paramaters to pass it
} nativeFN;

typedef struct _VM_t
{
    nativeFN *natives; // native function table
    uint16_t ncnt; // number of native functions
    
    varTable vt;
    
    COMPSL_ERROR errorno; // holds the error code of the last error generated by this VM
} VM;

VM *createVM(void);
void destroyVM(VM *vm);

//NOTE: the string of the name of new vars is copyed and the copy is retained by the VM
//      for the purpose of identifying the variable. Same goes for native functions.
float *vm_addFloat(VM *vm, const char *name);// add a float to the vm's global vars, and return a pointer to it
int32_t *vm_addInt(VM *vm, const char *name);

float *vm_getFloat(VM *vm, const char *name);
int32_t *vm_getInt(VM *vm, const char *name);

/** add a native function to this vm, return true on success false otherwise
 * sets errno on fail
 */
bool addFunc(VM *vm, var (*func)(var *)); //TODO work out how to spec params

#endif /*VM_H_*/
