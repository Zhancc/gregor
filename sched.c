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
}

unsigned long gregor_rand() {
    unsigned state = CURRENT_WORKER->rand;
    state = (unsigned) ((RNGMUL * (unsigned long long) state) % RNGMOD);
    CURRENT_WORKER->rand = state;
    return state;
}

/*grab the work from its queue or sleep on semaphore until waken up*/
#warning: currently looping looking for the job

jcb *do_pick_work(int sync) {
    jcb *node = NULL;
    unsigned long victim;

    int fail_cnt = 0;
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

            if (victim != tid && unvisisted & (1 << victim)) {
                unvisisted &= ~(1 << victim);
                /* attempt to steal */
                node = work_steal(victim);

            }
        }

        /* at this point, no one has any work to do*/
        if (sync && node == NULL) {
            fail_cnt++;
            if(fail_cnt < 300)
	            usleep(1);
	        else{
	        	usleep(5);
	        	fail_cnt = 0;
	        }
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
}

jcb *work_steal(int victim) {
    if (!(mstate.worker_info[victim].setup) || isEmpty(mstate.worker_info[victim].deque)) {
        return NULL;
    }

    jcb *j = GetNodeFromHead((mstate.worker_info[victim].deque));
    return j;

}
/* free the address space of current job */
/* we should have been working at pthread stack at this point */
void free_current() {
    FreeMemory(CURRENT_WORKER->mm, CURRENT->mmap_addr, CURRENT->mmap_size);
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
