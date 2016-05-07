#include "gregor_error.h"

void __gregor_error(char *s) {
    fprintf(stderr, "error: %s\n", s);
    exit(-1);
}

void __gregor_panic(char *s) {
    fprintf(stderr, "panic: %s\n", s);
    exit(-1);
}
