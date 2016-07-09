#include "pass.h"


int valid_password(char* password){
  int i; // variable i for loop
  int upper=0;
  int num=0;
  int symbol=0;

  //remove_newline(password);
  int pass_len = strlen(password);

  //printf("pass workd length is : %d\n",pass_len );

  if(pass_len < 5){
    return INVALID;
  }else{
    for(i=0; i<pass_len; i++){
      char ch = password[i];

      if(isupper(password[i])!=0){
        //printf("%s\n","is a upper case" );
        upper++;
      }else if (isdigit(password[i])!=0){
        //printf("%s\n","is a digit" );
        num++;
      }else if(('!'<=ch && ch<='/' )|| (':'<=ch && ch<='@' )|| ('['<=ch && ch<='`') || ('{'<=ch && ch <='~')){
        //printf("%s\n","some symbol" );
        symbol++;
      }
    }
    if(num >= 1 && upper>=1 && symbol>=1){
      //printf("%s\n","valid password yay" );
      return VALID;
    }else{
      //printf("%s\n","inalid password ew" );
      return INVALID;
    }

  }

}
