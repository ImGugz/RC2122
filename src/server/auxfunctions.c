#include "auxfunctions.h"

#define DIRNAME_SIZE 530

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

int isGID(char *GID)
{
    return validRegex(GID, "^[0-9]{2}");
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

int validPort(char *portStr)
{
    return validRegex(portStr, "^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$");
}

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
    char status[MAX_SENDUDP_SIZE];
    // sprintf terminates with null terminator
    switch (statusCode)
    {
    case ERR:
        sprintf(status, "ERR\n");
        break;
    case OK:
        sprintf(status, "%s %s", command, "OK\n");
        break;
    case NOK:
        sprintf(status, "%s %s", command, "NOK\n");
        break;
    case DUP:
        sprintf(status, "%s %s", command, "DUP\n");
        break;
    case E_USR:
        sprintf(status, "%s %s", command, "E_USR\n");
        break;
    case E_GRP:
        sprintf(status, "%s %s", command, "E_GRP\n");
        break;
    case E_GNAME:
        sprintf(status, "%s %s", command, "E_GNAME\n");
        break;
    case E_FULL:
        sprintf(status, "%s %s", command, "E_FULL\n");
        break;
    default:
        break;
    }
    return strdup(status);
}

char *createSubscribeMessage(int statusCode, char *GID)
{
    if (GID == NULL)
    {
        return createStatusMessage("RGS", statusCode);
    }
    else
    {
        char status[MAX_SENDUDP_SIZE];
        sprintf(status, "RGS NEW %s\n", GID);
        free(GID);
        return strdup(status);
    }
}

int removeDirectory(const char *path)
{
    DIR *d = opendir(path);
    size_t pathLen = strlen(path);
    int r = -1;
    if (d)
    {
        struct dirent *dir;
        r = 0;
        while (!r && (dir = readdir(d)))
        {
            int r2 = -1;
            char *buf;
            size_t len;
            // Skip the names "." and ".." as we don't want to recurse on them
            if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
                continue;
            len = pathLen + strlen(dir->d_name) + 2;
            buf = malloc(len);
            if (buf)
            {
                struct stat statbuf;
                snprintf(buf, len, "%s/%s", path, dir->d_name);
                if (!stat(buf, &statbuf))
                {
                    if (S_ISDIR(statbuf.st_mode))
                    {
                        r2 = removeDirectory(buf);
                    }
                    else
                    {
                        r2 = unlink(buf);
                    }
                }
                free(buf);
            }
            r = r2;
        }
        closedir(d);
    }
    if (!r)
    {
        r = rmdir(path);
    }
    return r;
}

int directoryExists(const char *path)
{
    struct stat stats;
    if (stat(path, &stats))
        return 0;
    return S_ISDIR(stats.st_mode);
}

int passwordsMatch(const char *userID, const char *userPW)
{
    FILE *fPtr;
    char passFileName[PATHPWLOGIN_SIZE];
    char *password;
    size_t n;

    sprintf(passFileName, "USERS/%s/%s_pass.txt", userID, userID);

    fPtr = fopen(passFileName, "r");
    if (fPtr == NULL)
    {
        fprintf(stderr, "[-] Unable to open password file.\n");
        return 0;
    }

    password = calloc(sizeof(char), USERPW_SIZE + 1);

    if (password)
    {
        n = fread(password, 1, USERPW_SIZE, fPtr);
        if (n == -1)
        {
            fprintf(stderr, "[-] Failed to read from password file.\n");
            return 0;
        }
    }
    else
    {
        fprintf(stderr, "[-] Failed to allocate memory for password.\n");
        return 0;
    }
    password[n] = '\0';
    if (fclose(fPtr) == -1)
    {
        fprintf(stderr, "[-] Failed to close password file.\n");
        return 0;
    }

    if (strcmp(password, userPW))
    {
        free(password);
        return 0;
    }

    free(password);
    return 1;
}

int groupNamesMatch(const char *GID, const char *groupName)
{
    FILE *fPtr;
    char groupNameFile[GROUPNAMEFILE_SIZE];
    size_t n;

    sprintf(groupNameFile, "GROUPS/%s/%s_name.txt", GID, GID);
    fPtr = fopen(groupNameFile, "r");
    if (fPtr == NULL)
    {
        fprintf(stderr, "[-] Unable to open group name file.\n");
        return 0;
    }
    size_t lenGivenGroup = strlen(groupName);
    fseek(fPtr, 0, SEEK_END);
    int length = ftell(fPtr) - 1; // -1 to avoid \n
    fseek(fPtr, 0, SEEK_SET);
    char *realGroupName = calloc(sizeof(char), length + 1);
    if (realGroupName)
    {
        n = fread(realGroupName, 1, length, fPtr);
        if (n == -1)
        {
            fprintf(stderr, "[-] Unable to read from group name file.\n");
            return 0;
        }
    }
    else
    {
        fprintf(stderr, "[-] Unable to allocate memory.\n");
        return 0;
    }
    fclose(fPtr);
    realGroupName[n] = '\0';
    if (strcmp(realGroupName, groupName) != 0)
    { // group names don't match
        free(realGroupName);
        return 0;
    }
    // group names match
    free(realGroupName);
    return 1;
}

int compare(const void *a, const void *b)
{
    GROUPINFO *q1 = (GROUPINFO *)a;
    GROUPINFO *q2 = (GROUPINFO *)b;
    return strcmp(q1->no, q2->no);
}

void sortGList(GROUPLIST *list)
{
    qsort(list->groupinfo, list->no_groups, sizeof(GROUPINFO), compare);
}

void fillGroupsInfo()
{
    DIR *d;
    struct dirent *dir;
    int i = 0;
    FILE *fp;
    char GIDname[DIRNAME_SIZE]; // compiler was complaining about this size
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
            if (i == 99)
            {
                break;
            }
        }
        (&dsGroups)->no_groups = i;
        closedir(d);
    }
}

char *createGroupListMessage()
{
    char listGroups[MAX_SENDUDP_SIZE] = "";
    char *ptr = listGroups;
    int numGroups = dsGroups.no_groups;
    sortGList((&dsGroups));
    ptr += sprintf(ptr, "RGL %d", numGroups);
    if (numGroups > 0)
    {
        struct dirent **d;
        int n, flag;
        char groupPath[MAX_GNAME_SIZE];
        for (int i = 0; i < numGroups; ++i)
        {
            sprintf(groupPath, "GROUPS/%s/MSG", dsGroups.groupinfo[i].no);
            n = scandir(groupPath, &d, 0, alphasort);
            if (n < 0)
            {
                perror("[-] scandir on grouppath");
            }
            else
            {
                flag = 0;
                while (n--)
                {
                    if (!flag)
                    {
                        if (validMID(d[n]->d_name))
                        { // Check for garbage
                            ptr += sprintf(ptr, " %s %s %s", dsGroups.groupinfo[i].no, dsGroups.groupinfo[i].name, d[n]->d_name);
                            flag = 1;
                        }
                    }
                    free(d[n]);
                }
                free(d);
                if (!flag)
                { // No messages are in the group
                    ptr += sprintf(ptr, " %s %s 0000", dsGroups.groupinfo[i].no, dsGroups.groupinfo[i].name);
                }
            }
        }
    }
    ptr += sprintf(ptr, "\n");
    return strdup(listGroups);
}

int validGName(char *gName)
{
    return validRegex(gName, "^[a-zA-Z0-9_-]{1,24}$");
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
