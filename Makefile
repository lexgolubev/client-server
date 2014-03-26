CC := $(CROSS_COMPILE)gcc
make:
	$(CC) client.c -o client
	$(CC) server.c -o server
clean:
	rm -f client server

