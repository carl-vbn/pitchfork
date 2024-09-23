#include "ctrlsock.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "cmdparse.h"

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

int handle_ctrlsock_cmd(int client_fd, char *cmd, tine_t *tines, childproc_t *tine_procs, size_t ntines) {
    // Command format: <tine-name> <command> [args]
    char *command = strtok(cmd, " ");

    if (command == NULL) {
        return -1;
    }

    if (strcmp(command, "signal") == 0) {
        char *tine_name = strtok(NULL, " ");
        char *signal = strtok(NULL, " ");
        if (tine_name == NULL || signal == NULL) {
            return -1;
        }

        int i = tine_index_by_name(tine_name, tines, ntines);
        if (i == -1) {
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
    } else if (strcmp(command, "stdioread") == 0) {
        char *tine_name = strtok(NULL, " ");
        if (tine_name == NULL) {
            return -1;
        }

        int i = tine_index_by_name(tine_name, tines, ntines);
        if (i == -1) {
            return -1;
        }

        char buf[IOBUFSIZE];
        size_t read_bytes = stdiobuf_read(&tine_procs[i].stdiobuf, buf, sizeof(buf));

        if (read_bytes > 0) {
            if (send(client_fd, buf, read_bytes, 0) == -1) {
                perror("send");
                return -1;
            }
        }
    } else {
        return -1;
    }

    return 0;
}