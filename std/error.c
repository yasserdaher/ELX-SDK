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

#include "elx_std_error.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

const char *elx_error_version(void) { return "0.1.0"; }

static const char *error_code_names[] = {
    "Success", "Unknown", "InvalidArgument", "OutOfMemory", "IO", "Parse",
    "Compile", "Runtime", "Permission", "NotFound", "AlreadyExists", "Timeout",
    "Cancelled", "Network", "NetworkConnect", "NetworkTimeout", "NetworkProtocol",
    "NetworkDNS", "FileNotFound", "FilePermission", "FileCorrupt", "FileAlreadyExists",
    "MathDivisionByZero", "MathOverflow", "MathUnderflow", "MathDomain",
    "TypeMismatch", "TypeConversion", "NullPointer", "BufferOverflow", "StackOverflow",
    "OutOfRange", "Locked", "Deadlock", "Unimplemented", "Internal"
};

static const char *error_code_messages[] = {
    "Success", "Unknown error", "Invalid argument", "Out of memory", "I/O error",
    "Parse error", "Compile error", "Runtime error", "Permission denied", "Not found",
    "Already exists", "Timeout", "Cancelled", "Network error", "Connection failed",
    "Network timeout", "Protocol error", "DNS resolution failed", "File not found",
    "Permission denied", "File corrupt", "File already exists", "Division by zero",
    "Math overflow", "Math underflow", "Math domain error", "Type mismatch",
    "Type conversion failed", "Null pointer", "Buffer overflow", "Stack overflow",
    "Out of range", "Resource locked", "Deadlock detected", "Not implemented",
    "Internal error"
};

const char *elx_error_code_name(elx_error_code_t code) {
    if (code < 0 || code >= ELX_ERR_LAST) return "Unknown";
    return error_code_names[code];
}

const char *elx_error_code_message(elx_error_code_t code) {
    if (code < 0 || code >= ELX_ERR_LAST) return "Unknown error";
    return error_code_messages[code];
}

elx_error_code_t elx_error_code_from_string(const char *name) {
    for (int i = 0; i < ELX_ERR_LAST; i++) {
        if (strcmp(error_code_names[i], name) == 0) return (elx_error_code_t)i;
    }
    return ELX_ERR_UNKNOWN;
}

elx_error_t *elx_error_new(elx_error_code_t code, const char *message) {
    elx_error_t *e = (elx_error_t *)calloc(1, sizeof(elx_error_t));
    if (!e) return NULL;
    e->code = code;
    e->message = message;
    return e;
}

elx_error_t *elx_error_from_errno(int errno_val) {
    elx_error_t *e = (elx_error_t *)calloc(1, sizeof(elx_error_t));
    if (!e) return NULL;
    if (errno_val == ENOENT) e->code = ELX_ERR_FILE_NOT_FOUND;
    else if (errno_val == EACCES || errno_val == EPERM) e->code = ELX_ERR_FILE_PERMISSION;
    else if (errno_val == EEXIST) e->code = ELX_ERR_FILE_ALREADY_EXISTS;
    else if (errno_val == EIO) e->code = ELX_ERR_IO;
    else if (errno_val == ENOMEM || errno_val == ENOBUFS) e->code = ELX_ERR_OUT_OF_MEMORY;
    else if (errno_val == ETIMEDOUT) e->code = ELX_ERR_TIMEOUT;
    else e->code = ELX_ERR_IO;
    e->message = strerror(errno_val);
    return e;
}

elx_error_t *elx_error_dup(const elx_error_t *src) {
    if (!src) return NULL;
    elx_error_t *e = (elx_error_t *)calloc(1, sizeof(elx_error_t));
    if (!e) return NULL;
    *e = *src;
    return e;
}

void elx_error_free(elx_error_t *err) { free(err); }

void elx_error_set_message(elx_error_t *err, const char *message) {
    if (err) err->message = message;
}

void elx_error_set_context(elx_error_t *err, void *context) {
    if (err) err->context = context;
}

void elx_error_set_location(elx_error_t *err, const char *file, size_t line, size_t column, const char *function) {
    if (!err) return;
    err->file = file;
    err->line = line;
    err->column = column;
    err->function = function;
}

bool elx_error_is_success(const elx_error_t *err) { return !err || err->code == ELX_ERR_SUCCESS; }
bool elx_error_is_failure(const elx_error_t *err) { return err && err->code != ELX_ERR_SUCCESS; }
bool elx_error_is_retryable(const elx_error_t *err) {
    if (!err) return false;
    return err->code == ELX_ERR_IO || err->code == ELX_ERR_NETWORK ||
           err->code == ELX_ERR_NETWORK_TIMEOUT || err->code == ELX_ERR_TIMEOUT ||
           err->code == ELX_ERR_LOCKED;
}

elx_error_t *elx_error_chain_parent(elx_error_t *err, elx_error_t *parent) {
    (void)err; (void)parent;
    return parent;
}

elx_error_t *elx_error_chain_cause(elx_error_t *err, elx_error_t *cause) {
    (void)err; (void)cause;
    return cause;
}

elx_error_list_t *elx_error_list_new(void) {
    elx_error_list_t *l = (elx_error_list_t *)calloc(1, sizeof(elx_error_list_t));
    if (!l) return NULL;
    l->capacity = 16;
    l->errors = (elx_error_t *)calloc(l->capacity, sizeof(elx_error_t));
    if (!l->errors) { free(l); return NULL; }
    return l;
}

void elx_error_list_free(elx_error_list_t *list) {
    if (!list) return;
    elx_error_list_clear(list);
    free(list->errors);
    free(list);
}

bool elx_error_list_add(elx_error_list_t *list, elx_error_t *error) {
    if (!list || !error) return false;
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        elx_error_t *ne = (elx_error_t *)realloc(list->errors, list->capacity * sizeof(elx_error_t));
        if (!ne) return false;
        list->errors = ne;
    }
    list->errors[list->count++] = *error;
    return true;
}

elx_error_t *elx_error_list_get(const elx_error_list_t *list, size_t index) {
    if (!list || index >= list->count) return NULL;
    return &list->errors[index];
}

size_t elx_error_list_count(const elx_error_list_t *list) { return list ? list->count : 0; }
bool elx_error_list_has_error(const elx_error_list_t *list) { return list && list->count > 0; }

bool elx_error_list_has_code(const elx_error_list_t *list, elx_error_code_t code) {
    if (!list) return false;
    for (size_t i = 0; i < list->count; i++) {
        if (list->errors[i].code == code) return true;
    }
    return false;
}

elx_error_list_t *elx_error_list_filter(const elx_error_list_t *list, elx_error_code_t code) {
    if (!list) return NULL;
    elx_error_list_t *result = elx_error_list_new();
    if (!result) return NULL;
    for (size_t i = 0; i < list->count; i++) {
        if (list->errors[i].code == code) {
            if (!elx_error_list_add(result, &list->errors[i])) {
                elx_error_list_free(result);
                return NULL;
            }
        }
    }
    return result;
}

void elx_error_list_foreach(const elx_error_list_t *list, elx_error_handler_t handler, void *user_data) {
    if (!list || !handler) return;
    for (size_t i = 0; i < list->count; i++) {
        handler(&list->errors[i], user_data);
    }
}

elx_error_t *elx_error_list_first(const elx_error_list_t *list) {
    if (!list || list->count == 0) return NULL;
    return &list->errors[0];
}

elx_error_t *elx_error_list_last(const elx_error_list_t *list) {
    if (!list || list->count == 0) return NULL;
    return &list->errors[list->count - 1];
}

void elx_error_list_print(const elx_error_list_t *list, FILE *stream) {
    if (!stream) stream = stderr;
    if (!list) return;
    for (size_t i = 0; i < list->count; i++) {
        const elx_error_t *e = &list->errors[i];
        fprintf(stream, "[ELX-ERR] %s (%s)\n", elx_error_code_name(e->code), e->message ? e->message : "(null)");
        if (e->file && e->line > 0) {
            fprintf(stream, "  at %s:%zu:%zu\n", e->file, e->line, e->column);
        }
    }
}

void elx_error_list_clear(elx_error_list_t *list) {
    if (!list) return;
    list->count = 0;
}

void *g_elx_error_user = NULL;
elx_error_handler_t g_elx_error_handler = NULL;

void elx_error_set_handler(elx_error_handler_t handler, void *user_data) {
    g_elx_error_handler = handler;
    g_elx_error_user = user_data;
}

void elx_error_report(const elx_error_t *err) {
    if (!err) return;
    fprintf(stderr, "[ELX-ERR] %s (%s)\n", elx_error_code_name(err->code), err->message ? err->message : "(null)");
    if (err->file && err->line > 0) {
        fprintf(stderr, "  at %s:%zu:%zu in %s\n", err->file, err->line, err->column, err->function ? err->function : "(unknown)");
    }
    if (g_elx_error_handler) {
        g_elx_error_handler(err, g_elx_error_user);
    }
}

void elx_error_reset(void) {
    g_elx_error_handler = NULL;
    g_elx_error_user = NULL;
}

elx_test_result_t *elx_test_result_new(void) {
    elx_test_result_t *r = (elx_test_result_t *)calloc(1, sizeof(elx_test_result_t));
    return r;
}

void elx_test_result_free(elx_test_result_t *r) { free(r); }

void elx_test_result_record(elx_test_result_t *r, bool passed) {
    if (!r) return;
    r->total++;
    if (!passed) r->failed++;
}

bool elx_test_result_all_passed(const elx_test_result_t *r) {
    return !r || r->failed == 0;
}

const char *elx_test_result_summary(const elx_test_result_t *r) {
    static char buf[128];
    if (!r) return "No tests";
    snprintf(buf, sizeof(buf), "%zu total, %zu passed, %zu failed", r->total, r->total - r->failed, r->failed);
    return buf;
}
