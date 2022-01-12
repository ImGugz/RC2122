#ifndef CLIENTAPI_H
#define CLIENTAPI_H

#include "../centralizedmsg-api.h"
#include "../centralizedmsg-api-constants.h"

/**
 * @brief Creates socket that enables client-server communication via UDP protocol.
 *
 * @param addrDS string that contains the server's address.
 * @param portDS string that contains the server's port number.
 */
void createDSUDPSocket(char *addrDS, char *portDS);

/**
 * @brief Exchanges messages between the client and the DS via UDP protocol.
 *
 * @param message string that contains the message that client wants to send to the DS.
 */
void exchangeDSUDPMsg(char *message);

/**
 * @brief Processes the DS's reply to what the user sent it via UDP protocol.
 *
 * @param message string that contains the message that the DS sent to the client.
 */
void processDSUDPReply(char *message);

/**
 * @brief Creates a new account on the DS exchanging messages via UDP protocol.
 *
 * @param tokenList list that contains all the command's arguments (including the command itself).
 * @param numTokens number of command arguments.
 */
void clientRegister(char **tokenList, int numTokens);

/**
 * @brief Deletes an existing account on the DS exchanging messages via UDP protocol.
 *
 * @param tokenList list that contains all the command's arguments (including the command itself).
 * @param numTokens number of command arguments.
 */
void clientUnregister(char **tokenList, int numTokens);

/**
 * @brief Logs a user in to an existing DS account exchanging messages via UDP protocol.
 *
 * @param tokenList list that contains all the command's arguments (including the command itself).
 * @param numTokens number of command arguments.
 */
void clientLogin(char **tokenList, int numTokens);

/**
 * @brief Logs a user out from an existing DS account exchanging messages via UDP protocol.
 *
 * @param tokenList list that contains all the command's arguments (including the command itself).
 * @param numTokens number of command arguments.
 */
void clientLogout(char **tokenList, int numTokens);

/**
 * @brief Locally displays the current logged in user ID.
 *
 * @param numTokens number of command arguments.
 */
void showCurrentClient(int numTokens);

/**
 * @brief Terminates the client program gracefully.
 *
 * @param numTokens number of command arguments.
 */
void clientExit(int numTokens);

/**
 * @brief Displays all the existing groups in the DS.
 *
 * @param numTokens number of command arguments.
 */
void showDSGroups(int numTokens);

/**
 * @brief Enables the current client to either create a new group in the DS or to subscribe to one.
 *
 * @param tokenList list that contains all the command's arguments (including the command itself).
 * @param numTokens number of command arguments.
 */
void clientSubscribeGroup(char **tokenList, int numTokens);

/**
 * @brief Unsubscribes the current client to the given group ID.
 *
 * @param tokenList list that contains all the command's arguments (including the command itself).
 * @param numTokens number of command arguments.
 */
void clientUnsubscribeGroup(char **tokenList, int numTokens);

/**
 * @brief Closes the socket created to exchange messages between the client and the DS.
 *
 */
void closeDSUDPSocket();

#endif