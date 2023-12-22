#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "commons.h"



void handle_error(char* err_msg){
    perror(err_msg);
    exit(1);
}


void main(int argc, char* argv[]){

    // Defining variables
    int socket_desc;                        // Socket descriptor for the server
    struct sockaddr_in server_addr = {0};   // Initialize all fields to zero

    bool isDefault;                         // Use to verify whether the executable has been invoked as default or customized
    int ret;                                // Used to check result of BIND and LISTEN operations

    if (argc == 1){
        fprintf(stdout, "Server will run using default configuration...\n");
        isDefault = true;
    }

    else if (argc == 3){
        fprintf(stdout,"Server will run using custom configuration...\n");
        isDefault = false;
    }

    else if (argc == 2 || argc > 3) {
	    fprintf(stderr,"Error! Invalid number of parameters!\n");
	    fprintf(stderr,"Default configuration usage: %s \n",argv[0]);
	    fprintf(stderr,"Custom configuration usage: %s IP_ADDR PORT \n",argv[0]);
	    exit(1);
    }


    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) handle_error("ERROR DURING SOCKET CREATION");

    if (isDefault){
        server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
    }

    else{
        server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(atoi(argv[1]));
    }

    // Binding socket
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    if (ret) handle_error("ERROR DURING SOCKET BINDING");

    // Start listening
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    if (ret) handle_error("ERROR DURING SOCKET LISTENING");
    fprintf(stdout, "Socket is now listening on IP address %s on port %d...\n", inet_ntoa(server_addr.sin_addr), server_addr.sin_port);


}