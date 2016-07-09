#ifndef user_h
#define user_h

#include <stdlib.h>
#include "stdint.h"

#define TRUE 1
#define FALSE 0

typedef struct user_t{

	char* name;

	struct sockaddr_in* networkInfo;

	int socket;

	int isAdmin; //set it to 1 if the user is admin 0 if is not

	int isFileUploader;//set to 1 if user upload a file else 0

	uint32_t uniqueID; // hash id

}user_t;





user_t* createUser(char* name, int socket);

void set_admin(user_t* user);

void set_file_uploader(user_t* user);



#endif
