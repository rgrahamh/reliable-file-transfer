CC=gcc
CFLAGS=-Wall

test: server.c
	$(CC) $(CFLAGS) -o server server.c -DTESTING

server: server.c
	$(CC) $(CFLAGS) -o server server.c

clean:
	rm ./server
