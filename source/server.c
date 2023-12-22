#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "commons.h"



void handle_error(char* err_msg){

}


void main(int argc, char* argv[]){

    for (int i = 0; i < argc; i++){
        printf("Contentuto di argv[%d] = %s", i, argv[i]);
    }

    bool isDefault;

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

    // Defining variables
    int socket_desc;
    struct sockaddr_in server_addr = {0};   // Initialize all fields to zero

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) handle_error("ERROR! Cannot create socket!");

    if (isDefault){
        server_addr.sin_addr.s_addr = htonl(SERVER_ADDRESS);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
    }
    else{
        server_addr.sin_addr.s_addr = htonl(argv[1]);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(argv[2]);
    }


    // Binding socket

    // Start listening



}