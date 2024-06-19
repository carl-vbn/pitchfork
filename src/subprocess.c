#include "subprocess.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int start_child(const char* executable, char* const* args, char* const* envp, child_process* child_info) {
    int stdin_pipe[2];
    int stdout_pipe[2];
    int stderr_pipe[2];

    if (pipe(stdin_pipe) != 0|| pipe(stdout_pipe) != 0 || pipe(stderr_pipe)) {
        perror("pipe");
        return -1;
    }

    pid_t child_pid = fork();

    if (child_pid < 0) { // Fork error
        perror("fork");
        return -1;
    } else if (child_pid == 0) { // Child code
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);

        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        // TODO Setuid

        setenv("_STDBUF_O", "L", 0);
        setenv("_STDBUF_E", "L", 0);
        setenv("LD_PRELOAD", "/usr/libexec/coreutils/libstdbuf.so", 0);
        setenv("PYTHONUNBUFFERED", "1", 1);
        execvp(executable, args);

        return 0;
    } else { // Parent code
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        if (child_info != NULL) {
            child_info->pid = child_pid;
            child_info->running = 1;
            child_info->stdin_fd = stdin_pipe[1];
            child_info->stdout_fd = stdout_pipe[0];
            child_info->stderr_fd = stderr_pipe[0];
        }

        return child_pid;
    }
}