CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -pthread -lsqlite3

all: client server

client: chat_client.c cJSON.c chat_head.h log_reg.h
	$(CC) $(CFLAGS) -o client chat_client.c cJSON.c $(LDFLAGS)

server: chat_server.c cJSON.c chat_head.h
	$(CC) $(CFLAGS) -o server chat_server.c cJSON.c $(LDFLAGS)

clean:
	rm -f client server
