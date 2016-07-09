#ifndef client_h
#define client_h

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include "wrap.h"
#include "pineappleProtocol.h"
#include "pass.h"
#include "filemanager.h"

int generateResponse_ClientSide(int connectionFileDescriptor,
                                char* rawClientInputBuffer_SERVER,
                                char* responseBuffer, char* userName, int newUser);

void clientHandler(int connectionFileDescriptor, char* buffer, char* userName);

int clientSRC_SERVER( int connectionFileDescriptor,
                      char* rawClientInputBuffer_SERVER,
                      char* responseBuffer,
                      int bufferSize_SERVER);

int clientSRC_STDIN(int connectionFileDescriptor,
                    char* rawClientInputBuffer_STDIN,
                    char* processedClientInputBuffer_STDIN,
                    size_t bufferSize_STDIN);

void generateMessageString(char* formattedMessageBuffer, char* rawMessageBuffer);

int listHandler(char* givenBuffer);

void quitGracefully(int mySocketFileDescriptor, struct addrinfo **myAddrInfo, char* readBuffer, char* outputBuffer);


#define SLASHCOMMANDS do{\
fprintf(stdout, "\x1B[1;34m[/quit] [/creater] [/listrooms] [/join] [/leave] [/kick]"\
                "\n[/tell] [/listusers] [/help] [/createp] [/joinp]\x1B[0m\n");\
}while(0)


#define USAGE do{ \
fprintf(stderr, "\n./client [-h] NAME SERVER_IP SERVER_PORT"\
				"\n\n\t-e Echo messages received on server's stdout."\
				"\n\t-h Displays help menu & returns EXIT_SUCCESS."\
        "\n\n\tNAME Username to display when chatting."\
				"\n\tSERVER_IP IP address of the server to connect to."\
				"\n\tSERVER_PORT Port to connect to.\n\n"\
				"╦ ╦ ╔═╗ ╦   ╔═╗ ╦╔═ ╔═╗ ╦ ╦ ╦ ╦╔═ ╦\n"\
				"╠═╣ ╠═╣ ║   ╠═╣ ╠╩╗ ╠═╣ ╠═╣ ║ ╠╩╗ ║\n"\
				"╩ ╩ ╩ ╩ ╩═╝ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩ ╩\n\n"\
				);\
}while(0)








#endif
