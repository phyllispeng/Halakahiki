#include "parser.h"


void truncstr(char* str, int charsToRemove){

	str[strlen(str)-charsToRemove]='\0';

}

char* reverse(char* string){

	char* str = strdup(string);

	int length, c;
	char *begin, *end, temp;

	length = strlen(str);
	begin  = str;
	end    = str;

	for (c = 0; c < length - 1; c++)
		end++;

	for (c = 0; c < length/2; c++){        
		temp   = *end;
		*end   = *begin;
		*begin = temp;

		begin++;
		end--;
	}

	return str;
}

#define DEFAULT_NUMBER_NODE 257

unsigned int hashString(char* string){
	unsigned int c ;
	unsigned int hash = DEFAULT_NUMBER_NODE;

    while ((c=*string++)){

        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		}
    return hash;
}