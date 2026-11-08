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

#include "elx_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void elx_error_init(elx_error_list_t *list) { if (!list) return; list->items = NULL; list->count = 0; list->capacity = 0; list->has_errors = false; }
void elx_error_destroy(elx_error_list_t *list) { if (!list) return; free(list->items); list->items = NULL; list->count = 0; list->capacity = 0; }
void elx_error_add(elx_error_list_t *list, int code, const char *message, size_t line, size_t column) {
    if (!list || !message) return;
    if (list->count == list->capacity) { size_t cap = list->capacity ? list->capacity * 2 : 16; elx_error_t *items = (elx_error_t *)realloc(list->items, cap * sizeof(*items)); if (!items) return; list->items = items; list->capacity = cap; }
    list->items[list->count].code = code; list->items[list->count].message = message; list->items[list->count].line = line; list->items[list->count].column = column; list->count++;
}
void elx_error_print(const elx_error_list_t *list, const char *source_name, FILE *stream) {
    if (!list || !stream) return;
    for (size_t i = 0; i < list->count; i++) { const elx_error_t *d = &list->items[i]; fprintf(stream, "%s:%zu:%zu: error %d: %s\n", source_name ? source_name : "<unknown>", d->line, d->column, d->code, d->message); }
}
