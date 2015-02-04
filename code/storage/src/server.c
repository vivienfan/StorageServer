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
#include <errno.h>
#include "utils.h"

#define MAX_LISTENQUEUELEN 20	///< The maximum number of queued connections.
#define LOGGING 2	


/**
 * @brief Process a command from the client.
 *
 * @param sock The socket connected to the client.
 * @param cmd The command received from the client.
 * @return Returns 0 on success, -1 otherwise.
 */

int handle_command(FILE *fp, int sock, char *cmd, struct config_params params, base_t *Bhead)
{
	char msg[50];
	sprintf(msg, "Processing command %s\n", cmd);
	logger(fp, msg); 

	// use strtok to separate tokens of the cmd
        char *command = strtok(cmd, " ");
	printf("server: %s", command);
        if(command)
        {
                if(!strcmp(command, "AUTH")){ 
                        // check if username and password match
                        char *username = strtok(NULL, " ");
                        char *password = strtok(NULL, " ");

                        if(strcmp(username, params.username)==0 && strcmp(password, params.password)==0){
                                // username and password are correct.
				sprintf(msg, "Username: %s and Password:%s\n",username, password );
				logger(fp, msg); 
				sendall(sock,"_0\n",3);		 
                        }                        
                        else{
				sendall(sock,"_1\n",4);
                                errno = ERR_AUTHENTICATION_FAILED;
                        }
                }
                else if(!strcmp(command, "GET")){
                        // get the value for the key in the table
                        char *table = strtok(NULL, " ");
			printf("server: %s\n", table);
                        char *key = strtok(NULL, " ");
			printf("server: %s\n", key); 

			table_t * the_table = findTable(table, Bhead);

			if(!the_table){
				// if search() -> table not found, errno = ERR_TABLE_NOT_FOUND, sendall(sock,"_2\n",3);
				sendall(sock,"_2\n",3);
			}
			else{
				// table found, now check if key exists
				list_t *tmp = retrieve(key, the_table);
				if(!tmp){
					// if search() -> key not found, errno = ERR_KEY_NOT_FOUND, sendall(sock,"_3\n",4);
					sendall(sock,"_3\n",3);
				}
				else{
					// key found, entry exists
					char *value = tmp->record;
					sendall(sock,value,sizeof value);
					sendall(sock,"\n",1);
				}
			}
					
                }
                else if(!strcmp(command, "SET")){
                        char *table = strtok(NULL, " ");
			char *key = strtok(NULL, " ");
			char *value = strtok(NULL, "\0");

			table_t * the_table = findTable(table, Bhead);

			if(!the_table){
				// if search() -> table not found, errno = ERR_TABLE_NOT_FOUND, sendall(sock,"_2\n",3);
				sendall(sock,"_2\n",3);
			}
			else{
				// table found, now process
				if(!value){
					// if value is NULL, client wants to delete

					// -> call retrieve check if key exist
					list_t *tmp = retrieve(key,the_table);
					if(!tmp){
						// if search() -> key not found, errno = ERR_KEY_NOT_FOUND, sendall(sock,"_3\n",4);
						sendall(sock,"_3\n",3);
					}
					else{
						// key exists, now delete
						if(deleteEntry(key, the_table)){
							// delete successed
							sendall(sock,"_0\n",3);
						}
						else{
							// unknown error happened
							sendall(sock,"_1\n",3);
						}
					}
				}
				else{
					// else, insert or update the data
					if(insertOrUpdate(key, value, the_table)){
						// insert or update successsed
						sendall(sock,"_0\n",3);
					}
					else{
						// cannnot update or insert, unknown error happened
						sendall(sock,"_1\n",3);
					}	
				}			
			}
                }
		//else, command does not match AUTH, GET, or SET? Should not happen
		sendall(sock,"_1\n",4);	                            
        }
	else{
		//else, command does not exist? Should not happen
		sendall(sock,"_1\n",4);
		errno = ERR_UNKNOWN;
	}
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
	else{
		file_ptr = NULL;
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
	params.server_host[0]='\0';
	params.server_port=0;
	params.username[0]='\0';
	params.password[0]='\0';
	int j;
	for(j=0;j<MAX_TABLES;j++){
		params.table[j][0]='\0';
	}

	int status = read_config(config_file, &params);
	if (status != 0) {
		printf("Error processing config file.\n");
		if(LOGGING == 1 || LOGGING == 2){
			fclose(file_ptr);
		}
		exit(EXIT_FAILURE);
	}
	
	int data_base_size = 0;
	int index_k;
	for(index_k = 0; params.table[index_k]; index_k++){
		data_base_size ++;
	} 
	
	base_t *base_head = create_data_base(&params, data_base_size);

	sprintf(msg, "connected to server\n");
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
	sprintf(msg, "Socket created");
	logger(file_ptr,msg);

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
	sprintf(msg, "Listening to Port\n");
	logger(file_ptr,msg);


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
	sprintf(msg, "Listen for connections\n");
	logger(file_ptr,msg);


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
			} 
			else {
				// Handle the command from the client.
				sprintf(msg, "Handle command\n");
				logger(file_ptr,msg);
				int status = handle_command(file_ptr, clientsock, cmd, params, base_head);
				printf("went through\n");
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


