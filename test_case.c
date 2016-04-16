#include "thread.h"
#include "gregor.h"
#include <stdarg.h>



char test2(char a){
	printf("this is %c\n",a);
	return a+1;
	
}


float test1(double a){
	printf("%f\n",a);
	return a+1.2;
}

float test3(int a,...){
	va_list varlist;
	va_start(varlist, a);
	double size = va_arg(varlist, double);
	return size;

}

int tt(int argc, char** argv){
	float ret1;
	char ret2 = 'C';
	test3(2,(float)4.0);
	//spawn(FLOAT,&ret1, test1,1, ARG(double,3.0));
	spawn(CHAR,&ret2, test2, 1, sizeof(char),'a');
	printf("here\n");
	__gregor_sync();
	printf("in tt:%c\n",ret2);
	return 3;
}

int main(int argc, char** argv){
	int ret;
	gregor_main(&ret,tt,4,argv);
}
