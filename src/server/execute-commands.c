#include "execute-commands.h"

int userRegister(char ** tokenList, int numTokens) {
    if (numTokens != 3) {
        fprintf(stderr, "[-] Incorrect unregister command usage. Please try again.\n");
        return 0;
    }
    char userDirname[12];
    sprintf(userDirname, "USERS/%s", tokenList[1]);
    int ret = mkdir(userDirname, 0700);
    return (ret == -1) ? 0 : 1;
}