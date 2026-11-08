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

#include "elx_std_map.h"
#include <stdlib.h>
#include <string.h>

const char *elx_map_version(void) { return "0.1.0"; }

elx_map_t *elx_map_new(void (*free_key)(void *), void (*free_value)(void *)) {
    elx_map_t *m = (elx_map_t *)calloc(1, sizeof(elx_map_t));
    if (!m) return NULL;
    m->free_key = free_key;
    m->free_value = free_value;
    m->capacity = 16;
    m->entries = (elx_map_entry_t *)calloc(m->capacity, sizeof(elx_map_entry_t));
    if (!m->entries) { free(m); return NULL; }
    return m;
}

void elx_map_free(elx_map_t *map) {
    if (!map) return;
    elx_map_clear(map);
    free(map->entries);
    free(map);
}

static void map_grow(elx_map_t *map) {
    size_t new_cap = map->capacity * 2;
    elx_map_entry_t *ne = (elx_map_entry_t *)calloc(new_cap, sizeof(elx_map_entry_t));
    if (!ne) return;
    memcpy(ne, map->entries, map->count * sizeof(elx_map_entry_t));
    free(map->entries);
    map->entries = ne;
    map->capacity = new_cap;
}

bool elx_map_set(elx_map_t *map, const char *key, void *value) {
    if (!map || !key) return false;
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0) {
            if (map->free_value && map->entries[i].value) map->free_value(map->entries[i].value);
            map->entries[i].value = value;
            return true;
        }
    }
    if (map->count >= map->capacity) map_grow(map);
    map->entries[map->count].key = key;
    map->entries[map->count].value = value;
    map->count++;
    return true;
}

bool elx_map_get(const elx_map_t *map, const char *key, void **out_value) {
    if (!map || !key) return false;
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0) {
            if (out_value) *out_value = map->entries[i].value;
            return true;
        }
    }
    return false;
}

bool elx_map_get_or(const elx_map_t *map, const char *key, void *default_value, void **out_value) {
    if (!map || !key) return false;
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0) {
            if (out_value) *out_value = map->entries[i].value;
            return true;
        }
    }
    if (out_value) *out_value = default_value;
    return false;
}

bool elx_map_remove(elx_map_t *map, const char *key) {
    if (!map || !key) return false;
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0) {
            if (map->free_key && map->entries[i].key) map->free_key((void *)map->entries[i].key);
            if (map->free_value && map->entries[i].value) map->free_value(map->entries[i].value);
            memmove(&map->entries[i], &map->entries[i + 1], (map->count - i - 1) * sizeof(elx_map_entry_t));
            map->count--;
            return true;
        }
    }
    return false;
}

bool elx_map_has(const elx_map_t *map, const char *key) {
    void *v;
    return elx_map_get(map, key, &v);
}

size_t elx_map_index_of(const elx_map_t *map, const char *key) {
    if (!map || !key) return SIZE_MAX;
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0) return i;
    }
    return SIZE_MAX;
}

void elx_map_foreach(const elx_map_t *map, void (*callback)(const char *, void *, size_t, void *), void *user) {
    if (!map || !callback) return;
    for (size_t i = 0; i < map->count; i++) {
        callback(map->entries[i].key, map->entries[i].value, i, user);
    }
}

elx_map_t *elx_map_clone(const elx_map_t *map) {
    if (!map) return NULL;
    elx_map_t *c = elx_map_new(map->free_key, map->free_value);
    if (!c) return NULL;
    for (size_t i = 0; i < map->count; i++) {
        elx_map_set(c, map->entries[i].key, map->entries[i].value);
    }
    return c;
}

char **elx_map_keys(const elx_map_t *map, size_t *out_count) {
    if (!map) return NULL;
    char **keys = (char **)calloc(map->count, sizeof(char *));
    if (!keys) return NULL;
    for (size_t i = 0; i < map->count; i++) keys[i] = (char *)map->entries[i].key;
    if (out_count) *out_count = map->count;
    return keys;
}

void **elx_map_values(const elx_map_t *map, size_t *out_count) {
    if (!map) return NULL;
    void **vals = (void **)calloc(map->count, sizeof(void *));
    if (!vals) return NULL;
    for (size_t i = 0; i < map->count; i++) vals[i] = map->entries[i].value;
    if (out_count) *out_count = map->count;
    return vals;
}

void elx_map_clear(elx_map_t *map) {
    if (!map) return;
    for (size_t i = 0; i < map->count; i++) {
        if (map->free_key && map->entries[i].key) map->free_key((void *)map->entries[i].key);
        if (map->free_value && map->entries[i].value) map->free_value(map->entries[i].value);
    }
    map->count = 0;
}

size_t elx_map_count(const elx_map_t *map) { return map ? map->count : 0; }
bool elx_map_is_empty(const elx_map_t *map) { return !map || map->count == 0; }
