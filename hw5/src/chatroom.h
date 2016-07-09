#ifndef chatroom_h
#define chatroom_h


#include "user.h"
#include "Hashmap.h"
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#define TRUE  1
#define FALSE 0

#define EMPTY_ROOM -234
#define INITIALNUMROOMS -12

typedef struct usersList{

	struct usersList* next;

	user_t* usr;

}usersList;


typedef struct chatroom_t{

	char* name;

	int numOfUsers;

	int chatRoomID; // the hash number

	usersList* users;

	user_t* admin;

	DIR* folderName; // folder with files

}chatroom_t;

typedef struct chatroomList_t{
	chatroom_t* cr;
	struct chatroomList_t* next;
}chatroomList_t;

int addUser(chatroom_t* room, user_t* user);

chatroom_t* createChatRoom(char* label, user_t* admin);

int removeUser(chatroom_t* room, user_t* user);

int moveFromTo(chatroom_t* from, chatroom_t* to, user_t* usr);

int addChatRoomToList(chatroomList_t* list, chatroom_t* room);

int removeChatRoomFromList(chatroomList_t* list, chatroom_t* room);

chatroom_t* roomExists(chatroomList_t*, char*);

int countRooms(chatroomList_t* list);


#endif
