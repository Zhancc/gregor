# I am a comment, and I want to say that the variable CC will be
# the compiler to use.
CC=gcc
# Hey!, I am comment number 2. I want to say that CFLAGS will be the
# options I'll pass to the compiler.
CFLAGS=-m32 -c -Wall -std=c99 
LDFLAGS=-m32 -shared -pthread

gregor.so: gregor_error.o gregor.o thread.o
	$(CC) gregor_error.o thread.o gregor.o -o gregor $(LDFLAGS)

gregor.o: gregor.c
	$(CC) $(CFLAGS) gregor.c

thread.o: thread.c
	$(CC) $(CFLAGS) thread.c $(LDFLAGS)

gregor_error.o: gregor_error.c
	$(CC) $(CFLAGS) gregor_error.c

clean:
	-rm *o gregor
