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

int listGroups(int numTokens)
{
    if (numTokens != 1)
    { // wrong protocol message received
        fprintf(stderr, "[-] Invalid GLS protocol message received.\n");
        return ERR;
    }
    return dsGroups.no_groups;
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
        sprintf(groupNameBuffer, "%s\n", tokenList[3]);
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
        dsGroups.no_groups++;
        strcpy(dsGroups.groupinfo[n].name, tokenList[3]);
        sprintf(dsGroups.groupinfo[n].no, "%s", *newGID);
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