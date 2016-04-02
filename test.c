#include <pthread.h>
#include <stdarg.h>

register int tid __asm__("ebx");

struct s{
	char a[100];
};

void* test(int num, ...){
	va_list varlist;
	va_start(varlist, num);
	int size = 0;
	for(int i = 0;i < num; i++){
		size = va_arg(varlist, int);
		printf("%d\n",size);
		switch(size){
			case sizeof(char):
				printf("char\n");
				va_arg(varlist, int);
				break;
			case sizeof(int):
				printf("int\n");
				va_arg(varlist, int);
				break;
			case sizeof(short):
				printf("short\n");
				va_arg(varlist, int);
				break;
			case sizeof(long):
				printf("long\n");

				va_arg(varlist, long);
				break;
			default:
				// for(int j = 0; j < size;j++){
					// char c = va_arg(varlist, int);
					printf("%c\n",va_arg(varlist, int));
				// }

		}
	}

}

int main(){
	struct s b;
	for(int i = 0; i < sizeof(struct s);i++){
		b.a[i] = 'T';
	}
	char a = 'c';
	test(4,sizeof(a),a,sizeof(2), 2,sizeof(b), b);
}
