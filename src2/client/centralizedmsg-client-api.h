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
 * @brief Creates a new account on DS exchaning messages via UDP protocol.
 *
 * @param tokenList list that contains all the command's arguments (including the command itself).
 * @param numTokens number of command arguments.
 */
void clientRegister(char **tokenList, int numTokens);

/**
 * @brief Closes the socket created to exchange messages between the client and the DS.
 *
 */
void closeDSUDPSocket();

#endif