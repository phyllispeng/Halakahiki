#ifndef signalHandlers_h
#define signalHandlers_h

#include <signal.h>
#include <stdlib.h>

void SIGINTHandler( int sig );

void SIGSEGVHandler( int sig );

__sighandler_t Signal(int signum, __sighandler_t handler);


#endif