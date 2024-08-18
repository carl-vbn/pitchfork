#ifndef _CMDPARSE_H
#define _CMDPARSE_H

#include <stddef.h>
#include "config.h"

char **split_string_into_args(const char *str, size_t *num_args);
void free_args(char **args, size_t num_args);
char *expand_tilde(const char *input);
int tine_index_by_name(const char *name, tine_t *tines, size_t ntines);

#endif