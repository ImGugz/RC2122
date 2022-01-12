#ifndef EXECUTECOMMANDS_H
#define EXECUTECOMMANDS_H

#include "udpandtcp.h"
#include "auxfunctions.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#define ERR 1
#define OK 2
#define NOK 3
#define DUP 4
#define E_USR 5
#define E_GRP 6
#define E_GNAME 7
#define E_FULL 8
#define NEW 9

int userRegister(char **tokenList, int numTokens);
int userUnregister(char **tokenlist, int numTokens);
int userLogin(char **tokenList, int numTokens);
int userLogout(char **tokenList, int numTokens);
char *createGroupListMessage(char *code, int *groups, int num);
char *createUserGroupsMessage(char **tokenList, int numTokens);
int userSubscribe(char **tokenList, int numTokens, char **newGID);
int userUnsubscribe(char **tokenList, int numTokens);
char *createUsersInGroupMessage(int acceptfd, char *peekedMsg);
char *userPost(int acceptfd, char *peekedMsg, int recvBytes);
void createRetrieveMessage(int acceptfd, char* peekedMsg);

#endif