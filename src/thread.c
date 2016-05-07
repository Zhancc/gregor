#define _GNU_SOURCE
#include <unistd.h>
#include "thread.h"
#include "gregor.h"
#include "sched.h"
#include "init.h"
#include <sched.h>

#define SEGMENT 2

typedef struct main_arg {
    void *p_esp;
    int *return_ptr;

    int (*routine)(int, char **);

    int argc;
    char **argv;
} main_arg;

typedef struct block {
    struct block *next;
} Block;

void Pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *),
                    void *restrict arg) {
    if (pthread_create(thread, attr, start_routine, arg) < 0) {
        __gregor_error("thread creation fail");
    }
}

pthread_t Pthread_self(void) {
    return pthread_self();
}

void do_gregor_main(void *p_esp, void *dummy_ret, int *return_ptr, int (*routine)(int, char **), int argc,
                    char **argv) {
    /* initialize the master thread */
    init();
    main_arg ma;
    ma.p_esp = p_esp;
    ma.return_ptr = return_ptr;
    ma.routine = routine;
    ma.argc = argc;
    ma.argv = argv;
    pthread_create(&(mstate.worker_info[0].threadId), NULL, do_gregor_main_init, (void *) &ma);

#warning: wait can be interrupted by signal
    sem_wait(&(mstate.sem));
    fini();
}

void init_data_structure() {
    // cpu_set_t cpuset;
    // CPU_ZERO(&cpuset);
    // CPU_SET(tid%NUM_PROCESSOR, &cpuset);
    // pthread_setaffinity_np(pthread_self(),sizeof(cpu_set_t),&cpuset);
    CURRENT_WORKER->cur = CURRENT_WORKER->next_job = NULL;
    CURRENT_WORKER->deque = Deque_new();
    CURRENT_WORKER->mm = MemoryManager_New();
    CURRENT_WORKER->setup = 1;
    CURRENT_WORKER->num_work = 0;
    gregor_srand(tid + 1);

}

/*this function should not return, the threads should be blocked in loop and killed by master thread*/
void *do_gregor_main_init(void *ptr) {
    main_arg *ma = (main_arg *) ptr;
    tid = 0;
    init_data_structure();
    jcb *main_job = create_job(NULL, INT, ma->return_ptr, ma->routine, 2, sizeof(int), sizeof(char **), ma->argc,
                               ma->argv);

    add_job_tail(CURRENT_WORKER->deque, main_job);
    __gregor_do_work_loop();
    return NULL;
}


/*this function should not return, the threads should be blocked in loop and killed by master thread*/
void *__gregor_worker_init(void *threadid) {
    tid = (int) (long) threadid;
    init_data_structure();
    __gregor_do_work_loop();

    return NULL;
}

/*invariant: always execute this function in ptrhead stack*/
void __gregor_do_work_loop() {
    while (1) {
        jcb *next = pick_work();
        /*jump to the work specified*/
        //CURRENT_WORKER->num_work++;
        if (next == NULL) {
            __gregor_panic("pick a NULL work in __gregor_do_work_loop");
        }
        set_next_job(next);
        reschedule_from_pthread();
    }
}


/* cleanup should not return, it would switch back to __gregor_do_work or main function*/
void do_cleanup(unsigned int eax, unsigned int edx) {
    unsigned char al = (unsigned char) eax;
    unsigned short ax = (unsigned short) eax;
    if (CURRENT->ret_ptr != NULL) {
        switch (CURRENT->ret_type) {
            case VOID :
                break;
            case SIGNED_CHAR:
                *(signed char *) (CURRENT->ret_ptr) = al;
                break;
            case UNSIGNED_CHAR:
                *(unsigned char *) (CURRENT->ret_ptr) = al;
                break;
            case CHAR:
                *(char *) (CURRENT->ret_ptr) = al;
                break;
            case SHORT_INT:
                *(short int *) (CURRENT->ret_ptr) = ax;
                break;
            case SIGNED_SHORT:
                *(signed short *) (CURRENT->ret_ptr) = ax;
                break;
            case UNSIGNED_SHORT_INT:
                *(unsigned short *) (CURRENT->ret_ptr) = ax;
                break;
            case INT:
                *(int *) (CURRENT->ret_ptr) = eax;
                break;
            case UNSIGNED_INT:
                *(unsigned int *) (CURRENT->ret_ptr) = eax;
                break;
            case LONG_INT:
                *(long int *) (CURRENT->ret_ptr) = eax;
                break;
            case UNSIGNED_LONG_INT:
                *(unsigned long int *) (CURRENT->ret_ptr) = eax;
                break;
            case LONG_LONG_INT:
                *(long int *) (CURRENT->ret_ptr) = eax;
                *((long int *) (CURRENT->ret_ptr) + 1) = edx;
                break;
            case UNSIGNED_LONG_LONG_INT:
                *(unsigned long int *) (CURRENT->ret_ptr) = eax;
                *((unsigned long int *) (CURRENT->ret_ptr) + 1) = edx;
                break;
            case FLOAT:
                __asm__(
                "fstps %0\t\n"
                :
                :"m"(*(float *) (CURRENT->ret_ptr))
                );
                break;
            case DOUBLE:
                __asm__(
                "fstpl %0\t\n"
                :
                :"m"(*(double *) (CURRENT->ret_ptr))
                );
                break;
            case LONG_DOUBLE:
                __asm__(
                "fstpt %0\t\n"
                :
                :"m"(*(long double *) (CURRENT->ret_ptr))
                );
                break;
            case PTR:
                *(int *) (CURRENT->ret_ptr) = eax;
                break;
        }
    }

#warning: refinement: currently just yield the processor
    __gregor_sync();

    if (CURRENT->parent) {
        int ret;
        jcb *j = CURRENT->parent;
        atomicDecrement(&(j->join_counter));
        ret = j->join_counter;

        if (ret < 0) {
            __gregor_panic("join_counter incorrect");
        }
    } else {
        /*invariant: at this point, no other threads should be working*/
        /*wake up the master thread*/
        sem_post(&(mstate.sem));
    }
    /* esp should be save in context of work loop, i.e. pthread stack */
    void *esp = CURRENT_WORKER->p_esp;
    swicth_free_current(esp);
}

// Node* Node_new(jcb* job) {
// 	Node *p = (Node *)malloc(sizeof(Node));
// 	p->job = job;
// 	p->next = NULL;
// 	p->prev = NULL;
// 	return p;
// }

void Deque_free(Deque *p) {
    free(p);
}

Deque *Deque_new() {
    Deque *p = (Deque *) malloc(sizeof(Deque));
    p->head_node = NULL;
    p->tail_node = NULL;
    p->size = 0;
    p->T = 0;
    p->H = 0;
    pthread_mutex_init(&p->queue_lock, NULL);
    return p;
}


// void atomicIncrement(volatile int* m){
// 	asm volatile("lock incl %0"
// 	             : "+m" (*m));
// }
// void atomicDecrement(volatile int* m){
// 	asm volatile("lock decl %0"
// 	             : "+m" (*m));
// }


void AddNodeToTail(Deque *deque, jcb *job) {
    // Node* node = Node_new(job);
     // pthread_mutex_lock(&deque->queue_lock);

    job->next = job->prev = NULL;
    if (deque->tail_node == NULL) {
        deque->head_node = job;
        deque->tail_node = job;
    } else {
        jcb *prev = deque->tail_node;
        prev->next = job;
        job->prev = prev;
        deque->tail_node = job;
    }
    // deque->size++;
    // atomicIncrement(&(deque->size));
    atomicIncrement(&(deque->T));

     // pthread_mutex_unlock(&deque->queue_lock);
}

// void AddNodeToHead(Deque *deque, jcb *job) {
//     //Node* node = Node_new(job);
//     pthread_mutex_lock(&deque->queue_lock);

//     job->next = job->prev = NULL;
//     if (deque->head_node == NULL) {
//         deque->head_node = job;
//         deque->tail_node = job;
//     } else {
//         jcb *next = deque->head_node;
//         job->next = next;
//         next->prev = job;
//         deque->head_node = job;
//     }
//     atomicIncrement(&(deque->T));

//     pthread_mutex_unlock(&deque->queue_lock);
// }

jcb *GetNodeFromTail(Deque *deque) {
     // pthread_mutex_lock(&deque->queue_lock);
    int flag = 0;
    atomicDecrement(&(deque->T));
    if (deque->H + 1> deque->T) {
        atomicIncrement(&(deque->T));
       pthread_mutex_lock(&deque->queue_lock);
        atomicDecrement(&(deque->T));
        if (deque->H > deque->T) {
            atomicIncrement(&(deque->T));
            pthread_mutex_unlock(&deque->queue_lock);
            return NULL;
        }
        flag = 1;
       // pthread_mutex_unlock(&deque->queue_lock);

    }
    jcb *prev = deque->tail_node;

    if (prev->prev == NULL) {
        deque->head_node = NULL;
    } else {
        prev->prev->next = NULL;
    }
    deque->tail_node = prev->prev;
    prev->prev = prev->next = NULL;
    if(flag)
     pthread_mutex_unlock(&deque->queue_lock);

    return prev;
}


/*steal*/

jcb *GetNodeFromHead(Deque *deque) {
    pthread_mutex_lock(&deque->queue_lock);
    atomicIncrement(&(deque->H));
    if (deque->H + 1> deque->T) {
        /* too few elements*/
        atomicDecrement(&(deque->H));
        pthread_mutex_unlock(&deque->queue_lock);
        return NULL;
    }
    /*guarantee to have at least one element*/
    jcb *head = deque->head_node;

    if (head->next == NULL) {
        deque->tail_node = NULL;
    } else {
        head->next->prev = NULL;
    }
    deque->head_node = head->next;
    head->prev = head->next = NULL;
    pthread_mutex_unlock(&deque->queue_lock);

    return head;
}

int isEmpty(Deque *deque) {
    return deque->T <= deque->H;
}

MemoryManager *MemoryManager_New() {
    MemoryManager *mm = (MemoryManager *) malloc(sizeof(MemoryManager));
    mm->availNum = 0;
    mm->head = NULL;
    return mm;
}

void *AllocMemory(MemoryManager *m, int pagesize) {
    void *space = NULL;
    if (m->availNum == 0) {
        space = mmap(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
        if (space == (void *) -1) {
            __gregor_error("mmap failed");
        }

        m->head = space;

        for (int i = 0; i < SEGMENT - 1; ++i) {
            ((Block *) space)->next = (Block *) ((char *) space + pagesize / SEGMENT);
            space = ((Block *) space)->next;
        }
        ((Block *) space)->next = NULL;
        m->availNum += SEGMENT;
    }

    space = m->head;
    m->head = (void *) ((Block *) space)->next;
    m->availNum--;
    return space;
}

void FreeMemory(MemoryManager *m, void *space, int pagesize) {
    // MemorySpace* s = MemorySpace_New(space, pagesize);
    m->availNum++;
    ((Block *) space)->next = (Block *) m->head;
    m->head = space;
}
