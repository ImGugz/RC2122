#include "udpandtcp.h"

int verbose = 0;
int fdDSUDP, fdListenTCP;
struct addrinfo hintsUDP, hintsTCP, *resUDP, *resTCP;
int errcode;
int maxfd;
fd_set dsSet;

#define MAX(a,b) (((a)>(b))?(a):(b))

char portDS[PORT_SIZE];

void setupDSSockets() {
    // UDP
    fdDSUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (fdDSUDP == -1) {
        perror("[-] Server UDP socket failed to create");
        exit(EXIT_FAILURE);
    }
    memset(&hintsUDP, 0, sizeof(hintsUDP));
    hintsUDP.ai_family = AF_INET;
    hintsUDP.ai_socktype = SOCK_DGRAM;
    errcode = getaddrinfo(DEFAULT_DSADDR, portDS, &hintsUDP, &resUDP);
    if (errcode != 0) {
        perror("[-] Failed on UDP address translation");
        close(fdDSUDP);
        exit(EXIT_FAILURE);
    }

    // TCP
    fdListenTCP = socket(AF_INET, SOCK_STREAM, 0);
    if (fdListenTCP == -1) {
        perror("[-] Server TCP socket failed to create");
        closeUDPSocket();
        exit(EXIT_FAILURE);
    }
    memset(&hintsTCP, 0, sizeof(hintsTCP));
    hintsTCP.ai_family = AF_INET;
    hintsTCP.ai_socktype = SOCK_STREAM;
    errcode = getaddrinfo(DEFAULT_DSADDR, portDS, &hintsTCP, &resTCP);
    if (errcode != 0) {
        perror("[-] Failed on TCP address translation");
        closeUDPSocket();
        close(fdListenTCP);
        exit(EXIT_FAILURE);
    }
    if (listen(fdListenTCP, DEFAULT_LISTENQ) == -1) {
        perror("[-] Failed to prepare TCP socket to accept connections");
        closeUDPSocket();
        closeTCPSocket();
        exit(EXIT_FAILURE);
    }
    maxfd = MAX(fdDSUDP, fdListenTCP) + 1;
    FD_ZERO(&dsSet);
    // To remove: this is just to test at the beggining
    closeUDPSocket();
    closeTCPSocket();
}

/**
 * @brief Closes the UDP's socket file descriptor and free's the memory allocated with getaddrinfo() function.
 * 
 */
void closeUDPSocket() {
    freeaddrinfo(resUDP);
    close(fdDSUDP);
}

/**
 * @brief Closes the TCP's socket file descriptor and free's the memory allocated with getaddrinfo() function.
 * 
 */
void closeTCPSocket() {
    freeaddrinfo(resTCP);
    close(fdListenTCP);
}