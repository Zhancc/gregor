#include <unistd.h>
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
	if (pthread_create(thread, attr, start_routine, arg) < 0){
		__gregor_error("thread creation fail");
	}
}

pthread_t Pthread_self(void){
	return pthread_self();
}

void do_gregor_main(void* p_esp, void* dummy_ret, int* return_ptr, int (*routine)(int, char**), int argc, char** argv){
	/* initialize the master thread */
	init();
	main_arg ma;
	ma.p_esp = p_esp;
	ma.return_ptr = return_ptr;
	ma.routine = routine;
	ma.argc = argc;
	ma.argv = argv;
	pthread_create(&(mstate.worker_info[0].threadId),NULL,do_gregor_main_init, (void*)&ma);

	#warning: wait can be interrupted by signal
	sem_wait(&(mstate.sem));
	fini();
}

void init_data_structure(){
	CURRENT_WORKER->cur = CURRENT_WORKER->next_job = NULL;
}
/*this function should not return, the threads should be blocked in loop and killed by master thread*/
void* do_gregor_main_init(void* ptr){
	main_arg* ma = (main_arg*)ptr;
	tid = 0;
	init_data_structure();
	jcb* main_job = create_job(NULL,INT,ma->return_ptr,ma->routine, 2 ,sizeof(int),sizeof(char**), ma->argc, ma->argv);
	add_job_tail(main_job);
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
		printf("this is worker %d\n",tid );
	/*jump to the work specified*/
		if(next==NULL){
			__gregor_panic("pick a NULL work in __gregor_do_work_loop");
		}
		set_next_job(next);
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
					:"m"(*(float*)(CURRENT->ret_ptr))
					);
				break;
			case DOUBLE:
				__asm__(
					"fstpl %0\t\n"
					:
					:"m"(*(double*)(CURRENT->ret_ptr))
					);
				break;
			case LONG_DOUBLE:
				__asm__(
					"fstpt %0\t\n"
					:
					:"m"(*(long double*)(CURRENT->ret_ptr))
					);
				break;
			case PTR:
				*(int*)(CURRENT->ret_ptr) = eax;
				break;
		}
	}
	#warning: refinement: currently just yield the processor
	while(CURRENT->join_counter){
		usleep(1);
	}	

	if(CURRENT->parent){
		int ret;
		jcb* j = CURRENT->parent;
		ret = __sync_fetch_and_sub(&(j->join_counter), 1);
		if(ret<=0){
			__gregor_panic("join_counter incorrect");
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

Node* Node_new(jcb* job) {
    Node *p = (Node *)malloc(sizeof(Node));
    p->job = job;
    p->next = NULL;
    p->prev = NULL;
    return p;
}

Deque* Deque_new() {
    Deque *p = (Deque *)malloc(sizeof(Deque));
    p->head_node = NULL;
    p->tail_node = NULL;
    pthread_cond_init(&p->queue_cond, NULL);
    pthread_mutex_init(&p->queue_lock, NULL);
    return p;
}

void AddNodeToTail(Deque* deque, jcb* job) {
    Node* node = Node_new(job);
    if (deque->tail_node == NULL) {
        deque->head_node = node;
        deque->tail_node = node;
    } else {
        Node* prev = deque->tail_node;
        prev->next = node;
        node->prev = prev;
        deque->tail_node = node;
    }
}

void AddNodeToHead(Deque* deque, jcb* job) {
	Node* node = Node_new(job);
	if (deque->head_node == NULL) {
		deque->head_node = node;
		deque->tail_node = node;
	} else {
		Node* next = deque->head_node;
		node->next = next;
		next->prev = node;
		deque->head_node = node;
	}
}


Node* GetNodeFromTail(Deque* deque) {
    if (deque->tail_node == NULL) return NULL;
    Node* prev = deque->tail_node;

    if (prev->prev == NULL) {
        deque->head_node = NULL;
    } else {
        prev->prev->next = NULL;
    }
    deque->tail_node = prev->prev;

    return prev;
}

Node* GetNodeFromHead(Deque *deque) {
    if (deque->head_node == NULL) return NULL;
    Node *head = deque->head_node;

    if (head->next == NULL) {
        deque->tail_node = NULL;
    } else {
        head->next->prev = NULL;
    }
    deque->head_node = head->next;
    return head;
}

// void print_list_from_head(Node* head) {
//     while (head != NULL) {
//         printf("%d\n", head->value);
//         head = head->next;
//     }
// }

// void print_list_from_tail(Node* tail) {
//     while (tail != NULL) {
//         printf("%d\n", tail->value);
//         tail = tail->prev;
//     }
// }

int isEmpty(Deque *deque) {
    return deque->head_node == NULL;
}
