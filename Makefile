CC = gcc -g


all: server client worker


server: server.c commons.h
	$(CC) -o run_server server.c


client: client.c commons.h
	$(CC) -o run_client client.c 


clean:
	rm -f run_server run_client