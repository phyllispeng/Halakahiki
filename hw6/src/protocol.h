#ifndef protocol_h
#define protocol_h

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "chatroom.h"
#include "user.h"
#include "wrap.h"
#include "parser.h"

typedef enum errorCodes {
    SORRY = 00,
    USER_EXISTS = 1,
    DOES_NOT_EXIST = 2,
    ROOM_EXISTS = 10,
    MAX_ROOMS = 11,
    ROOM_NOT_EXIST = 20,
    USER_NOT_PRESENT = 30,
    NOT_OWNER = 40,
    INVALID_USER = 41,
    INVALID_OPERA = 60,
    INVALID_PASSWORD = 61,
    FILE_LIMIT_REACHED = 70,
    FILE_SIZE_EXCEED = 71,
    FILE_DOES_NOT_EX =72,
    FILE_NO_PERMISSION = 72,
    GEN_ERROR = 100
} errorCode;

enum iam {
    NEW,
    EXISTS,
    BAD
};

#define PRIVATE_TYPE 2
#define PUBLIC_TYPE 1

#define   DISCONNECTED     " has disconnected."
#define   PRIVATECREATED   " private chatroom has been created."
#define   HASBEENCREATED   " has been created."
#define   JOINNOTIFY     " has joined the room."
#define   LEFTNOTIFY     " has left the room."
#define   AVANOTIFY      " is available for download."
#define   KICKNOTIFY     " was kick from the room."
#define  ALOHA      "ALOHA!"
#define  REV_ALOHA    "!AHOLA"

#define  ECHO     "ECHO"
#define  MSG      "MSG"
#define  HI      "HI"
#define  BYE      "BYE"
#define  IAM      "IAM"
#define  ERR      "ERR"
#define  SPACE     " "

#define   APPEND_END                  " \r\n"
#define   END_STRING                  "\r\n"

//part 2 verbs

#define  CREATER     "CREATER"
#define  LISTR     "LISTR"
#define  JOIN     "JOIN"
#define  LEAVE     "LEAVE"
#define  KICK     "KICK"
#define  KBYE     "KBYE"
#define  TELL     "TELL"
#define  ECHOP     "ECHOP"
#define  LISTU     "LISTU"

#define  REV_CREATER    "RETAERC"
#define  REV_LISTR    "RTSIL"
#define  REV_JOIN    "NIOJ"
#define  REV_LEAVE    "EVAEL"
#define  REV_KICK    "KCIK"
#define  REV_TELL    "LLET"
#define  REV_LISTU    "UTSIL"


//part 3 verbs
#define  IAMNEW     "IAMNEW"
#define  HINEW     "HINEW"
#define  NEWPASS     "NEWPASS"
#define  AUTH     "AUTH"
#define  PASS     "PASS"

#define  CREATEP     "CREATEP"
#define  REV_CREATEP    "PETAERC"


#define  JOINP     "JOINP"
#define  REV_JOINP    "PNIOJ"


#define  UPLOAD     "UPLOAD"
#define  DOWNLOAD    "DOWNLOAD"
#define  DELETE     "DELETE"
#define  LISTF     "LISTF"
#define  LISTFU     "LISTFU"

#define  REV_UPLOAD    "DAOLPU"
#define  REV_DOWNLOAD   "DAOLNWOD"
#define  REV_DELETE    "ETELED"
#define  REV_LISTF    "FTSIL"
#define  REV_LISTFU    "UFTSIL"

#define MSG_SIZE 2000

#define   ANSI_BLUE_INFORMATIVE           " \x1B[1;34m"
#define   ANSI_RED_ERRORS                 " \x1B[1;31m"
#define   ANSI_MAGENTA_PRIVATE_MESSAGES   " \x1B[1;35m"
#define   ANSI_RESET_DEFAULT              "\x1B[0m"

bool isALOHA(char* buf);

void sendError(user_t* user, errorCode errorType);

void sendAHOLA(user_t* user);

int isNewOrExisting(char* inStr);

void SendHINEW(user_t* user);

void sendBYE(user_t*);

void sendHI(user_t*);

void sendMessageOfTheDay(user_t* user, char* motd);

void sendAUTH(user_t* user);

bool isECHO(char* buf);



void sendEVAEL(user_t* user);

void sendNOIJ(user_t* user, int id);

void sendKBYE(user_t* originUser);

void sendKCIK(user_t* originUser, user_t* userToKick);

void sendLLET(user_t* originUser, char* nameToTell, char* messageToSend);

void sendECHOP(user_t* userToTell, char* nameFrom, char* messageToSend);



void sendPNOIJ(user_t*, int id);

void generateERRorStringPhyllis(char* errorBuffer, int errorType, char* userName);

#endif
