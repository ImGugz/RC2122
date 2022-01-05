#ifndef EXECUTECOMMANDS_H
#define EXECUTECOMMANDS_H

#include "udpandtcp.h"
#include "auxfunctions.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define OK 1
#define NOK 2
#define DUP 3

int userRegister(char **tokenList, int numTokens);
int userUnregister(char **tokenlist, int numTokens);
int userLogin(char **tokenList, int numTokens);
int userLogout(char **tokenList, int numTokens);

#endif