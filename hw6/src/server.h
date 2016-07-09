#ifndef server_h
#define server_h

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <stdbool.h>
#include <openssl/ssl.h>
#include <openssl/rand.h> // -lssl -lcrypto
#include <openssl/sha.h>
#include "chatroom.h"
#include "user.h"
#include "signalHandlers.h"
#include "fileManager.h"
#include "wrap.h"
#include "protocol.h"
#include "password.h"
#include "threadpool.h"

#define USERS_DB "users.db"
#define SALTS_DB "salts.db"
#define HASHS_DB "hash.db"
#define INITIALNUMROOMS 5
#define INITIAL_FILE_SIZE 5000000
#define INITIAL_FILE_LIMIT 5
#define SERVER "server"

int ct = 0;

typedef struct serverData{

	int echo_flag;

	char* messageOfTheDay;

	int serverSocket;	/* Socket descriptor for server */

	unsigned short serverPort;      /* Server port */

	LIST_HEAD(chatroom_list, chatroom_t) chatroomHead;

	int roomLimit;

	int roomCount;

        int maxFileSize;

        int fileCount;

        int fileCountLimit;

	user_t* theServerAsAUser;

	pthread_t echoThreadID;

        pool_t* pool;
        
        uint16_t min;
        uint16_t max;
        uint16_t secs;
        uint16_t queueSize;
        
        

}serverData;

/* Here is the DatabaseItem struct it holds
 * a username
 * a hash - string of bytes (password+salt)
 * and a salt - the salt for the hash
 *
 * It has the LIST_ENTRY API macro that declares doubly linked pointers
 */
typedef struct databaseItem{
    
	char username[MSG_SIZE];

	unsigned char* passhash;

        unsigned char* salt;

	LIST_ENTRY(databaseItem) entry;

}databaseItem;

/* This is the database struct it merely points to the head of the list
 */
typedef struct database{

	LIST_HEAD(database_list, databaseItem) databaseHead;

}database;

struct loginInfo{
    int clntSock;
    char ip[16];
};

void initServer();

void parseArgs(int* argc, char*** argv);

void createServerSocket(struct sockaddr_in* echoServAddr);

void runServer();

void* login(void* clientSocketVoid);

void* echoThread(void* data);

void createNewUserAndLogin(user_t* newUser, char* in__buff);

void loginExistingUser(user_t* newUser, char* in__buff);

bool userNameAlreadyLoggedIn(char* name);

bool existsInDb(char* name);

bool incorrectPassword(char* user, char* pass);

void fillActiveFileSet(fd_set* set);

void handleClientCommunication(int currentSocket);

user_t* findUserBySocket(int socket);

chatroom_t* findChatroomByUser(user_t* currentUser);

bool roomNameIsTaken(char* roomName);

void echo(chatroom_t*, user_t*, char*);

void leave(chatroom_t*, user_t*, char*);

void createR(chatroom_t*, user_t*, char*);

void listR(chatroom_t*, user_t*, char*);

void join(chatroom_t*, user_t*, char*);

void kick(chatroom_t*, user_t*, char*);

void tell(chatroom_t*, user_t*, char*);

void listU(chatroom_t*, user_t*, char*);

void bye(chatroom_t*, user_t*, char*);

void download(chatroom_t*, user_t*, char*);

void upload(chatroom_t*, user_t*, char*);

void delete(chatroom_t*, user_t*, char*);

void listf(chatroom_t*, user_t*, char*);

void listfu(chatroom_t*, user_t*, char*);
void createP(chatroom_t* room, user_t* originUser, char* inputCommand);
void joinP(chatroom_t* room, user_t* originUser, char* inputCommand);

void logdump();

void writeOutToDatabase(char* name, char* hash, char* salt);

#define NUM_COMMANDS 16
char* verbToCommandList[] = {
	"MSG",
	"LEAVE",
	"CREATER",
	"LISTR",
	"JOIN",
	"KICK",
	"TELL",
	"LISTU",
	"BYE",
	"DOWNLOAD",
	"UPLOAD",
	"DELETE",
	"LISTF",
	"LISTFU",
        "CREATEP",
        "JOINP"
        
};
#define LOGDUMP "/logdump\n"
void (*commandFuncs[]) (chatroom_t*,user_t*,char*) = {
	&echo,
	&leave,
	&createR,
	&listR,
	&join,
	&kick,
	&tell,
	&listU,
	&bye,
	&download,
	&upload,
	&delete,
	&listf,
	&listfu,
        &createP,
        &joinP,
};


#define USAGE do{ 										\
fprintf(stderr, "\n./server [-he] PORT_NUMBER MOTD CONFIG_FILE"					\
                "\n\n\t-e Echo messages received on server's stdout."				\
                "\n\t-h Displays help menu & returns EXIT_SUCCESS."				\
                "\n\n\tPORT_NUMBER Port number to listen on."					\
                "\n\tMOTD Message to display to the client when they connect.\n\n"              \
                "\n\tCONFIG_FILE the name of the thread pool config file in the cwd.\n\n"       \
                "╦ ╦ ╔═╗ ╦   ╔═╗ ╦╔═ ╔═╗ ╦ ╦ ╦ ╦╔═ ╦\n"						\
                "╠═╣ ╠═╣ ║   ╠═╣ ╠╩╗ ╠═╣ ╠═╣ ║ ╠╩╗ ║\n"						\
                "╩ ╩ ╩ ╩ ╩═╝ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩\n\n"					\
                );										\
}while(0)


#endif
