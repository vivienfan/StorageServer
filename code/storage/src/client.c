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
#include "utils.h"

#define LOGGING 1
FILE *file_ptr_client;



/**
 * @brief Start a client to interact with the storage server.
 *
 * If connect is successful, the client performs a storage_set/get() on
 * TABLE and KEY and outputs the results on stdout. Finally, it exists
 * after disconnecting from the server.
 */
int main(int argc, char *argv[]) {
	int sPort=0, status=0;
	char selec[10], sHost[2*MAX_HOST_LEN],  sUsername[2*MAX_USERNAME_LEN],
		sPassword[2*MAX_ENC_PASSWORD_LEN], dTable[2*MAX_TABLE_LEN], dKey[2*MAX_KEY_LEN],
		dValue[1024];//sPort[MAX_PORT_LEN+1],
	char *temp = malloc(sizeof(char)*100);
	/*char *selec = malloc(sizeof(char)*100);
	char *sHost = malloc(sizeof(char)*100);
	char *sPort = malloc(sizeof(char)*100);
	char *sUsername = malloc(sizeof(char)*100);
	char *sPassword = malloc(sizeof(char)*100);
	char *dTable = malloc(sizeof(char)*100);
	char *dKey = malloc(sizeof(char)*100);
	char *dValue = malloc(sizeof(char)*100);
*/	void *conn;
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
	else{
		file_ptr_client = NULL;
	}

	while(true){
		printf("> ---------------------\n"
				"> 1) Connect\n"
				"> 2) Authenticate\n"
				"> 3) Get\n"
				"> 4) Set\n"
				"> 5) Disconnect\n"
				"> 6) Exit\n"
				"> ---------------------\n");
	
		printf("> Please enter your selection: ");
		scanf(" ");
		fgets(temp,sizeof(selec),stdin);
		temp = strtok(temp,"\n");
		strcpy(selec,temp);
		

		// Connect to server
		if(strcmp(selec,"1")==0){

			printf("> Please input the hostname: ");
			scanf(" ");
			fgets(temp,sizeof(sHost),stdin);
			temp = strtok(temp,"\n");
			strcpy(sHost,temp);

			printf("> Please input the port: ");
			scanf(" ");
			scanf("%d", &sPort);
			conn = storage_connect(sHost, sPort);
			if(!conn){
				printf("Cannot connect to server @ %s:%d. Error code: %d.\n"
						, sHost, sPort, errno);
			}
			else{
				printf("Got a connection from %s:%d\n", sHost, sPort);
			}		
		}

		// Authenticate the client
		else if(strcmp(selec,"2")==0){
			printf("> Please input the username: ");
			scanf(" ");
			fgets(temp,sizeof(sUsername),stdin);
			temp = strtok(temp,"\n");
			strcpy(sUsername,temp);

			printf("> Please input the password: ");
			scanf(" ");
			fgets(temp,sizeof(sPassword),stdin);
			temp = strtok(temp,"\n");
                        strcpy(sPassword,temp);
			
			status = storage_auth(sUsername, sPassword, conn);
			if(status != 0){
				printf("storage_auth failed with username '%s' and password '%s'."
						"Error code: %d.\n", sUsername, sPassword, errno);
				//storage_disconnect(conn);
				//return status;
			}
			else{
				printf("storage_auth: successful. \n");
			}	
		}

		// Issue storage_get
		else if(strcmp(selec, "3")==0){
			printf("> Please select a table from the database: ");
			scanf(" ");
			fgets(temp,sizeof(dTable),stdin);
			temp = strtok(temp,"\n");
			strcpy(dTable,temp);		

			printf("> Please input the key:");
			scanf(" ");
			fgets(temp,sizeof(dKey),stdin);
			temp = strtok(temp,"\n");
			strcpy(dKey,temp);

			status = storage_get(dTable, dKey, &r, conn);
			if(status != 0){
				printf("storage_get failed. Error code: %d.\n", errno);
				//storage_disconnect(conn);
				//return status;
			}
			else{
				printf("storage_get: the value returned for key '%s' is '%s'.\n",
						dKey, r.value);
			}	
		}

		// Issue storage_set
		else if(strcmp(selec,"4")==0){
			printf("> Please select a table from the database: ");
			scanf(" ");
			fgets(temp,sizeof(dTable),stdin);
			temp = strtok(temp,"\n");
			strcpy(dTable,temp);

			printf("> Please input the key: "); 
			scanf(" ");
			fgets(temp,sizeof(dKey),stdin);
			temp = strtok(temp,"\n");
			strcpy(dKey,temp);

			printf("> Please input the value: ");
			scanf(" ");
			fgets(temp,sizeof(dValue),stdin);
			temp = strtok(temp,"\n");
			strcpy(dValue,temp);
			strncpy(r.value, dValue, sizeof r.value);

			status = storage_set(dTable, dKey, &r, conn);
			if(status != 0){
				printf("storage_set failed. Error code: %d.\n", errno);
				//storage_disconnect(conn);
				//return status;
			}
			else{
				printf("storage_set: successful.\n");
			}
		}

		// Disconnect from server
		else if(strcmp(selec, "5")==0){
			status = storage_disconnect(conn);
			if(status != 0){
				printf("storage_disconnet failed. Error code: %d.\n", errno);
				//return status;
			}
		}

		// Exit
		else if(strcmp(selec,"6")==0){
			status = storage_disconnect(conn);
			break;
		}

		// Invalid command
		else{
			printf("Invalid command.\n");
		}
	}

	return 0;
}


