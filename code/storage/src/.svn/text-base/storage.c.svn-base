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
#include "storage.h"
#include "utils.h"


/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
void* storage_connect(const char *hostname, const int port)
{	
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
	int status2 = connect(sock, res->ai_addr, res->ai_addrlen);	// Connect to the server.
	sprintf(msg, "storage_connect:\tgetaddrinfo()=%d, connect()=%d\n",status1, status2);
	logger(file_ptr_client, msg); 
	if (status1 != 0)
		return NULL; 
	if (status2 != 0)
		return NULL;

	return (void*) sock;
}


/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
int storage_auth(const char *username, const char *passwd, void *conn)
{
	// Connection is really just a socket file descriptor.
	int sock = (int)conn;

	// Send some data.
	char buf[MAX_CMD_LEN];
	memset(buf, 0, sizeof buf);
	char *encrypted_passwd = generate_encrypted_password(passwd, NULL);
	snprintf(buf, sizeof buf, "AUTH %s %s\n", username, encrypted_passwd);

	// This logger function provides the values for both sendall() and revline(),
	// so that when this function returns -1, we can tell where the error(s) is.
	char msg[20];
	int sen = sendall(sock, buf, strlen(buf));
	int rec = recvline(sock, buf, sizeof buf);
	sprintf(msg,  "storage_auth:\t sendall()=%d, revline()=%d\n", sen, rec);
	logger(file_ptr_client, msg);

	if (sen == 0 && rec == 0) {
		return 0;
	}

	return -1;
}

/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
int storage_get(const char *table, const char *key, struct storage_record *record, void *conn)
{
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
		return 0;
	} {
		strncpy(record->value, buf, sizeof record->value);
		return 0;
	}

	return -1;
}


/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
int storage_set(const char *table, const char *key, struct storage_record *record, void *conn)
{
	// Connection is really just a socket file descriptor.
	int sock = (int)conn;

	// Send some data.
	char buf[MAX_CMD_LEN];
	memset(buf, 0, sizeof buf);
	snprintf(buf, sizeof buf, "SET %s %s %s\n", table, key, record->value);

	// This logger function provides the values for both sendall() and revline(),
	// so that when this function returns -1, we can tell where the error(s) is.
	char msg[20];
	int sen = sendall(sock, buf, strlen(buf));
	int rec = recvline(sock, buf, sizeof buf);
	sprintf(msg,  "storage_set:\t sendall()=%d, revline()=%d\n", sen, rec);
	logger(file_ptr_client, msg);

	if (sen == 0 && rec == 0) {
		return 0;
	} {
		return 0;
	}

	return -1;
}


/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
int storage_disconnect(void *conn)
{
	// Cleanup
	int sock = (int)conn;
	close(sock);

	return 0;
}

