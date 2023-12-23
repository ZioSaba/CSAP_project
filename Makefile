CC = gcc -g
SERVER_CODE = ./source/server.c
CLIENT_CODE = ./source/client.c
COMMONS = ./source/commons.h


all: server client


server: $(SERVER_CODE) $(COMMONS)
	$(CC) -o run_server $(SERVER_CODE)


client: $(CLIENT_CODE) $(COMMONS)
	$(CC) -o run_client $(CLIENT_CODE)


clean:
	rm -f run_server run_client ./logs/*