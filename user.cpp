#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <vector>
#include <string>

using namespace std;

#define DSIP_DEFAULT "127.0.0.1"
#define DSPORT_DEFAULT "58011" // change afterwards to group 18

#define MAX_COMMAND_SIZE 12
#define MAX_IP_SIZE 64 // Including large domain names
#define MAX_PORT_SIZE 6 // 65535 (5+1)
#define MAX_BUFFER_SIZE 128
#define MAX_NUM_TOKENS 4
#define MAX_COMMAND_CODE_SIZE 4
#define UID_SIZE 5
#define PASSWORD_SIZE 8

/* Constants used for switch on commands */
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

int fdDS, errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo hintsDS, *resDS;
struct sockaddr_in addrDS;

char ipDS[MAX_IP_SIZE] = DSIP_DEFAULT;
char portDS[MAX_PORT_SIZE] = DSPORT_DEFAULT;

char * tokenList[MAX_NUM_TOKENS];
int numTokens = 0;
int flag = 0;
char buffer[MAX_BUFFER_SIZE];
char serverMessage[MAX_BUFFER_SIZE];
char commandCode[MAX_COMMAND_CODE_SIZE];
char userID[UID_SIZE];
char userPW[PASSWORD_SIZE];


regex_t regex; int reti;

/* Auxiliary Functions declaration */
void exitProtocol();
void validateInput(int argc, char * argv[]);
void resetVariables();
void socketMount();
int parseUserCommand(char * command);
void interactUDPServer();
void userRegister();
void userUnregister();

/* Main Body */
int main(int argc, char * argv[]) {
    char command[MAX_COMMAND_SIZE];
    int op;
    validateInput(argc, argv);
    socketMount();
    while (scanf("%s", command) != -1) {
        op = parseUserCommand(command);
        char bufferTemp[MAX_BUFFER_SIZE];
        fgets(bufferTemp, sizeof(bufferTemp), stdin);
        strcpy(buffer, bufferTemp);
        strtok(buffer, "\n");
        char * token = strtok(bufferTemp, " \n");
        while (token != NULL) {
            tokenList[numTokens++] = token;
            token = strtok(NULL, " \n");
        }
        switch(op) {
            case REGISTER:
                userRegister();
                break;
            case UNREGISTER:
                userUnregister();
                break;
        }
        interactUDPServer();
        resetVariables();
    }
}

void exitProtocol() {
    freeaddrinfo(resDS);
    close(fdDS);
    fprintf(stderr, "System error. Please try again.\n");
    exit(EXIT_FAILURE);
}

void resetVariables() {
    memset(buffer, 0, sizeof(buffer));
    memset(serverMessage, 0, sizeof(serverMessage));
    memset(commandCode, 0, sizeof(commandCode));
    memset(userID, 0, sizeof(userID));
    memset(userPW, 0, sizeof(userPW));
    memset(tokenList, 0, sizeof(tokenList));
    numTokens = 0;
}

/* Auxiliary Functions implementation */
void validateInput(int argc, char * argv[]) {
    char c;
    if (argc != 1 && argc != 3 && argc != 5) {
        fprintf(stderr, "Invalid input. Please try again.\n");
        exit(EXIT_FAILURE);
    }
    while ((c = getopt(argc, argv, ":n:p:")) != -1) {
        switch(c) {
            case 'n':
                if (strcmp(optarg, "-p") == 0) { // corner case
                    fprintf(stderr, "Missing argument. Please try again.\n");
                    exit(EXIT_FAILURE);
                }
                strcpy(ipDS, optarg);
                break;
            case 'p':
                strcpy(portDS, optarg);
                break;
            case ':':
                fprintf(stderr, "Missing argument. Please try again.\n");
                exit(EXIT_FAILURE);
            case '?':
                printf("Unknown flag. Please try again.\n");
                break;
            default:
                printf("Something is wrong here. Please try again.\n");
                break;
        }
    }
}

void socketMount() {
    fdDS = socket(AF_INET, SOCK_DGRAM, 0);
    if (fdDS == -1) {
        fprintf(stderr, "Error creating Client TCP Socket. Please try again.\n");
        exit(EXIT_FAILURE);
    }
    memset(&hintsDS, 0, sizeof(hintsDS));
    hintsDS.ai_family = AF_INET;
    hintsDS.ai_socktype = SOCK_DGRAM;
    errcode = getaddrinfo(ipDS, portDS, &hintsDS, &resDS);
    if (errcode != 0) {
        fprintf(stderr, "Error getting DS IP info. Please try again.\n");
        exit(EXIT_FAILURE);
    }
    // Estabelish TCP connection as well
    n = connect(fdDS, resDS->ai_addr, resDS->ai_addrlen);
    if (n == -1) {
        fprintf(stderr, "Error conecting to DS. Please try again.\n");
        exit(EXIT_FAILURE);
    }
}

int parseUserCommand(char * command) {
    if (strcmp(command, "reg") == 0) return REGISTER;
    else if ((strcmp(command, "unregister") == 0) || (strcmp(command, "unr") == 0)) return UNREGISTER;
    else if (strcmp(command, "login") == 0) return LOGIN;
    else if (strcmp(command, "logout") == 0) return LOGOUT;
    else if (strcmp(command, "exit") == 0) return USER_EXIT;
    else if ((strcmp(command, "groups") == 0) || (strcmp(command, "gl") == 0)) return GROUPS_LIST;
    else if (strcmp(command, "subscribe") == 0) return SUBSCRIBE;
    else if (strcmp(command, "unsubscribe") == 0) return UNSUBSCRIBE;
    else if ((strcmp(command, "my_groups") == 0) || (strcmp(command, "mgl") == 0)) return USER_GROUPS;
    else if ((strcmp(command, "select") == 0) || (strcmp(command, "sag") == 0)) return SELECT;
    else if ((strcmp(command, "ulist") == 0) || (strcmp(command, "ul") == 0)) return USERS_LIST;
    else if (strcmp(command, "post") == 0) return GROUP_POST;
    else if (strcmp(command, "retrieve") == 0) return GROUP_RETRIEVE;
    else {
        fprintf(stderr, "Invalid user command. Please try again.\n");
        exit(EXIT_FAILURE);
    }
}

void interactUDPServer() {
    memset(buffer, 0, sizeof(buffer)); // Clear to use it for recvfrom
    if (flag) {
        n = sendto(fdDS, serverMessage, strlen(serverMessage), 0, resDS->ai_addr, resDS->ai_addrlen);
        if (n == -1) {
            fprintf(stderr, "Error on sending message ""%s"" to server. Please try again.\n", serverMessage);
            exit(EXIT_FAILURE);
        }
        n = recvfrom(fdDS, buffer, sizeof(buffer), 0, (struct sockaddr *) &addrDS, &addrlen);
        if (n == -1) {
            fprintf(stderr, "Error on receiving message from server. Please try again.\n");
            exit(EXIT_FAILURE);
        }
        write(1, "DS: ", 4); write(1, buffer, n);
        flag = 0;
    }
}

void userRegister() {
    reti = regcomp(&regex, "^ [0-9]{5} [a-zA-Z0-9]{8}$", REG_EXTENDED);
    if (reti) { printf("Error on parsing command. Please try again.\n"); return; }
    if (regexec(&regex, buffer, (size_t) 0, NULL, 0)) { fprintf(stderr, "Invalid register command. Please try again.\n"); return; }
    if (numTokens == 2) {
        sprintf(commandCode, "REG");
    }
    if (strlen(tokenList[0]) != UID_SIZE) {
        printf("Error. Invalid UID with: %s\n", tokenList[0]);
        return;
    }
    strcpy(userID, tokenList[0]);
    if (strlen(tokenList[1]) != PASSWORD_SIZE) {
        printf("Error. Invalid Password with: %s\n", tokenList[1]);
        return;
    }
    strcpy(userPW, tokenList[1]);
    sprintf(serverMessage, "%s %s %s\n", commandCode, userID, userPW);
    flag = 1;
}

void userUnregister() {
    reti = regcomp(&regex, "^ [0-9]{5} [a-zA-Z0-9]{8}$", REG_EXTENDED);
    if (reti) { printf("Error on parsing command. Please try again.\n"); return; }
    if (regexec(&regex, buffer, (size_t) 0, NULL, 0)) { fprintf(stderr, "Invalid register command. Please try again.\n"); return; }
    if (numTokens == 2) {
        sprintf(commandCode, "UNR");
    }
    if (strlen(tokenList[0]) != UID_SIZE) {
        printf("Error. Invalid UID with: %s\n", tokenList[0]);
        return;
    }
    strcpy(userID, tokenList[0]);
    if (strlen(tokenList[1]) != PASSWORD_SIZE) {
        printf("Error. Invalid Password with: %s\n", tokenList[1]);
        return;
    }
    strcpy(userPW, tokenList[1]);
    sprintf(serverMessage, "%s %s %s\n", commandCode, userID, userPW);
    flag = 1;
    // TODO: Servidor deve dar unsubscribe de todos os grupos do utilizador
}