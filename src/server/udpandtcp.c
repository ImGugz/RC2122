#include "udpandtcp.h"

#define HOSTNAME_SIZE 1023

int verbose = 0;
int fdDSUDP, fdListenTCP;
struct addrinfo hintsUDP, hintsTCP, *resUDP, *resTCP;
int errcode, n;

char portDS[PORT_SIZE] = DEFAULT_DSPORT;

void setupDSSockets()
{
    char hostname[HOSTNAME_SIZE + 1];
    // UDP
    fdDSUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (fdDSUDP == -1)
    {
        perror("[-] Server UDP socket failed to create");
        exit(EXIT_FAILURE);
    }
    memset(&hintsUDP, 0, sizeof(hintsUDP));
    hintsUDP.ai_family = AF_INET;
    hintsUDP.ai_socktype = SOCK_DGRAM;
    hintsUDP.ai_flags = AI_PASSIVE;
    errcode = getaddrinfo(NULL, portDS, &hintsUDP, &resUDP);
    if (errcode != 0)
    {
        perror("[-] Failed on UDP address translation");
        close(fdDSUDP);
        exit(EXIT_FAILURE);
    }
    n = bind(fdDSUDP, resUDP->ai_addr, resUDP->ai_addrlen);
    if (n == -1)
    {
        perror("[-] Failed to bind UDP server");
        close(fdDSUDP);
        exit(EXIT_FAILURE);
    }

    // TCP
    fdListenTCP = socket(AF_INET, SOCK_STREAM, 0);
    if (fdListenTCP == -1)
    {
        perror("[-] Server TCP socket failed to create");
        closeUDPSocket();
        exit(EXIT_FAILURE);
    }
    memset(&hintsTCP, 0, sizeof(hintsTCP));
    hintsTCP.ai_family = AF_INET;
    hintsTCP.ai_socktype = SOCK_STREAM;
    hintsTCP.ai_flags = AI_PASSIVE;
    errcode = getaddrinfo(NULL, portDS, &hintsTCP, &resTCP);
    if (errcode != 0)
    {
        perror("[-] Failed on TCP address translation");
        closeUDPSocket();
        close(fdListenTCP);
        exit(EXIT_FAILURE);
    }
    n = bind(fdListenTCP, resTCP->ai_addr, resTCP->ai_addrlen);
    if (n == 1)
    {
        perror("[-] Server TCP socket failed to bind");
        closeUDPSocket();
        closeTCPSocket();
        exit(EXIT_FAILURE);
    }
    if (listen(fdListenTCP, DEFAULT_LISTENQ) == -1)
    {
        perror("[-] Failed to prepare TCP socket to accept connections");
        closeUDPSocket();
        closeTCPSocket();
        exit(EXIT_FAILURE);
    }

    gethostname(hostname, HOSTNAME_SIZE);
    printf("[+] DS server started at %s.\n[!] Currently listening in port %s for UDP and TCP connections...\n\n", hostname, portDS);
    fillGroupsInfo(); // fill global struct with already existing groups
    pid_t pid = fork();
    if (pid == 0)
    { // Child process will handle everything related to UDP
        handleUDP(fdDSUDP);
    }
    else if (pid > 0)
    { // Parent process will handle everything related to TCP
        handleTCP(fdListenTCP);
    }
    else
    {
        perror("[-] Failed on fork");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Closes the UDP's socket file descriptor and free's the memory allocated with getaddrinfo() function.
 *
 */
void closeUDPSocket()
{
    freeaddrinfo(resUDP);
    close(fdDSUDP);
}

/**
 * @brief Closes the TCP's socket file descriptor and free's the memory allocated with getaddrinfo() function.
 *
 */
void closeTCPSocket()
{
    freeaddrinfo(resTCP);
    close(fdListenTCP);
}