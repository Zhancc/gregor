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
#if 0
static int linux_get_affinity_count ()
{
    long system_cores = sysconf(_SC_NPROCESSORS_ONLN);
    int affinity_cores = 0;

#if defined HAVE_PTHREAD_AFFINITY_NP

#if defined (CPU_ALLOC_SIZE) && ! defined(DONT_USE_CPU_ALLOC_SIZE)
    // Statically allocated cpu_set_t's max out at 1024 cores.  If
    // CPU_ALLOC_SIZE is available, use it to support large numbers of cores
    size_t cpusetsize = CPU_ALLOC_SIZE(system_cores);
    cpu_set_t *process_mask = (cpu_set_t *)__cilkrts_malloc(cpusetsize);

    // Get the affinity mask for this thread
    int err = pthread_getaffinity_np(pthread_self(),
                                     cpusetsize,
                                     process_mask);

    // Count the available cores.
    if (0 == err)
        affinity_cores = CPU_COUNT_S(cpusetsize, process_mask);

    __cilkrts_free(process_mask);

#else
    // CPU_ALLOC_SIZE isn't available, or this is the Intel compiler build
    // and we have to support RHEL5.  Use a statically allocated cpu_set_t

    cpu_set_t process_mask;

    // Extract the thread affinity mask
    int err = pthread_getaffinity_np(pthread_self(),
                                     sizeof(process_mask),
                                     &process_mask);

    if (0 == err)
    {
        // We have extracted the mask OK, so now we can count the number of
        // threads in it.  This is linear in the maximum number of CPUs
        // available, We could do a logarithmic version, if we assume the
        // format of the mask, but it's not really worth it. We only call
        // this at thread startup anyway.
        int i;
        for (i = 0; i < CPU_SETSIZE; i++)
        {
            if (CPU_ISSET(i, &process_mask))
            {
                affinity_cores++;
            }
        }
    }
#endif  // CPU_ALLOC_SIZE
#endif  //  ! defined HAVE_PTHREAD_AFFINITY_NP

    // If we've got a count of cores this thread is supposed to use, that's
    // the number or cores we'll use.  Otherwise, default to the number of
    // cores on the system.
    if (0 == affinity_cores)
        return system_cores;
    else
        return affinity_cores;
}
#endif

void init(void){
	NUM_PROCESSOR = sysconf(_SC_NPROCESSORS_ONLN);
	pagesize = getpagesize();
	mem_op = 0;
	//NUM_PROCESSOR = 33;
	if(NUM_PROCESSOR < 0){
		__gregor_error("Initialization fail");
	}
	printf("num %d\n",NUM_PROCESSOR);
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
	 for(int i = 0; i < NUM_WORKER; i++){
	 	printf("tid %d: num %d\n",i,mstate.worker_info[i].num_work );
	 }
//	free(mstate.worker_info);
//	Deque_free(mstate.deque);
}


