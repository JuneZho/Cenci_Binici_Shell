CC= g++

CFLAGS= -c -Wall

all:		shellex.o csapp.o
		$(CC) $(CFLAGS) shellex.o csapp.o -o output

shellex.o:	shellex.c
		$(CC) $(CFLAGS) shellex.c

csapp.o:	csapp.c csapp.h
		$(CC) $(CFLAGS) csapp.c
	
clean:
	rm *.o all
	