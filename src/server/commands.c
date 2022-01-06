#include "commands.h"

#define CMDBUF_SIZE 256
#define MAX_NUM_TOKENS 4

char *processClientUDP(char *buf)
{
    char *token, *tokenList[MAX_NUM_TOKENS];
    char tmp[CMDBUF_SIZE];
    int numTokens = 0;
    int op, status;
    char *response;
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
        return NULL;
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
        status = listGroups(numTokens);
        response = createGroupListMessage(status);
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
    char clientBuf[MAX_RECVUDP_SIZE] = "";
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
            perror("[-] Server UDP failed on recvfrom");
            closeUDPSocket();
            exit(EXIT_FAILURE);
        }
        write(1, "Client: ", 8);
        write(1, clientBuf, n);
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
        if (timerOn(newTCPfd) < 0)
        {
            perror("[-] Setsockopt on server TCP has failed");
            exit(EXIT_FAILURE);
        }
        if ((pid = fork()) == 0)
        {
            close(listenSocket);
            bzero(clientBuf, sizeof(clientBuf));
            n = read(newTCPfd, clientBuf, sizeof(clientBuf));
            if (n == -1)
            {
                perror("[-] Server TCP failed on read");
                exit(EXIT_FAILURE);
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