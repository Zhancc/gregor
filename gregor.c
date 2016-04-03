#include "gregor.h"
#include "gregor_error.h"
#include "thread.h"
#include <stdarg.h>

#define SET_RET(T) do{ }while(0)

/*the first argument being function ptr, 
* the rest argument should be the argument list to routine:
* sizeof(arg1),arg1,sizeof(arg2),arg2.....
* return negative if anything bad happens, or 0 otherwise
*/
int spawn(void* routine, int num_arg, ...){
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
	__asm__(


		);

}
