#ifndef SUBPROCESS_H
#define SUBPROCESS_H

#include <sys/types.h>

typedef struct child_process {
    pid_t pid;
    int running;
    int stdin_fd;
    int stdout_fd;
    int stderr_fd;
} child_process;

int start_child(const char* executable, char* const* args, char* const* envp, child_process* child_info);

#endif