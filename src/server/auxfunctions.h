#ifndef AUXFUNCTIONS_H
#define AUXFUNCTIONS_H

#include "udpandtcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <regex.h>

// Commands auxiliar macros
#define INVALID_COMMAND -1
#define REGISTER 1
#define UNREGISTER 2
#define LOGIN 3
#define LOGOUT 4
#define USER_EXIT 5
#define GROUPS_LIST 6
#define SUBSCRIBE 7
#define UNSUBSCRIBE 8
#define USER_GROUPS 9
#define SELECT 10
#define USERS_LIST 11
#define GROUP_POST 12
#define GROUP_RETRIEVE 13
#define SHOW_SELECTED 14
#define SHOW_USER 15

int validRegex(char * buf, char * reg);
void parseArgs(int argc, char * argv[]);
int parseUserCommandUDP(char *command);
int timerOn(int fd);
int timerOff(int fd);

#endif