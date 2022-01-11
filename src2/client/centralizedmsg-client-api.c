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

char messageToDS[CLIENT_TO_DS_UDP_SIZE];

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
    {
        fprintf(stderr, "[-] Incorrect register command usage. Please try again.\n");
        return;
    }
    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    {
        fprintf(stderr, "[-] Invalid register command arguments. Please check given UID and/or password and try again.\n");
        return;
    }
    sprintf(messageToDS, "REG %s %s\n", tokenList[1], tokenList[2]);
    exchangeDSUDPMsg(messageToDS);
}

void closeDSUDPSocket()
{
    freeaddrinfo(resUDP);
    close(fdDSUDP);
}