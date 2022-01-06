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

#define MAX_GROUPS 100
#define MAX_GID_SIZE 3
#define MAX_GNAME_SIZE 25

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

int userRegister(char **tokenList, int numTokens);
int userUnregister(char **tokenlist, int numTokens);
int userLogin(char **tokenList, int numTokens);
int userLogout(char **tokenList, int numTokens);
int listGroups(int numTokens);
int userSubscribe(char **tokenList, int numTokens, char *newGID);
int userUnsubscribe(char **, int);

#endif