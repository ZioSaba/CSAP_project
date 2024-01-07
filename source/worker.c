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



void worker_connection_handler(int client_desc, struct sockaddr_in* client_addr, int logfile_fd){

    // Variables that will be used for network communication
    char network_buf[1024];
    size_t net_buf_len = sizeof(network_buf);
    int msg_len;
    memset(network_buf, 0, net_buf_len);
    int bytes_recv = 0;
    int bytes_sent = 0;


    int ret = 0;                                    // Generic variable to keep track of syscalls' results
    int client_ID = ntohs(client_addr->sin_port);   // Use client's port as unique ID, for simplicity


    // Variables that will be used for logging
    char file_buf[1024];
    size_t file_buf_len = sizeof(file_buf);
    memset(file_buf, 0, file_buf_len);
    int bytes_written = 0;




    // Worker sends "Hello" message
    sprintf(network_buf, "Hi! You are connected to the log server. You can close this connection using %s \n", QUIT_COMMAND);
    msg_len = strlen(network_buf);
	while (bytes_sent < msg_len) {
        ret = send(client_desc, network_buf + bytes_sent, msg_len - bytes_sent, 0);
        if (ret == -1 && errno == EINTR) continue;
        else if (ret == -1) HANDLE_ERROR("ERROR! WORKER CANNOT WRITE TO SOCKET HELLO MESSAGE");
        bytes_sent += ret;
    }
    memset(network_buf, 0, net_buf_len);




    // Write connection handling log message on file
    time_t ltime;
    time(&ltime);
    sprintf(file_buf, "\n\nThe worker %d is now handling client %d connected at %s\n\n", getpid(), client_ID, ctime(&ltime));
    
    bytes_written = 0;
    while (bytes_written < strlen(file_buf)) {
            ret = write(logfile_fd, file_buf + bytes_written, strlen(file_buf) - bytes_written);
            if (ret == -1 && errno == EINTR) continue;
            else if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT PRINT LOG START MESSAGE - MAIN");
            bytes_written += ret;
    }  

    memset(file_buf, 0, file_buf_len);




    // Main recv loop and log
    while (1) {
        
        // Read message received from client
        bytes_recv = 0;
        do {
            ret = recv(client_desc, network_buf + bytes_recv, net_buf_len - bytes_recv, 0);
            if (ret == -1 && errno == EINTR) continue;
            else if (ret == -1) HANDLE_ERROR("ERROR! WORKER CANNOT READ FROM SOCKET");
            else if (ret == 0) break;                   // Check if client closed socket unexpectedly
            bytes_recv += ret;
		} while (network_buf[bytes_recv-1] != '\n');

        // Check if client closed socket unexpectedly
        if (bytes_recv == 0) break;

        // Check if client sent 'QUIT\n' command
        if (bytes_recv == strlen(QUIT_COMMAND) && !memcmp(network_buf, QUIT_COMMAND, strlen(QUIT_COMMAND))) break;

        
        // Perform log on file
        msg_len = strlen(network_buf);
        sprintf(file_buf, "Client %d said: ", client_ID);
        strncat(file_buf, network_buf, msg_len);
        msg_len = strlen(file_buf);
        bytes_written = 0;
	    while (bytes_written < msg_len) {
            ret = write(logfile_fd, file_buf + bytes_written, msg_len - bytes_written);
            if (ret == -1 && errno == EINTR) continue;
            else if (ret == -1) HANDLE_ERROR("ERROR! WORKER CANNOT WRITE TO LOGFILE");
            bytes_written += ret;
        }

        memset(file_buf, 0, file_buf_len);
        memset(network_buf, 0, net_buf_len);
    }


    // Log connection closed
    time(&ltime);
    sprintf(file_buf, "\n\nThe worker %d closed connection with client %d at %s\n\n", getpid(), client_ID, ctime(&ltime));
    
    bytes_written = 0;
    while (bytes_written < strlen(file_buf)) {
            ret = write(logfile_fd, file_buf + bytes_written, strlen(file_buf) - bytes_written);
            if (ret == -1 && errno == EINTR) continue;
            else if (ret == -1) HANDLE_ERROR("ERROR! SERVER CANNOT PRINT LOG START MESSAGE - MAIN");
            bytes_written += ret;
    }  

    memset(file_buf, 0, file_buf_len);




    // Exit procedure

    // Closing file descriptors
    ret = close(client_desc);
    if (ret == -1) HANDLE_ERROR("ERROR! WORKER CANNOT CLOSE SOCKET");

    // Close file desc
    ret = close(logfile_fd);
    if (ret == -1) HANDLE_ERROR("ERROR! WORKER CANNOT CLOSE FILE");
}