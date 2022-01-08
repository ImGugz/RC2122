#include "commands.h"

#define CMDBUF_SIZE 512
#define MAX_NUM_TOKENS 4

char *processClient(char *buf)
{
    char *token, *tokenList[MAX_NUM_TOKENS];
    char tmp[CMDBUF_SIZE];
    int numTokens = 0;
    int op, status;
    char *response, *newGID = NULL;
    if (buf[strlen(buf) - 1] != '\n')
    { // all protocol messages exchanged must end with a \n
        return strdup(ERR_MSG);
    }
    strtok(buf, "\n");
    strcpy(tmp, buf);
    token = strtok(tmp, " \n");
    while (token)
    {
        tokenList[numTokens++] = token;
        token = strtok(NULL, " \n");
    }
    op = parseUserCommand(tokenList[0]);
    if (op == INVALID_COMMAND)
    { // invalid protocol message received
        return strdup(ERR_MSG);
    }
    switch (op)
    {
    case REGISTER:
        status = userRegister(tokenList, numTokens);
        response = createStatusMessage("RRG", status);
        break;
    case UNREGISTER:
        status = userUnregister(tokenList, numTokens);
        response = createStatusMessage("RUN", status);
        break;
    case LOGIN:
        status = userLogin(tokenList, numTokens);
        response = createStatusMessage("RLO", status);
        break;
    case LOGOUT:
        status = userLogout(tokenList, numTokens);
        response = createStatusMessage("ROU", status);
        break;
    case GROUPS_LIST:
        response = createGroupListMessage("RGL", NULL, 0);
        break;
    case SUBSCRIBE:
        status = userSubscribe(tokenList, numTokens, &newGID);
        response = createSubscribeMessage(status, newGID);
        break;
    case UNSUBSCRIBE:
        status = userUnsubscribe(tokenList, numTokens);
        response = createStatusMessage("RGU", status);
        printf("Aqi wih %s\n", response);
        break;
    case USER_GROUPS:
        response = createUserGroupsMessage(tokenList, numTokens);
        break;
    }
    return response;
}

char *createPostStatusMessage(char *status)
{
    if (!strcmp(status, "NOK"))
    {
        return strdup("RPT NOK\n");
    }
    else if (validMID(status))
    {
        char answer[10];
        sprintf(answer, "RPT %s\n", status);
        free(status);
        return strdup(answer);
    }
}

char *processClientTCP(int acceptfd, char *peekedMsg, int recvBytes)
{
    char opMsg[4];
    char *response;
    char *status;
    sscanf(peekedMsg, "%3s", opMsg);
    int op = parseUserCommand(opMsg);
    if (op == INVALID_COMMAND)
    { // invalid protocol message received
        return strdup(ERR_MSG);
    }
    switch (op)
    {
    case USERS_LIST:
        response = createUsersInGroupMessage(acceptfd, peekedMsg);
        break;
    case GROUP_POST:
        status = userPost(acceptfd, peekedMsg, recvBytes);
        response = createPostStatusMessage(status);
        break;
    }
    return response;
}

void handleUDP(int clientSocket)
{
    char clientBuf[MAX_RECVUDP_SIZE];
    struct sockaddr_in cliaddr;
    socklen_t addrlen;
    ssize_t n;
    char *serverBuf;
    while (1)
    {
        addrlen = sizeof(cliaddr);
        n = recvfrom(clientSocket, clientBuf, sizeof(clientBuf), 0, (struct sockaddr *)&cliaddr, &addrlen);
        if (n == -1)
        {
            perror("[-] Server UDP failed on read");
            closeUDPSocket();
            exit(EXIT_FAILURE);
        }
        clientBuf[n] = '\0';
        if (verbose)
        {
            logVerbose(clientBuf, cliaddr);
        }
        serverBuf = processClient(clientBuf);
        n = sendto(clientSocket, serverBuf, strlen(serverBuf) + 1, 0, (struct sockaddr *)&cliaddr, addrlen);
        if (n == -1)
        {
            perror("[-] Server UDP failed on sendto");
            closeUDPSocket();
            exit(EXIT_FAILURE);
        }
        free(serverBuf);
    }
}

void handleTCP(int listenSocket)
{
    char clientBuf[MAX_RECVTCP_SIZE] = "";
    struct sockaddr_in cliaddr;
    socklen_t addrlen;
    ssize_t n;
    char *serverBuf;
    int newTCPfd;
    pid_t pid;
    while (1)
    {
        addrlen = sizeof(cliaddr);
        if ((newTCPfd = accept(listenSocket, (struct sockaddr *)&cliaddr, &addrlen)) == -1)
        {
            perror("[-] Server TCP failed to accept new connection");
            continue;
        }
        if ((pid = fork()) == 0)
        {
            close(listenSocket);
            memset(clientBuf, 0, sizeof(clientBuf));
            if (timerOn(newTCPfd) == -1)
            {
                perror("[-] Failed to start TCP timer");
                close(newTCPfd);
                break;
            }
            n = readTCP(newTCPfd, clientBuf, MAX_RECVTCP_SIZE - 1, MSG_PEEK);
            if (n == -1)
            {
                if (!(errno == EAGAIN || errno == EWOULDBLOCK))
                {
                    perror("[-] Server TCP failed on read");
                    close(newTCPfd);
                    exit(EXIT_FAILURE);
                }
                else
                {
                    fprintf(stderr, "[-] TCP timeout on receive has been reached.\n");
                    char errMSG[5];
                    strcpy(errMSG, "ERR\n");
                    write(newTCPfd, errMSG, strlen(errMSG) + 1);
                    close(newTCPfd);
                    exit(EXIT_FAILURE);
                }
            }
            clientBuf[n] = '\0';
            if (verbose)
            {
                logVerbose(clientBuf, cliaddr);
            }
            serverBuf = processClientTCP(newTCPfd, clientBuf, n);
            n = write(newTCPfd, serverBuf, strlen(serverBuf) + 1);
            if (n == -1)
            {
                perror("[-] Server UDP failed on sendto");
                close(newTCPfd);
                exit(EXIT_FAILURE);
            }
            close(newTCPfd);
            exit(EXIT_SUCCESS);
        }
        close(newTCPfd);
    }
}