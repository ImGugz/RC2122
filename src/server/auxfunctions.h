#ifndef AUXFUNCTIONS_H
#define AUXFUNCTIONS_H

#include "udpandtcp.h"
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
#define USERS_LIST 9
#define GROUP_POST 10
#define GROUP_RETRIEVE 11

#define MAX_GROUPS 100
#define MAX_GID_SIZE 3
#define MAX_UID_SIZE 6
#define MAX_GNAME_SIZE 25
#define DIRNAME_SIZE 530
#define USERDIR_SIZE 12
#define GROUPDIR_SIZE 10
#define GROUPMSGDIR_SIZE 14
#define GROUPNAMEFILE_SIZE 22
#define GROUPUSERSUBFILE_SIZE 20
#define PATHPWLOGIN_SIZE 32
#define USERPW_SIZE 8
#define GROUPNAME_SIZE 26
#define INITIAL_ULBUF_SIZE 92 // 32 + 6*10

#define ERR_MSG "ERR\n"

typedef struct ginfo
{
    char no[MAX_GID_SIZE];
    char name[MAX_GNAME_SIZE];
} GROUPINFO;

typedef struct glist
{
    GROUPINFO groupinfo[MAX_GROUPS];
    int no_groups;
} GROUPLIST;

extern GROUPLIST dsGroups;

int validUID(char *UID);
int validPW(char *PW);
int isGID(char *GID);
int validGID(char *GID);
int validMID(char *MID);
int validGName(char *gName);
int validRegex(char *buf, char *reg);

void parseArgs(int argc, char *argv[]);
void logVerbose(char *buf, struct sockaddr_in s);
int parseUserCommand(char *command);
void fillGroupsInfo();
int compareIDs(const void *a, const void *b);

char *createStatusMessage(char *command, int status);
char *createSubscribeMessage(int statusCode, char *GID);

int removeDirectory(const char *path);
int directoryExists(const char *path);
int passwordsMatch(const char *userID, const char *userPW);
int groupNamesMatch(const char *GID, const char *groupName);
void sortGList(GROUPLIST *list);

int readTCP(int fd, char *message, int maxSize, int flag);
int timerOn(int fd);
int timerOff(int fd);

#endif