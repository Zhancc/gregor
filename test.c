#include <pthread.h>
#include <stdarg.h>
#include <semaphore.h>

register int tid __asm__("ebx");
void* esp = 0;
int g=0;
void ex(){
	int i = 0;
	while(i<1000)
	L3:printf("%d:son\n",tid);
	return;

}

void another(){
	int i = 0;
	while(i<1000)
	L3:printf("%d:dad\n",tid);
}
void* test(void* args){

	tid = 1;
	void* st = malloc(0x1000);
	__asm__(
		// "push %%edx\t\n"
		// "pusha\t\n"
		"movl %%esi, (%%eax)\t\n"
		"movl %%edx, (%%esi)\t\n"
		"jmp *%%ecx\t\n"
		:
		:"a"(&esp),"c"(ex),"d"(another),"S"((char*)st+1000)
		);
	while(1){
		printf("1 : %d",tid);
	}
}



int main(){
	tid = 0;
	pthread_t p;
	pthread_create(&p,NULL,test,NULL);

	while(esp == 0){
		// printf("0 : %d",tid);
	}
	switch_context(esp);
}
