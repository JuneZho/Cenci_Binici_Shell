CC = gcc

CFLAGS= -c -Wall

default: all

all:		shellex.c csapp.o
		$(CC) $(CFLAGS) shellex.c csapp.o -o lsh

csapp.o:	csapp.c csapp.h
		$(CC) $(CFLAGS) csapp.c
	
clean:
	rm -f *.o; rm lsh
	