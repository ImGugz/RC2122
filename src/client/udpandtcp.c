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
#define MAX_RPT_SIZE 10

#define RRT_CODE_SIZE 4
#define RRT_STATUS_SIZE 4
#define MAX_NUMRRT_SIZE 3  // maxlen(20) + 1
#define DISCARDCHAR_SIZE 2 // len(' ') + 1
#define RRTMID_SIZE 5
#define RRTUID_SIZE 6
#define RRTTXTSZ_SIZE 4
#define RRTTEXT_SIZE 241
#define RRTFILENAME_SIZE 25
#define RRTFILESZ_SIZE 11
#define MSG_OK 0
#define MSG_CONCAT 1

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
    if (fclose(post) == -1)
    {
        perror("[-] Failed to close posted file.");
        exit(EXIT_FAILURE);
    }
    char msgRecvBuf[MAX_RPT_SIZE];
    if ((nTCP = readTCP(msgRecvBuf, MAX_RPT_SIZE - 1, 0)) == -1)
    { // readTCP already closes socket's fds upon error
        fclose(post);
        exit(EXIT_FAILURE);
    }
    msgRecvBuf[nTCP] = '\0'; // will never SIGSEGV
    processTCPMsg(msgRecvBuf, NULL);
    closeTCPSocket();
}

void retrieveERR()
{
    fprintf(stderr, "[-] Wrong protocol message received on retrieve. Program will now exit.\n");
    closeUDPSocket();
    closeTCPSocket();
    exit(EXIT_FAILURE);
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
    processTCPMsg(listRecvBuf, NULL);
    free(listRecvBuf);
    closeTCPSocket();
}

void exchangeTCPRet(char *message)
{
    char codeRRT[RRT_CODE_SIZE + 1];
    char statusRRT[RRT_STATUS_SIZE];
    connectTCPSocket();
    sendTCP(message);

    // Read the code (RRT)
    if ((nTCP = readTCP(codeRRT, RRT_CODE_SIZE, 0)) == -1)
    {
        exit(EXIT_FAILURE);
    }
    codeRRT[nTCP] = '\0';
    printf("codeRRT: %s\n", codeRRT);
    if (strcmp(codeRRT, "RRT "))
    { // Wrong protocol message received
        printf("yo\n");
        retrieveERR();
    }

    // Read the status (OK/NOK)
    if ((nTCP = readTCP(statusRRT, RRT_STATUS_SIZE - 1, 0)) == -1)
    {
        exit(EXIT_FAILURE);
    }
    statusRRT[nTCP] = '\0';
    printf("statusRRT: %s\n", statusRRT);
    char discard[DISCARDCHAR_SIZE];
    if (!strcmp(statusRRT, "EOF") || !strcmp(statusRRT, "NOK"))
    {
        if ((nTCP = readTCP(discard, DISCARDCHAR_SIZE - 1, 0)) == -1)
        {
            exit(EXIT_FAILURE);
        }
        discard[nTCP] = '\0';
        if (discard[nTCP - 1] != '\n')
        {
            retrieveERR();
        }
        printf("discard 506: %s\n", discard);
    }
    int flag;
    char codeStatusRRT[RRT_CODE_SIZE + RRT_STATUS_SIZE + 1];
    sprintf(codeStatusRRT, "%s %s", codeRRT, statusRRT);
    processTCPMsg(codeStatusRRT, &flag);
    if (!flag)
    { // if it gets here then either RRT NOK or RRT EOF
        closeTCPSocket();
        return;
    }

    // Read the number of messages to retrieve
    char numMsgsRRT[MAX_NUMRRT_SIZE];
    if ((nTCP = readTCP(numMsgsRRT, MAX_NUMRRT_SIZE - 1, 0)) == -1)
    {
        exit(EXIT_FAILURE);
    }
    numMsgsRRT[nTCP] = '\0';
    printf("numMsgsRRT: %s\n", numMsgsRRT);
    int numMsgs = atoi(numMsgsRRT);
    if (numMsgs >= 10)
    { // if there are more than 10 messages we must read an extra space in the case where there are < 10 messages
        if ((nTCP = readTCP(discard, DISCARDCHAR_SIZE - 1, 0)) == -1)
        {
            exit(EXIT_FAILURE);
        }
        discard[nTCP] = '\0';
        printf("discard: %s\n", discard);
        if (discard[0] != ' ')
        {
            retrieveERR();
        }
    }
    if (numMsgs == 1)
    {
        printf("[+] %d message to display: (-> MID: text \\(Fname - Fsize)):\n", numMsgs);
    }
    else
    {
        printf("[+] %d messages to display: (-> MID: text \\(Fname - Fsize)):\n", numMsgs);
    }
    int flagRRT = MSG_OK;
    char MID[RRTMID_SIZE] = "", UID[RRTUID_SIZE] = "", TsizeBuf[RRTTXTSZ_SIZE] = "", Text[RRTTEXT_SIZE + 1] = "";
    char Fname[RRTFILENAME_SIZE] = "", FsizeBuf[RRTFILESZ_SIZE] = "";
    int Tsize;
    long Fsize;
    int j;
    for (int i = 1; i <= numMsgs; ++i)
    {
        memset(MID, 0, sizeof(MID));
        memset(UID, 0, sizeof(UID));
        memset(TsizeBuf, 0, sizeof(TsizeBuf));
        memset(Text, 0, sizeof(Text));
        memset(Fname, 0, sizeof(Fname));
        memset(FsizeBuf, 0, sizeof(FsizeBuf));
        // Read MID
        if (flagRRT == MSG_OK)
        { // if flag is MSG_OK then we read a normal MID
            if ((nTCP = readTCP(MID, RRTMID_SIZE, 0)) == -1)
            { // RRTMID_SIZE to also read backspace
                exit(EXIT_FAILURE);
            }
            MID[nTCP - 1] = '\0';
            printf("MID: %s\n", MID);
            if (!validMID(MID))
            {
                retrieveERR();
            }
            printf("-> %s: ", MID);
        }
        else if (flagRRT == MSG_CONCAT)
        { // if flag is MSG_CONCAT then we must concat a character read previously to MID
            strcat(MID, discard);
            printf("MID 2.1: %s\n", MID);
            char *ptrMID = MID + 1;
            if ((nTCP = readTCP(ptrMID, RRTMID_SIZE - 1, 0)) == -1)
            { // RRTMID_SIZE-1 because discard contains first character and to also read backspace
                exit(EXIT_FAILURE);
            }
            printf("MID 2.2: %s\n", MID);
            MID[nTCP] = '\0';
            printf("MID 2.3: %s\n", MID);
            if (!validMID(MID))
            {
                retrieveERR();
            }
            printf("-> %s: ", MID);
        }

        // Read UID
        if ((nTCP = readTCP(UID, RRTUID_SIZE, 0)) == -1)
        { // RRTMID_SIZE to also read backspace
            exit(EXIT_FAILURE);
        }
        UID[nTCP - 1] = '\0';
        printf("UID: %s\n", UID);
        if (!validUID(UID))
        {
            retrieveERR();
        }

        // Read text size
        for (j = 0; j < RRTTXTSZ_SIZE; ++j)
        {
            if ((nTCP = readTCP(discard, DISCARDCHAR_SIZE - 1, 0)) == -1)
            {
                exit(EXIT_FAILURE);
            }
            if (discard[0] != ' ')
            { // anything other than the space we'll append or new line if retrieve message ends with text
                strcat(TsizeBuf, &discard[0]);
            }
            else
            { // space has been read
                break;
            }
        }
        TsizeBuf[j] = '\0';
        printf("TsizeBuf: %s\n", TsizeBuf);
        if (discard[0] != ' ' || !isNumber(TsizeBuf))
        { // make sure space was read and Tsize is a number - otherwise wrong protocol message received
            retrieveERR();
        }
        Tsize = atoi(TsizeBuf);

        // Read text
        if ((nTCP = readTCP(Text, Tsize + 1, 0)) == -1)
        {
            exit(EXIT_FAILURE);
        }
        Text[nTCP] = '\0';
        printf("Text: %s\n", Text);
        if ((Text[Tsize] != ' ') && (Text[Tsize] != '\n'))
        { // wrong protocol message received -> either it ends or has a file
            retrieveERR();
        }
        if (Text[Tsize] == '\n')
        { // end of reply has been made - remove new line on printing
            Text[Tsize] = '\0';
            printf("%s\n", Text);
            break;
        }
        printf("%s\n", Text);
        // Read next character -> either it's a slash(/) or the first char of a MID
        if ((nTCP = readTCP(discard, DISCARDCHAR_SIZE - 1, 0)) == -1)
        {
            exit(EXIT_FAILURE);
        }
        discard[nTCP] = '\0';
        printf("discard 660: %s\n", discard);
        if (isNumber(&discard[0]))
        { // it read the first digit of the next MID
            flagRRT = MSG_CONCAT;
        }
        else if (discard[0] == '/')
        { // there's a file to be read
            flagRRT = MSG_OK;

            // Read backspace
            if ((nTCP = readTCP(discard, DISCARDCHAR_SIZE - 1, 0)) == -1)
            {
                exit(EXIT_FAILURE);
            }
            discard[nTCP] = '\0';
            printf("discard 672: %s\n", discard);
            if (discard[0] != ' ')
            {
                retrieveERR();
            }

            // Read filename
            for (j = 0; j < RRTFILENAME_SIZE; ++j)
            {
                if ((nTCP = readTCP(discard, DISCARDCHAR_SIZE - 1, 0)) == -1)
                {
                    exit(EXIT_FAILURE);
                }
                if (discard[0] != ' ')
                { // anything other than the space we'll append
                    strcat(Fname, &discard[0]);
                }
                else
                { // space has been read
                    break;
                }
            }
            Fname[j] = '\0';
            printf("Fname: %s\n", Fname);
            if (discard[0] != ' ' || !validFilename(Fname))
            {
                retrieveERR();
            }
            printf("(%s - ", Fname);

            // Read file size
            for (j = 0; j < RRTFILESZ_SIZE; ++j)
            {
                if ((nTCP = readTCP(discard, DISCARDCHAR_SIZE - 1, 0)) == -1)
                {
                    exit(EXIT_FAILURE);
                }
                if (discard[0] != ' ')
                { // anything other than the space we'll append
                    strcat(FsizeBuf, &discard[0]);
                }
                else
                { // space has bee read
                    break;
                }
            }
            FsizeBuf[j] = '\0';
            printf("FsizeBuf: %s\n", FsizeBuf);
            printf("%s bytes)\n", FsizeBuf);
            if (discard[0] != ' ')
            {
                retrieveERR();
            }
            Fsize = atol(FsizeBuf);
            printf("BANANA\n");
            recvFile(Fname, Fsize);
            printf("PERAS\n");
            if ((nTCP = readTCP(discard, DISCARDCHAR_SIZE - 1, 0)) == -1)
            {
                exit(EXIT_FAILURE);
            }
            printf("I'm here\n");
            discard[nTCP] = '\0';
            printf("discard 732: %s\n", discard);
            if (discard[0] != ' ' && discard[0] != '\n')
            { // read extra space in file or last message \n
                printf("Bumbum\n");
                retrieveERR();
            }
        }
        else
        { // wrong protocol message received
            printf("Entered Here!\n");
            retrieveERR();
        }
    }
    printf("FUUUUUUUUUUUUUCK!!!!!!!!!!!!!!!!!!!!!!!!\n");
    sendTCP("OK\n");
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