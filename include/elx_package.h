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

#ifndef ELX_PACKAGE_H
#define ELX_PACKAGE_H

#include "elx_config.h"
#include "elx_std.h"
#include "elx_module.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_PACKAGE_MANIFEST_NAME "elx.pkg.json"
#define ELX_PACKAGE_MAX_NAME 256
#define ELX_PACKAGE_MAX_VERSION 64
#define ELX_PACKAGE_MAX_AUTHOR 256
#define ELX_PACKAGE_MAX_DESC 1024
#define ELX_PACKAGE_MAX_DEPS 128
#define ELX_PACKAGE_MAX_LICENSE 128

typedef enum {
    ELX_PACKAGE_LICENSE_UNKNOWN = 0,
    ELX_PACKAGE_LICENSE_MIT,
    ELX_PACKAGE_LICENSE_APACHE2,
    ELX_PACKAGE_LICENSE_GPL3,
    ELX_PACKAGE_LICENSE_LGPL3,
    ELX_PACKAGE_LICENSE_BSD,
    ELX_PACKAGE_LICENSE_MPL2,
    ELX_PACKAGE_LICENSE_UNLICENSE,
    ELX_PACKAGE_LICENSE_WTFPL,
    ELX_PACKAGE_LICENSE_CUSTOM,
} elx_package_license_t;

typedef enum {
    ELX_PACKAGE_STATUS_PRIVATE = 0,
    ELX_PACKAGE_STATUS_PUBLIC,
    ELX_PACKAGE_STATUS_VERIFIED,
} elx_package_status_t;

typedef struct {
    char name[ELX_PACKAGE_MAX_NAME];
    char version[ELX_PACKAGE_MAX_VERSION];
    char author[ELX_PACKAGE_MAX_AUTHOR];
    char description[ELX_PACKAGE_MAX_DESC];
    char license[ELX_PACKAGE_MAX_LICENSE];
    elx_package_license_t license_type;
    elx_package_status_t status;
    char homepage[ELX_MODULE_MAX_PATH];
    char repository[ELX_MODULE_MAX_PATH];
    char entry_point[ELX_MODULE_MAX_PATH];
    char target[ELX_MODULE_MAX_PATH];
    char dependencies[ELX_PACKAGE_MAX_DEPS][ELX_MODULE_MAX_NAME];
    char dev_dependencies[ELX_PACKAGE_MAX_DEPS][ELX_MODULE_MAX_NAME];
    size_t dependency_count;
    size_t dev_dependency_count;
    char modules[ELX_PACKAGE_MAX_DEPS][ELX_MODULE_MAX_NAME];
    size_t module_count;
    elx_module_type_t module_type;
    bool sandbox;
    bool verified;
    uint64_t created_at;
    uint64_t updated_at;
} elx_package_info_t;

typedef struct elx_package_entry {
    elx_package_info_t info;
    struct elx_package_entry *next;
} elx_package_entry_t;

typedef struct {
    elx_package_entry_t *head;
    size_t count;
    char registry_url[ELX_MODULE_MAX_PATH];
    char cache_dir[ELX_MODULE_MAX_PATH];
    void *impl;
} elx_package_registry_t;

typedef void (*elx_package_progress_fn)(const char *action, const char *name,
                                        float progress, void *user);

const char *elx_package_version(void);

elx_package_registry_t *elx_package_registry_new(void);
void elx_package_registry_free(elx_package_registry_t *registry);

bool elx_package_info_init(elx_package_info_t *info);
void elx_package_info_clear(elx_package_info_t *info);
bool elx_package_info_parse(elx_package_info_t *info, const char *manifest_path);
bool elx_package_info_write(const elx_package_info_t *info, const char *manifest_path);

elx_package_license_t elx_package_license_from_string(const char *string);
const char *elx_package_license_name(elx_package_license_t license);
elx_package_status_t elx_package_status_from_string(const char *string);
const char *elx_package_status_name(elx_package_status_t status);

bool elx_package_registry_add(elx_package_registry_t *registry, const elx_package_info_t *info);
elx_package_entry_t *elx_package_registry_find(const elx_package_registry_t *registry, const char *name);
elx_package_entry_t *elx_package_registry_get(const elx_package_registry_t *registry, size_t index);
size_t elx_package_registry_count(const elx_package_registry_t *registry);
bool elx_package_registry_is_empty(const elx_package_registry_t *registry);
void elx_package_registry_clear(elx_package_registry_t *registry);
void elx_package_registry_foreach(elx_package_registry_t *registry,
                                  void (*callback)(const elx_package_info_t *info, void *user),
                                  void *user);

bool elx_package_registry_set_url(elx_package_registry_t *registry, const char *url);
const char *elx_package_registry_url(const elx_package_registry_t *registry);
bool elx_package_registry_set_cache_dir(elx_package_registry_t *registry, const char *dir);
const char *elx_package_registry_cache_dir(const elx_package_registry_t *registry);

bool elx_package_install(elx_package_registry_t *registry, const char *name,
                         const char *version_constraint, elx_package_progress_fn progress,
                         void *user);
bool elx_package_update(elx_package_registry_t *registry, const char *name,
                        elx_package_progress_fn progress, void *user);
bool elx_package_remove(elx_package_registry_t *registry, const char *name);
bool elx_package_list(elx_package_registry_t *registry);
bool elx_package_search(elx_package_registry_t *registry, const char *query, size_t max_results);
bool elx_package_resolve_dependencies(elx_package_registry_t *registry,
                                      const elx_package_info_t *package,
                                      char resolved[][ELX_MODULE_MAX_NAME],
                                      size_t *resolved_count);
bool elx_package_validate(const elx_package_info_t *info, char *error, size_t error_size);
bool elx_package_verify_signature(const elx_package_info_t *info);

#ifdef __cplusplus
}
#endif

#endif
