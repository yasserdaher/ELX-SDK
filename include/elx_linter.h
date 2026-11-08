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

#ifndef ELX_LINTER_H
#define ELX_LINTER_H

#include "elx_config.h"
#include "elx_std.h"
#include "elx_diagnostics.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_LINT_MAX_RULES 128
#define ELX_LINT_MAX_RULE_NAME 64
#define ELX_LINT_MAX_RULE_DESC 256

typedef enum {
    ELX_LINT_PARSE = 0,
    ELX_LINT_TYPE_CHECK,
    ELX_LINT_IMPORT_RESOLVE,
    ELX_LINT_PERMISSION_CHECK,
    ELX_LINT_STYLE_CHECK,
    ELX_LINT_DEAD_CODE,
    ELX_LINT_UNUSED,
    ELX_LINT_NAMING,
    ELX_LINT_COMPLEXITY,
    ELX_LINT_COMPAT,
} elx_lint_rule_t;

typedef enum {
    ELX_LINT_OFF = 0,
    ELX_LINT_WARN,
    ELX_LINT_ERROR,
} elx_lint_level_t;

typedef struct {
    char name[ELX_LINT_MAX_RULE_NAME];
    char description[ELX_LINT_MAX_RULE_DESC];
    elx_lint_rule_t rule;
    elx_lint_level_t default_level;
    bool enabled;
} elx_lint_rule_desc_t;

typedef struct {
    elx_lint_rule_desc_t rules[ELX_LINT_MAX_RULES];
    size_t rule_count;
    elx_lint_level_t global_level;
    size_t max_line_length;
    char exclude_patterns[32][256];
    size_t exclude_count;
    bool strict;
} elx_lint_config_t;

typedef struct {
    char *linted_source;
    size_t source_length;
    elx_diagnostics_t *diagnostics;
    size_t issues_found;
    bool success;
} elx_lint_result_t;

const char *elx_linter_version(void);

elx_lint_config_t *elx_lint_config_new(void);
void elx_lint_config_free(elx_lint_config_t *config);
bool elx_lint_config_enable_rule(elx_lint_config_t *config, elx_lint_rule_t rule,
                                     elx_lint_level_t level);
bool elx_lint_config_disable_rule(elx_lint_config_t *config, elx_lint_rule_t rule);
bool elx_lint_config_set_global_level(elx_lint_config_t *config, elx_lint_level_t level);
bool elx_lint_config_add_exclude(elx_lint_config_t *config, const char *pattern);
const elx_lint_rule_desc_t *elx_lint_config_get_rule(const elx_lint_config_t *config,
                                                        elx_lint_rule_t rule);

elx_lint_result_t *elx_lint_source(const char *source, size_t length,
                                       const elx_lint_config_t *config);
elx_lint_result_t *elx_lint_file(const char *path, const elx_lint_config_t *config);
elx_lint_result_t *elx_lint_project(const char *project_dir, const elx_lint_config_t *config);

bool elx_lint_rule_name(elx_lint_rule_t rule, char *name, size_t name_size);
elx_lint_rule_t elx_lint_rule_from_name(const char *name);
const char *elx_lint_rule_description(elx_lint_rule_t rule);

bool elx_lint_fix(const char *source, size_t length,
                      elx_lint_config_t *config, char **fixed, size_t *fixed_len);

#ifdef __cplusplus
}
#endif

#endif
