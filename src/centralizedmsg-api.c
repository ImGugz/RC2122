#include "centralizedmsg-api.h"

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
    return validRegex(fileName, "^[a-zA-Z0-9_-]{1,20}[.]{1}[a-zA-Z]{3}$");
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
 * @brief Closes the UDP's socket file descriptor and free's the memory allocated with getaddrinfo() function.
 *
 */
void closeUDPSocket(struct addrinfo *resUDP, int fdUDP)
{
    freeaddrinfo(resUDP);
    close(fdUDP);
}

/**
 * @brief Closes the TCP's socket file descriptor and free's the memory allocated with getaddrinfo() function.
 *
 */
void closeTCPSocket(struct addrinfo *resTCP, int fdTCP)
{
    freeaddrinfo(resTCP);
    close(fdTCP);
}