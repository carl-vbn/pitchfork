#include "subprocess.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

#include "cmdparse.h"

int start_tine_proc(tine_t *tine, child_process* child_info) {
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
        // Setup pipes
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);

        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        chdir(tine->wdir);

        // TODO setuid
        // TODO setenv

        // Prevent block buffering
        setenv("_STDBUF_O", "L", 0);
        setenv("_STDBUF_E", "L", 0);
        setenv("LD_PRELOAD", "/usr/libexec/coreutils/libstdbuf.so", 0);
        setenv("PYTHONUNBUFFERED", "1", 1);

        // Run
        char **child_args = split_string_into_args(tine->run_cmd, NULL);
        execvp(child_args[0], child_args);
        // No need to free child_args because of execvp
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

int any_running(child_process *procs, size_t nprocs) {
    for (size_t i = 0; i<nprocs; i++) {
        if (procs[i].running) {
            return 1;
        }
    }
    return 0;
}