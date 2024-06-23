#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <stdlib.h>

#include "subprocess.h"
#include "colors.h"
#include "config.h"
#include "cmdparse.h"

#define BUFSIZE 4096

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config-file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *config_file = fopen(argv[1], "r");
    if (config_file == NULL) {
        fprintf(stderr, "%s: No such file or directory\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    config_t config;
    if (load_config(config_file, &config) < 0) {
        fprintf(stderr, "Failed to load config\n");
        return EXIT_FAILURE;
    }
    fclose(config_file);

    child_process *tine_procs = malloc(config.ntines * sizeof(child_process));
    for (size_t i = 0; i<config.ntines; i++) {
        start_tine_proc(&config.tines[i], tine_procs+i);
    }

    delete_config(&config);

    fd_set fdset;

    while (any_running(tine_procs, config.ntines)) {
        FD_ZERO(&fdset);

        for (int i = 0; i<config.ntines; i++) {
            child_process *task = tine_procs + i;
            FD_SET(task->stdout_fd, &fdset);
            FD_SET(task->stderr_fd, &fdset);

            int status;
            if (waitpid(task->pid, &status, WNOHANG) == task->pid) {
                if (WIFEXITED(status)) {
                    task->running = 0;
                    printf("%sChild %d terminated.%s\n", CYN, task->pid, CRESET);
                }
            }
        }
        
        if (select(FD_SETSIZE, &fdset, NULL, NULL, NULL) != -1) {
            int read_bytes;
            char *buf[BUFSIZE];

            for (int i = 0; i<config.ntines; i++) {
                child_process *task = tine_procs + i;

                if (FD_ISSET(task->stdout_fd, &fdset)) {
                    while ((read_bytes = read(task->stdout_fd, buf, sizeof(buf))) > 0) {
                        write(STDOUT_FILENO, buf, read_bytes);
                    }
                }

                if (FD_ISSET(task->stderr_fd, &fdset)) {
                    while ((read_bytes = read(task->stderr_fd, buf, sizeof(buf))) > 0) {
                        write(STDOUT_FILENO, RED, 7);
                        write(STDOUT_FILENO, buf, read_bytes);
                        write(STDOUT_FILENO, CRESET, 4);
                    }
                }
            }
        } else {
            perror("select");
            return EXIT_FAILURE;
        }
    }

    free(tine_procs);

    return EXIT_SUCCESS;
}