#ifndef _SUBPROCESS_H
#define _SUBPROCESS_H

#include <sys/types.h>

#include "config.h"
#include "stdiobuf.h"

typedef struct childproc_s {
    pid_t pid;
    int running;
    int stdin_fd;
    int stdout_fd;
    int stderr_fd;
    stdiobuf_t stdiobuf;
} childproc_t;

int start_tine_proc(tine_t *tine, childproc_t *child_info);
int any_running(childproc_t *procs, size_t nprocs);

#endif