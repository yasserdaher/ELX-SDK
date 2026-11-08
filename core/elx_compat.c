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
#include "elx_compat.h"
#include "elx_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *elx_compat_version(void) { return "0.1.0"; }

elx_compat_suite_t *elx_compat_suite_new(void) {
    elx_compat_suite_t *s = (elx_compat_suite_t *)calloc(1, sizeof(*s));
    if (s) {
        s->diagnostics = elx_diagnostics_new();
    }
    return s;
}

void elx_compat_suite_free(elx_compat_suite_t *s) {
    if (!s) return;
    if (s->diagnostics) elx_diagnostics_free(s->diagnostics);
    free(s);
}

bool elx_compat_suite_add_module(elx_compat_suite_t *s, const char *name,
                                            const char *version) {
    if (!s || !name || s->graph.node_count >= ELX_COMPAT_MAX_GRAPH) return false;
    elx_compat_node_t *n = &s->graph.nodes[s->graph.node_count];
    strncpy(n->name, name, ELX_MODULE_MAX_NAME - 1);
    if (version) strncpy(n->version, version, ELX_MODULE_MAX_VERSION - 1);
    n->dependency_count = 0;
    n->dependent_count = 0;
    s->graph.node_count++;
    return true;
}

bool elx_compat_suite_add_dependency(elx_compat_suite_t *s,
                                                const char *from, const char *to,
                                                const char *version_constraint) {
    if (!s || !from || !to || s->graph.edge_count >= ELX_COMPAT_MAX_GRAPH) return false;
    elx_compat_edge_t *e = &s->graph.edges[s->graph.edge_count];
    strncpy(e->from_module, from, ELX_MODULE_MAX_NAME - 1);
    strncpy(e->to_module, to, ELX_MODULE_MAX_NAME - 1);
    if (version_constraint) strncpy(e->version_constraint, version_constraint,
                                          ELX_COMPAT_MAX_VERSION_RANGE - 1);
    e->is_cyclic = false;
    s->graph.edge_count++;

    for (size_t i = 0; i < s->graph.node_count; i++) {
        if (strcmp(s->graph.nodes[i].name, from) == 0 &&
            s->graph.nodes[i].dependency_count < ELX_PACKAGE_MAX_DEPS) {
            strncpy(s->graph.nodes[i].dependencies
                        [s->graph.nodes[i].dependency_count],
                    to, ELX_MODULE_MAX_NAME - 1);
            s->graph.nodes[i].dependency_count++;
            break;
        }
    }
    return true;
}

bool elx_compat_suite_add_package(elx_compat_suite_t *s,
                                         const elx_package_info_t *pkg) {
    (void)s; (void)pkg;
    return false;
}

elx_compat_graph_t *elx_compat_suite_graph(const elx_compat_suite_t *s) {
    return s ? (elx_compat_graph_t *)&s->graph : NULL;
}

static bool find_node(const elx_compat_graph_t *g, const char *name) {
    for (size_t i = 0; i < g->node_count; i++) {
        if (strcmp(g->nodes[i].name, name) == 0) return true;
    }
    return false;
}

bool elx_compat_detect_cycles(const elx_compat_suite_t *s,
                                         char cyclic[][ELX_MODULE_MAX_NAME],
                                         size_t *cycle_count) {
    if (!s || !cyclic || !cycle_count) return false;
    elx_compat_graph_t *g = (elx_compat_graph_t *)&s->graph;
    *cycle_count = 0;
    for (size_t i = 0; i < g->edge_count; i++) {
        for (size_t j = 0; j < g->edge_count; j++) {
            if (i == j) continue;
            if (strcmp(g->edges[i].to_module, g->edges[j].from_module) == 0 &&
                strcmp(g->edges[j].to_module, g->edges[i].from_module) == 0) {
                if (*cycle_count < ELX_COMPAT_MAX_GRAPH) {
                    strncpy(cyclic[*cycle_count], g->edges[i].from_module,
                            ELX_MODULE_MAX_NAME - 1);
                    (*cycle_count)++;
                }
            }
        }
    }
    return true;
}

bool elx_compat_detect_conflicts(const elx_compat_suite_t *s,
                                          elx_version_conflict_t *conflicts,
                                          size_t *conflict_count) {
    (void)s; (void)conflicts; (void)conflict_count;
    return false;
}

bool elx_compat_check_dependencies(const elx_compat_suite_t *s,
                                          char missing[][ELX_MODULE_MAX_NAME],
                                          size_t *missing_count) {
    if (!s || !missing || !missing_count) return false;
    *missing_count = 0;
    for (size_t i = 0; i < s->graph.node_count; i++) {
        for (size_t j = 0; j < s->graph.nodes[i].dependency_count; j++) {
            if (!find_node(&s->graph, s->graph.nodes[i].dependencies[j])) {
                if (*missing_count < ELX_COMPAT_MAX_GRAPH) {
                    strncpy(missing[*missing_count],
                            s->graph.nodes[i].dependencies[j],
                            ELX_MODULE_MAX_NAME - 1);
                    (*missing_count)++;
                }
            }
        }
    }
    return true;
}

bool elx_compat_validate_manifest(const elx_package_info_t *pkg,
                                         char *error, size_t error_size) {
    if (!pkg) {
        if (error && error_size > 0) strncpy(error, "package is null", error_size - 1);
        return false;
    }
    if (strlen(pkg->name) == 0) {
        if (error && error_size > 0) strncpy(error, "name is required", error_size - 1);
        return false;
    }
    if (strlen(pkg->version) == 0) {
        if (error && error_size > 0) strncpy(error, "version is required", error_size - 1);
        return false;
    }
    if (error && error_size > 0) error[0] = '\0';
    return true;
}

bool elx_compat_validate_package(const char *package_dir, char *error,
                                        size_t error_size) {
    (void)package_dir; (void)error; (void)error_size;
    return false;
}

bool elx_compat_check_imports(const char *source, size_t length,
                                       const elx_module_cache_t *cache,
                                       elx_diagnostics_t *diagnostics) {
    if (!source || !length || !diagnostics) return false;
    const char *ptr = source;
    while (ptr < source + length) {
        if (strncmp(ptr, "import ", 7) == 0) {
            ptr += 7;
            const char *end = ptr;
            while (end < source + length && *end != ' ' && *end != '\n' &&
                   *end != '\r' && *end != '\t' && *end != '\0') end++;
            size_t name_len = (size_t)(end - ptr);
            if (name_len > 0) {
                char module_name[ELX_MODULE_MAX_NAME];
                if (name_len >= ELX_MODULE_MAX_NAME) name_len = ELX_MODULE_MAX_NAME - 1;
                memcpy(module_name, ptr, name_len);
                module_name[name_len] = '\0';
                elx_module_info_t info;
                if (cache && !elx_module_resolve((elx_module_cache_t *)cache,
                                                  module_name, &info)) {
                    elx_diagnostic_t *d = elx_diag_create("<source>", 0, 0,
                        ELX_DIAG_CODE_IMPORT, ELX_DIAG_ERROR,
                        "Module not found");
                    if (d) {
                        char detail[ELX_DIAG_MAX_MESSAGE];
                        snprintf(detail, sizeof(detail),
                                 "Module '%s' not found in cache or stdlib",
                                 module_name);
                        elx_diag_add_note(d, "import", detail,
                                          "Ensure the module is installed");
                        elx_diag_add_suggestion(d, "resolve",
                                                "Run elx module resolve <name>");
                        elx_diagnostics_add(diagnostics, d);
                        free(d);
                    }
                }
            }
        }
        ptr++;
    }
    return true;
}

bool elx_compat_check_exports(const char *source, size_t length,
                                       elx_diagnostics_t *diagnostics) {
    (void)source; (void)length; (void)diagnostics;
    return true;
}

bool elx_compat_resolve_dependencies(const elx_compat_suite_t *s,
                                            char resolved[][ELX_MODULE_MAX_NAME],
                                            size_t *resolved_count) {
    if (!s || !resolved || !resolved_count) return false;
    *resolved_count = 0;
    for (size_t i = 0; i < s->graph.node_count; i++) {
        for (size_t j = 0; j < s->graph.nodes[i].dependency_count; j++) {
            int found = 0;
            for (size_t k = 0; k < *resolved_count; k++) {
                if (strcmp(resolved[k], s->graph.nodes[i].dependencies[j]) == 0) {
                    found = 1; break;
                }
            }
            if (!found) {
                strncpy(resolved[*resolved_count],
                        s->graph.nodes[i].dependencies[j],
                        ELX_MODULE_MAX_NAME - 1);
                (*resolved_count)++;
            }
        }
    }
    return true;
}

bool elx_compat_build_dependency_tree(const elx_compat_suite_t *s,
                                               const char *root, char tree[][ELX_COMPAT_MAX_PATH],
                                               size_t *line_count, size_t max_depth) {
    (void)s; (void)root; (void)tree; (void)line_count; (void)max_depth;
    return false;
}

bool elx_compat_print_graph(const elx_compat_suite_t *s, FILE *stream) {
    if (!s || !stream) return false;
    fprintf(stream, "=== Dependency Graph ===\n");
    for (size_t i = 0; i < s->graph.node_count; i++) {
        elx_compat_node_t *n = (elx_compat_node_t *)&s->graph.nodes[i];
        fprintf(stream, "  %s@%s", n->name, n->version);
        if (n->dependency_count > 0) {
            fprintf(stream, " -> ");
            for (size_t j = 0; j < n->dependency_count; j++) {
                if (j > 0) fprintf(stream, ", ");
                fprintf(stream, "%s", n->dependencies[j]);
            }
        }
        fprintf(stream, "\n");
    }
    fprintf(stream, "=========================\n");
    return true;
}

bool elx_compat_print_report(const elx_compat_suite_t *s, FILE *stream) {
    if (!s || !stream) return false;
    fprintf(stream, "=== Compatibility Report ===\n");
    fprintf(stream, "Nodes: %zu | Edges: %zu\n",
            s->graph.node_count, s->graph.edge_count);
    char cyclic[ELX_COMPAT_MAX_GRAPH][ELX_MODULE_MAX_NAME];
    size_t cycle_count = 0;
    elx_compat_detect_cycles(s, cyclic, &cycle_count);
    if (cycle_count > 0) {
        fprintf(stream, "Circular dependencies: %zu\n", cycle_count);
        for (size_t i = 0; i < cycle_count; i++) {
            fprintf(stream, "  Cycle: %s\n", cyclic[i]);
        }
    } else {
        fprintf(stream, "No circular dependencies found\n");
    }
    char missing[ELX_COMPAT_MAX_GRAPH][ELX_MODULE_MAX_NAME];
    size_t missing_count = 0;
    elx_compat_check_dependencies(s, missing, &missing_count);
    if (missing_count > 0) {
        fprintf(stream, "Missing dependencies: %zu\n", missing_count);
        for (size_t i = 0; i < missing_count; i++) {
            fprintf(stream, "  Missing: %s\n", missing[i]);
        }
    } else {
        fprintf(stream, "All dependencies resolved\n");
    }
    fprintf(stream, "=============================\n");
    return true;
}

bool elx_compat_check_circular(elx_compat_graph_t *graph,
                                        char cyclic[][ELX_MODULE_MAX_NAME],
                                        size_t *cycle_count) {
    if (!graph || !cyclic || !cycle_count) return false;
    *cycle_count = 0;
    for (size_t i = 0; i < graph->edge_count; i++) {
        for (size_t j = 0; j < graph->edge_count; j++) {
            if (i == j) continue;
            if (strcmp(graph->edges[i].to_module, graph->edges[j].from_module) == 0 &&
                strcmp(graph->edges[j].to_module, graph->edges[i].from_module) == 0) {
                if (*cycle_count < ELX_COMPAT_MAX_GRAPH) {
                    strncpy(cyclic[*cycle_count], graph->edges[i].from_module,
                            ELX_MODULE_MAX_NAME - 1);
                    (*cycle_count)++;
                }
            }
        }
    }
    return true;
}

bool elx_compat_check_version_range(const char *version,
                                             const char *constraint, bool *satisfied) {
    if (!version || !constraint || !satisfied) return false;
    if (strcmp(constraint, "*") == 0) {
        *satisfied = true;
        return true;
    }
    if (strncmp(constraint, ">=", 2) == 0) {
        *satisfied = strcmp(version, constraint + 2) >= 0;
        return true;
    }
    if (strncmp(constraint, "<=", 2) == 0) {
        *satisfied = strcmp(version, constraint + 2) <= 0;
        return true;
    }
    if (strncmp(constraint, ">", 1) == 0) {
        *satisfied = strcmp(version, constraint + 1) > 0;
        return true;
    }
    if (strncmp(constraint, "<", 1) == 0) {
        *satisfied = strcmp(version, constraint + 1) < 0;
        return true;
    }
    *satisfied = strcmp(version, constraint) == 0;
    return true;
}
