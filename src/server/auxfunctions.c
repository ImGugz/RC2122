#include "auxfunctions.h"

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
                if (isNumber(optarg)) {
                    strcpy(portDS, optarg);
                }
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
