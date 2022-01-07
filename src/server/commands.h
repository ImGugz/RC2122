#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>
#include <string.h>
#include "auxfunctions.h"
#include "execute-commands.h"

#define MAX_RECVUDP_SIZE 40 // actually it's 39 from GSR (including null terminator)
#define MAX_SENDUDP_SIZE 4096
#define MAX_RECVTCP_SIZE 260 // actually it's 258 from PST (with no file)

void handleUDP(int clientSocket);
void handleTCP(int listenSocket);

#endif