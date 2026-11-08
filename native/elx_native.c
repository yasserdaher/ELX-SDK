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

#if defined(_WIN32)
    #define _CRT_SECURE_NO_WARNINGS 1
#endif
#include "elx_native.h"
#include "elx_config.h"
#include "elx_native_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
#endif

const char *elx_native_version(void) { return "0.1.0"; }

bool elx_native_is_available(void) { return elx_native_compiler_path() != NULL; }

const char *elx_native_compiler_path(void) {
#if defined(_WIN32)
    #if defined(_MSC_VER)
    return "cl.exe";
    #else
    return "gcc";
    #endif
#else
    return "gcc";
#endif
}

static const char *elx_native_temp_dir(void) {
#if defined(_WIN32)
    static char buf[MAX_PATH];
    DWORD len = GetTempPathA(sizeof(buf), buf);
    if (len == 0 || len >= sizeof(buf)) return ".";
    return buf;
#else
    const char *tmp = getenv("TMPDIR");
    if (tmp && tmp[0]) return tmp;
    tmp = getenv("TMP");
    if (tmp && tmp[0]) return tmp;
    tmp = getenv("TEMP");
    if (tmp && tmp[0]) return tmp;
    return "/tmp";
#endif
}

elx_native_status_t elx_native_compile_c(const char *c_source, size_t c_length,
                                         const char *output_path,
                                         const elx_native_config_t *config,
                                         elx_native_result_t *result) {
    if (!c_source || !output_path || !result) return ELX_NATIVE_ERROR;
    result->c_source = c_source;
    result->c_length = c_length;
    result->executable_path = output_path;
    result->status = ELX_NATIVE_OK;
    result->error_message = NULL;

    const char *tmp_dir = elx_native_temp_dir();
    char tmp_path[2048];
    snprintf(tmp_path, sizeof(tmp_path), "%s/elx_native_%p.c", tmp_dir, (void *)c_source);

    FILE *out = fopen(tmp_path, "wb");
    if (!out) { result->status = ELX_NATIVE_COMPILE_ERROR; result->error_message = strdup("cannot write temp file"); return result->status; }
    size_t written = fwrite(c_source, 1, c_length, out);
    fclose(out);
    if (written != c_length) {
        remove(tmp_path);
        result->status = ELX_NATIVE_COMPILE_ERROR;
        result->error_message = strdup("incomplete write to temp file");
        return result->status;
    }

    const char *compiler = config ? (config->compiler_path ? config->compiler_path : "gcc") : "gcc";
    const char *c_flags = config ? (config->c_flags ? config->c_flags : "-O2 -w") : "-O2 -w";

    char command[8192];
    if (config && config->linker_flags && config->linker_flags[0]) {
        snprintf(command, sizeof(command), "%s %s \"%s\" %s -o \"%s\" 2>&1",
                 compiler, c_flags, tmp_path, config->linker_flags, output_path);
    } else {
        snprintf(command, sizeof(command), "%s %s \"%s\" -o \"%s\" 2>&1",
                 compiler, c_flags, tmp_path, output_path);
    }

    FILE *rp = popen(command, "r");
    if (!rp) {
        remove(tmp_path);
        result->status = ELX_NATIVE_COMPILE_ERROR;
        result->error_message = strdup("cannot run compiler");
        return result->status;
    }

    char error_buf[1024];
    size_t err_len = 0;
    char *error_msg = NULL;
    size_t n;
    while ((n = fread(error_buf, 1, sizeof(error_buf) - 1, rp)) > 0) {
        error_buf[n] = '\0';
        char *nm = (char *)realloc(error_msg, err_len + n + 1);
        if (!nm) break;
        error_msg = nm;
        memcpy(error_msg + err_len, error_buf, n);
        err_len += n;
        error_msg[err_len] = '\0';
    }

    int exit_code = pclose(rp);
    remove(tmp_path);

    if (exit_code != 0) {
        result->status = ELX_NATIVE_COMPILE_ERROR;
        result->error_message = error_msg ? error_msg : strdup("unknown compiler error");
    } else {
        free(error_msg);
    }
    return result->status;
}

elx_native_status_t elx_native_compile_file(const char *c_file_path, const char *output_path,
                                             const elx_native_config_t *config, elx_native_result_t *result) {
    if (!c_file_path || !output_path || !result) return ELX_NATIVE_ERROR;
    FILE *file = fopen(c_file_path, "rb");
    if (!file) { result->status = ELX_NATIVE_ERROR; result->error_message = strdup("cannot open C source file"); return result->status; }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *source = (char *)malloc((size_t)size + 1);
    if (!source) { fclose(file); result->status = ELX_NATIVE_ERROR; result->error_message = strdup("out of memory"); return result->status; }
    size_t read = fread(source, 1, (size_t)size, file);
    source[read] = '\0';
    fclose(file);
    elx_native_status_t status = elx_native_compile_c(source, read, output_path, config, result);
    free(source);
    return status;
}

elx_native_status_t elx_native_compile_run(const char *c_source, size_t c_length,
                                            int *out_exit_code, const elx_native_config_t *config) {
    const char *tmp_dir = elx_native_temp_dir();
    char tmp_exe[2048];
    snprintf(tmp_exe, sizeof(tmp_exe), "%s/elx_native_%p", tmp_dir, (void *)c_source);

    elx_native_result_t result;
    elx_native_status_t status = elx_native_compile_c(c_source, c_length, tmp_exe, config, &result);
    if (status != ELX_NATIVE_OK) {
        elx_native_result_destroy(&result);
        return status;
    }

    int ret = system(tmp_exe);
    if (out_exit_code) *out_exit_code = ret;
    remove(tmp_exe);
    elx_native_result_destroy(&result);
    return status;
}

elx_native_status_t elx_native_build(const char *project_dir, const char *target_name,
                                      bool release, elx_native_result_t *result) {
    (void)project_dir;
    (void)target_name;
    (void)release;
    (void)result;
    return ELX_NATIVE_ERROR;
}

void elx_native_result_destroy(elx_native_result_t *result) {
    if (!result) return;
    free(result->error_message);
    result->error_message = NULL;
}
