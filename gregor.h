#ifndef GREGOR_H
#define GREGOR_H

#include "thread.h"
int spawn(void* routine, int num_arg, ...);
void sync();
jcb* create_job(void* ret, void* routine, int num_arg, ...);
int add_job(jcb* job);
#endif
