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

#ifndef ELX_STD_PROCESS_H
#define ELX_STD_PROCESS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ELX_PROC_STATUS_RUNNING = 0,
    ELX_PROC_STATUS_EXITED,
    ELX_PROC_STATUS_SIGNALED,
    ELX_PROC_STATUS_UNKNOWN,
} elx_proc_status_t;

typedef struct {
    int id;
    char name[256];
    elx_proc_status_t status;
    int exit_code;
    uint64_t run_time_ms;
    size_t memory_usage_kb;
} elx_proc_info_t;

typedef struct {
    int id;
    void *impl;
} elx_proc_handle_t;

const char *elx_std_process_version(void);

int elx_proc_get_id(void);
int elx_proc_get_exit_code(void);
elx_proc_status_t elx_proc_get_status(void);

bool elx_proc_execute(const char *command, const char **args, size_t arg_count,
                      const char *working_dir, const char **env_pairs, size_t env_count,
                      int *out_exit_code, char **out_stdout, size_t *out_stdout_len,
                      char **out_stderr, size_t *out_stderr_len);

elx_proc_handle_t *elx_proc_spawn(const char *program, const char **args, size_t arg_count,
                                   const char *working_dir, const char **env_pairs, size_t env_count,
                                   bool background);
bool elx_proc_wait(elx_proc_handle_t *handle, int *out_exit_code);
bool elx_proc_kill(elx_proc_handle_t *handle);
bool elx_proc_close(elx_proc_handle_t *handle);

bool elx_proc_get_info(elx_proc_info_t *info);
char *elx_proc_get_exe_path(size_t *out_length);
char *elx_proc_get_working_dir(size_t *out_length);

bool elx_proc_pipe_create(int *read_fd, int *write_fd);
bool elx_proc_pipe_close(int fd);

bool elx_exec(const char *program, const char **args);

#ifdef __cplusplus
}
#endif

#endif
