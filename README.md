# CSAP January Project


## Objective
The task is to create a multi-process log server, where each client is served by a "worker" process spawned by the main server.
After a connection has been established, the client starts communicating with the assigned worker, which in turn logs said messages on a file created initially by the main server.
Assuming the server received an exit command, it will wait for all workers to terminate before initiating the exit procedure.<br><br>



## Specification

### Server
The server uses the following default configuration, specified in "commons.h":
* IP Address 127.0.0.1
* Port number 10000
* Logs stored in ./logs

The "Port number" and "Logs Location" parameters can be customized by specifying them on the command line.<br><br>

### Client
The client uses the following default configuration, specified in "commons.h":
* IP Address 127.0.0.1
* Port number 10000

The "IP Address" and "Port Number" parameters can be customized by specifying them on the command line.<br><br>




## Compiling the program
I provided a Makefile with the following targets:

```sh
$ make all       #compiles all files and create both client and server executables

$ make server    #compiles only server.c and its executable

$ make client    #compiles only client.c and its executable

$ make clean     #removes the logs in ./logs and all executables
```
<br>



## Running the program
First, you should launch the server:

```sh
$ ./run_server           # server uses default configuration

$ ./run_server 12345     # server uses default logs location and custom port number

$ ./run_server /home/USERNAME/Documents/    # server uses default port number and custom logs location
                                            # pathname must terminate with /, otherwise it may be a file :D
                                            
$ ./run_server 12345 /home/USERNAME/Documents/      # server uses full custom configuration
```
<br>

Then, you can start spawning clients:

```sh
$ ./run_client                      # client uses default configuration
                                            
$ ./run_client x.x.x.x PORT_N       # client uses custom configuration
```
<br>



## Expected behavior
Upon starting, the server checks the number of arguments provided on the command line and sets some configuration variables that will be used to apply the requested parameters.
Then, it creates, binds and starts listening on a socket.
<br>
Assuming the above operations were successfull, the server creates a new logfile named "csap_logserver" (defined in "commons.h") in the specified pathname, opens it using write permissions and logs a debug message specifying the timestamp when the log session started.
<br> <br>
After this initial configuration, the server starts monitoring both the standard input and the socket for incoming connections.
<br> <br>
Assuming the user types something on the standard input, the server will not accept any commands except for the SIGINT signal (Ctrl+C) or the 'QUIT' command, in which case both of them will start the exit procedure.
<br> <br>
Assuming a new connection is received, the server accepts the connection and spawns a new worker that will handle the communication using a fork(), increases the number of active connections and resets the "sockaddr" structure so that it can be reused for a new connection.
<br> <br>
Once spawned, the worker closes the main server's socket and sends a "Hello" message to the client, specifying the connection's parameters and the command to close the connection. Then, it also writes on the log file a debug message specifying the timestamp when the connection started.
<br>
Now the worker enters the main loop, where it will receive the messages sent by the client, check if the "quit connection" command has been received, and store the messages received on the log file along with the ID of the client that sent them.
Assuming the "quit connection" command has been received, the worker will close the socket and write on the log file a debug message specifying when the connection ended.
<br> <br>
When the server receives either a SIGINT or a 'QUIT' command from the user, it will initiate the exit procedure only after all workers created terminated, which is done by collecting the SIGCHLD signal and decrementing the number of active connections.
When the number of connections reaches 0, the server closes its socket and terminates.