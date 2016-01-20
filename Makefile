#makefile for project 1 works both in linux and unix system now
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g
LDFLAGS = #-lnsl

# compile client and server
all: client #server

# compile client only
client: client.o  util.o
	$(CC) $(CFLAGS) -o SimpClient client.o  util.o  $(LDFLAGS)

#compile server only
server: server.o util.o
	$(CC) $(CFLAGS) -o server server.o util.o  $(LDFLAGS)

util.o: util.c
	$(CC) $(CFLAGS) -c util.c

clean:
	-rm -f *.o SimpClient server
