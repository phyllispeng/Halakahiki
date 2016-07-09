#ifndef server_h
#define server_h

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
#include <openssl/sha.h>
#include <openssl/rand.h>
#include "wrap.h"
#include "pineappleProtocol.h"
#include "user.h"
#include "Hashmap.h"
#include "chatroom.h"
#include "pass.h"
#include "parser.h"
#include "filemanager.h"

#define NEWLINE write(1," \r\n",3)

#define TRUE  1
#define FALSE 0
#define MSG_SIZE 1024

#define SERVER "server"
#define DBFILE "server.db"

int showMssgs;

typedef struct serverData{

	int echo_flag;

	char* motd;

	int servSock;	/* Socket descriptor for server */

	unsigned short echoServPort;      /* Server port */

	Hashmap* users;
	//
	chatroomList_t* chatrooms;

	int	numChatRoomsPossible;

	int numChatRooms;

	pthread_t echoThreadid;

	chatroom_t* waitingRoom;

	user_t* me;

	FILE* db_file;

}serverData;

serverData* server;

typedef struct clientThreadData{

	pthread_t threadid;

	int socketfd;

	char* name; //pass user_t struct actually

}clientThreadData;

typedef struct database{
	
	struct database* next;

	char* username;

	char* passhash;

}database;

database* db;

int serverRun();

void sigintHandler(int sig);

void sigsegvHandler(int sig);

void createNewClientThread(int clntSocket);

void* clientThread(void*);

int addNewClient(int clientSocket);

void* echoThread(void*);

void checkIn(int clntSock, struct sockaddr_in* echoClntAddr);

int incorrectPassword(char* user, char* pass);

int echo(chatroom_t*,user_t*,char*);

int SendServer(int sockfd, const void *buf, size_t len, int flags, user_t* reciever);

int removeAllInstancesofUser(user_t*);

void close_all_users();

int	leave(chatroom_t*, user_t*, char*);

int	createR(chatroom_t*, user_t*, char*);

int	listR(chatroom_t*, user_t*, char*);

int	join(chatroom_t*, user_t*, char*);

int	kick(chatroom_t*, user_t*, char*);

int	tell(chatroom_t*, user_t*, char*);

int	listU(chatroom_t*, user_t*, char*);

int bye(chatroom_t*, user_t*, char*);

int download(chatroom_t*, user_t*, char*);

int upload(chatroom_t*, user_t*, char*);

int delete(chatroom_t*, user_t*, char*);

int listf(chatroom_t*, user_t*, char*);

int listfu(chatroom_t*, user_t*, char*);

int existsInDb(char* name);

int writeOutUsers();

int writeOutAUser(char* name, char* pass);

#define NUM_COMMANDS 14
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
	"LISTFU"
};
int (*commandFuncs[]) (chatroom_t*,user_t*,char*) = {
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
	&listfu
};

#define USAGE do{ 																	\
fprintf(stderr, "\n./server [-he] PORT_NUMBER MOTD"									\
				"\n\n\t-e Echo messages received on server's stdout."				\
				"\n\t-h Displays help menu & returns EXIT_SUCCESS."					\
				"\n\n\tPORT_NUMBER Port number to listen on."						\
				"\n\tMOTD Message to display to the client when they connect.\n\n"	\
				"╦ ╦ ╔═╗ ╦   ╔═╗ ╦╔═ ╔═╗ ╦ ╦ ╦ ╦╔═ ╦\n"								\
				"╠═╣ ╠═╣ ║   ╠═╣ ╠╩╗ ╠═╣ ╠═╣ ║ ╠╩╗ ║\n"								\
				"╩ ╩ ╩ ╩ ╩═╝ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩\n\n"							\
				);																	\
}while(0)

#endif
