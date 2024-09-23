#ifndef _CTRLSOCK_H
#define _CTRLSOCK_H

#include "subprocess.h"
#include "config.h"

int setup_ctrlsock(int *sock, const char *path);
int handle_ctrlsock_cmd(int client_fd, char *cmd, tine_t *tines, childproc_t *tine_procs, size_t ntines);

#endif