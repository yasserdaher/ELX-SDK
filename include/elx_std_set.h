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

#ifndef ELX_STD_SET_H
#define ELX_STD_SET_H

#include "elx_std_map.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    elx_map_t *map;
} elx_set_t;

elx_set_t *elx_set_new(void);
void elx_set_free(elx_set_t *set);
bool elx_set_add(elx_set_t *set, const char *value);
bool elx_set_remove(elx_set_t *set, const char *value);
bool elx_set_has(const elx_set_t *set, const char *value);
void elx_set_foreach(const elx_set_t *set, void (*callback)(const char *value, size_t index, void *user), void *user);
elx_set_t *elx_set_clone(const elx_set_t *set);
char **elx_set_values(const elx_set_t *set, size_t *out_count);
void elx_set_clear(elx_set_t *set);
size_t elx_set_count(const elx_set_t *set);
bool elx_set_is_empty(const elx_set_t *set);
const char *elx_set_version(void);

#ifdef __cplusplus
}
#endif

#endif
