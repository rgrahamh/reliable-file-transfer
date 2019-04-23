CC=gcc
CFLAGS=-Wall

test: client.c
	$(CC) $(CFLAGS) -o client client.c -DTESTING

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm ./client
