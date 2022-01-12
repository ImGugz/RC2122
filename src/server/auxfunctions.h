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

#define MAX_STATUS_SIZE 14
#define MAX_GROUPS 100
#define USERID_SIZE 6
#define USERPWD_SIZE 9
#define MAX_GID_SIZE 3
#define MAX_MID_SIZE 5
#define MAX_GNAME_SIZE 25
#define DIRNAME_SIZE 530
#define USERDIR_SIZE 12
#define GROUPDIR_SIZE 10
#define GROUPMSGDIR_SIZE 14
#define GROUPNEWMSGDIR_SIZE 19
#define GROUPNEWMSGAUT_SIZE 35
#define GROUPNEWMSGTXT_SIZE 31
#define GROUPNEWMSGFILE_SIZE 43
#define GROUPNAMEFILE_SIZE 22
#define GROUPUSERSUBFILE_SIZE 20
#define PATHPWLOGIN_SIZE 32
#define GROUPNAME_SIZE 26
#define INITIAL_ULBUF_SIZE 92 // 32 + 6*10
#define ULCLIENT_BUF_SIZE 7
#define MAX_TEXTSZ_SIZE 4
#define MAX_PSTTEXT_SIZE 242
#define MAX_FILEINFO_SIZE 37
#define MAX_FILENAME_SIZE 25
#define MAX_FILESZ_SIZE 11
#define DIRENTDIR_SIZE 257
#define RTVBUF_SIZE 280

#define MIN(x, y) (((x) < (y)) ? (x) : (y)) // Macro to determine min(x, y)

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
int validFilename(char *fName);
int validRegex(char *buf, char *reg);

void parseArgs(int argc, char *argv[]);
void logVerbose(char *buf, struct sockaddr_in s);
int parseUserCommand(char *command);
void fillGroupsInfo();

char *createMessageInGroup(char *GID, char *UID, char *msgText, int msgTextSize);
int readFile(int fd, char *GID, char *MID, char *fileName, long int fileSize);

int compareIDs(const void *a, const void *b);
int invsort(const struct dirent **a, const struct dirent **b);
int numMessagesToRetrieve(struct dirent **d, int n, char *MID);

char *createStatusMessage(char *command, int status);
char *createSubscribeMessage(int statusCode, char *GID);

int removeDirectory(const char *path);
int directoryExists(const char *path);
int userSubscribedToGroup(char *UID, char *GID);
int passwordsMatch(const char *userID, const char *userPW);
int groupNamesMatch(const char *GID, const char *groupName);
void sortGList(GROUPLIST *list);
void failRetrieve(int fd, char *msg);

void sendTCP(int fd, char *message, int bytes);
int readTCP(int fd, char *message, int maxSize, int flag);
int sendData(int fd, unsigned char *buffer, size_t num);
int sendFile(int fd, FILE *post, long lenFile);
int timerOn(int fd);
int timerOff(int fd);

#endif