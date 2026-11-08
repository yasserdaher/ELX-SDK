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

#ifndef ELX_DIAGNOSTICS_H
#define ELX_DIAGNOSTICS_H

#include "elx_config.h"
#include "elx_std.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_DIAG_MAX_CODE 64
#define ELX_DIAG_MAX_MESSAGE 512
#define ELX_DIAG_MAX_SNIPPET 2048
#define ELX_DIAG_MAX_SUGGESTIONS 16
#define ELX_DIAG_MAX_LABELS 8
#define ELX_DIAG_MAX_NOTES 16
#define ELX_DIAG_MAX_SOURCE_PATH 4096

typedef enum {
    ELX_DIAG_ERROR = 0,
    ELX_DIAG_WARNING,
    ELX_DIAG_INFO,
    ELX_DIAG_HINT,
} elx_diag_severity_t;

typedef enum {
    ELX_DIAG_CODE_PARSE = 1000,
    ELX_DIAG_CODE_TYPE,
    ELX_DIAG_CODE_SEMANTIC,
    ELX_DIAG_CODE_IMPORT,
    ELX_DIAG_CODE_MODULE,
    ELX_DIAG_CODE_PERMISSION,
    ELX_DIAG_CODE_COMPAT,
    ELX_DIAG_CODE_STYLE,
    ELX_DIAG_CODE_LINT,
    ELX_DIAG_CODE_NAMING,
    ELX_DIAG_CODE_UNKNOWN,
} elx_diag_code_t;

typedef struct {
    const char *label;
    const char *message;
    const char *detail;
} elx_diag_note_t;

typedef struct {
    const char *label;
    const char *description;
} elx_diag_suggestion_t;

typedef struct {
    const char *file;
    size_t line;
    size_t column;
    size_t end_line;
    size_t end_column;
} elx_diag_location_t;

typedef struct {
    elx_diag_code_t code;
    elx_diag_severity_t severity;
    elx_diag_location_t location;
    char message[ELX_DIAG_MAX_MESSAGE];
    char snippet[ELX_DIAG_MAX_SNIPPET];
    char source_path[ELX_DIAG_MAX_SOURCE_PATH];
    char labels[ELX_DIAG_MAX_LABELS][64];
    size_t label_count;
    elx_diag_note_t notes[ELX_DIAG_MAX_NOTES];
    size_t note_count;
    elx_diag_suggestion_t suggestions[ELX_DIAG_MAX_SUGGESTIONS];
    size_t suggestion_count;
    bool has_errors;
    bool has_warnings;
    bool is_fatal;
} elx_diagnostic_t;

typedef struct {
    elx_diagnostic_t *items;
    size_t count;
    size_t capacity;
    size_t error_count;
    size_t warning_count;
    size_t info_count;
} elx_diagnostics_t;

typedef void (*elx_diag_handler_t)(const elx_diagnostic_t *diag, void *user);

const char *elx_diagnostics_version(void);

elx_diag_code_t elx_diag_code_from_string(const char *string);
const char *elx_diag_code_name(elx_diag_code_t code);
const char *elx_diag_code_description(elx_diag_code_t code);
const char *elx_diag_code_suggestion(elx_diag_code_t code);
elx_diag_severity_t elx_diag_severity_from_string(const char *string);
const char *elx_diag_severity_name(elx_diag_severity_t severity);

elx_diagnostics_t *elx_diagnostics_new(void);
void elx_diagnostics_free(elx_diagnostics_t *diag);

bool elx_diagnostics_add(elx_diagnostics_t *diag, const elx_diagnostic_t *entry);
elx_diagnostic_t *elx_diagnostics_get(const elx_diagnostics_t *diag, size_t index);
size_t elx_diagnostics_count(const elx_diagnostics_t *diag);
size_t elx_diagnostics_error_count(const elx_diagnostics_t *diag);
size_t elx_diagnostics_warning_count(const elx_diagnostics_t *diag);
size_t elx_diagnostics_info_count(const elx_diagnostics_t *diag);
bool elx_diagnostics_has_errors(const elx_diagnostics_t *diag);
bool elx_diagnostics_has_warnings(const elx_diagnostics_t *diag);
bool elx_diagnostics_is_clean(const elx_diagnostics_t *diag);
void elx_diagnostics_clear(elx_diagnostics_t *diag);
void elx_diagnostics_foreach(const elx_diagnostics_t *diag,
                                  elx_diag_handler_t handler, void *user);

elx_diagnostic_t *elx_diag_create(const char *file, size_t line, size_t column,
                                      elx_diag_code_t code,
                                      elx_diag_severity_t severity,
                                      const char *message);
elx_diagnostic_t *elx_diag_create_error(const char *file, size_t line, size_t column,
                                           const char *message);
elx_diagnostic_t *elx_diag_create_warning(const char *file, size_t line, size_t column,
                                              const char *message);
elx_diagnostic_t *elx_diag_create_info(const char *file, size_t line, size_t column,
                                          const char *message);

bool elx_diag_set_location(elx_diagnostic_t *diag, const char *file, size_t line,
                               size_t column, size_t end_line, size_t end_column);
bool elx_diag_set_snippet(elx_diagnostic_t *diag, const char *source, size_t source_len,
                              size_t start_line, size_t start_col, size_t end_line, size_t end_col);
bool elx_diag_add_label(elx_diagnostic_t *diag, const char *label);
bool elx_diag_add_note(elx_diagnostic_t *diag, const char *label,
                           const char *message, const char *detail);
bool elx_diag_add_suggestion(elx_diagnostic_t *diag, const char *label,
                                 const char *description);
bool elx_diag_mark_fatal(elx_diagnostic_t *diag);

void elx_diag_print(const elx_diagnostic_t *diag, FILE *stream);
void elx_sdk_diag_print(const elx_diagnostics_t *diag, FILE *stream);
void elx_sdk_diag_print_json(const elx_diagnostics_t *diag, FILE *stream);
bool elx_diagnostics_save(const elx_diagnostics_t *diag, const char *path);
bool elx_diagnostics_load(elx_diagnostics_t *diag, const char *path);
bool elx_sdk_diag_report(const elx_diagnostics_t *diag);

#ifdef __cplusplus
}
#endif

#endif
