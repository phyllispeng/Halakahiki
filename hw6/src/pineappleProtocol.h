#ifndef pineappleProtocol_h
#define pineappleProtocol_h

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

#include "wrap.h"


enum verblen{
	SZ_ECHO 	= 4,
	SZ_MSG 		= 3,
	SZ_HI 		= 2,
	SZ_BYE 		= 3,
	SZ_IAM 		= 3,
	SZ_ERR 		= 3,
	SZ_ALOHA 	= 9,
	SZ_AHOLA 	= 9,
	SZ_KBYE		= 4
};

typedef enum errorCodes{
	SORRY 			= 00,
	USER_EXISTS 	=  1,
	DOES_NOT_EXIST 	=  2,
	ROOM_EXISTS 	= 10,
	MAX_ROOMS		= 11,
	ROOM_NOT_EXIST	= 20,
	USER_NOT_PRESENT= 30,
	NOT_OWNER		= 40,
	INVALID_USER 	= 41,
	INVALID_OPERA	= 60,
	INVALID_PASSWORD= 61,
	GEN_ERROR		=100
}errorCodes;

//Error Mssgs

#define ER_00	"SORRY "
#define ER_01p1	"USER "
#define ER_01p2	" EXISTS \r\n"
#define ER_02	"DOES NOT EXIST \r\n"
#define ER_10	"ROOM EXISTS \r\n"
#define ER_11	"MAXIUM ROOMS REACHED \r\n"
#define ER_20	"ROOM DOES NOT EXIST \r\n"
#define ER_30	"USER NOT PRESENT \r\n"
#define ER_40	"NOT OWNER \r\n"
#define ER_41	"INVALID USER \r\n"
#define ER_60	"INVALID OPERATION \r\n"
#define ER_61	"INVALID PASSWORD \r\n"

#define TRUE  1
#define FALSE 0
#define MSG_SIZE 1024

//part 1 verbs

#define   ECHO                        "ECHO"
#define   MSG                         "MSG"
#define   HI                          "HI"
#define   BYE                         "BYE"
#define   IAM                         "IAM"
#define   ERR                         "ERR"

#define	  SPACE						  " "
#define   DISCONNECTED 				  " has disconnected."
#define	  HASBEENCREATED			  " has been created."
#define	  JOINNOTIFY			  " has joined the room."
#define	  LEFTNOTIFY			  " has left the room."
#define   AVANOTIFY				  " is available for download."

#define   ALOHA                       "ALOHA!"
#define   AHOLA                       "!AHOLA"

#define   APPEND_END                  " \r\n"
#define   END_STRING                  "\r\n"

//part 2 verbs

#define   CREATER                     "CREATER"
#define   RETAERC                     "RETAERC"

#define   LISTR                       "LISTR"
#define   RTSIL                       "RTSIL"

#define   JOIN                        "JOIN"
#define   NIOJ                        "NIOJ"

#define   LEAVE                       "LEAVE"
#define   EVAEL                       "EVAEL"

#define   KICK                        "KICK"
#define   KCIK                        "KCIK"
#define   KBYE                        "KBYE"

#define   TELL                        "TELL"
#define   LLET                        "LLET"
#define   ECHOP                       "ECHOP"

#define   LISTU                       "LISTU"
#define   UTSIL                       "UTSIL"

//part 3 verbs
#define   IAMNEW                      "IAMNEW"
#define   HINEW                       "HINEW"
#define   NEWPASS                     "NEWPASS"
#define   AUTH                        "AUTH"
#define   PASS                        "PASS"

#define   CREATEP                     "CREATEP"
#define   PETAERC                     "PETAERC"

#define   JOINP                       "JOINP"
#define   PNIOJ                       "PNIOJ"

//also, modify LISTR for part 3 verbs

//HW5+

#define 	UPLOAD											"UPLOAD"
#define		DAOLPU											"DAOLPU"

#define		DOWNLOAD										"DOWNLOAD"
#define		DAOLNWOD 										"DAOLNWOD"

#define 	DELETE											"DELETE"
#define		ETELED											"ETELED"

#define		LISTF												"LISTF"
#define		FTSIL												"FTSIL"

#define		LISTFU											"LISTFU"
#define		UFTSIL											"UFTSIL"


#define   MAX_BUFFER_SIZE             1501
#define   MAX_BUFFER_SIZE_MINUS_1     1500

#define   ANSI_BLUE_INFORMATIVE           " \x1B[1;34m"
#define   ANSI_RED_ERRORS                 " \x1B[1;31m"
#define   ANSI_MAGENTA_PRIVATE_MESSAGES   " \x1B[1;35m"
#define   ANSI_RESET_DEFAULT              "\x1B[0m"

void messageTruncation(char* messageBuffer);

void* userNameRetrieval(int fileDescriptor);

void writeToServerFileDescriptor(char* stringToWrite, int fileDescriptor);

void clearBuffer(char* buffer, int size);

void echoFromServerToTerminal(char* messageFromServer);

//void echoFromServerToTerminal(char* messageFromServer, char* userInputBuffer);

void writeToTerminal(char* string, int newLine);

void readFromSTDIN_IntoBuffer_ToTerminal(char* rawClientInputBuffer_STDIN);

void generateERRorString(char* errorBuffer, int errorType, char* userName);


//keyboard MACROS


//global variables
int alohaholaWithServer;
int userConnected;
int userLoggedIn;
int chatRoomIDAsInt;
char* fileNameWithPath;









#endif
