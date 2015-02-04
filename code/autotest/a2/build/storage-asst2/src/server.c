/**
 * @file
 * @brief This file implements the storage server.
 *
 * The storage server should be named "server" and should take a single
 * command line argument that refers to the configuration file.
 * 
 * The storage server should be able to communicate with the client
 * library functions declared in storage.h and implemented in storage.c.
 */

// comment

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include "utils.h"

#define MAX_LISTENQUEUELEN 20	///< The maximum number of queued connections.
#define LOGGING 2	// logging constant


/**
 * @brief Process a command from the client.
 *
 * @param sock The socket connected to the client.
 * @param cmd The command received from the client.
 * @return Returns 0 on success, -1 otherwise.
 */
//hello
int handle_command(FILE *fp, int sock, char *cmd)
{
	char msg[50];
	sprintf(msg, "Processing command %s\n", cmd);
	logger(fp, msg); 

	// For now, just send back the command to the client.
	sendall(sock, cmd, strlen(cmd));
	sendall(sock, "\n", 1);

	return 0;
}


/**
 * @brief Start the storage server.
 *
 * This is the main entry point for the storage server.  It reads the
 * configuration file, starts listening on a port, and proccesses
 * commands from clients.
 */

int main(int argc, char *argv[])
{
	char msg[50];
	// Create file name
	FILE *file_ptr;

	if(LOGGING == 1){
		file_ptr = stdout;
	}
	else if(LOGGING == 2){		
		time_t timer;
		char file_name[31];
		struct tm *curr_tm;
	
		time(&timer);
		curr_tm = localtime(&timer);
		strftime(file_name, 31, "Server-%Y-%m-%d-%H-%M-%S.log", curr_tm);
		file_ptr = fopen(file_name, "w");
	}


	// Process command line arguments.
	// This program expects exactly one argument: the config file name.
	assert(argc > 0);
	if (argc != 2) {
		printf("Usage %s <config_file>\n", argv[0]);
		if(LOGGING == 1 || LOGGING == 2){
			fclose(file_ptr);
		}
		exit(EXIT_FAILURE);
	}
	char *config_file = argv[1];

	// Read the config file.
	struct config_params params;
	int status = read_config(config_file, &params);
	if (status != 0) {
		printf("Error processing config file.\n");
		if(LOGGING == 1 || LOGGING == 2){
			fclose(file_ptr);
		}
		exit(EXIT_FAILURE);
	}
	

	sprintf(msg, "Server on %s:%d\n", params.server_host, params.server_port);
	logger(file_ptr,msg);


	// Create a socket.
	int listensock = socket(PF_INET, SOCK_STREAM, 0);
	if (listensock < 0) {
		printf("Error creating socket.\n");
		if(LOGGING == 1 || LOGGING == 2){
			fclose(file_ptr);
		}
		exit(EXIT_FAILURE);
	}

	// Allow listening port to be reused if defunct.
	int yes = 1;
	status = setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
	if (status != 0) {
		printf("Error configuring socket.\n");
		if(LOGGING == 1 || LOGGING == 2){
			fclose(file_ptr);
		}
		exit(EXIT_FAILURE);
	}
//more comment
	// Bind it to the listening port.
	struct sockaddr_in listenaddr;
	memset(&listenaddr, 0, sizeof listenaddr);
	listenaddr.sin_family = AF_INET;
	listenaddr.sin_port = htons(params.server_port);
	inet_pton(AF_INET, params.server_host, &(listenaddr.sin_addr)); // bind to local IP address
	status = bind(listensock, (struct sockaddr*) &listenaddr, sizeof listenaddr);
	if (status != 0) {
		printf("Error binding socket.\n");
		if(LOGGING == 1 || LOGGING == 2){
			fclose(file_ptr);
		}
		exit(EXIT_FAILURE);
	}

	// Listen for connections.
	status = listen(listensock, MAX_LISTENQUEUELEN);
	if (status != 0) {
		printf("Error listening on socket.\n");
		if(LOGGING == 1 || LOGGING == 2){
			fclose(file_ptr);
		}
		exit(EXIT_FAILURE);
	}

	// Listen loop.
	int wait_for_connections = 1;
	while (wait_for_connections) {
		// Wait for a connection.
		struct sockaddr_in clientaddr;
		socklen_t clientaddrlen = sizeof clientaddr;
		int clientsock = accept(listensock, (struct sockaddr*)&clientaddr, &clientaddrlen);
		if (clientsock < 0) {
			printf("Error accepting a connection.\n");
			if(LOGGING == 1 || LOGGING == 2){
				fclose(file_ptr);
			}
			exit(EXIT_FAILURE);
		}

		sprintf(msg, "Got a connection from %s:%d\n", inet_ntoa(clientaddr.sin_addr),clientaddr.sin_port);
		logger(file_ptr,msg);

		// Get commands from client.
		int wait_for_commands = 1;
		do {
			// Read a line from the client.
			char cmd[MAX_CMD_LEN];
			int status = recvline(clientsock, cmd, MAX_CMD_LEN);
			if (status != 0) {
				// Either an error occurred or the client closed the connection.
				wait_for_commands = 0;
			} else {
				// Handle the command from the client.
				int status = handle_command(file_ptr, clientsock, cmd);
				if (status != 0)
					wait_for_commands = 0; // Oops.  An error occured.
			}
		} while (wait_for_commands);
		
		// Close the connection with the client.
		close(clientsock);
		sprintf(msg, "Closed connection from %s:%d.\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
		logger(file_ptr, msg);
	}

	// Stop listening for connections.
	close(listensock);

	if(LOGGING == 1 || LOGGING == 2){
			fclose(file_ptr);
	}
	return EXIT_SUCCESS;
}


