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

#include "elx_std_set.h"
#include <stdlib.h>

const char *elx_set_version(void) { return "0.1.0"; }

elx_set_t *elx_set_new(void) {
    elx_set_t *s = (elx_set_t *)calloc(1, sizeof(elx_set_t));
    if (!s) return NULL;
    s->map = elx_map_new(NULL, NULL);
    if (!s->map) { free(s); return NULL; }
    return s;
}

void elx_set_free(elx_set_t *set) {
    if (!set) return;
    elx_map_free(set->map);
    free(set);
}

bool elx_set_add(elx_set_t *set, const char *value) {
    if (!set || !value) return false;
    return elx_map_set(set->map, value, (void *)1);
}

bool elx_set_remove(elx_set_t *set, const char *value) {
    if (!set || !value) return false;
    return elx_map_remove(set->map, value);
}

bool elx_set_has(const elx_set_t *set, const char *value) {
    if (!set || !value) return false;
    return elx_map_has(set->map, value);
}

typedef struct {
    void (*callback)(const char *, size_t, void *);
    void *user;
} elx_set_foreach_ctx_t;

static void set_foreach_wrapper(const char *key, void *value, size_t index, void *user) {
    elx_set_foreach_ctx_t *ctx = (elx_set_foreach_ctx_t *)user;
    (void)value;
    ctx->callback(key, index, ctx->user);
}

void elx_set_foreach(const elx_set_t *set, void (*callback)(const char *, size_t, void *), void *user) {
    if (!set || !callback) return;
    elx_set_foreach_ctx_t ctx = { .callback = callback, .user = user };
    elx_map_foreach(set->map, set_foreach_wrapper, &ctx);
}

static void set_clone_wrapper(const char *key, void *value, size_t index, void *user) {
    (void)index; (void)value;
    elx_map_set((elx_map_t *)user, key, (void *)1);
}

elx_set_t *elx_set_clone(const elx_set_t *set) {
    if (!set) return NULL;
    elx_set_t *c = elx_set_new();
    if (!c) return NULL;
    elx_map_foreach(set->map, set_clone_wrapper, c->map);
    return c;
}

char **elx_set_values(const elx_set_t *set, size_t *out_count) {
    if (!set) return NULL;
    return elx_map_keys(set->map, out_count);
}

void elx_set_clear(elx_set_t *set) {
    if (!set) return;
    elx_map_clear(set->map);
}

size_t elx_set_count(const elx_set_t *set) { return set ? elx_map_count(set->map) : 0; }
bool elx_set_is_empty(const elx_set_t *set) { return !set || set->map->count == 0; }
