#include "sched.h"
#include "init.h"
#include <sys/mman.h>
#include <unistd.h>



/* reschedule the current thread to next job, not pthread! the current context should have been saved*/
/* this will first decide use which swap routine,
*  set the job state of next job accordingly
*  do swap and never return.
*  after swap it should enqueue current if it's not NULL and set current to next job and reset next job
*/
void do_reschedule(void *esp) {
    /*save the esp first*/
    if (CURRENT == NULL) {
        /* if CURRENT is NULL, then it's using the pthread stack now*/
        CURRENT_WORKER->p_esp = esp;
    } else {
        /* otherwise, it's executing some job */
        CURRENT->esp = esp;
    }
    if (CURRENT_WORKER->next_job) {
#warning: get the next job from CURRENT_WORKER->next_job, it should have been dequeued
        switch (CURRENT_WORKER->next_job->status) {
            case SPAWN:
                CURRENT_WORKER->next_job->status = RUNNING;
                switch_context_to_new(CURRENT_WORKER->next_job->esp);
            case RUNNING:
                switch_context(CURRENT_WORKER->next_job->esp);
                break;
            case SYNC:
                CURRENT_WORKER->num_work++;
                switch_context(CURRENT_WORKER->next_job->esp);
                __gregor_panic("switch to a synced job");
                break;
        }
    } else {
        __gregor_panic("no next_job when do_reschedule");
    }
    /*this function should never return*/
    return;
}

/*after swap it should enqueue current if it's not NULL and set current to next job and reset next job*/
void do_reschedule_reset_current() {
    jcb *prev_cur = CURRENT;
    CURRENT = CURRENT_WORKER->next_job;
    CURRENT_WORKER->next_job = NULL;
    if (prev_cur) {
        if (prev_cur->status == RUNNING)
            AddNodeToTail(CURRENT_WORKER->deque, prev_cur);
        else
            AddNodeToTail(CURRENT_WORKER->deque, prev_cur);
    }
    return;
}

static const unsigned RNGMOD = ((1ULL << 32) - 5);
static const unsigned RNGMUL = 69070U;

void gregor_srand(unsigned long seed) {
    seed %= RNGMOD;
    seed += (seed == 0); /* 0 does not belong to the multiplicative
                            group.  Use 1 instead */
    CURRENT_WORKER->rand = seed;
    // CURRENT_WORKER->rand = seed;
}

unsigned long gregor_rand() {
    unsigned state = CURRENT_WORKER->rand;
    state = (unsigned) ((RNGMUL * (unsigned long long) state) % RNGMOD);
    CURRENT_WORKER->rand = state;
    return state;
    // CURRENT_WORKER->rand = (unsigned long)(CURRENT_WORKER->rand * 110351)+12345;//1103515245 + 12345;
    // return (CURRENT_WORKER->rand >> 16);
}

/*grab the work from its queue or sleep on semaphore until waken up*/
#warning: currently looping looking for the job

jcb *do_pick_work(int sync) {
    // jcb* node = NULL;
    // pthread_mutex_lock(&CURRENT_WORKER->deque->queue_lock);
    // while (isEmpty(CURRENT_WORKER->deque)) {
    // 	pthread_cond_wait(&CURRENT_WORKER->deque->queue_cond, &mstate.deque->queue_lock);
    // }
    // node = GetNodeFromHead(CURRENT_WORKER->deque);
    // pthread_mutex_unlock(&CURRENT_WORKER->deque->queue_lock);
    // return node;
    jcb *node = NULL;
    unsigned long victim;
    /*check local first*/
    // if(!isEmpty(CURRENT_WORKER->deque)){
    // 	pthread_mutex_lock(&CURRENT_WORKER->deque->queue_lock);
    // 	if(isEmpty(CURRENT_WORKER->deque)){
    // 		pthread_mutex_unlock(&CURRENT_WORKER->deque->queue_lock);
    // 	}else{
    // 		node = GetNodeFromTail(CURRENT_WORKER->deque);
    // 		pthread_mutex_unlock(&CURRENT_WORKER->deque->queue_lock);
    // 		return node;
    // 	}
    // }
    // node = GetNodeFromTail(CURRENT_WORKER->deque);
    // if(node)
    // 	return node;

    while (!node) {
        if (!isEmpty(CURRENT_WORKER->deque)) {
            node = GetNodeFromTail(CURRENT_WORKER->deque);
            if (node)
                return node;
        }

        int unvisisted = (1 << (NUM_WORKER)) - 1;
        unvisisted &= ~(1 << tid);
        /* steal */
        while (!node && unvisisted) {
            victim = gregor_rand() % (NUM_WORKER);
            // if(unvisisted&(1<<victim)){
            // 	printf("%d:again\n", tid);
            // }
            if (victim != tid && unvisisted & (1 << victim)) {
                unvisisted &= ~(1 << victim);
                /* attempt to steal */
                node = work_steal(victim);

            }
        }

        /* at this point, no one has any work to do*/
        if (sync && node == NULL) {
            // for(int i = 0; i < 12; i++){
            // 	if(mstate.worker_info[i].setup&&mstate.worker_info[i].deque->size != 0){
            // 		printf("%d:should steal %d\n",tid,i);
            // 	}
            // }
            // if(tid == 32){
            // 	for( int i = 0; i < NUM_WORKER; i++){
            // 		if((mstate.worker_info[victim].setup) && mstate.worker_info[victim].deque->size){
            // 			printf("%d have %d:%s\n",i,mstate.worker_info[victim].deque->size,"fail again 32" );

            // 		}
            // 	}
            // }
            usleep(1);

        } else {
            break;
        }
    }

    return node;
}

jcb *pick_work() {
    return do_pick_work(1);
}

jcb *try_pick_work() {
    return do_pick_work(0);

    // jcb* node = NULL;
    // pthread_mutex_lock(&CURRENT_WORKER->deque->queue_lock);
    // while (isEmpty(CURRENT_WORKER->deque)) {
    // 	pthread_mutex_unlock(&CURRENT_WORKER->deque->queue_lock);
    // 	return NULL;
    // }
    // node = GetNodeFromHead(CURRENT_WORKER->deque);
    // pthread_mutex_unlock(&CURRENT_WORKER->deque->queue_lock);
    // return node;
}

jcb *work_steal(int victim) {
    if (!(mstate.worker_info[victim].setup)) {
        return NULL;
    }

    // pthread_mutex_lock(&(mstate.worker_info[victim].deque->queue_lock));
    // if(mstate.worker_info[victim].deque->size == 0){
    // 	pthread_mutex_unlock(&(mstate.worker_info[victim].deque->queue_lock));
    // 	return NULL;
    // }
    jcb *j = GetNodeFromHead((mstate.worker_info[victim].deque));
    // pthread_mutex_unlock(&(mstate.worker_info[victim].deque->queue_lock));
    return j;

}
/* free the address space of current job */
/* we should have been working at pthread stack at this point */
void free_current() {
    FreeMemory(CURRENT_WORKER->mm, CURRENT->mmap_addr, CURRENT->mmap_size);
    // munmap(CURRENT->mmap_addr, CURRENT->mmap_size);
    CURRENT = NULL;
}


#warning: x87 registers seem to be caller-saved, so worry

void fstate_save() {
    // __asm__(" fxsave %0"
    // 	:
    // 	:"m"(CURRENT->fstate)
    // 	);
}

void fstate_restore() {
    // __asm__(" fxrstor %0"
    // 	:
    // 	:"m"(CURRENT->fstate)
    // 	);
}
