#include "config.h"

#include <yaml.h>
#include <string.h>

#define TRY_PARSE(parser_ptr, event_ptr) if (!yaml_parser_parse(parser_ptr, event_ptr)) {\
    fprintf(stderr, "YAML Parser error %d\n", (parser_ptr)->error);\
    return -1;\
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
    tine->run_cmd = NULL;
    tine->wdir = NULL;
    char propname[64] = {0};
    while (1) {
        TRY_PARSE(parser, &event);

        if (event.type == YAML_MAPPING_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        if (event.type != YAML_SCALAR_EVENT) {
            fprintf(stderr, "%d %d\n", event.type, YAML_MAPPING_END_EVENT);
            fprintf(stderr, "parse_tine: Bad config.\n");
            yaml_event_delete(&event);
            return -1;
        }

        if (propname[0]) {
            char *propval = (char*) event.data.scalar.value;
            
            if (strcmp(propname, "run") == 0) {
                tine->run_cmd = strdup(propval);
            } else if (strcmp(propname, "working-directory") == 0) {
                tine->wdir = strdup(propval);
            }

            propname[0] = '\0';
        } else {
            strncpy(propname, (char*) event.data.scalar.value, 64);
            propname[63] = '\0';
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
            tine_t *copy = malloc(sizeof(tine_t) * new_capacity);
            memcpy(copy, *tines, sizeof(tine_t) * capacity);
            free(*tines);
            *tines = copy;
            capacity = new_capacity;
        }
        
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
    for (size_t i = 0; i<config->ntines; i++) {
        free(config->tines[i].name);
        free(config->tines[i].wdir);
        free(config->tines[i].run_cmd);
    }
    free(config->tines);
}