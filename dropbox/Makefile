CC = gcc
CFLAGS = -pthread -Wall

SRC = src/server.c src/client_handler.c src/worker.c src/queue.c src/metadata.c
OBJ = $(SRC:.c=.o)

server: $(OBJ)
	$(CC) $(CFLAGS) -o server $(OBJ)

clean:
	rm -f src/*.o server

client: test_client.c
	gcc -o test_client test_client.c

