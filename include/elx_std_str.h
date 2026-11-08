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

#ifndef ELX_STD_STR_H
#define ELX_STD_STR_H

#include "elx_std.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} elx_string_t;

elx_string_t *elx_str_new(void);
elx_string_t *elx_str_from(const char *s);
elx_string_t *elx_str_from_n(const char *s, size_t n);
elx_string_t *elx_str_dup(const char *s);
void elx_str_free(elx_string_t *s);
void elx_str_clear(elx_string_t *s);
void elx_str_reserve(elx_string_t *s, size_t additional);
void elx_str_append(elx_string_t *s, const char *data);
void elx_str_append_n(elx_string_t *s, const char *data, size_t n);
void elx_str_append_char(elx_string_t *s, char c);
void elx_str_append_fmt(elx_string_t *s, const char *fmt, ...);
int elx_str_compare(const elx_string_t *a, const elx_string_t *b);
int elx_str_compare_c(const elx_string_t *a, const char *b);
bool elx_str_equals(const elx_string_t *a, const char *b);
bool elx_str_starts_with(const elx_string_t *s, const char *prefix);
bool elx_str_ends_with(const elx_string_t *s, const char *suffix);
char *elx_str_find(const elx_string_t *s, const char *needle);
char *elx_str_find_char(const elx_string_t *s, char c);
char *elx_str_rfind_char(const elx_string_t *s, char c);
elx_string_t *elx_str_slice(const elx_string_t *s, size_t start, size_t end);
elx_string_t *elx_str_trim(const elx_string_t *s);
elx_string_t *elx_str_trim_left(const elx_string_t *s);
elx_string_t *elx_str_trim_right(const elx_string_t *s);
elx_string_t *elx_str_to_lower(const elx_string_t *s);
elx_string_t *elx_str_to_upper(const elx_string_t *s);
elx_string_t *elx_str_replace(const elx_string_t *s, const char *old, const char *new);
elx_string_t *elx_str_join(const char *sep, const char **parts, size_t count);
elx_string_t *elx_str_format(const char *fmt, ...);
size_t elx_str_hash(const elx_string_t *s);
size_t elx_str_hash_c(const char *s);
void elx_str_split(const elx_string_t *s, char delimiter, char ***out_parts, size_t *out_count);
void elx_str_free_parts(char **parts, size_t count);
size_t elx_str_count(const elx_string_t *s, char c);
bool elx_str_is_empty(const elx_string_t *s);
const char *elx_str_c_str(const elx_string_t *s);
size_t elx_str_capacity(const elx_string_t *s);
void elx_str_shrink_to_fit(elx_string_t *s);
const char *elx_str_version(void);

#ifdef __cplusplus
}
#endif

#endif
