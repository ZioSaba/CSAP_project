#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include "commons.h"



void initiate_server_transmission(int socket_desc, char* location){
    
    int ret = 0;

    struct sockaddr_in client_addr = {0};           // Struct for client socket parameters + initialize all fields to zero
    int sockaddr_len = sizeof(struct sockaddr_in);  // Needed for the accept

    // Main loop to manage incoming connections
    while(1){
        
        int client_desc = accept(socket_desc, (struct sockaddr* ) &client_addr, (socklen_t* ) &sockaddr_len);

        if (client_desc == -1 && errno == EINTR) continue;  // If interrupted by a signal, continue execution
        else if (client_desc < 0) HANDLE_ERROR("ERROR! CANNOT OPEN SOCKET FOR INCOMING CONNECTION");

        fprintf(stdout, "Incoming connection received, spawning worker...\n");
    }
}



void main(int argc, char* argv[]){

    // Defining variables
    int socket_desc;                        // Socket descriptor for the server
    struct sockaddr_in server_addr = {0};   // Struct for server socket parameters + initialize all fields to zero

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
	    fprintf(stderr,"Custom configuration usage: %s PORT LOG_PATH \n",argv[0]);
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
        server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(atoi(argv[1]));
    }

    // Binding socket
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    if (ret) HANDLE_ERROR("ERROR DURING SOCKET BINDING");

    // Socket starts listening
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    if (ret) HANDLE_ERROR("ERROR DURING SOCKET LISTENING");
    fprintf(stdout, "Socket is now listening on IP address %s on port %d...\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    // Invoke looping function
    if (isDefault) initiate_server_transmission(socket_desc, LOG_LOCATION);
    else initiate_server_transmission(socket_desc, argv[2]);

    // The program should never reach here
}