#include "auxfunctions.h"

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

int validFilename(char *fName)
{
    return validRegex(fName, "^[a-zA-Z0-9_-]{1,20}[.]{1}[a-z]{3}$");
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
            { // corner case (./DS -p -v)
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

void logVerbose(char *clientBuf, struct sockaddr_in s)
{
    printf("[!] Client @ %s in port %d sent: %s\n", inet_ntoa(s.sin_addr), ntohs(s.sin_port), clientBuf);
}

int parseUserCommand(char *command)
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
    else if (strcmp(command, "ULS") == 0)
        return USERS_LIST;
    else if (strcmp(command, "PST") == 0)
        return GROUP_POST;
    else
    {
        fprintf(stderr, "[-] Invalid protocol command code received. Please try again.\n");
        return INVALID_COMMAND;
    }
}

char *createStatusMessage(char *command, int statusCode)
{
    char status[MAX_STATUS_SIZE];
    // sprintf terminates with null terminator
    switch (statusCode)
    {
    case ERR:
        sprintf(status, ERR_MSG);
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

int unsubscribeUserGroups(char *UID)
{
    DIR *d;
    struct dirent *group;
    char userSubscribedFile[268];

    if ((d = opendir("GROUPS")) < 0)
        return 0;

    while (group = readdir(d))
    {
        sprintf(userSubscribedFile, "GROUPS/%s/%s.txt", group->d_name, UID);
        if (unlink(userSubscribedFile) != 0 && errno != ENOENT)
        {
            free(d);
            return 0;
        }
    }

    free(d);
    return 1;
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

char *createMessageInGroup(char *GID, char *UID, char *msgText, int msgTextSize)
{
    char groupMsgPath[GROUPMSGDIR_SIZE];
    sprintf(groupMsgPath, "GROUPS/%s/MSG", GID);
    struct dirent **d;
    int n = scandir(groupMsgPath, &d, 0, alphasort);
    int max = 0, flag = 0;
    if (n < 0)
    {
        perror("[-] scandir on post");
        return NULL;
    }
    while (n--)
    {
        if (!flag)
        {
            if (validMID(d[n]->d_name))
            {
                max = atoi(d[n]->d_name);
                flag = 1;
            }
        }
        free(d[n]);
    }
    free(d);
    char newMID[MAX_MID_SIZE];
    if (0 <= max && max <= 8)
    {
        sprintf(newMID, "000%d", max + 1);
    }
    else if (9 <= max && max <= 98)
    {
        sprintf(newMID, "00%d", max + 1);
    }
    else if (99 <= max && max <= 998)
    {
        sprintf(newMID, "0%d", max + 1);
    }
    else if (999 <= max && max <= 9998)
    {
        sprintf(newMID, "%d", max + 1);
    }
    else
    { // message limit
        return NULL;
    }
    char newMIDPath[GROUPNEWMSGDIR_SIZE];
    sprintf(newMIDPath, "GROUPS/%s/MSG/%s", GID, newMID);
    int ret = mkdir(newMIDPath, 0700);
    if (ret == -1)
    {
        perror("[-] Post failed to create new MSG directory");
        return NULL;
    }
    FILE *author, *text;

    // New message's author file
    char newMIDAuthorPath[GROUPNEWMSGAUT_SIZE];
    sprintf(newMIDAuthorPath, "%s/A U T H O R.txt", newMIDPath);
    author = fopen(newMIDAuthorPath, "w");
    if (author == NULL)
    {
        perror("[-] Post failed to create new msg author file");
        return NULL;
    }
    if (fwrite(UID, sizeof(char), MAX_UID_SIZE - 1, author) != MAX_UID_SIZE - 1)
    {
        perror("[-] Post failed to write on new message author file");
        return NULL;
    }
    if (fwrite("\n", sizeof(char), 1, author) != 1)
    { // end author file with \n (same as tejo)
        perror("[-] Post failed to write on new message author file (nl)");
        return NULL;
    }
    if (fclose(author) == -1)
    {
        perror("[-] Post failed to close new msg author file");
        return NULL;
    }

    // New message's text file
    char newMIDTextPath[GROUPNEWMSGTXT_SIZE];
    sprintf(newMIDTextPath, "%s/T E X T.txt", newMIDPath);
    text = fopen(newMIDTextPath, "w");
    if (text == NULL)
    {
        perror("[-] Post failed to create new msg text file");
        return NULL;
    }
    if (fwrite(msgText, sizeof(char), msgTextSize, text) != msgTextSize)
    {
        perror("[-] Post failed to write on new message text file");
        return NULL;
    }
    if (fclose(text) == -1)
    {
        perror("[-] Post failed to close new msg text file");
        return NULL;
    }
    return strdup(newMID);
}

int readFile(int fd, char *GID, char *MID, char *fileName, long int fileSize)
{
    char newMIDFilePath[GROUPNEWMSGFILE_SIZE];
    sprintf(newMIDFilePath, "GROUPS/%s/MSG/%s/%s", GID, MID, fileName);
    FILE *newFile = fopen(newMIDFilePath, "wb");
    if (newFile == NULL)
    {
        perror("[-] Failed to create post file");
        return 0;
    }
    unsigned char fileBuffer[2048] = "";
    long bytesRecv = 0;
    int toRead;
    ssize_t n;
    do
    {
        toRead = MIN(sizeof(fileBuffer), fileSize - bytesRecv);
        n = recv(fd, fileBuffer, toRead, 0);
        if (n == -1)
        {
            if (!(errno == EAGAIN || errno == EWOULDBLOCK))
            {
                perror("[-] Failed to read file on post");
                close(fd);
                exit(EXIT_FAILURE);
            }
            else
            {
                perror("[-] Timeout reading file on post");
                close(fd);
                exit(EXIT_FAILURE);
            }
        }
        if (n > 0)
        {
            bytesRecv += n;
            fwrite(fileBuffer, sizeof(unsigned char), n, newFile);
        }
    } while (bytesRecv < fileSize);
    if (fclose(newFile) == -1)
    {
        perror("[-] Failed to close new file");
        return 0;
    }
    return 1;
}

int compareIDs(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;
}

int validGName(char *gName)
{
    return validRegex(gName, "^[a-zA-Z0-9_-]{1,24}$");
}

/**
 * @brief Reads at most maxSize bytes of given message with a given flag
 *
 * @param message buffer to store message read
 * @param maxSize max number of bytes to be read
 * @param flag 0 or MSG_PEEK
 * @return -1 if recv failed or N > 0 corresponding to the number of bytes received
 */
int readTCP(int fd, char *message, int maxSize, int flag)
{
    int bytesRecv = 0;
    ssize_t n;
    if (flag == MSG_PEEK)
    { // We assume that recv can get at least offset bytes (usually really low -> reliable)
        n = recv(fd, message, maxSize, MSG_PEEK);
        if (n == -1)
        {
            perror("[-] Failed to receive from server on TCP");
        }
        return n;
    }
    while (bytesRecv < maxSize)
    {
        n = recv(fd, message + bytesRecv, maxSize - bytesRecv, 0);
        if (n == -1)
        {
            return n;
        }
        bytesRecv += n;
        if (message[bytesRecv - 1] == '\n')
        { // every request ends with \n (message[n] = '\0')
            // TODO: CHECK THIS CONDITION WITH USER SCRIPT GIVEN BY TEACHERS AND PRINT OUT HOW IT ENDS
            break;
        }
        if (n == 0)
        {
            break;
        }
        bytesRecv += n;
    }
    return bytesRecv;
}

int getAuthorID(char *messageDir, char *authorID)
{
    FILE *fPtr;
    char authorFileName[GROUPNEWMSGAUT_SIZE];
    size_t n;

    sprintf(authorFileName, "%s/A U T H O R", messageDir);

    fPtr = fopen(authorFileName, "r");
    if (fPtr == NULL)
    {
        fprintf(stderr, "[-] Unable to open author file.\n");
        return 0;
    }

    authorID = calloc(sizeof(char), MAX_UID_SIZE);

    if (authorID)
    {
        n = fread(authorID, 1, MAX_UID_SIZE - 1, fPtr);
        if (n == -1)
        {
            fprintf(stderr, "[-] Failed to read from author file.\n");
            return 0;
        }
    }
    else
    {
        fprintf(stderr, "[-] Failed to allocate memory for author.\n");
        return 0;
    }
    authorID[n] = '\0';
    if (fclose(fPtr) == -1)
    {
        fprintf(stderr, "[-] Failed to close author file.\n");
        return 0;
    }

    return 1;
}

int getMessage(char *currMessageDir, char *message, size_t *msg_size)
{
    FILE *fPtr;
    char messageFileName[GROUPNEWMSGTXT_SIZE];
    size_t n;

    sprintf(messageFileName, "%s/T E X T.txt", currMessageDir);
    fPtr = fopen(messageFileName, "r");
    if (fPtr == NULL)
    {
        fprintf(stderr, "[-] Unable to open author file.\n");
        return 0;
    }

    fseek(fPtr, 0, SEEK_END);
    *msg_size = ftell(fPtr);
    fseek(fPtr, 0, SEEK_SET);

    message = calloc(sizeof(char), *msg_size);

    if (message)
    {
        n = fread(message, 1, *msg_size, fPtr);
        if (n == -1)
        {
            fprintf(stderr, "[-] Failed to read from message file.\n");
            return 0;
        }
    }
    else
    {
        fprintf(stderr, "[-] Failed to allocate memory for message.\n");
        return 0;
    }
    message[n] = '\0';
    if (fclose(fPtr) == -1)
    {
        fprintf(stderr, "[-] Failed to close message file.\n");
        return 0;
    }
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
