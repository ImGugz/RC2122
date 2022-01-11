#include "centralizedmsg-client-api.h"
#include "../centralizedmsg-api.h"
#include "../centralizedmsg-api-constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* DS address and port number */
char addrDS[DS_ADDR_SIZE] = DS_DEFAULT_ADDR;
char portDS[DS_PORT_SIZE] = DS_DEFAULT_PORT;

/**
 * @brief Parses the program's arguments for the DS address and port.
 *
 * @param argc number of arguments (including the executable) given.
 * @param argv arguments given.
 */
static void parseArgs(int argc, char *argv[]);

/**
 * @brief Process all comands given by the client.
 *
 */
void processInput();

int main(int argc, char *argv[])
{
    parseArgs(argc, argv);
    createDSUDPSocket(addrDS, portDS);
    processInput();
    exit(EXIT_SUCCESS);
}

static void parseArgs(int argc, char *argv[])
{
    if (!(argc == 1 || argc == 3 || argc == 5))
    { // Usage: ./user [-n DSIP] [-p DSport]
        fprintf(stderr, "[-] Invalid client program arguments. Usage: ./user [-n DSIP] [-p DSport]\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc - 1; ++i)
    {
        if (argv[i][0] != '-')
        { // If it's not a flag ignore
            continue;
        }
        switch (argv[i][1])
        { // Check all possible flags
        case 'n':
            if (validAddress(argv[i + 1]))
            {
                strcpy(addrDS, argv[i + 1]);
            }
            else
            {
                fprintf(stderr, "[-] Invalid DS hostname/IP address given. Please try again.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'p':
            if (validPort(argv[i + 1]))
            {
                strcpy(portDS, argv[i + 1]);
            }
            else
            {
                fprintf(stderr, "[-] Invalid DS port given. Please try again.\n");
                exit(EXIT_FAILURE);
            }
            break;
        default:
            fprintf(stderr, "[-] Invalid flag given. Usage: ./user [-n DSIP] [-p DSport]\n");
            exit(EXIT_FAILURE);
        }
    }
}

void processInput()
{
    int cmd;
    while (1)
    {
        char command[CLIENT_COMMAND_SIZE];
        char *tokenList[CLIENT_NUMTOKENS];
        char *token;
        int numTokens = 0;
        printf(">>> "); // For user input
        fgets(command, sizeof(command), stdin);
        strtok(command, "\n");
        char commandTok[CLIENT_COMMAND_SIZE]; // We must preserve command so perform token separation here
        strcpy(commandTok, command);
        token = strtok(commandTok, " ");
        if (token[0] == '\n')
        { // User presses enter -> avoid parsing wrong things
            continue;
        }
        while (token)
        {
            tokenList[numTokens++] = token;
            token = strtok(NULL, " ");
        }
        cmd = parseClientCommand(tokenList[0]);
        switch (cmd)
        {
        case REGISTER:
            clientRegister(tokenList, numTokens);
            break;
        }
    }
}