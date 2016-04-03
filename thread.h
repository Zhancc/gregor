#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include "gregor_error.h"

/*global state data structure begin*/
#define MAX_QUEUE 20

typedef void* continuation_ptr;

typedef struct wstate{
	pthread_t threadId;
	
	continuation_ptr worklist[MAX_QUEUE];
	int queue_head, queue_rear;

} wstate;

struct mstate{
	wstate *worker_info;

} mstate;


/* the register global to store the tid. linked program must avoid using this register in compilation*/ 
register int tid __asm__("ebx"); 

/*global state data structure end*/

void* __gregor_worker_init(void* threadid);
void __gregor_do_work(int threadId);




/*wrapper of pthread begin*/
void Pthread_create(pthread_t *restrict thread,
		              const pthread_attr_t *restrict attr,
		              void *(*start_routine)(void*), void *restrict arg);
pthread_t Pthread_self(void);

/*wrapper of pthread end*/
#endif
