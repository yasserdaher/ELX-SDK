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

#ifndef ELX_NATIVE_H
#define ELX_NATIVE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "elx_config.h"

typedef enum {
    ELX_NATIVE_OK = 0,
    ELX_NATIVE_ERROR,
    ELX_NATIVE_COMPILE_ERROR,
    ELX_NATIVE_LINK_ERROR,
    ELX_NATIVE_RUNTIME_ERROR,
} elx_native_status_t;

typedef struct {
    const char *c_source;
    size_t c_length;
    const char *object_path;
    const char *executable_path;
    elx_native_status_t status;
    char *error_message;
} elx_native_result_t;

typedef struct {
    const char *compiler_path;
    const char *linker_path;
    const char *c_flags;
    const char *linker_flags;
    const char *output_dir;
    bool use_temp_dir;
} elx_native_config_t;

const char *elx_native_version(void);
bool elx_native_is_available(void);
const char *elx_native_compiler_path(void);
elx_native_status_t elx_native_compile_c(const char *c_source, size_t c_length,
                                         const char *output_path,
                                         const elx_native_config_t *config,
                                         elx_native_result_t *result);
elx_native_status_t elx_native_compile_file(const char *c_file_path,
                                            const char *output_path,
                                            const elx_native_config_t *config,
                                            elx_native_result_t *result);
elx_native_status_t elx_native_compile_run(const char *c_source, size_t c_length,
                                            int *out_exit_code,
                                            const elx_native_config_t *config);
elx_native_status_t elx_native_build(const char *project_dir, const char *target_name,
                                      bool release, elx_native_result_t *result);
void elx_native_result_destroy(elx_native_result_t *result);

#ifdef __cplusplus
}
#endif

#endif
