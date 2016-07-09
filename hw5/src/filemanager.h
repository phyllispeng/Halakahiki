#ifndef filemanager_h
#define filemanager_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "user.h"
#include "chatroom.h"
#include <dirent.h>
#include <sys/socket.h>
#include <sys/mman.h>

#define TRUE 1
#define FALSE 0
#define MAX_IN 1024

typedef struct file_info{
  user_t* user;
  char* fileName;
  uint32_t file_id;
  chatroom_t* cm;

}file_info;

typedef struct file_list{
  struct file_list* next;
  file_info* finfo;
}file_list;


int isFileValid(char* fileName);

DIR* build_file_folder(char* folder);
void remove_file_folder(char* dir_path);
void recieve_file(int socket_num, char* fileName, chatroom_t* cm);
void send_file(int socket_num, int size, char* fileName);
void list_files();
void set_file_info(user_t* user, chatroom_t* cm, char* fileName);
void remove_file(user_t* usr, char* fName );
//user_t* get_file_holder(file_info* finfo, char* fileName);


#endif
