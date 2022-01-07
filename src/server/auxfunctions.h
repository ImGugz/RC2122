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
#include <netinet/in.h>
#include <arpa/inet.h>

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

#define USERDIR_SIZE 12
#define GROUPDIR_SIZE 10
#define GROUPMSGDIR_SIZE 14
#define GROUPNAMEFILE_SIZE 22
#define GROUPUSERSUBFILE_SIZE 20
#define PATHPWLOGIN_SIZE 32
#define USERPW_SIZE 8
#define GROUPNAME_SIZE 26

#define ERR_MSG "ERR\n"

int validUID(char *UID);
int validPW(char *PW);
int isGID(char *GID);
int validGID(char *GID);
int validMID(char *MID);
int validRegex(char *buf, char *reg);
void parseArgs(int argc, char *argv[]);
void logVerbose(char *buf, struct sockaddr_in s);
int parseUserCommandUDP(char *command);
char *createStatusMessage(char *command, int status);
char *createSubscribeMessage(int statusCode, char *GID);
int removeDirectory(const char *path);
int directoryExists(const char *path);
int passwordsMatch(const char *userID, const char *userPW);
int groupNamesMatch(const char *GID, const char *groupName);
void fillGroupsInfo();
char *createGroupListMessage(char *code, int *groups, int num);
char *createUserGroupsMessage(char **tokenList, int numTokens);
int validGName(char *gName);
int timerOn(int fd);
int timerOff(int fd);

#endif