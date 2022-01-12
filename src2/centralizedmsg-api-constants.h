#ifndef CENTRALIZEDMESSAGING_API_CONSTANTS_H
#define CENTRALIZEDMESSAGING_API_CONSTANTS_H

/* By default DS server is set to be listening on localhost */
#define DS_DEFAULT_ADDR "127.0.0.1"

/* By default DS server is set to be listening on port 58000 + GN */
#define DS_DEFAULT_PORT "58018"

/* Hostnames (including the dots) can be at most 253 characters long */
#define DS_ADDR_SIZE 254

/* Ports range from 0 to 65535 */
#define DS_PORT_SIZE 6

/* The maximum buffer size to read from stdin from client side (POST command has the most characters) */
#define CLIENT_COMMAND_SIZE 274

/* The maximum number of words written on command in stdin by the client (also from POST command) */
#define CLIENT_NUMTOKENS 256

/* Macros used to parse client command and use switch cases instead of strcmp */
#define INVALID_COMMAND -1
#define REGISTER 1
#define UNREGISTER 2
#define LOGIN 3
#define LOGOUT 4
#define SHOWUID 5
#define EXIT 6
#define GROUPS 7
#define SUBSCRIBE 8
#define UNSUBSCRIBE 9
#define MY_GROUPS 10
#define SELECT 11
#define SHOWGID 12
#define ULIST 13
#define POST 14
#define RETRIEVE 15

/* The buffer size for a message from the client to the DS via UDP protocol */
#define CLIENT_TO_DS_UDP_SIZE 295

/* The buffer size for a message from the DS to the client via UDP protocol */
#define DS_TO_CLIENT_UDP_SIZE 4096

/* The buffer size for a protocol message code */
#define PROTOCOL_CODE_SIZE 4

/* The buffer size for a protocol message status via UDP protocol */
#define PROTOCOL_STATUS_UDP_SIZE 8

/* Macros used to keep current client session state */
#define LOGGED_OUT 0
#define LOGGED_IN 1

/* The size of a client's UID according to the statement's rules */
#define CLIENT_UID_SIZE 6

/* The size of a client's password according to the statement's rules */
#define CLIENT_PWD_SIZE 9

/* The size of a buffer containing all relevant information about a DS group */
#define DS_GROUPINFO_SIZE 34

/* The size of a DS's group ID according to the statement's rules */
#define DS_GID_SIZE 3

/* The maximum size of DS group name */
#define DS_GNAME_SIZE 25

/* The size of a DS group message ID according to the statement's rules */
#define DS_MID_SIZE 5

/* The size of a ulist command buffer from the client to the server */
#define CLIENTDS_ULISTBUF_SIZE 8

/* Arbitrary size to read from TCP in ULIST each time readTCP is called */
#define DSCLIENT_ULISTREAD_SIZE 512

/* The buffer size for a protocol message status via UDP protocol */
#define PROTOCOL_STATUS_TCP_SIZE 5

#endif