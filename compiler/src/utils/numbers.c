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
#include "dbug.h"

#include"numbers.h"

int *strToInt(char * s) {
    DBUG_ENTER("strToInt");

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
            DBUG_RETURN(integer);
        } else {
            // Not the same
            free(asString);
        }
    }
    DBUG_RETURN(NULL);
}

float *strToFloat(char * s) {
    DBUG_ENTER("strToFloat");
    // Lets convert it into a long
    double value = 0;
    sscanf(s, "%lf", &value);
    // and check if it is a legal float value
    if (value >= -FLT_MAX && value <= FLT_MAX) {
        // Now allocate space for the actual value
        float* floatNumber = malloc(sizeof(float));
        *floatNumber = (float)value;
        DBUG_RETURN(floatNumber);
    } else {
        DBUG_PRINT("UTIL", ("%lf > %lf > %lf", -FLT_MAX, value, FLT_MAX));
    }
    DBUG_RETURN(NULL);
}
