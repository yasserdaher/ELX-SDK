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
#include "elx_formatter.h"
#include "elx_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char *elx_formatter_version(void) { return "0.1.0"; }

elx_fmt_config_t *elx_fmt_config_new(void) {
    elx_fmt_config_t *c = (elx_fmt_config_t *)calloc(1, sizeof(*c));
    if (c) {
        c->indent_size = 4;
        c->use_tabs = false;
        c->trim_trailing_whitespace = true;
        c->ensure_final_newline = true;
        c->collapse_empty_lines = true;
        c->max_line_length = ELX_FMT_MAX_LINE_LENGTH;
        c->style = ELX_FMT_STYLE_ELX;
    }
    return c;
}

void elx_fmt_config_free(elx_fmt_config_t *c) {
    free(c);
}

bool elx_fmt_config_set_indent(elx_fmt_config_t *c, size_t size) {
    if (!c) return false;
    c->indent_size = size;
    return true;
}

bool elx_fmt_config_set_use_tabs(elx_fmt_config_t *c, bool use_tabs) {
    if (!c) return false;
    c->use_tabs = use_tabs;
    return true;
}

bool elx_fmt_config_set_max_line_length(elx_fmt_config_t *c, size_t length) {
    if (!c) return false;
    c->max_line_length = length;
    return true;
}

bool elx_fmt_config_set_style(elx_fmt_config_t *c, elx_fmt_style_t style) {
    if (!c) return false;
    c->style = style;
    return true;
}

bool elx_fmt_config_set_quote(elx_fmt_config_t *c, elx_fmt_quote_t quote) {
    if (!c) return false;
    c->default_quote = quote;
    return true;
}

static char *indent_str(size_t level, size_t indent_size, bool use_tabs) {
    (void)level; (void)indent_size; (void)use_tabs;
    return NULL;
}

static elx_fmt_result_t *fmt_result(char *formatted, size_t length, size_t changes) {
    elx_fmt_result_t *r = (elx_fmt_result_t *)calloc(1, sizeof(*r));
    if (!r) return NULL;
    r->formatted = formatted;
    r->length = length;
    r->changes = changes;
    r->success = true;
    return r;
}

static bool is_block_opener(const char *line) {
    while (*line && isspace((unsigned char)*line)) line++;
    if (!*line) return false;
    if (strstr(line, "fn ") == line || strstr(line, "permissions") == line ||
        strstr(line, "import ") == line || strstr(line, "struct ") == line ||
        strstr(line, "enum ") == line || strstr(line, "service ") == line ||
        strstr(line, "{") != NULL || strstr(line, "(") != NULL) {
        return true;
    }
    return false;
}

elx_fmt_result_t *elx_fmt_format(const char *source, size_t length,
                                          const elx_fmt_config_t *config) {
    if (!source || length == 0) return NULL;
    char *out = (char *)malloc(length * 2);
    if (!out) return NULL;
    size_t out_len = 0;
    size_t changes = 0;
    size_t indent_level = 0;
    size_t line_start = 0;

    for (size_t i = 0; i <= length; i++) {
        char c = (i < length) ? source[i] : '\n';
        if (c == '\n') {
            size_t line_len = i - line_start;
            char line_buf[ELX_FMT_MAX_LINE_LENGTH + 1];
            if (line_len > ELX_FMT_MAX_LINE_LENGTH) line_len = ELX_FMT_MAX_LINE_LENGTH;
            memcpy(line_buf, source + line_start, line_len);
            line_buf[line_len] = '\0';

            /* Trim trailing whitespace */
            if (config && config->trim_trailing_whitespace) {
                while (line_len > 0 && isspace((unsigned char)line_buf[line_len - 1])) {
                    line_len--; changes++;
                }
            }

            /* Build indentation */
            size_t indent_len = indent_level * (config ? config->indent_size : 4);
            if (config && config->use_tabs) indent_len = indent_level;
            char *indent = (char *)calloc(indent_len + 1, 1);
            if (!indent) { free(out); return NULL; }
            if (config && config->use_tabs) {
                for (size_t j = 0; j < indent_level; j++) indent[j] = '\t';
            } else {
                for (size_t j = 0; j < indent_len; j++) indent[j] = ' ';
            }

            /* Check for block closing */
            bool has_close_brace = false;
            for (size_t j = 0; j < line_len; j++) {
                if (line_buf[j] == '}') { has_close_brace = true; break; }
            }

            if (has_close_brace && indent_level > 0) {
                indent_level = (indent_level > 0) ? indent_level - 1 : 0;
            }

            size_t needed = out_len + indent_len + line_len + 2;
            if (needed > length * 4) {
                char *n = (char *)realloc(out, needed);
                if (!n) { free(indent); free(out); return NULL; }
                out = n;
            }

            if (line_len > 0 || !config || !config->collapse_empty_lines) {
                memcpy(out + out_len, indent, indent_len);
                out_len += indent_len;
                memcpy(out + out_len, line_buf, line_len);
                out_len += line_len;
                out[out_len++] = '\n';
            } else {
                changes++;
            }
            free(indent);

            if (has_close_brace) {
                /* already decremented above */
            } else {
                if (is_block_opener(line_buf)) {
                    indent_level++;
                }
            }
            line_start = i + 1;
        }
    }

    if (config && config->ensure_final_newline && out_len > 0 && out[out_len - 1] != '\n') {
        out = (char *)realloc(out, out_len + 1);
        if (out) out[out_len++] = '\n';
    }

    return fmt_result(out, out_len, changes);
}

elx_fmt_result_t *elx_fmt_format_file(const char *path, const elx_fmt_config_t *config) {
    if (!path) return NULL;
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) { fclose(f); return NULL; }
    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);
    elx_fmt_result_t *r = elx_fmt_format(buf, (size_t)size, config);
    free(buf);
    return r;
}

elx_fmt_result_t *elx_fmt_format_range(const char *source, size_t length,
                                              size_t start, size_t end,
                                              const elx_fmt_config_t *config) {
    if (!source || start >= end || end > length) return NULL;
    size_t range_len = end - start;
    char *range = (char *)malloc(range_len + 1);
    if (!range) return NULL;
    memcpy(range, source + start, range_len);
    range[range_len] = '\0';
    elx_fmt_result_t *r = elx_fmt_format(range, range_len, config);
    free(range);
    return r;
}

bool elx_fmt_format_in_place(char *source, size_t length, const elx_fmt_config_t *config) {
    elx_fmt_result_t *r = elx_fmt_format(source, length, config);
    if (!r || !r->success) { free(r); return false; }
    memcpy(source, r->formatted, r->length > length ? length : r->length);
    free(r->formatted);
    free(r);
    return true;
}

bool elx_fmt_can_format(const char *source, size_t length) {
    (void)source; (void)length;
    return true;
}

size_t elx_fmt_line_count(const char *source, size_t length) {
    if (!source) return 0;
    size_t count = 1;
    for (size_t i = 0; i < length; i++) {
        if (source[i] == '\n') count++;
    }
    return count;
}

size_t elx_fmt_line_length(const char *source, size_t length, size_t line) {
    if (!source || line == 0) return 0;
    size_t current_line = 1;
    size_t start = 0;
    for (size_t i = 0; i < length; i++) {
        if (source[i] == '\n') {
            if (current_line == line) return i - start;
            current_line++;
            start = i + 1;
        }
    }
    if (current_line == line) return length - start;
    return 0;
}

char *elx_fmt_extract_line(const char *source, size_t length, size_t line) {
    if (!source || line == 0) return NULL;
    size_t current_line = 1;
    size_t start = 0;
    for (size_t i = 0; i < length; i++) {
        if (source[i] == '\n') {
            if (current_line == line) {
                size_t len = i - start;
                char *result = (char *)malloc(len + 1);
                if (result) {
                    memcpy(result, source + start, len);
                    result[len] = '\0';
                }
                return result;
            }
            current_line++;
            start = i + 1;
        }
    }
    if (current_line == line) {
        size_t len = length - start;
        char *result = (char *)malloc(len + 1);
        if (result) {
            memcpy(result, source + start, len);
            result[len] = '\0';
        }
        return result;
    }
    return NULL;
}

bool elx_fmt_check_style(const char *source, size_t length,
                                  elx_fmt_config_t *config, size_t *violations) {
    if (!source || !violations) return false;
    *violations = 0;
    size_t line = 1;
    size_t start = 0;
    for (size_t i = 0; i < length; i++) {
        if (source[i] == '\n') {
            size_t line_len = i - start;
            if (config && line_len > config->max_line_length) (*violations)++;
            if (config && config->trim_trailing_whitespace) {
                size_t end = i;
                while (end > start && isspace((unsigned char)source[end - 1])) { end--; (*violations)++; }
            }
            line++;
            start = i + 1;
        }
    }
    return true;
}
