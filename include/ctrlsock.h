#ifndef _CTRLSOCK_H
#define _CTRLSOCK_H

#include "subprocess.h"
#include "config.h"

int setup_ctrlsock(int *sock, const char *path);
int handle_ctrlsock_cmd(char *cmd, tine_t *tines, child_process *tine_procs, size_t ntines);

#endif