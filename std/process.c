/*
 * Copyright (c) 2026 ELECTRON VISION.
 *
 * ELX SDK — ELECTRON VISION
 *
 * All rights reserved.
 *
 * Licensed under the VOLT 1.26 License Agreement.
 * See the LICENSE file for the applicable terms.
 */

#define _POSIX_C_SOURCE 200809L
#include "elx_std_process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#endif
const char *elx_std_process_version(void) { return "0.1.0"; }
int elx_proc_get_id(void) {
#if defined(_WIN32)
    return _getpid();
#else
    return getpid();
#endif
}
int elx_proc_get_exit_code(void) { return 0; }
elx_proc_status_t elx_proc_get_status(void) { return ELX_PROC_STATUS_RUNNING; }
bool elx_proc_execute(const char *command, const char **args, size_t arg_count,
                        const char *working_dir, const char **env_pairs, size_t env_count,
                        int *out_exit_code, char **out_stdout, size_t *out_stdout_len,
                        char **out_stderr, size_t *out_stderr_len) {
    (void)args; (void)arg_count; (void)env_pairs; (void)env_count;
    (void)out_stdout; (void)out_stdout_len; (void)out_stderr; (void)out_stderr_len;
    if (!command) return false;
#if defined(_WIN32)
    int result = system(command);
    if (out_exit_code) *out_exit_code = (result == -1) ? -1 : result;
#else
    pid_t pid = fork();
    if (pid < 0) return false;
    if (pid == 0) {
        if (working_dir) chdir(working_dir);
        execl("/bin/sh", "sh", "-c", command, (char *)NULL);
        _exit(127);
    }
    int status; waitpid(pid, &status, 0);
    if (out_exit_code) *out_exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
    return true;
}
elx_proc_handle_t *elx_proc_spawn(const char *program, const char **args, size_t arg_count,
                                   const char *working_dir, const char **env_pairs, size_t env_count,
                                   bool background) {
    (void)env_pairs; (void)env_count; (void)args; (void)arg_count;
    if (!program) return NULL;
#if defined(_WIN32)
    elx_proc_handle_t *handle = (elx_proc_handle_t *)calloc(1, sizeof(*handle));
    if (!handle) return NULL;
    handle->id = _getpid();
    return handle;
#else
    pid_t pid = fork();
    if (pid < 0) return NULL;
    if (pid == 0) {
        if (working_dir) chdir(working_dir);
        char *const *argv = (char *const *)args;
        if (args && arg_count > 0) execvp(args[0], argv);
        else execl("/bin/sh", "sh", "-c", program, (char *)NULL);
        _exit(127);
    }
    if (!background) { int status; waitpid(pid, &status, 0); }
    elx_proc_handle_t *handle = (elx_proc_handle_t *)calloc(1, sizeof(*handle));
    if (!handle) return NULL;
    handle->id = (int)pid;
    return handle;
#endif
}
bool elx_proc_wait(elx_proc_handle_t *handle, int *out_exit_code) { (void)handle; (void)out_exit_code; return false; }
bool elx_proc_kill(elx_proc_handle_t *handle) {
    if (!handle) return false;
#if !defined(_WIN32)
    if (!handle->id) return false;
    return kill(handle->id, SIGTERM) == 0;
#else
    return false;
#endif
}
bool elx_proc_close(elx_proc_handle_t *handle) { if (!handle) return false; free(handle); return true; }
bool elx_proc_get_info(elx_proc_info_t *info) {
    if (!info) return false;
    info->id = elx_proc_get_id(); info->status = ELX_PROC_STATUS_RUNNING; info->exit_code = 0;
    return true;
}
char *elx_proc_get_exe_path(size_t *out_length) { (void)out_length; return NULL; }
char *elx_proc_get_working_dir(size_t *out_length) { (void)out_length; return NULL; }
bool elx_proc_pipe_create(int *read_fd, int *write_fd) { (void)read_fd; (void)write_fd; return false; }
bool elx_proc_pipe_close(int fd) { (void)fd; return false; }
bool elx_exec(const char *program, const char **args) {
    if (!program) return false;
    char *const *argv = (char *const *)args;
    execvp(program, argv);
    return false;
}
