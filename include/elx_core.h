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

#ifndef ELX_CORE_H
#define ELX_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_CORE_VERSION "0.4.0-phase4"

typedef enum {
    ELX_TARGET_UNKNOWN = 0,
    ELX_TARGET_ELVI,
    ELX_TARGET_WINDOWS,
    ELX_TARGET_LINUX,
    ELX_TARGET_MACOS,
    ELX_TARGET_ARM64,
} elx_target_t;

typedef enum {
    ELX_BACKEND_ELBC = 0,
    ELX_BACKEND_NATIVE_C,
    ELX_BACKEND_NATIVE_ASM,
} elx_backend_t;

typedef struct {
    const char *source;
    size_t length;
    const char *input_path;
    const char *output_path;
    elx_target_t target;
    elx_backend_t backend;
    bool optimize;
    bool dump_tokens;
    bool dump_ast;
    bool dump_c;
} elx_sdk_compile_request_t;

typedef struct {
    int code;
    const char *message;
    size_t line;
    size_t column;
} elx_error_t;

typedef struct {
    elx_error_t *items;
    size_t count;
    size_t capacity;
    bool has_errors;
} elx_error_list_t;

typedef struct {
    const char *c_source;
    size_t c_length;
    const char *elbc_path;
    const char *output_path;
    bool compiled;
} elx_compile_result_t;

void elx_error_init(elx_error_list_t *list);
void elx_error_destroy(elx_error_list_t *list);
void elx_error_add(elx_error_list_t *list, int code, const char *message, size_t line, size_t column);
void elx_error_print(const elx_error_list_t *list, const char *source_name, FILE *stream);

const char *elx_core_version(void);
elx_target_t elx_target_from_name(const char *name);
const char *elx_target_name(elx_target_t target);
elx_backend_t elx_backend_from_name(const char *name);
const char *elx_backend_name(elx_backend_t backend);

bool elx_sdk_compile(const elx_sdk_compile_request_t *request, elx_compile_result_t *result, elx_error_list_t *errors);
bool elx_sdk_compile_source(const char *source, size_t length, const char *output_path, elx_target_t target, elx_backend_t backend, bool optimize, elx_compile_result_t *result, elx_error_list_t *errors);
bool elx_sdk_compile_file(const char *input_path, const char *output_path, elx_target_t target, elx_backend_t backend, bool optimize, elx_compile_result_t *result, elx_error_list_t *errors);

bool elx_sdk_run(const char *elbc_path, int *exit_code);

const char *elx_std_io_version(void);
const char *elx_std_fs_version(void);
const char *elx_std_process_version(void);
const char *elx_std_env_version(void);
const char *elx_std_time_version(void);
const char *elx_std_str_version(void);
const char *elx_std_list_version(void);
const char *elx_std_map_version(void);
const char *elx_std_set_version(void);
const char *elx_std_math_version(void);
const char *elx_std_error_version(void);
const char *elx_std_net_version(void);
const char *elx_native_version(void);
bool elx_native_is_available(void);
const char *elx_native_compiler_path(void);

const char *elx_sdk_security_version(void);

#ifdef __cplusplus
}
#endif

#endif
