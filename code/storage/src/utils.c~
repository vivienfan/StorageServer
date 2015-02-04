/**
 * @file
 * @brief This file implements various utility functions that are
 * can be used by the storage server and client library. 
 */

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "utils.h"


int sendall(const int sock, const char *buf, const size_t len)
{
	size_t tosend = len;
	while (tosend > 0) {
		ssize_t bytes = send(sock, buf, tosend, 0);
		if (bytes <= 0) 
			break; // send() was not successful, so stop.
		tosend -= (size_t) bytes;
		buf += bytes;
	};

	return tosend == 0 ? 0 : -1;
}

/**
 * In order to avoid reading more than a line from the stream,
 * this function only reads one byte at a time.  This is very
 * inefficient, and you are free to optimize it or implement your
 * own function.
 */
int recvline(const int sock, char *buf, const size_t buflen)
{
	int status = 0; // Return status.
	size_t bufleft = buflen;

	while (bufleft > 1) {
		// Read one byte from scoket.
		ssize_t bytes = recv(sock, buf, 1, 0);
		if (bytes <= 0) {
			// recv() was not successful, so stop.
			status = -1;
			break;
		} else if (*buf == '\n') {
			// Found end of line, so stop.
			*buf = 0; // Replace end of line with a null terminator.
			status = 0;
			break;
		} else {
			// Keep going.
			bufleft -= 1;
			buf += 1;
		}
	}
	*buf = 0; // add null terminator in case it's not already there.

	return status;
}


/**
 * @brief Parse and process a line in the config file.
 */
int process_config_line(char *line, struct config_params *params, int* all_set)
{
	// Ignore comments.
	if (line[0] == CONFIG_COMMENT_CHAR)
		return 0;

	// Extract config parameter name and value.
	char name[MAX_CONFIG_LINE_LEN];
	char value[MAX_CONFIG_LINE_LEN];
	int items = sscanf(line, "%s %s\n", name, value);

	// Line wasn't as expected.
	if (items != 2){
		if(items==-1)		//empty line ignored
			return 0;
		else{
			printf("Not enough parameter!\n");		
			return -1;
		}
	}

	// Process this line.
	if (strcmp(name, "server_host") == 0) {
		all_set[0]=1;						// server_host param ensured
		if(params->server_host[0]=='\0'){		// first time see server_host ensured
			if(strlen(value)<=MAX_HOST_LEN){
				strncpy(params->server_host, value, sizeof params->server_host);
				printf("params->server_host = %s//\n", params->server_host);
			}
			else{
				printf("Host name is too long!\n");
				return -1;
				}
		}
		else{
			printf("Duplicate Server_Host!\n");
			return -1;
		}
	} 
	else if (strcmp(name, "server_port") == 0) {
		all_set[1]=1;						// server_port param ensured
		if(params->server_port!=0){			// first time see server_port ensured
			printf("Duplicate Server_Port!\n");
			return -1;
		}
		else{
			if(strlen(value)<=MAX_PORT_LEN){
				params->server_port = atoi(value);
				printf("params->server_port = %d//\n", params->server_port);
			}
			else{
				printf("The server port is too long!\n");
				return -1;
				}
				
			}
	} 
	else if (strcmp(name, "username") == 0) {
		all_set[2]=1;						// username param ensured
		if(params->username[0]=='\0'){			// first time see username ensured

			if(strlen(value)<=MAX_USERNAME_LEN){
				strncpy(params->username, value, sizeof params->username);
				printf("params->username = %s\n", params->username);
			}
			else{
				printf("User name is too long!\n");
				return -1;
			}
		}
		else{
			printf("Duplicate Username!\n");
			return -1;
		}
	} 
	else if (strcmp(name, "password") == 0) {
		all_set[3]=1;						// password param ensured
		if(params->password[0]=='\0'){			// first time see password ensured
			if(strlen(value)<=MAX_ENC_PASSWORD_LEN){
				strncpy(params->password, value, sizeof params->password);
				printf("params->password = %s\n", params->password);
			}
			else{
				printf("Password is too long!\n");
				return -1;
			}
		}
		else{
			printf("Duplicate Password!\n");
			return -1;
		}
	} 
	else if (strcmp(name, "table") == 0){
		all_set[4]=1;						// table param ensured
		int i;	
		if(strlen(value)<=MAX_TABLE_LEN){						
			for(i=0;i<MAX_TABLES&&params->table[i][0]!='\0';i++){		// check for duplicate table names and insert a new name if no duplication occurred
				if(strcmp(params->table[i], value)==0){
					printf("Duplicate table name!\n");
					return -1;
				}
			}
			strncpy(params->table[i], value, sizeof params->table[i]);
			printf("params->table[%d] = %s\n", i,params->table[i]); 
		}
		else{
			printf("Table name is too long!\n");
			return -1;
		}
		
	}
	// else if (strcmp(name, "data_directory") == 0) {
	//	strncpy(params->data_directory, value, sizeof params->data_directory);
	//} 
	else {
		// Ignore unknown config parameters.
	}

	return 0;
}


int read_config(const char *config_file, struct config_params *params)
{
	int error_occurred = 0;										// set error flag
    int detector=0;												// set error detector
	int all_set[5]={0,0,0,0,0};									// initialize missing-param indicator
	// Open file for reading.
	FILE *file = fopen(config_file, "r");
	if (file == NULL)
		error_occurred = 1;

	// Process the config file.
	while (!error_occurred && !feof(file)) {
		// Read a line from the file.
		char line[MAX_CONFIG_LINE_LEN];
		char *l = fgets(line, sizeof line, file);

		// Process the line.
		if (l == line)
			detector=process_config_line(line, params, all_set);		// process this line in config file
		
		if (detector!=0)												
			error_occurred = 1;										// detector detected error
		if(feof(file))
			error_occurred = 0;
	}

	if(all_set[0]==0&&error_occurred==0){												// cases of missing parameters
		printf("Missing server_host!\n");
		error_occurred = 1;
	}
	else if(all_set[1]==0&&error_occurred==0){
		printf("Missing server_port!\n");
		error_occurred = 1;
    }
	else if(all_set[2]==0&&error_occurred==0){
		printf("Missing Username!\n");
		error_occurred = 1;
    }
	else if(all_set[3]==0&&error_occurred==0){
		printf("Missing Password!\n");
		error_occurred = 1;
    }
	else if(all_set[1]==0&&error_occurred==0){
		printf("Missing Table name!\n");
		error_occurred = 1;
    }
	return error_occurred ? -1 : 0;
}


void logger(FILE *file, char *message)
{
	if(file){
		// when LOGGING is either 1 or 2
		fprintf(file,"%s",message);
		fflush(file);
	}
	// else, when LOGGING is set to 0
	return;
}


char *generate_encrypted_password(const char *passwd, const char *salt)
{
	if(salt != NULL)
		return crypt(passwd, salt);
	else
		return crypt(passwd, DEFAULT_CRYPT_SALT);
}


/**
 * parser function check for invalid table and key input
 * returns 0 if input is valid
 * returns -1 if input is invalid
 */
int invalid_tk(const char *tk_param) {
	int i = 0;
	while(tk_param[i]) {
		if(!isalnum(tk_param[i])) {
			return -1;
		}
		i++;
	}
	return 0;
}


/**
 * parser function check for invalid value input
 * returns 0 if input is valid
 * returns -1 if input is invalid
 */
int invalid_value(const char *value_param) {
	int i = 0;
	while(value_param[i]) {
		if(!isalnum(value_param[i])) {
			if(value_param[i] != ' '){
				return -1;
			}
		}
		i++;
	}
	return 0;
}



table_t *createTable(void)
{
    table_t *table = (table_t *)malloc(sizeof(table_t) * TABLE_BASE_SIZE);

    int i, j;

    for(i = 0; i < TABLE_BASE_SIZE; i++)
        for(j = 0; j < TABLE_BRANCH_SIZE; j++)
            table[i].entry[j] = NULL;

    return table;
}


unsigned primaryHash(char *key)
{
    return key[0] % TABLE_BASE_SIZE;
}


unsigned secondaryHash(char *key)
{
    unsigned hashed_key = 0;

    for(key++; *key != '\0'; key++)
        hashed_key += *key;

    return hashed_key % TABLE_BRANCH_SIZE;
}


list_t *retrieve(char *key, table_t *table)
{
    unsigned base_hash_index, branch_hash_index;

    base_hash_index = primaryHash(key);
    branch_hash_index = secondaryHash(key);

    list_t *tmp = table[base_hash_index].entry[branch_hash_index];

    if(tmp == NULL)
        return NULL;
    else
        return findInList(key, tmp);
}


list_t *findInList(char *key, list_t *head)
{
    list_t *tmp = head;

    while(tmp != NULL)
    {
        if(strcmp(key, tmp->key) == 0)
            return tmp;
        tmp = tmp->next;
    }

    return tmp;
}


bool insertOrUpdate(char *key, char *record, table_t *table)
{
    list_t *tmp;

    if((tmp = retrieve(key, table)) == NULL)
    {
        if(insert(key, record, table) != NULL)
            return true;
    }
    else
    {
        if(strcmp(record, tmp->record) != 0)
            strcpy(tmp->record,record);
        return true;
    }
    return false;
}


list_t *insert(char *key, char *record, table_t *table)
{
    unsigned base, branch;

    base = primaryHash(key);
    branch = secondaryHash(key);

    list_t *head = table[base].entry[branch];

    if(head == NULL)
    {   
        table[base].entry[branch] = addEntry(key, record);
        return table[base].entry[branch];
    }
    else
    {
        list_t *tmp = head;
        while(tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = addEntry(key, record);

        return tmp->next;
    }
}


list_t *addEntry(char *key, char *record)
{
    list_t *tmp = (list_t *)malloc(sizeof(list_t));

    if(tmp != NULL)
    {
        strcpy(tmp->key,key);
        strcpy(tmp->record,record);
        tmp->next = NULL;
    }

    return tmp;
}


bool deleteEntry(char *key, table_t *table)
{
    list_t *tmp;

    if((tmp = retrieve(key, table)) == NULL)
        return false;
    else
    {
        list_t *prev = NULL, *curr = table[primaryHash(key)].entry[secondaryHash(key)];

        while(curr != tmp)
        {
            prev = curr;
            curr = curr->next;
        }

        if(prev == NULL)
            table[primaryHash(key)].entry[secondaryHash(key)] = curr->next;
        else
            prev->next = curr->next;

        free(tmp);    
    }

    return true;
}


void deleteTable(table_t *table)
{
    list_t *tmp;

    int i, j;

    for(i = 0; i < TABLE_BASE_SIZE; i++)
        for(j = 0; j < TABLE_BRANCH_SIZE; j++)
        {
            if((tmp = table[i].entry[j]) == NULL)
                continue;

            deleteList(tmp);
        }

    free(table);
}


void deleteList(list_t *head)
{
    list_t *tmp;

    while(head != NULL)
    {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}


base_t *create_data_base(struct config_params *params, int data_base_size){
	int i;
	base_t *head = (base_t *)malloc(sizeof(base_t) * data_base_size);
	base_t *temp = head;
	for(i=0; i <= data_base_size; i++){
		strcpy(temp->table_name, params->table[i]);
		temp->table = (table_t *)malloc(sizeof(table_t) *TABLE_BASE_SIZE);
		temp = temp->next;
	}
	return head;
}
		

table_t *findTable(char* table, base_t *head){
	base_t *tmp = head;
	while(tmp){
		if(!strcmp(table, tmp->table_name))
		{
			// table name matches, table found, return the table pointer
			return tmp->table;
		}
		else{
			// table name does not matches, keep looping
			tmp = tmp->next;
		}
	}
	// finish looping, did not find the table, table not found
	return NULL;
}



















	

