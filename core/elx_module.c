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

/*
 * ELVI OS — VOLT 1.26 License Agreement (ELECTRON VISION)
 * See LICENSE-ELX at the project root for full terms.
 */
#define _POSIX_C_SOURCE 200809L
#include "elx_module.h"
#include "elx_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

const char *elx_module_version(void) { return "0.1.0"; }

elx_module_type_t elx_module_type_from_string(const char *string) {
    if (!string) return ELX_MODULE_TYPE_LOCAL;
    if (strcmp(string, "stdlib") == 0) return ELX_MODULE_TYPE_STDLIB;
    if (strcmp(string, "local") == 0) return ELX_MODULE_TYPE_LOCAL;
    if (strcmp(string, "package") == 0) return ELX_MODULE_TYPE_PACKAGE;
    if (strcmp(string, "builtin") == 0) return ELX_MODULE_TYPE_BUILTIN;
    return ELX_MODULE_TYPE_LOCAL;
}

const char *elx_module_type_name(elx_module_type_t type) {
    switch (type) {
        case ELX_MODULE_TYPE_STDLIB: return "stdlib";
        case ELX_MODULE_TYPE_LOCAL: return "local";
        case ELX_MODULE_TYPE_PACKAGE: return "package";
        case ELX_MODULE_TYPE_BUILTIN: return "builtin";
        default: return "local";
    }
}

bool elx_module_info_init(elx_module_info_t *info) {
    if (!info) return false;
    memset(info, 0, sizeof(*info));
    strncpy(info->name, "unknown", ELX_MODULE_MAX_NAME - 1);
    strncpy(info->version, "0.1.0", ELX_MODULE_MAX_VERSION - 1);
    info->type = ELX_MODULE_TYPE_LOCAL;
    return true;
}

void elx_module_info_clear(elx_module_info_t *info) {
    (void)info;
}

bool elx_module_info_parse(elx_module_info_t *info, const char *manifest_path) {
    if (!info || !manifest_path) return false;
    FILE *f = fopen(manifest_path, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) { fclose(f); return false; }
    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) { fclose(f); return false; }
    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);
    char key[128];
    char value[512];
    char *p = buf;
    elx_module_info_init(info);
    strncpy(info->name, "unknown", ELX_MODULE_MAX_NAME - 1);
    strncpy(info->version, "0.1.0", ELX_MODULE_MAX_VERSION - 1);
    strncpy(info->path, manifest_path, ELX_MODULE_MAX_PATH - 1);
    info->type = ELX_MODULE_TYPE_LOCAL;
    while (*p) {
        while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
        if (*p != '"') { p++; continue; }
        size_t ki = 0;
        p++;
        while (*p && *p != '"' && ki < sizeof(key) - 1) { key[ki++] = *p++; }
        key[ki] = '\0';
        while (*p && *p != ':') p++;
        if (!*p) { free(buf); return false; }
        p++;
        while (*p && (*p == ' ' || *p == '\t')) p++;
        if (*p == '"') {
            p++;
            size_t vi = 0;
            while (*p && *p != '"' && vi < sizeof(value) - 1) { value[vi++] = *p++; }
            value[vi] = '\0';
            if (strcmp(key, "name") == 0) { strncpy(info->name, value, ELX_MODULE_MAX_NAME - 1); }
            else if (strcmp(key, "version") == 0) { strncpy(info->version, value, ELX_MODULE_MAX_VERSION - 1); }
            else if (strcmp(key, "type") == 0) { info->type = elx_module_type_from_string(value); }
            else if (strcmp(key, "path") == 0) { strncpy(info->path, value, ELX_MODULE_MAX_PATH - 1); }
        }
        while (*p && *p != ',' && *p != '}') p++;
        if (*p == ',') p++;
    }
    free(buf);
    info->available = true;
    return true;
}

bool elx_module_info_write(const elx_module_info_t *info, const char *manifest_path) {
    if (!info || !manifest_path) return false;
    FILE *f = fopen(manifest_path, "wb");
    if (!f) return false;
    fprintf(f, "{\n");
    fprintf(f, "  \"name\": \"%s\",\n", info->name);
    fprintf(f, "  \"version\": \"%s\",\n", info->version);
    fprintf(f, "  \"type\": \"%s\",\n", elx_module_type_name(info->type));
    fprintf(f, "  \"path\": \"%s\"\n", info->path);
    fprintf(f, "}\n");
    fclose(f);
    return true;
}

elx_module_cache_t *elx_module_cache_new(void) {
    elx_module_cache_t *cache = (elx_module_cache_t *)calloc(1, sizeof(*cache));
    return cache;
}

void elx_module_cache_free(elx_module_cache_t *cache) {
    if (!cache) return;
    elx_module_cache_clear(cache);
    free(cache);
}

bool elx_module_cache_add(elx_module_cache_t *cache, const elx_module_info_t *info) {
    if (!cache || !info) return false;
    elx_module_entry_t *entry = (elx_module_entry_t *)calloc(1, sizeof(*entry));
    if (!entry) return false;
    entry->info = *info;
    entry->next = cache->head;
    cache->head = entry;
    cache->count++;
    return true;
}

elx_module_entry_t *elx_module_cache_find(const elx_module_cache_t *cache, const char *name) {
    if (!cache || !name) return NULL;
    for (elx_module_entry_t *e = cache->head; e; e = e->next) {
        if (strcmp(e->info.name, name) == 0) return e;
    }
    return NULL;
}

elx_module_entry_t *elx_module_cache_get(const elx_module_cache_t *cache, size_t index) {
    if (!cache) return NULL;
    size_t i = 0;
    for (elx_module_entry_t *e = cache->head; e; e = e->next) {
        if (i == index) return e;
        i++;
    }
    return NULL;
}

size_t elx_module_cache_count(const elx_module_cache_t *cache) {
    return cache ? cache->count : 0;
}

bool elx_module_cache_is_empty(const elx_module_cache_t *cache) {
    return cache ? cache->count == 0 : true;
}

void elx_module_cache_clear(elx_module_cache_t *cache) {
    if (!cache) return;
    elx_module_entry_t *e = cache->head;
    while (e) {
        elx_module_entry_t *next = e->next;
        elx_module_info_clear(&e->info);
        free(e);
        e = next;
    }
    cache->head = NULL;
    cache->count = 0;
}

void elx_module_cache_foreach(elx_module_cache_t *cache,
                                  void (*callback)(const elx_module_info_t *info, void *user),
                                  void *user) {
    if (!cache || !callback) return;
    for (elx_module_entry_t *e = cache->head; e; e = e->next) {
        callback(&e->info, user);
    }
}

const char *elx_module_cache_stdlib_path(void) {
    return "stdlib";
}

bool elx_module_load_manifest(elx_module_cache_t *cache, const char *module_dir) {
    if (!cache || !module_dir) return false;
    char manifest[ELX_MODULE_MAX_PATH];
    snprintf(manifest, sizeof(manifest), "%s/%s", module_dir, ELX_MODULE_MANIFEST_NAME);
    elx_module_info_t info;
    if (elx_module_info_parse(&info, manifest)) {
        strncpy(info.path, module_dir, ELX_MODULE_MAX_PATH - 1);
        return elx_module_cache_add(cache, &info);
    }
    return false;
}

bool elx_module_resolve(elx_module_cache_t *cache, const char *name, elx_module_info_t *out_info) {
    if (!cache || !name || !out_info) return false;
    elx_module_entry_t *found = elx_module_cache_find(cache, name);
    if (found) { *out_info = found->info; return true; }
    char path[ELX_MODULE_MAX_PATH];
    snprintf(path, sizeof(path), "stdlib/%s", name);
    if (elx_module_load_manifest(cache, path)) {
        found = elx_module_cache_find(cache, name);
        if (found) { *out_info = found->info; return true; }
    }
    snprintf(path, sizeof(path), "modules/%s", name);
    if (elx_module_load_manifest(cache, path)) {
        found = elx_module_cache_find(cache, name);
        if (found) { *out_info = found->info; return true; }
    }
    return false;
}
