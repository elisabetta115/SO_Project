CC = gcc
CFLAGS = -std=gnu99 -g -m32 -Ofast -Wall -Wextra

all: test

malloc.o: malloc.c malloc.h
	$(CC) $(CFLAGS) -c malloc.c 

testing_suite.o: testing_suite.c malloc.h stack.h
	$(CC) $(CFLAGS) -c testing_suite.c

test.o: stack.c stack.h malloc.h
	$(CC) $(CFLAGS) -c test.c

test: malloc.o testing_suite.o stack.o
	$(CC) $(CFLAGS) -o test malloc.o testing_suite.o stack.o

clean:
	rm -f *.o test