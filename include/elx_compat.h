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

#ifndef ELX_COMPAT_H
#define ELX_COMPAT_H

#include "elx_config.h"
#include "elx_std.h"
#include "elx_module.h"
#include "elx_package.h"
#include "elx_diagnostics.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_COMPAT_MAX_GRAPH 1024
#define ELX_COMPAT_MAX_PATH 4096
#define ELX_COMPAT_MAX_VERSION_RANGE 64

typedef struct {
    char from_module[ELX_MODULE_MAX_NAME];
    char to_module[ELX_MODULE_MAX_NAME];
    char version_constraint[ELX_COMPAT_MAX_VERSION_RANGE];
    bool is_cyclic;
} elx_compat_edge_t;

typedef struct {
    char name[ELX_MODULE_MAX_NAME];
    char version[ELX_MODULE_MAX_VERSION];
    char dependencies[ELX_PACKAGE_MAX_DEPS][ELX_MODULE_MAX_NAME];
    size_t dependency_count;
    char dependents[ELX_PACKAGE_MAX_DEPS][ELX_MODULE_MAX_NAME];
    size_t dependent_count;
} elx_compat_node_t;

typedef struct {
    elx_compat_node_t nodes[ELX_COMPAT_MAX_GRAPH];
    elx_compat_edge_t edges[ELX_COMPAT_MAX_GRAPH];
    size_t node_count;
    size_t edge_count;
} elx_compat_graph_t;

typedef enum {
    ELX_COMPAT_OK = 0,
    ELX_COMPAT_VERSION_CONFLICT,
    ELX_COMPAT_CYCLIC_DEPENDENCY,
    ELX_COMPAT_MISSING_DEPENDENCY,
    ELX_COMPAT_INCOMPATIBLE_VERSION,
    ELX_COMPAT_MALFORMED_MANIFEST,
    ELX_COMPAT_BROKEN_IMPORT,
    ELX_COMPAT_UNUSED_EXPORT,
    ELX_COMPAT_DUPLICATE_IMPORT,
    ELX_COMPAT_SANDBOX_VIOLATION,
} elx_compat_issue_t;

typedef struct {
    elx_compat_issue_t issue_type;
    char subject[ELX_MODULE_MAX_NAME];
    char detail[ELX_DIAG_MAX_MESSAGE];
    char suggestion[ELX_DIAG_MAX_MESSAGE];
    bool is_error;
} elx_compat_issue_entry_t;

typedef struct {
    elx_compat_graph_t graph;
    elx_diagnostics_t *diagnostics;
    char issues[ELX_COMPAT_MAX_GRAPH][ELX_DIAG_MAX_MESSAGE];
    size_t issue_count;
    bool resolved;
} elx_compat_suite_t;

typedef struct {
    char package_a[ELX_MODULE_MAX_NAME];
    char package_b[ELX_MODULE_MAX_NAME];
    char conflict_detail[ELX_DIAG_MAX_MESSAGE];
} elx_version_conflict_t;

const char *elx_compat_version(void);

elx_compat_suite_t *elx_compat_suite_new(void);
void elx_compat_suite_free(elx_compat_suite_t *suite);

bool elx_compat_suite_add_module(elx_compat_suite_t *suite, const char *name,
                                     const char *version);
bool elx_compat_suite_add_dependency(elx_compat_suite_t *suite, const char *from,
                                         const char *to, const char *version_constraint);
bool elx_compat_suite_add_package(elx_compat_suite_t *suite,
                                      const elx_package_info_t *pkg);

elx_compat_graph_t *elx_compat_suite_graph(const elx_compat_suite_t *suite);

bool elx_compat_detect_cycles(const elx_compat_suite_t *suite,
                                  char cyclic[][ELX_MODULE_MAX_NAME],
                                  size_t *cycle_count);
bool elx_compat_detect_conflicts(const elx_compat_suite_t *suite,
                                     elx_version_conflict_t *conflicts,
                                     size_t *conflict_count);
bool elx_compat_check_dependencies(const elx_compat_suite_t *suite,
                                       char missing[][ELX_MODULE_MAX_NAME],
                                       size_t *missing_count);
bool elx_compat_validate_manifest(const elx_package_info_t *pkg,
                                      char *error, size_t error_size);
bool elx_compat_validate_package(const char *package_dir, char *error,
                                     size_t error_size);

bool elx_compat_check_imports(const char *source, size_t length,
                                  const elx_module_cache_t *cache,
                                  elx_diagnostics_t *diagnostics);
bool elx_compat_check_exports(const char *source, size_t length,
                                  elx_diagnostics_t *diagnostics);

bool elx_compat_resolve_dependencies(const elx_compat_suite_t *suite,
                                         char resolved[][ELX_MODULE_MAX_NAME],
                                         size_t *resolved_count);
bool elx_compat_build_dependency_tree(const elx_compat_suite_t *suite,
                                          const char *root, char tree[][ELX_COMPAT_MAX_PATH],
                                          size_t *line_count, size_t max_depth);
bool elx_compat_print_graph(const elx_compat_suite_t *suite, FILE *stream);
bool elx_compat_print_report(const elx_compat_suite_t *suite, FILE *stream);

bool elx_compat_check_circular(elx_compat_graph_t *graph,
                                   char cyclic[][ELX_MODULE_MAX_NAME],
                                   size_t *cycle_count);
bool elx_compat_check_version_range(const char *version,
                                        const char *constraint, bool *satisfied);

#ifdef __cplusplus
}
#endif

#endif
