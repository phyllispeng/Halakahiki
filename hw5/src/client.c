#include "client.h"



//Client main():
/*￼
./client [-h] NAME SERVER_IP SERVER_PORT
-h              Displays help menu & returns EXIT_SUCCESS.
NAME            Username to display when chatting.
SERVER_IP       IP address of the server to connect to.
SERVER_PORT     Port to connect to.
*/

#ifdef DEBUG
#define debug(fmt, ...) printf("DEBUG: %s:%s:%d " fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define debug(fmt, ...)
#endif




int main(int argc, char *argv[]){
	/* code */

	if(argc < 4){
		fprintf(stderr, "\n\nMissing Arguments!\n\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	int opt;
	int createNewUser = 0;

	while ((opt = getopt(argc, argv, "hc")) != -1) {
		switch (opt) {
			case 'h':
							USAGE;
							exit(EXIT_FAILURE);
							break;
			case 'c':
							createNewUser = 1;
							break;
			default: /* '?' */
							USAGE;
							exit(EXIT_FAILURE);
							break;
		} /* switch (opt) */
	} /* while ((opt = getopt(argc, argv, "h")) != -1) */

	//optind is the index in argv of the first argv-element not an option

	if(createNewUser == 1){
		fprintf(stdout, "Welcome! So you're a new user.\n");
		fflush(stdout);
	}

	int argvIndex = optind;
	char* userName = argv[argvIndex];

	if(strcasecmp(userName, "server") == 0){
			fprintf(stdout, "\x1B[1;31mInvalid Username.\x1B[0m\n");
			exit(EXIT_FAILURE);
	}

	argvIndex++;
	char* serverIP = argv[argvIndex];

	argvIndex++;
	char* serverPort = argv[argvIndex];

	struct addrinfo **myAddrInfo = calloc(1, sizeof(struct addrinfo));
	int getAddrSuccessVal = GetAddrInfo(serverIP, serverPort, NULL, myAddrInfo);

	if(getAddrSuccessVal != 0){
		Freeaddrinfo(*myAddrInfo);
		exit(EXIT_FAILURE);
	}

	int mySocketFileDescriptor = Socket(((*myAddrInfo)->ai_family), ((*myAddrInfo)->ai_socktype), ((*myAddrInfo)->ai_protocol));

	if(mySocketFileDescriptor == -1){
		Freeaddrinfo(*myAddrInfo);
		exit(EXIT_FAILURE);
	}

	int myConnectStatus = Connect(mySocketFileDescriptor, ((*myAddrInfo)->ai_addr), ((*myAddrInfo)->ai_addrlen));
	//pointer to socket address placed in ai_addr field, and its byte-length in ai_addrlen.

	//Upon successful completion, connect() shall return 0;
	//otherwise, -1 shall be returned and errno set to indicate the error.

	if(myConnectStatus == -1){
		Freeaddrinfo(*myAddrInfo);
		exit(EXIT_FAILURE);
	}

	//we successfully connected and the server is handling login protocol
	char *alohaVerb = "ALOHA! \r\n";
	Send(mySocketFileDescriptor, alohaVerb, 9, 0);
	//Client expects to receive "!AHOLA \r\n"
	char* readBuffer = calloc(MAX_BUFFER_SIZE, sizeof(char));

	//validate that it is AHOLA
	char*   outputBuffer    	= calloc(MAX_BUFFER_SIZE, sizeof(char));

	int recvSize = Recv(mySocketFileDescriptor, readBuffer, MAX_BUFFER_SIZE_MINUS_1, 0);
	if(recvSize == -1){
		quitGracefully(mySocketFileDescriptor, myAddrInfo, readBuffer, outputBuffer);
	}

	//readbuffer should have "!AHOLA \r\n"
	//if so, can generate a response for it

	//since creating a new user,
	//send IAMNEW username when receive AHOLA with new user
	//send IAM username when receive AHOLA  with previous user
	int imNewValidity = generateResponse_ClientSide(mySocketFileDescriptor, readBuffer, outputBuffer, userName, createNewUser);
	if(imNewValidity == 0){
		fprintf(stdout, "Failed to generate a valid response to the server.\n");
		fflush(stdout);
		quitGracefully(mySocketFileDescriptor, myAddrInfo, readBuffer, outputBuffer);

	}
	clearBuffer(readBuffer, MAX_BUFFER_SIZE);
	clearBuffer(outputBuffer, MAX_BUFFER_SIZE);

	if(createNewUser == 1){
			recvSize = Recv(mySocketFileDescriptor, readBuffer, MAX_BUFFER_SIZE_MINUS_1, 0);
			if(recvSize == -1){
				quitGracefully(mySocketFileDescriptor, myAddrInfo, readBuffer, outputBuffer);
			}
			//in readBuffer, expect we have:  HINEW username \r\n
			//pass it, which should prompt user for password
			//send NEWPASS
			int newPassValidity = generateResponse_ClientSide(mySocketFileDescriptor, readBuffer, outputBuffer, userName, 1);
			if(newPassValidity == 0){
				quitGracefully(mySocketFileDescriptor, myAddrInfo, readBuffer, outputBuffer);
			}
			clearBuffer(readBuffer, MAX_BUFFER_SIZE);
			clearBuffer(outputBuffer, MAX_BUFFER_SIZE);
			//should receive HI from the server
			recvSize = Recv(mySocketFileDescriptor, readBuffer, MAX_BUFFER_SIZE_MINUS_1, 0);
			if(recvSize == -1){
				quitGracefully(mySocketFileDescriptor, myAddrInfo, readBuffer, outputBuffer);
			}
			int hiValidity = generateResponse_ClientSide(mySocketFileDescriptor, readBuffer, outputBuffer, userName, 1);
			if(hiValidity == 0){
				quitGracefully(mySocketFileDescriptor, myAddrInfo, readBuffer, outputBuffer);
			}
		}//if(createNewUser == 1)
	else{
		recvSize = Recv(mySocketFileDescriptor, readBuffer, MAX_BUFFER_SIZE_MINUS_1, 0);
		if(recvSize == -1){
			quitGracefully(mySocketFileDescriptor, myAddrInfo, readBuffer, outputBuffer);
		}
		//in readBuffer, expect we have: AUTH name \r\n
		//pass it, which should prompt user for password
		//send PASS password
		int newPassValidity = generateResponse_ClientSide(mySocketFileDescriptor, readBuffer, outputBuffer, userName, 0);
		if(newPassValidity == 0){
			quitGracefully(mySocketFileDescriptor, myAddrInfo, readBuffer, outputBuffer);
		}
		clearBuffer(readBuffer, MAX_BUFFER_SIZE);
		clearBuffer(outputBuffer, MAX_BUFFER_SIZE);
		//should receive HI from the server
		recvSize = Recv(mySocketFileDescriptor, readBuffer, MAX_BUFFER_SIZE_MINUS_1, 0);
		if(recvSize == -1){
			quitGracefully(mySocketFileDescriptor, myAddrInfo, readBuffer, outputBuffer);
		}
		int hiValidity = generateResponse_ClientSide(mySocketFileDescriptor, readBuffer, outputBuffer, userName, 0);
		if(hiValidity == 0){
			quitGracefully(mySocketFileDescriptor, myAddrInfo, readBuffer, outputBuffer);
		}
	}

	clientHandler(mySocketFileDescriptor, readBuffer, userName);

	Close(mySocketFileDescriptor);
	Freeaddrinfo(*myAddrInfo);
	free(readBuffer);
	free(outputBuffer);
	exit(EXIT_SUCCESS);
}

void quitGracefully(int mySocketFileDescriptor, struct addrinfo **myAddrInfo, char* readBuffer, char* outputBuffer){
	fprintf(stderr, "Something went wrong. \n");
	fflush(stderr);
	Close(mySocketFileDescriptor);
	Freeaddrinfo(*myAddrInfo);
	free(readBuffer);
	free(outputBuffer);
	exit(EXIT_FAILURE);
}


void clientHandler(int connectionFileDescriptor, char* buffer, char* userName){

  size_t  bufferSize_STDIN                    = MAX_BUFFER_SIZE;
  char*   rawClientInputBuffer_STDIN          = calloc(bufferSize_STDIN, sizeof(char));
  char*   processedClientInputBuffer_STDIN    = calloc(bufferSize_STDIN, sizeof(char));

  size_t  bufferSize_SERVER                   = MAX_BUFFER_SIZE;
  char*   rawClientInputBuffer_SERVER         = calloc(bufferSize_SERVER, sizeof(char));
  char*   responseBuffer_SERVER               = calloc(bufferSize_SERVER, sizeof(char));

	fileNameWithPath	= calloc(MAX_BUFFER_SIZE, sizeof(char));

  fd_set read_set;
  fd_set ready_set;

  while(userConnected || userLoggedIn){

		FD_ZERO(&read_set);
	  FD_SET(STDIN_FILENO, &read_set);
	  FD_SET(connectionFileDescriptor, &read_set);

    ready_set = read_set;
    Select(connectionFileDescriptor+1, &ready_set, NULL);
    if(FD_ISSET(connectionFileDescriptor, &ready_set)){
      clientSRC_SERVER(connectionFileDescriptor, rawClientInputBuffer_SERVER, responseBuffer_SERVER, bufferSize_SERVER);
			FD_CLR(connectionFileDescriptor, &read_set);
    }
    if(FD_ISSET(STDIN_FILENO, &ready_set)){
      clientSRC_STDIN(connectionFileDescriptor, rawClientInputBuffer_STDIN, processedClientInputBuffer_STDIN, bufferSize_STDIN);
			FD_CLR(STDIN_FILENO, &read_set);
    }
		//FD_CLR(fd, fd_set *fdset);

  }

	fprintf(stdout, "Exiting...\r\n");

	free(rawClientInputBuffer_STDIN);
	free(processedClientInputBuffer_STDIN);

	free(rawClientInputBuffer_SERVER);
	free(responseBuffer_SERVER);

	free(fileNameWithPath);

	return;
}




int clientSRC_STDIN(int connectionFileDescriptor, char* rawClientInputBuffer_STDIN,
	char* processedClientInputBuffer_STDIN, size_t bufferSize_STDIN){

	clearBuffer(processedClientInputBuffer_STDIN, bufferSize_STDIN);
	clearBuffer(rawClientInputBuffer_STDIN, bufferSize_STDIN);

	//readFromSTDIN_IntoBuffer_ToTerminal(rawClientInputBuffer_STDIN);

	Read(STDIN_FILENO, rawClientInputBuffer_STDIN, MAX_BUFFER_SIZE_MINUS_1);

	if(rawClientInputBuffer_STDIN[strlen(rawClientInputBuffer_STDIN) -1] == '\n'){
		rawClientInputBuffer_STDIN[strlen(rawClientInputBuffer_STDIN) -1] = '\0';
	}

	int rawInputStartPosition = 0;


	char* intermediateBuffer = calloc(bufferSize_STDIN, sizeof(char));
	char* intermediateBufferPtr = strcpy(intermediateBuffer, &rawClientInputBuffer_STDIN[rawInputStartPosition]);

	if(intermediateBufferPtr[0] == '/'){
			//then we have a potential command
			char* tokenizedArgument = strtok(intermediateBufferPtr, " ");
			char possibleCommand[21];
			strncpy(possibleCommand, tokenizedArgument, 20);

			int argCount = 0;
			tokenizedArgument = strtok(NULL, " ");

			while((tokenizedArgument != NULL) && (argCount < 4)){
				argCount++;
				tokenizedArgument = strtok(NULL, " ");
			}

			if(strcmp(possibleCommand, "/quit") == 0){

					strcat(processedClientInputBuffer_STDIN, BYE);

			}else if(strcmp(possibleCommand, "/creater") == 0){

				if(argCount == 1){
					strcat(processedClientInputBuffer_STDIN, CREATER);
					strcat(processedClientInputBuffer_STDIN, &(rawClientInputBuffer_STDIN[rawInputStartPosition + 8]));
				}else{
					fprintf(stderr, "\x1B[1;31mInvalid number of arguments.\x1B[0m\r\n");
					fprintf(stderr, "\x1B[1;34m/creater <name>\x1B[0m\r\n");
					fflush(stderr);
					free(intermediateBuffer);
					return 0;
				}
			}else if(strcmp(possibleCommand, "/listrooms") == 0){

					strcat(processedClientInputBuffer_STDIN, LISTR);

			}else if(strcmp(possibleCommand, "/join") == 0){

				if(argCount == 1){
					strcat(processedClientInputBuffer_STDIN, JOIN);
					strcat(processedClientInputBuffer_STDIN, &(rawClientInputBuffer_STDIN[rawInputStartPosition + 5]));

					debug("what follows the verb is: %s", &(rawClientInputBuffer_STDIN[rawInputStartPosition + 5]));

					char* chatroomIDAsString = strtok(&(rawClientInputBuffer_STDIN[rawInputStartPosition + 5]), " ");
					debug("A-the roomid trying to join is %s\n", chatroomIDAsString);

					chatRoomIDAsInt = atoi(chatroomIDAsString);

					debug("B-the roomid trying to join is %d\n", chatRoomIDAsInt);

				}else{
					fprintf(stderr, "\x1B[1;31mInvalid number of arguments.\x1B[0m\r\n");
					fprintf(stderr, "\x1B[1;34m/join <id>\x1B[0m\r\n");
					fflush(stderr);
					free(intermediateBuffer);
					return 0;
				}

			}else if(strcmp(possibleCommand, "/leave") == 0){

					//it said leave, so ignore everything afterwards
					//just leave!!
					strcat(processedClientInputBuffer_STDIN, LEAVE);

					//strcat(processedClientInputBuffer_STDIN, &(rawClientInputBuffer_STDIN[rawInputStartPosition + 6]));

			}else if(strcmp(possibleCommand, "/kick") == 0){

				if(argCount == 1){
					strcat(processedClientInputBuffer_STDIN, KICK);
					strcat(processedClientInputBuffer_STDIN, &(rawClientInputBuffer_STDIN[rawInputStartPosition + 5]));
				}else{
					fprintf(stderr, "\x1B[1;31mInvalid number of arguments.\x1B[0m\r\n");
					fprintf(stderr, "\x1B[1;34m/kick <username>\x1B[0m\r\n");
					fflush(stderr);
					free(intermediateBuffer);
					return 0;
				}

			}else if(strcmp(possibleCommand, "/tell") == 0){

				if(argCount > 1){
					strcat(processedClientInputBuffer_STDIN, TELL);
					strcat(processedClientInputBuffer_STDIN, &(rawClientInputBuffer_STDIN[rawInputStartPosition + 5]));
				}else{
					fprintf(stderr, "\x1B[1;31mInvalid number of arguments.\x1B[0m\r\n");
					fprintf(stderr, "\x1B[1;34m/tell <username>\x1B[0m\r\n");
					fflush(stderr);
					free(intermediateBuffer);
					return 0;
				}

			}else if(strcmp(possibleCommand, "/listusers") == 0){

					strcat(processedClientInputBuffer_STDIN, LISTU);

			}else if(strcmp(possibleCommand, "/help") == 0){

					SLASHCOMMANDS;
					free(intermediateBuffer);
					return 1;

			}else if(strcmp(possibleCommand, "/createp") == 0){

				if(argCount == 2){
					strcat(processedClientInputBuffer_STDIN, CREATEP);
					strcat(processedClientInputBuffer_STDIN, &(rawClientInputBuffer_STDIN[rawInputStartPosition + 8]));
				}else{
					fprintf(stderr, "\x1B[1;31mInvalid number of arguments.\x1B[0m\r\n");
					fprintf(stderr, "\x1B[1;34m/createp <name> <password>\x1B[0m\r\n");
					fflush(stderr);
					free(intermediateBuffer);
					return 0;
				}

			}else if(strcmp(possibleCommand, "/joinp") == 0){
				if(argCount == 2){
					strcat(processedClientInputBuffer_STDIN, JOINP);
					strcat(processedClientInputBuffer_STDIN, &(rawClientInputBuffer_STDIN[rawInputStartPosition + 6]));
				}else{
					fprintf(stderr, "\x1B[1;31mInvalid number of arguments.\x1B[0m\n");
					fprintf(stderr, "\x1B[1;34m/joinp <id> <password>\x1B[0m\n");
					fflush(stderr);
					free(intermediateBuffer);
					return 0;
				}

			}else if(strcmp(possibleCommand, "/upload") == 0){

				if(argCount == 2){

					strcat(processedClientInputBuffer_STDIN, UPLOAD);
					strcat(processedClientInputBuffer_STDIN, &(rawClientInputBuffer_STDIN[rawInputStartPosition + 7]));

					char* tempFileName = strtok(&rawClientInputBuffer_STDIN[rawInputStartPosition + 7], " ");
					strcpy(fileNameWithPath, tempFileName);

					//check if file is valid
					int fileValidity = isFileValid(fileNameWithPath);
					fprintf(stdout, "file validity is: %d\n", fileValidity);
					fflush(stdout);

					if(fileValidity == 0){
						fprintf(stderr, "\x1B[1;31mInvalid File.\x1B[0m\n");
						fflush(stderr);
						free(intermediateBuffer);
						return 0;
					}
					char* sizeInBytes = strtok(NULL, " ");
					int atoiValidity = atoi(sizeInBytes);

					if(atoiValidity <= 0){
						fprintf(stderr, "\x1B[1;31mInvalid File Size.\x1B[0m\n");
						fflush(stderr);
						free(intermediateBuffer);
						return 0;
					}


				}else{
					fprintf(stderr, "\x1B[1;31mInvalid number of arguments.\x1B[0m\n");
					fprintf(stderr, "\x1B[1;34m/upload <filename> <size in bytes>\x1B[0m\n");
					fflush(stderr);
					free(intermediateBuffer);
					return 0;
				}

			}else if(strcmp(possibleCommand, "/download") == 0){

					if(argCount == 2){

						strcat(processedClientInputBuffer_STDIN, DOWNLOAD);
						strcat(processedClientInputBuffer_STDIN, &(rawClientInputBuffer_STDIN[rawInputStartPosition + 9]));
						strcat(processedClientInputBuffer_STDIN, APPEND_END);

						writeToServerFileDescriptor(processedClientInputBuffer_STDIN, connectionFileDescriptor);

						//PHYLLIS: NEED TO HANDLE DOWNLOADING HERE



						free(intermediateBuffer);
						return 1;

					}else{
						fprintf(stderr, "\x1B[1;31mInvalid number of arguments.\x1B[0m\n");
						fprintf(stderr, "\x1B[1;34m/download <filename>\x1B[0m\n");
						fflush(stderr);
						free(intermediateBuffer);
						return 0;
					}

			}else if(strcmp(possibleCommand, "/delete") == 0){

				if(argCount == 1){

					strcat(processedClientInputBuffer_STDIN, DELETE);
					strcat(processedClientInputBuffer_STDIN, &(rawClientInputBuffer_STDIN[rawInputStartPosition + 7]));

				}else{
					fprintf(stderr, "\x1B[1;31mInvalid number of arguments.\x1B[0m\n");
					fprintf(stderr, "\x1B[1;34m/delete <id>\x1B[0m\n");
					fflush(stderr);
					free(intermediateBuffer);
					return 0;
				}

			}else if(strcmp(possibleCommand, "/listf") == 0){

			}else if(strcmp(possibleCommand, "/listfu") == 0){

			}else{
				fprintf(stderr, "\x1B[1;31mInvalid Slash Command.\x1B[0m\n");
				fflush(stderr);
				free(intermediateBuffer);
				return 0;
			}

			strcat(processedClientInputBuffer_STDIN, APPEND_END);

	}else{
		//we have a message, and no need for an intermediateBuffer
		//send it directly with all of the prefix spaces
		generateMessageString(processedClientInputBuffer_STDIN, rawClientInputBuffer_STDIN);

	}
	writeToServerFileDescriptor(processedClientInputBuffer_STDIN, connectionFileDescriptor);

	//there might be an error in the Write, and might need to account for errors later
	free(intermediateBuffer);
	return 1;

}


int generateResponse_ClientSide(int connectionFileDescriptor, char* rawClientInputBuffer_SERVER, char* responseBuffer, char* userName, int newUser){
  //client received something from the server

	if(connectionFileDescriptor < 0){
		fprintf(stderr, "\x1B[1;31mThere was a problem with your connection.\x1B[0m\r\n");
		fflush(stderr);
		return 0;
	}
	if(responseBuffer == NULL){
		fprintf(stderr, "\x1B[1;31mThere was a problem with the response buffer.\x1B[0m\r\n");
		fflush(stderr);
		return 0;
	}
	if(rawClientInputBuffer_SERVER == NULL){

		fprintf(stderr, "\x1B[1;31mThere was a problem with the input buffer.\x1B[0m\r\n");
		fflush(stderr);
		return 0;
	}else{
		//does it end in \r\n?
		int receivedLength = strlen(rawClientInputBuffer_SERVER);

		if(receivedLength <= 2){
			fprintf(stderr, "\x1B[1;31mReceived invalid data.\x1B[0m\r\n");
			fflush(stderr);
			return 0;
		}else{
				if((rawClientInputBuffer_SERVER[receivedLength - 2] != '\r') &&
								(rawClientInputBuffer_SERVER[receivedLength - 1] != '\n')) {
					fprintf(stderr, "\x1B[1;31mReceived incorrectly formatted data: %s.\x1B[0m\n", rawClientInputBuffer_SERVER);
					fflush(stderr);
					return 0;
				}else{

					if(strcmp(&(rawClientInputBuffer_SERVER[receivedLength - 3]), " \r\n") == 0){
						rawClientInputBuffer_SERVER[receivedLength - 3] = '\0';
						rawClientInputBuffer_SERVER[receivedLength - 2] = '\0';
						rawClientInputBuffer_SERVER[receivedLength - 1] = '\0';
					}else{
						rawClientInputBuffer_SERVER[receivedLength - 2] = '\0';
						rawClientInputBuffer_SERVER[receivedLength - 1] = '\0';
					}
				}
		}/* if(receivedLength <= 2) */
	} /* 	if(rawClientInputBuffer_SERVER == NULL) */

	//reaching this far, the last two characters should be correctly converted to NULLS

	char* intermediateBuffer = calloc(MAX_BUFFER_SIZE, sizeof(char));
	strcpy(intermediateBuffer, rawClientInputBuffer_SERVER);
  char* verb = strtok(intermediateBuffer, " \n");
  clearBuffer(responseBuffer, MAX_BUFFER_SIZE);

	debug(stdout, "The returned verb is %s\n", verb);

  if(strcmp(verb, ECHO) == 0){

		clearBuffer(intermediateBuffer, MAX_BUFFER_SIZE);

		char* indexOfSpace = index(rawClientInputBuffer_SERVER, ' '); //first space after echo
		indexOfSpace++; //the first character after "ECHO "
		char* indexOfSpace2 = index(indexOfSpace, ' ');
		*(indexOfSpace2) = '\0';
		indexOfSpace2++;
		strcat(intermediateBuffer, indexOfSpace);

		if(strcasecmp(intermediateBuffer, "server") == 0){
			strcat(responseBuffer, "\x1B[1;34m");
		}
    strcat(responseBuffer, intermediateBuffer);

    strcat(responseBuffer, " > ");
    strcat(responseBuffer, indexOfSpace2);

		if(strcasecmp(intermediateBuffer, "server") == 0){
			strcat(responseBuffer, "\x1B[0m");
		}
		strcat(responseBuffer, "\n");
    echoFromServerToTerminal(responseBuffer);

  }else if(strcmp(verb, AHOLA) == 0){
	  //Client responds using the IAM verb.
	  //C > S | IAM <name> \r\n
    if(userName == NULL){
      writeToTerminal("\x1B[1;31mThere was a problem with your userName.\x1B[0m\n", 0);
			clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
			free(intermediateBuffer);
			return 0;
    }

		if(newUser == 1){
			strcat(responseBuffer, IAMNEW);
		}else{
			strcat(responseBuffer, IAM);
		}

		strcat(responseBuffer, " ");
		strcat(responseBuffer, userName);
		strcat(responseBuffer, APPEND_END);

		alohaholaWithServer = 1;
		writeToServerFileDescriptor(responseBuffer, connectionFileDescriptor);


  }else if((strcmp(verb, AUTH) == 0) || (strcmp(verb, HINEW) == 0)){
    //C < S | AUTH <name> \r\n
    //Server prompts user for password

    //is the client already logged in? connecting to a specific chatroom?

		//if(userLoggedIn == false){}
		//the maximum password size can be 1000

		if(alohaholaWithServer == 1){
			char clientPassword[1001];
			//first, does the name match current client?
			fprintf(stdout, "\nPlease enter your password: ");
			fflush(stdout);
			fscanf(stdin, "%s", clientPassword);
			fflush(stdin);
			fprintf(stdin, "\n");
			fflush(stdin);

			int passwordValidity = valid_password(clientPassword);

			if(passwordValidity == 0){
					generateERRorString(responseBuffer, 61, NULL);
					clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
					free(intermediateBuffer);
					return 0;
			}

			if(strcmp(verb, AUTH) == 0){
				strcat(responseBuffer, PASS);
			}else{
				strcat(responseBuffer, NEWPASS);
			}
			strcat(responseBuffer, " ");
			strcat(responseBuffer, clientPassword);
			strcat(responseBuffer, APPEND_END);

			writeToServerFileDescriptor(responseBuffer, connectionFileDescriptor);
		}

  }else if(strcmp(verb, HI) == 0){
    //Server sends HI verb confirming that logging in was successful
    //Or on successful user creation
    //C < S | HI <name> \r\n

		if(alohaholaWithServer == 1){
			userConnected = 1;
			echoFromServerToTerminal("\x1B[1;34mWe connected to the server.\x1B[0m\n");
		}else{
			fprintf(stderr, "\x1B[1;31Can't connect with the server prior to an aloha/ahola exchange!\x1B[0m\n");
			fflush(stderr);
			clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
			free(intermediateBuffer);
			return 0;
		}

  }else if(strcmp(verb, RTSIL) == 0){

		fprintf(stdout, "\x1B[1;34mRequest to list rooms confirmed.\x1B[0m\n");
		fflush(stdout);

		int handlerVal = listHandler(rawClientInputBuffer_SERVER);

		if(handlerVal == 0){
			clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
			free(intermediateBuffer);
			return 0;
		}

		char* pointerToRTSIL = strstr(rawClientInputBuffer_SERVER, RTSIL);
		char* restOfMessage = pointerToRTSIL + 6;
		fprintf(stdout, "%s\n", restOfMessage);

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
  }else if(strcmp(verb, NIOJ) == 0){
    //only here if successfully joined the room

			debug("expect this chatroom id: %d\n", chatRoomIDAsInt);

			char* chatroomIDAsString = strtok(NULL, " ");
			int validatingRoomID = atoi(chatroomIDAsString);

			debug("receive this chatroom id: %d\n", validatingRoomID);


			if((userConnected == 1) && (chatRoomIDAsInt == validatingRoomID)){
				userLoggedIn = 1;
				strcat(responseBuffer, "\x1B[1;34mYou have successfully joined room ");
				strcat(responseBuffer, chatroomIDAsString); //rawClientInputBuffer_SERVER should end with "\r\n"
				strcat(responseBuffer, "\x1B[0m\n");
				echoFromServerToTerminal(responseBuffer);
			}else if(userConnected == 1){
				userLoggedIn = 1;
				strcat(responseBuffer, "\x1B[1;31mUndefined Behavior: The server returned an invalid ID, but the client may be in the requested room.");
				strcat(responseBuffer, "\x1B[0m\n");
				echoFromServerToTerminal(responseBuffer);
			}else{
				fprintf(stderr, "\x1B[1;31mNeed to be connected to log in!\x1B[0m\n");
				fflush(stderr);
				clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
				free(intermediateBuffer);
				return 0;
			}
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

  }else if(strcmp(verb, EVAEL) == 0){

		debug(stdout, "userLoggedIn value is %d\n", userLoggedIn);

		if(userLoggedIn == 1){
			userLoggedIn = 0;
			echoFromServerToTerminal("\x1B[1;34mYou have left the room.\x1B[0m\n");
		}else{
			echoFromServerToTerminal("\x1B[1;31mUndefined behavior: "\
			"The server said you can leave the room, but the client didn't expect you to be in a room.\x1B[0m\n");
			clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
			free(intermediateBuffer);
			return 0;
		}


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

  }else if(strcmp(verb, KCIK) == 0){

    char* kickedUser = strtok(NULL, " \n");

		if(kickedUser != NULL){
			strcat(responseBuffer, "You have successfully requested that user ");
			strcat(responseBuffer, kickedUser);
			strcat(responseBuffer, " be kicked out.\n");
			echoFromServerToTerminal(responseBuffer);
		}else{
			echoFromServerToTerminal("\x1B[1;31mThe server returned a NULL username in its confirmation.\x1B[0m\n");
			clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
			free(intermediateBuffer);
			return 0;
		}

  }else if(strcmp(verb, KBYE) == 0){

		if(userLoggedIn == 1){
			userLoggedIn = 0;
	    echoFromServerToTerminal("\x1B[1;34mYou were kicked out of the chatroom.\x1B[0m\n");
		}else{
			echoFromServerToTerminal("\x1B[1;31mUndefined behavior: "\
			"You were kicked out of a chatroom, but you were never logged in.\x1B[0m\n");
		}

  }else if(strcmp(verb, BYE) == 0){

      echoFromServerToTerminal("\x1B[1;34mYou have been disconnected.\x1B[0m\n");
			userLoggedIn = 0;
      userConnected = 0;

  }else if(strcmp(verb, LLET) == 0){

		clearBuffer(intermediateBuffer, MAX_BUFFER_SIZE);

		echoFromServerToTerminal(responseBuffer);

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

  }else if(strcmp(verb, UTSIL) == 0){

		debug(stdout, "\x1B[1;34mRequest to list users confirmed.\x1B[0m\n");
		//fflush(stdout);

		int handlerVal = listHandler(rawClientInputBuffer_SERVER);

		if(handlerVal == 0){
			clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
			free(intermediateBuffer);
			return 0;
		}

		char* pointerToUTSIL = strstr(rawClientInputBuffer_SERVER, UTSIL);
		char* restOfMessage = pointerToUTSIL + 6;
		fprintf(stdout, "%s\n", restOfMessage);

  }else if(strcmp(verb, PETAERC) == 0){

		char* privateRoomName = strtok(NULL, " \n");

		if(privateRoomName == NULL){

			fprintf(stderr, "\x1B[1;31mReceived an Incomplete Response from the Server.\x1B[0m\n");
			fflush(stderr);
			clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
			free(intermediateBuffer);
			return 0;
		}else{
			strcat(responseBuffer, privateRoomName);
			fprintf(stdout, "\x1B[1;34mNow in room: %s.\x1B[0m\n", responseBuffer);
			fflush(stdout);
			userLoggedIn = 1;
		}

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
        OR
        ￼# Failed request; Password not valid
        C > S | CREATEP <name> <password> \r\n
        C < S | ERR 61 <message> \r\n
        OR
        # Failed request; Maximum number of rooms exist
        C > S | CREATEP <name> <password> \r\n
        C < S | ERR 11 <message> \r\n
    */


  }else if(strcmp(verb, RETAERC) == 0){

		char* roomName = strtok(NULL, " \n");

		if(roomName == NULL){

			fprintf(stderr, "\x1B[1;31mReceived an Incomplete Response from the Server.\x1B[0m\n");
			fflush(stderr);
			clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
			free(intermediateBuffer);
			return 0;
		}

		strcat(responseBuffer, roomName);
		fprintf(stdout, "\x1B[1;34mNow in room: %s.\x1B[0m\n", responseBuffer);
		fflush(stdout);
		userLoggedIn = 1;

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
  }else if(strcmp(verb, ECHOP) == 0){

		char* indexOfSpace = index(rawClientInputBuffer_SERVER, ' '); //first space after echo
		indexOfSpace++; //the first character after "ECHO "
		char* indexOfSpace2 = index(indexOfSpace, ' ');
		*(indexOfSpace2) = '\0';
		indexOfSpace2++;
		strcat(intermediateBuffer, indexOfSpace);

		strcat(responseBuffer, "\x1B[1;35m");
    strcat(responseBuffer, intermediateBuffer);

    strcat(responseBuffer, " > ");
    strcat(responseBuffer, indexOfSpace2);

		strcat(responseBuffer, "\x1B[0m");

		strcat(responseBuffer, "\n");
    echoFromServerToTerminal(responseBuffer);




    /*
      # A successful request
      C > S | TELL <name> <message> \r\n
      C < S | LLET <name> <message> \r\n
      # Echo to user <name> the message
      C < S | ECHOP <sendername> <message>

      # Failed request; No user with name in room
      C > S | TELL <name> <message> \r\n
      C < S | ERR 30 <message> \r\n
    */


	}else if(strcmp(verb, DAOLPU) == 0){

		//arriving here means the file was uploaded successfully

		if(strlen(fileNameWithPath) != 0){

			int fileSize = StatSize(fileNameWithPath);

			if(fileSize <= 0){
				fprintf(stderr, "\x1B[1;31mInvalid File Size.\x1B[0m\n");
				fflush(stderr);
				clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
				free(intermediateBuffer);
				return 0;
			}

			Send(connectionFileDescriptor, fileNameWithPath, fileSize, 0);

			clearBuffer(fileNameWithPath, MAX_BUFFER_SIZE);

		}else{
			fprintf(stderr, "\x1B[1;31mReceived an Invalid Upload Request from the Server.\x1B[0m\n");
			fflush(stderr);
			clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
			free(intermediateBuffer);
			return 0;
		}

	}else if(strcmp(verb, DAOLNWOD) == 0){

			//shouldn't be here




	}else if(strcmp(verb, ETELED) == 0){

		fprintf(stderr, "\x1B[1;34mServer received request to delete file.\x1B[0m\n");
		fflush(stderr);
		clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
		free(intermediateBuffer);
		return 1;

	}else if(strcmp(verb, FTSIL) == 0){

	}else if(strcmp(verb, UFTSIL) == 0){

  }else if(strcmp(verb, ERR) == 0){
		//handle errors received from server
		fprintf(stderr, "\x1B[1;31m%s\x1B[0m\n", rawClientInputBuffer_SERVER);
		fflush(stderr);
		clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
		free(intermediateBuffer);
		return 0;
  }else{
		//shouldn't be here, so call an ERR
		fprintf(stderr, "\x1B[1;31mReceived an Invalid Verb from the Server: %s.\x1B[0m\n", rawClientInputBuffer_SERVER);
		fflush(stderr);
		clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
		free(intermediateBuffer);
		return 0;
	}

	clearBuffer(rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE);
	free(intermediateBuffer);
  return 1;

}




int listHandler(char* givenBuffer){
		int listPosition = strlen(givenBuffer);

		if((givenBuffer[listPosition - 2] == '\r') && (givenBuffer[listPosition - 1] == '\n')){
					//replace the last four characters with nulls
					givenBuffer[listPosition - 1] = '\0';
					givenBuffer[listPosition - 2] = '\0';

		}else{
			//it wasn't correctly formatted
			fprintf(stderr, "\x1B[1;31mReceived improperly formatted data.\x1B[0m\n");
			fflush(stderr);
			return 0;
		}
		//already shortened string with removal of last two characters
		listPosition = listPosition - 2;

		while(listPosition >= 0){
			if(givenBuffer[listPosition] == '\r'){
					givenBuffer[listPosition] = ' ';
			}
			listPosition--;
		}
		return 1;
}






int clientSRC_SERVER(int connectionFileDescriptor, char* rawClientInputBuffer_SERVER, char* responseBuffer, int bufferSize_SERVER){

		clearBuffer(rawClientInputBuffer_SERVER, bufferSize_SERVER);
		clearBuffer(responseBuffer, bufferSize_SERVER);

		ssize_t recvVal= Recv(connectionFileDescriptor, rawClientInputBuffer_SERVER, MAX_BUFFER_SIZE, 0);

		if((recvVal <= 0) || (rawClientInputBuffer_SERVER == NULL)){
				fprintf(stderr, "\x1B[1;31mThe connection to the server was lost.\x1B[0m\n");
				fflush(stderr);
				userConnected = 0;
				userLoggedIn = 0;
				return 0;
		}

    int responseStatus = generateResponse_ClientSide(connectionFileDescriptor, rawClientInputBuffer_SERVER, responseBuffer, NULL, 0);
    return responseStatus;
}


void generateMessageString(char* formattedMessageBuffer, char* rawMessageBuffer){

		if(rawMessageBuffer == NULL){
				return;
		}

		char* intermediateBuffer = calloc(MAX_BUFFER_SIZE, sizeof(char));

		strcpy(intermediateBuffer, rawMessageBuffer);
		char* validateForSpaceOrNewLineOnly = strtok(intermediateBuffer, " \n");

		if(validateForSpaceOrNewLineOnly == NULL){
			free(intermediateBuffer);
			return;
		}

		free(intermediateBuffer);

    clearBuffer(formattedMessageBuffer, MAX_BUFFER_SIZE);
    rawMessageBuffer[1000] = '\0'; //the message can't go past byte 0-999
    strcat(formattedMessageBuffer, MSG);
    strcat(formattedMessageBuffer, " ");
    strcat(formattedMessageBuffer, rawMessageBuffer);
    strcat(formattedMessageBuffer, APPEND_END);
    clearBuffer(rawMessageBuffer, MAX_BUFFER_SIZE);

    return;

}



//
