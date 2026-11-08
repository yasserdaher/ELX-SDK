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

#ifndef ELX_STD_MAP_H
#define ELX_STD_MAP_H

#include "elx_std.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *key;
    void *value;
} elx_map_entry_t;

typedef struct {
    elx_map_entry_t *entries;
    size_t count;
    size_t capacity;
    void (*free_key)(void *);
    void (*free_value)(void *);
} elx_map_t;

elx_map_t *elx_map_new(void (*free_key)(void *), void (*free_value)(void *));
void elx_map_free(elx_map_t *map);
bool elx_map_set(elx_map_t *map, const char *key, void *value);
bool elx_map_get(const elx_map_t *map, const char *key, void **out_value);
bool elx_map_get_or(const elx_map_t *map, const char *key, void *default_value, void **out_value);
bool elx_map_remove(elx_map_t *map, const char *key);
bool elx_map_has(const elx_map_t *map, const char *key);
size_t elx_map_index_of(const elx_map_t *map, const char *key);
void elx_map_foreach(const elx_map_t *map, void (*callback)(const char *key, void *value, size_t index, void *user), void *user);
elx_map_t *elx_map_clone(const elx_map_t *map);
char **elx_map_keys(const elx_map_t *map, size_t *out_count);
void **elx_map_values(const elx_map_t *map, size_t *out_count);
void elx_map_clear(elx_map_t *map);
size_t elx_map_count(const elx_map_t *map);
bool elx_map_is_empty(const elx_map_t *map);
const char *elx_map_version(void);

#define ELX_MAP_FOREACH(map, key, value) \
    if ((map) && (map)->count > 0) { \
        for (size_t _i = 0; _i < (map)->count; _i++) { \
            const char *key = (map)->entries[_i].key; \
            void *value = (map)->entries[_i].value; \
            (void)_i;

#define ELX_MAP_END_FOREACH } \
    }

#ifdef __cplusplus
}
#endif

#endif
