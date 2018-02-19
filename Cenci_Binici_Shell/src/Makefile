LIBS = -pthread

CC = gcc

CFLAGS= -c -Wall

default: all

all:		shellex.o csapp.o
		$(CC) shellex.o -o ../install/bin/lsh

shellex.o:	shellex.c
		$(CC) $(CFLAGS) shellex.c

csapp.o:	csapp.c csapp.h
		$(CC) $(CFLAGS) csapp.c

clean:
		rm -f *.o; rm ../install/bin/lsh; rm ../install/man/lsh

build:
		cp man/lsh ../install/man/lsh