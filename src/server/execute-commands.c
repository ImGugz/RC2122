#include "execute-commands.h"

GROUPLIST dsGroups;

int userRegister(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    { // wrong protocol message received
        fprintf(stderr, "[-] Incorrect user register command usage.\n");
        return ERR;
    }

    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid register command arguments given.");
        return ERR;
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
        return ERR;
    }
    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid unregister command arguments given.");
        return ERR;
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
        return ERR;
    }

    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid login command arguments given.");
        return ERR;
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
        fprintf(stderr, "[-] Invalid login command arguments given.");
        return ERR;
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
        char groupPath[MAX_GNAME_SIZE];
        for (int i = 0; i < numGroups; ++i)
        {
            j = (groups == NULL) ? i : groups[i];
            sprintf(groupPath, "GROUPS/%s/MSG", dsGroups.groupinfo[j].no);
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
        return ERR;
    }
    if (!(validUID(tokenList[1]) && isGID(tokenList[2]) && validGName(tokenList[3])))
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid user subscribe command arguments.\n");
        return ERR;
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
        char groupDirName[GROUPDIR_SIZE];
        sprintf(groupDirName, "GROUPS/%s", tokenList[2]);

        if (!directoryExists(groupDirName))
        { // check if given group exists
            fprintf(stderr, "[-] Invalid group number given.\n");
            return E_GRP;
        }

        if (!groupNamesMatch(tokenList[2], tokenList[3]))
        { // check if given group name matches the stored one
            fprintf(stderr, "[-] Wrong group name given.\n");
            return E_GNAME;
        }

        // Create subscribed user file
        char userSubscribedFile[GROUPUSERSUBFILE_SIZE];
        sprintf(userSubscribedFile, "%s/%s.txt", groupDirName, tokenList[1]);
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
        return ERR;
    }

    if (!(validUID(tokenList[1]) && validGID(tokenList[2])))
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid unsubscribe command arguments given.\n");
        return ERR;
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

char *createUsersInGroupMessage(char **tokenList, int numTokens)
{
    if (numTokens != 2)
    { // server at tejo exits
        fprintf(stderr, "[-] Invalid ULS message received");
        return strdup(ERR_MSG);
    }
    if (!validGID(tokenList[1]))
    { // server at tejo exits
        fprintf(stderr, "[-] Invalid ULS arguments receibed.\n");
        return strdup("RUL NOK\n");
    }
    sortGList((&dsGroups));
    char *users, *tmp;
    int len = 0, curr = 0;
    int groupNum = atoi(tokenList[1]);
    if (groupNum > dsGroups.no_groups)
    { // group doesn't exist - tejo returns NOK
        return strdup("RUL NOK\n");
    }
    char groupDirName[GROUPDIR_SIZE];
    sprintf(groupDirName, "GROUPS/%s", tokenList[1]);
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
        curr += sprintf(users + curr, "RUL OK %s ", tokenList[1]);
        while ((dir = readdir(d)) != NULL)
        {
            if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
            {
                continue;
            }
            if (strlen(dir->d_name) < 9) // UID + 1 + 3
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