CC = gcc
CFLAGS = -g


all: test

malloc.o: malloc.c malloc.h
	$(CC) $(CFLAGS) -c malloc.c 

testing_suite.o: testing_suite.c malloc.h
	$(CC) $(CFLAGS) -c testing_suite.c

test: malloc.o testing_suite.o
	$(CC) $(CFLAGS) -o test malloc.o testing_suite.o

clean:
	rm -f *.o test
