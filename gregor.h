#ifndef GREGOR_H
#define GREGOR_H

#include "thread.h"
int spawn(enum return_type rt,void* return_ptr, void* routine, int num_arg, ...);
int __gregor_sync();

#endif
