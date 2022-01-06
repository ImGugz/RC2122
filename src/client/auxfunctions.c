/**
 * @file auxfunctions.c
 * @author Group 18
 * @brief Contains all auxiliary functions used throughout multiple .c files.
 *
 */

#include "auxfunctions.h"

#define MAX_GROUPS 100                      // It's actually 99 but we'll round it
#define MIN(x, y) (((x) < (y)) ? (x) : (y)) // Macro to determine min(x, y)

/**
 * @brief Checks if a given buffer specifies a given pattern.
 *
 * @param buf buffer to be checked
 * @param reg pattern that buffer is being checked on
 * @return 1 if buffer specifies the given pattern and 0 otherwise
 */
int validRegex(char *buf, char *reg)
{
    int reti;
    regex_t regex;
    reti = regcomp(&regex, reg, REG_EXTENDED);
    if (reti)
    {
        fprintf(stderr, "[-] Internal error on parsing regex. Please try again later and/or contact the developers.\n");
        return 0;
    }
    if (regexec(&regex, buf, (size_t)0, NULL, 0))
    {
        regfree(&regex);
        return 0;
    }
    regfree(&regex);
    return 1;
}

/**
 * @brief Checks if a given address is valid according to pre-defined protocols.
 * 
 * @param addrStr address to check if it's valid
 * @return 1 if addrStr is a valid hostname/ipv4 address, 0 otherwise 
 */
int validAddress(char *addrStr)
{
    return (validRegex(addrStr, "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9-]*[a-zA-Z0-9]).)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9-]*[A-Za-z0-9])$") ||
            validRegex(addrStr, "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]).){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$"));
}

/**
 * @brief Checks if a given port is valid according to pre-defined protocols.
 * 
 * @param portStr port to check if it's valid
 * @return 1 if portStr is a valid port number, 0 otherwise
 */
int validPort(char *portStr)
{
    return validRegex(portStr, "^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$");
}

/**
 * @brief Checks if a given user ID is valid according to the statement's rules.
 * 
 * @param UID user ID to check if it's valid
 * @return 1 if UID is valid, 0 otherwise
 */
int validUID(char *UID)
{
    return validRegex(UID, "^[0-9]{5}$");
}

/**
 * @brief Checks if a given user password is valid according to the statement's rules.
 * 
 * @param PW user password to check if it's valid
 * @return 1 if PW is valid, 0 otherwise
 */
int validPW(char *PW)
{
    return validRegex(PW, "^[a-zA-Z0-9]{8}$");
}

/**
 * @brief Checks if a given group ID is valid according to the statement's rules.
 * 
 * @param GID group ID to check if it's valid
 * @return 1 if GID is valid, 0 otherwise
 */
int validGID(char *GID)
{
    return validRegex(GID, "^([0][1-9]|[1-9][0-9])$");
}

/**
 * @brief Checks if a given group name is valid according to the statement's rules.
 * 
 * @param gName group name to check if it's valid
 * @return 1 if gName is valid, 0 otherwise
 */
int validGName(char *gName)
{
    return validRegex(gName, "^[a-zA-Z0-9_-]{1,24}$");
}

/**
 * @brief Checks if a given file name is valid according to the statement's rules.
 * 
 * @param fileName file name to check if it's valid
 * @return 1 if fileName is valid, 0 otherwise
 */
int validFilename(char *fileName)
{
    return validRegex(fileName, "^[a-zA-Z0-9_-]{1,20}[.]{1}[a-z]{3}$");
}

/**
 * @brief Checks if a given message ID is valid according to the statement's rules.
 * 
 * @param MID message ID to check if it's valid
 * @return 1 if MID is valid, 0 otherwise
 */
int validMID(char *MID)
{
    if (!strcmp(MID, "0000"))
        return 0;
    return validRegex(MID, "^[0-9]{0,4}$");
}

/**
 * @brief Reads user executable arguments and parses them.
 *
 * @param argc Number of user executable arguments
 * @param argv User executable arguments
 */
void parseArgs(int argc, char *argv[])
{
    char addrDS[ADDR_SIZE] = DEFAULT_DSADDR, portDS[PORT_SIZE] = DEFAULT_DSPORT;
    char c;
    if (argc != 1 && argc != 3 && argc != 5)
    {
        fprintf(stderr, "[-] Invalid input. Please try again.\n");
        exit(EXIT_FAILURE);
    }
    while ((c = getopt(argc, argv, ":n:p:")) != -1)
    {
        switch (c)
        {
        case 'n':
            if (strcmp(optarg, "-p") == 0)
            { // corner case (./user -n -p 58011)
                fprintf(stderr, "[-] Missing argument (n). Usage: ./user [-n DSIP] [-p DSport].\n");
                exit(EXIT_FAILURE);
            }
            if (!validAddress(optarg))
            {
                fprintf(stderr, "[-] Invalid given address. Please try again.\n");
                exit(EXIT_FAILURE);
            }
            strcpy(addrDS, optarg);
            break;
        case 'p':
            if (strcmp(optarg, "-n") == 0)
            { // corner case (./user -p -n)
                fprintf(stderr, "[-] Missing argument (p). Usage: ./user [-n DSIP] [-p DSport].\n");
                exit(EXIT_FAILURE);
            }
            if (!validPort(optarg))
            {
                fprintf(stderr, "[-] Invalid port number. Please try again.\n");
                exit(EXIT_FAILURE);
            }
            strcpy(portDS, optarg);
            break;
        case ':':
            fprintf(stderr, "[-] Usage: ./user [-n DSIP] [-p DSport]. Please try again.\n");
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "[-] Usage: ./user [-n DSIP] [-p DSport]. Please try again.\n");
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "[-] Usage: ./user [-n DSIP] [-p DSport]. Please try again.\n");
            exit(EXIT_FAILURE);
        }
    }
    setAddrPortDS(addrDS, portDS);
    createUDPSocket();
}

/**
 * @brief Checks if a given string is a number.
 *
 * @param num string containing (or not) a number
 * @return 1 if it's a number, 0 otherwise
 */
int isNumber(char *num)
{
    size_t len = strlen(num);
    for (int i = 0; i < len; ++i)
    {
        if (num[i] < '0' || num[i] > '9')
            return 0;
    }
    return 1;
}

/**
 * @brief Checks if a given string is a slash.
 * 
 * @param slash string containing (or not) a slash 
 * @return 1 if it's a slash, 0 otherwise
 */
int isSlash(char *slash)
{
    return (strlen(slash) == 1 && slash[0] == '/');
}

/**
 * @brief Prints groups from gl and ulist commands.
 *
 * @param buffer DS protocol response to gl and ulist commands
 * @param numGroups Number of groups to print out
 */
void printGroups(char *buffer, int numGroups)
{
    char *groupsBuffer, *token;
    char *groupsList[3 * MAX_GROUPS]; // 3 tokens per group (GID, GName, MID)
    int cnt = 0;
    if (numGroups == 0)
    {
        printf("[+] There are no available groups.\n");
        return;
    }
    if (numGroups == 1)
        printf("[+] %d group: (GID | GName | Last MID)\n", numGroups);
    else
        printf("[+] %d groups: (GID | GName | Last MID)\n", numGroups);
    groupsBuffer = (numGroups >= 10) ? buffer + 7 : buffer + 6; // Update pointer to string accordingly
    token = strtok(groupsBuffer, " ");
    while (token != NULL)
    {
        groupsList[cnt++] = token;
        token = strtok(NULL, " \n");
    }
    for (int i = 0; i <= cnt - 3; i += 3)
    {
        if (!(validGID(groupsList[i]) && validGName(groupsList[i + 1]) && validMID(groupsList[i + 2])))
        {
            printf("[-] Incorrect groups list protocol message was received. Program will now exit.\n");
            closeUDPSocket();
            exit(EXIT_FAILURE);
        }
        printf("%s %s %s\n", groupsList[i], groupsList[i + 1], groupsList[i + 2]);
    }
}

/**
 * @brief Reads the given command and parses it to a macro.
 *
 * @param command string containing user command
 * @return macro corresponding to given command
 */
int parseUserCommand(char *command)
{
    if (strcmp(command, "reg") == 0)
        return REGISTER;
    else if ((strcmp(command, "unregister") == 0) || (strcmp(command, "unr") == 0))
        return UNREGISTER;
    else if (strcmp(command, "login") == 0)
        return LOGIN;
    else if (strcmp(command, "logout") == 0)
        return LOGOUT;
    else if ((strcmp(command, "showuid") == 0) || (strcmp(command, "su") == 0))
        return SHOW_USER;
    else if (strcmp(command, "exit") == 0)
        return USER_EXIT;
    else if ((strcmp(command, "groups") == 0) || (strcmp(command, "gl") == 0))
        return GROUPS_LIST;
    else if (strcmp(command, "subscribe") == 0 || strcmp(command, "s") == 0)
        return SUBSCRIBE;
    else if (strcmp(command, "unsubscribe") == 0 || strcmp(command, "u") == 0)
        return UNSUBSCRIBE;
    else if ((strcmp(command, "my_groups") == 0) || (strcmp(command, "mgl") == 0))
        return USER_GROUPS;
    else if ((strcmp(command, "select") == 0) || (strcmp(command, "sag") == 0))
        return SELECT;
    else if ((strcmp(command, "showgid") == 0) || (strcmp(command, "sg") == 0))
        return SHOW_SELECTED;
    else if ((strcmp(command, "ulist") == 0) || (strcmp(command, "ul") == 0))
        return USERS_LIST;
    else if (strcmp(command, "post") == 0)
        return GROUP_POST;
    else if ((strcmp(command, "retrieve") == 0) || (strcmp(command, "r") == 0))
        return GROUP_RETRIEVE;
    else
    {
        fprintf(stderr, "[-] Invalid user command code. Please try again.\n");
        return INVALID_COMMAND;
    }
}

/**
 * @brief Auxiliary function of sendFile that sends a buffer via TCP connection
 *
 * @param buffer buffer to be sent
 * @param num number of bytes to be sent
 * @return 1 if num bytes of buffer were sent and 0 otherwise
 */
int sendData(char *buffer, size_t num)
{
    unsigned char *tmpBuf = (unsigned char *)buffer;
    ssize_t n;
    while (num > 0)
    {
        n = send(fdDSTCP, tmpBuf, num, 0);
        if (n == -1)
        {
            perror("[-] Failed to send file data via TCP");
            return 0;
        }
        tmpBuf += n;
        num -= n;
    }
    return 1;
}

/**
 * @brief Sends a file via TCP.
 *
 * @param post file stream of the file being sent
 * @param lenFile number of bytes in file being sent
 * @return 1 if file was sent and 0 otherwise
 */
int sendFile(FILE *post, long lenFile)
{
    char buffer[1025];
    do
    {
        size_t num = MIN(lenFile, sizeof(buffer) - 1); // sizeof(buffer)-1 so it only reads 1024 worst case and buffer[num] = '\n' doesn't SIGSEGV
        num = fread(buffer, sizeof(char), num, post);
        if (num < 1)
        {
            fprintf(stderr, "[-] Failed on reading the given file. Please try again.\n");
            closeUDPSocket();
            closeTCPSocket();
            fclose(post);
            return 0;
        }
        if (lenFile - num == 0)
        { // Each request/reply ends with a \n
            buffer[num] = '\n';
        }
        if (!sendData(buffer, num))
        {
            closeUDPSocket();
            closeTCPSocket();
            fclose(post);
            return 0;
        }
        lenFile -= num;
    } while (lenFile > 0);
    return 1;
}

/**
 * @brief Sends a message via TCP.
 *
 * @param message message to be sent
 */
void sendTCP(char *message)
{
    int bytesSent = 0;
    ssize_t nSent;
    size_t tempLen = strlen(message);
    while (bytesSent < tempLen)
    { // Send initial message
        nSent = send(fdDSTCP, message + bytesSent, tempLen - bytesSent, 0);
        if (nSent == -1)
        {
            perror("[-] Failed to write on TCP");
            closeUDPSocket();
            closeTCPSocket();
            exit(EXIT_FAILURE);
        }
        bytesSent += nSent;
    }
}

/**
 * @brief Reads at most maxSize bytes of given message with a given flag
 *
 * @param message buffer to store message read
 * @param maxSize max number of bytes to be read
 * @param flag 0 or MSG_PEEK
 * @return -1 if recv failed or N > 0 corresponding to the number of bytes received
 */
int readTCP(char *message, int maxSize, int flag)
{
    int bytesRecv = 0;
    ssize_t n;
    if (flag == MSG_PEEK)
    { // We assume that recv can get at least offset bytes (usually really low -> reliable)
        n = recv(fdDSTCP, message, maxSize, MSG_PEEK);
        if (n == -1)
        {
            perror("[-] Failed to receive from server on TCP");
            closeUDPSocket();
            closeTCPSocket();
        }
        return n;
    }
    while (bytesRecv < maxSize)
    {
        n = recv(fdDSTCP, message + bytesRecv, maxSize - bytesRecv, 0);
        if (n == -1)
        {
            perror("[-] Failed to receive from server on TCP");
            closeUDPSocket();
            closeTCPSocket();
            return n;
        }
        if (n == 0)
            break; // Peer has performed an orderly shutdown -> message complete
        bytesRecv += n;
    }
    return bytesRecv;
}

/**
 * @brief Receives a file via TCP.
 *
 * @param fileName name of the file being received
 * @param lenFile number of bytes in the file being received
 */
void recvFile(char *fileName, long lenFile)
{
    long bytesRecv = 0;
    int toRead;
    unsigned char bufFile[2048] = "";
    ssize_t n;
    FILE *retFile = fopen(fileName, "wb");
    if (!retFile)
    {
        perror("Failed to create retrieved file");
        closeUDPSocket();
        closeTCPSocket();
        exit(EXIT_FAILURE);
    }
    do
    {
        toRead = MIN(sizeof(bufFile), lenFile - bytesRecv);
        n = recv(fdDSTCP, bufFile, toRead, 0);
        if (n == -1)
        {
            perror("[-] Failed to receive from server on TCP");
            closeUDPSocket();
            closeTCPSocket();
            exit(EXIT_FAILURE);
        }
        if (n > 0)
        {
            bytesRecv += n;
            fwrite(bufFile, sizeof(unsigned char), n, retFile);
        }
        memset(bufFile, 0, n);
    } while (bytesRecv < lenFile);
    fclose(retFile);
}