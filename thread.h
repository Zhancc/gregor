#ifndef THREAD_H
#define THREAD_H
#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>
#include "gregor_error.h"
#include <sys/mman.h>

/*global state data structure begin*/
#define STACK_ALIGN(T, addr) ((T)(((uint32_t)(addr))&((uint32_t)~0x3)))

typedef struct space {
    void* pointer;
    struct space* next;
} MemorySpace;

MemorySpace* MemorySpace_New(void* freedSpace, int pagesize); 

typedef struct mm {
    int availNum;
    MemorySpace* head;
} MemoryManager;

MemoryManager* MemoryManager_New();
void* AllocMemory(MemoryManager* m, int pagesize); 
void FreeMemory(MemoryManager* m, void* space, int pagesize);


/*support for primitive types + pointer types only so far*/
enum type{
	VOID = 0,
	/* 4 Byte */
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
	PTR,
	FLOAT,
	/* 8 Byte */
	LONG_LONG_INT,
	UNSIGNED_LONG_LONG_INT,
	DOUBLE,
	/* 16 Byte */
	LONG_DOUBLE,
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
    struct jcb *prev, *next;
} jcb;

typedef struct deque {
	struct jcb* head_node;
	struct jcb* tail_node;
	int size;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_cond;
} Deque;


typedef struct wstate{
	pthread_t threadId;


	Deque* deque;
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
	MemoryManager* mm;
} wstate;

<<<<<<< HEAD
// typedef struct node {
//     jcb* job;
//     struct node* next;
//     struct node* prev;
// } Node;

=======
typedef struct deque {
	struct jcb* head_node;
	struct jcb* tail_node;
	int size;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_cond;
} Deque;
>>>>>>> 9f8f6bf3eed26110f61ad4a29eb8ec97a0782f3f

struct mstate{
	wstate *worker_info;
	sem_t sem;
	Deque *deque;
} mstate;

<<<<<<< HEAD
// Node* Node_new(jcb* job);
=======
>>>>>>> 9f8f6bf3eed26110f61ad4a29eb8ec97a0782f3f
Deque* Deque_new();
void Deque_free(Deque* d);
void AddNodeToTail(Deque* deque, jcb* job);
void AddNodeToHead(Deque* deque, jcb* job);
jcb* GetNodeFromTail(Deque* deque);
jcb* GetNodeFromHead(Deque *deque);
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
void add_job_tail(Deque* deque, jcb* job);
void add_job_head(Deque* deque, jcb* job);
/*wrapper of pthread begin*/
void Pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void*), void *restrict arg);

pthread_t Pthread_self(void);

/*wrapper of pthread end*/
#endif
