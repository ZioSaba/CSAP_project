#ifndef COMMON_H
#define COMMON_H


/* Configuration parameters */
#define SERVER_ADDRESS  "127.0.0.1"
#define SERVER_COMMAND  "QUIT"
#define SERVER_PORT     10000
#define MAX_CONN_QUEUE  5
#define LOG_PATH        "./logs/"
#define LOG_NAME        "csap_logserver"


/* Macros for error handling */
#define HANDLE_ERROR(msg)   do { perror(msg); exit(1); } while(0)


/* Functions declarations */
void handle_error(char* err_msg);
void initiate_server_transmission(int socket_desc, int logfile_fd);
void initiate_client_transmission(int socket_desc);
void worker_connection_handler(int client_desc, struct sockaddr_in* client_addr);

#endif  //COMMON_H