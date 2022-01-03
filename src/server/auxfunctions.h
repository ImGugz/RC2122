#ifndef AUXFUNCTIONS_H
#define AUXFUNCTIONS_H

#include "udpandtcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void parseArgs(int argc, char * argv[]);
int isNumber(char * num);

#endif