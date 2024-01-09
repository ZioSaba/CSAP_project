#ifndef COMMON_H
#define COMMON_H


/* Configuration parameters */
#define SERVER_ADDRESS  "127.0.0.1"
#define SERVER_PORT     10000
#define MAX_CONN_QUEUE  5

#define LOG_PATH        "./logs/"
#define LOG_NAME        "csaplogserver"
#define MAX_SIZE        1024

#define QUIT_COMMAND    "QUIT\n"


/* Macros for error handling */
#define HANDLE_ERROR(msg)   do { perror(msg); exit(1); } while(0)


/* Functions declarations */
void initiate_server_transmission(int socket_desc, int logfile_fd);
void exit_procedure(int socket_desc, int logfile_fd);
int directory_lookup(char* buf, char* pathname, int total_length);

void initiate_client_transmission(int socket_desc);

void worker_connection_handler(int client_desc, struct sockaddr_in* client_addr, int logfile_fd);


#endif  //COMMON_H