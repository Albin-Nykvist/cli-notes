
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../include/util.h"


int stringToInteger(const char * string) {
    char *endptr;
    int integer = strtol(string, &endptr, 10); // (base 10)

    if(*endptr != '\0' && *endptr != '\n') {
        return -1;
    }
    return integer;
}

int extractFirstInteger(const char * string) {
    while (*string && !isdigit(*string)) {
        string++;
    }

    if (*string == '\0') {
        return -1;
    }

    char *endptr;
    int integer = strtol(string, &endptr, 10);

    return integer; 
}
