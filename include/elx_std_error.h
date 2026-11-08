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

#ifndef ELX_STD_ERROR_H
#define ELX_STD_ERROR_H

#include "elx_std.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ELX_ERR_SUCCESS = 0,
    ELX_ERR_UNKNOWN = 1,
    ELX_ERR_INVALID_ARGUMENT,
    ELX_ERR_OUT_OF_MEMORY,
    ELX_ERR_IO,
    ELX_ERR_PARSE,
    ELX_ERR_COMPILE,
    ELX_ERR_RUNTIME,
    ELX_ERR_PERMISSION,
    ELX_ERR_NOT_FOUND,
    ELX_ERR_ALREADY_EXISTS,
    ELX_ERR_TIMEOUT,
    ELX_ERR_CANCELLED,
    ELX_ERR_NETWORK,
    ELX_ERR_NETWORK_CONNECT,
    ELX_ERR_NETWORK_TIMEOUT,
    ELX_ERR_NETWORK_PROTOCOL,
    ELX_ERR_NETWORK_DNS,
    ELX_ERR_FILE_NOT_FOUND,
    ELX_ERR_FILE_PERMISSION,
    ELX_ERR_FILE_CORRUPT,
    ELX_ERR_FILE_ALREADY_EXISTS,
    ELX_ERR_MATH_DIVISION_BY_ZERO,
    ELX_ERR_MATH_OVERFLOW,
    ELX_ERR_MATH_UNDERFLOW,
    ELX_ERR_MATH_DOMAIN,
    ELX_ERR_TYPE_MISMATCH,
    ELX_ERR_TYPE_CONVERSION,
    ELX_ERR_NULL_POINTER,
    ELX_ERR_BUFFER_OVERFLOW,
    ELX_ERR_STACK_OVERFLOW,
    ELX_ERR_OUT_OF_RANGE,
    ELX_ERR_LOCKED,
    ELX_ERR_DEADLOCK,
    ELX_ERR_UNIMPLEMENTED,
    ELX_ERR_INTERNAL,
    ELX_ERR_LAST
} elx_error_code_t;

typedef struct {
    elx_error_code_t code;
    const char *message;
    const char *domain;
    size_t line;
    size_t column;
    const char *file;
    const char *function;
    void *context;
} elx_error_t;

typedef void (*elx_error_handler_t)(const elx_error_t *error, void *user_data);

elx_error_t *elx_error_new(elx_error_code_t code, const char *message);
elx_error_t *elx_error_from_errno(int errno_val);
elx_error_t *elx_error_dup(const elx_error_t *src);
void elx_error_free(elx_error_t *err);
void elx_error_set_message(elx_error_t *err, const char *message);
void elx_error_set_context(elx_error_t *err, void *context);
void elx_error_set_location(elx_error_t *err, const char *file, size_t line, size_t column, const char *function);

const char *elx_error_code_name(elx_error_code_t code);
const char *elx_error_code_message(elx_error_code_t code);
elx_error_code_t elx_error_code_from_string(const char *name);

bool elx_error_is_success(const elx_error_t *err);
bool elx_error_is_failure(const elx_error_t *err);
bool elx_error_is_retryable(const elx_error_t *err);

elx_error_t *elx_error_chain_parent(elx_error_t *err, elx_error_t *parent);
elx_error_t *elx_error_chain_cause(elx_error_t *err, elx_error_t *cause);

typedef struct {
    elx_error_t *errors;
    size_t count;
    size_t capacity;
} elx_error_list_t;

elx_error_list_t *elx_error_list_new(void);
void elx_error_list_free(elx_error_list_t *list);
bool elx_error_list_add(elx_error_list_t *list, elx_error_t *error);
elx_error_t *elx_error_list_get(const elx_error_list_t *list, size_t index);
size_t elx_error_list_count(const elx_error_list_t *list);
bool elx_error_list_has_error(const elx_error_list_t *list);
bool elx_error_list_has_code(const elx_error_list_t *list, elx_error_code_t code);
elx_error_list_t *elx_error_list_filter(const elx_error_list_t *list, elx_error_code_t code);
void elx_error_list_foreach(const elx_error_list_t *list, elx_error_handler_t handler, void *user_data);
elx_error_t *elx_error_list_first(const elx_error_list_t *list);
elx_error_t *elx_error_list_last(const elx_error_list_t *list);
void elx_error_list_print(const elx_error_list_t *list, FILE *stream);
void elx_error_list_clear(elx_error_list_t *list);

void elx_error_set_handler(elx_error_handler_t handler, void *user_data);
void elx_error_report(const elx_error_t *err);
void elx_error_reset(void);

typedef struct {
    size_t total;
    size_t failed;
    size_t skipped;
    double duration_ms;
} elx_test_result_t;

elx_test_result_t *elx_test_result_new(void);
void elx_test_result_free(elx_test_result_t *r);
void elx_test_result_record(elx_test_result_t *r, bool passed);
bool elx_test_result_all_passed(const elx_test_result_t *r);
const char *elx_test_result_summary(const elx_test_result_t *r);

const char *elx_error_version(void);

#ifdef __cplusplus
}
#endif

#endif
