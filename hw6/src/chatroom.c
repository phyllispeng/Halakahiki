#include "chatroom.h"


void broadcastToRoom(chatroom_t* room, char* out_buff) {

    user_t* userCursor;
    for (userCursor = room->userHead.lh_first; userCursor != NULL; userCursor = userCursor->entry.le_next) {

        Send(userCursor->socket, out_buff, strlen(out_buff));

    }
}

chatroom_t* createChatroom(char* label, char* salt, char* hashedPass, int type) {

    chatroom_t* newRoom = calloc(1, sizeof (struct chatroom_t));
    newRoom->name = strdup(label);
    newRoom->chatroomID = hashString(label) % DEFAULT_NUMBER_NODE;

    newRoom->userCount = 0;
    LIST_INIT(&newRoom->userHead);
    
    newRoom->fileCount=0;
    LIST_INIT(&newRoom->files);

    if(type == PRIVATE_TYPE){
        newRoom->roomType = PRIVATE_TYPE;
        newRoom->salt = strdup(salt);
        newRoom->hashedPass = strdup(hashedPass);
        
    }else{
        if(type == PUBLIC_TYPE){
            newRoom->roomType = PUBLIC_TYPE;
        }else{
            //invalid chatroom creation attempt
        }
    }
    
    

    return newRoom;
}

void moveFromTo(chatroom_t* fromRoom, chatroom_t* toRoom, user_t* user) {

    removeUser(fromRoom, user);
    addUser(toRoom, user);
}

void removeUser(chatroom_t* room, user_t* user) {

    // Remove from doubly-linked list
    LIST_REMOVE(user, entry);

    // Update user count
    room->userCount--;
    if (room->userCount == 0) {
        destroyRoom(room);
    }

    // Reassign administrator 
    if (room->admin == user) {
        if (user->entry.le_next != NULL) {
            room->admin = user->entry.le_next;
        } else if (user->entry.le_prev != NULL) {
            room->admin = *user->entry.le_prev;
        }
    }
}

void addUser(chatroom_t* room, user_t* user) {

    LIST_INSERT_HEAD(&room->userHead, user, entry);

    if (room->userCount == 0) {
        room->admin = user;
    }

    room->userCount++;
}

void destroyRoom(chatroom_t* room) {

    LIST_REMOVE(room, entry);
}

user_t* getUserByName(chatroom_t* room, char* name) {

    user_t* userCursor;
    for (userCursor = room->userHead.lh_first; userCursor != NULL; userCursor = userCursor->entry.le_next) {

        if(strcmp(name, userCursor->name) == 0){
            return userCursor;
        }

    }
    
    return NULL;
}

file_info* getFileInfoById(chatroom_t* room, unsigned int id) {

    file_info* fileCursor;
    for (fileCursor = room->files.lh_first; fileCursor != NULL; fileCursor = fileCursor->entry.le_next) {

        if (id == fileCursor->file_id) {
            return fileCursor;
        }

    }

    return NULL;

}

void addFileToRoom(chatroom_t* room, file_info* file){
    LIST_INSERT_HEAD(&room->files, file, entry);
    room->fileCount++;
}

void removeFileFromRoom(chatroom_t* room, file_info* file){
    LIST_REMOVE(file, entry);
    room->fileCount--;
    //delete the file use  removeFile not error check
    char* fName = file->fileName;
    
    int remove_result = remove(fName);
        if (remove_result != 0) {
            printf("%s\n", "Something happened, you cannot remove it");
        } else {
            
            printf("%s\n", "File removed successfully");
        }
}


void sendPETAERC(user_t* user, chatroom_t* newRoom){
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, REV_CREATEP);
    strcat(out_buff, SPACE);
    strcat(out_buff, newRoom->name);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}

void sendRETAERC(user_t* user, chatroom_t* newRoom) {
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, REV_CREATER);
    strcat(out_buff, SPACE);
    strcat(out_buff, newRoom->name);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}