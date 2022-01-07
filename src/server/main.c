#include "auxfunctions.h"
#include "udpandtcp.h"

int main(int argc, char *argv[])
{
    parseArgs(argc, argv);
    setupDSSockets();
}