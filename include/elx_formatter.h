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

#ifndef ELX_FORMATTER_H
#define ELX_FORMATTER_H

#include "elx_config.h"
#include "elx_std.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_FMT_MAX_INDENT 8
#define ELX_FMT_MAX_LINE_LENGTH 200
#define ELX_FMT_MAX_OPTIONS 32

typedef enum {
    ELX_FMT_STYLE_ELX = 0,
    ELX_FMT_STYLE_COMPACT,
    ELX_FMT_STYLE_EXPANDED,
} elx_fmt_style_t;

typedef enum {
    ELX_FMT_QUOTE_DOUBLE = 0,
    ELX_FMT_QUOTE_SINGLE,
    ELX_FMT_QUOTE_BACKTICK,
} elx_fmt_quote_t;

typedef struct {
    size_t indent_size;
    bool use_tabs;
    bool trim_trailing_whitespace;
    bool ensure_final_newline;
    bool collapse_empty_lines;
    size_t max_line_length;
    elx_fmt_style_t style;
    elx_fmt_quote_t default_quote;
    char options[ELX_FMT_MAX_OPTIONS][64];
    size_t option_count;
} elx_fmt_config_t;

typedef struct {
    char *formatted;
    size_t length;
    size_t changes;
    bool success;
} elx_fmt_result_t;

const char *elx_formatter_version(void);

elx_fmt_config_t *elx_fmt_config_new(void);
void elx_fmt_config_free(elx_fmt_config_t *config);
bool elx_fmt_config_set_indent(elx_fmt_config_t *config, size_t size);
bool elx_fmt_config_set_use_tabs(elx_fmt_config_t *config, bool use_tabs);
bool elx_fmt_config_set_max_line_length(elx_fmt_config_t *config, size_t length);
bool elx_fmt_config_set_style(elx_fmt_config_t *config, elx_fmt_style_t style);
bool elx_fmt_config_set_quote(elx_fmt_config_t *config, elx_fmt_quote_t quote);

elx_fmt_result_t *elx_fmt_format(const char *source, size_t length,
                                     const elx_fmt_config_t *config);
elx_fmt_result_t *elx_fmt_format_file(const char *path, const elx_fmt_config_t *config);
elx_fmt_result_t *elx_fmt_format_range(const char *source, size_t length,
                                           size_t start, size_t end,
                                           const elx_fmt_config_t *config);
bool elx_fmt_format_in_place(char *source, size_t length, const elx_fmt_config_t *config);

bool elx_fmt_can_format(const char *source, size_t length);
size_t elx_fmt_line_count(const char *source, size_t length);
size_t elx_fmt_line_length(const char *source, size_t length, size_t line);
char *elx_fmt_extract_line(const char *source, size_t length, size_t line);
bool elx_fmt_check_style(const char *source, size_t length,
                             elx_fmt_config_t *config, size_t *violations);

#ifdef __cplusplus
}
#endif

#endif
