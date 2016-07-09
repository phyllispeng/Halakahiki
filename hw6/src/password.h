#ifndef password_h
#define password_h
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <openssl/ssl.h>
#include <openssl/rand.h> // -lssl -lcrypto
#include <openssl/sha.h>

bool valid_password(char* password);

void encryptPassword(char* password, unsigned char** ret);

void unsignPassword(char* password, unsigned char* passUnsign);

void decryptPassword(char* passAttempt, unsigned char* hash, unsigned char* salt);

bool checkPassword(char* passAttempt, unsigned char* hash, unsigned char* salt);

#endif


