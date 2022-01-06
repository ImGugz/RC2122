#include "execute-commands.h"

GROUPLIST dsGroups;

int userRegister(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    {
        fprintf(stderr, "[-] Incorrect user register command usage. Please try again.\n");
        return NOK;
    }
    char userDirname[12];
    char passFileName[28];
    FILE *fPtr;

    // Create user directory
    sprintf(userDirname, "USERS/%s", tokenList[1]);
    int ret = mkdir(userDirname, 0700);
    if (ret == -1)
    {
        if (errno == EEXIST)
            return DUP;
        else
            return NOK;
    }
    // Create password file
    sprintf(passFileName, "%s/%s_pass.txt", userDirname, tokenList[1]);
    fPtr = fopen(passFileName, "w");
    if (fPtr == NULL)
    {
        fprintf(stderr, "[-] Unable to create file.\n");
        return NOK;
    }
    fputs(tokenList[2], fPtr);
    fclose(fPtr);

    return OK;
}

int userUnregister(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    {
        fprintf(stderr, "[-] Incorrect user unregister command usage. Please try again.\n");
        return NOK;
    }
    char userDirname[12];

    sprintf(userDirname, "USERS/%s", tokenList[1]);

    // Check if user is registered
    if (isDirectoryExists(userDirname) == 0)
    {
        // fprintf(stderr, "[-] User is not previously registered. Please try again.\n");
        return NOK;
    }

    // Check if it is the right password
    if (isCorrectPassword(userDirname, tokenList[1], tokenList[2]) == 0)
    {
        // fprintf(stderr, "[-] Wrong password.\n");
        return NOK;
    };

    // Apagar a diretoria do utilizador
    removeDirectory(userDirname);

    return OK;
}

int userLogin(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    {
        fprintf(stderr, "[-] Incorrect user login command usage. Please try again.\n");
        return NOK;
    }
    char userDirname[12];

    sprintf(userDirname, "USERS/%s", tokenList[1]);

    // Check if user is registered
    if (isDirectoryExists(userDirname) == 0)
    {
        // fprintf(stderr, "[-] User is not registered. Please try again.\n");
        return NOK;
    }

    // Check if it is the right password
    if (isCorrectPassword(userDirname, tokenList[1], tokenList[2]) == 0)
    {
        // fprintf(stderr, "[-] Wrong password.\n");
        return NOK;
    }

    char loginFileName[29];
    sprintf(loginFileName, "%s/%s_login.txt", userDirname, tokenList[1]);

    FILE *fp = fopen(loginFileName, "a");
    if (fp == NULL)
    {
        fprintf(stderr, "[-] Unable to create user login file.\n");
        return NOK;
    }

    fclose(fp);

    return OK;
}

int userLogout(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    {
        fprintf(stderr, "[-] Incorrect user login command usage. Please try again.\n");
        return NOK;
    }

    // Check if user is registered
    char userDirname[12];
    sprintf(userDirname, "USERS/%s", tokenList[1]);
    if (isDirectoryExists(userDirname) == 0)
    {
        // fprintf(stderr, "[-] User is not registered. Please try again.\n");
        return NOK;
    }

    // Check if it is the right password
    if (isCorrectPassword(userDirname, tokenList[1], tokenList[2]) == 0)
    {
        // fprintf(stderr, "[-] Wrong password.\n");
        return NOK;
    }

    // Check if user is logged in
    FILE *fp;
    char loginFileName[29];
    sprintf(loginFileName, "%s/%s_login.txt", userDirname, tokenList[1]);
    if (fp = fopen(loginFileName, "r"))
    {
        if (unlink(loginFileName) == 0)
            return OK;
        fprintf(stderr, "[-] Unable to delete login file.\n");
    }

    return NOK;
}

int listGroups(int numTokens)
{
    if (numTokens != 1)
    {
        fprintf(stderr, "[-] Invalid GLS protocol message received.\n");
        return NOK;
    }
    return listGroupsDir(&dsGroups);
}

int userSubscribe(char **tokenList, int numTokens, char *newGID)
{
    if (numTokens != 4)
    {
        fprintf(stderr, "[-] Incorrect user subscribe command usage. Please try again.\n");
        return NOK;
    }

    // Check if user is registered
    char userDirname[12];
    sprintf(userDirname, "USERS/%s", tokenList[1]);

    if (isDirectoryExists(userDirname) == 0)
    {
        // fprintf(stderr, "[-] User is not previously registered. Please try again.\n");
        return E_USR;
    }

    // Check if user is logged in
    FILE *fPtr;
    char loginFileName[29];
    sprintf(loginFileName, "%s/%s_login.txt", userDirname, tokenList[1]);

    if (!(fPtr = fopen(loginFileName, "r")))
        return E_USR;
    fclose(fPtr);

    // Create a group
    if (strcmp(tokenList[2], "00") == 0)
    {
        // Check if there is less than 99 groups created
        if (dsGroups.no_groups >= 99)
            return E_FULL;

        // Check if group name is valid
        if (!(validGName(tokenList[3])))
            return ERR;

        // Create group
        // Add groups to struct
        dsGroups.no_groups++;
        if (dsGroups.no_groups < 10)
            sprintf(newGID, "0%d", dsGroups.no_groups);
        else
            sprintf(newGID, "%d", dsGroups.no_groups);

        strcpy(dsGroups.groupinfo[dsGroups.no_groups].name, tokenList[3]);
        sprintf(dsGroups.groupinfo[dsGroups.no_groups].no, "%d", dsGroups.no_groups);

        // Add group to directory
        char groupDirName[10];
        char msgDirName[14];
        char groupNameFile[32];
        char userSubscribedFile[20];

        // Create group directory

        sprintf(groupDirName, "GROUPS/%s", newGID);
        int ret = mkdir(groupDirName, 0700);
        if (ret == -1)
        {
            printf("Same Mistake\n");
            return NOK;
        }

        // Create messages directory
        sprintf(msgDirName, "%s/MSG", groupDirName);
        ret = mkdir(msgDirName, 0700);
        if (ret == -1)
            return NOK;

        // Create name file
        FILE *fPtr;
        sprintf(groupNameFile, "%s/%s_name.txt", groupDirName, newGID);
        fPtr = fopen(groupNameFile, "w");
        if (fPtr == NULL)
        {
            fprintf(stderr, "[-] Unable to create group name file.\n");
            return NOK;
        }
        fputs(tokenList[3], fPtr);
        fclose(fPtr);

        // Create subscribed user file
        sprintf(userSubscribedFile, "%s/%s.txt", groupDirName, tokenList[1]);
        fPtr = fopen(userSubscribedFile, "a");
        if (fPtr == NULL)
        {
            fprintf(stderr, "[-] Unable to create subscribed user file.\n");
            return NOK;
        }
        fclose(fPtr);

        printf("O numero que vou retornar e: %s\n", newGID);
        return NEW;
    }

    // If group is already created
    else
    {
        char groupDirName[13];
        sprintf(groupDirName, "GROUPS/%s", tokenList[2]);
        printf("Vou procurar a seguinte diretoria: %s\n", groupDirName);

        // Check if group exists
        if (isDirectoryExists(groupDirName) == 0)
        {
            // fprintf(stderr, "[-] Invalid group number. Please try again.\n");
            return E_GRP;
        }

        // Check if Gname is correct
        if (isCorrectGroupName(groupDirName, tokenList[2], tokenList[3]) == 0)
        {
            // fprintf(stderr, "[-] Wrong group name.\n");
            return E_GNAME;
        }

        // Create subscribed user file
        char userSubscribedFile[20];
        sprintf(userSubscribedFile, "%s/%s.txt", groupDirName, tokenList[1]);
        fPtr = fopen(userSubscribedFile, "a");
        if (fPtr == NULL)
        {
            fprintf(stderr, "[-] Unable to create subscribed user file.\n");
            return NOK;
        }
        fclose(fPtr);

        return OK;
    }
}

int userUnsubscribe(char ** tokenList, int numTokens) {

    if (numTokens != 3) {
        fprintf(stderr, "[-] Incorrect user subscribe command usage. Please try again.\n");
        return NOK;
    }

    // Check if user is registered
    char userDirname[12];
    sprintf(userDirname, "USERS/%s", tokenList[1]);

    if (isDirectoryExists(userDirname) == 0) {
        // fprintf(stderr, "[-] User is not previously registered. Please try again.\n");
        return E_USR;
    } else {
        char groupDirName[13];
        sprintf(groupDirName, "GROUPS/%s", tokenList[2]);
        printf("Vou procurar a seguinte diretoria: %s\n", groupDirName);

        // Check if group exists
        if (isDirectoryExists(groupDirName) == 0) {
            // fprintf(stderr, "[-] Invalid group number. Please try again.\n");
            return E_GRP;
        } else { //UID and GID valid
            char filename[20];
            sprintf(filename, "GROUPS/%s/%s.txt", tokenList[2], tokenList[1]);
            if(unlink(filename) != 0 && errno != ENOENT) {
               fprintf(stderr, "Error deleting file.\n");
               exit(EXIT_FAILURE); //TODO exit???
            } else {
                return OK;
            }
            
        }

    }


}