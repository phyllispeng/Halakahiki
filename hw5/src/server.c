#include "server.h"

int main(int argc, char *argv[]){



	signal (SIGINT,  sigintHandler);
	signal (SIGSEGV, sigsegvHandler);


	server = calloc(1, sizeof(struct serverData));
	server->users = Hashmap_create();
	server->echo_flag = 0;
	server->me = createUser(SERVER, 1);

	int fileValid;
	fileValid = isFileValid(DBFILE);
	if(!fileValid){ // Create file for the first time
		server->db_file = fopen(DBFILE, "w");
		fclose(server->db_file);
	}else{
		char *line = NULL;
        size_t len = 0;
        ssize_t read;
		server->db_file = fopen(DBFILE, "r");
		read = getline(&line, &len, server->db_file);
		if(db == NULL){
			db = calloc(1, sizeof(database));
			db->username = strdup(strtok(line, " "));
			db->passhash = strdup(strtok(NULL, " "));
		}
		database* cursor = db;
		while ((read = getline(&line, &len, server->db_file)) != -1) {
			if(cursor->next == NULL){
				cursor->next = calloc(1, sizeof(database)); 
				cursor = cursor->next;
				cursor->username = strdup(strtok(line, " "));
				cursor->passhash = strdup(strtok(NULL, " "));
			}
		}
		fclose(server->db_file);
	}
	

	if(argc < 3){
		fprintf(stderr, "\n\n\e[91mMissing Arguments!\e[0m\n\n");
		USAGE;
		free(server);
		exit(EXIT_FAILURE);
	}

	server->waitingRoom = createChatRoom("Waiting Room", NULL);
	server->waitingRoom->numOfUsers--;
	server->waitingRoom->users->next = NULL;
	server->chatrooms = calloc(1, sizeof(chatroomList_t));
	server->chatrooms->cr = server->waitingRoom;

	int opt;
	server->numChatRoomsPossible = INITIALNUMROOMS;
	showMssgs = FALSE;
	while ((opt = getopt(argc, argv, "heN:")) != -1) {
		switch (opt) {
			case 'N':
				server->numChatRoomsPossible = atoi(optarg);
				break;
			case 'h':
				USAGE;
				free(server);
				exit(EXIT_SUCCESS);
			break;
			case 'e':
				showMssgs = TRUE;
			break;
			default: /* '?' */
				USAGE;
				free(server);
				exit(EXIT_FAILURE);
		}
	}
	if(server->numChatRoomsPossible == INITIALNUMROOMS){
		server->numChatRoomsPossible = 5;
	}
	int argvIndex = optind;

	//argvIndex++;
	char* port = argv[argvIndex];

	argvIndex++;
	char* mssgOfDay = argv[argvIndex];


	server->motd = mssgOfDay;
	server->echoServPort = atoi(port);
	// parse and i/o functions should be in parse.c

	serverRun();

	writeOutUsers();

	return EXIT_SUCCESS;
}

int serverRun(){

	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr; /* Client address */

	server->servSock = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));   		/* Zero out structure */
	echoServAddr.sin_family = AF_INET;                		/* Internet address family */
	echoServAddr.sin_addr.s_addr = INADDR_ANY;				/* Any incoming interface */
	echoServAddr.sin_port = htons(server->echoServPort);	/* Local port */

	/* Bind to the local address */
    Bind(server->servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));

    /*Don't Block on the Socket*/
    //MakeSocketNotBlocking(server->servSock);

    /* Mark the socket so it will listen for incoming connections */
    Listen(server->servSock, 1);


	int clntSock;					  /* Socket descriptor for client */
	unsigned int clntLen;             /* Length of client address data structure */

    int everythingIsFine = TRUE;
	while(everythingIsFine) {
		/* Set the size of the in-out parameter */
		clntLen = sizeof(echoClntAddr);

		/* Wait for a client to connect */
		clntSock = Accept(server->servSock, (struct sockaddr *) &echoClntAddr, &clntLen);

		checkIn(clntSock, &echoClntAddr);

		if(server->users->used_size == 1){
			server->echo_flag++;
			Thread_create(&server->echoThreadid, NULL, &echoThread, NULL);
		}

		sleep(1);
	}

	return everythingIsFine;
}

void checkIn(int clntSock, struct sockaddr_in* echoClntAddr){
	int recvMsgSize;
	user_t* newUser = calloc(1, sizeof(struct user_t));
	newUser->networkInfo = echoClntAddr;
	newUser->socket = clntSock;
	char in__buff[MSG_SIZE];
	char out_buff[MSG_SIZE];
	memset(in__buff, 0, MSG_SIZE);
	memset(out_buff, 0, MSG_SIZE);

	recvMsgSize =Recv(clntSock, in__buff, MSG_SIZE, 0); // Recieve ALOHA!

	if(showMssgs)write(STDOUT_FILENO, in__buff, strlen(in__buff));

	char* this = in__buff;
	char* inAloha = strtok_r(in__buff, " \r\n", &this);

	if(strcmp(inAloha, ALOHA) != 0){
		Send(clntSock, "ERR 100 IMPROPER INIT MESSAGE \r\n", 32, 0);
		Send(clntSock, BYE, SZ_BYE, 0);
		Close(clntSock);
		return;
	}

	int socket = newUser->socket;
	memset(out_buff, 0, MSG_SIZE);
	strcat(out_buff, AHOLA);
	strcat(out_buff, APPEND_END);
	Send(socket, out_buff, strlen(out_buff), 0); // Send !AHOLA
	memset(out_buff, 0, MSG_SIZE);


	memset(in__buff, 0, MSG_SIZE);
	memset(out_buff, 0, MSG_SIZE);
	/* Check in user */
    recvMsgSize = Recv(socket, in__buff, MSG_SIZE, 0); // EXPECTED: IAM <name>
    if(recvMsgSize == -2){
    	Close(socket);
    	return;
    }

    if(showMssgs)write(STDOUT_FILENO, in__buff, strlen(in__buff));

	char* iamStr = strtok(in__buff, " \n");
	char* name = strdup(strtok(NULL, " \n\r"));

	if(strcmp(iamStr, IAM) == 0){ /* Incorrect IAM message */
		if(isTaken(server->users,name)){
			strcat(out_buff, "ERR 00 ");
			strcat(out_buff, ER_00);
			strcat(out_buff, name);
			strcat(out_buff, END_STRING);
			Send(socket, out_buff, strlen(out_buff), 0);
			Send(socket, BYE, SZ_BYE, 0);
			Close(socket);
			return;
		}

		if(!existsInDb(name)){
			memset(out_buff, 0, MSG_SIZE);
			generateERRorString(out_buff, 2, name);
			Send(socket, out_buff, strlen(out_buff), 0);
			Close(socket);
			return;
		}

		memset(out_buff, 0, MSG_SIZE);
		strcat(out_buff, AUTH);
		strcat(out_buff, SPACE);
		strcat(out_buff, name);
		strcat(out_buff, APPEND_END);
		Send(socket, out_buff, strlen(out_buff), 0);	

		memset(in__buff, 0, MSG_SIZE);
		recvMsgSize = Recv(socket, in__buff, MSG_SIZE, 0); 
		
		char* pass = in__buff;
		pass = pass + 5;
		pass[strlen(pass)-3] = '\0';

		if(incorrectPassword(name, pass)){
			memset(out_buff, 0, MSG_SIZE);
			generateERRorString(out_buff, 61, NULL);
			Send(socket, out_buff, strlen(out_buff), 0);
		}

		memset(out_buff, 0, MSG_SIZE);
		strcat(out_buff, HI);
		strcat(out_buff, SPACE);
		strcat(out_buff, name);
		strcat(out_buff, APPEND_END);
		Send(socket, out_buff, strlen(out_buff), 0);

		memset(out_buff, 0, MSG_SIZE);
		strcat(out_buff, ECHO);
		strcat(out_buff, SPACE);
		strcat(out_buff, SERVER);
		strcat(out_buff, SPACE);
		strcat(out_buff, server->motd);
		strcat(out_buff, APPEND_END);
		Send(socket, out_buff, strlen(out_buff), 0);

		newUser->name = name;
		Hashmap_set(server->users, name, newUser);

		// Logged in!
		if(server->echo_flag <= 0){
			server->echo_flag++;
		}

		addUser(server->waitingRoom, newUser);

		recvMsgSize++;

	}else if(strcmp(iamStr, IAMNEW) == 0){
		
		if(isTaken(server->users,name) || existsInDb(name)){
			strcat(out_buff, "ERR 00 ");
			strcat(out_buff, ER_00);
			strcat(out_buff, name);
			strcat(out_buff, END_STRING);
			Send(socket, out_buff, strlen(out_buff), 0);
			Send(socket, BYE, SZ_BYE, 0);
			Close(socket);
			return;
		}

		memset(out_buff, 0, MSG_SIZE);
		strcat(out_buff, HINEW);
		strcat(out_buff, SPACE);
		strcat(out_buff, name);
		strcat(out_buff, APPEND_END);
		Send(socket, out_buff, strlen(out_buff), 0);

		memset(in__buff, 0, MSG_SIZE);
		memset(out_buff, 0, MSG_SIZE);
		recvMsgSize = Recv(socket, in__buff, MSG_SIZE, 0);
		char* pass = in__buff;
		pass=pass+8;
		pass[strlen(pass)-3] = '\0';

		

		if(valid_password(pass) == FALSE){
			memset(in__buff, 0, MSG_SIZE);
			generateERRorString(out_buff, 60, NULL);
			Send(socket, out_buff, strlen(out_buff), 0);
			Close(socket);
			return;
		}else{
			//add new user
			if(db == NULL){
				db = calloc(1, sizeof(database));
				db->username = name;
				db->passhash = pass; // NO!!!!
			}else{
				database* cursor = db;
				while(cursor->next!=NULL){
					cursor = cursor->next;
				}
				cursor->next = calloc(1, sizeof(database));
				cursor = cursor->next;
				cursor->username = name;
				// RAND_bytes((unsigned char*)pass, 12);

				cursor->passhash = pass; //FIX ME AHHHHHH
			}
			
			writeOutAUser(name, pass);

			memset(out_buff, 0, MSG_SIZE);
			strcat(out_buff, HI);
			strcat(out_buff, SPACE);
			strcat(out_buff, name);
			strcat(out_buff, APPEND_END);
			Send(socket, out_buff, strlen(out_buff), 0);

			memset(out_buff, 0, MSG_SIZE);
			strcat(out_buff, ECHO);
			strcat(out_buff, SPACE);
			strcat(out_buff, SERVER);
			strcat(out_buff, SPACE);
			strcat(out_buff, server->motd);
			strcat(out_buff, APPEND_END);
			Send(socket, out_buff, strlen(out_buff), 0);

			newUser->name = name;
			Hashmap_set(server->users, name, newUser);

			// Logged in!
			if(server->echo_flag <= 0){
				server->echo_flag++;
			}

			addUser(server->waitingRoom, newUser);

			recvMsgSize++;

		}

	}

}

int incorrectPassword(char* user, char* pass){

	database* cursor = db;
	while(cursor!=NULL){
		if(strcmp(user, cursor->username) == 0){
			return strcmp(pass, cursor->passhash) == 0?TRUE:FALSE;
		}
		cursor = cursor->next;
	}

	return FALSE;
}

void* echoThread(void* args){

	//Create some local pointers for code readability:
	Hashmap* users = server->users;
	//Hashmap* chatrooms = server->chatrooms;

	//Set Select to listen to all client Sockets
	fd_set active_fd_set, read_fd_set;

	// Begin listening Echoing and VERB handling ~

	char in__buff[MSG_SIZE];
	char out_buff[MSG_SIZE];
	memset(in__buff, 0, MSG_SIZE);
	memset(out_buff, 0, MSG_SIZE);

	struct timeval tv;
	tv.tv_sec = 0;
    tv.tv_usec = 5;

	int everythingIsFine = TRUE;
	while(everythingIsFine){

		FD_ZERO (&active_fd_set);
		for (int i = 0; i < users->Hashmap_size; ++i){
			user_t* cur = users->node[i].value;
			if((cur!=NULL)){
				FD_SET(cur->socket, &active_fd_set);
			}
		}

		// And now we wait...
		read_fd_set = active_fd_set;
		int sret = Select(FD_SETSIZE, &read_fd_set, &tv);
		// ACTION STATIONS! ACTION STATIONS! WE HAVE INCOMING!!

		for(int currentSocket = 0 ; currentSocket < FD_SETSIZE; currentSocket++){ // for each socket with data
			if(sret == 0)break; // timeout!
			if(FD_ISSET(currentSocket, &read_fd_set)){ // Only do work on sockets with data // Mark them in the read (RED) filedes set

				int readret = Read(currentSocket, in__buff, MSG_SIZE);
				if(readret < 0){
					break;
				}

				if(in__buff[0] == '\0')break;

				if(showMssgs)write(STDOUT_FILENO, in__buff, strlen(in__buff));

				char* verb = strdup(in__buff);

				verb = strtok(verb, " ");

				user_t* communicatingUser = NULL;

				server->numChatRooms = countRooms(server->chatrooms);

				//Find the current user
				for (int j = 0; j < users->Hashmap_size; j++){
					user_t* cur = users->node[j].value;
					if(cur!=NULL){
						if(cur->socket == currentSocket){
							communicatingUser = cur;
						}
					}
				}

				if(communicatingUser == NULL){
					printf("User not found????\n");
					exit(EXIT_FAILURE);
				}

				chatroom_t* currentRoom = NULL;

				chatroomList_t* cursor = server->chatrooms;
				while(cursor!=NULL){
					usersList* usersCursor = cursor->cr->users;
					while(usersCursor!=NULL){
						if(usersCursor->usr == communicatingUser){
							currentRoom = cursor->cr;
							break;
						}
						usersCursor = usersCursor->next;
					}
					cursor=cursor->next;
				}

				if(currentRoom == NULL) {
					printf("Unable to find user's room\n");
					exit(EXIT_FAILURE);
				}

				for(int k = 0; k < NUM_COMMANDS; k++){
					if(strcmp(verb, verbToCommandList[k]) == 0){
						(*commandFuncs[k])(currentRoom, communicatingUser, in__buff);
					}
				}

				FD_CLR(currentSocket, &read_fd_set);

				memset(in__buff, 0, MSG_SIZE);

			}

		}

	}

	return NULL;
}

int echo(chatroom_t* room, user_t* originUser, char* inputCommand){
	char* manip = strdup(inputCommand);
	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);
	if(room!=server->waitingRoom || originUser==server->me){ // Only server can echo to wait room

		if(manip[0]=='M'){
			manip = manip+4;
			strcat(out_buff, ECHO);
			strcat(out_buff, SPACE);
			strcat(out_buff, originUser->name);
			strcat(out_buff, SPACE);
			strcat(out_buff, manip);
		}else if(manip[0]=='E'){
			// ERR messages are prepped to go
		}

		//strcat(out_buff, END_STRING);

		usersList* cursor = room->users;

		while(cursor != NULL){
			if(cursor->usr == NULL){
				cursor = cursor->next;
				continue;
			}
			int soc = cursor->usr->socket;
			Send(soc, out_buff, strlen(out_buff), 0);
			cursor = cursor->next;
		}
	}else{
		generateERRorString(out_buff, 60, NULL);
		Send(originUser->socket, out_buff, strlen(out_buff), 0);
	}
	return 0;
}

int	leave(chatroom_t* room, user_t* originUser, char* inputCommand){
	int ret = 0;
	char out_buff[MSG_SIZE];
	if(room != server->waitingRoom){
		if(countRooms(server->chatrooms) > 1){
			if(inputCommand != NULL){
				memset(out_buff, 0, MSG_SIZE);
				strcat(out_buff, EVAEL);
				strcat(out_buff, SPACE);
				strcat(out_buff, APPEND_END);
				Send(originUser->socket, out_buff, strlen(out_buff), 0);

				memset(out_buff, 0, MSG_SIZE);
				strcat(out_buff, MSG);
				strcat(out_buff, SPACE);
				strcat(out_buff, originUser->name);
				strcat(out_buff, LEFTNOTIFY);
				strcat(out_buff, APPEND_END);

				echo(room, server->me, out_buff);

				int mret = moveFromTo(room, server->waitingRoom, originUser);
				if(mret == EMPTY_ROOM){
					removeChatRoomFromList(server->chatrooms, room);
				}

			}else{
				//quitting for good!

				Hashmap_remove_node(server->users, originUser->name);


			}
		}else{
			strcat(out_buff, EVAEL);
			strcat(out_buff, SPACE);
			strcat(out_buff, "no_rooms -1");
			strcat(out_buff, END_STRING);
			strcat(out_buff, END_STRING);
		}
	}else{
		memset(out_buff, 0, MSG_SIZE);
		generateERRorString(out_buff, 60, NULL);
		Send(originUser->socket, out_buff, strlen(out_buff), 0);
	}

	return ret;
}

int	createR(chatroom_t* room, user_t* originUser, char* inputCommand){
	int ret = 0;

	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);
	if(room == server->waitingRoom){
		if(server->numChatRoomsPossible == server->numChatRooms+1){
			generateERRorString(out_buff, 11, NULL);
			Send(originUser->socket, out_buff, strlen(out_buff), 0);
		}else{

			char* manip = strdup(inputCommand);

			manip = manip+8;
			manip[strlen(manip)-1] = '\0';
			manip[strlen(manip)-2] = '\0';

			chatroom_t* secretRoom;
			if((secretRoom = roomExists(server->chatrooms, manip)) != NULL && secretRoom->numOfUsers == 0){

				memset(out_buff, 0, MSG_SIZE);
				generateERRorString(out_buff, 10, NULL);
				Send(originUser->socket, out_buff, strlen(out_buff), 0);

			}else if(secretRoom != NULL && secretRoom->numOfUsers == 0){

				addUser(secretRoom, originUser);

				memset(out_buff, 0, MSG_SIZE);
				strcat(out_buff, RETAERC);
				strcat(out_buff, SPACE);
				strcat(out_buff, secretRoom->name);
				strcat(out_buff, APPEND_END);

				server->numChatRooms = countRooms(server->chatrooms);

				Send(originUser->socket, out_buff, strlen(out_buff), 0);

				memset(out_buff, 0, MSG_SIZE);
				strcat(out_buff, MSG);
				strcat(out_buff, SPACE);
				strcat(out_buff, secretRoom->name);
				strcat(out_buff, HASBEENCREATED);
				strcat(out_buff, APPEND_END);

				echo(server->waitingRoom, server->me, out_buff);

			}else{

				chatroom_t* newRoom = createChatRoom(manip, originUser);

				addChatRoomToList(server->chatrooms, newRoom);

				memset(out_buff, 0, MSG_SIZE);
				strcat(out_buff, RETAERC);
				strcat(out_buff, SPACE);
				strcat(out_buff, newRoom->name);
				strcat(out_buff, APPEND_END);

				server->numChatRooms = countRooms(server->chatrooms);

				Send(originUser->socket, out_buff, strlen(out_buff), 0);

				memset(out_buff, 0, MSG_SIZE);
				strcat(out_buff, MSG);
				strcat(out_buff, SPACE);
				strcat(out_buff, newRoom->name);
				strcat(out_buff, HASBEENCREATED);
				strcat(out_buff, APPEND_END);

				echo(server->waitingRoom, server->me, out_buff);
			}
		}
	}else{
		memset(out_buff, 0, MSG_SIZE);
		generateERRorString(out_buff, 60, NULL);
		Send(originUser->socket, out_buff, strlen(out_buff), 0);
	}


	return ret;
}

int	listR(chatroom_t* room, user_t* originUser, char* inputCommand){
	int ret = 0;

	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);
	server->numChatRooms = countRooms(server->chatrooms);
	if(room != server->waitingRoom){
		memset(out_buff, 0, MSG_SIZE);
		generateERRorString(out_buff, 60, NULL);
		Send(originUser->socket, out_buff, strlen(out_buff), 0);
	}else{
		if(server->numChatRooms == 0){
			char* err = "RTSIL no_rooms -1 \r\n\r\n";
			Send(originUser->socket, err, strlen(err),0);
		}else{
			strcat(out_buff, RTSIL);
			strcat(out_buff, SPACE);

			chatroomList_t* cursor = server->chatrooms;
			int no_roomsFlg = TRUE;
			while(cursor != NULL){

				if(	   cursor->cr != NULL 
					&& cursor->cr->numOfUsers !=0 
					&& cursor->cr != server->waitingRoom) {

					strcat(out_buff, cursor->cr->name);
					strcat(out_buff, SPACE);
					char id[70];
					sprintf(id, "%d", cursor->cr->chatRoomID);
					strcat(out_buff, id);
					strcat(out_buff, APPEND_END);
					no_roomsFlg = FALSE;
				}

				cursor = cursor->next;
			}
			strcat(out_buff, END_STRING);

			if(no_roomsFlg){
				char* err = "RTSIL no_rooms -1 \r\n\r\n";
				Send(originUser->socket, err, strlen(err),0);
			}else{
				Send(originUser->socket, out_buff, strlen(out_buff),0);
			}	
		}
	}

	return ret;
}

int	join(chatroom_t* room, user_t* originUser, char* inputCommand){
	int ret = 0;

	char* manip = strdup(inputCommand);
	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);

	manip=manip+5;

	char* truncatorPtr=manip;
	while(*truncatorPtr != ' '){
		truncatorPtr++;
	}
	*truncatorPtr='\0';

	int id = atoi(manip);
	chatroom_t* wantedRoom = NULL;

	chatroomList_t* cursor = server->chatrooms;
	while(cursor != NULL){
		if(cursor->cr != NULL){
			if(cursor->cr->chatRoomID == id){
				wantedRoom = cursor->cr;
			}
		}
		cursor=cursor->next;
	}

	if(room == wantedRoom){
		memset(out_buff, 0, MSG_SIZE);
		generateERRorString(out_buff, 60, NULL);
		Send(originUser->socket, out_buff, strlen(out_buff), 0);
	}else{
		if(wantedRoom != NULL){
			moveFromTo(room,wantedRoom, originUser);

			memset(out_buff, 0, MSG_SIZE);
			strcat(out_buff, NIOJ);
			strcat(out_buff, SPACE);
			char id[70];
			sprintf(id, "%d", wantedRoom->chatRoomID);
			strcat(out_buff, id);
			strcat(out_buff, APPEND_END);
			Send(originUser->socket, out_buff, strlen(out_buff), 0);

			memset(out_buff, 0, MSG_SIZE);
			strcat(out_buff, MSG);
			strcat(out_buff, SPACE);
			strcat(out_buff, originUser->name);
			strcat(out_buff, JOINNOTIFY);
			strcat(out_buff, APPEND_END);

			echo(wantedRoom, server->me, out_buff);
		}else{
			memset(out_buff, 0, MSG_SIZE);
			generateERRorString(out_buff, 20, NULL);
			Send(originUser->socket, out_buff, strlen(out_buff), 0);
		}
	}

	return ret;
}

int	kick(chatroom_t* room, user_t* originUser, char* inputCommand){ // What everone does to Paul. . .
	int ret = 0;

	char* manip = strdup(inputCommand);
	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);

	manip=manip+5;

	char* truncatorPtr=manip;
	while(*truncatorPtr != ' '){
		truncatorPtr++;
	}
	*truncatorPtr='\0';

	char* nameToKick = manip;
	user_t* userToKick = NULL;

	usersList* cursor = room->users;
	while(cursor != NULL){
		if(cursor->usr != NULL && strcmp(nameToKick, cursor->usr->name) == 0){
			userToKick = cursor->usr;
			break;
		}
		cursor = cursor->next;
	}

	if(userToKick == NULL){
		memset(out_buff, 0, MSG_SIZE);
		generateERRorString(out_buff, 61, NULL);
		Send(originUser->socket, out_buff, strlen(out_buff), 0);
	}else{

		moveFromTo(room, server->waitingRoom, userToKick);

		memset(out_buff, 0, MSG_SIZE);
		strcat(out_buff, KBYE);
		strcat(out_buff, APPEND_END);
		Send(userToKick->socket, out_buff, strlen(out_buff), 0);

		memset(out_buff, 0, MSG_SIZE);
		strcat(out_buff, KCIK);
		strcat(out_buff, SPACE);
		strcat(out_buff, userToKick->name);
		strcat(out_buff, APPEND_END);
		Send(originUser->socket, out_buff, strlen(out_buff), 0);

	}




	moveFromTo(room, server->waitingRoom, cursor->usr);

	return ret;
}

int	tell(chatroom_t* room, user_t* originUser, char* inputCommand){
	int ret = 0;

	char* manip = strdup(inputCommand);
	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);

	manip = manip+5;
	char* temp = manip;

	char* userToSendTo;
	char* messageToSend;

	while(*temp != ' '){
		temp++;
	}
	*temp='\0';
	userToSendTo = manip;
	temp++;
	messageToSend = temp;
	messageToSend[strlen(messageToSend)-3] = '\0';

	user_t* recvUser = NULL;
	usersList* cursor = room->users;
	while(cursor!=NULL){
		if(    cursor->usr != NULL
			&& strcmp(userToSendTo, cursor->usr->name) == 0){

			recvUser = cursor->usr;
			break;
		}
		cursor=cursor->next;
	}

	if(recvUser == NULL){
		generateERRorString(out_buff, 30, NULL);
		Send(originUser->socket, out_buff, strlen(out_buff), 0);

	}else{
		strcat(out_buff, LLET);
		strcat(out_buff, SPACE);
		strcat(out_buff, userToSendTo);
		strcat(out_buff, SPACE);
		strcat(out_buff, messageToSend);
		strcat(out_buff, APPEND_END);
		Send(originUser->socket, out_buff, strlen(out_buff), 0);

		memset(out_buff, 0, MSG_SIZE);
		strcat(out_buff, ECHOP);
		strcat(out_buff, SPACE);
		strcat(out_buff, originUser->name);
		strcat(out_buff, SPACE);
		strcat(out_buff, messageToSend);
		strcat(out_buff, APPEND_END);
		Send(recvUser->socket, out_buff, strlen(out_buff), 0);
	}

	return ret;
}

int	listU(chatroom_t* room, user_t* originUser, char* inputCommand){
	int ret = 0;

	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);

	if(room == server->waitingRoom){
		generateERRorString(out_buff, 60, NULL);
	}else{

		strcat(out_buff, UTSIL);
		strcat(out_buff, SPACE);
		usersList* cursor = room->users;
		while(cursor!=NULL){
			if(cursor->usr != NULL){
					strcat(out_buff, cursor->usr->name);
					strcat(out_buff, APPEND_END);
			}
			cursor = cursor->next;
		}
		strcat(out_buff, END_STRING);
	}

	Send(originUser->socket, out_buff, strlen(out_buff),0);

	return ret;
}

int bye(chatroom_t* room, user_t* originUser, char* inputCommand){
	int ret = 0;

	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);

	strcat(out_buff, BYE);
	strcat(out_buff, SPACE);
	strcat(out_buff, END_STRING);

	Send(originUser->socket, out_buff, strlen(out_buff), 0);

	Close(originUser->socket);

	memset(out_buff, 0, MSG_SIZE);

	strcat(out_buff, MSG);
	strcat(out_buff, SPACE);
	strcat(out_buff, originUser->name);
	strcat(out_buff, DISCONNECTED);

	leave(room, originUser, NULL);

	Hashmap_remove_node(server->users, originUser->name);

	echo(room, server->me, out_buff);

	return ret;
}

int download(chatroom_t* room, user_t* originUser, char* inputCommand){
	int ret = 0;

	// char* manip = strdup(inputCommand);
	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);

	generateERRorString(out_buff, 100, NULL);
	Send(originUser->socket, out_buff, strlen(out_buff), 0);
	// manip = manip+9;
	// char* temp = manip;

	// char* id;
	// char* size;

	// while(*temp != ' '){
	// 	temp++;
	// }
	// *temp='\0';
	// id = manip;
	// temp++;
	// size = temp;
	// size[strlen(size)-3] = '\0';


	// // TODO Phyillis Upload filename and size avaible and socket and ID 
	// // I don't have the filename at this point but we can use the id to find it 
	// // let me know how I can help u !! 
	// // u r so super
	// //PHY
	// // struct stat st;

	// // char* filename;

	// // stat(filename, &st);

	// // send_file(originUser->socket, st.st_size, filename);

	// //PHY

	// char* done = "done";
	// Send(originUser->socket, done, strlen(done), 0);

	return ret;
}

int upload(chatroom_t* room, user_t* originUser, char* inputCommand){
	int ret = 0;

	// char* manip = strdup(inputCommand);
	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);

	generateERRorString(out_buff, 100, NULL);
	Send(originUser->socket, out_buff, strlen(out_buff), 0);

	// manip = manip+7;
	// char* temp = manip;

	// char* filename;
	// char* size;

	// while(*temp != ' '){
	// 	temp++;
	// }
	// *temp='\0';
	// filename = manip;
	// temp++;
	// size = temp;
	// size[strlen(size)-3] = '\0';

	// int id = hash_function(filename)%DEFAULT_NUMBER_NODE;

	// // TODO Phyillis Upload filename and size avaible and socket and room
	// // PHY

	// // recieve_file(originUser->socket, room);

	// ///PHY

	// strcat(out_buff, MSG);
	// strcat(out_buff, SPACE);
	// strcat(out_buff, filename);
	// strcat(out_buff, SPACE);
	// char ids[70];
	// sprintf(ids, "%d", id);
	// strcat(out_buff, ids);
	// strcat(out_buff, AVANOTIFY);

	// echo(room, server->me, out_buff);

	return ret;
}

int delete(chatroom_t* room, user_t* originUser, char* inputCommand){
	int ret = 0;

	//CODE GOES HERE I GUESS i didn't do this yet I'm workin on it
	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);

	generateERRorString(out_buff, 100, NULL);
	Send(originUser->socket, out_buff, strlen(out_buff), 0);
	// PHY

	return ret;
}

int listf(chatroom_t* room, user_t* originUser, char* inputCommand){
	int ret = 0;

	//CODE GOES HERE I GUESS i didn't do this yet I'm workin on it
	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);

	generateERRorString(out_buff, 100, NULL);
	Send(originUser->socket, out_buff, strlen(out_buff), 0);
	// PHY

	return ret;
}

int listfu(chatroom_t* room, user_t* originUser, char* inputCommand){
	int ret = 0;

	//CODE GOES HERE I GUESS i didn't do this yet I'm workin on it
	char out_buff[MSG_SIZE];
	memset(out_buff, 0, MSG_SIZE);

	generateERRorString(out_buff, 100, NULL);
	Send(originUser->socket, out_buff, strlen(out_buff), 0);
	// PHY

	return ret;
}

void sigintHandler(int sig){
	Close(server->servSock);
	//close_all_users();
	//writeOutUsers();
	write(1," Force Exit Success\n", 20);
	free(server);
	exit(0);
}

void sigsegvHandler(int sig){
	Close(server->servSock);
	//close_all_users();
	write(1,"Everything is fine.\n", 20);
	//close_all_users();
	//writeOutUsers();
	free(server);
	exit(0);
}

int SendServer(int sockfd, const void *buf, size_t len, int flags, user_t* reciever){
	int ret = 0;

	int sret = Send(sockfd, buf, len, flags);

	if(sret == USRDISCONN){
		removeAllInstancesofUser(reciever);
	}

	return ret;
}

int removeAllInstancesofUser(user_t* reciever){
	int ret = 0;

	chatroomList_t* cursorRooms;


	//Loop through every user in every chat room
	cursorRooms = server->chatrooms;
	while(cursorRooms != NULL){
		chatroom_t* room = cursorRooms->cr;
		if(room != NULL){
			usersList* usersCursor = room->users;
			while(usersCursor!=NULL){
				user_t* currentUsr = usersCursor->usr;
				if(reciever == currentUsr){
					printf("Remove %p", currentUsr);
					printf(" in %s\n", room->name);
					usersCursor->usr = NULL;
				}
				usersCursor = usersCursor->next;
			}
		}
		cursorRooms = cursorRooms->next;
	}

	Hashmap_remove_node(server->users, reciever->name);

	return ret;
}

void close_all_users(){

	for (int j = 0; j < DEFAULT_NUMBER_NODE; j++){
		user_t* cur = server->users->node[j].value;
		if(cur!=NULL){
			Close(cur->socket);
		}
	}
}

int existsInDb(char* name){

	database* cursor = db;
	while(cursor!=NULL){

		if(strcmp(name, cursor->username) == 0){
			return TRUE;
		}

		cursor=cursor->next;
	}

	return FALSE;
}

int writeOutUsers(){
	int ret = 0;

	char buff[MSG_SIZE];
	FILE* dbfp = fopen(DBFILE, "w");

	database* cursor = db;
	while(cursor != NULL){
		if(cursor->username != NULL && cursor->passhash != NULL){
			memset(buff, 0, MSG_SIZE);
			strcat(buff, cursor->username);
			strcat(buff, SPACE);
			strcat(buff, cursor->passhash);
			strcat(buff, "\n");
			fputs(buff, dbfp);
		}
		cursor = cursor->next;
	}

	fclose(server->db_file);

	return ret;
}

int writeOutAUser(char* name, char* pass){
	int ret = 0;

	char buff[MSG_SIZE];
	memset(buff, 0, MSG_SIZE);
	FILE* dbfp = fopen(DBFILE, "a");

	strcat(buff, name);
	strcat(buff, SPACE);
	strcat(buff, pass);
	strcat(buff, "\n");
	fputs(buff, dbfp);

	fclose(dbfp);

	return ret;
}	