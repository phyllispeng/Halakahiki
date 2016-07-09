#include "password.h"
#include "parser.h"

bool valid_password(char* password){
	int i; // variable i for loop
	int upper=0;
	int num=0;
	int symbol=0;


	int pass_len = strlen(password);

	// Test for validity

	if(pass_len < 5){
		return false;
	}else{
		for(i=0; i<pass_len; i++){
			char ch = password[i];
			if(isupper(password[i])!=0){
				// Found Uppercase Character
				upper++;
			}else if (isdigit(password[i])!=0){
				// Found Digit character
				num++;
			}else if(  ('!'<=ch && ch<='/' )
					|| (':'<=ch && ch<='@' )
					|| ('['<=ch && ch<='`' )
					|| ('{'<=ch && ch<='~')){
				// Found Symbol character
				symbol++;
			}
		}
		if(num >= 1 && upper>=1 && symbol>=1){
			// At least one of each type, good password
			return true;
		}else{
			// Lacks requirements
			return false;
		}

	}

}

void encryptPassword(char* password, unsigned char** ret){

    // create password buffer

    // create salt buffer
    int saltSize = 20;
    unsigned char salt[saltSize + 1];
    memset(salt, 0, saltSize + 1);

    // create buffer to hold both pass and salt
    int totalSize = strlen(password)+saltSize;
    unsigned char totalbuff[totalSize + 1];
    memset(totalbuff, 0, totalSize + 1);

    // generate salt
    RAND_bytes(salt, saltSize);

    // copy password and salt into total buffer
    strcat((char*)totalbuff, (char*) password);
    strcat((char*)totalbuff, (char*) salt);

    // create hash buffer
    unsigned char hash[SHA_DIGEST_LENGTH + 1];
    memset(hash, 0, SHA_DIGEST_LENGTH + 1);

    // Hash total buffer, and put hash into "hash"
    SHA1(totalbuff, totalSize, hash);			//Spiro Note To Self: Review this if needed
		//unsigned char *SHA1(const unsigned char *d, unsigned long n, unsigned char *md);


    // return the salt and hash to the calling function's array
    ret[0]=(unsigned char*)strdup((char*)salt);
    ret[1]=(unsigned char*)strdup((char*)hash);


}

void decryptPassword(char* passAttempt, unsigned char* hash, unsigned char* salt){

}

void unsignPassword(char* password, unsigned char* passUnsign){

    for(int i = 0 ; i < strlen(password); i++){
        passUnsign[i] = (unsigned char)password[i];
    }

}

bool checkPassword(char* passAttempt, unsigned char* savedhash, unsigned char* salt){

    // Create a buffer so we can stick together (passAttempt+salt)
    size_t bufsize = strlen(passAttempt)+strlen((char*)salt); // len(passAttempt) + 20
    char buff[bufsize + 1];
    memset(buff, 0, bufsize + 1);


    // put them both into the buffer
    strcat(buff, passAttempt);
    strcat(buff, (char*) salt);

    // Same as before create a place to put the hash
    unsigned char testhash[SHA_DIGEST_LENGTH + 1];
    memset(testhash, 0, SHA_DIGEST_LENGTH + 1);

    // Hash! (passAttempt+salt)
    SHA1((unsigned char*)buff, bufsize, testhash); 		//Spiro note to self: check later if needed

    // If the testHash on (passAttempt+salt) is equal to the saved hash
    // then we have a match and the user logged in correctly
    // otherwise nope.
    if(strcmp((char*)testhash, (char*)savedhash) == 0){
        return true;
    }else{
        return false;
    }

}
