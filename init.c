/* 
* This file describe the initialization of Gregor's environment
*/

#include <unistd.h>
#include <stdlib.h>
#include "gregor_error.h"
#include "thread.h"
#include "init.h"

static int NUM_PROCESSOR;
#define NUM_WORKER (NUM_PROCESSOR*2) //minus one because we include the calling thread


void _init(void){
	NUM_PROCESSOR = sysconf(_SC_NPROCESSORS_ONLN);
	if(NUM_PROCESSOR < 0){
		__gregor_error("Initialization fail");
	}
	
	/* init mstate*/
	mstate.worker_info = (wstate*)malloc((NUM_WORKER)*sizeof(wstate));

	for(int i = 1 ; i < NUM_WORKER ; i++){
		Pthread_create(&(mstate.worker_info[i].threadId),NULL,__gregor_worker_init, (void*)(long)i);
	}

}



void _fini(void){
	free(mstate.worker_info);
}
