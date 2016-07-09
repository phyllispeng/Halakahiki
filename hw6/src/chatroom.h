#ifndef chatroom_h
#define chatroom_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <stdbool.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <semaphore.h>
#include <time.h>
#include "user.h"
#include "wrap.h"
#include "parser.h"
#include "fileManager.h"
#include "protocol.h"

#define DEFAULT_NUMBER_NODE 257

#define TRUE 1
#define FALSE 0
#define MAX_IN 1024
#define SUCCESS 7

typedef struct file_info{

	user_t* uploader;

	char* fileName;

	unsigned int file_id;

        size_t bytes;

        LIST_ENTRY(file_info) entry;

}file_info;

typedef struct chatroom_t{

	char* name;

	int userCount;

	unsigned int chatroomID; // the hash number

	user_t* admin;

	LIST_HEAD(file_list_t, file_info) files;
        
        int fileCount;
        
        char* salt;
        
        char* hashedPass;
        
        int roomType;

	LIST_ENTRY(chatroom_t) entry;

	LIST_HEAD(user_list, user_t) userHead;

}chatroom_t;

void broadcastToRoom(chatroom_t* room, char* out_buff);

chatroom_t* createChatroom(char* label, char* salt, char* hashedPass, int type);


void moveFromTo(chatroom_t* fromRoom, chatroom_t* toRoom, user_t*);

void removeUser(chatroom_t* room, user_t* user);
void addUser(chatroom_t* room, user_t* user);
void destroyRoom(chatroom_t* room);
user_t* getUserByName(chatroom_t* room, char* userToKickName);
file_info* getFileInfoById(chatroom_t* room, unsigned int id);
void addFileToRoom(chatroom_t* room, file_info* file);
void removeFileFromRoom(chatroom_t* room, file_info* file);



//int isFileValid(char* fileName);

void sendFile(file_info* fileToSend, user_t* originUser);

file_info* receiveFile(user_t* user, char* filename, size_t size);
// should be called when user use /logdum 
// file name should be athu.log
void log_file_reader();
// should be called everytime while the user is being made 
void log_file_writer(char* ip, int error_num, char* user_name);

void sendRETAERC(user_t* originUser, chatroom_t* newRoom);

void sendPETAERC(user_t*, chatroom_t*);

void initFileStuff();

#endif