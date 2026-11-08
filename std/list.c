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

#include "elx_std_list.h"
#include <stdlib.h>
#include <string.h>

const char *elx_list_version(void) { return "0.1.0"; }

elx_list_t *elx_list_new(void (*free_data)(void *)) {
    elx_list_t *l = (elx_list_t *)calloc(1, sizeof(elx_list_t));
    if (!l) return NULL;
    l->free_data = free_data;
    return l;
}

void elx_list_free(elx_list_t *list) {
    if (!list) return;
    elx_list_clear(list);
    free(list);
}

bool elx_list_push_front(elx_list_t *list, void *data) {
    if (!list) return false;
    elx_list_node_t *n = (elx_list_node_t *)calloc(1, sizeof(elx_list_node_t));
    if (!n) return false;
    n->data = data;
    n->next = list->head;
    if (list->head) list->head->prev = n;
    list->head = n;
    if (!list->tail) list->tail = n;
    list->count++;
    return true;
}

bool elx_list_push_back(elx_list_t *list, void *data) {
    if (!list) return false;
    elx_list_node_t *n = (elx_list_node_t *)calloc(1, sizeof(elx_list_node_t));
    if (!n) return false;
    n->data = data;
    n->prev = list->tail;
    if (list->tail) list->tail->next = n;
    list->tail = n;
    if (!list->head) list->head = n;
    list->count++;
    return true;
}

void *elx_list_pop_front(elx_list_t *list) {
    if (!list || !list->head) return NULL;
    elx_list_node_t *n = list->head;
    void *data = n->data;
    list->head = n->next;
    if (list->head) list->head->prev = NULL;
    else list->tail = NULL;
    if (list->free_data) list->free_data(data);
    free(n);
    list->count--;
    return data;
}

void *elx_list_pop_back(elx_list_t *list) {
    if (!list || !list->tail) return NULL;
    elx_list_node_t *n = list->tail;
    void *data = n->data;
    list->tail = n->prev;
    if (list->tail) list->tail->next = NULL;
    else list->head = NULL;
    if (list->free_data) list->free_data(data);
    free(n);
    list->count--;
    return data;
}

void *elx_list_get(const elx_list_t *list, size_t index) {
    if (!list || index >= list->count) return NULL;
    elx_list_node_t *n = list->head;
    for (size_t i = 0; i < index; i++) n = n->next;
    return n->data;
}

bool elx_list_set(elx_list_t *list, size_t index, void *data) {
    if (!list || index >= list->count) return false;
    elx_list_node_t *n = list->head;
    for (size_t i = 0; i < index; i++) n = n->next;
    if (list->free_data) list->free_data(n->data);
    n->data = data;
    return true;
}

void *elx_list_remove(elx_list_t *list, size_t index) {
    if (!list || index >= list->count) return NULL;
    elx_list_node_t *n = list->head;
    for (size_t i = 0; i < index; i++) n = n->next;
    void *data = n->data;
    if (n->prev) n->prev->next = n->next;
    else list->head = n->next;
    if (n->next) n->next->prev = n->prev;
    else list->tail = n->prev;
    if (list->free_data) list->free_data(data);
    free(n);
    list->count--;
    return data;
}

size_t elx_list_index_of(const elx_list_t *list, const void *data, int (*compare)(const void *, const void *)) {
    if (!list || !compare) return SIZE_MAX;
    size_t i = 0;
    for (elx_list_node_t *n = list->head; n; n = n->next, i++) {
        if (compare(n->data, data) == 0) return i;
    }
    return SIZE_MAX;
}

bool elx_list_contains(const elx_list_t *list, const void *data, int (*compare)(const void *, const void *)) {
    return elx_list_index_of(list, data, compare) != SIZE_MAX;
}

void elx_list_foreach(const elx_list_t *list, void (*callback)(void *, size_t, void *), void *user) {
    if (!list || !callback) return;
    size_t i = 0;
    for (elx_list_node_t *n = list->head; n; n = n->next, i++) {
        callback(n->data, i, user);
    }
}

static elx_list_node_t *merge_sort(elx_list_node_t *head, int (*compare)(const void *, const void *)) {
    if (!head || !head->next) return head;
    elx_list_node_t *slow = head, *fast = head->next;
    while (fast && fast->next) { slow = slow->next; fast = fast->next->next; }
    elx_list_node_t *mid = slow->next;
    slow->next = NULL;
    head = merge_sort(head, compare);
    mid = merge_sort(mid, compare);
    elx_list_node_t dummy = {0};
    elx_list_node_t *tail = &dummy;
    while (head && mid) {
        if (compare(head->data, mid->data) <= 0) { tail->next = head; head = head->next; }
        else { tail->next = mid; mid = mid->next; }
        tail = tail->next;
    }
    tail->next = head ? head : mid;
    return dummy.next;
}

void elx_list_sort(elx_list_t *list, int (*compare)(const void *, const void *)) {
    if (!list || !list->head || !compare) return;
    list->head = merge_sort(list->head, compare);
    list->tail = list->head;
    while (list->tail && list->tail->next) list->tail = list->tail->next;
}

void elx_list_clear(elx_list_t *list) {
    if (!list) return;
    elx_list_node_t *n = list->head;
    while (n) {
        elx_list_node_t *next = n->next;
        if (list->free_data) list->free_data(n->data);
        free(n);
        n = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

size_t elx_list_count(const elx_list_t *list) { return list ? list->count : 0; }
bool elx_list_is_empty(const elx_list_t *list) { return !list || list->count == 0; }
elx_list_node_t *elx_list_first(const elx_list_t *list) { return list ? list->head : NULL; }
elx_list_node_t *elx_list_last(const elx_list_t *list) { return list ? list->tail : NULL; }
elx_list_node_t *elx_list_next(elx_list_node_t *node) { return node ? node->next : NULL; }
elx_list_node_t *elx_list_prev(elx_list_node_t *node) { return node ? node->prev : NULL; }
void *elx_list_node_data(elx_list_node_t *node) { return node ? node->data : NULL; }
