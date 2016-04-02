#include "gregor.h"
#include "gregor_error.h"
#include <stdarg.h>

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

}
