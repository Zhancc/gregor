#include <time.h>
#include "thread.h"
#include "gregor.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

int fib(int n)
{
     if (n < 2)
	  return (n);
     else {
	  int x, y;
      spawn(INT,&x, fib, 1, INT,n-1);
      spawn(INT,&y, fib, 1, INT,n-2);

	  __gregor_sync();
	  return (x + y);
     }
}

int g_main(int argc, char *argv[])
{
     int n, result;

     if (argc != 2) {
	  fprintf(stderr, "Usage: fib [<cilk options>] <n>\n");
	  exit(1);
     }
     n = atoi(argv[1]);
     spawn(INT,&result, fib, 1, INT ,n);
     __gregor_sync();

     printf("Result: %d\n", result);
     return 0;
}

int main(int argc, char *argv[]){
    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);
    gregor_main(NULL,g_main,argc,argv);
    // void* addr;
    // int pagesize = getpagesize();
    // for(int i = 0; i < 10000000;i++){
    //     addr = malloc(40);//mmap(NULL, pagesize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0 );
    //     free(addr);
    //     // munmap(addr, pagesize);
    // }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Total time taken by CPU: %.3f\n", elapsed);
}
