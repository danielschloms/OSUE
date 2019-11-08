CC = gcc
CFLAGS = -Wall -g -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
OBJ = client.o server.o findpath.o getclientsocket.o checkresponse.o

all: client server

client: client.o findpath.o getclientsocket.o checkresponse.o clientheader.h
	$(CC) -o client client.o findpath.o getclientsocket.o checkresponse.o clientheader.h

server: server.o getserversocket.o checkrequest.o serverheader.h
	$(CC) -o server server.o getserversocket.o checkrequest.o serverheader.h

%.o: %.c clientheader.h serverheader.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o client server
