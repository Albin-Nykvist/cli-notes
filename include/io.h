
#ifndef IO_H
#define IO_H

#include <stdbool.h>

#define MAX_INPUT_LENGTH 100

void getInput(char* buffer, int size);
bool hasFlag(char* flag, char ** argv, int argc);

#endif