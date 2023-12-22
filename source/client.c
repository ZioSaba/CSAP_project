#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "commons.h"


void main(int argc, char* argv[]){

    bool isDefault;

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
	    fprintf(stderr,"Default configuration usage: %s \n",argv[0]);
	    fprintf(stderr,"Custom configuration usage: %s IP_ADDR PORT \n",argv[0]);
	    exit(1);
    }

    // Defining variables
    int socket_desc;
    struct sockaddr_in server_addr;

    // Initialize


    if (isDefault){

    }

    else{

    }

}