#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>

#include "commons.h"



void initiate_server_transmission(int socket_desc, int logfile_fd){
    
    int ret = 0;

    struct sockaddr_in client_addr = {0};           // Struct for client socket parameters + initialize all fields to zero
    int sockaddr_len = sizeof(struct sockaddr_in);  // Needed for the accept

    // Main loop to manage incoming connections
    while(1){
        
        int client_desc = accept(socket_desc, (struct sockaddr* ) &client_addr, (socklen_t* ) &sockaddr_len);
        if (client_desc == -1 && errno == EINTR) continue;  // If interrupted by a signal, continue execution
        else if (client_desc < 0) HANDLE_ERROR("ERROR! CANNOT OPEN SOCKET FOR INCOMING CONNECTION - SERVER_TRANSMISSION");

        fprintf(stdout, "Incoming connection received, spawning worker...\n");


        pid_t pid = fork();
        if (pid < 0) HANDLE_ERROR("ERROR! SERVER CANNOT FORK - SERVER TX");
        
        else if (pid == 0) {
            // child: close the listening socket and process the request
            ret = shutdown(socket_desc, SHUT_WR);
            if (ret) HANDLE_ERROR("ERROR! CHILD CANNOT SHUTDOWN MAIN SERVER SOCKET - SERVER TX");
            ret = close(socket_desc);
            if (ret) HANDLE_ERROR("ERROR! CHILD CANNOT CLOSE MAIN SERVER SOCKET SOCKET - SERVER TX");
            worker_connection_handler(client_desc, &client_addr);
            fprintf(stdout, "Process creation to handle request has completed.\n");
            _exit(EXIT_SUCCESS);
        } 
        
        else {
            // server: close the incoming socket and continue
            ret = close(client_desc);
            if (ret) HANDLE_ERROR("ERROR! SERVER CANNOT CLOSE CLIENT SOCKET - SERVER TX");
            fprintf(stdout, "Child process successfully created to handle the request...\n");
            // reset fields in client_addr so it can be reused for the next accept()
            memset(&client_addr, 0, sizeof(struct sockaddr_in));
        }
    }
}



void main(int argc, char* argv[]){

    // Defining variables
    int socket_desc;                        // Socket descriptor for the server
    struct sockaddr_in server_addr = {0};   // Struct for server socket parameters + initialize all fields to zero
    struct hostent* host_info;              // Struct needed by gethostbyname

    bool isDefault = false;                 // Use to verify whether the executable has been invoked as default or customized
    
    int isPartialCustom = 0;                // 0 if both port and log location are specified
                                            // 1 if only port number is provided
                                            // 2 if only log location is provided

    int ret;                                // Used to check result of various operations
    

    // Checking command line parameters
    if (argc == 1){
        fprintf(stdout, "Server will run using default configuration...\n");
        isDefault = true;
    }

    else if (argc == 2){
        char* ptr;
        long test = strtol(argv[1], &ptr, 10);      // checks if the entire string inside argv[1] can be converted to a long in base 10
        if (*ptr == '\0') {
            fprintf(stdout, "Server will run using custom port number and default log file location...\n");
            isPartialCustom = 1;
        }
        else{
            fprintf(stdout, "Server will run using custom log file location and default port number...\n");
            isPartialCustom = 2;
        }
    }

    else if (argc == 3){
        fprintf(stdout,"Server will run using full custom configuration...\n");
    }

    else if (argc > 3){
        fprintf(stderr,"Error! Invalid number of parameters!\n");
	    fprintf(stderr,"Default configuration: %s \n",argv[0]);
	    fprintf(stderr,"Custom port configuration: %s PORT \n",argv[0]);
	    fprintf(stderr,"Custom pathname configuration: %s LOG_PATH \n",argv[0]);
	    fprintf(stderr,"Full custom configuration: %s PORT LOG_PATH \n",argv[0]);
	    exit(1);
    }


    // Creating the server socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) HANDLE_ERROR("ERROR! SOCKET CREATION - MAIN");

    host_info = gethostbyname(SERVER_ADDRESS);
    server_addr.sin_addr.s_addr = *((unsigned long* )host_info->h_addr_list[0]);
    server_addr.sin_family = AF_INET;

    if (isDefault || isPartialCustom == 2) server_addr.sin_port = htons(SERVER_PORT);
    else server_addr.sin_port = htons(atoi(argv[1]));


    // Enabling SO_REUSEADDR to quickly restart our server after a crash
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    if (ret) HANDLE_ERROR("ERROR! REUSEADDR OPTION - MAIN");


    // Binding the server socket
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    if (ret) HANDLE_ERROR("ERROR! SOCKET BINDING - MAIN");


    // Server socket starts listening
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    if (ret) HANDLE_ERROR("ERROR! SOCKET LISTENING - MAIN");
    fprintf(stdout, "Socket is now listening on IP address %s on port %d...\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));


    // Variables for opening fd to log file
    int logfile_fd;
    size_t total_length;
    char* path;
    
    // Compute total length of the path, allocate memory for the string and perform concatenation of path + log name
    if (isDefault || isPartialCustom == 1) {
        total_length = strlen(LOG_PATH) + strlen(LOG_NAME)+1;
        path = (char*) calloc(total_length, sizeof(char));
        if (path == NULL) HANDLE_ERROR("ERROR! DEFAULT PATH MALLOC - MAIN");
        snprintf(path, total_length, "%s%s", LOG_PATH, LOG_NAME);
    }
    else{
        total_length = strlen(argv[1]) + strlen(LOG_NAME)+1;
        path = (char*) calloc(total_length, sizeof(char));
        if (path == NULL) HANDLE_ERROR("ERROR! CUSTOM PATH MALLOC - MAIN");
        snprintf(path, total_length, "%s%s", argv[1], LOG_NAME);
    }

    // Opening fd
    logfile_fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (logfile_fd < 0){
        if(errno == EEXIST) {
            fprintf(stderr, "WARNING! FILE %s ALREADY EXISTS, IT WILL BE OVERWRITTEN!\n", path);
            logfile_fd = open(path, O_WRONLY | O_CREAT, 0644);
        }else
            HANDLE_ERROR("ERROR! CANNOT CREATE LOGFILE - MAIN");
    }
    
    fprintf(stdout, "Opened log file in %s\n", path);
    free(path);


    // Server prints log start timestamp
    time_t ltime;
    char date[27];
    char* timestamp = "The server started logging as of ";
    time(&ltime);
    ctime_r(&ltime, date);
    printf("Contenuto di date = %s", date);
    ret = write(logfile_fd, timestamp, strlen(timestamp));
    if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT WRITE LOG SESSION START MESSAGE - MAIN");
    ret = write(logfile_fd, date, strlen(date));
    if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT WRITE LOG SESSION TIMESTAMP - MAIN");
    

    // Invoke looping function
    initiate_server_transmission(socket_desc, logfile_fd);

    // The program should never reach this line
}