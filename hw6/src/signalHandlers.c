#include "signalHandlers.h"

void SIGINTHandler( int sig ){
    exit(EXIT_FAILURE);
}

void SIGSEGVHandler( int sig ){
    exit(EXIT_FAILURE);
}

__sighandler_t Signal(int signum, __sighandler_t handler){

	return signal(signum,handler);
	
}