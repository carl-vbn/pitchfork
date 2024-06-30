#include "ctrlsock.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

int setup_ctrlsock(int *sock, const char *path) {
    *sock = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (*sock == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);

    unlink(path);

    if (bind(*sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        return -1;
    }

    if (listen(*sock, 5) == -1) {
        perror("listen");
        return -1;
    }

    return 0;
}

int handle_ctrlsock_cmd(char *cmd, tine_t *tines, child_process *tine_procs, size_t ntines) {
    // Command format: <tine-name> <command> [args]
    char *tine_name = strtok(cmd, " ");
    char *command = strtok(NULL, " ");

    if (tine_name == NULL || command == NULL) {
        return -1;
    }

    for (size_t i = 0; i<ntines; i++) {
        if (strcmp(tines[i].name, tine_name) == 0) {
            if (strcmp(command, "signal") == 0) {
                char *signal = strtok(NULL, " ");
                if (signal == NULL) {
                    return -1;
                }

                int signum = atoi(signal);
                if (signum == 0) {
                    return -1;
                }

                if (kill(tine_procs[i].pid, signum) == -1) {
                    perror("kill");
                    return -1;
                }
            }
        }
    }

    return 0;
}