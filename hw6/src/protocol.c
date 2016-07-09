#include "protocol.h"

bool isALOHA(char* buf) {
    bool ret;

    if (strcmp(buf, ALOHA) == 0) {
        ret = true;
    } else {
        ret = false;
    }

    return ret;
}

bool isECHO(char* buf) {

    if (buf[0] != 'E')return false;
    if (buf[1] != 'C')return false;
    if (buf[2] != 'H')return false;
    if (buf[3] != 'O')return false;
    if (buf[4] != ' ')return false;

    char* temp = &buf[5];
    while (*temp != ' ')
        temp++;

    if (isalnum(*(++temp)) == 0) {
        return false;
    }

    while (*temp != ' ')
        temp++;

    if (*(++temp) == '\r') {
        return true;
    } else {
        return false;
    }


}

void sendError(user_t* user, errorCode errorType) {
    if(user==NULL){
        return;
    }
    char* userName = user->name;
    int socket = user->socket;

    //somehow include file descriptor/user name as a parameter; perhaps as part of some struct?
    //user name can be NULL
    char errorBuffer[MSG_SIZE];

    //first, clear the buffer
    memset(errorBuffer, 0, strlen(errorBuffer));
    strcat(errorBuffer, ERR);
    strcat(errorBuffer, ANSI_RED_ERRORS); //includes a starting space
    //may write ">server ERR" if desired

    //if userName == NULL, then cannot have errors 0, 1, and 2
    switch (errorType) {
        case SORRY:
            strcat(errorBuffer, "00 SORRY ");
            strcat(errorBuffer, userName);
            break;
        case USER_EXISTS:
            strcat(errorBuffer, "01 USER ");
            strcat(errorBuffer, userName);
            strcat(errorBuffer, " EXISTS");
            break;
        case DOES_NOT_EXIST:
            strcat(errorBuffer, "02 ");
            strcat(errorBuffer, userName);
            strcat(errorBuffer, " DOES NOT EXIST");
            break;
        case ROOM_EXISTS:
            strcat(errorBuffer, "10 ROOMEXISTS");
            break;
        case MAX_ROOMS:
            strcat(errorBuffer, "11 MAXIMUM ROOMS REACHED");
            break;
        case ROOM_NOT_EXIST:
            strcat(errorBuffer, "20 ROOM DOES NOT EXIST");
            break;
        case USER_NOT_PRESENT:
            strcat(errorBuffer, "30 USER NOT PRESENT");
            break;
        case NOT_OWNER:
            strcat(errorBuffer, "40 NOT OWNER");
            break;
        case INVALID_USER:
            strcat(errorBuffer, "41 INVALIDUSER");
            break;
        case INVALID_OPERA:
            strcat(errorBuffer, "60 INVALID OPERATION");
            break;
        case INVALID_PASSWORD:
            strcat(errorBuffer, "61 INVALID PASSWORD");
            break;
        case FILE_LIMIT_REACHED:
            strcat(errorBuffer, "70 FILE LIMIT REACHED");
            break;
        case FILE_SIZE_EXCEED:
            strcat(errorBuffer, "71 FILE EXCEEDS MAX SIZE");
            break;
        case FILE_DOES_NOT_EX:
            strcat(errorBuffer, "72 FILE DOES NOT EXIST");
            break;
        default:
            strcat(errorBuffer, "100 INTERNAL SERVER ERROR");
            break;
    }
    strcat(errorBuffer, ANSI_RESET_DEFAULT);
    strcat(errorBuffer, APPEND_END);

    Send(socket, errorBuffer, strlen(errorBuffer));

    return;

    /*
    ERR Code Message
		
    00 SORRY <name>
    01 USER <name> EXISTS
    02 <name> DOES NOT EXIST
    10 ROOMEXISTS
    11 MAXIMUM ROOMS REACHED
    20 ROOM DOES NOT EXIST
    30 USER NOT PRESENT
    40 NOT OWNER
    41 INVALIDUSER
    60 INVALID OPERATION
    61 INVALID PASSWORD

    The default error message for any other conditions
    which were not characterized should be:
    ERR 100 INTERNAL SERVER ERROR
     */

}

void sendAHOLA(user_t* user) {

    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, REV_ALOHA);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff)); // Send !AHOLA
}

void SendHINEW(user_t* user) {
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, HINEW);
    strcat(out_buff, SPACE);
    strcat(out_buff, user->name);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}

void sendBYE(user_t* user) {
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, BYE);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}

void sendHI(user_t* user) {
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, HI);
    strcat(out_buff, SPACE);
    strcat(out_buff, user->name);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}

void sendMessageOfTheDay(user_t* user, char* motd) {
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, ECHO);
    strcat(out_buff, SPACE);
    strcat(out_buff, "server");
    strcat(out_buff, SPACE);
    strcat(out_buff, motd);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}

void sendAUTH(user_t* user) {
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, AUTH);
    strcat(out_buff, SPACE);
    strcat(out_buff, user->name);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}

int isNewOrExisting(char* inStr) {

    char iambuff[7];
    memset(iambuff, 0, 7);

    for (int i = 0; i < 6; ++i) {
        char temp = inStr[i];
        if (temp == ' ') {
            break;
        }
        iambuff[i] = temp;
    }

    if (strcmp(IAM, iambuff) == 0) {
        return EXISTS;
    } else if (strcmp(IAMNEW, iambuff) == 0) {
        return NEW;
    } else {
        return BAD;
    }


}

void sendEVAEL(user_t* user) {
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, REV_LEAVE);
    strcat(out_buff, SPACE);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}

void sendNOIJ(user_t* user, int id) {
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, REV_JOIN);
    strcat(out_buff, SPACE);
    char ids[70];
    sprintf(ids, "%d", id);
    strcat(out_buff, ids);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}

void sendKBYE(user_t* user){
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, KBYE);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}
    
void sendKCIK(user_t* user, user_t* userToKick){
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, REV_KICK);
    strcat(out_buff, SPACE);
    strcat(out_buff, userToKick->name);
    strcat(out_buff, APPEND_END);
    Send(userToKick->socket, out_buff, strlen(out_buff));
}

void sendLLET(user_t* user, char* nameToTell, char* messageToSend){
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, REV_TELL);
    strcat(out_buff, SPACE);
    strcat(out_buff, nameToTell);
    strcat(out_buff, SPACE);
    strcat(out_buff, messageToSend);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}
    
void sendECHOP(user_t* user, char* nameFrom, char* messageToSend){
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, ECHOP);
    strcat(out_buff, SPACE);
    strcat(out_buff, nameFrom);
    strcat(out_buff, SPACE);
    strcat(out_buff, messageToSend);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}

void sendPNOIJ(user_t* user, int id){
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, REV_JOINP);
    strcat(out_buff, SPACE);
    char ids[70];
    sprintf(ids, "%d", id);
    strcat(out_buff, ids);
    strcat(out_buff, APPEND_END);
    Send(user->socket, out_buff, strlen(out_buff));
}

void generateERRorStringPhyllis(char* errorBuffer, int errorType, char* userName){
    //somehow include fd/username as a parameter; perhaps as part of some struct?
    //username can be NULL

    if(errorBuffer != NULL){
      //first, clear the buffer
      memset(errorBuffer, 0,strlen(errorBuffer));
      strcat(errorBuffer, ERR);
      strcat(errorBuffer, ANSI_RED_ERRORS);  //includes a starting space
      //may write ">server ERR" if desired
    }else{
      return; //set errno?
    }

    if(userName != NULL){
      //if userName == NULL, then cannot have errors 0, 1, and 2
      switch (errorType) {
        case 0:
                strcat(errorBuffer, "00 SORRY ");
                strcat(errorBuffer, userName);
                break;
        case 1:
                strcat(errorBuffer, "01 USER ");
                strcat(errorBuffer, userName);
                strcat(errorBuffer, " EXISTS");
                break;
        case 2:
                strcat(errorBuffer, "02 ");
                strcat(errorBuffer, userName);
                strcat(errorBuffer, " DOES NOT EXIST");
                break;
        default:
                strcat(errorBuffer, "100 INTERNAL SERVER ERROR");
                break;
      }
    }else{
      switch (errorType){
        case 10:
                strcat(errorBuffer, "10 ROOMEXISTS");
                break;
        case 11:
                strcat(errorBuffer, "11 MAXIMUM ROOMS REACHED");
                break;
        case 20:
                strcat(errorBuffer, "20 ROOM DOES NOT EXIST");
                break;
        case 30:
                strcat(errorBuffer, "30 USER NOT PRESENT");
                break;
        case 40:
                strcat(errorBuffer, "40 NOT OWNER");
                break;
        case 41:
                strcat(errorBuffer, "41 INVALIDUSER");
                break;
        case 60:
                strcat(errorBuffer, "60 INVALID OPERATION");
                break;
        case 61:
                strcat(errorBuffer, "61 INVALID PASSWORD");
                break;
        default:
                strcat(errorBuffer, "100 INTERNAL SERVER ERROR");
                break;
      }
    }
    strcat(errorBuffer, ANSI_RESET_DEFAULT);
    strcat(errorBuffer, APPEND_END);
    return;

    /*
    ERR Code Message
    ￼￼￼￼￼￼
    ￼￼00 SORRY <name>
    01 USER <name> EXISTS
    02 <name> DOES NOT EXIST
    10 ROOMEXISTS
    11 MAXIMUM ROOMS REACHED
    20 ROOM DOES NOT EXIST
    30 USER NOT PRESENT
    40 NOT OWNER
    41 INVALIDUSER
    60 INVALID OPERATION
    61 INVALID PASSWORD

    The default error message for any other conditions
    which were not characterized should be:
    ERR 100 INTERNAL SERVER ERROR
    */
}