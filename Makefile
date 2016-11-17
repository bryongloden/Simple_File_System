CC=gcc
CFLAGS= -c --std=gnu99 -Wall -Wpedantic

all: simpleFS 
	$(CC) simpleFS.o -o simpleFS
	$(CC) recoverFS.o -o recoverFS

simpleFS: simpleFS.c
	$(CC) $(CFLAGS) simpleFS.c recoverFS.c

clean:
	rm -f *.o simpleFS recoverFS recover_file*