#include <pthread.h>
#include <stdarg.h>

register int tid __asm__("ebx");

struct s{
	char a[100];
};

struct s test(int num, ...){
	struct s a;
	return a;
	va_list varlist;
	va_start(varlist, num);
	int size = 0;
	for(int i = 0;i < num; i++){
		size = va_arg(varlist, int);
		printf("%d\n",sizeof(int));
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
			// case sizeof(long):
				printf("long\n");

				va_arg(varlist, long);
				break;
			default:
			printf("size:%d\n",size );
				for(int j = 0; j < size;j+=sizeof(int)){
					// char c = va_arg(varlist, int);
					int c = va_arg(varlist,int);
					printf("%c\n",(char)c);
				}

		}
	}

}

int main(){
	struct s b;
	for(int i = 0; i < sizeof(struct s);i++){
		b.a[i] = 'T';
	}
	char a = 'c';
	float f = 4;
	b = test(3,sizeof(a),sizeof(b),sizeof(2),a,b,2,);
}
