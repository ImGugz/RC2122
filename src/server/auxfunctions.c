#include "auxfunctions.h"

int validPort(char * portStr) {
    return validRegex(portStr, "^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$");
}

/**
 * @brief Reads user executable arguments and parses them.
 * 
 * @param argc Number of user executable arguments
 * @param argv User executable arguments
 */
void parseArgs(int argc, char * argv[]) {
    char c;
    if (argc != 1 && argc != 2 && argc != 3 && argc != 4) {
        fprintf(stderr, "[-] Invalid input. Please try again.\n");
        exit(EXIT_FAILURE);
    }
    while ((c = getopt(argc, argv, ":p:v")) != -1) {
        switch(c) {
            case 'p':
                if (strcmp(optarg, "-v") == 0) { // corner case (./user -p -v)
                    fprintf(stderr, "[-] Usage: ./DS [-p DSport] [-v].\n");
                    exit(EXIT_FAILURE);
                }
                if (!validPort(optarg))
                {
                    fprintf(stderr, "[-] Invalid port number. Please try again.\n");
                    exit(EXIT_FAILURE);
                }
                strcpy(portDS, optarg);
                break;
            case 'v':
                verbose = 1;
                break;
            case ':':
                fprintf(stderr, "[-] Usage: ./DS [-p DSport] [-v].\n");
                exit(EXIT_FAILURE);
            case '?':
                fprintf(stderr, "[-] Usage: ./DS [-p DSport] [-v].\n");
                exit(EXIT_FAILURE);
            default:
                fprintf(stderr, "[-] Usage: ./DS [-p DSport] [-v].\n");
                exit(EXIT_FAILURE);
        }
    }
    setupDSSockets();
}

/**
 * @brief Checks if a given buffer specifies a given pattern.
 *
 * @param buf buffer to be checked
 * @param reg pattern that buffer is being checked on
 * @return 1 if buffer specifies the given pattern and 0 otherwise
 */
int validRegex(char * buf, char * reg)
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

int parseUserCommandUDP(char *command)
{
    if (strcmp(command, "REG") == 0)
        return REGISTER;
    else if (strcmp(command, "UNR") == 0)
        return UNREGISTER;
    else if (strcmp(command, "LOG") == 0)
        return LOGIN;
    else if (strcmp(command, "OUT") == 0)
        return LOGOUT;
    else if (strcmp(command, "GLS") == 0)
        return GROUPS_LIST;
    else if (strcmp(command, "GSR") == 0)
        return SUBSCRIBE;
    else if (strcmp(command, "GUR") == 0)
        return UNSUBSCRIBE;
    else if (strcmp(command, "GLM") == 0)
        return USER_GROUPS;
    else {
        fprintf(stderr, "[-] Invalid protocol command code received. Please try again.\n");
        return INVALID_COMMAND;
    }
}

int timerOn(int fd) {
    struct timeval timeout;
    memset((char *) &timeout, 0, sizeof(timeout));
    timeout.tv_sec = DEFAULT_TIMEOUT;
    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &timeout, sizeof(struct timeval));
}

int timerOff(int fd) {
    struct timeval timeout;
    memset((char *) &timeout, 0, sizeof(timeout));
    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &timeout, sizeof(struct timeval));
}
