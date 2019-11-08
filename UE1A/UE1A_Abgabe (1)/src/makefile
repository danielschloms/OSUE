CC = gcc
CFLAGS = -Wall -g -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

all: mydiff

mydiff: mydiff.o cmpline.o
	$(CC) -o mydiff mydiff.o cmpline.o

mydiff.o: mydiff.c util.h
	$(CC) $(CFLAGS) $<

cmpline.o: cmpline.c util.h
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o mydiff
