#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "commons.h"



void initiate_client_transmission(int socket_desc){
    
}



void main(int argc, char* argv[]){

    // Defining variables
    int socket_desc;                        // Socket descriptor for the client
    struct sockaddr_in server_addr = {0};   // Struct for server client parameters + initialize all fields to zero

    bool isDefault;                         // Use to verify whether the executable has been invoked as default or customized
    int ret;                                // Used to check result of CONNECT operation
    

    if (argc == 1){
        fprintf(stdout, "Client will run using default configuration...\n");
        isDefault = true;
    }

    else if (argc == 3){
        fprintf(stdout,"Client will run using custom configuration...\n");
        isDefault = false;
    }

    else if (argc == 2 || argc > 3) {
	    fprintf(stderr,"Error! Invalid number of parameters!\n");
	    fprintf(stderr,"Default connection: %s \n",argv[0]);
	    fprintf(stderr,"Custom connection: %s IP_ADDRESS PORT \n",argv[0]);
	    exit(1);
    }


    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) HANDLE_ERROR("ERROR DURING SOCKET CREATION");

    if (isDefault){
        server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
    }

    else{
        server_addr.sin_addr.s_addr = inet_addr(argv[1]);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(atoi(argv[2]));
    }

    // Establish a connection
    ret = connect(socket_desc, (struct sockaddr* ) &server_addr, sizeof(struct sockaddr_in));
    if (ret) HANDLE_ERROR("ERROR DURING CONNECTION");

    fprintf(stdout, "Connection established, waiting to be accepted...\n");

    initiate_client_transmission(socket_desc);
    // The program should never this line
}