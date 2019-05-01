CC=gcc
CFLAGS=-Wall

client: client.c
	$(CC) $(CFLAGS) -o client client.c

test: client.c
	$(CC) $(CFLAGS) -o client client.c -DTESTING

clean:
	rm ./client
