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

    FILE *fp;
    char loginFileName[29];
    sprintf(loginFileName, "%s/%s_login.txt", userDirname, tokenList[1]);

    // Check if user is logged in
    if (fp = fopen(loginFileName, "r"))
    {
        if (remove(loginFileName) == 0)
            return OK;
        fprintf(stderr, "[-] Unable to delete login file.\n");
    }

    return NOK;
}

int listGroups(int numTokens) {
    if (numTokens != 1) {
        fprintf(stderr, "[-] Invalid GLS protocol message received.\n");
        return NOK;
    }
    return listGroupsDir(&dsGroups);
}