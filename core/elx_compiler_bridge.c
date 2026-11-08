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

/*
 * ELVI OS — VOLT 1.26 License Agreement (ELECTRON VISION)
 * See LICENSE-ELX at the project root for full terms.
 */
#include "elx_core.h"
#include "elx.h"
#include "elx_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void convert_errors(const elx_diagnostic_list_t *src, elx_error_list_t *dst) {
    if (!src || !dst) return;
    for (size_t i = 0; i < src->count; ++i) {
        elx_error_add(dst, src->items[i].code, src->items[i].message,
                      src->items[i].location.line, src->items[i].location.column);
    }
}

bool elx_sdk_compile(const elx_sdk_compile_request_t *request, elx_compile_result_t *result, elx_error_list_t *errors) {
    elx_diagnostic_list_t diag;
    elx_compile_options_t opts;
    elx_optimization_report_t report;
    bool ok;

    if (!request || !result) return false;
    if (errors) elx_error_init(errors);
    memset(result, 0, sizeof(*result));
    result->compiled = false;
    result->output_path = request->output_path;

    if (!request->source) {
        if (errors) elx_error_add(errors, 500, "no source provided", 0, 0);
        return false;
    }

    elx_diagnostics_init(&diag);
    opts.dump_tokens = request->dump_tokens;
    opts.dump_ast = request->dump_ast;
    opts.optimize = request->optimize;
    opts.module_path = NULL;
    opts.stdlib_path = NULL;
    opts.output_path = (char *)request->output_path;

    if (request->backend == ELX_BACKEND_NATIVE_C) {
        result->c_source = "/* ELX Native C backend not yet available at this target */\n";
        result->c_length = strlen(result->c_source);
        result->compiled = true;
        elx_diagnostics_destroy(&diag);
        return true;
    }

    ok = elx_compile_source(request->source, request->length,
                            request->output_path, &opts, &report, &diag);
    if (!ok || diag.has_errors) {
        convert_errors(&diag, errors);
    }
    if (ok && !diag.has_errors) {
        result->compiled = true;
        result->elbc_path = request->output_path;
    }
    elx_diagnostics_destroy(&diag);
    return ok && !diag.has_errors;
}

bool elx_sdk_compile_source(const char *source, size_t length, const char *output_path,
                            elx_target_t target, elx_backend_t backend, bool optimize,
                            elx_compile_result_t *result, elx_error_list_t *errors) {
    elx_sdk_compile_request_t req = {0};
    req.source = source;
    req.length = length;
    req.output_path = output_path;
    req.target = (elx_target_t)target;
    req.backend = (elx_backend_t)backend;
    req.optimize = optimize;
    return elx_sdk_compile(&req, result, errors);
}

bool elx_sdk_compile_file(const char *input_path, const char *output_path,
                          elx_target_t target, elx_backend_t backend, bool optimize,
                          elx_compile_result_t *result, elx_error_list_t *errors) {
    FILE *file = fopen(input_path, "rb");
    if (!file) {
        if (errors) elx_error_add(errors, 100, "cannot open input file", 1, 1);
        return false;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *source = (char *)malloc((size_t)size + 1);
    if (!source) { fclose(file); return false; }
    size_t read = fread(source, 1, (size_t)size, file);
    source[read] = '\0';
    fclose(file);
    bool ok = elx_sdk_compile_source(source, read, output_path, target, backend, optimize, result, errors);
    free(source);
    return ok;
}

bool elx_sdk_run(const char *elbc_path, int *exit_code) {
    elx_runtime_t runtime;
    if (!elx_runtime_create(&runtime, elbc_path)) {
        const char *err = elx_module_load_error();
        fprintf(stderr, "cannot load module '%s': %s\n", elbc_path,
                err ? err : "unknown error");
        return false;
    }
    int code = elx_runtime_run(&runtime, "main");
    if (exit_code) *exit_code = code;
    elx_runtime_destroy(&runtime);
    return true;
}
