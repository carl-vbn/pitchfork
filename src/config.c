#include "config.h"

#include <yaml.h>
#include <string.h>

#define TRY_PARSE(parser_ptr, event_ptr) if (!yaml_parser_parse(parser_ptr, event_ptr)) {\
    fprintf(stderr, "YAML Parser error %d\n", (parser_ptr)->error);\
    return -1;\
}

void init_config(config_t *cfg) {
    cfg->ctrlsock_enabled = 0;
    cfg->ctrlsock_path = NULL;
    cfg->ntines = 0;
    cfg->tines = NULL;
}

void init_tine(tine_t *tine) {
    tine->name = NULL;
    tine->wdir = NULL;
    tine->run_cmd = NULL;
    tine->env = NULL;
    tine->nenv = 0;
}

int parse_env(tine_t *tine, yaml_parser_t *parser) {    
    yaml_event_t event;

    TRY_PARSE(parser, &event);

    if (event.type != YAML_MAPPING_START_EVENT) {
        fprintf(stderr, "parse_env: Bad config.\n");
        yaml_event_delete(&event);
        return -1;
    }

    yaml_event_delete(&event);
    char propname[64] = {0};
    while (1) {
        TRY_PARSE(parser, &event);

        if (event.type == YAML_MAPPING_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        if (event.type != YAML_SCALAR_EVENT) {
            fprintf(stderr, "parse_env: Bad config.\n");
            yaml_event_delete(&event);
            return -1;
        }

        if (propname[0]) {
            char *propval = (char*) event.data.scalar.value;
            
            tine->env = realloc(tine->env, sizeof(env_t) * (tine->nenv + 1));
            tine->env[tine->nenv].key = strdup(propname);
            tine->env[tine->nenv].value = strdup(propval);
            tine->nenv++;

            propname[0] = '\0';
        } else {
            strncpy(propname, (char*) event.data.scalar.value, 64);
            propname[63] = '\0';
        }

        yaml_event_delete(&event);
    }

    return 0;
}

// Returns 1 if a tine was successfully parsed
//         0 if we have reached the end of the tines mapping
//        -1 on error
int parse_tine(tine_t *tine, yaml_parser_t *parser) {
    yaml_event_t event;

    TRY_PARSE(parser, &event);

    if (event.type == YAML_MAPPING_END_EVENT) {
        yaml_event_delete(&event);
        return 0;
    } else if (event.type == YAML_SCALAR_EVENT) {
        char *tine_name = (char*)event.data.scalar.value;
        tine->name = strdup(tine_name);
    }

    yaml_event_delete(&event);
    TRY_PARSE(parser, &event);

    if (event.type != YAML_MAPPING_START_EVENT) {
        fprintf(stderr, "parse_tine: Bad config.\n");
        yaml_event_delete(&event);
        return -1;
    }

    yaml_event_delete(&event);

    // Parse provided properties
    char propname[64] = {0};
    while (1) {
        TRY_PARSE(parser, &event);

        if (event.type == YAML_MAPPING_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        if (event.type != YAML_SCALAR_EVENT) {
            fprintf(stderr, "parse_tine: Bad config.\n");
            yaml_event_delete(&event);
            return -1;
        }

        if (propname[0]) {
            char *propval = (char*) event.data.scalar.value;
            
            if (strcmp(propname, "run") == 0) {
                tine->run_cmd = strdup(propval);
            } else if (strcmp(propname, "wd") == 0) {
                tine->wdir = strdup(propval);
            } else if (strcmp(propname, "env") == 0) {
                parse_env(tine, parser);
            }

            propname[0] = '\0';
        } else {
            strncpy(propname, (char*) event.data.scalar.value, 64);
            propname[63] = '\0';

            if (strcmp(propname, "env") == 0) {
                parse_env(tine, parser);
            }
        }

        yaml_event_delete(&event);
    }

    return 1;
}

int parse_tines(tine_t **tines, size_t *ntines, yaml_parser_t *parser) {
    size_t capacity = 2;
    *tines = malloc(sizeof(tine_t) * capacity);
    *ntines = 0;

    yaml_event_t event;

    TRY_PARSE(parser, &event);
    if (event.type != YAML_MAPPING_START_EVENT) {
        fprintf(stderr, "parse_tines: Bad config.\n");
        return -1;
    }

    yaml_event_delete(&event);

    while (1) {
        if (*ntines >= capacity) {
            // Reallocate tine array with bigger capacity
            size_t new_capacity = capacity * 2;
            *tines = realloc(*tines, sizeof(tine_t) * new_capacity);
            capacity = new_capacity;
        }
        
        init_tine(*tines + *ntines);
        int result = parse_tine(*tines + *ntines, parser);
        
        if (result == 0) {
            break; // Done parsing
        } else if (result < 0) {
            fprintf(stderr, "parse_tines: Failed to parse tines[%ld]\n", *ntines);
            return -1;
        }

        (*ntines)++;
    }

    return 0;
}

int parse_ctrlsock_settings(config_t *config, yaml_parser_t *parser) {
    yaml_event_t event;

    TRY_PARSE(parser, &event);

    if (event.type != YAML_MAPPING_START_EVENT) {
        fprintf(stderr, "parse_ctrlsock_settings: Bad config.\n");
        return -1;
    }

    yaml_event_delete(&event);

    char propname[64] = {0};
    while (1) {
        TRY_PARSE(parser, &event);

        if (event.type == YAML_MAPPING_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        if (event.type != YAML_SCALAR_EVENT) {
            fprintf(stderr, "parse_ctrlsock_settings: Bad config.\n");
            yaml_event_delete(&event);
            return -1;
        }

        if (propname[0]) {
            char *propval = (char*) event.data.scalar.value;
            
            if (strcmp(propname, "enabled") == 0) {
                config->ctrlsock_enabled = strcmp(propval, "true") == 0;
            } else if (strcmp(propname, "path") == 0) {
                config->ctrlsock_path = strdup(propval);
            }

            propname[0] = '\0';
        } else {
            strncpy(propname, (char*) event.data.scalar.value, 64);
            propname[63] = '\0';
        }

        yaml_event_delete(&event);
    }

    return 0;
}

int load_config(FILE *file, config_t *config) {
    yaml_parser_t parser;

    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize YAML parser.\n");
        return -1;
    }

    yaml_parser_set_input_file(&parser, file);

    yaml_event_t  event;
    do
    {
        TRY_PARSE(&parser, &event);

        if (event.type == YAML_SCALAR_EVENT) {
            if (strcmp((char*)event.data.scalar.value, "tines") == 0) {
                parse_tines(&config->tines, &config->ntines, &parser);
            } else if (strcmp((char*)event.data.scalar.value, "control-socket") == 0) {
                parse_ctrlsock_settings(config, &parser);
            }
        }
        if (event.type != YAML_STREAM_END_EVENT) {
            yaml_event_delete(&event);
        }
    } while (event.type != YAML_STREAM_END_EVENT);

    yaml_event_delete(&event);
    yaml_parser_delete(&parser);

    return 0;
}

void delete_config(config_t *config) {
    free(config->ctrlsock_path);
    
    for (size_t i = 0; i<config->ntines; i++) {
        free(config->tines[i].name);
        free(config->tines[i].wdir);
        free(config->tines[i].run_cmd);

        for (size_t j = 0; j<config->tines[i].nenv; j++) {
            free(config->tines[i].env[j].key);
            free(config->tines[i].env[j].value);
        }
        free(config->tines[i].env);
    }
    free(config->tines);

}