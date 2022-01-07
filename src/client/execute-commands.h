/**
 * @file execute-commands.h
 * @author Group 18
 * @brief Header file containing all commands execution function prototypes.
 *
 */

#ifndef EXECUTECOMMANDS_H
#define EXECUTECOMMANDS_H

#include "udpandtcp.h"
#include "auxfunctions.h"
#include <stdio.h>
#include <string.h>

// Macros and extern variable to be set according to whether the user is logged in or not
#define LOGGED_OUT 0
#define LOGGED_IN 1
extern int userSession;

// Commands
void userRegister(char **tokenList, int numTokens);
void userUnregister(char **tokenList, int numTokens);
void userLogin(char **tokenList, int numTokens);
void userLogout(char **tokenList, int numTokens);
void showActiveUser(int numTokens);
void userExit(int numTokens);
void showGroups(int numTokens);
void userGroupSubscribe(char **tokenList, int numTokens);
void userGroupUnsubscribe(char **tokenList, int numTokens);
void userGroupsList(int numTokens);
void userSelectGroup(char **tokenList, int numTokens);
void showSelectedGroup(int numTokens);
void showUsersGroup(int numTokens);
void userPostGroup(char *command);
void userRetrieveMsgs(char **tokenList, int numTokens);
void userDebug(char *command);

#endif