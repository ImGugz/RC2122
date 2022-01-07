#include "commands.h"

#define CMDBUF_SIZE 512
#define MAX_NUM_TOKENS 4

char *processClientUDP(char *buf)
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
    op = parseUserCommandUDP(tokenList[0]);
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
        break;
    case USER_GROUPS:
        response = createUserGroupsMessage(tokenList, numTokens);
        break;
    default:
        break;
    }

    return response;
}

char *processClientTCP(char *buf)
{
    return NULL;
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
        serverBuf = processClientUDP(clientBuf);
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
    char *servBuf;
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
            exit(EXIT_FAILURE);
        }
        if ((pid = fork()) == 0)
        {
            close(listenSocket);
            bzero(clientBuf, sizeof(clientBuf));

            n = read(newTCPfd, clientBuf, sizeof(clientBuf));
            if (n == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    perror("[-] Server UDP timeout has been reached");
                }
                else
                {
                    perror("[-] Server TCP failed on read");
                }
                close(newTCPfd);
                break;
            }
            clientBuf[n - 1] = '\0';
            write(1, "Client: ", 8);
            write(1, clientBuf, n);
            servBuf = processClientTCP(clientBuf);
            n = write(newTCPfd, serverBuf, strlen(serverBuf) + 1);
            if (n == -1)
            {
                perror("[-] Server UDP failed on sendto");
                exit(EXIT_FAILURE);
            }
            close(newTCPfd);
            exit(EXIT_SUCCESS);
        }
        close(newTCPfd);
    }
}