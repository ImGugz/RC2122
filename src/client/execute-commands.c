/**
 * @file execute-commands.c
 * @author Group 18
 * @brief Execute all USER-DS commands.
 *
 */

#include "execute-commands.h"

#define USERID_SIZE 6    // len(UID) + 1
#define USERPWD_SIZE 9   // len(UPW) + 1
#define GID_SIZE 6       // len(GID) + 1
#define MSGTXT_SIZE 241  // maxlen(MSGTXT) + 1
#define MAXFN_SIZE 25    // maxlen(MSGFILE) + 1
#define SERVMSG_SIZE 512 // 2 ^ (math.top(log_2(maxlen(MSG2SRV)))) = 2 ^ 9 (maxlen(MSG2SRV) comes from POST)

// Logged in user's details
char activeUser[USERID_SIZE] = "";
char activeUserPwd[USERPWD_SIZE] = "";
// Selected group
char activeGID[GID_SIZE] = "";

char serverMessage[SERVMSG_SIZE] = ""; // Message that'll be sent to DS following the statement's protocol
int userSession = LOGGED_OUT;          // Keeps user's account activity log (LOGGED IN OR LOGGED OUT)

/**
 * @brief User register command - creates a new account on DS.
 *
 * @param tokenList List containing all command arguments (including the command itself)
 * @param numTokens Number of command arguments
 */
void userRegister(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    {
        fprintf(stderr, "[-] Incorrect register command usage. Please try again.\n");
        return;
    }
    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    {
        fprintf(stderr, "[-] Invalid register command arguments. Please check given UID and/or password and try again.\n");
        return;
    }
    sprintf(serverMessage, "REG %s %s\n", tokenList[1], tokenList[2]);
    exchangeUDPMsg(serverMessage);
}

/**
 * @brief User unregister command - removes an account from DS.
 *
 * @param tokenList List containing all command arguments (including the command itself)
 * @param numTokens Number of command arguments
 */
void userUnregister(char **tokenList, int numTokens)
{
    if (numTokens != 3)
    {
        fprintf(stderr, "[-] Incorrect unregister command usage. Please try again.\n");
        return;
    }
    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    {
        fprintf(stderr, "[-] Invalid unregister command arguments. Please check given UID and/or password and try again.\n");
        return;
    }
    sprintf(serverMessage, "UNR %s %s\n", tokenList[1], tokenList[2]);
    exchangeUDPMsg(serverMessage);
}

/**
 * @brief User login command - logs a user into a DS account.
 *
 * @param tokenList List containing all command arguments (including the command itself)
 * @param numTokens Number of command arguments
 */
void userLogin(char **tokenList, int numTokens)
{
    if (userSession == LOGGED_IN)
    {
        fprintf(stderr, "[-] You're already logged in. Please logout before you try to log in again.\n");
        return;
    }
    if (numTokens != 3)
    {
        fprintf(stderr, "[-] Incorrect login command usage. Please try again.\n");
        return;
    }
    if (!(validUID(tokenList[1]) && validPW(tokenList[2])))
    {
        fprintf(stderr, "[-] Invalid login command arguments. Please check given UID and/or password and try again.\n");
        return;
    }
    sprintf(serverMessage, "LOG %s %s\n", tokenList[1], tokenList[2]);
    exchangeUDPMsg(serverMessage);
    if (userSession == LOGGED_IN)
    { // User session is an extern variable that is modified if LOG protocol response is OK
        strcpy(activeUser, tokenList[1]);
        strcpy(activeUserPwd, tokenList[2]);
    }
}

/**
 * @brief User logout command - forgets the credentials of the previously logged in user.
 *
 * @param tokenList List containing all command arguments (including the command itself)
 * @param numTokens Number of command arguments
 */
void userLogout(char **tokenList, int numTokens)
{
    if (userSession == LOGGED_OUT)
    {
        fprintf(stderr, "[-] You're not logged in into any account.\n");
        return;
    }
    if (numTokens != 1)
    {
        fprintf(stderr, "[-] Incorrect logout command usage. Please try again.\n");
        return;
    }
    sprintf(serverMessage, "OUT %s %s\n", activeUser, activeUserPwd);
    exchangeUDPMsg(serverMessage);
    if (userSession == LOGGED_OUT)
    { // User session is an extern variable that is modified if OUT protocol response is OK
        memset(activeUser, 0, sizeof(activeUser));
        memset(activeUserPwd, 0, sizeof(activeUserPwd));
    }
}

/**
 * @brief User showuid command - displays the UID of the user that is logged in.
 *
 * @param numTokens Number of command arguments
 */
void showActiveUser(int numTokens)
{
    if (userSession == LOGGED_OUT)
    {
        fprintf(stderr, "[-] You're not logged in into any account.\n");
        return;
    }
    if (numTokens != 1)
    {
        fprintf(stderr, "[-] Incorrect showuid command usage. Please try again.\n");
        return;
    }
    // Safe to assume that if userSession == LOGGED_IN then activeUser is a valid string containing a valid UID
    printf("[+] You're logged in with user ID %s.\n", activeUser);
}

/**
 * @brief User exit command - terminates user application.
 *
 * @param numTokens Number of command arguments
 */
void userExit(int numTokens)
{
    if (numTokens != 1)
    {
        fprintf(stderr, "[-] Incorrect exit command usage. Please try again.\n");
        return;
    }
    closeUDPSocket();
    printf("[+] Exiting...\n");
    exit(EXIT_SUCCESS);
}

/**
 * @brief User groups command - shows all available groups on DS.
 *
 * @param numTokens Number of command arguments
 */
void showGroups(int numTokens)
{
    if (numTokens != 1)
    {
        fprintf(stderr, "[-] Incorrect groups list command usage. Please try again.\n");
        return;
    }
    sprintf(serverMessage, "GLS\n");
    exchangeUDPMsg(serverMessage);
}

/**
 * @brief User group subscribe command - subscribes the logged in user to a specific group.
 *
 * @param tokenList List containing all command arguments (including the command itself)
 * @param numTokens Number of command arguments
 */
void userGroupSubscribe(char **tokenList, int numTokens)
{
    if (userSession == LOGGED_OUT)
    {
        fprintf(stderr, "[-] Please login before you subscribe to a group.\n");
        return;
    }
    if (numTokens != 3)
    {
        fprintf(stderr, "[-] Incorrect subscribe command usage. Please try again.\n");
        return;
    }
    if (!strcmp(tokenList[1], "0"))
    { // Create a new group
        if (!validGName(tokenList[2]))
        {
            fprintf(stderr, "[-] Invalid new group name. Please try again.\n");
            return;
        }
        sprintf(serverMessage, "GSR %s 00 %s\n", activeUser, tokenList[2]);
    }
    else if (validGID(tokenList[1]) && validGName(tokenList[2]))
    { // Subscribe to a group
        sprintf(serverMessage, "GSR %s %s %s\n", activeUser, tokenList[1], tokenList[2]);
    }
    else
    {
        fprintf(stderr, "[-] Incorrect subscribe command usage. Please check if the given arguments follow the User-DS messaging protocol and try again.\n");
        return;
    }
    exchangeUDPMsg(serverMessage);
}

/**
 * @brief User group unsubscribe command - unsubscribes the logged in user from a specific group.
 *
 * @param tokenList List containing all command arguments (including the command itself)
 * @param numTokens Number of command arguments
 */
void userGroupUnsubscribe(char **tokenList, int numTokens)
{
    if (userSession == LOGGED_OUT)
    {
        fprintf(stderr, "[-] Please login before you unsubscribe to a group.\n");
        return;
    }
    if (numTokens != 2)
    {
        fprintf(stderr, "[-] Incorrect unsubscribe command usage. Please try again.\n");
        return;
    }
    if (!validGID(tokenList[1]))
    {
        fprintf(stderr, "[-] Invalid given group ID to unsubscribe. Please try again.\n");
        return;
    }
    sprintf(serverMessage, "GUR %s %s\n", activeUser, tokenList[1]);
    exchangeUDPMsg(serverMessage);
}

/**
 * @brief User my_groups command - displays logged in user's subscribed groups.
 *
 * @param numTokens Number of command arguments
 */
void userGroupsList(int numTokens)
{
    if (userSession == LOGGED_OUT)
    {
        fprintf(stderr, "[-] Please login before you request for your subscribed groups list.\n");
        return;
    }
    if (numTokens != 1)
    {
        fprintf(stderr, "[-] Incorrect user groups' list command usage. Please try again.\n");
        return;
    }
    sprintf(serverMessage, "GLM %s\n", activeUser);
    exchangeUDPMsg(serverMessage);
}

/**
 * @brief User select command - selects a group to sg/ulist/post/retrieve commands.
 *
 * @param tokenList List containing all command arguments (including the command itself)
 * @param numTokens Number of command arguments
 */
void userSelectGroup(char **tokenList, int numTokens)
{
    if (userSession == LOGGED_OUT)
    {
        fprintf(stderr, "[-] Please login before you select a group.\n");
        return;
    }
    if (numTokens != 2)
    {
        fprintf(stderr, "[-] Incorrect select command usage. Please try again.\n");
        return;
    }
    if (!validGID(tokenList[1]))
    {
        fprintf(stderr, "[-] Invalid given group ID to select. Please try again.\n");
        return;
    }
    memset(activeGID, 0, sizeof(activeGID));
    strcpy(activeGID, tokenList[1]);
    printf("[+] You have successfully selected group %s.\n[!] Make sure you've chosen a GID that actually exists in the DS's database. For further details use command 'gl'.\n", activeGID);
}

/**
 * @brief User showgid command - displays the GID of the group that is selected.
 *
 * @param numTokens Number of command arguments
 */
void showSelectedGroup(int numTokens)
{
    if (numTokens != 1)
    {
        fprintf(stderr, "[-] Incorrect show select group command usage. Please try again.\n");
        return;
    }
    if (strlen(activeGID) == 0)
    {
        printf("[-] You haven't selected any group yet.\n");
    }
    else
    { // We know that either it's "" or something with bigger length
        printf("[+] Group %s is selected.\n", activeGID);
    }
}

/**
 * @brief User ulist command - displays all the users that are subscribed to the selected group.
 *
 * @param numTokens Number of command arguments
 */
void showUsersGroup(int numTokens)
{
    if (userSession == LOGGED_OUT)
    {
        fprintf(stderr, "[-] Please login before you request the list of users that are subscribed to your selected group.\n");
        return;
    }
    if (strlen(activeGID) == 0)
    {
        fprintf(stderr, "[-] Please select a group before you request the list of users that are subscribed to it.\n");
        return;
    }
    if (numTokens != 1)
    {
        fprintf(stderr, "[-] Incorrect list group's users command usage. Please try again.\n");
        return;
    }
    // No need to check if activeGroup's syntax is okay because that was done before actually selecting a group
    sprintf(serverMessage, "ULS %s\n", activeGID);
    exchangeTCPMsg(serverMessage);
}

/**
 * @brief User group post command - Sends a message containing text and possibly a file to the selected group.
 *
 * @param command Entire command given by the user (not separated by tokens like before)
 */
void userPostGroup(char *command)
{
    if (userSession == LOGGED_OUT)
    {
        fprintf(stderr, "[-] Please login before you post to a group.\n");
        return;
    }
    if (strlen(activeGID) == 0)
    {
        fprintf(stderr, "[-] Please select a group before you post on it.\n");
        return;
    }
    char messageText[MSGTXT_SIZE];
    char fileName[MAXFN_SIZE] = "";
    sscanf(command, "post \"%240[^\"]\" %s", messageText, fileName);
    if (strlen(fileName) > 0)
    { // fileName was 'filled up' with something
        if (!validFilename(fileName))
        {
            fprintf(stderr, "[-] The file you submit can't exceed 24 characters and must have a 3 letter file extension. Please try again.\n");
            return;
        }
        FILE *post = fopen(fileName, "rb");
        if (post == NULL)
        {
            fprintf(stderr, "[-] The file you submit must be in your working directory. Please check the given file name and try again.\n");
            return;
        }
        if (fseek(post, 0, SEEK_END) == -1)
        {
            perror("[-] Post file seek failed");
            closeUDPSocket();
            exit(EXIT_FAILURE); // Failed syscall
        }
        long lenFile = ftell(post); // long because it can have at most 10 digits and int goes to 2^31 - 1 which is 214--.7 (len 10) - it can be 999 999 999 9 bytes
        if (lenFile == -1)
        {
            perror("[-] Post file tell failed");
            closeUDPSocket();
            exit(EXIT_FAILURE); // Failed syscall
        }
        if (fseek(post, 0, SEEK_SET) == -1)
        {
            perror("[-] Post file seek failed");
            closeUDPSocket();
            exit(EXIT_FAILURE); // Failed syscall
        }
        sprintf(serverMessage, "PST %s %s %ld %s %s %ld ", activeUser, activeGID, strlen(messageText), messageText, fileName, lenFile);
        exchangeTCPPost(serverMessage, post, lenFile);
        fclose(post);
    }
    else
    {
        sprintf(serverMessage, "PST %s %s %ld %s\n", activeUser, activeGID, strlen(messageText), messageText);
        exchangeTCPMsg(serverMessage);
    }
}

/**
 * @brief User group retrieve command - retrieves at most 20 messages from the starting given message.
 *
 * @param tokenList List containing all command arguments (including the command itself)
 * @param numTokens Number of command arguments
 */
void userRetrieveMsgs(char **tokenList, int numTokens)
{
    if (userSession == LOGGED_OUT)
    {
        fprintf(stderr, "[-] Please login before you retrieve messages from a group.\n");
        return;
    }
    if (strlen(activeGID) == 0)
    {
        fprintf(stderr, "[-] Please select a group before you retrieve messages from it.\n");
        return;
    }
    if (numTokens != 2)
    {
        fprintf(stderr, "[-] Incorrect retrieve command usage. Please try again.\n");
        return;
    }
    if (!validMID(tokenList[1]))
    {
        fprintf(stderr, "[-] Invalid starting message to retrieve. Please try again.\n");
        return;
    }
    sprintf(serverMessage, "RTV %s %s %s\n", activeUser, activeGID, tokenList[1]);
    exchangeTCPRet(serverMessage);
}

void userDebug(char *command)
{
    char debugCmd[4096], tmp[4096];
    int numTokens = 0;
    char *tokenList[2048];
    sscanf(command, "debug \"%[^\"]\"", debugCmd);
    size_t lenDebug = strlen(debugCmd);
    debugCmd[lenDebug] = '\n';
    debugCmd[lenDebug + 1] = '\0';
    strcpy(tmp, debugCmd);
    char *token = strtok(tmp, " \n");
    while (token != NULL)
    {
        tokenList[numTokens] = token;
        numTokens++;
        token = strtok(NULL, " \n");
    }
    if (!strcmp(tokenList[0], "REG") || !strcmp(tokenList[0], "UNR") || !strcmp(tokenList[0], "LOG") || !strcmp(tokenList[0], "OUT") ||
        !strcmp(tokenList[0], "GLS") || !strcmp(tokenList[0], "GSR") || !strcmp(tokenList[0], "GUR") || !strcmp(tokenList[0], "GLM"))
    {
        exchangeUDPMsg(debugCmd);
    }
    else if (!strcmp(tokenList[0], "ULS") || !strcmp(tokenList[0], "PST") || !strcmp(tokenList[0], "RTV"))
    {
        exchangeTCPMsg(debugCmd);
    }
    else
    {
        printf("[-] Mete uma mensagem de protocolo a sério caralho.\n");
    }
}