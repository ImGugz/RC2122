#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <map>
#include <string>

using namespace std;

#define DSIP_DEFAULT "127.0.0.1"
#define DSPORT_DEFAULT "58011" // change afterwards to group 18
#define USER_LOGGEDOUT 0
#define USER_LOGGEDIN 1
#define FALSE 0
#define TRUE 1

#define SUCCESS 99
#define FAILURE -3

#define MAX_COMMAND_SIZE 12
#define MAX_IP_SIZE 64 // Including large domain names
#define MAX_PORT_SIZE 6 // 65535 (5+1)
#define MAX_BUFFER_SIZE 128
#define MAX_NUM_TOKENS 4
#define MAX_COMMAND_CODE_SIZE 4
#define MAX_COMMAND_STATUS_SIZE 6
#define UID_SIZE 6
#define PASSWORD_SIZE 8
#define GROUPID_SIZE 2
#define MAX_GROUPNAME_SIZE 24
#define MSGID_SIZE 4
#define MAX_GROUPS 100
#define MAX_GROUP_STRING_SIZE 3307 // RGL + backspace + N(max is 99) + 100 * (backspace + GID(max is 99) + backspace + GName(max 24) + backspace + MID(max 9999)) 

/* Constants used for switch on commands */
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

int fdDSUDP, fdDSTCP, errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo hintsDS, *resDSUDP, *resDSTCP;
struct sockaddr_in addrDS;

char ipDS[MAX_IP_SIZE] = DSIP_DEFAULT;
char portDS[MAX_PORT_SIZE] = DSPORT_DEFAULT;
int userSession = USER_LOGGEDOUT;
int groupSelected = FALSE;

char * tokenList[MAX_NUM_TOKENS];
int numTokens = 0;
int udpFlag = 0;
char buffer[MAX_BUFFER_SIZE];
char serverMessage[MAX_BUFFER_SIZE];
char commandCode[MAX_COMMAND_CODE_SIZE];
char serverResponseCode[MAX_COMMAND_CODE_SIZE];
char serverResponseStatus[MAX_COMMAND_STATUS_SIZE];

char userID[UID_SIZE];
char userPW[PASSWORD_SIZE];
char userGroupID[MAX_NUM_TOKENS]; // we use this macro to avoid strcpy overflow
char userGroupName[MAX_GROUPNAME_SIZE];

char activeUser[UID_SIZE];
char activePassword[PASSWORD_SIZE];
char activeGroup[GROUPID_SIZE];

regex_t regex; int reti;

/* Auxiliary Functions declaration */
void exitProtocol();
void validateInput(int argc, char * argv[]);
void resetVariables();
void socketMount();
int parseUserCommand(char * command);

int startTCP();

void interactUDPServer();
void interactUDPServerGroups();

void registerServerFeedback();
void unregisterServerFeedback();
void loginServerFeedback();
void logoutServerFeedback();
void subscribeServerFeedback();
void unsubscribeServerFeedback();

void userRegister();
void userUnregister();
void userLogin();
void userLogout();
void userExit();
void showGroups();
void showUsers();
void userGroupSubscribe();
void userGroupUnsubscribe();
void userGroupsList();
void userSelectGroup();
void selectServerFeedback();
void showSelectedGroup();
void showActiveUser();

/* Main Body */
int main(int argc, char * argv[]) {
    char command[MAX_COMMAND_SIZE];
    int op;
    validateInput(argc, argv);
    socketMount();
    while (scanf("%s", command) != -1) {
        op = parseUserCommand(command);
        if(op==INVALID_COMMAND) continue;
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
                registerServerFeedback();
                break;
            case UNREGISTER:
                userUnregister();
                unregisterServerFeedback();
                break;
            case LOGIN:
                userLogin();
                loginServerFeedback();
                break;
            case LOGOUT:
                userLogout();
                logoutServerFeedback();
                break;
            case USER_EXIT:
                userExit();
                break;
            case GROUPS_LIST:
                showGroups();
                break;
            case USERS_LIST:
                showUsers();
                break;
            case SUBSCRIBE:
                userGroupSubscribe();
                subscribeServerFeedback();
                break;
            case UNSUBSCRIBE:
                userGroupUnsubscribe();
                unsubscribeServerFeedback();
                break;
            case USER_GROUPS:
                userGroupsList();
                break;
            case SELECT:
                userSelectGroup();
                break;
            case SHOW_SELECTED:
                showSelectedGroup();
                break;
            case SHOW_USER:
                showActiveUser();
                break;
            default:
                //extra safety check
                fprintf(stderr, "Invalid command. Please try again.\n");
        }
        resetVariables();
    }
}

void exitProtocol() {
    freeaddrinfo(resDSUDP);
    freeaddrinfo(resDSTCP);
    close(fdDSUDP);
    close(fdDSTCP);
    fprintf(stderr, "System error. Please try again.\n");
    exit(EXIT_FAILURE);
}

void resetVariables() {
    memset(tokenList, 0, sizeof(tokenList));
    numTokens = 0;
    memset(buffer, 0, sizeof(buffer));
    memset(serverMessage, 0, sizeof(serverMessage));
    memset(commandCode, 0, sizeof(commandCode));
    memset(userID, 0, sizeof(userID));
    memset(userPW, 0, sizeof(userPW));
    memset(userGroupID, 0, sizeof(userGroupID));
    memset(userGroupName, 0, sizeof(userGroupName));
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
                if (strcmp(optarg, "-n") == 0) { // corner case
                    fprintf(stderr, "Missing argument. Please try again.\n");
                    exit(EXIT_FAILURE);
                }
                strcpy(portDS, optarg);
                break;
            case ':':
                fprintf(stderr, "Missing argument. Please try again.\n");
                exit(EXIT_FAILURE);
            case '?':
                printf("Unknown flag. Please try again.\n");
                exit(EXIT_FAILURE);
            default:
                printf("Something is wrong here. Please try again.\n");
                exit(EXIT_FAILURE);
        }
    }
}

void socketMount() {
    fdDSUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (fdDSUDP == -1) {
        fprintf(stderr, "Error creating Client UDP Socket. Please try again.\n");
        exit(EXIT_FAILURE);
    }
    memset(&hintsDS, 0, sizeof(hintsDS));
    hintsDS.ai_family = AF_INET;
    hintsDS.ai_socktype = SOCK_DGRAM;
    errcode = getaddrinfo(ipDS, portDS, &hintsDS, &resDSUDP);
    if (errcode != 0) {
        fprintf(stderr, "Error getting DS IP info. Please try again.\n");
        close(fdDSUDP);
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
    else if (strcmp(command, "subscribe") == 0 || strcmp(command, "s") == 0) return SUBSCRIBE;
    else if (strcmp(command, "unsubscribe") == 0 || strcmp(command, "u") == 0) return UNSUBSCRIBE;
    else if ((strcmp(command, "my_groups") == 0) || (strcmp(command, "mgl") == 0)) return USER_GROUPS;
    else if ((strcmp(command, "select") == 0) || (strcmp(command, "sag") == 0)) return SELECT;
    else if ((strcmp(command, "ulist") == 0) || (strcmp(command, "ul") == 0)) return USERS_LIST;
    else if (strcmp(command, "post") == 0) return GROUP_POST;
    else if (strcmp(command, "retrieve") == 0) return GROUP_RETRIEVE;
    else if ((strcmp(command, "showgid") == 0) || (strcmp(command, "sg") == 0)) return SHOW_SELECTED;
    else if ((strcmp(command, "showuid") == 0) || (strcmp(command, "su") == 0)) return SHOW_USER;
    else {
        fprintf(stderr, "Invalid user command code. Please try again.\n");
        return INVALID_COMMAND;
    }
}

void interactUDPServer() {
    memset(buffer, 0, sizeof(buffer)); // Clear to use it for recvfrom
    if (udpFlag) {
        n = sendto(fdDSUDP, serverMessage, strlen(serverMessage), 0, resDSUDP->ai_addr, resDSUDP->ai_addrlen);
        if (n == -1) {
            fprintf(stderr, "Error on sending message ""%s"" to server. Please try again.\n", serverMessage);
            return;
        }
        n = recvfrom(fdDSUDP, buffer, sizeof(buffer), 0, (struct sockaddr *) &addrDS, &addrlen);
        if (n == -1) {
            fprintf(stderr, "Error on receiving message from server. Please try again.\n");
            return;
        }
        write(1, "DS: ", 4); write(1, buffer, n); // TO REMOVE AFTERWARDS -> ONLY SERVES FEEDBACK DEBUGGING PURPOSES
    }
}

void userRegister() {
    reti = regcomp(&regex, "^ [0-9]{5} [a-zA-Z0-9]{8}$", REG_EXTENDED);
    if (reti) { printf("Error on parsing command. Please try again.\n"); return; }
    if (regexec(&regex, buffer, (size_t) 0, NULL, 0)) { fprintf(stderr, "Invalid register command. Please try again.\n"); return; }
    if (numTokens == 2) {
        sprintf(commandCode, "REG");
    }
    puts(tokenList[0]);
    puts(tokenList[1]);
    strcpy(userID, tokenList[0]);
    strcpy(userPW, tokenList[1]);
    sprintf(serverMessage, "%s %s %s\n", commandCode, userID, userPW);
    udpFlag = 1;
    interactUDPServer();
}

void registerServerFeedback() {
    if (udpFlag) {
        sscanf(buffer, "%s %s", serverResponseCode, serverResponseStatus);
        if (strcmp(serverResponseCode, "RRG")) exitProtocol();
        if (strcmp(serverResponseStatus, "OK") && strcmp(serverResponseStatus, "DUP") && strcmp(serverResponseStatus, "NOK")) exitProtocol();
        if (!strcmp(serverResponseStatus, "OK")) printf("User %s succesfully registered with password %s.\n", userID, userPW);
        if (!strcmp(serverResponseStatus, "DUP")) printf("User %s is already registered.\n", userID);
        if (!strcmp(serverResponseStatus, "NOK")) printf("User %s failed to register. Please try again later.\n", userID);
        udpFlag = 0;
    }
}

void userUnregister() {
    reti = regcomp(&regex, "^ [0-9]{5} [a-zA-Z0-9]{8}$", REG_EXTENDED);
    if (reti) { printf("Error on parsing command. Please try again.\n"); return; }
    if (regexec(&regex, buffer, (size_t) 0, NULL, 0)) { fprintf(stderr, "Invalid unregister command. Please try again.\n"); return; }
    if (numTokens == 2) {
        sprintf(commandCode, "UNR");
    }
    strcpy(userID, tokenList[0]);
    strcpy(userPW, tokenList[1]);
    sprintf(serverMessage, "%s %s %s\n", commandCode, userID, userPW);
    udpFlag = 1;
    interactUDPServer();
    // TODO: Servidor deve dar unsubscribe de todos os grupos do utilizador
}

void unregisterServerFeedback() {
    if (udpFlag) {
        sscanf(buffer, "%s %s", serverResponseCode, serverResponseStatus);
        if (strcmp(serverResponseCode, "RUN")) exitProtocol();
        if (strcmp(serverResponseStatus, "OK") && strcmp(serverResponseStatus, "NOK")) exitProtocol();
        if (!strcmp(serverResponseStatus, "OK")) printf("User %s succesfully unregistered.\n", userID);
        if (!strcmp(serverResponseStatus, "NOK")) printf("Unregister request failed. Please check the given UID and pass.\n");
        udpFlag = 0;
    }
}

void userLogin() {
    reti = regcomp(&regex, "^ [0-9]{5} [a-zA-Z0-9]{8}$", REG_EXTENDED);
    if (reti) { printf("Error on parsing command. Please try again.\n"); return; }
    if (regexec(&regex, buffer, (size_t) 0, NULL, 0)) { fprintf(stderr, "Invalid login command. Please try again.\n"); return; }
    if (userSession == USER_LOGGEDIN) { fprintf(stderr, "User is already logged in. Please log out before you try to log in.\n"); return; }
    if (numTokens == 2) {
        sprintf(commandCode, "LOG");
    }
    strcpy(userID, tokenList[0]);
    strcpy(userPW, tokenList[1]);
    sprintf(serverMessage, "%s %s %s\n", commandCode, userID, userPW);
    udpFlag = 1;
    interactUDPServer();
}

void loginServerFeedback() {
    if (udpFlag) {
        sscanf(buffer, "%s %s", serverResponseCode, serverResponseStatus);
        if (strcmp(serverResponseCode, "RLO")) exitProtocol();
        if (strcmp(serverResponseStatus, "OK") && strcmp(serverResponseStatus, "NOK")) exitProtocol();
        if (!strcmp(serverResponseStatus, "OK")) {
            userSession = USER_LOGGEDIN; 
            strcpy(activeUser, userID); 
            strcpy(activePassword, userPW);
            printf("User %s succesfully logged in.\n", activeUser);
        }
        if (!strcmp(serverResponseStatus, "NOK")) printf("Please check login credentials and try again.\n");
        udpFlag = 0;
    }
}

void userLogout() {
    sprintf(commandCode, "OUT");
    sprintf(serverMessage, "%s %s %s\n", commandCode, activeUser, activePassword);
    udpFlag = 1;
    interactUDPServer();
    userSession = USER_LOGGEDOUT;
    memset(activePassword, 0, sizeof(activePassword));
}

void logoutServerFeedback() {
    if (udpFlag) {
        sscanf(buffer, "%s %s", serverResponseCode, serverResponseStatus);
        if (strcmp(serverResponseCode, "ROU")) exitProtocol();
        if (strcmp(serverResponseStatus, "OK") && strcmp(serverResponseStatus, "NOK")) exitProtocol();
        if (!strcmp(serverResponseStatus, "OK")) printf("User %s succesfully logged out.\n", activeUser);
        if (!strcmp(serverResponseStatus, "NOK")) printf("User %s logout failed. Please try again.\n", activeUser);
        // To be able to know who logged out only reset af
        memset(activeUser, 0, sizeof(activeUser));
        udpFlag = 0;
    }
}

void userExit() {
    freeaddrinfo(resDSUDP);
    freeaddrinfo(resDSTCP);
    close(fdDSUDP);
    // TODO: Check TCP
    exit(EXIT_SUCCESS);
}

void interactUDPServerGroups() {
    char groupBufferTemp[MAX_GROUP_STRING_SIZE];
    char * groupBuffer, * token;
    int numGroups;
    char * groupsList[3*MAX_GROUPS];
    int aux = 0;
    n = sendto(fdDSUDP, serverMessage, strlen(serverMessage), 0, resDSUDP->ai_addr, resDSUDP->ai_addrlen);
    if (n == -1) {
        fprintf(stderr, "Error on sending message ""%s"" to server. Please try again.\n", serverMessage);
        return;
    }
    n = recvfrom(fdDSUDP, groupBufferTemp, sizeof(groupBufferTemp), 0, (struct sockaddr *) &addrDS, &addrlen);
    if (n == -1) {
        fprintf(stderr, "Error on sending message ""%s"" to server. Please try again.\n", serverMessage);
        return;
    }
    sscanf(groupBufferTemp, "%s %d ", serverResponseCode, &numGroups);
    if (strcmp(serverResponseCode, "RGL") && strcmp(serverResponseCode, "RGM")) exitProtocol();
    printf("%d groups: (GID | GName | Last MID)\n", numGroups);
    if (numGroups == 0) { return; }
    groupBuffer = (numGroups >= 10) ? groupBufferTemp + 7 : groupBufferTemp + 6;
    token = strtok(groupBuffer, " \n");
    while (token != NULL && aux < 3 * MAX_GROUPS) {
        groupsList[aux++] = token;
        token = strtok(NULL, " \n");
    }
    for (int i = 0; i < aux-1; i += 3) {
        if ((strlen(groupsList[i]) != 2) || (strlen(groupsList[i+1]) > 24) || (strlen(groupsList[i+2]) != 4)) exitProtocol();
        printf("%s %s %s\n", groupsList[i], groupsList[i+1], groupsList[i+2]);
    }
    udpFlag = 0;
}

void showGroups() {
    sprintf(commandCode, "GLS");
    sprintf(serverMessage, "%s\n", commandCode);
    udpFlag = 1;
    interactUDPServerGroups();
}

void userGroupSubscribe() {
    reti = regcomp(&regex, "^ [0-9]{1,2} [a-zA-Z0-9_-]{1,24}$", REG_EXTENDED);
    if (reti) { printf("Error on parsing command arguments. Please try again.\n"); return; }
    if (regexec(&regex, buffer, (size_t) 0, NULL, 0)) { fprintf(stderr, "Invalid subscribe command. Please try again.\n"); return; }
    if (userSession == USER_LOGGEDOUT) { fprintf(stderr, "Please login before you subscribe to a group.\n"); return; }
    if (numTokens == 2) {
        sprintf(commandCode, "GSR");
    }
    if (!strcmp(tokenList[0], "00")) { fprintf(stderr, "This group doesn't exist. Please try again.\n"); return; }
    if (!strcmp(tokenList[0], "0")) strcpy(userGroupID, "00");
    else strcpy(userGroupID, tokenList[0]);
    strcpy(userGroupName, tokenList[1]);
    sprintf(serverMessage, "%s %s %s %s\n", commandCode, activeUser, userGroupID, userGroupName);
    udpFlag = 1;
    interactUDPServer();
}

void subscribeServerFeedback() {
    if (udpFlag) {
        sscanf(buffer, "%s %s", serverResponseCode, serverResponseStatus);
        if (strcmp(serverResponseCode, "RGS")) exitProtocol();
        if (strcmp(serverResponseStatus, "NEW") && strcmp(serverResponseStatus, "E_USR") 
        && strcmp(serverResponseStatus, "E_GRP") && strcmp(serverResponseStatus, "E_GNAME")
        && strcmp(serverResponseStatus, "E_FULL") && strcmp(serverResponseStatus, "OK")
        && strcmp(serverResponseStatus, "NOK")) exitProtocol();
        if (!strcmp(serverResponseStatus, "OK")) printf("User %s has succesfully subscribed to %s.\n", activeUser, userGroupName);
        if (!strcmp(serverResponseStatus, "NEW")) printf("Group %s has been succesfully created.\n", userGroupName);
        if (!strcmp(serverResponseStatus, "E_USR")) printf("User ID %s that is logged in is invalid. Please try again.\n", activeUser);
        if (!strcmp(serverResponseStatus, "E_GRP")) printf("Group ID %s is invalid. Please try again.\n", userGroupID);
        if (!strcmp(serverResponseStatus, "E_GNAME")) printf("Group name %s is invalid. Please try again.\n", userGroupName);
        if (!strcmp(serverResponseStatus, "E_FULL")) printf("Maximum of 100 groups has been reached. Please try again later.\n");
        if (!strcmp(serverResponseStatus, "NOK")) printf("The subscribing process has failed. Please try again later.\n");
        udpFlag = 0;
    }
}

void userGroupUnsubscribe() {
    reti = regcomp(&regex, "^ [0-9]{2}$", REG_EXTENDED);
    if (reti) { printf("Error on parsing command arguments. Please try again.\n"); return; }
    if (regexec(&regex, buffer, (size_t) 0, NULL, 0)) { fprintf(stderr, "Invalid unsubscribe command. Please try again.\n"); return; }
    if (userSession == USER_LOGGEDOUT) { fprintf(stderr, "Please login before you unsubscribe to a group.\n"); return; }
    if (numTokens == 1) {
        sprintf(commandCode, "GUR");
    }
    strcpy(userGroupID, tokenList[0]);
    sprintf(serverMessage, "%s %s %s\n", commandCode, activeUser, userGroupID);
    udpFlag = 1;
    interactUDPServer();
}

void unsubscribeServerFeedback() {
    if (udpFlag) {
        sscanf(buffer, "%s %s", serverResponseCode, serverResponseStatus);
        if (strcmp(serverResponseCode, "RGU")) exitProtocol();
        if (strcmp(serverResponseStatus, "OK") && strcmp(serverResponseStatus, "E_USR") 
        && strcmp(serverResponseStatus, "E_GRP") && strcmp(serverResponseStatus, "NOK")) exitProtocol();
        if (!strcmp(serverResponseStatus, "OK")) printf("User %s has succesfully unsubscribed to group with ID %s.\n", activeUser, userGroupID);
        if (!strcmp(serverResponseStatus, "NEW")) printf("Group %s has been succesfully created.\n", userGroupName);
        if (!strcmp(serverResponseStatus, "E_USR")) printf("User ID %s that is logged in is invalid. Please try again.\n", activeUser);
        if (!strcmp(serverResponseStatus, "E_GRP")) printf("Group ID %s is invalid. Please try again.\n", userGroupID);
        if (!strcmp(serverResponseStatus, "NOK")) printf("The unsubscribing process has failed. Please try again later.\n");
        udpFlag = 0;
    }
}

void userGroupsList() {
    if (userSession == USER_LOGGEDOUT) { fprintf(stderr, "Please login before requesting user's groups.\n"); return; }
    sprintf(commandCode, "GLM");
    sprintf(serverMessage, "%s %s\n", commandCode, activeUser);
    udpFlag = 1;
    interactUDPServerGroups();
}

void userSelectGroup() {
    reti = regcomp(&regex, "^ [0-9]{2}$", REG_EXTENDED);
    if (reti) { printf("Error on parsing command arguments. Please try again.\n"); return; }
    if (regexec(&regex, buffer, (size_t) 0, NULL, 0)) { fprintf(stderr, "Invalid select command. Please try again.\n"); return; }
    if (userSession == USER_LOGGEDOUT) { fprintf(stderr, "Please login before you select a group.\n"); return; }
    if (numTokens == 1) { strcpy(activeGroup, tokenList[0]); groupSelected = TRUE; printf("Group %s selected.\n", activeGroup); }
    else exitProtocol();
}

void showSelectedGroup() {
    printf("Selected Group: %s\n", activeGroup);
}

void showActiveUser() {
    printf("Active User ID: %s\n", activeUser);
}

void showUsers() {
    memset(buffer, 0, sizeof(buffer));

    if (startTCP() != SUCCESS) return;
    sprintf(serverMessage, "ULS %s\n", activeGroup);

    n = write(fdDSTCP, serverMessage, strlen(serverMessage));
    if(n == -1) {
        fprintf(stderr, "Error sending command to server. Please try again.\n");
        close(fdDSTCP);
        return;
    }

    n = read(fdDSTCP, buffer, sizeof(buffer));
    sscanf(buffer, "%s %s", serverResponseCode, serverResponseStatus);
    if (strcmp(serverResponseCode, "RUL") != 0) {
        fprintf(stderr, "Error reading response from server. Please try again.\n");
        close(fdDSTCP);
        return;
    }

    if (strcmp(serverResponseStatus, "NOK") == 0){
        printf("Selected group does not exist.\n");
        close(fdDSTCP);
        return;
    }

    while(n = read(fdDSTCP, buffer, sizeof(buffer))){
        if(n == -1) {
            fprintf(stderr, "Error reading response from server. Please try again.\n");
            close(fdDSTCP);
            return;
        }
        printf("%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }
    close(fdDSTCP);
}

int startTCP() {
    // Establish TCP connection as well
    fdDSTCP = socket(AF_INET, SOCK_STREAM, 0);
    if (fdDSTCP == -1) {
        fprintf(stderr, "Error creating Client TCP Socket. Please try again.\n");
        return FAILURE;
    }
    memset(&hintsDS, 0, sizeof(hintsDS));
    hintsDS.ai_family = AF_INET;
    hintsDS.ai_socktype = SOCK_STREAM;
    errcode = getaddrinfo(ipDS, portDS, &hintsDS, &resDSTCP);
    if (errcode != 0) {
        fprintf(stderr, "Error getting DS IP info. Please try again.\n");
        close(fdDSTCP);
        return FAILURE;
    }
    n = connect(fdDSTCP, resDSTCP->ai_addr, resDSTCP->ai_addrlen);
    if (n == -1) {
        fprintf(stderr, "Error conecting to DS via TCP. Please try again.\n");
        close(fdDSTCP);
        return FAILURE;
    }
    return SUCCESS;
}