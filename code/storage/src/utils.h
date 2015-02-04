/**
 * @file
 * @brief This file declares various utility functions that are
 * can be used by the storage server and client library.
 */

#ifndef	UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdbool.h>
#include "storage.h"


FILE *file_ptr_client;
int status_rec;

/**
 * @brief Any lines in the config file that start with this character 
 * are treated as comments.
 */
static const char CONFIG_COMMENT_CHAR = '#';

/**
 * @brief The max length in bytes of a command from the client to the server.
 */
#define MAX_CMD_LEN (1024 * 8)

/**
 * @brief A macro to log some information.
 *
 * Use it like this:  LOG(("Hello %s", "world\n"))
 *
 * Don't forget the double parentheses, or you'll get weird errors!
 */
#define LOG(x)  {printf x; fflush(stdout);}

/**
 * @brief A macro to output debug information.
 * 
 * It is only enabled in debug builds.
 */
#ifdef NDEBUG
#define DBG(x)  {}
#else
#define DBG(x)  {printf x; fflush(stdout);}
#endif

/**
 * @brief A struct to store config parameters.
 */
struct config_params {
	/// The hostname of the server.
	char server_host[MAX_HOST_LEN];
	
	/// The listening port of the server.
	int server_port;

	/// The storage server's username
	char username[MAX_USERNAME_LEN];

	/// The storage server's encrypted password
	char password[MAX_ENC_PASSWORD_LEN];

	/// The directory where tables are stored.
	char table[MAX_TABLES][MAX_TABLE_LEN];


};

/**
 * @brief Exit the program because a f
#include <stdbool.h>

#define TABLE_BASE_SIZE 10
#define TABLE_BRANCH_SIZE 10
atal error occured.
 *
 * @param msg The error message to print.
 * @param code The program exit return value.
 */
static inline void die(char *msg, int code)
{
	printf("%s\n", msg);
	exit(code);
}

/**
 * @brief Keep sending the contents of the buffer until complete.
 * @return Return 0 on success, -1 otherwise.
 *
 * The parameters mimic the send() function.
 */
int sendall(const int sock, const char *buf, const size_t len);

/**
 * @brief Receive an entire line from a socket.
 * @return Return 0 on success, -1 otherwise.
 */
int recvline(const int sock, char *buf, const size_t buflen);

/**
 * @brief Read and load configuration parameters.
 *
 * @param config_file The name of the configuration file.
 * @param params The structure where config parameters are loaded.
 * @return Return 0 on success, -1 otherwise.
 */
int read_config(const char *config_file, struct config_params *params);

/**
 * @brief Generates a log message.
 * 
 * @param file The output stream
 * @param message Message to log.
 */
void logger(FILE *file, char *message);

/**
 * @brief Default two character salt used for password encryption.
 */
#define DEFAULT_CRYPT_SALT "xx"

/**
 * @brief Generates an encrypted password string using salt CRYPT_SALT.
 * 
 * @param passwd Password before encryption.
 * @param salt Salt used to encrypt the password. If NULL default value
 * DEFAULT_CRYPT_SALT is used.
 * @return Returns encrypted password.
 */
char *generate_encrypted_password(const char *passwd, const char *salt);

/**
 * parser function check for invalid table and key input
 * returns 0 if input is valid
 * returns -1 if input is invalid
 */
int invalid_tk(const char *tk_param);

/**
 * parser function check for invalid value input
 * returns 0 if input is valid
 * returns -1 if input is invalid
 */
int invalid_value(const char *value_param);



#define TABLE_BASE_SIZE 10
#define TABLE_BRANCH_SIZE 10

typedef struct linked_list {
    char *key;
    char *record;
    struct linked_list *next;
} list_t;


typedef struct hash_hash_table {
    list_t *entry[TABLE_BRANCH_SIZE];
} table_t;


table_t *createTable(void);
unsigned primaryHash(char *key);
unsigned secondaryHash(char *key);
list_t *retrieve(char *key, table_t *table);
list_t *findInList(char *key, list_t *head);
bool insertOrUpdate(char *key, char *record, table_t *table);
list_t *insert(char *key, char *record, table_t *table);
list_t *addEntry(char *key, char *record);
bool deleteEntry(char *key, table_t *table);
void deleteTable(table_t *table);
void deleteList(list_t *head);


typedef struct data_base{
	char *table_name;
	table_t *table;
	struct data_base *next;
} base_t; 


base_t *create_data_base(struct config_params *params, int data_base_size);

table_t *findTable(char* table, base_t *head);


#endif
