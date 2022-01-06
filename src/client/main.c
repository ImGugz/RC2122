#include "auxfunctions.h"
#include "commands.h"

int main(int argc, char *argv[])
{
    parseArgs(argc, argv);
    processCommands();
}