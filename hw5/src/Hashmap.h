#ifndef Hashmap_h
#define Hashmap_h

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define TRUE 1
#define FALSE 0

#define CHATROOM 23
#define USER 54

#define DEFAULT_NUMBER_NODE 257


/*
*Void pointer key, point to the key of the hash node
*Void pointer value, point to the value in the hash node
*uint32_t index number
*/
typedef struct _HashNode{
	//the key
	void *key;
	//value that linked with the key
	void *value;

}HashNode;

/*
*Define the hash map with node type Darray
*The dynamic array set to size of 257
*Comparision function that the hashmap useto find elements by their key
*Hash function take a key and producting a single uint32_t index number
*/
typedef struct _Hashmap{
	//size of the hash map
	int Hashmap_size;
	// used size
	int used_size;
	//the position
	HashNode *node;

	int type;
}Hashmap;


/*Hashmap functions */
//remove map
void Hashmap_remove_map(Hashmap *map);
//set
void Hashmap_set(Hashmap *map, void *RorU_name, void *value);
//get
void Hashmap_get(Hashmap *map, void *RorU_name);
//remove node
void Hashmap_remove_node(Hashmap *map, void *RorU_name);
//create
Hashmap *Hashmap_create();
//hash function
uint32_t hash_function(char *str);
//check if User/Room name is taken
int isTaken(Hashmap *map,char* Name);
//Free all data in hasmap properly
void Hashmap_free(Hashmap* map);

#endif