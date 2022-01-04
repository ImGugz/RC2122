#include "commands.h"

#define UDP_RECV_SIZE 4096

ssize_t n;
socklen_t addrlen;
struct sockaddr_in addr;

char buffer[UDP_RECV_SIZE];

void handleUDPConnection(int client_socket)
{
    addrlen = sizeof(buffer);
    n = recvfrom(client_socket, buffer, UDP_RECV_SIZE, 0, (struct sockaddr *)&addr, &addrlen);
    if (n == -1)
    {
        perror("[-] Error receiving message from UDP socket");
        // How to handle?
    }

    // ToDo: Parse UDP message and execute command
}

void handleTCPConnection(int client_socket)
{
    n = read(client_socket, buffer, UDP_RECV_SIZE);
    if (n == -1)
    {
        perror("[-] Error receiving message from UDP socket");
        // How to handle?
    }

    // ToDo: Parse TCP message and execute command
}