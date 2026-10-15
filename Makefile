CC = gcc
CFLAGS = -Wall -Wextra -g

all: memgrind memcheck

memgrind: memgrind.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^

memcheck: memcheck.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^

memgrind.o: memgrind.c mymalloc.h
	$(CC) $(CFLAGS) -c memgrind.c

memcheck.o: memcheck.c mymalloc.h
	$(CC) $(CFLAGS) -c memcheck.c

mymalloc.o: mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -c mymalloc.c

clean:
	rm -f memgrind memcheck *.o
