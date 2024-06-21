#include "cmdparse.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char **split_string_into_args(const char *str, size_t *num_args) {
    char *str_copy = strdup(str);
    if (!str_copy) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    size_t count = 0;
    char *token = strtok(str_copy, " ");
    while (token) {
        count++;
        token = strtok(NULL, " ");
    }

    char **args = malloc((count + 1) * sizeof(char *));
    if (!args) {
        perror("malloc");
        free(str_copy);
        exit(EXIT_FAILURE);
    }

    strcpy(str_copy, str);
    size_t index = 0;
    token = strtok(str_copy, " ");
    while (token) {
        args[index++] = strdup(token);
        token = strtok(NULL, " ");
    }
    args[index] = NULL; // Null-terminator

    if (num_args) *num_args = count;
    free(str_copy);
    return args;
}

void free_args(char **args, size_t num_args) {
    for (size_t i = 0; i < num_args; i++) {
        free(args[i]);
    } 

    free(args);
}