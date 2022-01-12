#include "centralizedmsg-client-api.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/* UDP Socket related variables */
int fdDSUDP;
struct addrinfo hintsUDP, *resUDP;
struct sockaddr_in addrUDP;
socklen_t addrlenUDP;

/* Client current session variables */
int clientSession; // LOGGED_IN or LOGGED_OUT
char activeClientUID[CLIENT_UID_SIZE], activeClientPWD[CLIENT_PWD_SIZE];

/* Message to DS via UDP protocol variable */
char messageToDSUDP[CLIENT_TO_DS_UDP_SIZE];

static void displayGroups(char *message, int numGroups)
{
    if (numGroups == 0)
    {
        printf("[+] There are no available groups.\n");
        return;
    }
    else if (numGroups == 1)
    {
        printf("[+] %d group: (GID | GName | Last MID)\n", numGroups);
    }
    else
    { // Safe to assume this else since numGroups is a positive number
        // len(GLS 10) = 7; len(GLS 9) = 5
        printf("[+] %d groups: (GID | GName | Last MID)\n", numGroups);
        char *p_message = (numGroups >= 10) ? message + 6 : message + 5;
        char groupInfo[DS_GROUPINFO_SIZE];
        char GID[DS_GID_SIZE], GName[DS_GNAME_SIZE], MID[DS_MID_SIZE];
        int offsetPtr;
        while (numGroups--)
        {
            sscanf(p_message, " %s %s %s%n", GID, GName, MID, &offsetPtr);
            if (!(validGID(GID) && validGName(GName) && isMID(MID)))
            {
                fprintf(stderr, "[-] Wrong protocol message received from server via UDP. Program will now exit.\n");
                closeDSUDPSocket();
                exit(EXIT_FAILURE);
            }
            printf("-> %2s | %24s | %4s\n", GID, GName, MID);
            p_message += offsetPtr;
        }
    }
}

void createDSUDPSocket(char *addrDS, char *portDS)
{
    fdDSUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (fdDSUDP == -1)
    {
        perror("[-] Client UDP socket failed to create");
        exit(EXIT_FAILURE);
    }
    memset(&hintsUDP, 0, sizeof(hintsUDP));
    hintsUDP.ai_family = AF_INET;
    hintsUDP.ai_socktype = SOCK_DGRAM;
    int errcode = getaddrinfo(addrDS, portDS, &hintsUDP, &resUDP);
    if (errcode != 0)
    {
        perror("[-] Failed on UDP address translation");
        close(fdDSUDP);
        exit(EXIT_FAILURE);
    }
}

void processDSUDPReply(char *message)
{
    char codeDS[PROTOCOL_CODE_SIZE], statusDS[PROTOCOL_STATUS_UDP_SIZE];
    sscanf(message, "%s %s", codeDS, statusDS);
    if (!strcmp(codeDS, "RRG"))
    { // REGISTER command
        if (!strcmp(statusDS, "OK"))
        {
            printf("[+] You have successfully registered.\n");
        }
        else if (!strcmp(statusDS, "DUP"))
        {
            fprintf(stderr, "[-] This user is already registered in the DS database.\n");
        }
        else if (!strcmp(statusDS, "NOK"))
        {
            fprintf(stderr, "[-] Something went wrong with register. Please try again later.\n");
        }
        else
        {
            fprintf(stderr, "[-] Unexpected register protocol status message was received. Program will now exit.\n");
            closeDSUDPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(codeDS, "RUN"))
    { // UNREGISTER command
        if (!strcmp(statusDS, "OK"))
        {
            printf("[+] You have successfully unregistered.\n");
        }
        else if (!strcmp(statusDS, "NOK"))
        {
            fprintf(stderr, "[-] Something went wrong with unregister. Please check user credentials and try again.\n");
        }
        else
        {
            fprintf(stderr, "[-] Unexpected unregister protocol status message was received. Program will now exit.\n");
            closeDSUDPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(codeDS, "RLO"))
    { // LOGIN command
        if (!strcmp(statusDS, "OK"))
        {
            clientSession = LOGGED_IN;
            printf("[+] You have successfully logged in.\n");
        }
        else if (!strcmp(statusDS, "NOK"))
        {
            fprintf(stderr, "[-] Something went wrong with login. Please check user credentials and try again.\n");
        }
        else
        {
            fprintf(stderr, "[-] Unexpected login protocol status message was received. Program will now exit.\n");
            closeDSUDPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(codeDS, "ROU"))
    { // LOGOUT command
        if (!strcmp(statusDS, "OK"))
        {
            clientSession = LOGGED_OUT;
            printf("[+] You have successfully logged out.\n");
        }
        else if (!strcmp(statusDS, "NOK"))
        {
            fprintf(stderr, "[-] Something went wrong with logout. Please try again and/or contact the developers.\n");
        }
        else
        {
            fprintf(stderr, "[-] Unexpected logout protocol status message was received. Program will now exit.\n");
            closeDSUDPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(codeDS, "RGL"))
    { // GROUPS command
        if (isNumber(statusDS))
        {
            displayGroups(message, atoi(statusDS));
        }
        else
        {
            fprintf(stderr, "[-] Unexpected groups list protocol message was received. Program will now exit.\n");
            closeDSUDPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(codeDS, "RGS"))
    { // SUBSCRIBE command
        if (!strcmp(statusDS, "OK"))
        {
            printf("[+] You have successfully subscribed to this group.\n");
        }
        else if (!strcmp(statusDS, "NEW"))
        {
            char newGID[DS_GID_SIZE];
            strncpy(newGID, message + 8, 2); // copy the new group ID : len(RGS NEW ) = 8
            newGID[DS_GID_SIZE - 1] = '\0';
            printf("[+] You have successfully created a new group (ID = %s).\n", newGID);
        }
        else if (!strcmp(statusDS, "E_FULL"))
        {
            fprintf(stderr, "[-] Group creation has failed. Maximum number of groups has been reached.\n");
        }
        else if (!strcmp(statusDS, "E_USR"))
        {
            fprintf(stderr, "[-] UID submitted to server is incorrect. Please try again and/or contact the developers.\n");
        }
        else if (!strcmp(statusDS, "E_GRP"))
        {
            fprintf(stderr, "[-] Group ID submitted is invalid. Please check available groups using 'gl' command and try again.\n");
        }
        else if (!strcmp(statusDS, "E_GNAME"))
        {
            fprintf(stderr, "[-] Group name submitted is invalid. Please try again.\n");
        }
        else if (!strcmp(statusDS, "NOK"))
        {
            fprintf(stderr, "[-] The group subscribing process has failed. Please try again.\n");
        }
        else
        {
            fprintf(stderr, "[-] Unexpected subscribe protocol message was received. Program will now exit.\n");
            closeDSUDPSocket();
            exit(EXIT_FAILURE);
        }
    }
}

void exchangeDSUDPMsg(char *message)
{
    ssize_t n;
    // Client -> DS message
    n = sendto(fdDSUDP, message, strlen(message), 0, resUDP->ai_addr, resUDP->ai_addrlen); // strlen counts nl
    if (n == -1)
    { // Syscall failed -> terminate gracefully
        perror("[-] UDP message failed to send");
        closeDSUDPSocket();
        exit(EXIT_FAILURE);
    }

    // DS -> Client message
    addrlenUDP = sizeof(addrUDP);
    char dsReply[DS_TO_CLIENT_UDP_SIZE];
    n = recvfrom(fdDSUDP, dsReply, DS_TO_CLIENT_UDP_SIZE, 0, (struct sockaddr *)&addrUDP, &addrlenUDP);
    if (n == -1)
    {
        perror("[-] Failed to receive UDP message");
        closeDSUDPSocket();
        exit(EXIT_FAILURE);
    }
    dsReply[n] = '\0';
    if (dsReply[n - 1] != '\n')
    { // Each request/reply ends with newline according to DS-Client communication protocol
        fprintf(stderr, "[-] Wrong protocol message received from server via UDP. Program will now exit.\n");
        closeDSUDPSocket();
        exit(EXIT_FAILURE);
    }
    dsReply[n - 1] = '\0'; // Replace \n with \0
    processDSUDPReply(dsReply);
}

void clientRegister(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    { // REG UID PWD
        fprintf(stderr, "[-] Incorrect register command usage. Please try again.\n");
        return;
    }
    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    { // Protocol validation
        fprintf(stderr, "[-] Invalid register command arguments. Please check given UID and/or password and try again.\n");
        return;
    }
    sprintf(messageToDSUDP, "REG %s %s\n", tokenList[1], tokenList[2]);
    exchangeDSUDPMsg(messageToDSUDP);
}

void clientUnregister(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    { // UNREGISTER UID PWD / UNR UID PWD
        fprintf(stderr, "[-] Incorrect unregister command usage. Please try again.\n");
        return;
    }
    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    { // Protocol validation
        fprintf(stderr, "[-] Invalid unregister command arguments. Please check given UID and/or password and try again.\n");
        return;
    }
    sprintf(messageToDSUDP, "UNR %s %s\n", tokenList[1], tokenList[2]);
    exchangeDSUDPMsg(messageToDSUDP);
}

void clientLogin(char **tokenList, int numTokens)
{
    if (clientSession == LOGGED_IN)
    { // Client is already logged in into an account
        fprintf(stderr, "[-] You're already logged in. Please logout before you try to log in again.\n");
        return;
    }
    if (numTokens != 3)
    { // LOGIN UID PWD
        fprintf(stderr, "[-] Incorrect login command usage. Please try again.\n");
        return;
    }
    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    { // Protocol message
        fprintf(stderr, "[-] Invalid login command arguments. Please check given UID and/or password and try again.\n");
        return;
    }
    sprintf(messageToDSUDP, "LOG %s %s\n", tokenList[1], tokenList[2]);
    exchangeDSUDPMsg(messageToDSUDP);
    if (clientSession == LOGGED_IN)
    { // The exchangeDSUDPMsg function sets the client session to LOGGED_IN if reply is OK
        strcpy(activeClientUID, tokenList[1]);
        strcpy(activeClientPWD, tokenList[2]);
    }
}

void clientLogout(char **tokenList, int numTokens)
{
    if (clientSession == LOGGED_OUT)
    { // Client isn't logged in into any account
        fprintf(stderr, "[-] You're not logged in into any account.\n");
        return;
    }
    if (numTokens != 1)
    { // LOGOUT
        fprintf(stderr, "[-] Incorrect logout command usage. Please try again.\n");
        return;
    }
    sprintf(messageToDSUDP, "OUT %s %s\n", activeClientUID, activeClientPWD);
    exchangeDSUDPMsg(messageToDSUDP);
    if (clientSession == LOGGED_OUT)
    { // The exchangeDSUDPMsg function sets the client session to LOGGED_OUT if reply is OK
        memset(activeClientUID, 0, sizeof(activeClientUID));
        memset(activeClientPWD, 0, sizeof(activeClientPWD));
    }
}

void showCurrentClient(int numTokens)
{
    if (numTokens != 1)
    { // SHOWUID / SU
        fprintf(stderr, "[-] Incorrect showuid command usage. Please try again.\n");
        return;
    }
    if (clientSession == LOGGED_IN)
    {
        printf("[+] You're logged in with user ID %s.\n", activeClientUID);
    }
    else
    {
        printf("[-] You're not logged in into any account.\n");
    }
}

void clientExit(int numTokens)
{
    if (numTokens != 1)
    { // EXIT
        fprintf(stderr, "[-] Incorrect exit command usage. Please try again.\n");
        return;
    }
    closeDSUDPSocket();
    printf("[+] Exiting...\n");
    exit(EXIT_SUCCESS);
}

void showDSGroups(int numTokens)
{
    if (numTokens != 1)
    { // GROUPS / GL
        fprintf(stderr, "[-] Incorrect groups list command usage. Please try again.\n");
        return;
    }
    sprintf(messageToDSUDP, "GLS\n");
    exchangeDSUDPMsg(messageToDSUDP);
}

void clientSubscribeGroup(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    { // S <CODE> GNAME
        fprintf(stderr, "[-] Incorrect subscribe command usage. Please try again.\n");
        return;
    }
    if (clientSession == LOGGED_OUT)
    { // User must be logged in in order to subscribe to a group
        fprintf(stderr, "[-] Please login before you subscribe to a group.\n");
        return;
    }
    if (!strcmp(tokenList[1], "0"))
    { // Create a new group
        if (!validGName(tokenList[2]))
        {
            fprintf(stderr, "[-] Invalid new group name. Please try again.\n");
            return;
        }
        sprintf(messageToDSUDP, "GSR %s 00 %s\n", activeClientUID, tokenList[2]);
    }
    else if (validGID(tokenList[1]) && validGName(tokenList[2]))
    { // Subscribe to a group
        sprintf(messageToDSUDP, "GSR %s %s %s\n", activeClientUID, tokenList[1], tokenList[2]);
    }
    else
    {
        fprintf(stderr, "[-] Incorrect subscribe command usage. Please try again.\n");
        return;
    }
    exchangeDSUDPMsg(messageToDSUDP);
}

void clientUnsubscribeGroup(char **tokenList, int numTokens)
{
    if (clientSession == LOGGED_OUT)
    {
        fprintf(stderr, "[-] Please login before you unsubscribe to a group.\n");
        return;
    }
    if (numTokens != 2)
    {
        fprintf(stderr, "[-] Incorrect unsubscribe command usage. Please try again.\n");
        return;
    }
    if (!validGID(tokenList[1]))
    {
        fprintf(stderr, "[-] Invalid given group ID to unsubscribe. Please try again.\n");
        return;
    }
    sprintf(messageToDSUDP, "GUR %s %s\n", activeClientUID, tokenList[1]);
    exchangeDSUDPMsg(messageToDSUDP);
}

void closeDSUDPSocket()
{
    freeaddrinfo(resUDP);
    close(fdDSUDP);
}