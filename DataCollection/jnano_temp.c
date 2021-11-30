/**
 * @file jnano_temp.c
 * @author cs
 * @brief This file contains the implementations of functions 
 * for reading Jetson Nano's values of thermal zones.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "jnano_temp.h"

/**
 * @brief Read on-chip and on-module temperatures.
 *
 * @param zone Indexed by ::jnano_tzone
 * @param *temp Reference to variable storing temperature
 */
void jnano_get_temp(jnano_tzone zone, unsigned int *temp)
{
    FILE *fp;
    char buff[MAX_BUFF];
    unsigned long ans;
    
    snprintf(buff, sizeof(buff), SYSFS_TEMP_PATH "/thermal_zone%d/temp", zone);

    fp = fopen(buff, "r");

    if (fp == NULL) {
        fprintf(stderr, "Error opening file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (!fscanf(fp, "%lu", &ans)) {
        fprintf(stderr, "Error scanning the file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        fclose(fp);
    }
    
    *temp = ans;
}
