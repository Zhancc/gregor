#include <stdarg.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "gregor.h"
#include "gregor_error.h"
#include "thread.h"
/*
* the second argument being function ptr, 
* the rest argument should be the argument list to routine:
* sizeof(arg1),arg1,sizeof(arg2),arg2.....
* return negative if anything bad happens, or 0 otherwise
*/

jcb* create_job(void* dummy_ret, enum return_type rt, void* return_ptr, void* routine, int num_arg, ...){
	/* get a stack*/
	int pagesize = getpagesize();

	#warning: find a stack map from the cache to be implemented
	void* task_stack = mmap(NULL, pagesize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0 );
	if(task_stack == (void*)-1)
		__gregor_error("mmap failed");

	/* the end of stack is used as jcb*/
		/*initialize job control block*/
	jcb* job = (jcb*)((char*)task_stack + pagesize) - 1;
	job->mmap_addr = task_stack;
	job->mmap_size = pagesize;
	job->status = SPAWN;
	job->ret_type = rt;
	job->ret_ptr = return_ptr;
	job->join_counter = 0;
	job->parent = CURRENT;
	/*update the join counter of the parrent*/
	__sync_fetch_and_add(&(job->parent->join_counter), 1);

	/* top of stack */
	int* top = STACK_ALIGN(int *, job);
	top = top - 1;

	/*enqueue-stack*/
	va_list varlist;
	va_start(varlist, num_arg);
	int sum = 0;
	for( int i = 0 ; i < num_arg; i++){
		int size = va_arg(varlist, int);
		switch(size){
			/* even sizeof(char), sizeof(int) is pushed to stack */
			case 1:
			case 2:
			case 4:
				sum+=4;
				break;
			default:
				sum+=size;
				break;
		}
	}
	top = (void*)((char*)top - sum);

	#warning: we rely on the fact that the arguments are passed by stack and stack grows downwards
	if(num_arg > 0)
		memcpy(top,&num_arg + num_arg + 1,sum);

	// top = (void*)((char*)top - sum);
	top--;
	
	*(int**)top = (int*)cleanup; /* return to cleanup routine after finishing the job*/
	top--;
	*(int**)top = (int*)routine;
	job->esp = (void*)top;
	return job;
}


#warning: add the job to some queue to be implemented
void add_job(jcb* job){	
	return ;
}

/* wait until all the descendents complete */
#warning: refinement: currently just yield the cpu and sleep 
int __gregor_sync(){
	while(CURRENT->join_counter){
		usleep(1);
	}
	return 1;
}
