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
#include "elx_spec_freeze.h"
#include "elx_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

const char *elx_spec_freeze_version(void) { return "0.1.0"; }

elx_freeze_registry_t *elx_freeze_registry_new(void) {
    elx_freeze_registry_t *r = (elx_freeze_registry_t *)calloc(1, sizeof(*r));
    return r;
}

void elx_freeze_registry_free(elx_freeze_registry_t *r) {
    if (!r) return;
    free(r);
}

bool elx_freeze_registry_lock(elx_freeze_registry_t *r, const char *version) {
    if (!r || !version) return false;
    strncpy(r->current_spec.version, version, ELX_FREEZE_MAX_VERSION - 1);
    r->current_spec.locked = true;
    r->freeze_active = true;
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    if (tm_info) {
        strftime(r->current_spec.date, sizeof(r->current_spec.date),
                 "%Y-%m-%d", tm_info);
    }
    return true;
}

bool elx_freeze_registry_unlock(elx_freeze_registry_t *r) {
    if (!r) return false;
    r->current_spec.locked = false;
    r->freeze_active = false;
    return true;
}

bool elx_freeze_registry_is_locked(const elx_freeze_registry_t *r) {
    return r && r->current_spec.locked;
}

elx_freeze_state_t elx_freeze_registry_state(const elx_freeze_registry_t *r) {
    if (!r) return ELX_FREEZE_STATE_DRAFT;
    if (r->freeze_active) return ELX_FREEZE_STATE_FROZEN;
    return ELX_FREEZE_STATE_DRAFT;
}

bool elx_freeze_registry_add_rule(elx_freeze_registry_t *r,
                                         const char *feature, const char *syntax,
                                         const char *semantics,
                                         bool is_syntax, bool is_semantic) {
    if (!r || !feature || r->rule_count >= ELX_FREEZE_MAX_RULES) return false;
    elx_freeze_rule_t *rule = &r->rules[r->rule_count];
    strncpy(rule->feature, feature, ELX_FREEZE_MAX_SPEC - 1);
    if (syntax) strncpy(rule->syntax, syntax, ELX_FREEZE_MAX_SPEC - 1);
    if (semantics) strncpy(rule->semantics, semantics, ELX_FREEZE_MAX_SPEC - 1);
    rule->is_syntax = is_syntax;
    rule->is_semantic = is_semantic;
    r->rule_count++;
    return true;
}

bool elx_freeze_registry_add_change(elx_freeze_registry_t *r,
                                           const char *feature, elx_freeze_change_t change_type,
                                           const char *description,
                                           const char *migration_guide, bool breaking) {
    if (!r || !feature || r->current_spec.change_count >= ELX_FREEZE_MAX_CHANGELOG) return false;
    elx_freeze_change_entry_t *c = &r->current_spec.changes[r->current_spec.change_count];
    strncpy(c->feature, feature, ELX_FREEZE_MAX_SPEC - 1);
    c->change_type = change_type;
    if (description) strncpy(c->description, description, ELX_FREEZE_MAX_SPEC - 1);
    if (migration_guide) strncpy(c->migration_guide, migration_guide, ELX_FREEZE_MAX_SPEC - 1);
    c->breaking = breaking;
    r->current_spec.change_count++;
    return true;
}

bool elx_freeze_validate_syntax(const elx_freeze_registry_t *r,
                                        const char *source, size_t length,
                                        bool *violations) {
    (void)r; (void)source; (void)length;
    if (violations) *violations = false;
    return true;
}

bool elx_freeze_validate_semantic(const elx_freeze_registry_t *r,
                                        const char *source, size_t length,
                                        bool *violations) {
    (void)r; (void)source; (void)length;
    if (violations) *violations = false;
    return true;
}

bool elx_freeze_check_breaking(const elx_freeze_registry_t *r,
                                      const char *old_source, size_t old_len,
                                      const char *new_source, size_t new_len,
                                      bool *has_breaking) {
    (void)r; (void)old_source; (void)old_len; (void)new_source; (void)new_len;
    if (has_breaking) *has_breaking = false;
    return true;
}

bool elx_freeze_validate_version(const char *version, char *error, size_t error_size) {
    if (!version) {
        if (error && error_size > 0) strncpy(error, "version is null", error_size - 1);
        return false;
    }
    size_t len = strlen(version);
    size_t dot_count = 0;
    for (size_t i = 0; i < len; i++) {
        if (version[i] == '.') dot_count++;
        else if (!isdigit((unsigned char)version[i]) &&
                 !(version[i] >= 'a' && version[i] <= 'z') &&
                 !(version[i] >= 'A' && version[i] <= 'Z') &&
                 version[i] != '-' && version[i] != '.' &&
                 version[i] != '_' && version[i] != 'r' &&
                 version[i] != 'c') {
            if (error && error_size > 0) strncpy(error, "invalid version format", error_size - 1);
            return false;
        }
    }
    if (dot_count < 1) {
        if (error && error_size > 0) strncpy(error, "version must contain at least one dot", error_size - 1);
        return false;
    }
    if (error && error_size > 0) error[0] = '\0';
    return true;
}

const char *elx_freeze_current_version(const elx_freeze_registry_t *r) {
    return r ? r->current_spec.version : "";
}

bool elx_freeze_get_changelog(const elx_freeze_registry_t *r, char *changelog,
                                       size_t changelog_size) {
    if (!r || !changelog) return false;
    size_t offset = 0;
    for (size_t i = 0; i < r->current_spec.change_count && offset < changelog_size - 1; i++) {
        const elx_freeze_change_entry_t *c = &r->current_spec.changes[i];
        const char *type_str = "change";
        switch (c->change_type) {
            case ELX_FREEZE_CHANGE_ADDITION: type_str = "add"; break;
            case ELX_FREEZE_CHANGE_MODIFICATION: type_str = "modify"; break;
            case ELX_FREEZE_CHANGE_REMOVAL: type_str = "remove"; break;
            case ELX_FREEZE_CHANGE_DEPRECATION: type_str = "deprecate"; break;
            default: break;
        }
        int written = snprintf(changelog + offset, changelog_size - offset,
                               "[%s] %s: %s\n", type_str, c->feature,
                               c->description[0] ? c->description : "");
        if (written < 0 || (size_t)written >= changelog_size - offset) break;
        offset += (size_t)written;
    }
    return true;
}

bool elx_freeze_get_migration_guide(const elx_freeze_registry_t *r,
                                            const char *feature, char *guide,
                                            size_t guide_size) {
    if (!r || !feature || !guide) return false;
    for (size_t i = 0; i < r->current_spec.change_count; i++) {
        if (strcmp(r->current_spec.changes[i].feature, feature) == 0 &&
            r->current_spec.changes[i].migration_guide[0]) {
            strncpy(guide, r->current_spec.changes[i].migration_guide, guide_size - 1);
            return true;
        }
    }
    if (guide_size > 0) guide[0] = '\0';
    return false;
}

bool elx_freeze_is_feature_frozen(const elx_freeze_registry_t *r, const char *feature) {
    (void)r; (void)feature;
    return r && r->freeze_active;
}

bool elx_freeze_print_report(const elx_freeze_registry_t *r, FILE *stream) {
    if (!r || !stream) return false;
    fprintf(stream, "=== Spec Freeze Report ===\n");
    fprintf(stream, "Version: %s\n", r->current_spec.version);
    fprintf(stream, "Date: %s\n", r->current_spec.date);
    fprintf(stream, "Locked: %s\n", r->current_spec.locked ? "yes" : "no");
    fprintf(stream, "Rules: %zu\n", r->rule_count);
    fprintf(stream, "Changes: %zu\n", r->current_spec.change_count);
    fprintf(stream, "Features: %zu\n", r->current_spec.feature_count);
    for (size_t i = 0; i < r->rule_count; i++) {
        fprintf(stream, "  Rule: %s (syntax=%d, semantic=%d)\n",
                r->rules[i].feature, r->rules[i].is_syntax, r->rules[i].is_semantic);
    }
    fprintf(stream, "==========================\n");
    return true;
}
