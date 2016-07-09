#include "Hashmap.h"


/*Hashmap functions */
//remove map
void Hashmap_remove_map(Hashmap *map){
	free(map->node);
	free(map);
}
//set
void Hashmap_set(Hashmap *map, void *RorU_name, void *value){

		uint32_t  curr = hash_function(RorU_name);
		curr=curr%DEFAULT_NUMBER_NODE;
		map->node[curr].key = RorU_name;
		map->node[curr].value = value;

		map->used_size++;
		printf("Added Username\n");

}
//get shoudl return the node or the map
void Hashmap_get(Hashmap *map, void *RorU_name){
	uint32_t curr = hash_function(RorU_name);
	curr=curr%DEFAULT_NUMBER_NODE;
	char* str = map->node[curr].key;
	if(str != NULL){	if(strcmp(str,RorU_name) == 0){
				printf("%s\n","find it..." );
			}else{
				printf("%s\n","wat..." );
			}
		}else{
			printf("%s\n","wat..., nothing is here" );
		}


}
//remove node
void Hashmap_remove_node(Hashmap *map, void *RorU_name){
	uint32_t curr = hash_function(RorU_name);
	curr=curr%DEFAULT_NUMBER_NODE;
	char* str = map->node[curr].key;
	if(str != NULL){
		if(strcmp(str, RorU_name)==0){
			free(map->node[curr].key);
			free(map->node[curr].value);

			map->node[curr].key=NULL;
			map->node[curr].value=NULL;
			map->used_size--;
		}else{
			printf("%s\n","invalid, variable dne" );
		}
	}else{
			printf("%s\n", "no variable in the node");
	}
}

int isTaken(Hashmap *map,char* Name){

	uint32_t curr = hash_function(Name);
	curr=curr%DEFAULT_NUMBER_NODE;
	char* str = map->node[curr].key;
	if(str != NULL){
		printf("%s\n","User/Room Name already exist" );
		return TRUE;
	}
		return FALSE;
}

//create
Hashmap *Hashmap_create(){
	Hashmap* hm = (Hashmap* ) malloc(sizeof(Hashmap));
	hm -> node = (HashNode* ) calloc (DEFAULT_NUMBER_NODE, sizeof(HashNode));

	hm -> Hashmap_size = DEFAULT_NUMBER_NODE;
	hm -> used_size = 0;

	return hm;

}

uint32_t hash_function(char *str){
    uint32_t c ;
		uint32_t hash = DEFAULT_NUMBER_NODE;
    while ((c=*str++)){

        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		}
    return hash;
}
