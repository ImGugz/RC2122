/**
 * @file auxfunctions.h
 * @author Group 18
 * @brief Header file containing all auxiliary functions used throught multiple .c files.
 * 
 */

#ifndef AUXFUNCTIONS_H
#define AUXFUNCTIONS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <ctype.h>

#include "udpandtcp.h"

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

#define DEBUG 16 // TODO: DELETE

int validUID(char *UID);
int validPW(char *PW);
int validGID(char *GID);
int validGName(char *gName);
int validFilename(char *fileName);
int validMID(char *MID);
void parseArgs(int argc, char *argv[]);
int isNumber(char *num);
void printGroups(char *buffer, int numGroups);
int parseUserCommand(char *command);
int sendData(unsigned char *buffer, size_t num);
int sendFile(FILE *post, long lenFile);
void sendTCP(char *message);
int readTCP(char *message, int maxSize, int flag);
void recvFile(char *fileName, long bytesToRead);

#endif