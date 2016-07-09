#include "chatroom.h"

chatroom_t* createChatRoom(char* label, user_t* admin){
	chatroom_t* newRoom = calloc(1, sizeof(struct chatroom_t));
	newRoom->name = label;
	newRoom->admin = admin;
	newRoom->chatRoomID = hash_function(label)%DEFAULT_NUMBER_NODE;
	newRoom->numOfUsers = 0;
	newRoom->users = calloc(1, sizeof(usersList));
	newRoom->users->usr = admin;
	newRoom->numOfUsers++;
	return newRoom;
}

int addChatRoomToList(chatroomList_t* list, chatroom_t* room){
	int ret = 0;

	chatroomList_t* cursor = list;
	while(cursor!=NULL){

		if(cursor->cr == NULL){
			cursor->cr = room;
			ret = 1;
			break;
		}

		if(cursor->next==NULL){
			cursor->next= calloc(1, sizeof(chatroomList_t));
			cursor->next->cr = room;
			ret = 1;
			break;
		}
		cursor = cursor->next;
	}

	return ret;
}

int removeChatRoomFromList(chatroomList_t* list, chatroom_t* room){
	int ret = 0;

	chatroomList_t* cursor = list;
	while(cursor!=NULL){
		if(cursor->cr == room){
			cursor->cr = NULL;
			//free(room);
			ret = 1;
			break;
		}
		cursor = cursor->next;
	}

	return ret;
}

int addUser(chatroom_t* room, user_t* user){

	usersList* cursor = room->users;
	while(cursor != NULL){
		if(cursor->usr == NULL){
			cursor->usr = user;
			break;
		}else if(cursor->next == NULL){
			cursor->next = calloc(1,sizeof(usersList));
			cursor->next->usr = NULL;
		}
		cursor = cursor->next;
	}
	
	room->numOfUsers++;

	return 1;
}

int removeUser(chatroom_t* room, user_t* user){

	
	if(user == room->admin){
		usersList* cursor = room->users;
		while(cursor != NULL){
			if(cursor->usr != user){
				room->admin = cursor->usr;
				break;
			}
			cursor = cursor->next;
		}
	}

	usersList* cursor = room->users;
	while(cursor != NULL){
		if(cursor->usr == user){
			cursor->usr = NULL;
			break;
		}
		cursor = cursor->next;
	}

	room->numOfUsers--;
	if(room->numOfUsers == 0){
		return EMPTY_ROOM-1;
	}

	return 1;

}

int moveFromTo(chatroom_t* from, chatroom_t* to, user_t* usr){

	int ret = removeUser(from, usr);
	addUser(to, usr);

	return ret;
}

chatroom_t* roomExists(chatroomList_t* list, char* checkName){

	chatroomList_t* cursor = list;
	while(cursor!=NULL){
		if(strcmp(checkName, cursor->cr->name) == 0){
			return cursor->cr;
		}
		cursor = cursor->next;
	}

	return NULL;

}

int countRooms(chatroomList_t* list){
	int ret = 0;
	chatroomList_t* cursor = list;
	while(cursor!=NULL){
		if(cursor->cr != NULL){
			ret++;
		}
		cursor = cursor->next;
	}
	return ret;
}