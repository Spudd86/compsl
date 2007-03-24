#ifndef COMPSL_H_
#define COMPSL_H_
#include <stdbool.h>
#include "vm.h"
#include "compart.h"
#include "var.h"

#ifdef __cplusplus
extern "C" {
#endif

int *fileCompile(char *filename , VM*, compart**);
int *stringCompile(char *code, size_t , compart**);

void runCubbyhole(compart *com, int id); // runs 

#ifdef __cplusplus
}
#endif

#endif /*COMPSL_H_*/
