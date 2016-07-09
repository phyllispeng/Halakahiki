#include "user.h"



user_t* createUser(char* name, int Socket){
	user_t* user = malloc(sizeof(struct user_t));
	user->name = name;
	user->socket=Socket;
	return user;
}

void set_admin(user_t* user){
	user->isAdmin = 1;
}

void set_file_uploader(user_t* user){
	user->isFileUploader =1;
}
