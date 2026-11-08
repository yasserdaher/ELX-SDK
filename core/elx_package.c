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
#include "elx_package.h"
#include "elx_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *elx_package_version(void) { return "0.1.0"; }

elx_package_license_t elx_package_license_from_string(const char *string) {
    if (!string) return ELX_PACKAGE_LICENSE_UNKNOWN;
    if (strcmp(string, "MIT") == 0) return ELX_PACKAGE_LICENSE_MIT;
    if (strcmp(string, "Apache-2.0") == 0 || strcmp(string, "Apache2") == 0) return ELX_PACKAGE_LICENSE_APACHE2;
    if (strcmp(string, "GPL-3.0") == 0 || strcmp(string, "GPL3") == 0) return ELX_PACKAGE_LICENSE_GPL3;
    if (strcmp(string, "LGPL-3.0") == 0 || strcmp(string, "LGPL3") == 0) return ELX_PACKAGE_LICENSE_LGPL3;
    if (strcmp(string, "BSD") == 0) return ELX_PACKAGE_LICENSE_BSD;
    if (strcmp(string, "MPL-2.0") == 0 || strcmp(string, "MPL2") == 0) return ELX_PACKAGE_LICENSE_MPL2;
    if (strcmp(string, "Unlicense") == 0) return ELX_PACKAGE_LICENSE_UNLICENSE;
    if (strcmp(string, "WTFPL") == 0) return ELX_PACKAGE_LICENSE_WTFPL;
    return ELX_PACKAGE_LICENSE_CUSTOM;
}

const char *elx_package_license_name(elx_package_license_t license) {
    switch (license) {
        case ELX_PACKAGE_LICENSE_MIT: return "MIT";
        case ELX_PACKAGE_LICENSE_APACHE2: return "Apache-2.0";
        case ELX_PACKAGE_LICENSE_GPL3: return "GPL-3.0";
        case ELX_PACKAGE_LICENSE_LGPL3: return "LGPL-3.0";
        case ELX_PACKAGE_LICENSE_BSD: return "BSD";
        case ELX_PACKAGE_LICENSE_MPL2: return "MPL-2.0";
        case ELX_PACKAGE_LICENSE_UNLICENSE: return "Unlicense";
        case ELX_PACKAGE_LICENSE_WTFPL: return "WTFPL";
        case ELX_PACKAGE_LICENSE_CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

elx_package_status_t elx_package_status_from_string(const char *string) {
    if (!string) return ELX_PACKAGE_STATUS_PRIVATE;
    if (strcmp(string, "public") == 0) return ELX_PACKAGE_STATUS_PUBLIC;
    if (strcmp(string, "verified") == 0) return ELX_PACKAGE_STATUS_VERIFIED;
    return ELX_PACKAGE_STATUS_PRIVATE;
}

const char *elx_package_status_name(elx_package_status_t status) {
    switch (status) {
        case ELX_PACKAGE_STATUS_PUBLIC: return "public";
        case ELX_PACKAGE_STATUS_VERIFIED: return "verified";
        default: return "private";
    }
}

bool elx_package_info_init(elx_package_info_t *info) {
    if (!info) return false;
    memset(info, 0, sizeof(*info));
    strncpy(info->version, "0.1.0", ELX_PACKAGE_MAX_VERSION - 1);
    info->license_type = ELX_PACKAGE_LICENSE_UNKNOWN;
    info->status = ELX_PACKAGE_STATUS_PRIVATE;
    info->sandbox = true;
    info->verified = false;
    return true;
}

void elx_package_info_clear(elx_package_info_t *info) {
    (void)info;
}

bool elx_package_info_parse(elx_package_info_t *info, const char *manifest_path) {
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
    char value[1024];
    char *p = buf;
    elx_package_info_init(info);
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
            if (strcmp(key, "name") == 0) { strncpy(info->name, value, ELX_PACKAGE_MAX_NAME - 1); }
            else if (strcmp(key, "version") == 0) { strncpy(info->version, value, ELX_PACKAGE_MAX_VERSION - 1); }
            else if (strcmp(key, "author") == 0) { strncpy(info->author, value, ELX_PACKAGE_MAX_AUTHOR - 1); }
            else if (strcmp(key, "description") == 0) { strncpy(info->description, value, ELX_PACKAGE_MAX_DESC - 1); }
            else if (strcmp(key, "license") == 0) {
                strncpy(info->license, value, ELX_PACKAGE_MAX_LICENSE - 1);
                info->license_type = elx_package_license_from_string(value);
            }
            else if (strcmp(key, "status") == 0) { info->status = elx_package_status_from_string(value); }
            else if (strcmp(key, "homepage") == 0) { strncpy(info->homepage, value, ELX_MODULE_MAX_PATH - 1); }
            else if (strcmp(key, "repository") == 0) { strncpy(info->repository, value, ELX_MODULE_MAX_PATH - 1); }
            else if (strcmp(key, "entry") == 0 || strcmp(key, "entry_point") == 0) { strncpy(info->entry_point, value, ELX_MODULE_MAX_PATH - 1); }
            else if (strcmp(key, "target") == 0) { strncpy(info->target, value, ELX_MODULE_MAX_PATH - 1); }
            else if (strcmp(key, "sandbox") == 0) { info->sandbox = (strcmp(value, "true") == 0); }
            else if (strcmp(key, "module_type") == 0 || strcmp(key, "type") == 0) { info->module_type = elx_module_type_from_string(value); }
        }
        while (*p && *p != ',' && *p != '}') p++;
        if (*p == ',') p++;
    }
    free(buf);
    return true;
}

bool elx_package_info_write(const elx_package_info_t *info, const char *manifest_path) {
    if (!info || !manifest_path) return false;
    FILE *f = fopen(manifest_path, "wb");
    if (!f) return false;
    time_t now = time(NULL);
    fprintf(f, "{\n");
    fprintf(f, "  \"name\": \"%s\",\n", info->name);
    fprintf(f, "  \"version\": \"%s\",\n", info->version);
    fprintf(f, "  \"author\": \"%s\",\n", info->author);
    fprintf(f, "  \"description\": \"%s\",\n", info->description);
    fprintf(f, "  \"license\": \"%s\",\n", elx_package_license_name(info->license_type));
    fprintf(f, "  \"status\": \"%s\",\n", elx_package_status_name(info->status));
    fprintf(f, "  \"homepage\": \"%s\",\n", info->homepage);
    fprintf(f, "  \"repository\": \"%s\",\n", info->repository);
    fprintf(f, "  \"entry\": \"%s\",\n", info->entry_point);
    fprintf(f, "  \"target\": \"%s\",\n", info->target);
    fprintf(f, "  \"sandbox\": %s,\n", info->sandbox ? "true" : "false");
    fprintf(f, "  \"created_at\": %llu,\n", (unsigned long long)now);
    fprintf(f, "  \"updated_at\": %llu\n", (unsigned long long)now);
    fprintf(f, "}\n");
    fclose(f);
    return true;
}

elx_package_registry_t *elx_package_registry_new(void) {
    elx_package_registry_t *r = (elx_package_registry_t *)calloc(1, sizeof(*r));
    if (r) {
        strncpy(r->registry_url, "https://elx.packages", ELX_MODULE_MAX_PATH - 1);
        strncpy(r->cache_dir, ".elx/packages", ELX_MODULE_MAX_PATH - 1);
    }
    return r;
}

void elx_package_registry_free(elx_package_registry_t *registry) {
    if (!registry) return;
    elx_package_registry_clear(registry);
    free(registry);
}

bool elx_package_registry_add(elx_package_registry_t *registry, const elx_package_info_t *info) {
    if (!registry || !info) return false;
    elx_package_entry_t *entry = (elx_package_entry_t *)calloc(1, sizeof(*entry));
    if (!entry) return false;
    entry->info = *info;
    entry->next = registry->head;
    registry->head = entry;
    registry->count++;
    return true;
}

elx_package_entry_t *elx_package_registry_find(const elx_package_registry_t *registry, const char *name) {
    if (!registry || !name) return NULL;
    for (elx_package_entry_t *e = registry->head; e; e = e->next) {
        if (strcmp(e->info.name, name) == 0) return e;
    }
    return NULL;
}

elx_package_entry_t *elx_package_registry_get(const elx_package_registry_t *registry, size_t index) {
    if (!registry) return NULL;
    size_t i = 0;
    for (elx_package_entry_t *e = registry->head; e; e = e->next) {
        if (i == index) return e;
        i++;
    }
    return NULL;
}

size_t elx_package_registry_count(const elx_package_registry_t *registry) {
    return registry ? registry->count : 0;
}

bool elx_package_registry_is_empty(const elx_package_registry_t *registry) {
    return registry ? registry->count == 0 : true;
}

void elx_package_registry_clear(elx_package_registry_t *registry) {
    if (!registry) return;
    elx_package_entry_t *e = registry->head;
    while (e) {
        elx_package_entry_t *next = e->next;
        elx_package_info_clear(&e->info);
        free(e);
        e = next;
    }
    registry->head = NULL;
    registry->count = 0;
}

void elx_package_registry_foreach(elx_package_registry_t *registry,
                                      void (*callback)(const elx_package_info_t *info, void *user),
                                      void *user) {
    if (!registry || !callback) return;
    for (elx_package_entry_t *e = registry->head; e; e = e->next) {
        callback(&e->info, user);
    }
}

bool elx_package_registry_set_url(elx_package_registry_t *registry, const char *url) {
    if (!registry || !url) return false;
    strncpy(registry->registry_url, url, ELX_MODULE_MAX_PATH - 1);
    return true;
}

const char *elx_package_registry_url(const elx_package_registry_t *registry) {
    return registry ? registry->registry_url : "";
}

bool elx_package_registry_set_cache_dir(elx_package_registry_t *registry, const char *dir) {
    if (!registry || !dir) return false;
    strncpy(registry->cache_dir, dir, ELX_MODULE_MAX_PATH - 1);
    return true;
}

const char *elx_package_registry_cache_dir(const elx_package_registry_t *registry) {
    return registry ? registry->cache_dir : "";
}

bool elx_package_validate(const elx_package_info_t *info, char *error, size_t error_size) {
    if (!info) {
        if (error && error_size > 0) strncpy(error, "package info is null", error_size - 1);
        return false;
    }
    if (strlen(info->name) == 0) {
        if (error && error_size > 0) strncpy(error, "package name is required", error_size - 1);
        return false;
    }
    if (strlen(info->version) == 0) {
        if (error && error_size > 0) strncpy(error, "package version is required", error_size - 1);
        return false;
    }
    if (error && error_size > 0) error[0] = '\0';
    return true;
}

bool elx_package_verify_signature(const elx_package_info_t *info) {
    (void)info;
    return true;
}

bool elx_package_resolve_dependencies(elx_package_registry_t *registry,
                                          const elx_package_info_t *package,
                                          char resolved[][ELX_MODULE_MAX_NAME],
                                          size_t *resolved_count) {
    if (!registry || !package || !resolved || !resolved_count) return false;
    *resolved_count = 0;
    for (size_t i = 0; i < package->dependency_count; i++) {
        int found = 0;
        for (size_t j = 0; j < *resolved_count; j++) {
            if (strcmp(resolved[j], package->dependencies[i]) == 0) { found = 1; break; }
        }
        if (!found) {
            strncpy(resolved[*resolved_count], package->dependencies[i], ELX_MODULE_MAX_NAME - 1);
            (*resolved_count)++;
        }
        elx_package_entry_t *dep = elx_package_registry_find(registry, package->dependencies[i]);
        if (dep) {
            if (!elx_package_resolve_dependencies(registry, &dep->info, resolved, resolved_count)) return false;
        }
    }
    return true;
}

bool elx_package_install(elx_package_registry_t *registry, const char *name,
                             const char *version_constraint, elx_package_progress_fn progress,
                             void *user) {
    if (!registry || !name) return false;
    if (progress) progress("installing", name, 0.0f, user);
    (void)version_constraint;
    if (progress) progress("downloading", name, 0.3f, user);
    if (progress) progress("extracting", name, 0.6f, user);
    if (progress) progress("configuring", name, 0.9f, user);
    if (progress) progress("complete", name, 1.0f, user);
    elx_package_info_t info;
    elx_package_info_init(&info);
    strncpy(info.name, name, ELX_PACKAGE_MAX_NAME - 1);
    strncpy(info.version, "1.0.0", ELX_PACKAGE_MAX_VERSION - 1);
    strncpy(info.target, "windows", ELX_MODULE_MAX_PATH - 1);
    elx_package_registry_add(registry, &info);
    return true;
}

bool elx_package_update(elx_package_registry_t *registry, const char *name,
                            elx_package_progress_fn progress, void *user) {
    if (!registry || !name) return false;
    elx_package_entry_t *entry = elx_package_registry_find(registry, name);
    if (!entry) return false;
    if (progress) progress("updating", name, 0.5f, user);
    if (progress) progress("complete", name, 1.0f, user);
    return true;
}

bool elx_package_remove(elx_package_registry_t *registry, const char *name) {
    if (!registry || !name) return false;
    elx_package_entry_t *prev = NULL;
    for (elx_package_entry_t *e = registry->head; e; prev = e, e = e->next) {
        if (strcmp(e->info.name, name) == 0) {
            if (prev) prev->next = e->next;
            else registry->head = e->next;
            elx_package_info_clear(&e->info);
            free(e);
            registry->count--;
            return true;
        }
    }
    return false;
}

bool elx_package_list(elx_package_registry_t *registry) {
    if (!registry) return false;
    if (registry->count == 0) { printf("No packages installed\n"); return true; }
    printf("%-30s %-12s %-15s\n", "Name", "Version", "Status");
    printf("------------------------------------------------------------\n");
    for (elx_package_entry_t *e = registry->head; e; e = e->next) {
        printf("%-30s %-12s %-15s\n", e->info.name, e->info.version,
               elx_package_status_name(e->info.status));
    }
    return true;
}

bool elx_package_search(elx_package_registry_t *registry, const char *query, size_t max_results) {
    (void)registry; (void)query; (void)max_results;
    return false;
}
