# I am a comment, and I want to say that the variable CC will be
# the compiler to use.
CC=g++
# Hey!, I am comment number 2. I want to say that CFLAGS will be the
# options I'll pass to the compiler.
CFLAGS=-c -Wall

all: gregor

gregor: thread.o init.o gregor.o
	$(CC) thread.o init.o gregor.o -o gregor

gregor.o: gregor.c
	$(CC) $(CFLAGS) gregor.cpp

init.o: init.c
	$(CC) $(CFLAGS) factorial.cpp

thread.o: thread.c
	$(CC) $(CFLAGS) thread.c

clean:
	rm *o gregor
