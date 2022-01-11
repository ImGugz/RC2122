#ifndef CENTRALIZEDMSGAPI_H
#define CENTRALIZEDMSGAPI_H

#include <regex.h>
#include <stdio.h>
#include <string.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define DEFAULT_DSADDR "127.0.0.1"
#define DEFAULT_DSPORT "58018"
#define ADDR_SIZE 64
#define PORT_SIZE 6

#define INVALID_COMMAND -1
#define REGISTER 1
#define UNREGISTER 2
#define LOGIN 3
#define LOGOUT 4
#define GROUPS_LIST 5
#define SUBSCRIBE 6
#define UNSUBSCRIBE 7
#define USER_GROUPS 8
#define USERS_LIST 9
#define GROUP_POST 10
#define GROUP_RETRIEVE 11

#define MAX_GROUPS 100
#define USERID_SIZE 6

int validPort(char *portStr);
int validRegex(char *buf, char *reg);
int validUID(char *UID);
int validPW(char *PW);
int validGID(char *GID);
int validGName(char *gName);
int validFilename(char *fileName);
int validMID(char *MID);
void closeUDPSocket(struct addrinfo *resUDP, int fdUDP);
void closeTCPSocket(struct addrinfo *resTCP, int fdTCP);

#endif