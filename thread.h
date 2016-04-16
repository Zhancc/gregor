#ifndef THREAD_H
#define THREAD_H
#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>
#include "gregor_error.h"

/*global state data structure begin*/
#define MAX_QUEUE 40
#define STACK_ALIGN(T, addr) ((T)(((uint32_t)(addr))&((uint32_t)~0x3)))


/*support for primitive types + pointer types only so far*/
enum type{
	VOID = 0,
	SIGNED_CHAR,
	UNSIGNED_CHAR,
	CHAR,
	SHORT_INT,
	SIGNED_SHORT,
	UNSIGNED_SHORT_INT,
	INT,
	UNSIGNED_INT,
	LONG_INT,
	UNSIGNED_LONG_INT,
	LONG_LONG_INT,
	UNSIGNED_LONG_LONG_INT,
	FLOAT,
	DOUBLE,
	LONG_DOUBLE,
	PTR,
	STRUCT
};
/*
enum arg_type{
	VOID = 0,
	SIGNED_CHAR,
	UNSIGNED_CHAR,
	CHAR,
	SHORT_INT,
	SIGNED_SHORT,
	UNSIGNED_SHORT_INT,
	INT,
	UNSIGNED_INT,
	LONG_INT,
	UNSIGNED_LONG_INT,
	LONG_LONG_INT,
	UNSIGNED_LONG_LONG_INT,
	FLOAT,
	DOUBLE,
	LONG_DOUBLE,
	PTR,

	STRUCT
};
*/
#define ARG(T,D) T, D
#define STRUCT_ARG(D) STRUCT, sizeof(D), D

enum job_status{
	SPAWN,
/*	stack:
*		arguments
*		return addr to cleanup
* 		return addr to job function
*/

	RUNNING,
/*	stack:
*		return addr before reschedule
*		eflags
*		ecx
*		edx
*		ebp
*		esi
*		edi
*/
	SYNC
};

#define CURRENT_WORKER (&(mstate.worker_info[tid]))
#define CURRENT   (CURRENT_WORKER->cur)
/* control block of each job reside at the top address of its stack*/
typedef struct jcb{
	void* esp;
	void* mmap_addr;
	int   mmap_size;
	enum job_status status;
	enum type ret_type;
	void* ret_ptr;
	int join_counter;
	struct jcb* parent;
	char fstate[512] __attribute__((aligned(16)));

} jcb;

// typedef void* continuation_ptr;

typedef struct wstate{
	pthread_t threadId;


	// jcb* worklist[MAX_QUEUE];
	// int queue_head, queue_rear;
	/* pointer to the currently executing job */
	jcb*  cur;

	/*the job for reschedule only*/
	jcb* next_job;


	/* the esp of the pthread stack */
	void* p_esp;
/*	stack:
*		return addr to __gregor_do_work or main
*		eflags
*		ecx
*		edx
*		ebp
*		esi
*		edi
*/

} wstate;

typedef struct node {
    jcb* job;
    struct node* next;
    struct node* prev;
} Node;

typedef struct deque {
    Node *head_node;
    Node *tail_node;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_cond;
} Deque;

struct mstate{
	wstate *worker_info;
	sem_t sem;
	Deque *deque;
} mstate;

Node* Node_new(jcb* job);
Deque* Deque_new();
void AddNodeToTail(Deque* deque, jcb* job);
void AddNodeToHead(Deque* deque, jcb* job);
Node* GetNodeFromTail(Deque* deque);
Node* GetNodeFromHead(Deque *deque);
int isEmpty(Deque *deque);

/* the register global to store the tid. linked program must avoid using this register in compilation*/
register int tid __asm__("ebx");
int pagesize;
int mem_op;

/*global state data structure end*/
void gregor_main(int* return_ptr, int (*routine)(int, char**),int argc, char** argv );
void do_gregor_main(void* p_esp, void* dummy_ret, int* return_ptr, int (*routine)(int, char**),int argc, char** argv );
void* do_gregor_main_init(void* ptr);
void* __gregor_worker_init(void* threadid);
void init_data_structure();
void __gregor_do_work_loop();
void cleanup();
void do_cleanup(unsigned int eax, unsigned int edx);
jcb* create_job(void* ret, enum type rt, void* return_ptr, void* routine, int num_arg, ...);
void add_job_tail(jcb* job);
void add_job_head(jcb* job);
/*wrapper of pthread begin*/
void Pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void*), void *restrict arg);

pthread_t Pthread_self(void);

/*wrapper of pthread end*/
#endif
