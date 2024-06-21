#ifndef SUBPROCESS_H
#define SUBPROCESS_H

#include <sys/types.h>

#include "config.h"

typedef struct child_process {
    pid_t pid;
    int running;
    int stdin_fd;
    int stdout_fd;
    int stderr_fd;
} child_process;

int start_tine_proc(tine_t *tine, child_process* child_info);
int any_running(child_process *procs, size_t nprocs);

#endif