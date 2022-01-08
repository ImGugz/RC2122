/**
 * @file udpandtcp.h
 * @author Group 18
 * @brief Header file containing both setup and message exchange via TCP and UDP sockets function prototypes
 * as well as important macros.
 * 
 */

#ifndef UDPANDTCP_H
#define UDPANDTCP_H

#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "execute-commands.h"

#define DEFAULT_DSADDR "127.0.0.1" // localhost
#define DEFAULT_DSPORT "58018"     // 58000 + group ID
#define ADDR_SIZE 64               // maxlen(domain names) + 1
#define PORT_SIZE 6                // len(65535) + 1

extern int fdDSTCP; // File descriptor for TCP socket to be used in auxiliary functions
extern char addrDS[ADDR_SIZE];
extern char portDS[PORT_SIZE];

// Setup functions
void createUDPSocket();

// UDP related functions
void exchangeUDPMsg(char *message);
void closeUDPSocket();

// TOOD: DELETE THIS
void logTCPServer(char *message);

// TCP related functions
void exchangeTCPMsg(char *message);
void exchangeTCPPost(char *message, FILE *toPost, long lenFile);
void exchangeTCPRet(char *message);
void closeTCPSocket();

#endif