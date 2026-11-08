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
#include "elx_diagnostics.h"
#include "elx_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *elx_diagnostics_version(void) { return "0.1.0"; }

elx_diag_code_t elx_diag_code_from_string(const char *string) {
    if (!string) return ELX_DIAG_CODE_UNKNOWN;
    if (strcmp(string, "parse") == 0) return ELX_DIAG_CODE_PARSE;
    if (strcmp(string, "type") == 0) return ELX_DIAG_CODE_TYPE;
    if (strcmp(string, "semantic") == 0) return ELX_DIAG_CODE_SEMANTIC;
    if (strcmp(string, "import") == 0) return ELX_DIAG_CODE_IMPORT;
    if (strcmp(string, "module") == 0) return ELX_DIAG_CODE_MODULE;
    if (strcmp(string, "permission") == 0) return ELX_DIAG_CODE_PERMISSION;
    if (strcmp(string, "compat") == 0) return ELX_DIAG_CODE_COMPAT;
    if (strcmp(string, "style") == 0) return ELX_DIAG_CODE_STYLE;
    if (strcmp(string, "lint") == 0) return ELX_DIAG_CODE_LINT;
    return ELX_DIAG_CODE_UNKNOWN;
}

const char *elx_diag_code_name(elx_diag_code_t code) {
    switch (code) {
        case ELX_DIAG_CODE_PARSE: return "parse";
        case ELX_DIAG_CODE_TYPE: return "type";
        case ELX_DIAG_CODE_SEMANTIC: return "semantic";
        case ELX_DIAG_CODE_IMPORT: return "import";
        case ELX_DIAG_CODE_MODULE: return "module";
        case ELX_DIAG_CODE_PERMISSION: return "permission";
        case ELX_DIAG_CODE_COMPAT: return "compat";
        case ELX_DIAG_CODE_STYLE: return "style";
        case ELX_DIAG_CODE_LINT: return "lint";
        default: return "unknown";
    }
}

const char *elx_diag_code_description(elx_diag_code_t code) {
    switch (code) {
        case ELX_DIAG_CODE_PARSE: return "Syntax parsing error";
        case ELX_DIAG_CODE_TYPE: return "Type mismatch or error";
        case ELX_DIAG_CODE_SEMANTIC: return "Semantic analysis error";
        case ELX_DIAG_CODE_IMPORT: return "Import resolution error";
        case ELX_DIAG_CODE_MODULE: return "Module loading error";
        case ELX_DIAG_CODE_PERMISSION: return "Permission violation";
        case ELX_DIAG_CODE_COMPAT: return "Compatibility issue";
        case ELX_DIAG_CODE_STYLE: return "Style violation";
        case ELX_DIAG_CODE_LINT: return "Linting issue";
        case ELX_DIAG_CODE_NAMING: return "Naming convention violation";
        default: return "Unknown error";
    }
}

const char *elx_diag_code_suggestion(elx_diag_code_t code) {
    switch (code) {
        case ELX_DIAG_CODE_PARSE: return "Check syntax near the error location";
        case ELX_DIAG_CODE_TYPE: return "Verify type annotations match usage";
        case ELX_DIAG_CODE_SEMANTIC: return "Review semantic context of the expression";
        case ELX_DIAG_CODE_IMPORT: return "Ensure module path is correct and module exists";
        case ELX_DIAG_CODE_MODULE: return "Check module manifest and permissions";
        case ELX_DIAG_CODE_PERMISSION: return "Request the required permission or adjust sandbox";
        case ELX_DIAG_CODE_COMPAT: return "Update dependency or check version constraints";
        case ELX_DIAG_CODE_STYLE: return "Run formatter to auto-fix";
        case ELX_DIAG_CODE_LINT: return "Review linting rules and fix violations";
        case ELX_DIAG_CODE_NAMING: return "Rename to follow naming conventions";
        default: return "Review code at the error location";
    }
}

elx_diag_severity_t elx_diag_severity_from_string(const char *string) {
    if (!string) return ELX_DIAG_ERROR;
    if (strcmp(string, "error") == 0) return ELX_DIAG_ERROR;
    if (strcmp(string, "warning") == 0) return ELX_DIAG_WARNING;
    if (strcmp(string, "info") == 0) return ELX_DIAG_INFO;
    if (strcmp(string, "hint") == 0) return ELX_DIAG_HINT;
    return ELX_DIAG_ERROR;
}

const char *elx_diag_severity_name(elx_diag_severity_t severity) {
    switch (severity) {
        case ELX_DIAG_WARNING: return "warning";
        case ELX_DIAG_INFO: return "info";
        case ELX_DIAG_HINT: return "hint";
        default: return "error";
    }
}

elx_diagnostics_t *elx_diagnostics_new(void) {
    elx_diagnostics_t *d = (elx_diagnostics_t *)calloc(1, sizeof(*d));
    return d;
}

void elx_diagnostics_free(elx_diagnostics_t *d) {
    if (!d) return;
    free(d->items);
    d->items = NULL;
    d->count = 0; d->capacity = 0;
}

bool elx_diagnostics_add(elx_diagnostics_t *d, const elx_diagnostic_t *entry) {
    if (!d || !entry) return false;
    if (d->count == d->capacity) {
        size_t cap = d->capacity ? d->capacity * 2 : 32;
        elx_diagnostic_t *items = (elx_diagnostic_t *)realloc(d->items, cap * sizeof(*items));
        if (!items) return false;
        d->items = items;
        d->capacity = cap;
    }
    d->items[d->count] = *entry;
    if (entry->has_errors) d->error_count++;
    if (entry->has_warnings) d->warning_count++;
    if (entry->severity == ELX_DIAG_INFO) d->info_count++;
    d->count++;
    return true;
}

elx_diagnostic_t *elx_diagnostics_get(const elx_diagnostics_t *d, size_t index) {
    if (!d || index >= d->count) return NULL;
    return &d->items[index];
}

size_t elx_diagnostics_count(const elx_diagnostics_t *d) { return d ? d->count : 0; }
size_t elx_diagnostics_error_count(const elx_diagnostics_t *d) { return d ? d->error_count : 0; }
size_t elx_diagnostics_warning_count(const elx_diagnostics_t *d) { return d ? d->warning_count : 0; }
size_t elx_diagnostics_info_count(const elx_diagnostics_t *d) { return d ? d->info_count : 0; }
bool elx_diagnostics_has_errors(const elx_diagnostics_t *d) { return d && d->error_count > 0; }
bool elx_diagnostics_has_warnings(const elx_diagnostics_t *d) { return d && d->warning_count > 0; }
bool elx_diagnostics_is_clean(const elx_diagnostics_t *d) { return d && d->error_count == 0 && d->warning_count == 0; }

void elx_diagnostics_clear(elx_diagnostics_t *d) {
    if (!d) return;
    free(d->items);
    d->items = NULL;
    d->count = 0; d->capacity = 0;
    d->error_count = 0; d->warning_count = 0; d->info_count = 0;
}

void elx_diagnostics_foreach(const elx_diagnostics_t *d,
                                  elx_diag_handler_t handler, void *user) {
    if (!d || !handler) return;
    for (size_t i = 0; i < d->count; i++) {
        handler(&d->items[i], user);
    }
}

static elx_diagnostic_t *alloc_diag(const char *file, size_t line, size_t column,
                                             elx_diag_code_t code,
                                             elx_diag_severity_t severity,
                                             const char *message) {
    elx_diagnostic_t *d = (elx_diagnostic_t *)calloc(1, sizeof(*d));
    if (!d) return NULL;
    d->code = code;
    d->severity = severity;
    d->location.file = file;
    d->location.line = line;
    d->location.column = column;
    d->location.end_line = line;
    d->location.end_column = column;
    if (message) {
        strncpy(d->message, message, ELX_DIAG_MAX_MESSAGE - 1);
    } else {
        strncpy(d->message, "unknown", ELX_DIAG_MAX_MESSAGE - 1);
    }
    d->has_errors = (severity == ELX_DIAG_ERROR);
    d->has_warnings = (severity == ELX_DIAG_WARNING);
    d->is_fatal = false;
    return d;
}

elx_diagnostic_t *elx_diag_create(const char *file, size_t line, size_t column,
                                         elx_diag_code_t code,
                                         elx_diag_severity_t severity,
                                         const char *message) {
    return alloc_diag(file, line, column, code, severity, message);
}

elx_diagnostic_t *elx_diag_create_error(const char *file, size_t line, size_t column,
                                                const char *message) {
    return alloc_diag(file, line, column, ELX_DIAG_CODE_UNKNOWN, ELX_DIAG_ERROR, message);
}

elx_diagnostic_t *elx_diag_create_warning(const char *file, size_t line, size_t column,
                                                const char *message) {
    return alloc_diag(file, line, column, ELX_DIAG_CODE_UNKNOWN, ELX_DIAG_WARNING, message);
}

elx_diagnostic_t *elx_diag_create_info(const char *file, size_t line, size_t column,
                                               const char *message) {
    return alloc_diag(file, line, column, ELX_DIAG_CODE_UNKNOWN, ELX_DIAG_INFO, message);
}

bool elx_diag_set_location(elx_diagnostic_t *d, const char *file, size_t line,
                                    size_t column, size_t end_line, size_t end_column) {
    if (!d) return false;
    d->location.file = file;
    d->location.line = line;
    d->location.column = column;
    d->location.end_line = end_line;
    d->location.end_column = end_column;
    return true;
}

bool elx_diag_set_snippet(elx_diagnostic_t *d, const char *source, size_t source_len,
                                 size_t start_line, size_t start_col, size_t end_line, size_t end_col) {
    if (!d || !source || source_len == 0) return false;
    char *line_start = (char *)source;
    for (size_t i = 1; i < start_line && *line_start; i++) {
        char *nl = strchr(line_start, '\n');
        if (nl) line_start = nl + 1; else break;
    }
    char *line_end = line_start;
    for (size_t i = start_line; i < end_line && *line_end; i++) {
        char *nl = strchr(line_end, '\n');
        if (nl) line_end = nl; else break;
    }
    size_t len = (size_t)(line_end - line_start);
    if (len >= ELX_DIAG_MAX_SNIPPET) len = ELX_DIAG_MAX_SNIPPET - 1;
    memcpy(d->snippet, line_start, len);
    d->snippet[len] = '\0';
    return true;
}

bool elx_diag_add_label(elx_diagnostic_t *d, const char *label) {
    if (!d || !label || d->label_count >= ELX_DIAG_MAX_LABELS) return false;
    strncpy(d->labels[d->label_count], label, 63);
    d->label_count++;
    return true;
}

bool elx_diag_add_note(elx_diagnostic_t *d, const char *label,
                              const char *message, const char *detail) {
    if (!d || !label || d->note_count >= ELX_DIAG_MAX_NOTES) return false;
    d->notes[d->note_count].label = label;
    d->notes[d->note_count].message = message;
    d->notes[d->note_count].detail = detail;
    d->note_count++;
    return true;
}

bool elx_diag_add_suggestion(elx_diagnostic_t *d, const char *label,
                                    const char *description) {
    if (!d || !label || d->suggestion_count >= ELX_DIAG_MAX_SUGGESTIONS) return false;
    d->suggestions[d->suggestion_count].label = label;
    d->suggestions[d->suggestion_count].description = description;
    d->suggestion_count++;
    return true;
}

bool elx_diag_mark_fatal(elx_diagnostic_t *d) {
    if (!d) return false;
    d->is_fatal = true;
    return true;
}

void elx_diag_print(const elx_diagnostic_t *d, FILE *stream) {
    if (!d || !stream) return;
    const char *sev = elx_diag_severity_name(d->severity);
    const char *code = elx_diag_code_name(d->code);
    fprintf(stream, "[%s %s:%zu:%zu] %s: %s\n", sev,
            d->location.file ? d->location.file : "<unknown>",
            d->location.line, d->location.column, code, d->message);
    if (d->snippet[0]) {
        fprintf(stream, "  snippet: %s\n", d->snippet);
    }
    for (size_t i = 0; i < d->note_count; i++) {
        fprintf(stream, "  note[%s]: %s", d->notes[i].label,
                d->notes[i].message ? d->notes[i].message : "");
        if (d->notes[i].detail) fprintf(stream, " — %s", d->notes[i].detail);
        fprintf(stream, "\n");
    }
    for (size_t i = 0; i < d->suggestion_count; i++) {
        fprintf(stream, "  suggestion[%s]: %s\n", d->suggestions[i].label,
                d->suggestions[i].description ? d->suggestions[i].description : "");
    }
}

void elx_sdk_diag_print(const elx_diagnostics_t *d, FILE *stream) {
    if (!d || !stream) return;
    fprintf(stream, "=== Diagnostics Report ===\n");
    fprintf(stream, "Total: %zu | Errors: %zu | Warnings: %zu | Info: %zu\n",
            d->count, d->error_count, d->warning_count, d->info_count);
    for (size_t i = 0; i < d->count; i++) {
        elx_diag_print(&d->items[i], stream);
    }
    fprintf(stream, "==========================\n");
}

void elx_sdk_diag_print_json(const elx_diagnostics_t *d, FILE *stream) {
    if (!d || !stream) return;
    fprintf(stream, "{\n  \"diagnostics\": [\n");
    for (size_t i = 0; i < d->count; i++) {
        const elx_diagnostic_t *diag = &d->items[i];
        fprintf(stream, "    {\n");
        fprintf(stream, "      \"code\": \"%s\",\n", elx_diag_code_name(diag->code));
        fprintf(stream, "      \"severity\": \"%s\",\n", elx_diag_severity_name(diag->severity));
        fprintf(stream, "      \"file\": \"%s\",\n", diag->location.file ? diag->location.file : "");
        fprintf(stream, "      \"line\": %zu,\n", diag->location.line);
        fprintf(stream, "      \"column\": %zu,\n", diag->location.column);
        fprintf(stream, "      \"message\": \"%s\",\n", diag->message);
        fprintf(stream, "      \"has_errors\": %s,\n", diag->has_errors ? "true" : "false");
        fprintf(stream, "      \"has_warnings\": %s,\n", diag->has_warnings ? "true" : "false");
        fprintf(stream, "      \"is_fatal\": %s\n", diag->is_fatal ? "true" : "false");
        fprintf(stream, "    }%s\n", i + 1 < d->count ? "," : "");
    }
    fprintf(stream, "  ]\n}\n");
}

bool elx_diagnostics_save(const elx_diagnostics_t *d, const char *path) {
    if (!d || !path) return false;
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    elx_sdk_diag_print_json(d, f);
    fclose(f);
    return true;
}

bool elx_diagnostics_load(elx_diagnostics_t *d, const char *path) {
    (void)d; (void)path;
    return false;
}

bool elx_sdk_diag_report(const elx_diagnostics_t *d) {
    if (!d) return false;
    elx_sdk_diag_print(d, stderr);
    return true;
}
