#ifndef CMDPARSE_H
#define CMDPARSE_H

#include <stddef.h>

char **split_string_into_args(const char *str, size_t *num_args);
void free_args(char **args, size_t num_args);
char *expand_tilde(const char *input);

#endif