#ifndef AUXFUNCTIONS_H
#define AUXFUNCTIONS_H

#include "udpandtcp.h"
#include "execute-commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <regex.h>
#include <dirent.h>

// Commands auxiliar macros
#define INVALID_COMMAND -1
#define REGISTER 1
#define UNREGISTER 2
#define LOGIN 3
#define LOGOUT 4
#define GROUPS_LIST 5
#define SUBSCRIBE 6
#define UNSUBSCRIBE 7
#define USER_GROUPS 8

int validRegex(char *buf, char *reg);
void parseArgs(int argc, char *argv[]);
int parseUserCommandUDP(char *command);
char *createStatusMessage(char *command, int status);
int remove_directory(const char *path);
int isDirectoryExists(const char *path);
int isCorrectPassword(const char *userDir, const char *userID, const char *pass);
int timerOn(int fd);
int timerOff(int fd);

#endif