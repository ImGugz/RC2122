/**
 * @file udpandtcp.c
 * @author Group 18
 * @brief Setup and message exchange via TCP and UDP sockets.
 *
 */

#include "udpandtcp.h"
#include "auxfunctions.h"

#define DSCODE_SIZE 4      // maxlen(reply code) + 1
#define DSSTATUS_SIZE 8    // maxlen(status code) + 1 = len(E_GNAME) + 1
#define UDP_RECV_SIZE 4096 // 2 ^ math.top(log_2(3307))
#define TCP_READ_SIZE 512  // Arbitrary

int fdDSUDP, fdDSTCP;
int errcode;
struct addrinfo hintsUDP, hintsTCP, *resUDP, *resTCP;
struct sockaddr_in addrUDP;
socklen_t addrlenUDP;

char addrDS[ADDR_SIZE], portDS[PORT_SIZE];
ssize_t nUDP; // Used to store number of bytes sent/received in messages exchanged in UDP/TCP protocol

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
            fprintf(stderr, "[-] This user is already registered in DS.\n");
        }
        else if (!strcmp(statusDS, "NOK"))
        {
            fprintf(stderr, "[-] Something went wrong with the registration process. Please try again later.\n");
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
            fprintf(stderr, "[-] Something went wrong. Please check user credentials and try again.\n");
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
            fprintf(stderr, "[-] Something went wrong. Please check user credentials and try again.\n");
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
            fprintf(stderr, "[-] Something went wrong. Please check user credentials and try again.\n");
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
            printf("[+] You have successfully created a new group (ID = %d).\n", atoi(buffer + 8));
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
            fprintf(stderr, "[-] Group ID submitted is invalid. Please check available groups and try again.\n");
        }
        else if (!strcmp(statusDS, "E_GNAME"))
        {
            fprintf(stderr, "[-] Group name submitted is invalid. Please try again.\n");
        }
        else if (!strcmp(statusDS, "NOK"))
        {
            fprintf(stderr, "[-] The subscribing process has failed. Please try again.\n");
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
            fprintf(stderr, "[-] Group ID submitted is invalid. Please check available groups and try again.\n");
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
        if (strlen(statusDS) == 4 && isNumber(statusDS))
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
 * @brief Set the given address/port to use in UDP/TCP protocols.
 *
 * @param addr domain name/address (string/NULL).
 * @param port port number (string/NULL).
 */
void setAddrPortDS(char *addr, char *port)
{
    if (strlen(addr) > 63 || strlen(port) > 5)
    {
        fprintf(stderr, "[-] Please insert a valid domain name/port.\n");
        return;
    }
    strcpy(addrDS, addr);
    strcpy(portDS, port);
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
    char recvBuffer[UDP_RECV_SIZE] = "";
    nUDP = sendto(fdDSUDP, message, strlen(message), 0, resUDP->ai_addr, resUDP->ai_addrlen);
    if (nUDP == -1)
    { // Syscall failed -> terminate gracefully
        perror("[-] UDP message failed to send");
        closeUDPSocket();
        exit(EXIT_FAILURE);
    }
    addrlenUDP = sizeof(addrUDP);
    nUDP = recvfrom(fdDSUDP, recvBuffer, UDP_RECV_SIZE, 0, (struct sockaddr *)&addrUDP, &addrlenUDP);
    if (nUDP == -1)
    {
        perror("[-] Failed to receive UDP message");
        closeUDPSocket();
        exit(EXIT_FAILURE);
    }
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
    {
        exit(EXIT_FAILURE);
    } // sendFile already closes socket's fds and post stream upon error
    char msgRecvBuf[9];
    if (readTCP(msgRecvBuf, 9, 0) == -1)
    {
        fclose(post);
        exit(EXIT_FAILURE);
    } // readTCP already closes socket's fds upon error
    processTCPMsg(msgRecvBuf, NULL);
    closeTCPSocket();
}

/**
 * @brief Sends and receives message to/from DS via TCP regarding to the ULIST and POST (w/o file) commands following the statement's protocol.
 *
 * @param message User to DS message that follows the statement's protocol.
 */
void exchangeTCPMsg(char *message)
{
    int n, bytesRecv = 0;
    char msgRecvBuf[TCP_READ_SIZE] = "";
    char *oldPtr, *listRecvBuf;
    connectTCPSocket();
    printf("Message: %s\b", message);
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
    while ((n = readTCP(msgRecvBuf, TCP_READ_SIZE, 0)) > 0)
    { // Read all data
        if (bytesRecv + n >= lenBuf)
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
        memcpy(listRecvBuf + bytesRecv, msgRecvBuf, n);
        bytesRecv += n;
    }
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
    int flag, nMsg;
    char msgRecvBuf[8] = "", numRetMsgs[3] = "", backspace[2] = "";
    char textBuf[256] = "", retBuf[256] = "", tempBuf1[5] = "", tempBuf2[25] = "", tempBuf3[11] = ""; // 5 = 1 + max(1, 4) ; 25 = 1 + max(24, 5); 11 = 1 + max(10, 3)
    int nRecv;
    char *retDel, *temp;
    size_t offset;
    connectTCPSocket();
    sendTCP(message);
    if (readTCP(msgRecvBuf, 7, 0) == -1)
        exit(EXIT_FAILURE);           // Read code and status
    processTCPMsg(msgRecvBuf, &flag); // flag as pointer to be set in processTCPMsg function
    if (!flag)
    {
        closeTCPSocket();
        return;
    } // EOF or NOK
    if (readTCP(numRetMsgs, 2, 0) == -1)
        exit(EXIT_FAILURE); // Read number of messages to read
    if (atoi(numRetMsgs) >= 10)
    {
        if (readTCP(backspace, 1, 0) == -1)
            exit(EXIT_FAILURE);
    } // Update stream pointer consistency (< 10 read an extra backspace)
    nMsg = atoi(numRetMsgs);
    printf("%d messages to display: (MID | UID | Tsize | text [| Fname | Fsize]):\n", nMsg);
    while (1)
    {
        memset(textBuf, 0, sizeof(textBuf));
        memset(retBuf, 0, sizeof(retBuf));
        nRecv = readTCP(retBuf, 255, MSG_PEEK); // Assumes at least offset bytes will come (few) -> EXTREMELY RELIABLE
        retBuf[nRecv] = '\0';
        if (nRecv == -1)
            exit(EXIT_FAILURE);
        if (nRecv == 0 || (nRecv == 1 && retBuf[0] == '\n'))
            break;
        sscanf(retBuf, "%s %s %s", tempBuf1, tempBuf2, tempBuf3);
        offset = strlen(tempBuf1) + 1 + strlen(tempBuf2) + 1 + strlen(tempBuf3) + 1;
        temp = (char *)calloc(sizeof(char), offset);
        if (temp == NULL)
        {
            fprintf(stderr, "[-] Error allocating memory in heap. Please try again.\n");
            closeTCPSocket();
            return;
        }
        retDel = temp;
        if (readTCP(retDel, offset, 0) == -1)
            exit(EXIT_FAILURE); // Read offset bytes to start reading text/file data
        free(temp);
        temp = NULL;
        long bytesToRead = atol(tempBuf3);
        if (strlen(tempBuf1) == 4 && isNumber(tempBuf1))
        { // Text case
            if ((nRecv = readTCP(textBuf, bytesToRead + 1, 0)) == -1)
                exit(EXIT_FAILURE);    // bytesToRead + 1 to read extra space
            textBuf[nRecv - 1] = '\0'; // This will never segfault as textBuf is 256 length and max(nRecv) = 241 (-1 because of extra space read)
            printf("-> MID=%s | UID=%s | Tsize=%s | Text=%s", tempBuf1, tempBuf2, tempBuf3, textBuf);
        }
        else if (strlen(tempBuf1) == 1 && tempBuf1[0] == '/')
        {
            recvFile(tempBuf2, bytesToRead);
            printf("| Fname=%s | Fsize=%ld\n", tempBuf2, bytesToRead);
            if (readTCP(backspace, 1, 0) == -1)
                ; // Update stream pointer consistency
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