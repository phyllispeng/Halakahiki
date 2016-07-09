#ifndef user_h
#define user_h

#include <stdbool.h>
#include <sys/queue.h>
#include <stdint.h>


typedef struct user_t{

	char* name;

	char* networkInfo;

	int socket;

	bool isAdmin; //set it to 1 if the user is admin 0 if is not

	bool isFileUploader;//set to 1 if user upload a file else 0

	uint32_t uniqueID; // hash id
	
	LIST_ENTRY(user_t) entry;

}user_t;


#endif