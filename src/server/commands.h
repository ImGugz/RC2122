#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>
#include <string.h>
#include "auxfunctions.h"
#include "execute-commands.h"

#define MAX_RECVUDP_SIZE 4096
#define MAX_RECVTCP_SIZE 512

void handleUDP(int clientSocket);
void handleTCP(int listenSocket);

#endif