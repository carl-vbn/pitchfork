#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>

typedef struct env_s {
    char *key;
    char *value;
} env_t;

typedef struct tine_s {
    char *name;
    char *wdir;
    char *run_cmd;
    env_t *env;
    size_t nenv;
} tine_t;

typedef struct config_s {
    int ctrlsock_enabled;
    char *ctrlsock_path;
    tine_t *tines;
    size_t ntines;
} config_t;

void init_config(config_t *cfg);
int load_config(FILE *file, config_t *cfg);
void delete_config(config_t *config);
#endif