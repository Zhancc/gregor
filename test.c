#include <pthread.h>
#include <stdarg.h>
#include <semaphore.h>
#include "gregor.h"
#include <signal.h>

register int tid __asm__("ebx");
void *esp = 0;
int g = 0;

void ex() {
    pthread_t pid = pthread_self();
    int i = 0;
    while (i++ < 1)
        L3:
        printf("%d:son,pid:%d\n", tid, pid);
    return;

}

long double another(char c) {
    // float c = ar;
    pthread_t pid = pthread_self();
    int i = 0;
    while (i++ < 1)
        L3:
        printf("%d:dad,pid:%d\n", tid, pid);
    return 1;
}

void *test(void *args) {

    tid = 1;
    void *st = malloc(0x1000);
    pthread_t pid = pthread_self();

    L3:
    printf("%d:son,pid:%d\n", tid, pid);
    while (1) {
        printf("still alive\n");
    }

    __asm__(
    // "push %%edx\t\n"
    // "pusha\t\n"
    "movl %%esi, (%%eax)\t\n"
            "movl %%edx, (%%esi)\t\n"
            "jmp *%%ecx\t\n"
    :
    :"a"(&esp), "c"(ex), "d"(another), "S"((char *) st + 1000)
    );
    while (1) {
        printf("1 : %d", tid);
    }
}

void __cleanup(int a) {
    short b = (short) a;
    printf("%hx\n", (short) b);
}

int gregor_main__(int argc, char **argv) {

}

int main(int argc, char **argv) {
    tid = 0;
    pthread_t p;
    pthread_t pid = pthread_self();

    // __gregor_sync();
    // gregor_main(NULL, gregor_main__, argc, argv );
    // return;
    __cleanup(0xffeeccaa);

    // return;
    pthread_create(&p, NULL, test, NULL);
    L3:
    printf("%d:dad,pid:%d\n", tid, pid);
    int i = 0;
    while (i++ < 100) {
        // printf("0 : %d",tid);
    }
    pthread_kill(p, SIGKILL);
    while (1) {
        printf("dad\n");
    }
    // switch_context(esp);
    char c = 0;
    long double tt = another(c);
}
