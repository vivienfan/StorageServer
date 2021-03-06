/**
 * @file
 * @brief This file implements a "very" simple sample client.
 * 
 * The client connects to the server, running at SERVERHOST:SERVERPORT
 * and performs a number of storage_* operations. If there are errors,
 * the client exists.
 */


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "storage.h"

#define LOGGING 2
FILE *file_ptr_client;

/**
 * @brief Start a client to interact with the storage server.
 *
 * If connect is successful, the client performs a storage_set/get() on
 * TABLE and KEY and outputs the results on stdout. Finally, it exists
 * after disconnecting from the server.
 */
int main(int argc, char *argv[]) {
	int sPort=0, connected=0, status=0, loggedin=0;
	char selec[100], sHost[100], sUsername[100], sPassword[100],
		dTable[100], dKey[100], dValue[100];
	void *conn;
	struct storage_record r;

	// Create file name
	if(LOGGING == 1){
		file_ptr_client = stdout;
	}
	else if(LOGGING == 2){		
		time_t timer;
		char file_name[31];
		struct tm *curr_tm;
	
		time(&timer);
		curr_tm = localtime(&timer);
		strftime(file_name, 31, "Client-%Y-%m-%d-%H-%M-%S.log", curr_tm);
		file_ptr_client = fopen(file_name, "w");
	}

	printf("> ---------------------\n"
				"> 1) Connect\n"
				"> 2) Authenticate\n"
				"> 3) Get\n"
				"> 4) Set\n"
				"> 5) Disconnect\n"
				"> 6) Exit\n"
				"> ---------------------\n");
	while(true){
		printf("> Please enter your selection: ");
		scanf("%s", selec);

		// Connect to server
		if(strcmp(selec,"1")==0){
			printf("> Please input the hostname: ");
			scanf("%s", sHost);
			printf("> Please input the port: ");
			scanf("%d", &sPort);
			printf("> Connecting to %s:%d ...\n", sHost, sPort);
			conn = storage_connect(sHost, sPort);
			if(!conn){
				printf("Cannot connect to server @ %s:%d. Error code: %d.\n"
						, sHost, sPort, errno);
			}
			connected = 1;
			//delay
			//int i;
			//for(i = 0; i <=1000000000;i++);
		}

		// Authenticate the client
		else if(strcmp(selec,"2")==0){
			if(!connected){
				printf("> You are not connected to the server yet.\n");
			}
			else{
				printf("> Please input the username: ");
				scanf("%s", sUsername);
				printf("> Please input the password: ");
				scanf("%s", sPassword);
				//
				status = storage_auth(sUsername, sPassword, conn);
				if(status != 0){
					printf("storage_auth failed with username '%s' and password '%s'."
							"Error code: %d.\n", sUsername, sPassword, errno);
					storage_disconnect(conn);
					return status;
				}
				printf("storage_auth: successful. \n");
				loggedin = 1;
			}
		}

		// Issue storage_get
		else if(strcmp(selec, "3")==0){
			if(!loggedin){
				printf("> You haven't logged in yet.\n");
			}
			else{
				printf("> Please select a table from the database: ");
				scanf("%s", dTable);
				printf("> Please input the key:");
				scanf("%s", dKey);
				status = storage_get(dTable, dKey, &r, conn);
				if(status != 0){
					printf("storage_get failed. Error code: %d.\n", errno);
					storage_disconnect(conn);
					return status;
				}
			}
			printf("storage_get: the value returned for key '%s' is '%s'.\n",
					dKey, r.value);
		}

		// Issue storage_set
		else if(strcmp(selec,"4")==0){
			if(!loggedin){
				printf("> You haven't logged in yet.\n");
			}
			else{
				printf("> Please select a table from the database: ");
				scanf("%s", dTable);
				printf("> Please input the key: ");
				scanf("%s", dKey);
				printf("> Please input the value: ");
				scanf("%s", dValue);
				strncpy(r.value, dValue, sizeof r.value);
				status = storage_set(dTable, dKey, &r, conn);
				if(status != 0){
					printf("storage_set failed. Error code: %d.\n", errno);
					storage_disconnect(conn);
					return status;
				}
				printf("storage_set: successful.\n");
			}
		}

		// Disconnect from server
		else if(strcmp(selec, "5")==0){
			status = storage_disconnect(conn);
			if(status != 0){
				printf("storage_disconnet failed. Error code: %d.\n", errno);
				return status;
			}
		}

		// Exit
		else if(strcmp(selec,"6")==0){
			break;
		}

		// Invalid command
		else{
			printf("Invalid command.\n");
		}
	}

	return 0;
}


