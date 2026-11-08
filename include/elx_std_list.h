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

#ifndef ELX_STD_LIST_H
#define ELX_STD_LIST_H

#include "elx_std.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct elx_list_node {
    void *data;
    struct elx_list_node *next;
    struct elx_list_node *prev;
} elx_list_node_t;

typedef struct {
    elx_list_node_t *head;
    elx_list_node_t *tail;
    size_t count;
    void (*free_data)(void *);
} elx_list_t;

elx_list_t *elx_list_new(void (*free_data)(void *));
void elx_list_free(elx_list_t *list);
bool elx_list_push_front(elx_list_t *list, void *data);
bool elx_list_push_back(elx_list_t *list, void *data);
void *elx_list_pop_front(elx_list_t *list);
void *elx_list_pop_back(elx_list_t *list);
void *elx_list_get(const elx_list_t *list, size_t index);
bool elx_list_set(elx_list_t *list, size_t index, void *data);
void *elx_list_remove(elx_list_t *list, size_t index);
size_t elx_list_index_of(const elx_list_t *list, const void *data, int (*compare)(const void *, const void *));
bool elx_list_contains(const elx_list_t *list, const void *data, int (*compare)(const void *, const void *));
void elx_list_foreach(const elx_list_t *list, void (*callback)(void *, size_t index, void *user), void *user);
void elx_list_sort(elx_list_t *list, int (*compare)(const void *, const void *));
void elx_list_clear(elx_list_t *list);
size_t elx_list_count(const elx_list_t *list);
bool elx_list_is_empty(const elx_list_t *list);
elx_list_node_t *elx_list_first(const elx_list_t *list);
elx_list_node_t *elx_list_last(const elx_list_t *list);
elx_list_node_t *elx_list_next(elx_list_node_t *node);
elx_list_node_t *elx_list_prev(elx_list_node_t *node);
void *elx_list_node_data(elx_list_node_t *node);
const char *elx_list_version(void);

#ifdef __cplusplus
}
#endif

#endif
