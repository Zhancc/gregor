#ifndef GREGOR_H
#define GREGOR_H

int spawn(void* routine, int num_arg, ...);
void sync();
void create_job(void* ret, void* routine, int num_arg, ...);
int add_job(jcb* job);
#endif
