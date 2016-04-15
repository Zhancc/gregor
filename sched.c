#include "sched.h"
#include <sys/mman.h>


/* reschedule the current thread to next job, not pthread! the current context should have been saved*/
/* this will first decide use which swap routine,
*  set the job state of next job accordingly
*  do swap and never return.
*  after swap it should enqueue current if it's not NULL and set current to next job and reset next job
*/
void do_reschedule(void* esp){

	/*save the esp first*/
	if(CURRENT == NULL){
	/* if CURRENT is NULL, then it's using the pthread stack now*/
		CURRENT_WORKER->p_esp = esp;
	}else{
	/* otherwise, it's executing some job */
		CURRENT->esp = esp;
	}
	if(CURRENT_WORKER->next_job){
		#warning: get the next job from CURRENT_WORKER->next_job, it should have been dequeued
		switch(CURRENT_WORKER->next_job->status){
			case SPAWN:
				CURRENT_WORKER->next_job->status = RUNNING;
				switch_context_to_new(CURRENT_WORKER->next_job->esp);
			case RUNNING:
				switch_context(CURRENT_WORKER->next_job->esp);
				break;
			case SYNC:
				__gregor_panic("switch to a synced job");
				break;
		}
	}else{
		__gregor_panic("no next_job when do_reschedule");
	}
	/*this function should never return*/
	return;
}

/*after swap it should enqueue current if it's not NULL and set current to next job and reset next job*/
void do_reschedule_reset_current(){
	jcb* prev_cur = CURRENT;
	CURRENT = CURRENT_WORKER->next_job;
	CURRENT_WORKER->next_job = NULL;
	if(prev_cur){
		add_job_tail(prev_cur);
	}
	return;

}

/*grab the work from its queue or sleep on semaphore until waken up*/
#warning: to be implemented
jcb* pick_work(){
	Node* node = NULL;
	pthread_mutex_lock(&mstate.deque->queue_lock);
	while (isEmpty(mstate.deque)) {
		pthread_cond_wait(&mstate.deque->queue_cond, &mstate.deque->queue_lock);
	}
	node = GetNodeFromHead(mstate.deque);
	pthread_mutex_unlock(&mstate.deque->queue_lock);
	jcb* job = node->job;
	free(node);
	return job;
}


/* free the address space of current job */
/* we should have been working at pthread stack at this point */
void free_current(){
	munmap(CURRENT->mmap_addr, CURRENT->mmap_size);
	CURRENT = NULL;
}
