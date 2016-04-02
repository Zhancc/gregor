/* 
* This file describe the initialization of Gregor's environment
*/

#include "thread.h"
#include <unistd.h>
#include <stdlib.h>
#include "gregor_error.h"

static int NUM_PROCESSOR;
#define NUM_WORKER (NUM_PROCESSOR*2-1) //minus one because we include the calling thread


void _init(void){
	NUM_PROCESSOR = sysconf(_SC_NPROCESSORS_ONLN);
	if(NUM_PROCESSOR < 0){
		__gregor_error("Initialization fail");
	}
	
	/* init mstate*/
	mstate.worker_info = (wstate*)malloc((NUM_WORKER+1)*sizeof(wstate));

	for(int i = 1 ; i < NUM_WORKER+1; i++){
		Pthread_create(&(mstate.worker_info[i].threadId),NULL,__gregor_worker_init, (void*)(long)i);
		mstate.worker_info[i].queue_head = mstate.worker_info[i].queue_rear = 0;
	}

	mstate.worker_info[0].threadId = Pthread_self();
	mstate.worker_info[0].queue_head = mstate.worker_info[0].queue_rear = 0;

	tid = 0;
}

void _fini(void){
	free(mstate.worker_info);
}