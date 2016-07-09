#include "client.h"
#include "pineappleProtocol.h"




void generateResponse_ServerSide(char* commandString, char* responseBuffer, int connectionFileDescriptor){

  /*
  if (commandString == NULL){
    //there was an error, so need to deal with error code
  }

  if(responseBuffer == NULL){
    //there was an error, so need to deal with error code
  }

  if(userInfo == NULL){
    //there was an error, so need to deal with error code
  }
  */

  clearBuffer(responseBuffer, MAX_BUFFER_SIZE);

  char* verb = strtok(commandString, " ");

  if(strcmp(verb, ALOHA) == 0){
    //Client initiates login to server by sending ALOHA! verb to the server.
    //Server receives ALOHA!
    //C > S | ALOHA! \r\n
    //Server responds with !AHOLA.
    //Client receives !AHOLA
    strcpy(responseBuffer, AHOLA);
    strcat(responseBuffer, APPEND_END);

    writeToServerFileDescriptor(responseBuffer, connectionFileDescriptor);

  }else if(strcmp(verb, IAM) == 0){

    /*
      C > S | IAM <name> \r\n
      # User account exists
      C < S | AUTH <name> \r\n
      # Client prompts user for password

      OR

      C > S | IAM <name> \r\n
      # User already logged in with user name
      # Reject the connection, and close.
      C < S | ERR 00 SORRY <name> \r\n
      C < S | BYE \r\n

      OR

      C > S | IAM <name> \r\n
      # User does not exist in the list of users
      # Reject the connection, and close.
      C < S | ERR 02 <message>\r\n
      C < S | BYE \r\n




    */

  }else if(strcmp(verb, MSG) == 0){
    //the client DOES NOT indicate the user with the MSG verb
    //Echo thread accesses shared resource (user list) to associate usernames with fd's
    //Echo thread then ECHOs to all connected clients
    //C < S | ECHO <username> <message> \r\n

    //userNameRetrieval(fileDescriptor);
    //


  }else if (strcmp(verb, PASS) == 0){

    //C > S | PASS <password> \r\n

    //[evaluate password here]

    //if successful:
        //C<S|HI <name>\r\n
    //else:
        //￼C < S | ERR 61 <message> \r\n
        //C < S | BYE \r\n


  }else if (strcmp(verb, JOIN) == 0){

    /*
      # A successful request
      C > S | JOIN <id> \r\n
      C < S | NIOJ <id> \r\n
      # Server echos to all clients in the room
      C < S | ECHO server <username> has joined the room.

      OR

      # Failed request; Room does not exist.
      C > S | JOIN <id> \r\n
      C < S | ERR 20 <message> \r\n

      OR

      # Failed request; Already in a room
      C > S | JOIN <id> \r\n
      C < S | ERR 60 <message> \r\n

    */


  }else if(strcmp(verb, KICK) == 0){
    //write KBYE to file descriptor of given username
    //kick out given user by first messaging kbye and then close the connection
    //# Echo to all users in the room
    //C < S | ECHO server <username> has been kicked out.

    /*
      # A successful request
      C > S | KICK <username> \r\n
      C < S | KCIK <username> \r\n
      # To client who was kicked out
      C < S | KBYE
      # Echo to all users in the room
      C < S | ECHO server <username> has been kicked out.

      OR

      # Failed request; not owner
      C > S | KICK <username> \r\n
      C < S | ERR 40 <message> \r\n

      OR

      # Failed request; invalid username.
      C > S | KICK <username> \r\n
      C < S | ERR 41 <message> \r\n
    */

  }else if (strcmp(verb, LEAVE) == 0){

    /*
      # A successful request
      C > S | LEAVE \r\n
      C < S | EVAEL \r\n
      # Echo to all users in the room
      C < S | ECHO server <username> has left the room.

      OR

      # A successful request; no owner
      C > S | LEAVE \r\n
      C < S | EVAEL \r\n
      # Echo to all users in the room
      ￼C < S | ECHO server <username> has left the room.
      C < S | ECHO server <username> has been promoted to owner.

      # Failed request; Client is not in a room.
      C > S | LEAVE \r\n
      C < S | ERR 30 <message> \r\n

    */



  }else if(strcmp(verb, BYE) == 0){
      /* C > S | BYE \r\n
        C < S | BYE \r\n
        # Server closes fd for that client
        # ECHO to all connected clients
        C < S | ECHO server <username> has disconnected. \r\n
      */


  }else if(strcmp(verb, LISTR) == 0){

    /*
        # A successful request; with rooms
        C > S | LISTR \r\n
        C < S | RTSIL <room1> <id1> \r\n <room2> <id2> \r\n <room3> <id3> \r\n\r\n
        # A successful request; no existing rooms
        C > S | LISTR \r\n
        C < S | RTSIL no_rooms -1 \r\n\r\n
        # Failed request; Not in waiting room
        C > S | LISTR \r\n
        C < S | ERR 60 <message> \r\n
    */

  }else if(strcmp(verb, CREATER) == 0){

    /*
    # A successful request
    C > S | CREATER <name> \r\n
    C < S | RETAERC <name> \r\n
    # Inform all user in waiting room
    C < S | ECHO server New Chat room added.

    OR

    # Failed request; Room with the same name
    C > S | CREATER <name> \r\n
    C < S | ERR 10 <message> \r\n

    OR

    # Failed request; Maximum number of rooms exist
    C > S | CREATER <name> \r\n
    ￼￼C < S | ERR 11 <message> \r\n

    */


  }else if(strcmp(verb, TELL) == 0){

    /*
      # A successful request
      C > S | TELL <name> <message> \r\n
      C < S | LLET <name> <message> \r\n
      # Echo to user <name> the message
      C < S | ECHOP <sendername> <message>

      OR

      # Failed request; No user with name in room
      C > S | TELL <name> <message> \r\n
      C < S | ERR 30 <message> \r\n

    */

  }else if(strcmp(verb, LISTU) == 0){

    /*
    # A successful request
    C > S | LISTU \r\n
    C < S | UTSIL <name1> \r\n<name2> \r\n<name3> \r\n\r\n

    OR

    # Failed request; Not in room
    C > S | LISTU \r\n
    C < S | ERR 60 <message> \r\n

    */


  }else if(strcmp(verb, CREATEP) == 0){

    /*
        # A successful request
        C > S | CREATEP <name> <password> \r\n
        # Server creates the room and adds user
        C < S | PETAERC <name> \r\n
        # Inform all user in waiting room
        C < S | ECHO server New Private Chat room added.

        OR

        # Failed request; User currently in a chat room
        C > S | CREATEP <name> <password> \r\n
        C < S | ERR 60 <message> \r\n

        OR

        # Failed request; Chat room with name already exists
        C > S | CREATEP <name> <password> \r\n
        C < S | ERR 10 <message> \r\n
        ￼￼￼
        OR

        ￼# Failed request; Password not valid
        C > S | CREATEP <name> <password> \r\n
        C < S | ERR 61 <message> \r\n

        OR

        # Failed request; Maximum number of rooms exist
        C > S | CREATEP <name> <password> \r\n
        C < S | ERR 11 <message> \r\n

    */



  }else if(strcmp(verb, IAMNEW) == 0){
    //create struct with new user if doesn't exist
    //change flag that will process as new user

    /*
      C > S | IAMNEW <name> \r\n
      # User name already exists in the user list/database
      # Reject the connection, and close.
      C < S | ERR 01 SORRY <name> \r\n
      C < S | BYE \r\n

      OR

      C > S | IAMNEW <name> \r\n
      C < S | HINEW <name> \r\n
      # Client prompts user for password
      C > S | NEWPASS <password> \r\n
      # Failed request; Password not valid C < S | ERR 61 <message> \r\n
      C < S | BYE \r\n


    */



  }else if(strcmp(verb, NEWPASS) == 0){
    //check flag that currently processing this user

    /*
    C > S | NEWPASS <password> \r\n
    # Failed request; Password not valid
    C < S | ERR 61 <message> \r\n
    C < S | BYE \r\n

    OR
    # On successful user creation
    C<S|HI <name> \r\n
    */
  }else{
    //shouldn't be here, so then call an ERR
  }
  return;
}









void generateERRorString(char* errorBuffer, int errorType, char* userName){
    //somehow include fd/username as a parameter; perhaps as part of some struct?
    //username can be NULL

    if(errorBuffer != NULL){
      //first, clear the buffer
      clearBuffer(errorBuffer, strlen(errorBuffer));
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


void generateInformativeString(char* messageBuffer){
    //ANSI_BLUE_INFORMATIVE           "\x1B[1;34m" <-length of 9
    //ANSI_RESET_DEFAULT              "\x1B[0m" <-length of 6
    //APPEND_END

}

void generatePrivateString(char* messageBuffer){
  //ANSI_MAGENTA_PRIVATE_MESSAGES   "\x1B[1;35m" <-length of 9
  //ANSI_RESET_DEFAULT              "\x1B[0m" <-length of 6
  //APPEND_END

}



/*
void messageTruncation(char* messageBuffer){
  //limits user communication to 1000 bytes
  //permits further processing
  char* messageAt1000 = &(messageBuffer[1000]);
  //0-999 are fine, but clear everything starting at 1000 until terminating NULL
  memset(messageAt1000, 0, (MAX_BUFFER_SIZE - 1000));
  //for example, if MAX_BUFFER_SIZE were 1015, clears positions 1000-1014
} //void messageTruncation(char* messageBuffer)
*/

void clearBuffer(char* buffer, int size){
  memset(buffer, 0, size);
}


void* userNameRetrieval(int fileDescriptor){
  //if fd exists: return char* of the user's name
  //else: return NULL
  return NULL;
}/* void* userNameRetrieval(int fileDescriptor) */


void echoFromServerToTerminal(char* messageFromServer){
//void echoFromServerToTerminal(char* messageFromServer, char* userInput)
    Write(STDOUT_FILENO, messageFromServer, strlen(messageFromServer));
}
/*
void echoFromServerToTerminal(char* messageFromServer, char* userInputBuffer){
//void echoFromServerToTerminal(char* messageFromServer, char* userInput)
    writeToTerminal(messageFromServer, 1); //assumes the message doesn't have a new line at the end
    writeToTerminal(userInputBuffer, 0);   //rewrite userinput
}
*/

void writeUserInputToTerminal_SameLine(char* inputBuffer){
    writeToTerminal(inputBuffer, 0);
}


void readFromSTDIN_IntoBuffer_ToTerminal(char* rawClientInputBuffer_STDIN){

    int readReturnVal = Read(STDIN_FILENO, rawClientInputBuffer_STDIN, MAX_BUFFER_SIZE_MINUS_1);

    if(readReturnVal != -1){
      printf("%s", rawClientInputBuffer_STDIN);
      fflush(stdout);
    }
    return;
}



void writeToTerminal(char* string, int newLine){
  if(newLine == 1){
    //new line
    Write(STDOUT_FILENO, "\r                                                                                           ", 91);
    Write(STDOUT_FILENO, string, strlen(string));
    Write(STDOUT_FILENO, "\n", 1);
  }else{
    Write(STDOUT_FILENO, "\r                                                                                           ", 91);
    Write(STDOUT_FILENO, string, strlen(string));
  }
  return;
}


void writeToServerFileDescriptor(char* stringToWrite, int fileDescriptor){

  Write(fileDescriptor, stringToWrite, strlen(stringToWrite));

  return;
}






  //server accepts a connection from a client
  //server blocks other clients from connecting
  //server spawns thread to handle login protocol

  //Client initiates login to server by sending ALOHA! verb to the server.
  //Server receives ALOHA!
  //C > S | ALOHA! \r\n

  //Server responds with !AHOLA.
  //Client receives !AHOLA
  //C < S | !AHOLA \r\n

  //Client responds using the IAM verb.
  //C > S | IAM <name> \r\n # <name> is user's name


  //Server should validate the username.

  //server checks the list of usernames currently in use
      //For part one, all user names are unique.
      //if name not in use (and password matches in later parts):
          //we are undergoing a successful login
          //Server creates new socket to communicate on.

          //we are going to check the echo flag
          //if echo flag == 0
              //echo flag needs to be set to 1
          //else, echo flag was 1 and echo thread is already running, so don't change it

          //ECHO message to all connected clients (if they exist): a new user connected.
          //C < S | ECHO server <username> has connected.

          //Server sends HI verb confirming that logging in was successful
          //C < S | HI <name> \r\n

          //Server ECHOs message of the day (MOTD) to newly connected client.
          //C < S | ECHO server <MOTD>

      //else:
        //there is already a user with that name connected

        //the server rejects the connection attempt by
        //sending the ERR verb with an associated error code and message.
        //C < S | ERR 00 SORRY <name> \r\n

        //Server sends BYE verb and terminates the connection.
        //C < S | BYE \r\n

  //??login thread completes/terminates
  //remove the block preventing other clients from connecting




//Echo Thread:
//upon logging out or somehow disconnecting:
//
  //server checks if users == 0
      //then echo flag is set to zero
      //terminate echo thread




//Communicating after logging into the server
//ECHO: the echo thread handles all communication between the client and server

//Use the MSG verb to message everyone on the server.
//C > S | MSG <message> \r\n # A message can be a max length of 1000 bytes
    //# So the max size of your buffer should be 1001 (null terminator)
    //Any characters in excess of 1000 bytes should be truncated.

//the client DOES NOT indicate the user with the MSG verb
//Echo thread accesses shared resource (user list) to associate usernames with fd's

//Echo thread then ECHOs to all connected clients
//C < S | ECHO <username> <message> \r\n



//Logging out of the server
//The client implements the command "/quit"
//Client then initiates the logging out procedure.
    //Client sends BYE verb to the server.
    //C > S | BYE \r\n
    //Echo server thread responds to the client by saying BYE.
    //C < S | BYE \r\n
    //Server closes connection with the client
        //"cleans up" the user
            //at a *MINIMUM*, the username is removed from *CURRENT* user list
        //decrements count of connected users
    //?Client closes connection with server
    //Serve echo thread ECHOs to all clients
    //C < S | ECHO server <username> has disconnected. \r\n












//
