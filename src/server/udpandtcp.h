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
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define DEFAULT_DSADDR "127.0.0.1" // localhost
#define DEFAULT_DSPORT "58018" // 58000 + group ID
#define ADDR_SIZE 64 // maxlen(domain names) + 1
#define PORT_SIZE 6 // len(65535) + 1
#define DEFAULT_LISTENQ 10
#define DEFAULT_TIMEOUT 15

extern int verbose;
extern char portDS[PORT_SIZE];

void setupDSSockets();
void closeUDPSocket();
void closeTCPSocket();

#endif