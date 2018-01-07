CC=gcc
CFLAGS= -c --std=gnu99 -Wall -Wpedantic

all: simpleFS
	$(CC) main.o helper.o simpleFS.o -o simpleFS

simpleFS: main.c simpleFS.c helper.c
	$(CC) $(CFLAGS) main.c simpleFS.c helper.c

clean:
	rm *.o *~ simpleFS
