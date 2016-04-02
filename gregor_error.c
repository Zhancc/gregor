#include "gregor_error.h"

void __gregor_error(char* s){
        fprintf(stderr, "%s\n", s );
        exit(-1);
}
