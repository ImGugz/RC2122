#include "auxfunctions.h"

int validPort(char *portStr)
{
    return validRegex(portStr, "^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$");
}

/**
 * @brief Reads user executable arguments and parses them.
 *
 * @param argc Number of user executable arguments
 * @param argv User executable arguments
 */
void parseArgs(int argc, char *argv[])
{
    char c;
    if (argc != 1 && argc != 2 && argc != 3 && argc != 4)
    {
        fprintf(stderr, "[-] Invalid input. Please try again.\n");
        exit(EXIT_FAILURE);
    }
    while ((c = getopt(argc, argv, ":p:v")) != -1)
    {
        switch (c)
        {
        case 'p':
            if (strcmp(optarg, "-v") == 0)
            { // corner case (./user -p -v)
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
    else
    {
        fprintf(stderr, "[-] Invalid protocol command code received. Please try again.\n");
        return INVALID_COMMAND;
    }
}

char *createStatusMessage(char *command, int statusCode)
{
    char status[MAX_RECVUDP_SIZE];

    switch (statusCode)
    {
    case OK:
        sprintf(status, "%s %s", command, "OK\n");
        break;
    case NOK:
        sprintf(status, "%s %s", command, "NOK\n");
        break;
    case DUP:
        sprintf(status, "%s %s", command, "DUP\n");
        break;
    default:
        break;
    }
    return strdup(status);
}

int remove_directory(const char *path)
{
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;

    if (d)
    {
        struct dirent *p;
        r = 0;
        while (!r && (p = readdir(d)))
        {
            int r2 = -1;
            char *buf;
            size_t len;

            /* Skip the names "." and ".." as we don't want to recurse on them. */
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
                continue;

            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len);

            if (buf)
            {
                struct stat statbuf;

                snprintf(buf, len, "%s/%s", path, p->d_name);
                if (!stat(buf, &statbuf))
                {
                    if (S_ISDIR(statbuf.st_mode))
                        r2 = remove_directory(buf);
                    else
                        r2 = unlink(buf);
                }
                free(buf);
            }
            r = r2;
        }
        closedir(d);
    }
    if (!r)
        r = rmdir(path);

    return r;
}

int isDirectoryExists(const char *path)
{
    struct stat stats;

    if (stat(path, &stats))
        return 0;

    return S_ISDIR(stats.st_mode);
}

int isCorrectPassword(const char *userDir, const char *userID, const char *pass)
{
    FILE *fPtr;
    char passFileName[28];
    char *password;

    sprintf(passFileName, "%s/%s_pass.txt", userDir, userID);

    fPtr = fopen(passFileName, "r");
    if (fPtr == NULL)
    {
        fprintf(stderr, "[-] Unable to open password file.\n");
        return 0;
    }

    password = malloc(8); // Nao sei onde por uma macro PASSWORD_SIZE

    if (password)
        fread(password, 1, 8, fPtr);

    fclose(fPtr);

    // Ha um invalid read of size 1 neste strcmp. Nao consigo resolver
    if (strcmp(password, pass) != 0)
    {
        free(password);
        return 0;
    }

    free(password);
    return 1;
}

int timerOn(int fd)
{
    struct timeval timeout;
    memset((char *)&timeout, 0, sizeof(timeout));
    timeout.tv_sec = DEFAULT_TIMEOUT;
    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&timeout, sizeof(struct timeval));
}

int timerOff(int fd)
{
    struct timeval timeout;
    memset((char *)&timeout, 0, sizeof(timeout));
    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&timeout, sizeof(struct timeval));
}
