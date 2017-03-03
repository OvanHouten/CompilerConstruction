/*
 * numbers.c
 *
 *  Created on: 2 Mar 2017
 *      Author: nico
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>

#include"numbers.h"

int *strToInt(char * s) {
    // Lets convert it into a long
    long value = atol(s);
    // and check if it is a legal integer value
    if (value >= INT_MIN && value <= INT_MAX) {
        // Now convert the integer value back to a string
        char* asString = malloc(12);
        sprintf(asString, "%ld", value);
        // And make sure it is equal to the original string
        if (strcmp(s, asString) == 0) {
            free(asString);
            // Now allocate space for the actual value
            int* integer = malloc(sizeof(int));
            *integer = (int)value;
            return integer;
        } else {
            // Not the same
            free(asString);
        }
    }
    return NULL;
}

float *strToFloat(char * s) {
    // Lets convert it into a long
    double value = 0;
    sscanf(s, "%lf", &value);
    // and check if it is a legal integer value
    if (value >= DBL_MIN && value <= DBL_MAX) {
        // Now convert the float value back to a string
        char* asString = malloc(20);
        int nrOfDigits = strlen(strstr(s, ".")) - 1;
        sprintf(asString, "%.*f", nrOfDigits,value);
        // And make sure it is equal to the original string
        if (strcmp(s, asString) == 0) {
            free(asString);
            // Now allocate space for the actual value
            float* floatNumber = malloc(sizeof(float));
            *floatNumber = (float)value;
            return floatNumber;
        } else {
            // Not the same
            free(asString);
        }
    }
    return NULL;
}
