#include <stdarg.h>
#include <sys/mman.h>
#include <unistd.h>
#include "gregor.h"
#include "gregor_error.h"
#include "thread.h"
/*the second argument being function ptr, 
* the rest argument should be the argument list to routine:
* sizeof(arg1),arg1,sizeof(arg2),arg2.....
* return negative if anything bad happens, or 0 otherwise
*/

jcb* create_job(void* ret, void* routine, int num_arg, ...){
	/* get a stack*/
	int pagesize = getpagesize();
	void* task_stack = mmap(NULL, pagesize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0 );
	if(task_stack == (void*)-1)
		__gregor_error("mmap failed");

	/* the end of stack is used as jcb*/
	jcb* job = (jcb*)((char*)task_stack + pagesize) - 1;
	job->mmap_addr = task_stack;
	job->mmap_size = pagesize;
	job->status = SPAWN;
	
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
			1:
			2:
			4:
				sum+=4;
				break;
			default:
				sum+=size;
				break;
		}
	}
	top = (char*)top - sum;

	#warning: we rely on the fact that the arguments are passed by stack and stack grows downwards
	if(num_arg > 0)
		memcpy(top,&num_arg + num_arg + 1,sum);

	top = (char*)top - sum;
	top--;
	*top = (int*)ret;
	job->esp = (void*)top;
	return job;
}


int add_job(jcb* job){

}

#if 0
int spawn(void*, void* routine, int num_arg, ...){
	if(num_arg%2!=0){
		return EARG;
	}

	void* (*start_routine)() = (void*(*)())routine;
	/*form a continuation*/
	int sum = 0;
	va_list varlist;
	va_start(varlist, num_arg);

	int size = 0;
	for( int i = 0;i < num_arg; i++){
		switch(size = va_arg(varlist, int)){
			case 1:
			case 2:
			case 3:
			 sum+=sizeof(int);
			 break;
			default:
			 sum+=size;
			 break;
		}
	}

	/* form a continuation begin */
	continuation_ptr con = (continuation_ptr)malloc(sizeof(start_routine)+sizeof(int)+sum);
	*(int*)con = num_arg;
	con = (int*)con + 1;

	for(int i = 0; i < sum; i+=sizeof(int)){
		*(int*)con = va_arg(varlist,int);
		con = (int*)con + 1;
	}
	*(void**)con = start_routine;
	con = (char*)con + sizeof(start_routine);

	/* form a continuation end */

	#warning test jump to the target function
	
	return 0;
}
#endif

