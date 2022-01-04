#include "udpandtcp.h"

verbose = 0;
int fdDSUDP, fdListenTCP;
ssize_t n;
socklen_t addrlen;
struct addrinfo hintsUDP, hintsTCP, *resUDP, *resTCP;
struct sockaddr_in addr;
int errcode;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

char portDS[PORT_SIZE];

void setupDSSockets()
{
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
    errcode = getaddrinfo(DEFAULT_DSADDR, portDS, &hintsUDP, &resUDP);
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
    errcode = getaddrinfo(DEFAULT_DSADDR, portDS, &hintsTCP, &resTCP);
    if (errcode != 0)
    {
        perror("[-] Failed on TCP address translation");
        closeUDPSocket();
        closeTCPSocket();
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

    waitConnection();
    // To remove: this is just to test at the beggining
    // This will never be run, server closed with ctrl+c in waitConnection()
    closeUDPSocket();
    closeTCPSocket();
}

/**
 * @brief Infinitly waits for client sockets' communications and handles them
 *
 */
void waitConnection()
{
    fd_set current_sockets, ready_sockets;

    FD_ZERO(&current_sockets);
    FD_SET(fdDSUDP, &current_sockets);
    FD_SET(fdListenTCP, &current_sockets);

    while (1)
    {
        // Select is destructive
        ready_sockets = current_sockets;

        // Last NULL is a timeout value, maybe useful
        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
        {
            perror("[-] Failed to establish a connection with a client socket");
            // ToDo: Should we close sockets or keep going?
        }

        // Isto e baseado neste video: https://www.youtube.com/watch?v=Y6pFtgRdUts
        // So que ele so usa TCP entao nao sei como fazer a distincao no else
        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &ready_sockets))
            {
                if (i == fdListenTCP)
                {
                    // This is a new TCP connection
                    int client_socket = acceptNewTCPConnection(fdListenTCP);
                    FD_SET(client_socket, &current_sockets);
                }

                else
                {
                    // if ("Is TCP Connection"){
                    //     handle_TCP_Connection(i)
                    // }
                    // else{
                    //     handle_UDP_Connection(i)
                    // }
                    // // ToDo: Execute the command; How to know if it is a UDP request or TCP request
                    FD_CLR(i, &current_sockets);
                }
            }
        }
    }
}

/**
 * @brief Accepts a new TCP connection after checking for error
 *
 * @return the new socket's file descriptor
 */
int acceptNewTCPConnection()
{
    addrlen = sizeof(addr);
    int client_socket;
    if ((client_socket = accept(fdListenTCP, (struct sockaddr *)&addr, &addrlen)) == -1)
    {
        perror("[-] Unable to establish a new TCP connection");
        // ToDo: How to handle error??
    }

    return client_socket;
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