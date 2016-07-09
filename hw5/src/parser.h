#ifndef parser_h
#define parser_h
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <arpa/inet.h>


uint32_t StringToInt_IP(char* ipStr);

void remove_newline(char* input);

uint64_t protocol_converter(char* input);

char* str_reverse(char* input);



#endif
