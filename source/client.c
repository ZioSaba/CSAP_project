#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include "commons.h"



void initiate_client_transmission(int socket_desc){
    
    // Client waits until receives "Hello" from server before sending its messages

    // Buffer that will be used to send and receive data
    char buf[1024];
    size_t buf_len = sizeof(buf);
    int msg_len;
    memset(buf, 0, buf_len);

    int ret = 0;        // Generic variable to keep track of syscalls' results

    // Variables to keep track of number of bytes sent/recv
    int recv_bytes = 0;
    int sent_bytes = 0;




    // Display welcome message
    do {
        ret = recv(socket_desc, buf + recv_bytes, buf_len - recv_bytes, 0);
        if (ret == -1 && errno == EINTR) continue;
        else if (ret == -1) HANDLE_ERROR("ERROR! CLIENT CANNOT READ FROM SOCKET - CLIENT TX");
        else if (ret == 0) break;
        recv_bytes += ret;
    } while (buf[recv_bytes-1] != '\n');
    fprintf(stdout, "%s", buf);




    // Main communication loop
    while (1) {

        fprintf(stdout, "Insert your message: ");

        // Read message from stdin
        if (fgets(buf, sizeof(buf), stdin) == NULL) HANDLE_ERROR("ERROR! CLIENT FAILED READING FROM STDIN - CLIENT TX");
        msg_len = strlen(buf);
        
        // Send message to server
        sent_bytes=0;
        while (sent_bytes < msg_len) {
            ret = send(socket_desc, buf + sent_bytes, msg_len - sent_bytes, 0);
            if (ret == -1 && errno == EINTR) continue;
            else if (ret == -1) HANDLE_ERROR("ERROR! CLIENT CANNOT WRITE TO SOCKET - CLIENT TX");
            sent_bytes += ret;
        }

        // Check if 'QUIT\n' command has been sent, in this case initate exit procedure
        if (msg_len == strlen(QUIT_COMMAND) && !memcmp(buf, QUIT_COMMAND, strlen(QUIT_COMMAND))) break;
    }




    // Exit procedure
    ret = shutdown(socket_desc, SHUT_RDWR);
    if (ret == -1) HANDLE_ERROR("ERROR! CLIENT CANNOT SHUTDOWN SOCKET - CLIENT TX");
    ret = close(socket_desc);
    if (ret == -1) HANDLE_ERROR("ERROR! CLIENT CANNOT CLOSE SOCKET - CLIENT TX");

    exit(0);
}




void main(int argc, char* argv[]){

    // Defining variables
    int socket_desc;                        // Socket descriptor for the client
    struct sockaddr_in server_addr = {0};   // Struct for server client parameters + initialize all fields to zero

    bool isDefault;                         // Use to verify whether the executable has been invoked as default or customized
    int ret;                                // Used to check result of CONNECT operation
    



    // Checking command line parameters
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


    // Creating the socket that will be used to connect
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
    if (ret == -1) HANDLE_ERROR("ERROR DURING CONNECTION");


    fprintf(stdout, "Connection established, waiting to be accepted...\n\n");

    // Inoke looping function
    initiate_client_transmission(socket_desc);

    // The program should never this line
}