#ifndef GREGOR_ERROR_H
#define GREGOR_ERROR_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define EARG -1


void __gregor_error(char* s);
void __gregor_panic(char* s);
#endif
