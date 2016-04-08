#include "thread.h"
#include "gregor.h"
#include "sched.h"
#include "init.h"
typedef struct main_arg{
	void* p_esp; 
	int* return_ptr; 
	int (*routine)(int, char**);
	int argc; 
	char** argv;
} main_arg;

void Pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void*), void *restrict arg){
	if(pthread_create(thread, attr, start_routine, arg) < 0){
		__gregor_error("thread creation fail");
	}

}

pthread_t Pthread_self(void){
	return pthread_self();
}

void do_gregor_main(void* p_esp, void* dummy_ret, int* return_ptr, int (*routine)(int, char**),int argc, char** argv){
		/* initialize the master thread */
		_init();
		main_arg ma;
		ma.p_esp = p_esp;
		ma.return_ptr = return_ptr;
		ma.routine = routine;
		ma.argc = argc;
		ma.argv = argv;
		Pthread_create(&(mstate.worker_info[0].threadId),NULL,do_gregor_main_init, (void*)&ma);
		mstate.worker_info[0].queue_head = mstate.worker_info[0].queue_rear = 0;
		mstate.worker_info[0].cur = NULL;

		#warning: wait can be interrupted by signal
		sem_wait(&(mstate.sem));
		_fini();
}

static void init_data_structure(){
	CURRENT_WORKER->cur = CURRENT_WORKER->next_job = NULL;

}
/*this function should not return, the threads should be blocked in loop and killed by master thread*/
void* do_gregor_main_init(void* ptr){
		main_arg* ma = (main_arg*)ptr;
		tid = 0;
		init_data_structure();
		jcb* main_job = create_job(NULL,INT,ma->return_ptr,ma->routine, 2 ,sizeof(int),sizeof(char**), ma->argc, ma->argv);
		add_job(main_job);
		__gregor_do_work_loop();
		return NULL;
}


/*this function should not return, the threads should be blocked in loop and killed by master thread*/
void* __gregor_worker_init(void* threadid){
	tid = (int)(long)threadid;
	init_data_structure();
	__gregor_do_work_loop();

	return NULL;
}

/*invariant: always execute this function in ptrhead stack*/
void __gregor_do_work_loop(){

	while(1){
		jcb* next = pick_work();
	/*jump to the work specified*/
		if(next==NULL){
			__gregor_panic("pick a NULL work in __gregor_do_work_loop");
		}
		CURRENT_WORKER->next_job = jcb;
		reschedule_from_pthread();

	}
}

/* cleanup should not return, it would switch back to __gregor_do_work or main function*/
void do_cleanup(unsigned int eax, unsigned int edx){
	unsigned char al = (unsigned char)eax;
	unsigned short ax = (unsigned short)eax; 
	if(CURRENT->ret_ptr!=NULL){
		switch(CURRENT->ret_type){
				case VOID :
					break;
				case SIGNED_CHAR:
					*(signed char*)(CURRENT->ret_ptr) = al;
					break;
				case UNSIGNED_CHAR:
					*(unsigned char*)(CURRENT->ret_ptr) = al;
					break;
				case CHAR:
					*(char*)(CURRENT->ret_ptr) = al;
					break;
				case SHORT_INT:
					*(short int*)(CURRENT->ret_ptr) = ax;
					break;
				case SIGNED_SHORT:
					*(signed short*)(CURRENT->ret_ptr) = ax;
					break;
				case UNSIGNED_SHORT_INT:
					*(unsigned short*)(CURRENT->ret_ptr) = ax;
					break;
				case INT:
					*(int*)(CURRENT->ret_ptr) = eax;
					break;
				case UNSIGNED_INT:
					*(unsigned int*)(CURRENT->ret_ptr) = eax;
					break;				
				case LONG_INT:
					*(long int*)(CURRENT->ret_ptr) = eax;
					break;					
				case UNSIGNED_LONG_INT:
					*(unsigned long int*)(CURRENT->ret_ptr) = eax;
					break;					
				case LONG_LONG_INT:
					*(long int*)(CURRENT->ret_ptr) = eax;
					*((long int*)(CURRENT->ret_ptr)+1) = edx;
					break;					
				case UNSIGNED_LONG_LONG_INT:
					*(unsigned long int*)(CURRENT->ret_ptr) = eax;
					*((unsigned long int*)(CURRENT->ret_ptr)+1) = edx;	
					break;		
				case FLOAT:
					__asm__(
						"fstps %0\t\n"
						:
						:"m"(CURRENT->ret_ptr)
						);
					break;
				case DOUBLE:
					__asm__(
						"fstpl %0\t\n"
						:
						:"m"(CURRENT->ret_ptr)
						);				
					break;
				case LONG_DOUBLE:
					__asm__(
						"fstpt %0\t\n"
						:
						:"m"(CURRENT->ret_ptr)
						);				
					break;
				case PTR:
					*(int*)(CURRENT->ret_ptr) = eax;
					break;
		}
	}
	__gregor_sync();
	if(CURRENT->parent){
		int ret;
		jcb* j = CURRENT->parent;
		ret = __sync_fetch_and_sub(&(j->join_counter), 1);
		if(ret<=0){
			__gregor_panic("join_counter incorrect");
		}
		/*iteratively update the join counter of predescendents*/
		while(!(j = j->parent)&& (ret == 1) ){
		 ret = __sync_fetch_and_sub(&(j->join_counter), 1);
 		 if(ret<=0){
		 __gregor_panic("join_counter incorrect");
		 }
		}

	}else{
		/*invariant: at this point, no other threads should be working*/
		/*wake up the master thread*/
		sem_post(&(mstate.sem));
	}
	/* esp should be save in context of work loop, i.e. pthread stack */
	void* esp = CURRENT_WORKER->p_esp;
	swicth_free_current(esp);
}

