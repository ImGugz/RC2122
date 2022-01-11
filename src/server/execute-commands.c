#include "execute-commands.h"

GROUPLIST dsGroups;

int userRegister(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    { // wrong protocol message received
        fprintf(stderr, "[-] Incorrect user register command usage.\n");
        return NOK;
    }

    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid register command arguments given.\n");
        return NOK;
    }

    char userDirname[USERDIR_SIZE];
    char passFileName[PATHPWLOGIN_SIZE];
    FILE *fPtr;

    // Create user directory
    sprintf(userDirname, "USERS/%s", tokenList[1]);
    int ret = mkdir(userDirname, 0700);
    if (ret == -1)
    {
        if (errno == EEXIST)
        { // this folder already exists -> duplicate user
            return DUP;
        }
        else
        { // other errno error -> registration process failed (NOK)
            return NOK;
        }
    }
    // Create password file
    sprintf(passFileName, "%s/%s_pass.txt", userDirname, tokenList[1]);
    fPtr = fopen(passFileName, "w");
    if (fPtr == NULL)
    {
        fprintf(stderr, "[-] Unable to create file.\n");
        return NOK;
    }
    if (fwrite(tokenList[2], sizeof(char), USERPW_SIZE, fPtr) != USERPW_SIZE)
    {
        fprintf(stderr, "[-] Writing password to file failed.\n");
        return NOK;
    }
    fclose(fPtr);
    return OK;
}

int userUnregister(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    { // wrong protocol message received
        fprintf(stderr, "[-] Incorrect user unregister command usage.\n");
        return NOK;
    }
    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid unregister command arguments given.\n");
        return NOK;
    }

    char userDirname[USERDIR_SIZE];
    sprintf(userDirname, "USERS/%s", tokenList[1]);

    if (!directoryExists(userDirname))
    { // user wasn't previously registered - no dir folder
        fprintf(stderr, "[-] %s wasn't previously registered.\n", tokenList[1]);
        return NOK;
    }

    if (!passwordsMatch(tokenList[1], tokenList[2]))
    { // given and stored passwords do not match
        fprintf(stderr, "[-] Given password doesn't match the stored one.\n");
        return NOK;
    }

    if (removeDirectory(userDirname) == -1)
    { // user directory failed to be removed
        fprintf(stderr, "[-] Failed to remove user directory.\n");
        return NOK;
    }
    return OK;
}

int userLogin(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    { // wrong protocol message received
        fprintf(stderr, "[-] Incorrect user login command usage.\n");
        return NOK;
    }

    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid login command arguments given.\n");
        return NOK;
    }

    char userDirname[USERDIR_SIZE];
    sprintf(userDirname, "USERS/%s", tokenList[1]);

    if (!directoryExists(userDirname))
    { // user isn't registered -> can't login
        fprintf(stderr, "[-] User %s isn't registered.\n", tokenList[1]);
        return NOK;
    }

    if (!passwordsMatch(tokenList[1], tokenList[2]))
    { // given and stored passwords do not match
        fprintf(stderr, "[-] Given password doesn't match the stored one.\n");
        return NOK;
    }

    char loginFileName[PATHPWLOGIN_SIZE];
    sprintf(loginFileName, "%s/%s_login.txt", userDirname, tokenList[1]);
    FILE *fp = fopen(loginFileName, "a"); // create dummy file
    if (fp == NULL)
    {
        fprintf(stderr, "[-] Unable to create user login file.\n");
        return NOK;
    }

    if (fclose(fp) == -1)
    {
        fprintf(stderr, "[-] Unable to close login file.\n");
        return NOK;
    }

    return OK;
}

int userLogout(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    { // wrong protocol message received
        fprintf(stderr, "[-] Incorrect user login command usage.\n");
        return NOK;
    }

    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid login command arguments given.\n");
        return NOK;
    }

    char userDirname[USERDIR_SIZE];
    sprintf(userDirname, "USERS/%s", tokenList[1]);
    if (!directoryExists(userDirname))
    {
        fprintf(stderr, "[-] User %s isn't registered.\n", tokenList[1]);
        return NOK;
    }

    if (!passwordsMatch(tokenList[1], tokenList[2]))
    {
        fprintf(stderr, "[-] Given password doesn't match the stored one.\n");
        return NOK;
    }

    FILE *fp;
    char loginFileName[PATHPWLOGIN_SIZE];
    sprintf(loginFileName, "%s/%s_login.txt", userDirname, tokenList[1]);
    if (!access(loginFileName, F_OK))
    { // file exists
        if (!unlink(loginFileName))
        { // login file was deleted
            return OK;
        }
    }
    // if it gets here either unlink failed or user isn't logged in
    fprintf(stderr, "[-] Unable to access/delete login file.\n");
    return NOK;
}

char *createGroupListMessage(char *code, int *groups, int num)
{
    char listGroups[MAX_SENDUDP_SIZE] = "";
    char *ptr = listGroups;
    int numGroups = (groups == NULL) ? dsGroups.no_groups : num;
    sortGList((&dsGroups));
    ptr += sprintf(ptr, "%s %d", code, numGroups);
    if (numGroups > 0)
    {
        struct dirent **d;
        int n, flag;
        int j;
        char groupPath[GROUPMSGDIR_SIZE];
        for (int i = 0; i < numGroups; ++i)
        {
            j = (groups == NULL) ? i : groups[i];
            sprintf(groupPath, "GROUPS/%s/MSG", dsGroups.groupinfo[j].no);
            printf("groupPath: %s\n", groupPath);
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
                            ptr += sprintf(ptr, " %s %s %s", dsGroups.groupinfo[j].no, dsGroups.groupinfo[j].name, d[n]->d_name);
                            flag = 1;
                        }
                    }
                    free(d[n]);
                }
                free(d);
                if (!flag)
                { // No messages are in the group
                    ptr += sprintf(ptr, " %s %s 0000", dsGroups.groupinfo[j].no, dsGroups.groupinfo[j].name);
                }
            }
        }
    }
    ptr += sprintf(ptr, "\n");
    return strdup(listGroups);
}

int userSubscribe(char **tokenList, int numTokens, char **newGID)
{
    if (numTokens != 4)
    { // wrong protocol message received
        fprintf(stderr, "[-] Incorrect user subscribe command usage.\n");
        return NOK;
    }
    if (!(validUID(tokenList[1]) && isGID(tokenList[2]) && validGName(tokenList[3])))
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid user subscribe command arguments.\n");
        return NOK;
    }

    char userDirname[USERDIR_SIZE];
    sprintf(userDirname, "USERS/%s", tokenList[1]);
    if (!directoryExists(userDirname))
    { // check if user is registered
        fprintf(stderr, "[-] User %s isn't registered.\n", tokenList[1]);
        return E_USR;
    }

    char loginFileName[PATHPWLOGIN_SIZE];
    sprintf(loginFileName, "%s/%s_login.txt", userDirname, tokenList[1]);
    if (access(loginFileName, F_OK))
    { // check if user is logged in
        fprintf(stderr, "[-] User %s isn't currently logged in.\n", tokenList[1]);
        return E_USR;
    }

    int numGroup = atoi(tokenList[2]);
    if (numGroup > dsGroups.no_groups)
    { // given GID doesn't exist
        fprintf(stderr, "[-] Invalid group ID given.\n");
        return E_GRP;
    }

    if (!strcmp(tokenList[2], "00"))
    { // create a new group
        if (dsGroups.no_groups == 99)
        { // can't create any other group
            return E_FULL;
        }

        char *temp = (char *)calloc(sizeof(char), MAX_GID_SIZE);
        if (!temp)
        {
            fprintf(stderr, "[-] Failed to allocate memory in heap.\n");
            return NOK;
        }
        *newGID = temp;
        if (dsGroups.no_groups < 9)
        { // + 1 is used because we're yet to actually create the new group directory
            sprintf(*newGID, "0%d", dsGroups.no_groups + 1);
        }
        else
        {
            sprintf(*newGID, "%d", dsGroups.no_groups + 1);
        }

        for (int i = 0; i < dsGroups.no_groups; ++i)
        { // check if there's already a group with this name
            if (!strcmp(tokenList[3], dsGroups.groupinfo[i].name))
            {
                free(*newGID);
                *newGID = NULL;
                return E_GNAME;
            }
        }

        // Add group to directory
        char groupDirName[GROUPDIR_SIZE];
        char msgDirName[GROUPMSGDIR_SIZE];
        char groupNameFile[GROUPNAMEFILE_SIZE];
        char userSubscribedFile[GROUPUSERSUBFILE_SIZE];
        char groupNameBuffer[GROUPNAME_SIZE];

        // Create group directory
        sprintf(groupDirName, "GROUPS/%s", *newGID);
        int ret = mkdir(groupDirName, 0700);
        if (ret == -1)
        {
            fprintf(stderr, "[-] Failed to create new group directory.\n");
            free(*newGID);
            *newGID = NULL;
            return NOK;
        }

        // Create messages directory
        sprintf(msgDirName, "%s/MSG", groupDirName);
        ret = mkdir(msgDirName, 0700);
        if (ret == -1)
        {
            fprintf(stderr, "[-] Failed to create new group messages' directory.\n");
            free(*newGID);
            *newGID = NULL;
            return NOK;
        }

        // Create name file
        FILE *fPtr;
        sprintf(groupNameFile, "%s/%s_name.txt", groupDirName, *newGID);
        fPtr = fopen(groupNameFile, "w");
        if (fPtr == NULL)
        {
            fprintf(stderr, "[-] Unable to create group name file.\n");
            free(*newGID);
            *newGID = NULL;
            return NOK;
        }
        sprintf(groupNameBuffer, "%s\n", tokenList[3]); // the groups in the given DS files had a \n after their name so we replicate behaviour
        size_t lenGroupName = strlen(groupNameBuffer);
        if (fwrite(groupNameBuffer, sizeof(char), lenGroupName, fPtr) != lenGroupName)
        {
            fprintf(stderr, "[-] Failed to write group name on group name file.\n");
            free(*newGID);
            *newGID = NULL;
            return NOK;
        }
        if (fclose(fPtr) == -1)
        {
            fprintf(stderr, "[-] Failed to close group name file.\n");
            free(*newGID);
            *newGID = NULL;
            return NOK;
        }

        // Create subscribed user file
        sprintf(userSubscribedFile, "%s/%s.txt", groupDirName, tokenList[1]);
        fPtr = fopen(userSubscribedFile, "a");
        if (fPtr == NULL)
        {
            fprintf(stderr, "[-] Unable to create subscribed user file.\n");
            free(*newGID);
            *newGID = NULL;
            return NOK;
        }
        if (fclose(fPtr) == -1)
        {
            fprintf(stderr, "[-] Unable to close user subscribed file.\n");
            free(*newGID);
            *newGID = NULL;
            return NOK;
        }

        // Add new group to global groups struct
        int n = dsGroups.no_groups;
        strcpy(dsGroups.groupinfo[n].name, tokenList[3]);
        sprintf(dsGroups.groupinfo[n].no, "%s", *newGID);
        dsGroups.no_groups++;
        return NEW;
    }

    // This else is safe to assume because isGID has already made sure that the given GID is valid (00 - 99)
    else
    { // Subscribe to existing group

        if (!groupNamesMatch(tokenList[2], tokenList[3]))
        { // check if given group name matches the stored one
            fprintf(stderr, "[-] Wrong group name given.\n");
            return E_GNAME;
        }

        // Create subscribed user file
        char userSubscribedFile[GROUPUSERSUBFILE_SIZE];
        sprintf(userSubscribedFile, "GROUPS/%s/%s.txt", tokenList[2], tokenList[1]);
        FILE *fPtr = fopen(userSubscribedFile, "a");
        if (fPtr == NULL)
        {
            fprintf(stderr, "[-] Unable to create subscribed user file.\n");
            return NOK;
        }
        if (fclose(fPtr) == -1)
        {
            fprintf(stderr, "[-] Unable to close user subscribed file.\n");
            return NOK;
        }
        return OK;
    }
}

int userUnsubscribe(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    { // wrong protocol message received
        fprintf(stderr, "[-] Incorrect user subscribe command usage.\n");
        return NOK;
    }

    if (!(validUID(tokenList[1]) && validGID(tokenList[2])))
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid unsubscribe command arguments given.\n");
        return NOK;
    }

    char userDirname[USERDIR_SIZE];
    sprintf(userDirname, "USERS/%s", tokenList[1]);
    if (!directoryExists(userDirname))
    { // check if user is registered
        fprintf(stderr, "[-] User %s is not previously registered.\n", tokenList[1]);
        return E_USR;
    }

    char loginFileName[PATHPWLOGIN_SIZE];
    sprintf(loginFileName, "%s/%s_login.txt", userDirname, tokenList[1]);
    if (access(loginFileName, F_OK))
    { // check if user is logged in
        fprintf(stderr, "[-] User %s isn't currently logged in.\n", tokenList[1]);
        return E_USR;
    }

    char groupDirName[GROUPDIR_SIZE];
    sprintf(groupDirName, "GROUPS/%s", tokenList[2]);
    if (!directoryExists(groupDirName))
    { // check if group exists
        fprintf(stderr, "[-] Group received doesn't exist.\n");
        return E_GRP;
    }

    // if it gets here then UID and GID are valid and exist
    char userSubscribedFile[GROUPUSERSUBFILE_SIZE];
    sprintf(userSubscribedFile, "%s/%s.txt", groupDirName, tokenList[1]);
    if (unlink(userSubscribedFile) != 0 && errno != ENOENT)
    { // errno = ENOENT means that the file doesn't exist -> the UID wasn't subscribed
        fprintf(stderr, "[-] Failed to delete user file from group directory.\n");
        return NOK;
    }
    return OK;
}

char *createUserGroupsMessage(char **tokenList, int numTokens)
{
    if (numTokens != 2)
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid GLM message received.\n");
        return strdup(ERR_MSG);
    }
    if (!(validUID(tokenList[1])))
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid GLM arguments received.\n");
        return strdup(ERR_MSG);
    }
    DIR *d;
    struct dirent *dir;
    char userSubscribeFile[DIRNAME_SIZE];
    int groupsSubscribed[MAX_GROUPS];
    int numGroupsSub = 0;
    d = opendir("GROUPS");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
            {
                continue;
            }
            if (strlen(dir->d_name) != 2)
            {
                continue;
            }
            sprintf(userSubscribeFile, "GROUPS/%s/%s.txt", dir->d_name, tokenList[1]);
            if (!access(userSubscribeFile, F_OK))
            { // user is subscribed to this group - save its index
                groupsSubscribed[numGroupsSub++] = atoi(dir->d_name) - 1;
            }
        }
    }
    if (numGroupsSub > 0)
    { // sort group index's
        qsort(groupsSubscribed, numGroupsSub, sizeof(int), compareIDs);
    }
    return createGroupListMessage("RGM", groupsSubscribed, numGroupsSub);
}

char *createUsersInGroupMessage(int acceptfd, char *peekedMsg)
{
    char clientBuf[ULCLIENT_BUF_SIZE + 1] = "";
    char givenGID[MAX_GID_SIZE];
    readTCP(acceptfd, clientBuf, ULCLIENT_BUF_SIZE, 0); // actually read all bytes - if it gets here then timeout isn't need to check
    sscanf(clientBuf, "ULS %2s", givenGID);
    if (!(clientBuf[ULCLIENT_BUF_SIZE - 1] == '\n'))
    { // every request must end with \n
        fprintf(stderr, "[-] Wrong ulist command received according to protocol.\n");
        exit(EXIT_FAILURE); // if not abandon new process execution
    }
    if (!validGID(givenGID))
    { // server at tejo exits
        fprintf(stderr, "[-] Invalid ULS arguments receibed.\n");
        return strdup("RUL NOK\n");
    }
    sortGList((&dsGroups));
    char *users, *tmp;
    int len = 0, curr = 0;
    int groupNum = atoi(givenGID);
    if (groupNum > dsGroups.no_groups)
    { // group doesn't exist - tejo returns NOK
        return strdup("RUL NOK\n");
    }
    char groupDirName[GROUPDIR_SIZE];
    sprintf(groupDirName, "GROUPS/%s", givenGID);
    DIR *d;
    d = opendir(groupDirName);
    if (d)
    {
        struct dirent *dir;
        char UIDtxt[MAX_UID_SIZE];
        tmp = (char *)calloc(sizeof(char), INITIAL_ULBUF_SIZE + 1); // 10 users
        if (!tmp)
        {
            fprintf(stderr, "[-] Failed on calloc ULS.\n");
            return strdup("RUL NOK\n");
        }
        users = tmp;
        len = INITIAL_ULBUF_SIZE;
        curr += sprintf(users + curr, "RUL OK %s ", givenGID);
        while ((dir = readdir(d)) != NULL)
        {
            if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
            {
                continue;
            }
            if (strlen(dir->d_name) != 9) // UID + 1 + 3
            {
                continue;
            }
            strncpy(UIDtxt, dir->d_name, MAX_UID_SIZE - 1);
            UIDtxt[MAX_UID_SIZE - 1] = '\0';
            if (validUID(UIDtxt))
            {
                if (curr + MAX_UID_SIZE >= len)
                {
                    char *newPtr = (char *)realloc(users, len + 6 * 10); // 10 more users
                    if (newPtr == NULL)
                    {
                        fprintf(stderr, "[-] Failed on realloc ULS.\n");
                        free(users);
                        return strdup("RUL NOK\n");
                    }
                    users = newPtr;
                    len += MAX_UID_SIZE * 10;
                }
                curr += sprintf(users + curr, "%s ", UIDtxt);
            }
        }
    }
    curr += sprintf(users + curr, "\n");
    return users;
}

char *userPost(int acceptfd, char *peekedMsg, int recvBytes)
{
    char clientBuf[MAX_RECVTCP_SIZE];
    char givenUID[MAX_UID_SIZE], givenGID[MAX_GID_SIZE], givenTextSize[MAX_TEXTSZ_SIZE], givenText[MAX_PSTTEXT_SIZE];
    int textSize;
    sscanf(peekedMsg, "PST %s %s %s ", givenUID, givenGID, givenTextSize); // assumes at least offset bytes will come
    textSize = atoi(givenTextSize);
    if (!(validUID(givenUID) && validGID(givenGID) && textSize <= 240))
    { // abort new process execution
        fprintf(stderr, "[-] Wrong post command received according to protocol.\n");
        close(acceptfd);
        exit(EXIT_FAILURE);
    }
    char *tmpPtr = clientBuf;
    size_t initialPostOffset = 3 + 1 + strlen(givenUID) + 1 + strlen(givenGID) + 1 + strlen(givenTextSize) + 1; // << 259
    readTCP(acceptfd, clientBuf, initialPostOffset, 0);
    clientBuf[recvBytes] = '\0';
    tmpPtr += initialPostOffset;
    readTCP(acceptfd, givenText, textSize + 1, 0);
    givenText[textSize + 1] = '\0';
    char *newMID = createMessageInGroup(givenGID, givenUID, givenText, textSize);
    if (!newMID)
    {
        return "NOK";
    }
    if (givenText[textSize] != '\n')
    { // text + file case
        char fileInfo[MAX_FILEINFO_SIZE];
        char fileName[MAX_FILENAME_SIZE], fileSize[MAX_FILESZ_SIZE];
        int n = readTCP(acceptfd, fileInfo, MAX_FILEINFO_SIZE - 1, MSG_PEEK);
        if (n == -1)
        {
            if (!(errno == EAGAIN || errno == EWOULDBLOCK))
            {
                perror("[-] Failed to read file on post");
                close(acceptfd);
                exit(EXIT_FAILURE);
            }
            else
            {
                perror("[-] Timeout reading file on post");
                close(acceptfd);
                exit(EXIT_FAILURE);
            }
        }
        fileInfo[MAX_FILEINFO_SIZE - 1] = '\0';
        sscanf(fileInfo, "%s %s ", fileName, fileSize);
        if (!(validFilename(fileName) && strlen(fileSize) <= 10))
        {
            fprintf(stderr, "[-] Incorrect filename or file size format.\n");
            exit(EXIT_FAILURE);
        }
        size_t offset = strlen(fileName) + 1 + strlen(fileSize) + 1;
        char *tmp = (char *)calloc(sizeof(char), offset + 1);
        if (tmp == NULL)
        {
            fprintf(stderr, "[-] Failed to allocate on post file");
            return "NOK";
        }
        char *offsetBuf = tmp;
        recv(acceptfd, offsetBuf, offset, 0);
        offsetBuf[offset] = '\0';
        free(offsetBuf);
        offsetBuf = NULL;
        long fileSz = atol(fileSize);
        int post = readFile(acceptfd, givenGID, newMID, fileName, fileSz);
        if (!post)
        {
            return "NOK";
        }
    }
    return newMID;
}

void createRetrieveMessage(int acceptfd, char *peekedMsg)
{
    char UID[MAX_UID_SIZE], GID[MAX_GID_SIZE], MID[MAX_MID_SIZE];
    char temp[300];
    sscanf(peekedMsg, "RTV %s %s %s", UID, GID, MID);
    int nTEMP;
    if ((nTEMP = readTCP(acceptfd, temp, strlen(peekedMsg) + 1, 0)) == -1)
    {
        fprintf(stderr, "[-] Failed to read dumbfuck.\n");
        exit(EXIT_FAILURE);
    }
    temp[nTEMP] = '\0';
    printf("temp: %s\n", temp);
    if (peekedMsg[strlen(peekedMsg) - 1] != '\n')
    { // every reply/request must end with \n
        fprintf(stderr, "[-] Wrong retrieve command received according to protocol.\n");
        failRetrieve(acceptfd, "ERR");
    }
    if (!(validUID(UID) && validGID(GID) && validMID(MID)))
    { // check if given arguments are protocol valid
        fprintf(stderr, "[-] Invalid retrieve command arguments were given.\n");
        failRetrieve(acceptfd, "NOK");
    }
    if (!userSubscribedToGroup(UID, GID))
    { // check if user is subscribed to group
        fprintf(stderr, "[-] User isn't subscribed to group.\n");
        failRetrieve(acceptfd, "NOK");
    }
    struct dirent **d;
    char groupPath[GROUPMSGDIR_SIZE];
    sprintf(groupPath, "GROUPS/%s/MSG", GID);
    int n = scandir(groupPath, &d, 0, invsort);
    if (n < 0)
    { // scandir failed
        perror("[-] scandir on retrieve groupPath failed");
        failRetrieve(acceptfd, "NOK");
    }
    int numMsgsRetrieve = numMessagesToRetrieve(d, n, MID);
    if (numMsgsRetrieve == 0)
    { // no messages to retrieve
        failRetrieve(acceptfd, "EOF");
    }
    else if (numMsgsRetrieve > 20)
    {
        numMsgsRetrieve = 20;
    }
    char buffer[100];
    sprintf(buffer, "RRT OK %d", numMsgsRetrieve);
    sendTCP(acceptfd, buffer, strlen(buffer));
    int count = 0;
    while (n--)
    {
        if (d[n]->d_type == DT_DIR && validMID(d[n]->d_name) && atoi(d[n]->d_name) >= atoi(MID) && count < numMsgsRetrieve)
        { // this is a message to retrieve
            char textMessage[270];
            char fileMessage[50];
            char readDName[DIRENTDIR_SIZE + 1];
            char dirMID[MAX_MID_SIZE] = "";
            char groupMsgPath[GROUPNEWMSGDIR_SIZE];
            DIR *msgsDir;
            struct dirent *msgEntry;
            int msgFileFlag = 0;
            char msgFileName[MAX_FILENAME_SIZE] = "";
            char msgFilePath[GROUPNEWMSGDIR_SIZE + MAX_FILENAME_SIZE] = "";
            long fileLength = 0;
            FILE *fileStream;
            strcpy(readDName, d[n]->d_name);
            strncpy(dirMID, readDName, 4);
            dirMID[MAX_MID_SIZE - 1] = '\0';
            if (!validMID(dirMID))
            { // for some reason it doesn't store a valid MID
                fprintf(stderr, "[-] Invalid MID was found: %s\n", dirMID);
                failRetrieve(acceptfd, "NOK");
            }
            sprintf(groupMsgPath, "GROUPS/%s/MSG/%s", GID, dirMID);
            // For each valid MID we're opening its directory and analysing its content
            msgsDir = opendir(groupMsgPath);
            if (msgsDir == NULL)
            {
                perror("[-] Failed to open msg dir");
                failRetrieve(acceptfd, "NOK");
            }
            while ((msgEntry = readdir(msgsDir)) != NULL)
            {
                if (!strcmp(".", msgEntry->d_name) || !strcmp("..", msgEntry->d_name) || !strcmp(msgEntry->d_name, "A U T H O R.txt") || !strcmp(msgEntry->d_name, "T E X T.txt"))
                {
                    continue;
                }
                // Check if message to retrieve has a file to send
                if (msgEntry->d_type == DT_REG)
                { // if it gets here it means this message contains a file to be sent to the client
                    msgFileFlag = 1;
                    // Now let's copy its name and determine its length in bytes
                    memset(readDName, 0, sizeof(readDName));
                    memset(msgFileName, 0, sizeof(msgFileName));
                    memset(msgFilePath, 0, sizeof(msgFilePath));
                    strcpy(readDName, msgEntry->d_name);
                    strncpy(msgFileName, readDName, strlen(readDName));
                    sprintf(msgFilePath, "%s/%s", groupMsgPath, msgFileName);
                    fileStream = fopen(msgFilePath, "rb");
                    if (fileStream == NULL)
                    {
                        perror("[-] Failed to open msg file");
                        failRetrieve(acceptfd, "NOK");
                    }
                    if (fseek(fileStream, 0, SEEK_END) == -1)
                    {
                        perror("[-] Failed to seek on msg file");
                        failRetrieve(acceptfd, "NOK");
                    }
                    fileLength = ftell(fileStream);
                    rewind(fileStream);
                    fclose(fileStream);
                }
            }
            free(msgsDir);
            // Open author and text files and read them
            FILE *autStream, *txtStream;
            char msgAuthorPath[GROUPNEWMSGAUT_SIZE], msgTextPath[GROUPNEWMSGTXT_SIZE];
            sprintf(msgAuthorPath, "%s/A U T H O R.txt", groupMsgPath);
            sprintf(msgTextPath, "%s/T E X T.txt", groupMsgPath);
            long lenAuthor, lenText;
            autStream = fopen(msgAuthorPath, "r");
            if (autStream == NULL)
            {
                perror("[-] Failed to open msg author file");
                failRetrieve(acceptfd, "NOK");
            }
            if (fseek(autStream, 0, SEEK_END) == -1)
            {
                perror("[-] Failed to seek msg author file");
                failRetrieve(acceptfd, "NOK");
            }
            lenAuthor = ftell(autStream) - 1; // author has \n so we avoid this extra byte
            rewind(autStream);
            txtStream = fopen(msgTextPath, "r");
            if (txtStream == NULL)
            {
                perror("[-] Failed to open msg text file");
                failRetrieve(acceptfd, "NOK");
            }
            if (fseek(txtStream, 0, SEEK_END) == -1)
            {
                perror("[-] Failed to seek msg author file");
                failRetrieve(acceptfd, "NOK");
            }
            lenText = ftell(txtStream);
            rewind(txtStream);
            // allocate needed memory for both buffers
            char *msgAuthor = (char *)calloc(sizeof(char), lenAuthor + 1);
            if (!msgAuthor)
            {
                fprintf(stderr, "[-] Failed to calloc on author\n");
                failRetrieve(acceptfd, "NOK");
            }
            char *msgText = (char *)calloc(sizeof(char), lenText + 1);
            if (!msgText)
            {
                fprintf(stderr, "[-] Failed to calloc on text\n");
                free(msgAuthor);
                failRetrieve(acceptfd, "NOK");
            }
            // fill both buffers
            if (fread(msgAuthor, sizeof(char), lenAuthor, autStream) == -1)
            {
                perror("[-] Failed to read from author file");
                failRetrieve(acceptfd, "NOK");
            }
            if (fread(msgText, sizeof(char), lenText, txtStream) == -1)
            {
                perror("[-] Failed to read from text file");
                failRetrieve(acceptfd, "NOK");
            }
            if (fclose(autStream) == -1)
            {
                perror("[-] Failed to close author file");
                failRetrieve(acceptfd, "NOK");
            }
            if (fclose(txtStream) == -1)
            {
                perror("[-] Failed to close text file");
                failRetrieve(acceptfd, "NOK");
            }
            msgAuthor[lenAuthor] = '\0';
            msgText[lenText] = '\0';
            if (count == numMsgsRetrieve - 1 && !msgFileFlag)
            {
                sprintf(textMessage, " %s %s %ld %s\n", dirMID, msgAuthor, lenText, msgText);
            }
            else
            {
                sprintf(textMessage, " %s %s %ld %s", dirMID, msgAuthor, lenText, msgText);
            }
            sendTCP(acceptfd, textMessage, strlen(textMessage));
            if (msgFileFlag)
            { // there's a file to send
                char fileInfo[41];
                fileStream = fopen(msgFilePath, "rb");
                sprintf(fileInfo, " / %s %ld ", msgFileName, fileLength);
                sendTCP(acceptfd, fileInfo, strlen(fileInfo));
                printf("Filename and file lenght: %s %ld\n", msgFileName, fileLength);
                if (!sendFile(acceptfd, fileStream, fileLength))
                {
                    failRetrieve(acceptfd, "NOK");
                }
                if (count == numMsgsRetrieve - 1)
                {
                    printf("I send in count = %d\n", count);
                    sendTCP(acceptfd, "\n", 1);
                }
            }
            free(msgAuthor);
            free(msgText);
            count++;
        }
        free(d[n]);
    }
    free(d);
    char userConfirmation[4];
    size_t n_bytes;

    if ((n_bytes = readTCP(acceptfd, userConfirmation, 3, 0)) == -1)
    {
        failRetrieve(acceptfd, "NOK");
    }
    userConfirmation[3] = '\0';
    printf("This shit has %ld bytes\n", n_bytes);
    printf("Read this shit: %s\n", userConfirmation);
}