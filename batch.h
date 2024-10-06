#ifndef BATCH_H
#define BATCH_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>

void printUsage();
void killThemAll(int signal);

#endif 
