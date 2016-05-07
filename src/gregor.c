#include <stdarg.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "gregor.h"
#include "gregor_error.h"
#include "thread.h"
#include "sched.h"

#define SEGMENT 2

/*
* the second argument being function ptr,
* the rest argument should be the argument list to routine:
* sizeof(arg1),arg1,sizeof(arg2),arg2.....
* return negative if anything bad happens, or 0 otherwise
*/

jcb *create_job(void *dummy_ret, enum type rt, void *return_ptr, void *routine, int num_arg, ...) {
    /* get a stack*/
    void *task_stack = AllocMemory(CURRENT_WORKER->mm,
                                   pagesize);


    /* the end of stack is used as jcb*/
    /*initialize job control block*/
    jcb *job = (jcb *) ((char *) task_stack + pagesize / SEGMENT) - 1;
    job->mmap_addr = task_stack;
    job->mmap_size = pagesize / SEGMENT;
    job->status = SPAWN;
    job->ret_type = rt;
    job->ret_ptr = return_ptr;
    job->join_counter = 0;
    job->parent = CURRENT;
    job->prev = job->next = NULL;
    /*update the join counter of the parrent*/
    if (CURRENT != NULL)
            atomicIncrement(&(job->parent->join_counter));

    /* top of stack */
    int *top = STACK_ALIGN(int *, job);
    top = top - 1;

    /* enqueue-stack */
    va_list varlist;
    va_start(varlist, num_arg);
    va_list vlist;
    va_copy(vlist, varlist);
    int sum = 0;
    for (int i = 0; i < num_arg; i++) {
        int size = va_arg(varlist, int);
        switch (size) {
            case VOID :
                break;
            case SIGNED_CHAR:
            case CHAR:
            case SHORT_INT:
            case SIGNED_SHORT:
            case INT:
            case LONG_INT:
            case UNSIGNED_CHAR:
            case UNSIGNED_SHORT_INT:
            case UNSIGNED_INT:
            case UNSIGNED_LONG_INT:
            case PTR:
            case FLOAT:
                sum += 4;
                break;
            case LONG_LONG_INT:
            case UNSIGNED_LONG_LONG_INT:
            case DOUBLE:
                sum += 8;
                break;
            case LONG_DOUBLE:
                sum += sizeof(long double);
                break;
            // case STRUCT:
            //     sum += va_arg(varlist, int);
            //     break;
        }
    }
    top = (void *) ((char *) top - sum);

#warning: we rely on the fact that the arguments are passed by stack and stack grows downwards
    if (num_arg > 0) {
        char *dst = (char *) top;
        char *src = (char *) (&num_arg + num_arg + 1);
        for (int i = 0; i < num_arg; i++) {
            int size = va_arg(vlist, int);
            int arg_size = 0;
            float temp;
            switch (size) {
                case VOID :
                    break;
                case SIGNED_CHAR:
                case UNSIGNED_CHAR:
                case CHAR:
                case SHORT_INT:
                case SIGNED_SHORT:
                case UNSIGNED_SHORT_INT:
                case INT:
                case UNSIGNED_INT:
                case LONG_INT:
                case UNSIGNED_LONG_INT:
                case PTR:
                    *(void **) dst = *(void **) src;
                    // memcpy(dst, src, 4);
                    dst += 4;
                    src += 4;
                    break;
                case FLOAT:
                    temp = (float) (*(double *) src);
                    *(float *) dst = temp;
                    // memcpy(dst, &temp, 4);
                    dst += 4;
                    src += 8;
                    break;
                case LONG_LONG_INT:
                case UNSIGNED_LONG_LONG_INT:
                    *(unsigned long long int *) dst = *(unsigned long long int *) src;
                    dst += 8;
                    src += 8;
                case DOUBLE:
                    *(double *) dst = *(double *) src;
                    // memcpy(dst, src, 8);
                    dst += 8;
                    src += 8;
                    break;
                case LONG_DOUBLE:
                    *(long double *) dst = *(long double *) src;
                    // memcpy(dst, src, sizeof(long double));
                    dst += sizeof(long double);
                    src += sizeof(long double);
                    break;
                // case STRUCT:
                //     arg_size = va_arg(vlist, int);
                //     memcpy(dst, src, arg_size);
                //     dst += arg_size;
                //     src += arg_size;
                //     break;
            }
        }
    }
    // memcpy(top, &num_arg + num_arg + 1,sum);

    // top = (void*)((char*)top - sum);
    top--;

    *(int **) top = (int *) cleanup; /* return to cleanup routine after finishing the job*/
    top--;
    *(int **) top = (int *) routine;
    job->esp = (void *) top;
    return job;
}


#warning: add the job to some queue to be refined

void add_job_tail(Deque *deque, jcb *job) {
    // pthread_mutex_lock(&deque->queue_lock);
    AddNodeToTail(deque, job);
    // pthread_mutex_unlock(&deque->queue_lock);
    // pthread_cond_signal(&deque->queue_cond);
    return;
}

#warning: add the job to some queue to be refined
// void add_job_head(Deque* deque, jcb* job){
// 	pthread_mutex_lock(&deque->queue_lock);
// 	AddNodeToHead(deque, job);
// 	pthread_mutex_unlock(&deque->queue_lock);
// 	pthread_cond_signal(&deque->queue_cond);
// 	return ;
// }


void set_next_job(jcb *job) {
    CURRENT_WORKER->next_job = job;
}

/* wait until all the descendents complete */
#warning: refinement: currently just enqueue the current work

int __gregor_sync() {
    CURRENT->status = SYNC;
    int fail_cnt = 0;
    while (CURRENT->join_counter) {
        jcb *job = try_pick_work();
        if (!job) {
        	fail_cnt++;
        	if(fail_cnt < 300)
	            usleep(1);
	        else{
	        	usleep(5);
	        	fail_cnt = 0;
	        }
            continue;
        }
        

        set_next_job(job);
        reschedule();
    }
    CURRENT->status = RUNNING;

    return 1;
}
