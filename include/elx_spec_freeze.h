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

#ifndef ELX_SPEC_FREEZE_H
#define ELX_SPEC_FREEZE_H

#include "elx_config.h"
#include "elx_std.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_FREEZE_MAX_SPEC 1024
#define ELX_FREEZE_MAX_VERSION 32
#define ELX_FREEZE_MAX_CHANGELOG 4096
#define ELX_FREEZE_MAX_RULES 256
#define ELX_FREEZE_MAX_FEATURES 128

typedef enum {
    ELX_FREEZE_STATE_DRAFT = 0,
    ELX_FREEZE_STATE_REVIEW,
    ELX_FREEZE_STATE_FROZEN,
    ELX_FREEZE_STATE_DEPRECATED,
} elx_freeze_state_t;

typedef enum {
    ELX_FREEZE_CHANGE_NONE = 0,
    ELX_FREEZE_CHANGE_ADDITION,
    ELX_FREEZE_CHANGE_MODIFICATION,
    ELX_FREEZE_CHANGE_REMOVAL,
    ELX_FREEZE_CHANGE_DEPRECATION,
} elx_freeze_change_t;

typedef struct {
    char feature[ELX_FREEZE_MAX_SPEC];
    elx_freeze_change_t change_type;
    char description[ELX_FREEZE_MAX_SPEC];
    char migration_guide[ELX_FREEZE_MAX_SPEC];
    bool breaking;
} elx_freeze_change_entry_t;

typedef struct {
    char spec[ELX_FREEZE_MAX_SPEC];
    char version[ELX_FREEZE_MAX_VERSION];
    char date[32];
    char changelog[ELX_FREEZE_MAX_CHANGELOG];
    elx_freeze_state_t state;
    elx_freeze_change_entry_t changes[ELX_FREEZE_MAX_CHANGELOG];
    size_t change_count;
    char features[ELX_FREEZE_MAX_FEATURES][ELX_FREEZE_MAX_SPEC];
    size_t feature_count;
    bool locked;
} elx_freeze_spec_t;

typedef struct {
    char feature[ELX_FREEZE_MAX_SPEC];
    char syntax[ELX_FREEZE_MAX_SPEC];
    char semantics[ELX_FREEZE_MAX_SPEC];
    bool is_syntax;
    bool is_semantic;
} elx_freeze_rule_t;

typedef struct {
    elx_freeze_rule_t rules[ELX_FREEZE_MAX_RULES];
    size_t rule_count;
    elx_freeze_spec_t current_spec;
    elx_freeze_spec_t prev_spec;
    bool freeze_active;
} elx_freeze_registry_t;

const char *elx_spec_freeze_version(void);

elx_freeze_registry_t *elx_freeze_registry_new(void);
void elx_freeze_registry_free(elx_freeze_registry_t *reg);

bool elx_freeze_registry_lock(elx_freeze_registry_t *reg, const char *version);
bool elx_freeze_registry_unlock(elx_freeze_registry_t *reg);
bool elx_freeze_registry_is_locked(const elx_freeze_registry_t *reg);
elx_freeze_state_t elx_freeze_registry_state(const elx_freeze_registry_t *reg);

bool elx_freeze_registry_add_rule(elx_freeze_registry_t *reg,
                                      const char *feature, const char *syntax,
                                      const char *semantics,
                                      bool is_syntax, bool is_semantic);
bool elx_freeze_registry_add_change(elx_freeze_registry_t *reg,
                                       const char *feature, elx_freeze_change_t change_type,
                                       const char *description,
                                       const char *migration_guide, bool breaking);

bool elx_freeze_validate_syntax(const elx_freeze_registry_t *reg,
                                    const char *source, size_t length,
                                    bool *violations);
bool elx_freeze_validate_semantic(const elx_freeze_registry_t *reg,
                                      const char *source, size_t length,
                                      bool *violations);
bool elx_freeze_check_breaking(const elx_freeze_registry_t *reg,
                                   const char *old_source, size_t old_len,
                                   const char *new_source, size_t new_len,
                                   bool *has_breaking);
bool elx_freeze_validate_version(const char *version, char *error, size_t error_size);
const char *elx_freeze_current_version(const elx_freeze_registry_t *reg);
bool elx_freeze_get_changelog(const elx_freeze_registry_t *reg, char *changelog,
                                  size_t changelog_size);
bool elx_freeze_get_migration_guide(const elx_freeze_registry_t *reg,
                                        const char *feature, char *guide,
                                        size_t guide_size);
bool elx_freeze_is_feature_frozen(const elx_freeze_registry_t *reg, const char *feature);
bool elx_freeze_print_report(const elx_freeze_registry_t *reg, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif
