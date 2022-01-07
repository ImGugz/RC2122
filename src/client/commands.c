/**
 * @file commands.c
 * @author Group 18
 * @brief Reads user's input and executes (or not upon unknown territory) the given command.
 *
 */

#include "commands.h"

#define CMDBUF_SIZE 256    // Arbitrary large constant for buffering input commands
#define MAX_NUM_TOKENS 256 // Textsize 240 + Extras

void processCommands()
{
    int opCmd;
    while (1)
    {
        char command[CMDBUF_SIZE], tmp[CMDBUF_SIZE];
        char *tokenList[MAX_NUM_TOKENS];
        char *token;
        int numTokens = 0;
        printf("USER: ");
        fgets(command, sizeof(command), stdin);
        strtok(command, "\n"); // Command needs to be saved from strtok for POST command
        strcpy(tmp, command);
        // Split all command args into tokens and store it into array
        token = strtok(tmp, " \n");
        while (token != NULL)
        {
            tokenList[numTokens] = token;
            numTokens++;
            token = strtok(NULL, " \n");
        }
        opCmd = parseUserCommand(tokenList[0]);
        if (opCmd == INVALID_COMMAND)
            continue;
        switch (opCmd)
        { // No default case necessary because parseCommand returns INVALID_COMMAND upon unknown command
        case REGISTER:
            userRegister(tokenList, numTokens);
            break;
        case UNREGISTER:
            userUnregister(tokenList, numTokens);
            break;
        case LOGIN:
            userLogin(tokenList, numTokens);
            break;
        case LOGOUT:
            userLogout(tokenList, numTokens);
            break;
        case SHOW_USER:
            showActiveUser(numTokens);
            break;
        case USER_EXIT:
            userExit(numTokens);
            break;
        case GROUPS_LIST:
            showGroups(numTokens);
            break;
        case SUBSCRIBE:
            userGroupSubscribe(tokenList, numTokens);
            break;
        case UNSUBSCRIBE:
            userGroupUnsubscribe(tokenList, numTokens);
            break;
        case USER_GROUPS:
            userGroupsList(numTokens);
            break;
        case SELECT:
            userSelectGroup(tokenList, numTokens);
            break;
        case SHOW_SELECTED:
            showSelectedGroup(numTokens);
            break;
        case USERS_LIST:
            showUsersGroup(numTokens);
            break;
        case GROUP_POST:
            userPostGroup(command);
            break;
        case GROUP_RETRIEVE:
            userRetrieveMsgs(tokenList, numTokens);
            break;

        case DEBUG: // TODO: DELETE
            userDebug(command);
            break;
        }
    }
}