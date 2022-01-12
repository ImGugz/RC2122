#ifndef API_H
#define API_H

#include "centralizedmsg-api-constants.h"

/**
 * @brief Checks if a given buffer specifies a given pattern.
 *
 * @param buffer buffer to be checked.
 * @param pattern pattern that buffer is being checked on.
 * @return 1 if buffer specifies the given pattern, 0 otherwise.
 */
int validRegex(char *buffer, char *pattern);

/**
 * @brief Checks if a given address/hostname is valid.
 *
 * @param address string containing the address.
 * @return 1 if it's valid, 0 otherwise.
 */
int validAddress(char *address);

/**
 * @brief Checks if a given port is valid.
 *
 * @param portStr string containing the port.
 * @return 1 if it's valid, 0 otherwise.
 */
int validPort(char *port);

/**
 * @brief "Translates" a given command into a pre-defined macro.
 *
 * @param command string containing the command.
 * @return respective MACRO assigned to given command.
 */
int parseClientCommand(char *command);

/**
 * @brief Checks if a given user ID is valid according to the statement's rules.
 *
 * @param UID string that contains the used ID.
 * @return 1 if it's valid, 0 otherwise.
 */
int validUID(char *UID);

/**
 * @brief Checks if a given user password is valid according to the statement's rules.
 *
 * @param PW string that contains the password.
 * @return 1 if it's valid, 0 otherwise.
 */
int validPW(char *PW);

/**
 * @brief Checks if a given string is a decimal positive number.
 *
 * @param num string that contains the number (or not).
 * @return 1 if it's a decimal positive number, 0 otherwise.
 */
int isNumber(char *number);

/**
 * @brief Checks if a given group ID is valid according to the statement's rules.
 *
 * @param GID string that contains the group ID.
 * @return 1 if it's valid, 0 otherwise.
 */
int validGID(char *GID);

/**
 * @brief Checks if a given group name is valid according to the statement's rules.
 *
 * @param gName string that contains the group name.
 * @return 1 if it's valid, 0 otherwise.
 */
int validGName(char *GName);

/**
 * @brief Checks if a given group message ID is valid according to the statement's rules.
 *
 * @param MID string that contains the group message ID.
 * @return 1 if 0000 <= MID <= 9999, 0 otherwise.
 */
int isMID(char *MID);

#endif