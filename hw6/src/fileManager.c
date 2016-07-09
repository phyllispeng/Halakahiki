#include "chatroom.h"

uint32_t counter = 0;



char current_dir[MAX_IN];
char* path = NULL;
FILE *log_file;
char final_buffer[200];
char errorBuffer[80];

int readcnt = 0; /* initially = 0 */
sem_t mutex, w; /* Both initially = 1 */

bool isFileValid(char* fileName) {
    FILE* fp;
    fp = fopen(fileName, "r");
    if (fp == NULL) {
        return false;
    } else {
        fclose(fp);
        return true;
    }
}

void initFileStuff(){
    
    int pshared;
    
    int value;
    pshared = 0;
    value = 1;
    sem_init(&mutex, pshared, value); 
    sem_init(&w,pshared,value);
}

void sendFile(file_info* fileToSend, user_t* user) {

    //take file_info: uploader fileName file_id bytes, user LIST_ENTRY(file_info) entry
    //take user_t: name networkInfo is_admin is_file_uploader hash_id(unique id)

    int socket_num = user->socket;
    int size = fileToSend->bytes;
    char* fileName = fileToSend->fileName;


    int srcfd;
    char* srcp;
    char buffer[size];

    srcfd = open(fileName, O_RDONLY, 0);
    srcp = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, srcfd, 0);
    close(srcfd);
    send(socket_num, buffer, size, 0);
    munmap(srcp, size);
}

file_info* receiveFile(user_t* user, char* filename, size_t size) {
    //make a new file_info
    file_info* receivedFile = calloc(1, sizeof (file_info));
    int socket_number = user->socket;

    user->isFileUploader = 1;

    char buffer[size];
    int srcfd;
    recv(socket_number, buffer, size, 0);

    srcfd = open(filename, O_CREAT | O_RDWR, 0);
    write(srcfd, buffer, size);
    close(srcfd);

    //after write into the file
    receivedFile->bytes = size;
    receivedFile->fileName = filename;
    receivedFile->uploader = user;

    return receivedFile;
}

/*
 * open the file check if file exit
 * if yes read the file
 * else send file dne
 *  
 */
void log_file_reader() {
    sem_wait(&mutex);
    readcnt++;
    if (readcnt == 1)//first in
        sem_wait(&w);
    sem_post(&mutex);


    //read from the file here 
    log_file = fopen("auth.log", "r");
    if (log_file != NULL) {
        char result[MAX_IN];
        while (fgets(result, MAX_IN, log_file) != NULL) {
            // creat_history(&A, result);
            printf("%s", result);
        }
        fclose(log_file);

        //TODO add the code !!
    } else {
        printf("file doesn't exist");
    }


    //TODO add the code !!

    sem_wait(&mutex);
    readcnt--;
    if (readcnt == 0)
        sem_post(&w);
    sem_post(&mutex);

}

/*
 * open the file check if file exit
 * if yes write in
 * else make a new one
 *  
 * # 08/23/12 - 12:40am
 * Connection: 192.168.1.1
 * Username: cse220
 * Success
 * 
 * 
 * # 08/23/12 - 12:45am
 * Connection: 172.130.100.3
 * Username: cse320
 * Failure ERR 00
 * 
 * 
 * use this for time size_t strftime(char *s, size_t max, const char *format,const struct tm *tm);
 * 
 */
void log_file_writer(char* ip, int error_num, char* user_name) {
    sem_wait(&w);
    // write to the file here 
    //char *strcat(char *dest, const char *src)
    log_file = fopen("auth.log", "a");
    if (log_file != NULL) {
        // write to the file here 
        //char *strcat(char *dest, const char *src)
        time_t rawtime;
        struct tm *info;
        char buffer[80];
        
        memset(errorBuffer, 0, 80);
        memset(final_buffer, 0, 200);

        time(&rawtime);

        info = localtime(&rawtime);
        // all the time info are in the buffer
        strftime(buffer, 80, "# %x  -  %I:%M%p", info);
        strcat(final_buffer, buffer);
        strcat(final_buffer, "\n");

        char* ip_info = "Connection: ";
        strcat(final_buffer, ip_info);
        strcat(final_buffer, ip);
        strcat(final_buffer, "\n");

        char* username_info = "Username: ";

        strcat(final_buffer, username_info);
        strcat(final_buffer, user_name);
        strcat(final_buffer, "\n");

        if (error_num ==  SUCCESS) {
            char* success = "Success";

            strcat(final_buffer, success);
            strcat(final_buffer, "\n");
            strcat(final_buffer, "\n");

            write(fileno(log_file), final_buffer, strlen(final_buffer));
            fclose(log_file);

        } else {
            //its other error number call the function in pineappleProtocol
            //generateERRorStringPhyllis(char* errorBuffer, int errorType, char* userName)
            char* fail = "Failure: ";
            generateERRorStringPhyllis(errorBuffer, error_num, user_name);
            //char* errorBuffer = "ERR 00 ";

            strcat(final_buffer, fail);
            strcat(final_buffer, errorBuffer);
            strcat(final_buffer, "\n");
            strcat(final_buffer, "\n");

            write(fileno(log_file), final_buffer, strlen(final_buffer));
            fclose(log_file);



        }




    }



    sem_post(&w);
}