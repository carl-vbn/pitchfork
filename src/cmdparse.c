#include "cmdparse.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>

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

char *expand_tilde(const char *input) {
    if (input[0] == '~') {
        if (input[1] == '/' || input[1] == '\0') {
            // Case for ~ or ~/path
            struct passwd *pw = getpwuid(getuid());
            if (!pw) {
                fprintf(stderr, "Error retrieving home directory: %s\n", strerror(errno));
                return strdup(input);
            }
            const char *homedir = pw->pw_dir;

            if (input[1] == '\0') {
                return strdup(homedir);
            } else {
                size_t output_length = strlen(homedir) + strlen(input + 1) + 1;
                char *output = malloc(output_length * sizeof(char));
                snprintf(output, output_length, "%s%s", homedir, input + 1);
                return output;
            }
        } else {
            // Case for ~username/path
            const char *slash = strchr(input, '/');
            char *username;
            if (slash) {
                size_t username_length = slash - input - 1;
                username = malloc(username_length + 1);
                 
                strncpy(username, input + 1, username_length);
                username[username_length] = '\0';
            } else {
                size_t input_length = strlen(input);
                username = malloc(input_length);
                strncpy(username, input + 1, input_length);
                username[input_length - 1] = '\0';
            }

            struct passwd *pw = getpwnam(username);
            if (!pw) {
                fprintf(stderr, "expand_tilde: User '%s' not found\n", username);
                free(username);
                return strdup(input);
            }
            free(username);
            const char *homedir = pw->pw_dir;
            size_t output_length;
            char *output;
            if (slash) {
                output_length = strlen(homedir) + strlen(slash) + 1;
                output = malloc(output_length * sizeof(char));
                snprintf(output, output_length, "%s%s", homedir, slash);
            } else {
                output_length = strlen(homedir) + 1;
                output = malloc(output_length * sizeof(char));
                strncpy(output, homedir, output_length - 1);
                output[output_length - 1] = '\0';
            }
            return output;
        }
    } else {
        return strdup(input);
    }
}