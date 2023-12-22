#include <errno.h>

/* Configuration parameters */
#define SERVER_ADDRESS  "127.0.0.1"
#define SERVER_COMMAND  "QUIT"
#define SERVER_PORT     10000
#define MAX_CONN_QUEUE  5
#define LOG_LOCATION    "./logs/"


/* Functions declarations */
void handle_error(char* err_msg);