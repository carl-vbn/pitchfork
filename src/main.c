#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>

#include "subprocess.h"
#include "colors.h"
#include "config.h"
#include "cmdparse.h"
#include "ctrlsock.h"
#include "stdiobuf.h"

// The size of the input buffer used when reading process IO
#define RDBUFSIZE 4096

// Maximum number of simulatenously connected control socket clients
#define MAX_CTRLSOCK_CLIENTS 5

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config-file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Config loading */
    FILE *config_file = fopen(argv[1], "r");
    if (config_file == NULL) {
        fprintf(stderr, "%s: No such file or directory\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    config_t config;
    init_config(&config);
    if (load_config(config_file, &config) < 0) {
        fprintf(stderr, "Failed to load config\n");
        return EXIT_FAILURE;
    }
    fclose(config_file);

    /* Start tines */
    childproc_t *tine_procs = malloc(config.ntines * sizeof(childproc_t));
    for (size_t i = 0; i<config.ntines; i++) {
        start_tine_proc(&config.tines[i], tine_procs+i);
        stdiobuf_init(&tine_procs[i].stdiobuf);
    }

    // Setup control socket
    int ctrlsock = -1;
    if (config.ctrlsock_enabled) {
        if (setup_ctrlsock(&ctrlsock, config.ctrlsock_path) != 0) {
            fprintf(stderr, "Failed to setup control socket\n");
            return EXIT_FAILURE;
        }

        printf("%sControl socket listening on %s%s\n", GRN, config.ctrlsock_path, CRESET);
    }
    

    fd_set fdset;
    int ctrlsock_clients[MAX_CTRLSOCK_CLIENTS];
    memset(ctrlsock_clients, -1, sizeof(ctrlsock_clients));

    /* IO Loop */
    while (any_running(tine_procs, config.ntines)) {
        FD_ZERO(&fdset);

        for (int i = 0; i<config.ntines; i++) {
            childproc_t *task = tine_procs + i;
            FD_SET(task->stdout_fd, &fdset);
            FD_SET(task->stderr_fd, &fdset);

            int status;
            if (waitpid(task->pid, &status, WNOHANG) == task->pid) {
                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    task->running = 0;
                    printf("%sChild %d terminated.%s\n", CYN, task->pid, CRESET);
                }
            }
        }

        if (config.ctrlsock_enabled) {
            int client = accept(ctrlsock, NULL, NULL);
            if (client != -1) {
                for (int i = 0; i<MAX_CTRLSOCK_CLIENTS + 1; i++) {
                    if (i == MAX_CTRLSOCK_CLIENTS) {
                        printf("%sMaximum number of control socket clients reached%s\n", RED, CRESET);
                        close(client);
                        break;
                    }

                    if (ctrlsock_clients[i] == -1) {
                        ctrlsock_clients[i] = client;
                        printf("%sAccepted connection on control socket%s\n", GRN, CRESET);
                        break;
                    }
                }
            }
        }
        
        if (select(FD_SETSIZE, &fdset, NULL, NULL, NULL) != -1) {
            int read_bytes;
            char buf[RDBUFSIZE];

            for (int i = 0; i<config.ntines; i++) {
                childproc_t *task = tine_procs + i;

                if (FD_ISSET(task->stdout_fd, &fdset)) {
                    read_bytes = read(task->stdout_fd, buf, sizeof(buf));
                    write(STDOUT_FILENO, buf, read_bytes);
                    stdiobuf_write(&tine_procs[i].stdiobuf, buf, read_bytes);
                }

                if (FD_ISSET(task->stderr_fd, &fdset)) {
                    read_bytes = read(task->stderr_fd, buf, sizeof(buf));
                    write(STDOUT_FILENO, RED, 7);
                    write(STDOUT_FILENO, buf, read_bytes);
                    write(STDOUT_FILENO, CRESET, 4);
                }
            }

            if (config.ctrlsock_enabled) {
                for (int j = 0; j<MAX_CTRLSOCK_CLIENTS; j++) {
                    if (ctrlsock_clients[j] == -1) {
                        continue;
                    }

                    ssize_t read_bytes = recv(ctrlsock_clients[j], buf, sizeof(buf) - 1, MSG_DONTWAIT);
                    if (read_bytes > 0) {
                        if (buf[read_bytes-1] == '\n') {
                            buf[read_bytes-1] = '\0';
                        } else {
                            buf[read_bytes] = '\0';
                        }
                        printf("%sReceived command: %s%s\n", CYN, buf, CRESET);
                        if (handle_ctrlsock_cmd(ctrlsock_clients[j], buf, config.tines, tine_procs, config.ntines) != 0) {
                            printf("%sFailed to handle command%s\n", RED, CRESET);
                        }
                    } else if (read_bytes == 0) {
                        printf("%sA control socket client disconnected%s\n", YEL, CRESET);
                        close(ctrlsock_clients[j]);
                        ctrlsock_clients[j] = -1;
                    } else {
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            perror("recv");
                        }
                    }
                }
            }
        } else {
            perror("select");
            return EXIT_FAILURE;
        }
    }

    /* Clean up */

    if (config.ctrlsock_enabled) {
        close(ctrlsock);
        unlink(config.ctrlsock_path);
    }


    for (size_t i = 0; i<config.ntines; i++) {
        stdiobuf_dispose(&tine_procs[i].stdiobuf);
    }

    free(tine_procs);
    delete_config(&config);

    return EXIT_SUCCESS;
}