/**
 * @file
 * @brief This file contains the implementation of the storage server
 * interface as specified in storage.h.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <math.h>
#include "storage.h"
#include "utils.h"


/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
void* storage_connect(const char *hostname, const int port)
{
	if((port < 10*MAX_PORT_LEN)|| strlen(hostname) > MAX_HOST_LEN){
		// parameter(s) is(are) too long
		errno = ERR_INVALID_PARAM;
		return NULL;	
	}

	// Create a socket.
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return NULL;
	// Get info about the server.
	struct addrinfo serveraddr, *res;
	memset(&serveraddr, 0, sizeof serveraddr);
	serveraddr.ai_family = AF_UNSPEC;
	serveraddr.ai_socktype = SOCK_STREAM;
	char portstr[MAX_PORT_LEN];
	snprintf(portstr, sizeof portstr, "%d", port);

	char msg[100];
	int status1 = getaddrinfo(hostname, portstr, &serveraddr, &res);	// Get info about the server.
        int status2 = -10;
	if (status1 != 0){
		errno = ERR_CONNECTION_FAIL;
	}
        else{
		//printf("addr: %
                status2 = connect(sock, res->ai_addr, res->ai_addrlen);	// Connect to the server.
                if (status2 != 0){
			//char *errmsg = xn_geterror_string(status2);
			//printf("%s",errmsg);
                        //errno = ERR_CONNECTION_FAIL;
                }
        }
        sprintf(msg, "storage_connect:\tgetaddrinfo()=%d, connect()=%d\n",status1, status2);
	logger(file_ptr_client, msg); 
        
	if(status1 || status2){
		// connection fail
                return NULL;
	}

	// if coonection sucess, client status is connected
	status_rec = 1;
	return (void*) sock;
}


/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
int storage_auth(const char *username, const char *passwd, void *conn)
{
	if(status_rec == 1){

		if(strlen(username) > MAX_USERNAME_LEN || strlen(passwd) > MAX_ENC_PASSWORD_LEN){
			// parameter(s) is(are) too long
			errno = ERR_INVALID_PARAM;
			return -1;	
		}

		// Connection is really just a socket file descriptor.
		int sock = (int)conn;

		// Send some data.
		char buf[MAX_CMD_LEN];
		memset(buf, 0, sizeof buf);
		char *encrypted_passwd = generate_encrypted_password(passwd, NULL);
		snprintf(buf, sizeof buf, "AUTH %s %s\n", username, encrypted_passwd);
		printf("buffer: %s",buf);
	
		// This logger function provides the values for both sendall() and revline(),
		// so that when this function returns -1, we can tell where the error(s) is.
		char msg[20];
		int sen = sendall(sock, buf, strlen(buf));
		int rec = recvline(sock, buf, sizeof buf);
		sprintf(msg,  "storage_auth:\t sendall()=%d, revline()=%d\n", sen, rec);
		logger(file_ptr_client, msg);
		if (sen == 0 && rec == 0) {
			//check for authentication status
			if(!strcmp(buf,"_0")) {
				status_rec = 2;
				return 0;		
			}	
			else{
				errno = ERR_AUTHENTICATION_FAILED; 
				return -1;
			}
		}
		// else, an authentication error happened		
		return -1;
	}
	// else, not connected.
	errno = ERR_CONNECTION_FAIL;
	return -1;
}

/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
int storage_get(const char *table, const char *key, struct storage_record *record, void *conn)
{
	if(status_rec == 0){
		// client is not connected to the server
		errno = ERR_CONNECTION_FAIL;
		return -1;
	}
	else if(status_rec == 1){
		// client is not authenticated
		errno = ERR_NOT_AUTHENTICATED;
		return -1;
	}
	else if(status_rec == 2){
		// check if parameters are valid

		if(strlen(table) > MAX_TABLE_LEN || strlen(key) > MAX_KEY_LEN || strlen(key) > MAX_VALUE_LEN){
			// parameter(s) is(are) too long
			errno = ERR_INVALID_PARAM;
			return -1;	
		}

		if(invalid_tk(table)==-1 || invalid_tk(key)==-1){
			// either table or key is invalid.
			errno = ERR_INVALID_PARAM;			
			return -1;

		}
		else{
			// client is authenticated

			// Connection is really just a socket file descriptor.
			int sock = (int)conn;

			// Send some data.
			char buf[MAX_CMD_LEN];
			memset(buf, 0, sizeof buf);
			snprintf(buf, sizeof buf, "GET %s %s\n", table, key);

			// This logger function provides the values for both sendall() and revline(),
			// so that when this function returns -1, we can tell where the error(s) is.
			char msg[20];
			int sen = sendall(sock, buf, strlen(buf));
			int rec = recvline(sock, buf, sizeof buf);
			sprintf(msg,  "storage_get:\t sendall()=%d, revline()=%d\n", sen, rec);
			logger(file_ptr_client, msg);

			if (sen == 0 && rec == 0) {
				if(!strcmp(buf,"_2")){
					errno = ERR_TABLE_NOT_FOUND;
					return -1;
				}
				else if(!strcmp(buf,"_3")){
					errno = ERR_KEY_NOT_FOUND;
					return -1;
				}
				else{
					strncpy(record->value, buf, sizeof record->value);
					return 0;
				}
			}
			return -1;
		}
	}
	// if status_rec is not equal to 0, 1, or 2...
	errno = ERR_UNKNOWN;
	return -1;
		
}


/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
int storage_set(const char *table, const char *key, struct storage_record *record, void *conn)
{
	if(status_rec == 0){
		// client is not connected to the server
		errno = ERR_CONNECTION_FAIL;
		return -1;
	}
	else if(status_rec == 1){
		// client is not authenticated
		errno = ERR_NOT_AUTHENTICATED;
		return -1;
	}
	else if(status_rec == 2){
		// check if parameters are valid
		if(invalid_tk(table)==-1 || invalid_tk(key)==-1 || invalid_value(record->value)==-1){
			// either table, key or value is invalid.
			errno = ERR_INVALID_PARAM;			
			return -1;
		}
		else{
			// table, key, and value are valid
			// Connection is really just a socket file descriptor.
			int sock = (int)conn;

			// Send some data.
			char buf[MAX_CMD_LEN];
			memset(buf, 0, sizeof buf);
			snprintf(buf, sizeof buf, "SET %s %s %s\n", table, key, record->value);
			printf("buf: %s\n",buf);

			// This logger function provides the values for both sendall() and revline(),
			// so that when this function returns -1, we can tell where the error(s) is.
			char msg[20];
			int sen = sendall(sock, buf, strlen(buf));
			int rec = recvline(sock, buf, sizeof buf);
			sprintf(msg,  "storage_set:\t sendall()=%d, revline()=%d\n", sen, rec);
			logger(file_ptr_client, msg);
			
			printf("buf: %s\n",buf);			

			if (sen == 0 && rec == 0) {
				if(!strcmp(buf,"_2")){
					errno = ERR_TABLE_NOT_FOUND;
					return -1;
				}
				else if(!strcmp(buf,"_3")){
					errno = ERR_KEY_NOT_FOUND;
					return -1;
				}
				else if(!strcmp(buf,"_1")){
					errno = ERR_UNKNOWN;
					return -1;
				}
				return 0;
			}
			return -1;
		}
	}
	// if status_rec is not equal to 0, 1, or 2...
	errno = ERR_UNKNOWN;
	return -1;
}


/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
int storage_disconnect(void *conn)
{
	if(status_rec == 1 || status_rec == 2)
	{
		// if the client is connected, cleanup
		int sock = (int)conn;
		close(sock);
		status_rec = 0;
		return 0;
	}

	else
	{
		// client is not connected, there's no connection to be closed
		errno = ERR_UNKNOWN;
		return -1;
	}
}

