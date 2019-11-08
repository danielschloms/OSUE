CC = gcc
CFLAGS = -Wall -g -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

all: generator supervisor

generator: generator.o
	$(CC) -o generator generator.o shm.h -lrt -pthread

supervisor: supervisor.o
	$(CC) -o supervisor supervisor.o shm.h -lrt -pthread

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o generator supervisor
