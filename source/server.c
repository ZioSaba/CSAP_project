#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <fcntl.h>

#include "commons.h"
#include "worker.c"


/* Global variables */
bool exit_signal_received = false;          // used to initiate exit procedure
int num_childs = 0;                         // keeps track of childs generated before exiting




// Declaration of worker's main function
void worker_connection_handler(int client_desc, struct sockaddr_in* client_addr, int logfile_fd);

  

void SIGINT_handler(){
    exit_signal_received = true;
}

// In the event that multiple childrens terminates simultaneously, only 1 SIGCHLD signal will be handled, so I need to catch'em all :D
// BEWARE: When waiting for any child, the waitpid might return 0 or (-1 && "No child processes") if there are no more childrens to collect
void SIGCHLD_handler(){
    while (1) {
        pid_t pid = waitpid(-1, NULL, WNOHANG);
        if (pid == 0 || (pid == -1 && errno == ECHILD)) break;
        else if (pid == -1 && errno != ECHILD) HANDLE_ERROR("ERROR! SIGCHLD WAITPID");
        else num_childs--;
    }
}




void exit_procedure(int socket_desc, int logfile_fd){
    
    fprintf(stdout, "Initiating exit procedure...\n");

    int ret;
    
    /* I decided to restore only the SIGCHLD handler to its default behavior because now I want to ensure synchronicity
    ** and have a defined program flow to ensure that all the remaining childs are collected.
    ** The SIGINT handler is still installed but only so that even if the programmer presses Ctrl+C by accident or to close the
    ** server quickly, the signal does not terminate the process immediately and the server can exit gracefully. 
    */
    struct sigaction SIGCHLD_act = {0} ;
    SIGCHLD_act.sa_handler = SIG_DFL;
    ret = sigaction(SIGCHLD, &SIGCHLD_act, 0);
    if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT HANDLE SIGCHDL SIGNAL - SERVER EXIT"); 


    while (num_childs > 0) {
        fprintf(stdout, "Waiting for %d childs \n", num_childs);
        ret = wait(NULL);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT PERFORM WAIT - SERVER EXIT");
        num_childs--;
    }
    

    // Server prints log end timestamp
    time_t ltime;
    char* timestamp = calloc(100, sizeof(char));
    if (timestamp == NULL) HANDLE_ERROR("ERROR! TIMESTAMP CALLOC - MAIN");
    time(&ltime);
    sprintf(timestamp, "The server stopped logging as of %s", ctime(&ltime));
    
    int written_bytes = 0; // index for reading from the buffer
    while (written_bytes < strlen(timestamp)) {
            ret = write(logfile_fd, timestamp + written_bytes, strlen(timestamp) - written_bytes);
            if (ret == -1 && errno == EINTR) continue;
            else if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT PRINT LOG END MESSAGE - SERVER EXIT");
            written_bytes += ret;
    }

    free(timestamp); 


    // Close socket
    ret = shutdown(socket_desc, SHUT_RDWR);
    if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT SHUTDOWN SOCKET - SERVER EXIT");
    ret = close(socket_desc);
    if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT CLOSE SOCKET - SERVER EXIT");


    // Close fd
    ret = close(logfile_fd);
    if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT CLOSE FILE -- SERVER EXIT");

    exit(0);
}




void initiate_server_transmission(int socket_desc, int logfile_fd){
    
    int ret;                                            // Used to check result of various operations

    struct sockaddr_in client_addr = {0};               // Struct for client socket parameters + initialize all fields to zero
    int sockaddr_len = sizeof(struct sockaddr_in);      // Needed for the accept



    
    // Installing signal handler for SIGINT so that user can press Ctrl+C on terminal to close server
    struct sigaction SIGINT_act = {0} ;
    SIGINT_act.sa_handler = SIGINT_handler;
    ret = sigaction(SIGINT, &SIGINT_act, 0);
    if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT INSTALL SIGINT HANDLER - SERVER TX");

    
    // Installing signal handler for SIGCHLD for asynchronous collection of workers
    struct sigaction SIGCHLD_act = {0} ;
    SIGCHLD_act.sa_handler = SIGCHLD_handler;
    ret = sigaction(SIGCHLD, &SIGCHLD_act, 0);
    if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT INSTALL SIGCHLD HANDLER - SERVER TX");     
    



    fprintf(stdout, "\nSERVER: Use Ctrl+C or type 'QUIT' to close the server...\n");




    // Initialize set of fd to read during polling using pselect()
    fd_set read_set;
    FD_ZERO(&read_set);

    // Create signal mask for pselect() to catch SIGCHLD and SIGINT
    sigset_t sigset, oldset;
    if (sigemptyset(&sigset) == -1) HANDLE_ERROR("ERROR! CANNOT CREATE EMTPY SIGNAL MASK - SERVER TX");
    if (sigaddset(&sigset, SIGINT) == -1) HANDLE_ERROR("ERROR! CANNOT ADD SIGINT TO SIGNAL MASK - SERVER TX");
    if (sigaddset(&sigset, SIGCHLD) == -1) HANDLE_ERROR("ERROR! CANNOT ADD SIGCHLD TO SIGNAL MASK - SERVER TX");
    if (sigprocmask(SIG_BLOCK, &sigset, &oldset) == -1) HANDLE_ERROR("ERROR! CANNOT SET NEW SIGNAL MASK - SERVER TX");




    // Main loop to manage incoming connections
    while(1){

        //Reset fd
        FD_ZERO(&read_set);
        FD_SET(STDIN_FILENO, &read_set);
        FD_SET(socket_desc, &read_set);


        /*  The pslect() works using the following idea.
        **  To ensure that both SIGINT and SIGCHLD are received only when the pselect() function is sleeping, and also to avoid not handling them 
        **  due to resource starvation (I think), I decided to block said signals and use the 'oldset' mask (default one with all signals allowed)
        **  to ensure that the process can handle said signals only while the pselect is blocking the execution.
        */
        ret = pselect(FD_SETSIZE, &read_set, NULL, NULL, NULL, &oldset);


        // If Interrupted should be only by SIGINT or SIGCHLD (at least unless something unexpected happens)
        if (ret < 0 && errno == EINTR) {
            if (exit_signal_received) {
                fprintf(stdout, "\nReceived SIGINT...\n\n");
                if (sigprocmask(SIG_UNBLOCK, &sigset, &oldset) == -1) HANDLE_ERROR("ERROR! CANNOT RESTORE OLD SIGNAL MASK - SERVER TX");
                exit_procedure(socket_desc, logfile_fd);
            }
            else continue;
        }
        else if (ret < 0) HANDLE_ERROR("ERROR! SELECT NOT WORKING - SERVER TX");


        // Check if user wrote something on the terminal
        if (FD_ISSET(STDIN_FILENO, &read_set)){

            // Read input from terminal and compare with 'QUIT\n'
            char buf[100];
            if (fgets(buf, sizeof(buf), stdin) == NULL) HANDLE_ERROR("ERROR! SERVER FAILED READING FROM STDIN - SERVER TX");
            if (strlen(buf) == strlen(QUIT_COMMAND) && !memcmp(buf, QUIT_COMMAND, strlen(QUIT_COMMAND))){
                exit_signal_received = true;
                fprintf(stdout, "Received 'QUIT' command...\n\n");
                exit_procedure(socket_desc, logfile_fd);
            }
            else fprintf(stdout, "Command not recognized, ignoring...\n");
        }


        // Check if server received new connection request
        else if (FD_ISSET(socket_desc, &read_set)){

            // Accept incoming request
            int client_desc = accept(socket_desc, (struct sockaddr* ) &client_addr, (socklen_t* ) &sockaddr_len);
            if (client_desc == -1 && errno == EINTR) continue;  // If interrupted by a signal, continue execution
            else if (client_desc < 0) HANDLE_ERROR("ERROR! CANNOT OPEN SOCKET FOR INCOMING CONNECTION - SERVER_TRANSMISSION");


            fprintf(stdout, "\nSERVER: Incoming connection received, spawning worker...\n");


            // Spawn worker
            pid_t pid = fork();
            if (pid < 0) HANDLE_ERROR("ERROR! SERVER CANNOT FORK - SERVER TX");


            // Child closes the listening socket and processes the request
            else if (pid == 0) {                
                ret = shutdown(socket_desc, SHUT_WR);
                if (ret == -1) HANDLE_ERROR("ERROR! CHILD CANNOT SHUTDOWN MAIN SERVER SOCKET - SERVER TX");
                ret = close(socket_desc);
                if (ret == -1) HANDLE_ERROR("ERROR! CHILD CANNOT CLOSE MAIN SERVER SOCKET SOCKET - SERVER TX");
                
                // Needed to avoid that the child also receive the SIGINT sent to the parent due to being in the same process group!
                struct sigaction SIGINT_act = {0};
                SIGINT_act.sa_handler = SIG_IGN;
                ret = sigaction(SIGINT, &SIGINT_act, 0);
                if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT HANDLE SIGINT SIGNAL - SERVER TX");

                worker_connection_handler(client_desc, &client_addr, logfile_fd);

                fprintf(stdout, "\nCHILD %d: Successfully handled a connection, terminating...\n", getpid());
                exit(0);
            } 


            // Server closes client socket and continues accepting new requests
            else {
                ret = close(client_desc);
                if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT CLOSE CLIENT SOCKET - SERVER TX");
                fprintf(stdout, "\nSERVER: Child process %d successfully created to handle the request...\n", pid);
                num_childs++;

                // Reset fields in client_addr so it can be reused for the next accept()
                memset(&client_addr, 0, sizeof(struct sockaddr_in));
            }
        }
    }
}




void main(int argc, char* argv[]){

    // Defining variables
    int socket_desc;                        // Socket descriptor for the server
    struct sockaddr_in server_addr = {0};   // Struct for server socket parameters + initialize all fields to zero
    struct hostent* host_info;              // Struct needed by gethostbyname

    bool isDefault = false;                 // Use to verify whether the executable has been invoked as default or customized
    
    int isCustom = 0;                       // 0 if both port and log location are specified
                                            // 1 if only port number is provided
                                            // 2 if only log location is provided

    int ret;                                // Used to check result of various operations
    



    // Checking command line parameters

    // No additional parameters -> no optional information provided
    if (argc == 1){
        fprintf(stdout, "Server will run using default configuration...\n");
        isDefault = true;
    }

    // 1 additional parameters -> check if PORT or PATHNAME have been provided
    else if (argc == 2){
        char* ptr;
        long test = strtol(argv[1], &ptr, 10);      // checks if entire string inside argv[1] can be converted to a 'long int' base 10
        if (*ptr == '\0') {
            fprintf(stdout, "Server will run using custom port number and default log file location...\n");
            isCustom = 1;
        }
        else{                                       // test failed implies it contains characters and will be interpreted as a pathname
            fprintf(stdout, "Server will run using custom log file location and default port number...\n");
            isCustom = 2;
        }
    }

    // 2 additional parameters -> both PORT and PATHNAME have been specified
    else if (argc == 3){
        fprintf(stdout,"Server will run using full custom configuration...\n");
    }

    // More than 2 additional parameters -> too much information!
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
    if (socket_desc == -1) HANDLE_ERROR("ERROR! SOCKET CREATION - MAIN");


    // Retrieving socket's address
    host_info = gethostbyname(SERVER_ADDRESS);
    if (host_info == NULL) HANDLE_ERROR("ERROR! GETHOSTBYNAME - MAIN");
    server_addr.sin_addr.s_addr = *((unsigned long* )host_info->h_addr_list[0]);
    server_addr.sin_family = AF_INET;


    // Choosing most appropriate PORT number
    if (isDefault || isCustom == 2) server_addr.sin_port = htons(SERVER_PORT);
    else server_addr.sin_port = htons(atoi(argv[1]));


    // Enabling SO_REUSEADDR to quickly restart our server in the event of a crash
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    if (ret == -1) HANDLE_ERROR("ERROR! REUSEADDR OPTION - MAIN");


    // Binding the server socket
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    if (ret == -1) HANDLE_ERROR("ERROR! SOCKET BINDING - MAIN");


    // Server socket starts listening
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    if (ret == -1) HANDLE_ERROR("ERROR! SOCKET LISTENING - MAIN");
    fprintf(stdout, "Socket is now listening on IP address %s on port %d...\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));




    // Variables for opening fd to log file
    int logfile_fd;
    size_t total_length;
    char* path;


    // Compute total length of the path, allocate memory for the string and perform concatenation of "path/filename"
    if (isDefault || isCustom == 1) {
        total_length = strlen(LOG_PATH) + strlen(LOG_NAME)+1;                       // Add +1 to total length for "/0"
        path = (char*) calloc(total_length, sizeof(char));
        if (path == NULL) HANDLE_ERROR("ERROR! DEFAULT PATH CALLOC - MAIN");
        snprintf(path, total_length, "%s%s", LOG_PATH, LOG_NAME);
    }
    else if (isCustom == 2) {
        total_length = strlen(argv[1]) + strlen(LOG_NAME)+1;                       // Add +1 to total length for "/0"
        path = (char*) calloc(total_length, sizeof(char));
        if (path == NULL) HANDLE_ERROR("ERROR! CUSTOM PATH CALLOC - MAIN");
        snprintf(path, total_length, "%s%s", argv[1], LOG_NAME);
    }
    else{
        total_length = strlen(argv[2]) + strlen(LOG_NAME)+1;                       // Add +1 to total length for "/0"
        path = (char*) calloc(total_length, sizeof(char));
        if (path == NULL) HANDLE_ERROR("ERROR! CUSTOM PATH CALLOC - MAIN");
        snprintf(path, total_length, "%s%s", argv[2], LOG_NAME);
    }

    printf("\n\nLa path totale è %s\n\n\n", path);


    // Opening fd in append to guarantee atomicity!!
    logfile_fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_APPEND, 0644);
    if (logfile_fd < 0){
        if(errno == EEXIST) {
            fprintf(stderr, "WARNING! FILE %s ALREADY EXISTS, IT WILL BE OVERWRITTEN!\n", path);
            logfile_fd = open(path, O_WRONLY | O_TRUNC | O_APPEND, 0644);
        }else
            HANDLE_ERROR("ERROR! CANNOT CREATE LOGFILE - MAIN");
    }
    
    fprintf(stdout, "Created log file in %s\n", path);
    free(path);




    // Server prints log start timestamp
    time_t ltime;
    char timestamp[100];
    time(&ltime);
    sprintf(timestamp, "The server started logging as of %s", ctime(&ltime));
    
    int written_bytes = 0;
    while (written_bytes < strlen(timestamp)) {
            ret = write(logfile_fd, timestamp + written_bytes, strlen(timestamp) - written_bytes);
            if (ret == -1 && errno == EINTR) continue;
            else if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT PRINT LOG START MESSAGE - MAIN");
            written_bytes += ret;
    }  




    // Invoke looping function
    initiate_server_transmission(socket_desc, logfile_fd);

    // The program should never reach this line
}