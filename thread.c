#include "thread.h"

void Pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void*), void *restrict arg){
	if(pthread_create(thread, attr, start_routine, arg) < 0){
		__gregor_error("thread creation fail");
	}

}

pthread_t Pthread_self(void){
	return pthread_self();
}

void* __gregor_worker_init(void* threadid){
	long tid = (long)threadid;

	__gregor_do_work(tid);
}

void __gregor_do_work(int threadId){
	/*grab the work from its queue or sleep on semaphore until waken up*/
	#warning to be done

	/*jump to the work specified*/
	#warning to be done

	/*cleanup*/
	#warning to be done
}
