#include "config.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "memory.h"

char  *config_read(char* key) {
    char p[PATH_MAX];
    memset(p, 0, sizeof(char)*PATH_MAX);
    strncpy(p, getenv("HOME"), strlen(getenv("HOME")));
    strcat(p, "/.config/lotus/lotus.conf\0");
    FILE *config_file = fopen(p, "r");
    if(config_file == NULL) {
        dprintf(2, "config file not found\n");
        return NULL;
    }
    char *value = NULL;
    char *line = NULL;
    size_t size = 0;
    ssize_t read = 0;

    while((read = getline(&line, &size, config_file)) != -1) {
        if(strstr(line, key) == NULL)
            continue;
        value = strchr(line, '=');
        if(value)
            value = strdup(value+1);
        break;
    }

    mem_free(line);
    fclose(config_file);
    return value;
}
