#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <stdint.h>
#include "gregor_error.h"

/*global state data structure begin*/
#define MAX_QUEUE 40
#define STACK_ALIGN(T, addr) ((T)(((uint32_t)(addr))&((uint32_t)~0x3)))


enum job_status{
	SPAWN,
/*	stack:
*		arguments
*		return addr
*/

	RUNNING,
/*	stack:
*		arguments
*		return addr
*		fsave
*		eflags
*		ecx
*		edx
*		ebp
*		esi
*		edi
*		esp		
*/	
	TRY_EXIT

};


#define CURRENT   do{ \
					wstate* w = &(mstate.worker_info[tid]);\
					w->worklist[w->cur];\
					}while(0);
/* control block of each job reside at the top address of its stack*/
typedef struct jcb{
	void* esp;
	void* mmap_addr;
	int   mmap_size;
	enum job_status status;
} jcb;

// typedef void* continuation_ptr;

typedef struct wstate{
	pthread_t threadId;

	jcb* worklist[MAX_QUEUE];
	int queue_head, queue_rear;
	int  cur;

	/* the esp of the pthread stack */
	void* p_esp;

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
void Pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void*), void *restrict arg);

pthread_t Pthread_self(void);

/*wrapper of pthread end*/
#endif
