/*
* This file describe the initialization of Gregor's environment
*/

#include <unistd.h>
#include <stdlib.h>
#include "gregor_error.h"
#include "thread.h"
#include "init.h"

// int NUM_PROCESSOR;
// #define NUM_WORKER (NUM_PROCESSOR/2) //minus one because we include the calling thread


void init(void){
	NUM_PROCESSOR = sysconf(_SC_NPROCESSORS_ONLN);
	pagesize = getpagesize();
	mem_op = 0;
	if(NUM_PROCESSOR < 0){
		__gregor_error("Initialization fail");
	}

	/* init mstate*/
	mstate.worker_info = (wstate*)malloc((NUM_WORKER)*sizeof(wstate));
	mstate.deque = Deque_new();
	for( int i = 0 ; i < NUM_WORKER ; i++){
		mstate.worker_info[i].setup = 0;
	}

	for(int i = 1; i < NUM_WORKER; i++){
		Pthread_create(&(mstate.worker_info[i].threadId), NULL, __gregor_worker_init, (void*)(long)i);
	}

}



void fini(void){
	// for(int i = 0; i < NUM_WORKER; i++){
	// 	printf("tid %d: num %d\n",i,mstate.worker_info[i].num_work );
	// }
//	free(mstate.worker_info);
//	Deque_free(mstate.deque);
}
