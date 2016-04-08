# I am a comment, and I want to say that the variable CC will be
# the compiler to use.
CC=gcc
# Hey!, I am comment number 2. I want to say that CFLAGS will be the
# options I'll pass to the compiler.
CFLAGS= -m32 -c -Wall -std=gnu99 -O3
LDFLAGS= -m32 -shared -ldl -lpthread 

# libgregor.so: gregor_error.o gregor.o thread.o closure.o
# 	$(CC) gregor_error.o thread.o gregor.o closure.o -o libgregor.so $(LDFLAGS)

libgregor.a: gregor_error.o gregor.o thread.o closure.o
	ar rcs $@ $^

gregor.o: gregor.c
	$(CC) $(CFLAGS) gregor.c

thread.o: thread.c
	$(CC) $(CFLAGS) thread.c 

gregor_error.o: gregor_error.c
	$(CC) $(CFLAGS) gregor_error.c

closure.o: closure.S
	$(CC) $(CFLAGS) closure.S

clean:
	-rm  *o
