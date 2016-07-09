#ifndef list_h
#define list_h

#include <stdlib.h>
#include "user.h"

#define SIZE 100

typedef struct list{
	
	int size;

	user_t* arr[SIZE];
	
}list;

void remove(user_t* thing);
void insert(user_t* thing);
list* getlist();

#endif