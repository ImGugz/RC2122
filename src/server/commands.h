#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>
#include <string.h>
#include "auxfunctions.h"
#include "execute-commands.h"

#define MAX_RECVUDP_SIZE 40 // actually it's 39 from GSR (including null terminator)
#define MAX_SENDUDP_SIZE 4096
#define MAX_RECVTCP_SIZE 259 // PST with no FILE

void handleUDP(int clientSocket);
void handleTCP(int listenSocket);

#endif