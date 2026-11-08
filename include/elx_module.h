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

#ifndef ELX_MODULE_H
#define ELX_MODULE_H

#include "elx_config.h"
#include "elx_std.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_MODULE_MANIFEST_NAME "elx.mod.json"
#define ELX_MODULE_MAX_NAME 256
#define ELX_MODULE_MAX_VERSION 64
#define ELX_MODULE_MAX_PATH 4096
#define ELX_MODULE_MAX_DEPS 64

typedef enum {
    ELX_MODULE_TYPE_STDLIB = 0,
    ELX_MODULE_TYPE_LOCAL,
    ELX_MODULE_TYPE_PACKAGE,
    ELX_MODULE_TYPE_BUILTIN,
} elx_module_type_t;

typedef struct {
    char name[ELX_MODULE_MAX_NAME];
    char version[ELX_MODULE_MAX_VERSION];
    char path[ELX_MODULE_MAX_PATH];
    elx_module_type_t type;
    char dependencies[ELX_MODULE_MAX_DEPS][ELX_MODULE_MAX_NAME];
    size_t dependency_count;
    size_t exports_count;
    bool available;
} elx_module_info_t;

typedef struct elx_module_entry {
    elx_module_info_t info;
    struct elx_module_entry *next;
} elx_module_entry_t;

typedef struct {
    elx_module_entry_t *head;
    size_t count;
    void *impl;
} elx_module_cache_t;

const char *elx_module_version(void);

elx_module_cache_t *elx_module_cache_new(void);
void elx_module_cache_free(elx_module_cache_t *cache);

bool elx_module_info_init(elx_module_info_t *info);
void elx_module_info_clear(elx_module_info_t *info);
bool elx_module_info_parse(elx_module_info_t *info, const char *manifest_path);
bool elx_module_info_write(const elx_module_info_t *info, const char *manifest_path);

elx_module_type_t elx_module_type_from_string(const char *string);
const char *elx_module_type_name(elx_module_type_t type);

bool elx_module_cache_add(elx_module_cache_t *cache, const elx_module_info_t *info);
elx_module_entry_t *elx_module_cache_find(const elx_module_cache_t *cache, const char *name);
elx_module_entry_t *elx_module_cache_get(const elx_module_cache_t *cache, size_t index);
size_t elx_module_cache_count(const elx_module_cache_t *cache);
bool elx_module_cache_is_empty(const elx_module_cache_t *cache);
void elx_module_cache_clear(elx_module_cache_t *cache);
void elx_module_cache_foreach(elx_module_cache_t *cache,
                               void (*callback)(const elx_module_info_t *info, void *user),
                               void *user);

const char *elx_module_cache_stdlib_path(void);
bool elx_module_resolve(elx_module_cache_t *cache, const char *name, elx_module_info_t *out_info);
bool elx_module_load_manifest(elx_module_cache_t *cache, const char *module_dir);

#ifdef __cplusplus
}
#endif

#endif
