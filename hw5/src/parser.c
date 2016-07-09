
#include "parser.h"




uint32_t StringToInt_IP(char* ipStr){
	struct in_addr addr;



	if(inet_aton(ipStr, &addr) == 0){
		printf("Invalid Ip address\n" );
		return -1;
	}

  //printf("addr: %" PRIx32 "\n", addr.s_addr);
	return addr.s_addr;

}
void remove_newline(char* input){

	if(input != NULL){
		int last = strlen(input)-1;
		if(input[last] == '\n'){

			input[last] = '\0';
		}

	}
}



uint64_t protocol_converter(char* input){
	if(input != NULL){
		uint64_t protocol;
		int int_protocol = atoi(input);
		protocol = (uint64_t) int_protocol;
		return protocol;
	}
	return 0;
}

char* str_reverse(char* input){
	char temp;
	size_t last= strlen(input)-1;
	int i;
	int k = last;


		 for (i=0; i < last; i++){

					 temp = input[k]; // put the last letter in the buffer
					 input[k] = input[i]; // put the first char in the position of the last char
					 input[i] = temp;	// put the last char which was in the buffer into the first char's position
					 k--;
					 if (k== (last/2)){
						 break;
					 }
		 }
		 return input;
}
