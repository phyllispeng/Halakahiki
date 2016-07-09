#include "server.h"
#include "threadpool.h"

serverData* server;
database* db;
chatroom_t* waitingRoom;
bool showAllComm;
pthread_mutex_t server_mutex = PTHREAD_MUTEX_INITIALIZER; /* protects the server data */

int main(int argc, char *argv[]) {
    setThreadName("MAIN");
    
    initServer();

    parseArgs(&argc, &argv);

    runServer();

    return EXIT_SUCCESS;
}

void initServer() {
    
    //Set signals
    Signal(SIGINT, SIGINTHandler);
    Signal(SIGSEGV, SIGSEGVHandler);
    
    initFileStuff();

    //Initialize data 
    server = calloc(1, sizeof (serverData));
    db = calloc(1, sizeof (database));
    LIST_INIT(&db->databaseHead);

    waitingRoom = calloc(1, sizeof (chatroom_t));
    LIST_INIT(&waitingRoom->userHead);

    LIST_INIT(&server->chatroomHead);


    /* TODO SPIRO: This is where I read in the files
     * There are three of them :
     * salts.db, hash.db, and users.db
     * There are macros to access these names at the top of the server header
     *
     * users.db has the username list
     * salts.db every 20 bytes has a salt for that given user's password
     * hash.db  every SHA_DIGEST_LENGTH bytes has the hash of that user's (password+salt)
     */

    //This chunk is a bunch of declarations for variables to be used in reading in from the files
    FILE    *user_db,        *salt_db,         *hash_db;                     //File pointers for each file
    size_t  userLen = MSG_SIZE - 1,     saltLen = 20,     hashLen = SHA_DIGEST_LENGTH;  //Length for each type, username Length is variable so set to 0
    ssize_t saltRead,         hashRead;                     // Read in variables to hold return from read and getline

    char* userRead;

    char* userLine = calloc(MSG_SIZE, 1);                            // The pointer to what is read in for the user name
    unsigned char            saltLine[saltLen + 1], hashLine[hashLen + 1]; // Buffers for holding salt and hash

    memset(saltLine, 0, saltLen + 1);
    memset(hashLine, 0, hashLen + 1);


    bool filesValid = isFileValid(USERS_DB) && isFileValid(SALTS_DB) && isFileValid(HASHS_DB);
    // The above expression tests to see if any of the files are valid.
    // If any of them aren't then we don't have proper data to populate the database with
    if (!filesValid) { // Creates file for the first time
        //Creates each file and then closes it because there's nothing to read
        user_db = fopen(USERS_DB, "w");
        salt_db = fopen(SALTS_DB, "w");
        hash_db = fopen(HASHS_DB, "w");
        fclose(user_db);
        fclose(salt_db);
        fclose(hash_db);
    } else {
        // Otherwise let's read the data already present to regenerate the database
        user_db = fopen(USERS_DB, "r");
        salt_db = fopen(SALTS_DB, "r");
        hash_db = fopen(HASHS_DB, "r");
        // Files are open: populate the database
        while (  ((userRead = fgets(userLine, userLen, user_db))   != NULL) // Usernames are separated by "\n"
              && ((saltRead = fread  (saltLine, saltLen, 1, salt_db))  != -1) // hash and salt are not so we read in by byte number
              && ((hashRead = fread  (hashLine, hashLen, 1, hash_db))  != -1) ){

            
            

            // create a new database item
            databaseItem* item = calloc(1, sizeof (databaseItem));

            LIST_INSERT_HEAD((&db->databaseHead), item, entry);

            // Remove the newline from the name
            truncstr(userRead, 1);

           

            // For some reason the username was being overwritten in memory so
            // my fix was to calloc some space and manually copy it over
            // strdup was failing me idk why. // but this worked
            
            // So I duplicate my duplicate here, but now it stays un edited,
            // I'm pretty sure the hash and salt are unaffected but I could be wrong
            strcpy(item->username, userLine);

            

            item->salt =     (unsigned char*)strdup((char*)saltLine);
            item->passhash = (unsigned char*)strdup((char*)hashLine);

            // Insert the data to the linked list that is our database
            

        }
        // Close the files
        fclose(user_db);
        fclose(salt_db);
        fclose(hash_db);
    }


    /* So this is how we read in, at this point I'd recommend checking out the
     * writeOutToDatabase(); function and you can confirm how I am writing out to file
     * you'll see as you explore the code but write out happens after a new user logs in
     *
     * This means that new user's log in info isn't added to the database until restart
     * maybe we should add a databaseItem containing their info at the same time as write-out
     */



}

void parseArgs(int* argcPtr, char*** argvPtr) {

    // -e SHOW MESSAGES TO SERVER
    // -N NUMBER OF CHAT ROOMS
    // -M MAX FILE NUM
    // -F MAX FILE SIZE

    int argc = *argcPtr;
    char** argv = *argvPtr;

    if (argc < 3) {
        fprintf(stderr, "\n\n\e[91mMissing Arguments!\e[0m\n\n");
        USAGE;
        free(server);
        exit(EXIT_FAILURE);
    }

    int opt;
    server->roomLimit = INITIALNUMROOMS;
    server->maxFileSize = INITIAL_FILE_SIZE;
    server->fileCountLimit = INITIAL_FILE_LIMIT;
    showAllComm = false;
    while ((opt = getopt(argc, argv, "heN:")) != -1) {
        switch (opt) {
            case 'F':
                server->maxFileSize = atoi(optarg);
                break;
            case 'M':
                server->fileCountLimit = atoi(optarg);
                break;
            case 'N':
                server->roomLimit = atoi(optarg);
                break;
            case 'h':
                USAGE;
                free(server);
                exit(EXIT_SUCCESS);
                break;
            case 'e':
                showAllComm = true;
                break;
            default: /* '?' */
                USAGE;
                free(server);
                exit(EXIT_FAILURE);
        }
    }
    int argvIndex = optind;

    //argvIndex++;
    char* port = argv[argvIndex];

    argvIndex++;
    char* mssgOfDay = argv[argvIndex];


    //ThreadPool stuff
    argvIndex++;
    char* configFileName = argv[argvIndex];

    uint16_t* configSettings = readConfigFile(configFileName);


    server->min = configSettings[0];
    server->max = configSettings[1];
    server->queueSize = configSettings[2];
    server->secs = configSettings[3];

    server->messageOfTheDay = mssgOfDay;
    server->serverPort = atoi(port);
}

bool incorrectPassword(char* user, char* pass) {

    // TODO Spiro this function is relevant to the hashing work

    Lock(&server_mutex);
    databaseItem* item;
    for (item = db->databaseHead.lh_first; item != NULL; item = item->entry.le_next) {
        if (strcmp(item->username, user) == 0) { // If we have the user's data
            if (checkPassword(pass, item->passhash, item->salt)) { // check their password
                Unlock(&server_mutex);
                return false;
            }
        }
    }
    Unlock(&server_mutex);
    return true;
}

bool userNameAlreadyLoggedIn(char* name) {

    Lock(&server_mutex);
    chatroom_t* chat;
    for (chat = server->chatroomHead.lh_first; chat != NULL; chat = chat->entry.le_next) {
        user_t* userCursor;
        for (userCursor = chat->userHead.lh_first; userCursor != NULL; userCursor = userCursor->entry.le_next) {
            if (strcmp(userCursor->name, name) == 0) {
                Unlock(&server_mutex);
                return true;
            }
        }
    }


    user_t* userCursor;
    for (userCursor = waitingRoom->userHead.lh_first; userCursor != NULL; userCursor = userCursor->entry.le_next) {
        if (strcmp(userCursor->name, name) == 0) {
            Unlock(&server_mutex);
            return true;
        }
    }

    Unlock(&server_mutex);
    return false;
}

bool existsInDb(char* name) {

    // TODO Spiro this function is relevant to the hashing work

    Lock(&server_mutex);
    databaseItem* item;

    for (item = db->databaseHead.lh_first; item != NULL; item = item->entry.le_next) {
        

        if (strcmp(item->username, name) == 0) {
            Unlock(&server_mutex);
            return true;
        }
    }
    Unlock(&server_mutex);
    return false;
}

chatroom_t* getChatroombyID(int id) {
    chatroom_t* chat;
    for (chat = server->chatroomHead.lh_first; chat != NULL; chat = chat->entry.le_next) {

        if (chat->chatroomID == id) {
            return chat;
        }

    }

    return NULL;
}

void fillActiveFileSet(fd_set* set) {

    
    chatroom_t* chat;
    for (chat = server->chatroomHead.lh_first; chat != NULL; chat = chat->entry.le_next) {
        user_t* userCursor;
        for (userCursor = chat->userHead.lh_first; userCursor != NULL; userCursor = userCursor->entry.le_next) {

            FD_SET(userCursor->socket, set);

        }
    }

    user_t* userCursor;
    for (userCursor = waitingRoom->userHead.lh_first; userCursor != NULL; userCursor = userCursor->entry.le_next) {

        FD_SET(userCursor->socket, set);

    }
    
    // Add stdin to the list of fd to check
    FD_SET(fileno(stdin), set);
    FD_SET(server->serverSocket, set);
}

bool roomNameIsTaken(char* roomName) {

    chatroom_t* chat;
    for (chat = server->chatroomHead.lh_first; chat != NULL; chat = chat->entry.le_next) {

        if (strcmp(roomName, chat->name) == 0) {
            return true;
        }

    }

    return false;
}

void runServer() {

    struct sockaddr_in serverAddr; // = calloc(1, sizeof(struct sockaddr_in)); /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */

    //createServerSocket(&echoServAddr);
    server->serverSocket = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    /* Construct local address structure */
    memset(&serverAddr, 0, sizeof (struct sockaddr_in)); /* Zero out structure */
    serverAddr.sin_family = AF_INET; /* Internet address family */
    serverAddr.sin_addr.s_addr = INADDR_ANY; /* Any incoming interface */
    serverAddr.sin_port = htons(server->serverPort); /* Local port */

    /* Bind to the local address */
    Bind(server->serverSocket, (struct sockaddr *) &serverAddr, sizeof (serverAddr));

    /* Mark the socket so it will listen for incoming connections */
    Listen(server->serverSocket, 1);

    int clntSock; /* Socket descriptor for client */
    unsigned int clntLen; /* Length of client address data structure */

    pool_t* pool = pool_create(server->min,server->max, server->secs, NULL);
    
    server->echo_flag = 0;
    int everythingIsFine = true;
    char str[INET_ADDRSTRLEN];
    while (everythingIsFine) {
        /* Set the size of the in-out parameter */
        clntLen = sizeof (echoClntAddr);

        /* Wait for a client to connect */
        clntSock = Accept(server->serverSocket, (struct sockaddr *) &echoClntAddr, &clntLen);
        server->echo_flag++;
        
        memset(str,0,INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &echoClntAddr.sin_addr, str, INET_ADDRSTRLEN);
        if(showAllComm){
            fprintf(stdout, "Accepted on %d from %s\n",clntSock, str);
            fflush(stdout);
        }
        
        struct loginInfo* log = calloc(1, sizeof(struct loginInfo));
        
        log->clntSock = clntSock;
        strcpy(log->ip, str);
        pool_queue(pool, &login, log);
        

        if (server->echo_flag == 1) {
            Thread_create(&server->echoThreadID, NULL, &echoThread, NULL);
        } else if (server->echo_flag == false) {
            // make sure echo tread is dead.
            Thread_cancel(server->echoThreadID);
        }

        sleep(1);
    }
    pool_wait(pool);
    pool_destroy(pool);
}

void* login(void* args) {
    char buf[16];
    snprintf(buf,16, "LOGIN - %d", ct++);
    setThreadName(buf);
    user_t* newUser = calloc(1, sizeof (user_t));
    struct loginInfo* log = (struct loginInfo*) args;
    newUser->socket = log->clntSock;
    newUser->networkInfo = log->ip;

    // Prep for communication
    char in__buff[MSG_SIZE];
    char out_buff[MSG_SIZE];
    memset(in__buff, 0, MSG_SIZE);
    memset(out_buff, 0, MSG_SIZE);

    // Receive ALOHA!
    Recv(newUser->socket, in__buff, MSG_SIZE);
    truncstr(in__buff, strlen(APPEND_END));

    if (!isALOHA(in__buff)) {
        sendError(newUser, GEN_ERROR);
        log_file_writer(newUser->networkInfo, GEN_ERROR, newUser->name);
        sendBYE(newUser);
        Thread_exit(NULL);
    }

    sendAHOLA(newUser);

    // Receive IAM or IAMNEW 
    Recv(newUser->socket, in__buff, MSG_SIZE);

    switch (isNewOrExisting(in__buff)) { //Are they new? Been here before?
        case EXISTS:
            loginExistingUser(newUser, in__buff);
            break;
        case NEW:
            createNewUserAndLogin(newUser, in__buff);
            break;
        case BAD:
            sendError(newUser, GEN_ERROR);
            sendBYE(newUser);
            Thread_exit(NULL);
    }

    sendHI(newUser);

    sendMessageOfTheDay(newUser, server->messageOfTheDay);

    // Insert user into waitingRoom list 
    LIST_INSERT_HEAD(&waitingRoom->userHead, newUser, entry);
    
    log_file_writer(newUser->networkInfo, SUCCESS, newUser->name);
    
    if (server->echo_flag == false) {
        server->echo_flag = true;
    }

    return NULL;
}

void createNewUserAndLogin(user_t* newUser, char* in__buff) {

    char* name;
    name = in__buff + strlen(IAMNEW)+1;
    truncstr(name, strlen(APPEND_END));
    
    // The incoming string has been manipulated so it only holds the username

    // printf("The data in the struct prior to strdup for creating a new user: %s\n", newUser->name);

    newUser->name = strdup(name);

     // printf("The data now in the struct after strdup: %s\n", newUser->name);

    // Check if the user exists on the server
    if (userNameAlreadyLoggedIn(name)) {
        sendError(newUser, SORRY);
        log_file_writer(newUser->networkInfo, SORRY, newUser->name);
        // printf("Already Logged In\n");
        sendBYE(newUser);
        Thread_exit(NULL);
    }
    
    // Check if the user exists in database
    if (existsInDb(name)) {
        sendError(newUser, SORRY);
        log_file_writer(newUser->networkInfo, SORRY, newUser->name);
        // printf("Already exists in db\n");
        sendBYE(newUser);
        Thread_exit(NULL);
    }

    // Everything checks out if we made it here
    SendHINEW(newUser);

    // Get Password 
    Recv(newUser->socket, in__buff, MSG_SIZE);

    // NEWPASS <password>
    char* pass = in__buff;
    pass = pass + 8;
    truncstr(pass, strlen(APPEND_END));

    // the incoming string has been manipulated so we just have their password
    // Phyllis checks if its valid (5 chars, capital, number, symbol)
    if (!valid_password(pass)) {
        sendError(newUser, INVALID_PASSWORD);
        log_file_writer(newUser->networkInfo, INVALID_PASSWORD, newUser->name);
        sendBYE(newUser);
        Thread_exit(NULL);
    }
    
    // This array of two strings will hold our return values
    unsigned char* saltAndHash[3];
    memset(saltAndHash, 0, (3*sizeof(char*)));

    // This function does the salting and hashing
    encryptPassword(pass, saltAndHash);

    // breaking them apart into individual char* so I can pass it to the write-out
    char* salt = (char*)saltAndHash[0];
    char* hash = (char*)saltAndHash[1];

    // Add to the saved database
    writeOutToDatabase(newUser->name, hash, salt);
    // All is swell return to login
}

void writeOutToDatabase(char* name, char* hash, char* salt){

    // Write out happens here it's called 4 lines above in
    // createNewUserAndLogin();

    FILE   *user_db,         *salt_db,         *hash_db;
    user_db = fopen(USERS_DB, "a");// Open for appending
    salt_db = fopen(SALTS_DB, "a");
    hash_db = fopen(HASHS_DB, "a");

    // NAME
    fputs(name, user_db);
    fputs("\n", user_db);
    fclose(user_db);
    // HASH
    fputs(hash, hash_db);
    fclose(hash_db);
    // SALT
    fputs(salt, salt_db);
    fclose(salt_db);

    databaseItem* item = calloc(1, sizeof (databaseItem));

    strcpy(item->username, name);

    printf("The username when writing out to database: %s\n", item->username);

    item->salt =     (unsigned char*)strdup((char*)salt);
    item->passhash = (unsigned char*)strdup((char*)hash);

    LIST_INSERT_HEAD(&db->databaseHead, item, entry);
}

void loginExistingUser(user_t* newUser, char* in__buff) {

    char* name;
    name = in__buff + strlen(IAM) + 1;
    truncstr(name, strlen(APPEND_END));

    // printf("Trying to login existing user: %s", name);

    newUser->name = strdup(name);

    // printf("Still trying to login, copied into the newUser struct, then check if in Db: %s", newUser->name);

    // if(userNameAlreadyLoggedIn(server, waitingRoom)){
    // TODO it shouldn't be possible for this... discuss...
    // }

    if (!existsInDb(name)) {
        sendError(newUser, DOES_NOT_EXIST);
        log_file_writer(newUser->networkInfo, DOES_NOT_EXIST, newUser->name);
        sendBYE(newUser);
        Thread_exit(NULL);
    }

    sendAUTH(newUser);

    // PASS <password>
    Recv(newUser->socket, in__buff, MSG_SIZE);
    char* pass = in__buff;
    pass = pass + 5;
    truncstr(pass, strlen(APPEND_END));

    if (incorrectPassword(newUser->name, pass)){
        sendError(newUser, INVALID_PASSWORD);
        log_file_writer(newUser->networkInfo, INVALID_PASSWORD, newUser->name);
        sendBYE(newUser);
        Thread_cancel(Thread_self());
    }

    // All is well return to login
}

void* echoThread(void* data) {
    setThreadName("ECHO");
    
    fd_set activeFileSet, readFileSet;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 5;

    while (true) {
        FD_ZERO(&activeFileSet);

        fillActiveFileSet(&activeFileSet);

        readFileSet = activeFileSet;

        int sret = Select(FD_SETSIZE, &readFileSet, &tv);

        for (int currentSocket = 0; currentSocket < FD_SETSIZE; currentSocket++) {
            if (sret == 0) break; // Timeout from select! 

            if (FD_ISSET(currentSocket, &readFileSet)) {

                handleClientCommunication(currentSocket);

                FD_CLR(currentSocket, &readFileSet);

            }
        }

    }
}

void handleClientCommunication(int socket) {
    
    if(socket == server->serverSocket){
        return;
    }
    
    char in__buff[MSG_SIZE];
    memset(in__buff, 0, MSG_SIZE);

    Read(socket, in__buff, MSG_SIZE);

    if(strcmp(in__buff, LOGDUMP) != 0){
        user_t* currentUser = findUserBySocket(socket);
        chatroom_t* currentRoom = findChatroomByUser(currentUser);
        char verb[20];
        memset(verb, 0, 20);
        char* temp = &in__buff[0];
        for (int i = 0; i < 20; i++) {
            verb[i] = in__buff[i];
            temp++;
            if (*temp == ' ')break;
        }

        for (int k = 0; k < NUM_COMMANDS; k++) {
            if (strcmp(verb, verbToCommandList[k]) == 0) {
                (*commandFuncs[k])(currentRoom, currentUser, in__buff);
            }
        }
    }else{
        logdump();
    }



    memset(in__buff, 0, MSG_SIZE);
}

chatroom_t* findChatroomByUser(user_t* currentUser) {

    chatroom_t* chat;
    for (chat = server->chatroomHead.lh_first; chat != NULL; chat = chat->entry.le_next) {
        user_t* userCursor;
        for (userCursor = chat->userHead.lh_first; userCursor != NULL; userCursor = userCursor->entry.le_next) {

            if (userCursor == currentUser) {
                return chat;
            }

        }
    }

    user_t* userCursor;
    for (userCursor = waitingRoom->userHead.lh_first; userCursor != NULL; userCursor = userCursor->entry.le_next) {

        if (userCursor == currentUser) {
            return waitingRoom;
        }

    }

    printf("User came up null, that can never happen.\n");
    exit(EXIT_FAILURE);
}

user_t* findUserBySocket(int socket) {

    chatroom_t* chat;
    for (chat = server->chatroomHead.lh_first; chat != NULL; chat = chat->entry.le_next) {
        user_t* userCursor;
        for (userCursor = chat->userHead.lh_first; userCursor != NULL; userCursor = userCursor->entry.le_next) {

            if (userCursor->socket == socket) {
                return userCursor;
            }

        }
    }

    user_t* userCursor;
    for (userCursor = waitingRoom->userHead.lh_first; userCursor != NULL; userCursor = userCursor->entry.le_next) {

        if (userCursor->socket == socket) {
            return userCursor;
        }

    }

    printf("User came up null, that can never happen.\n");
    exit(EXIT_FAILURE);
}

void echo(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // INFORMAT:	MSG <name> <message>
    // OUTFORMAT: 	ECHO <name> <message>

    if (isECHO(inputCommand)) {
        sendError(originUser, 100);
        return;
    }

    // Only allow communication in NON waiting-room chat rooms 
    // Or allow server to communicate anywhere
    if (room != waitingRoom || originUser == server->theServerAsAUser) {

        char* manip = strdup(inputCommand);
        char out_buff[MSG_SIZE];
        memset(out_buff, 0, MSG_SIZE);


        // Obtain the name message with " \r\n"
        // For code readability
        char* message = manip + 4;

        strcat(out_buff, ECHO);
        strcat(out_buff, SPACE);
        strcat(out_buff, originUser->name);
        strcat(out_buff, SPACE);
        strcat(out_buff, message);

        broadcastToRoom(room, out_buff);

    } else {
        sendError(originUser, 60);
    }
}

void leave(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // FORMAT: 		LEAVE 
    // OUTFORMAT:	EVAEL
    char out_buff[MSG_SIZE];

    if (room == waitingRoom) {
        sendError(originUser, USER_NOT_PRESENT);
        return;
    }

    moveFromTo(room, waitingRoom, originUser);

    if (room->userCount == 0) {
        server->roomCount--;
    }

    sendEVAEL(originUser);

    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, ECHO);
    strcat(out_buff, SPACE);
    strcat(out_buff, SERVER);
    strcat(out_buff, SPACE);
    strcat(out_buff, originUser->name);
    strcat(out_buff, LEFTNOTIFY);
    strcat(out_buff, APPEND_END);
    broadcastToRoom(room, out_buff);
}

void createR(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // INFORMAT: 	CREATER <RoomName>
    // OUTFORMAT:	RETAERC <RoomName>
    char out_buff[MSG_SIZE];

    char* newRoomName = inputCommand + 8;
    truncstr(newRoomName, strlen(APPEND_END));

    if (room != waitingRoom) {
        sendError(originUser, INVALID_OPERA);
        return;
    }
    if (roomNameIsTaken(newRoomName)) {
        sendError(originUser, ROOM_EXISTS);
        return;
    }
    if (server->roomCount == server->roomLimit) {
        sendError(originUser, MAX_ROOMS);
        return;
    }

    chatroom_t* newRoom = createChatroom(newRoomName, NULL, NULL ,PUBLIC_TYPE);

    moveFromTo(waitingRoom, newRoom, originUser);

    LIST_INSERT_HEAD(&server->chatroomHead, newRoom, entry);
    server->roomCount++;

    sendRETAERC(originUser, newRoom);

    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, ECHO);
    strcat(out_buff, SPACE);
    strcat(out_buff, SERVER);
    strcat(out_buff, SPACE);
    strcat(out_buff, newRoom->name);
    strcat(out_buff, HASBEENCREATED);
    strcat(out_buff, APPEND_END);
    broadcastToRoom(waitingRoom, out_buff);
}

void listR(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // INFORMAT: 	LISTR
    // OUTFORMAT:	RTSIL

    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    // Not in waiting room
    if (room != waitingRoom) {
        sendError(originUser, INVALID_OPERA);
        return;
    }
    // If no rooms
    if (server->roomCount == 0) {
        char* err = "RTSIL no_rooms -1 \r\n\r\n";
        Send(originUser->socket, err, strlen(err));
        return;
    }

    strcat(out_buff, REV_LISTR);
    strcat(out_buff, SPACE);

    chatroom_t* chat;
    for (chat = server->chatroomHead.lh_first; chat != NULL; chat = chat->entry.le_next) {

        strcat(out_buff, chat->name);
        strcat(out_buff, SPACE);
        char id[70];
        sprintf(id, "%u", chat->chatroomID);
        strcat(out_buff, id);
        strcat(out_buff, APPEND_END);

    }

    strcat(out_buff, END_STRING);

    Send(originUser->socket, out_buff, strlen(out_buff));

}

void join(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // INFORMAT: 	JOIN <id>
    // OUTFORMAT:	NOIJ

    if(room != waitingRoom){
        sendError(originUser, INVALID_OPERA);
        return;
    }
    
    char* manip = strdup(inputCommand);
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);

    manip = manip + 5;
    truncstr(manip, strlen(APPEND_END));

    int id = atoi(manip);
    chatroom_t* wantedroom = getChatroombyID(id);

    if (wantedroom == NULL) {
        sendError(originUser, ROOM_NOT_EXIST);
        return;
    }

    if (wantedroom == room) {
        sendError(originUser, INVALID_OPERA);
        return;
    }

    moveFromTo(room, wantedroom, originUser);

    sendNOIJ(originUser, id);

    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, ECHO);
    strcat(out_buff, SPACE);
    strcat(out_buff, SERVER);
    strcat(out_buff, SPACE);
    strcat(out_buff, originUser->name);
    strcat(out_buff, JOINNOTIFY);
    strcat(out_buff, APPEND_END);

    broadcastToRoom(wantedroom, out_buff);


}

void kick(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // INFORMAT: 	KICK <nameToKick>
    // OUTFORMAT:	KCIK

    if (originUser != room->admin) {
        sendError(originUser, INVALID_OPERA);
        return;
    }

    char* manip = strdup(inputCommand);
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);

    manip = manip + 5;
    truncstr(manip, strlen(APPEND_END));

    char* userToKickName = manip;
    user_t* userToKick = getUserByName(room, userToKickName);
    if(userToKick == NULL){
        sendError(originUser, USER_NOT_PRESENT);
    }

    moveFromTo(room, waitingRoom, userToKick);

    sendKBYE(originUser);

    sendKCIK(originUser, userToKick);
}

void tell(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // FORMAT: 		TELL <nameTo> <message>
    // OUTFORMAT: 	ECHOP <nameFrom> <message>

    char* manip = strdup(inputCommand);
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);

    manip = manip+5;
    truncstr(manip, strlen(APPEND_END));
    
    // temp pointer for iteration
    char* temp = manip;
    while(*temp !=' ')
        temp++;
    *temp = '\0'; // End the manip pointer so it points to <nameTo>
    temp++; // point it at the message
    char* nameToTell = manip;
    char* messageToSend = temp;
    
    user_t* userToTell = getUserByName(room, nameToTell);
    
    if(userToTell == NULL){
        sendError(originUser, USER_NOT_PRESENT);
        return;
    }
    
    sendLLET(originUser, nameToTell, messageToSend);
    
    sendECHOP(userToTell, originUser->name, messageToSend);
}

void listU(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // INFORMAT: 	LISTU
    // OUTFORMAT:	UTSIL

    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    // In anywhere but waiting room
    if (room == waitingRoom) {
        sendError(originUser, INVALID_OPERA);
        return;
    }
    
    strcat(out_buff, REV_LISTU);
    strcat(out_buff, SPACE);
    
    user_t* userCursor;
    for (userCursor = room->userHead.lh_first; userCursor != NULL; userCursor = userCursor->entry.le_next) {

        strcat(out_buff, userCursor->name);
        strcat(out_buff, APPEND_END);

    }
    strcat(out_buff, END_STRING);
    
    Send(originUser->socket, out_buff, strlen(out_buff));
}

void bye(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // INFORMAT: 	BYE
    // OUTFORMAT:	BYE
    
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    
    removeUser(room,originUser);
    sendBYE(originUser);
    
    strcat(out_buff, MSG);
    strcat(out_buff, SPACE);
    strcat(out_buff, SERVER);
    strcat(out_buff, SPACE);
    strcat(out_buff, originUser->name);
    strcat(out_buff, DISCONNECTED);
    
    broadcastToRoom(room, out_buff);
}

void download(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // INFORMAT: 	DOWNLOAD <id> <path>
    // OUTFORMAT:	<file data . . .>

    char* manip = strdup(inputCommand);
        
    manip+=9;
    char* temp = manip;
    while(*temp == ' ')
        temp++;
    *temp = '\0';
    char* idstr = manip;
    unsigned int id = atoi(idstr);
    
    file_info* fileToSend = getFileInfoById(room, id);
    
    if(fileToSend == NULL){
        sendError(originUser, FILE_DOES_NOT_EX);
        return;
    }
    
    sendFile(fileToSend, originUser);
}

void upload(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // INFORMAT: 	UPLOAD <filename> <size>
    // OUTFORMAT:	DAOLPU

    if(server->fileCountLimit == server->fileCount){
        sendError(originUser, FILE_LIMIT_REACHED);
    }
    server->fileCount++;
    
    char out_buff[MSG_SIZE];
    
    
    char* manip = strdup(inputCommand);
    manip = manip + 7;
    truncstr(manip, strlen(APPEND_END));
    
    // temp pointer for iteration
    char* temp = manip;
    while(*temp !=' ')
        temp++;
    *temp = '\0'; // End the manip pointer so it points to <filename>
    temp++; // point it at the message
    char* filename = manip;
    size_t size = atoi(temp);
    
    if(server->maxFileSize < size){
        sendError(originUser, FILE_SIZE_EXCEED);
        return;
    }
    
    file_info* file = receiveFile(originUser, filename, size);
    
    addFileToRoom(room, file); 
    
    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, ECHO);
    strcat(out_buff, SPACE);
    strcat(out_buff, SERVER);
    strcat(out_buff, SPACE);
    strcat(out_buff, filename);
    strcat(out_buff, AVANOTIFY);
    strcat(out_buff, APPEND_END);


    broadcastToRoom(room, out_buff);
}

void delete(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // INFORMAT:	DELETE <id>
    // OUTFORMAT:	ETELED <id>

    char* manip = strdup(inputCommand);
    manip+=9;
    char* temp = manip;
    while(*temp == ' ')
        temp++;
    *temp = '\0';
    char* idstr = manip;
    unsigned int id = atoi(idstr);
    
    file_info* fileToDelete = getFileInfoById(room, id);
    
    if(fileToDelete == NULL){
        sendError(originUser, FILE_DOES_NOT_EX);
        return;
    }
    if(originUser != room->admin){ // Administrator check
        sendError(originUser, FILE_NO_PERMISSION);
        return;
    }else if(strcmp(originUser->name, fileToDelete->uploader->name) != 0){ // Owner check
        sendError(originUser, FILE_NO_PERMISSION);
        return;
    }
  
    removeFileFromRoom(room, fileToDelete);
}

void listf(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // INFORMAT: 	LISTF
    // OUTFORMAT:	FTSIL
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    
    if(room->fileCount == 0){
        strcat(out_buff, REV_LISTF);
        strcat(out_buff, SPACE);
        strcat(out_buff, "-1 no_files");
        strcat(out_buff, APPEND_END);
        strcat(out_buff, END_STRING);
        Send(originUser->socket, out_buff, strlen(out_buff));
        return;
    }
    
    
    
    strcat(out_buff, REV_LISTF);
    file_info* fileCursor;
    for (fileCursor = room->files.lh_first; fileCursor != NULL; fileCursor = fileCursor->entry.le_next) {
        
        char id[70];
        sprintf(id, "%u", fileCursor->file_id);
        strcat(out_buff, id);
        strcat(out_buff, SPACE);
        strcat(out_buff, fileCursor->fileName);
        strcat(out_buff, APPEND_END);

    }
    strcat(out_buff, END_STRING);
    
    Send(originUser->socket, out_buff, strlen(out_buff));
}

void listfu(chatroom_t* room, user_t* originUser, char* inputCommand) {
    // INFORMAT: 	LISTFU
    // OUTFORMAT:	UFTSIL
    char out_buff[MSG_SIZE];
    memset(out_buff, 0, MSG_SIZE);
    
    if(room->fileCount == 0){
        strcat(out_buff, REV_LISTF);
        strcat(out_buff, SPACE);
        strcat(out_buff, "-1 no_files");
        strcat(out_buff, APPEND_END);
        strcat(out_buff, END_STRING);
        Send(originUser->socket, out_buff, strlen(out_buff));
        return;
    }
    
    
    bool nofiles = true;
    strcat(out_buff, REV_LISTF);
    file_info* fileCursor;
    for (fileCursor = room->files.lh_first; fileCursor != NULL; fileCursor = fileCursor->entry.le_next) {
        
        if(strcmp(fileCursor->uploader->name, originUser->name) == 0){
            nofiles = false;
            char id[70];
            sprintf(id, "%u", fileCursor->file_id);
            strcat(out_buff, id);
            strcat(out_buff, SPACE);
            strcat(out_buff, fileCursor->fileName);
            strcat(out_buff, APPEND_END);
        }
        

    }
    strcat(out_buff, END_STRING);
    
    if(nofiles){ // If this user has no files 
        memset(out_buff, 0, strlen(out_buff));
        strcat(out_buff, REV_LISTFU);
        strcat(out_buff, SPACE);
        strcat(out_buff, "-1 no_files");
        strcat(out_buff, APPEND_END);
        strcat(out_buff, END_STRING);
        Send(originUser->socket, out_buff, strlen(out_buff));
        return;
    }
    
    Send(originUser->socket, out_buff, strlen(out_buff));
}

void createP(chatroom_t* room, user_t* originUser, char* inputCommand){
    // INFORTMAT:  CREATEP <RoomName> <password>
    // OUTFORMAT:  PETAERC <RoomName>

    if (room != waitingRoom) {
        sendError(originUser, INVALID_OPERA);
        return;
    }
    if (server->roomCount == server->roomLimit) {
        sendError(originUser, MAX_ROOMS);
        return;
    }

    char out_buff[MSG_SIZE];

    char* manip = inputCommand + 8;
    truncstr(manip, strlen(APPEND_END));

    char* temp = manip;
    while(*temp !=' ')
        temp++;
    *temp = '\0'; // End the manip pointer so it points to <roomName>
    temp++; // point it at the message
    char* newRoomName = manip;

    char* password = temp;

    if (roomNameIsTaken(newRoomName)) {
        sendError(originUser, ROOM_EXISTS);
        return;
    }
    if (!valid_password(password)) {
        sendError(originUser, INVALID_PASSWORD);
        return;
    }

    unsigned char* saltAndHash[3];
    memset(saltAndHash, 0, (3*sizeof(char*)));

    // This function does the salting and hashing
    encryptPassword(password, saltAndHash);

    // breaking them apart into individual char* so I can pass it to the write-out
    char salt[21];
    char hashedPass[21];
    strcpy(salt, (char*)saltAndHash[0]);
    strcpy(hashedPass, (char*)saltAndHash[1]);

    chatroom_t* newRoom = createChatroom(newRoomName, salt, hashedPass, PRIVATE_TYPE);

    LIST_INSERT_HEAD(&server->chatroomHead, newRoom, entry);
    server->roomCount++;

    sendPETAERC(originUser, newRoom);

    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, ECHO);
    strcat(out_buff, SPACE);
    strcat(out_buff, SERVER);
    strcat(out_buff, SPACE);
    strcat(out_buff, newRoom->name);
    strcat(out_buff, PRIVATECREATED);
    strcat(out_buff, APPEND_END);
    broadcastToRoom(waitingRoom, out_buff);
}

void joinP(chatroom_t* room, user_t* originUser, char* inputCommand){
    // INFORTMAT:  JOINP <id> <password>
    // OUTFORMAT:  PNIOJ <id>
    
    if(room != waitingRoom){
        sendError(originUser, INVALID_OPERA);
        return;
    }
    
    char out_buff[MSG_SIZE];
    
    char* manip = inputCommand + 8;
    truncstr(manip, strlen(APPEND_END));
    
    char* temp = manip;
    while(*temp !=' ')
        temp++;
    *temp = '\0'; // End the manip pointer so it points to <id>
    temp++; // point it at the message
    int id = atoi(manip);
    char* password = temp;
    
    chatroom_t* wantedroom = getChatroombyID(id);

    if (wantedroom == NULL) {
        sendError(originUser, ROOM_NOT_EXIST);
        return;
    }    
    
    if(strcmp(password, wantedroom->hashedPass) != 0){
        sendError(originUser, INVALID_PASSWORD);
        return;
    }
    
    moveFromTo(room, wantedroom, originUser);

    sendPNOIJ(originUser, id);

    memset(out_buff, 0, MSG_SIZE);
    strcat(out_buff, ECHO);
    strcat(out_buff, SPACE);
    strcat(out_buff, SERVER);
    strcat(out_buff, SPACE);
    strcat(out_buff, originUser->name);
    strcat(out_buff, JOINNOTIFY);
    strcat(out_buff, APPEND_END);

    broadcastToRoom(wantedroom, out_buff);
}


void logdump(){
    
    printf("Auth Log:\n\n");
    // TODO PhylliChese
    log_file_reader();
    // do anything u want here
    
}

void serverSend(int sockfd, const void *buf, size_t len){
    ssize_t ret = send(sockfd, buf, len, 0);

    if(ret < 0 && errno == EBADF){
        user_t* discon = findUserBySocket(sockfd);
        chatroom_t* room = findChatroomByUser(discon);
        removeUser(room, discon);
        server->echo_flag--;
    }else if(ret<0){
            perror("Failed To Send");
    }

    
}
