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
#include "elx_linter.h"
#include "elx_formatter.h"
#include "elx_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char *elx_linter_version(void) { return "0.1.0"; }

elx_lint_config_t *elx_lint_config_new(void) {
    elx_lint_config_t *c = (elx_lint_config_t *)calloc(1, sizeof(*c));
    if (c) {
        c->global_level = ELX_LINT_WARN;
        c->max_line_length = ELX_FMT_MAX_LINE_LENGTH;
        c->strict = false;
        for (int i = 0; i < ELX_LINT_MAX_RULES; i++) {
            c->rules[i].enabled = true;
            c->rules[i].default_level = ELX_LINT_WARN;
        }
    }
    return c;
}

void elx_lint_config_free(elx_lint_config_t *c) {
    free(c);
}

bool elx_lint_config_enable_rule(elx_lint_config_t *c, elx_lint_rule_t rule,
                                              elx_lint_level_t level) {
    if (!c || rule >= ELX_LINT_MAX_RULES) return false;
    c->rules[rule].enabled = true;
    c->rules[rule].default_level = level;
    return true;
}

bool elx_lint_config_disable_rule(elx_lint_config_t *c, elx_lint_rule_t rule) {
    if (!c || rule >= ELX_LINT_MAX_RULES) return false;
    c->rules[rule].enabled = false;
    return true;
}

bool elx_lint_config_set_global_level(elx_lint_config_t *c, elx_lint_level_t level) {
    if (!c) return false;
    c->global_level = level;
    return true;
}

bool elx_lint_config_add_exclude(elx_lint_config_t *c, const char *pattern) {
    if (!c || !pattern || c->exclude_count >= 32) return false;
    strncpy(c->exclude_patterns[c->exclude_count], pattern, 255);
    c->exclude_count++;
    return true;
}

const elx_lint_rule_desc_t *elx_lint_config_get_rule(const elx_lint_config_t *c,
                                                                elx_lint_rule_t rule) {
    if (!c || rule >= ELX_LINT_MAX_RULES) return NULL;
    return &c->rules[rule];
}

bool elx_lint_rule_name(elx_lint_rule_t rule, char *name, size_t name_size) {
    if (!name || name_size == 0) return false;
    const char *n;
    switch (rule) {
        case ELX_LINT_PARSE: n = "parse"; break;
        case ELX_LINT_TYPE_CHECK: n = "type-check"; break;
        case ELX_LINT_IMPORT_RESOLVE: n = "import-resolve"; break;
        case ELX_LINT_PERMISSION_CHECK: n = "permission-check"; break;
        case ELX_LINT_STYLE_CHECK: n = "style-check"; break;
        case ELX_LINT_DEAD_CODE: n = "dead-code"; break;
        case ELX_LINT_UNUSED: n = "unused"; break;
        case ELX_LINT_NAMING: n = "naming"; break;
        case ELX_LINT_COMPLEXITY: n = "complexity"; break;
        case ELX_LINT_COMPAT: n = "compat"; break;
        default: n = "unknown"; break;
    }
    strncpy(name, n, name_size - 1);
    name[name_size - 1] = '\0';
    return true;
}

elx_lint_rule_t elx_lint_rule_from_name(const char *name) {
    if (!name) return ELX_LINT_PARSE;
    if (strcmp(name, "parse") == 0) return ELX_LINT_PARSE;
    if (strcmp(name, "type-check") == 0) return ELX_LINT_TYPE_CHECK;
    if (strcmp(name, "import-resolve") == 0) return ELX_LINT_IMPORT_RESOLVE;
    if (strcmp(name, "permission-check") == 0) return ELX_LINT_PERMISSION_CHECK;
    if (strcmp(name, "style-check") == 0) return ELX_LINT_STYLE_CHECK;
    if (strcmp(name, "dead-code") == 0) return ELX_LINT_DEAD_CODE;
    if (strcmp(name, "unused") == 0) return ELX_LINT_UNUSED;
    if (strcmp(name, "naming") == 0) return ELX_LINT_NAMING;
    if (strcmp(name, "complexity") == 0) return ELX_LINT_COMPLEXITY;
    if (strcmp(name, "compat") == 0) return ELX_LINT_COMPAT;
    return ELX_LINT_PARSE;
}

const char *elx_lint_rule_description(elx_lint_rule_t rule) {
    switch (rule) {
        case ELX_LINT_PARSE: return "Parse validation";
        case ELX_LINT_TYPE_CHECK: return "Type checking";
        case ELX_LINT_IMPORT_RESOLVE: return "Import resolution";
        case ELX_LINT_PERMISSION_CHECK: return "Permission validation";
        case ELX_LINT_STYLE_CHECK: return "Style conformance";
        case ELX_LINT_DEAD_CODE: return "Dead code detection";
        case ELX_LINT_UNUSED: return "Unused detection";
        case ELX_LINT_NAMING: return "Naming conventions";
        case ELX_LINT_COMPLEXITY: return "Complexity analysis";
        case ELX_LINT_COMPAT: return "Compatibility check";
        default: return "Unknown";
    }
}

static bool is_excluded(const elx_lint_config_t *config, const char *path) {
    (void)config; (void)path;
    return false;
}

static void lint_add_diagnostic(elx_diagnostics_t *diag, const char *file,
                                        size_t line, elx_diag_code_t code,
                                        const char *message) {
    elx_diagnostic_t *d = elx_diag_create(file, line, 0, code,
                                                    ELX_DIAG_ERROR, message);
    if (d) {
        elx_diagnostics_add(diag, d);
        free(d);
    }
}

elx_lint_result_t *elx_lint_source(const char *source, size_t length,
                                             const elx_lint_config_t *config) {
    if (!source || !length) return NULL;
    elx_lint_result_t *r = (elx_lint_result_t *)calloc(1, sizeof(*r));
    if (!r) return NULL;
    r->diagnostics = elx_diagnostics_new();
    r->linted_source = (char *)malloc(length + 1);
    if (!r->linted_source) {
        free(r);
        return NULL;
    }
    memcpy(r->linted_source, source, length);
    r->linted_source[length] = '\0';
    r->source_length = length;

    size_t line = 1;
    size_t start = 0;
    for (size_t i = 0; i < length; i++) {
        if (source[i] == '\n') {
            size_t line_len = i - start;
            char line_buf[ELX_FMT_MAX_LINE_LENGTH + 1];
            if (line_len > ELX_FMT_MAX_LINE_LENGTH)
                line_len = ELX_FMT_MAX_LINE_LENGTH;
            memcpy(line_buf, source + start, line_len);
            line_buf[line_len] = '\0';

            if (config && config->rules[ELX_LINT_STYLE_CHECK].enabled) {
                if (line_len > config->max_line_length) {
                    lint_add_diagnostic(r->diagnostics, "<source>", line,
                                       ELX_DIAG_CODE_STYLE,
                                       "Line exceeds max length");
                }
            }

            if (config && config->rules[ELX_LINT_PARSE].enabled) {
                if (line_buf[0] == '}' && line_len == 1) {
                    lint_add_diagnostic(r->diagnostics, "<source>", line,
                                       ELX_DIAG_CODE_PARSE,
                                       "Unexpected closing brace");
                }
            }

            if (config && config->rules[ELX_LINT_NAMING].enabled) {
                if (strstr(line_buf, "fn ") == line_buf) {
                    char *fn_start = line_buf + 3;
                    while (*fn_start && isspace((unsigned char)*fn_start)) fn_start++;
                    if (*fn_start && isupper((unsigned char)*fn_start)) {
                        lint_add_diagnostic(r->diagnostics, "<source>", line,
                                           ELX_DIAG_CODE_NAMING,
                                           "Function name should start with lowercase");
                    }
                }
            }

            line++;
            start = i + 1;
        }
    }
    r->issues_found = elx_diagnostics_error_count(r->diagnostics) +
                      elx_diagnostics_warning_count(r->diagnostics);
    r->success = r->issues_found == 0;
    return r;
}

elx_lint_result_t *elx_lint_file(const char *path, const elx_lint_config_t *config) {
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
    elx_lint_result_t *r = elx_lint_source(buf, (size_t)size, config);
    free(buf);
    return r;
}

elx_lint_result_t *elx_lint_project(const char *project_dir,
                                            const elx_lint_config_t *config) {
    (void)project_dir; (void)config;
    return NULL;
}

bool elx_lint_fix(const char *source, size_t length,
                          elx_lint_config_t *config, char **fixed, size_t *fixed_len) {
    (void)config;
    if (!source || !fixed || !fixed_len) return false;
    *fixed = (char *)malloc(length + 1);
    if (!*fixed) return false;
    memcpy(*fixed, source, length);
    (*fixed)[length] = '\0';
    *fixed_len = length;
    return true;
}
