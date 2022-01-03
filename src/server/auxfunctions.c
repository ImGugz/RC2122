#include "auxfunctions.h"

int validPort(char * portStr) {
    if (isNumber(portStr)) {
        int port = atoi(portStr);
        if (0 <= port && port <= 65535) return 1;
    }
    return 0;
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
                if (validPort(optarg)) strcpy(portDS, optarg);
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
 * @brief Checks if a given string is a number.
 * 
 * @param num string containing (or not) a number
 * @return 0 if it's not a number and 1 otherwise
 */
int isNumber(char * num) {
    size_t len = strlen(num);
    for (int i = 0; i < len; ++i) {
        if (num[i] < '0' || num[i] > '9') {
            return 0;
        }
    }
    return 1;
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
