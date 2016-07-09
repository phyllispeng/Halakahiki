#include "filemanager.h"

uint32_t counter=0;

DIR* dir;

char current_dir[MAX_IN];
char* path = NULL;
int isFileValid(char* fileName){
  FILE* fp;
  fp = fopen(fileName, "r");
  if(fp == NULL){
    printf("%s\n","invlaid" );
    return FALSE;
  }else{
  printf("%s\n", "valid lol" );

    fclose(fp);
    return TRUE;
  }


}

DIR* build_file_folder(char* folder){

  int result = mkdir(folder,S_IRWXU);
  if (result !=0 ){
    printf("%s\n","fail to make dir" );
  }else{

    //cm->folderName = opendir(path);
    printf("%s\n","you made a dir" );
    dir = opendir(folder);
  }
return dir;
}

void remove_file_folder(char* dir_path){
  int rmdir_result = rmdir(dir_path);
  if (rmdir_result != 0 ){
    printf("%s\n","Fail to remove directory" );
  }else{
    printf("%s\n","Directory deleted" );
  }

}

void recive_file(int socket_num, char* fileName,chatroom_t* cm){
  size_t buf_size = 1024;
  char recieve_buff[buf_size];
  int bytes;
  FILE* fp =  fopen(fileName, "w");
  //recieve
  while( (bytes = recv(socket_num, recieve_buff, buf_size, 0) )> 0) {
    fwrite(recieve_buff, sizeof(char), buf_size, fp);
  }
  fclose(fp);

}
void send_file(int socket_num, int size, char* fileName){
int srcfd;
char* srcp;
char buffer[size];

srcfd = open(fileName, O_RDONLY,0);
srcp = mmap(NULL, size, PROT_READ|PROT_WRITE,MAP_PRIVATE, srcfd,0);
close(srcfd);
send(socket_num, buffer, size, 0);
munmap(srcp, size);
}

void set_file_info(user_t* user, chatroom_t* cm, char* fileName){


  file_info*finfo = malloc(sizeof(struct file_info));
  finfo->user = user;
  finfo->fileName = fileName;
  finfo->file_id = counter;
  finfo->cm = cm;
  counter++;
}

int get_file_id(char* fileName){
return 0;
}

/*void creat_list(file_list* flist, file_info* finfo){
  if(flist = NULL){
    finfo  = flist;
    char
  }
}*/

//should return a list of fildes
void list_files(){
  DIR* dp;
  struct dirent *ep;
  dp = opendir("./");
  if(dp != NULL){
    while((ep = readdir(dp)))
      puts(ep->d_name);

    (void) closedir(dp);
  }else{
    printf("%s\n","Couldnt open the directiory" );
  }
}

void remove_file(user_t* usr, char* fName){
  //call the remove function
  int check_admin = usr->isAdmin;
  int check_file_uploader = usr->isFileUploader;
  if(check_admin == 1 || check_file_uploader ==1){
    //call remove
    int remove_result = remove(fName);
    if(remove_result != 0){
      printf("%s\n","Something happened, you cannot remove it" );
    }else{
      //free(&finfo);
      printf("%s\n","File removed successfully" );
    }



  }else{
    printf("%s\n","You cannot remove the file, jerk" );
  }


}
// int main (int argc, char** argv){
// char* file_folder = "lol";
// //build_file_folder(file_folder);
// remove_file_folder(file_folder);
// char*name = "phyllis";
// char*file = "lol.txt";
// user_t* phyllis = createUser(name, 4);

// //set_admin(phyllis);
// set_file_uploader(phyllis);
// //chatroom_t* cm = createChatRoom("BIU", phyllis);
// //set_file_info(phyllis,  cm, file);
// remove_file(phyllis, file);
// return 0;
//  }
