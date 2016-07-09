#include "list.h"

list* items;

void init_list(){
	items = malloc(sizeof(struct list));
}

void insert(user_t* thing){
	user_t* ptr;
	for (int i = 0; i < SIZE; ++i){
		if((ptr = items->arr[i]) == NULL){
			ptr = thing;
		}
	}
}

void remove(user_t* thing){
	user_t* ptr;
	for (int i = 0; i < SIZE; ++i){
		if((ptr = items->arr[i]) != NULL
			&& ptr->uniqueID == thing->uniqueID){
			ptr=NULL;
		}
	}
}

list* getlist(){
	return items;
}