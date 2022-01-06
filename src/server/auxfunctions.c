#include "auxfunctions.h"

#define USERPW_SIZE 8

int validPort(char *portStr)
{
    return validRegex(portStr, "^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$");
}

int validMID(char * MID) {
    if (!strcmp(MID, "0000")) return 0;
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
    // sprintf terminates with null terminator
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

int removeDirectory(const char * path)
{
    DIR * d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;
    if (d)
    {
        struct dirent * p;
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
                        r2 = removeDirectory(buf);
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
    size_t n;

    sprintf(passFileName, "%s/%s_pass.txt", userDir, userID);

    fPtr = fopen(passFileName, "r");
    if (fPtr == NULL)
    {
        fprintf(stderr, "[-] Unable to open password file.\n");
        return 0;
    }

    password = calloc(sizeof(char), USERPW_SIZE + 1); // Nao sei onde por uma macro PASSWORD_SIZE

    if (password) {
        n = fread(password, 1, USERPW_SIZE, fPtr);
        if (n == -1) {
            return 0;
        }
    } else {
        return 0;
    }
    password[n] = '\0';
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

int compare(const void * a, const void * b) {
    GROUPINFO * q1 = (GROUPINFO *) a;
    GROUPINFO * q2 = (GROUPINFO *) b;
    return strcmp(q1->no, q2->no);
}

void sortGList(GROUPLIST * list) {
    printf("I'll now qsort with:\n");
    printf("B: list->groupinfo[0].no = %s and list->groupinfo[0].name = %s\n", list->groupinfo[0].no, list->groupinfo[0].name);
    printf("I have %d groups\n", list->no_groups);
    qsort(list->groupinfo, list->no_groups, sizeof(GROUPINFO), compare);
    printf("A: list->groupinfo[0].no = %s and list->groupinfo[0].name = %s\n", list->groupinfo[0].no, list->groupinfo[0].name);
}

int listGroupsDir() {
    DIR * d;
    struct dirent * dir;
    int i = 0;
    FILE * fp;
    char GIDname[530]; // compiler was complaining about this size
    (&dsGroups)->no_groups = 0;
    d = opendir("GROUPS");
    if (d)
    {
        while ((dir = readdir(d)) != NULL) 
        {
            if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
                continue;
            if (strlen(dir->d_name) != 2)
                continue;
            strcpy((&dsGroups)->groupinfo[i].no, dir->d_name);
            sprintf(GIDname, "GROUPS/%s/%s_name.txt", dir->d_name, dir->d_name);
            fp = fopen(GIDname, "r");
            if (fp) 
            {
                fscanf(fp, "%24s", (&dsGroups)->groupinfo[i].name);
                fclose(fp);
            }
            ++i;
            if (i == 99) break;
        }
        (&dsGroups)->no_groups = i;
        closedir(d);
    } else
        return 0;
    if((&dsGroups)->no_groups > 1) {
        sortGList((&dsGroups));
    }
    return ((&dsGroups)->no_groups);
}

char * createGroupListMessage(int numGroups) {
    char listGroups[MAX_RECVUDP_SIZE] = "";
    char * ptr = listGroups;
    ptr += sprintf(ptr, "RGL %d", numGroups);
    if (numGroups > 0) {
        struct dirent ** d;
        int n, flag;
        char groupPath[MAX_GNAME_SIZE];
        for (int i = 0; i < numGroups; ++i) {
            sprintf(groupPath, "GROUPS/%s/MSG", dsGroups.groupinfo[i].no);
            n = scandir(groupPath, &d, 0, alphasort);
            if (n < 0) {
                perror("[-] scandir on grouppath");
            } else {
                flag = 0;
                while (n--) {
                    if (!flag) { 
                        if (validMID(d[n]->d_name)) { // Check for garbage
                            ptr += sprintf(ptr, " %s %s %s", dsGroups.groupinfo[i].no, dsGroups.groupinfo[i].name, d[n]->d_name);
                            flag = 1;
                        }
                    }
                    free(d[n]);
                }
                free(d);
                if (!flag) { // No messages are in the group
                    ptr += sprintf(ptr, " %s %s 0000", dsGroups.groupinfo[i].no, dsGroups.groupinfo[i].name);
                }
            }
        }
    }
    ptr += sprintf(ptr, "\n");
    return strdup(listGroups);
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
