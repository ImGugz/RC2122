/**
 * @file udpandtcp.c
 * @author Group 18
 * @brief Setup and message exchange via TCP and UDP sockets.
 *
 */

#include "udpandtcp.h"
#include "auxfunctions.h"

#define DSCODE_SIZE 4       // maxlen(reply code) + 1
#define DSSTATUS_SIZE 8     // maxlen(status code) + 1 = len(E_GNAME) + 1
#define UDP_RECVF_SIZE 4096 // 2 ^ math.top(log_2(3274)) (maxlen(glcmd) = 3274)
#define TCP_READ_SIZE 512   // Arbitrary

#define MAX_RPT_SIZE 10     // maxlen(rpt xxxx\n) + 1
#define RRT_CODESTAT_SIZE 8 // maxlen(rrt) + 1 (RRT NOK\n)
#define MAX_NUMRRT_SIZE 3   // maxlen(20) + 1
#define BACKSPACE_SIZE 2    // len(' ') + 1
#define RRT_BUFFER_SIZE 256 // arbitrary
#define SLASH_MID_SIZE 5    // max(1, 4) + 1
#define UID_FNAME_SIZE 25   // max(5, 24) + 1
#define FILE_TEXT_SIZE 11   // max(3, 10) + 1
#define MAX_TEXT_SIZE 242   // maxlen(text) + backspace + 1
#define BACKSPACE 1         // used for avoiding meaningless "+1" in offset calcs

int fdDSUDP, fdDSTCP;
int errcode;
struct addrinfo hintsUDP, hintsTCP, *resUDP, *resTCP;
struct sockaddr_in addrUDP;
socklen_t addrlenUDP;

char addrDS[ADDR_SIZE], portDS[PORT_SIZE];
ssize_t nUDP, nTCP; // Used to store number of bytes sent/received in messages exchanged in UDP/TCP protocol

/**
 * @brief Reads DS's UDP response and gives the user a brief report following the statement's protocol.
 *
 * @param buffer DS's response to a command sent via UDP.
 */
void processUDPMsg(char *buffer)
{
    char codeDS[DSCODE_SIZE], statusDS[DSSTATUS_SIZE];
    sscanf(buffer, "%s %s", codeDS, statusDS);
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
            closeUDPSocket();
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
            closeUDPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(codeDS, "RLO"))
    { // LOGIN command
        if (!strcmp(statusDS, "OK"))
        {
            userSession = LOGGED_IN;
            printf("[+] You have successfully logged in.\n");
        }
        else if (!strcmp(statusDS, "NOK"))
        {
            fprintf(stderr, "[-] Something went wrong with login. Please check user credentials and try again.\n");
        }
        else
        {
            fprintf(stderr, "[-] Unexpected login protocol status message was received. Program will now exit.\n");
            closeUDPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(codeDS, "ROU"))
    { // LOGOUT command
        if (!strcmp(statusDS, "OK"))
        {
            userSession = LOGGED_OUT;
            printf("[+] You have successfully logged out.\n");
        }
        else if (!strcmp(statusDS, "NOK"))
        {
            fprintf(stderr, "[-] Something went wrong with logout. Please try again and/or contact the developers.\n");
        }
        else
        {
            fprintf(stderr, "[-] Unexpected logout protocol status message was received. Program will now exit.\n");
            closeUDPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(codeDS, "RGL"))
    { // GROUPS command
        if (isNumber(statusDS))
        {
            printGroups(buffer, atoi(statusDS));
        }
        else
        {
            fprintf(stderr, "[-] Unexpected groups list protocol message was received. Program will now exit.\n");
            closeUDPSocket();
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
            char GID[3];
            strncpy(GID, buffer + 8, 2);
            GID[2] = '\0';
            printf("[+] You have successfully created a new group (ID = %s).\n", GID); // 8 is the fixed offset from buffer to the new GID
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
            closeUDPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(codeDS, "RGU"))
    { // UNSUBSCRIBE command
        if (!strcmp(statusDS, "OK"))
        {
            printf("[+] You have successfully unsubscribed to this group.\n");
        }
        else if (!strcmp(statusDS, "E_USR"))
        {
            fprintf(stderr, "[-] UID submitted to server is incorrect. Please try again and/or contact the developers.\n");
        }
        else if (!strcmp(statusDS, "E_GRP"))
        {
            fprintf(stderr, "[-] Invalid given group ID. Please check available groups using 'gl' command and try again.\n");
        }
        else
        {
            fprintf(stderr, "[-] Unexpected unsubscribe protocol message was received. Program will now exit.\n");
            closeUDPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(codeDS, "RGM"))
    { // MY_GROUPS command
        if (isNumber(statusDS))
        {
            printGroups(buffer, atoi(statusDS));
        }
        else
        {
            fprintf(stderr, "[-] Unexpected my groups list protocol message was received. Program will now exit.\n");
            closeUDPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else
    { // Unexpected protocol -> ABORT EXECUTION
        fprintf(stderr, "[-] Unexpected protocol message was received. Program will now exit.\n");
        closeUDPSocket();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Reads DS's TCP response and gives the user a brief report following the statement's protocol.
 *
 * @param buffer DS's response to a command sent via UDP.
 * @param flag Pointer to flag used in command 'retrieve' (if set to 0 there's nothing to retrieve & set to 1 otherwise)
 */
void processTCPMsg(char *buffer, int *flag)
{
    char codeDS[DSCODE_SIZE], statusDS[DSSTATUS_SIZE];
    sscanf(buffer, "%s %s", codeDS, statusDS);
    if (!strcmp(codeDS, "RUL"))
    { // ULIST command
        if (!strcmp(statusDS, "OK"))
        {
            buffer += 7;
            char groupName[25], userInGroup[6];
            sscanf(buffer, "%s", groupName);
            buffer += strlen(groupName);
            printf("[+] List of users subscribed to %s:\n", groupName);
            while (sscanf(buffer, " %s", userInGroup) == 1)
            {
                printf("- %s\n", userInGroup);
                buffer += 6;
            }
        }
        else if (!strcmp(statusDS, "NOK"))
        {
            fprintf(stderr, "[-] The group you've selected doesn't exist. Please try again.\n");
        }
        else
        {
            fprintf(stderr, "[-] Unexpected protocol message was received. Program will now exit.\n");
            closeUDPSocket();
            closeTCPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(codeDS, "RPT"))
    { // POST command
        if (validMID(statusDS))
        {
            printf("[+] You have successfully posted in the selected group with message ID %s.\n", statusDS);
        }
        else if (!strcmp(statusDS, "NOK"))
        {
            fprintf(stderr, "[-] Failed to post in group. Please try again.\n");
            return;
        }
        else
        {
            fprintf(stderr, "[-] Unexpected protocol message was received. Program will now exit.\n");
            closeUDPSocket();
            closeTCPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(codeDS, "RRT"))
    { // RETRIEVE command
        if (!strcmp(statusDS, "OK"))
        {
            *flag = 1;
        }
        else if (!strcmp(statusDS, "NOK"))
        {
            fprintf(stderr, "[-] Failed to retrieve from group. Please check if you have a selected subscribed group and try again.\n");
            *flag = 0;
        }
        else if (!strcmp(statusDS, "EOF"))
        {
            printf("[+] There are no available messages to show in the selected group from the given starting message.\n");
            *flag = 0;
        }
        else
        {
            fprintf(stderr, "[-] Unexpected protocol message was received. Program will now exit.\n");
            closeUDPSocket();
            closeTCPSocket();
            exit(EXIT_FAILURE);
        }
    }
    else
    { // Unexpected protocol -> ABORT EXECUTION
        fprintf(stderr, "[-] Unexpected protocol message was received. Program will now exit.\n");
        closeUDPSocket();
        closeTCPSocket();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Create client's UDP socket.
 *
 */
void createUDPSocket()
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
    errcode = getaddrinfo(addrDS, portDS, &hintsUDP, &resUDP);
    if (errcode != 0)
    {
        perror("[-] Failed on UDP address translation");
        close(fdDSUDP);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Connect client's TCP socket.
 *
 */
void connectTCPSocket()
{
    fdDSTCP = socket(AF_INET, SOCK_STREAM, 0);
    if (fdDSTCP == -1)
    {
        perror("[-] Client TCP socket failed to create");
        closeUDPSocket();
        exit(EXIT_FAILURE);
    }
    memset(&hintsTCP, 0, sizeof(hintsTCP));
    hintsTCP.ai_family = AF_INET;
    hintsTCP.ai_socktype = SOCK_STREAM;
    errcode = getaddrinfo(addrDS, portDS, &hintsTCP, &resTCP);
    if (errcode != 0)
    {
        perror("[-] Failed on TCP address translation");
        close(fdDSTCP);
        closeUDPSocket();
        exit(EXIT_FAILURE);
    }
    ssize_t n = connect(fdDSTCP, resTCP->ai_addr, resTCP->ai_addrlen);
    if (n == -1)
    {
        perror("[-] Failed to connect to TCP socket");
        closeUDPSocket();
        closeTCPSocket();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Sends and receives message to/from DS via UDP following the statement's protocol.
 *
 * @param message User to DS message that follows the statement's protocol.
 */
void exchangeUDPMsg(char *message)
{
    char recvBuffer[UDP_RECVF_SIZE];
    nUDP = sendto(fdDSUDP, message, strlen(message), 0, resUDP->ai_addr, resUDP->ai_addrlen);
    if (nUDP == -1)
    { // Syscall failed -> terminate gracefully
        perror("[-] UDP message failed to send");
        closeUDPSocket();
        exit(EXIT_FAILURE);
    }
    addrlenUDP = sizeof(addrUDP);
    nUDP = recvfrom(fdDSUDP, recvBuffer, UDP_RECVF_SIZE, 0, (struct sockaddr *)&addrUDP, &addrlenUDP);
    if (nUDP == -1)
    {
        perror("[-] Failed to receive UDP message");
        closeUDPSocket();
        exit(EXIT_FAILURE);
    }
    recvBuffer[nUDP] = '\0'; // will never SIGSEGV
    printf("[!] Server @ %s in port %d sent: %s", inet_ntoa(addrUDP.sin_addr), ntohs(addrUDP.sin_port), recvBuffer);
    processUDPMsg(recvBuffer);
}

/**
 * @brief Sends and receives message to/from DS via TCP regarding to the POST (with file) command following the statement's protocol.
 *
 * @param message User POST message that follows the statement's protocol (ish) - the file will be sent separately.
 * @param post pointer to FILE stream associated to the file that'll be sent.
 * @param lenFile number of bytes inside the file that'll be sent.
 */
void exchangeTCPPost(char *message, FILE *post, long lenFile)
{
    connectTCPSocket();
    sendTCP(message);
    if (!sendFile(post, lenFile))
    { // sendFile already handles proper pre-exiting operations (close file stream, socket fd's, etc)
        exit(EXIT_FAILURE);
    }
    char msgRecvBuf[MAX_RPT_SIZE];
    if ((nTCP = readTCP(msgRecvBuf, MAX_RPT_SIZE - 1, 0)) == -1)
    { // readTCP already closes socket's fds upon error
        fclose(post);
        exit(EXIT_FAILURE);
    }
    msgRecvBuf[nTCP] = '\0'; // will never SIGSEGV
    logTCPServer(msgRecvBuf);
    processTCPMsg(msgRecvBuf, NULL);
    closeTCPSocket();
}

in_port_t get_in_port(struct sockaddr *sa)
{ // TODO: DELETE
    if (sa->sa_family == AF_INET)
        return (((struct sockaddr_in *)sa)->sin_port);

    return (((struct sockaddr_in6 *)sa)->sin6_port);
}

void logTCPServer(char *message)
{ // TODO: DELETE
    struct sockaddr_in *addr = (struct sockaddr_in *)resTCP->ai_addr;
    printf("[!] Server @ %s in port %d sent: %s", inet_ntoa(((struct in_addr)addr->sin_addr)), ntohs(get_in_port((struct sockaddr *)resTCP->ai_addr)), message);
}

/**
 * @brief Sends and receives message to/from DS via TCP regarding to the ULIST and POST (w/o file) commands following the statement's protocol.
 *
 * @param message User to DS message that follows the statement's protocol.
 */
void exchangeTCPMsg(char *message)
{
    int bytesRecv = 0;
    char msgRecvBuf[TCP_READ_SIZE];
    char *oldPtr, *listRecvBuf;
    connectTCPSocket();
    sendTCP(message);
    oldPtr = (char *)calloc(sizeof(char), TCP_READ_SIZE + 1);
    if (oldPtr == NULL)
    {
        fprintf(stderr, "[-] Error allocating memory in heap. Please try again.\n");
        closeTCPSocket();
        return;
    }
    listRecvBuf = oldPtr;
    int lenBuf = TCP_READ_SIZE;
    while ((nTCP = readTCP(msgRecvBuf, TCP_READ_SIZE, 0)) > 0)
    { // Read all data
        if (bytesRecv + nTCP >= lenBuf)
        { // Buffer isn't big enough -> must realloc
            char *newPtr = (char *)realloc(listRecvBuf, 2 * lenBuf);
            if (newPtr == NULL)
            {
                fprintf(stderr, "[-] Error allocating memory in heap. Please try again.\n");
                free(listRecvBuf); // Free the calloc or previous realloc
                closeTCPSocket();
                return;
            }
            listRecvBuf = newPtr;
            lenBuf *= 2;
        }
        memcpy(listRecvBuf + bytesRecv, msgRecvBuf, nTCP);
        bytesRecv += nTCP;
    }
    listRecvBuf[bytesRecv] = '\0'; // will never SIGSEGV (worst case when it gets here bytesRecv = lenBuf-1)
    logTCPServer(listRecvBuf);
    processTCPMsg(listRecvBuf, NULL);
    free(listRecvBuf);
    closeTCPSocket();
}

/**
 * @brief Sends and receives message to/from DS via TCP regarding to the RETRIEVE command following the statement's protocol.
 *
 * @param message User RETRIEVE message that follows the statement's protocol.
 */
void exchangeTCPRet(char *message)
{
    char codeStatus[RRT_CODESTAT_SIZE], numRRTMsgs[MAX_NUMRRT_SIZE], backspace[BACKSPACE_SIZE];
    char rrtBuf[RRT_BUFFER_SIZE], textBuf[MAX_TEXT_SIZE];
    char slashOrMID[SLASH_MID_SIZE], fileNameOrUID[UID_FNAME_SIZE], textOrFileSize[FILE_TEXT_SIZE];
    char *rrtOffset, *temp;
    size_t offset;
    connectTCPSocket();
    sendTCP(message);
    if ((nTCP = readTCP(codeStatus, RRT_CODESTAT_SIZE - 1, 0)) == -1)
    { // Read code and status
        exit(EXIT_FAILURE);
    }
    codeStatus[nTCP] = '\0'; // \n will be ignored (irrelevant) -> will never SIGSEGV
    logTCPServer(codeStatus);
    int flag;
    processTCPMsg(codeStatus, &flag); // flag as pointer to be set in processTCPMsg function
    if (!flag)
    { // flag = 1 : messages to retrieve, flag = 0 : no messages to retrieve/error
        closeTCPSocket();
        return;
    }
    if ((nTCP = readTCP(numRRTMsgs, MAX_NUMRRT_SIZE - 1, 0)) == -1)
    { // Read number of messages to read
        exit(EXIT_FAILURE);
    }
    numRRTMsgs[nTCP] = '\0'; // will never SIGSEGV
    logTCPServer(numRRTMsgs);
    if (atoi(numRRTMsgs) >= 10)
    { // if there are more than 10 messages we must read an extra space to match the pointer
        // in the case where there are < 10 messages
        if ((nTCP = readTCP(backspace, BACKSPACE_SIZE - 1, 0)) == -1)
        {
            exit(EXIT_FAILURE);
        }
        backspace[nTCP] = '\0'; // will never SIGSEGV
        logTCPServer(backspace);
    }
    int nMsg = atoi(numRRTMsgs);
    if (nMsg == 1)
    {
        printf("[+] %d message to display: (-> MID: text \\(Fname - Fsize)):\n", nMsg);
    }
    else
    {
        printf("[+] %d messages to display: (-> MID: text \\(Fname - Fsize)):\n", nMsg);
    }
    while (1)
    {
        memset(textBuf, 0, sizeof(textBuf));
        memset(rrtBuf, 0, sizeof(rrtBuf));
        nTCP = readTCP(rrtBuf, RRT_BUFFER_SIZE - 1, MSG_PEEK); // Assumes at least offset bytes will come (few) -> EXTREMELY RELIABLE
        if (nTCP == -1)
        {
            exit(EXIT_FAILURE);
        }
        rrtBuf[nTCP] = '\0';
        logTCPServer(rrtBuf);
        if (nTCP == 0 || (nTCP == 1 && rrtBuf[0] == '\n'))
        { // nothing else to read
            break;
        }
        sscanf(rrtBuf, "%s %s %s", slashOrMID, fileNameOrUID, textOrFileSize);
        offset = strlen(slashOrMID) + BACKSPACE + strlen(fileNameOrUID) + BACKSPACE + strlen(textOrFileSize) + BACKSPACE;
        temp = (char *)calloc(sizeof(char), offset + 1);
        if (temp == NULL)
        {
            fprintf(stderr, "[-] Error allocating memory in heap. Please try again.\n");
            closeTCPSocket();
            return;
        }
        rrtOffset = temp;
        if ((nTCP = readTCP(rrtOffset, offset, 0)) == -1)
        { // Read offset bytes to start reading text/file data
            free(rrtOffset);
            exit(EXIT_FAILURE);
        }
        rrtOffset[nTCP] = '\0';
        logTCPServer(rrtOffset);
        free(temp);
        temp = NULL;
        long bytesToRead = atol(textOrFileSize);
        if (validMID(slashOrMID))
        { // Text case
            if (!(validMID(slashOrMID) && validUID(fileNameOrUID) && bytesToRead <= 240))
            {
                fprintf(stderr, "[-] Wrong protocol message received on retrieve. Program will now exit.\n");
                closeUDPSocket();
                closeTCPSocket();
                exit(EXIT_FAILURE);
            }
            if ((nTCP = readTCP(textBuf, bytesToRead + BACKSPACE, 0)) == -1)
            { // bytesToRead + 1 to read extra space
                exit(EXIT_FAILURE);
            }
            textBuf[nTCP - 1] = '\0'; // will never SIGSEGV (-1 because of extra space read)
            logTCPServer(textBuf);
            printf("-> %s: %s\n", slashOrMID, textBuf);
        }
        else if (strlen(slashOrMID) == 1 && slashOrMID[0] == '/')
        { // File case
            if (!(validFilename(fileNameOrUID) && strlen(textOrFileSize) <= 10))
            {
                fprintf(stderr, "[-] Wrong protocol message received on retrieve. Program will now exit.\n");
                closeUDPSocket();
                closeTCPSocket();
                exit(EXIT_FAILURE);
            }
            recvFile(fileNameOrUID, bytesToRead);
            printf("(%s - %s bytes)\n", fileNameOrUID, textOrFileSize);
            if ((nTCP = readTCP(backspace, BACKSPACE_SIZE - 1, 0)) == -1)
            { // Update stream pointer consistency
                exit(EXIT_FAILURE);
            }
            backspace[nTCP] = '\0';
            logTCPServer(backspace);
        }
        else
        {
            fprintf(stderr, "[-] Wrong protocol message received on retrieve. Program will now exit\n");
            closeUDPSocket();
            closeTCPSocket();
            exit(EXIT_FAILURE);
        }
    }
    closeTCPSocket();
}

/**
 * @brief Closes the UDP's socket file descriptor and free's the memory allocated with getaddrinfo() function.
 *
 */
void closeUDPSocket()
{
    freeaddrinfo(resUDP);
    close(fdDSUDP);
}

/**
 * @brief Closes the TCP's socket file descriptor and free's the memory allocated with getaddrinfo() function.
 *
 */
void closeTCPSocket()
{
    freeaddrinfo(resTCP);
    close(fdDSTCP);
}