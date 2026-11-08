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

#include "elx_std_str.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

const char *elx_str_version(void) { return "0.1.0"; }

elx_string_t *elx_str_new(void) {
    elx_string_t *s = (elx_string_t *)malloc(sizeof(elx_string_t));
    if (!s) return NULL;
    s->data = (char *)malloc(16);
    if (!s->data) { free(s); return NULL; }
    s->data[0] = '\0';
    s->length = 0;
    s->capacity = 16;
    return s;
}

elx_string_t *elx_str_from(const char *s) {
    if (!s) return elx_str_new();
    size_t len = strlen(s);
    elx_string_t *r = (elx_string_t *)malloc(sizeof(elx_string_t));
    if (!r) return NULL;
    r->data = (char *)malloc(len + 1);
    if (!r->data) { free(r); return NULL; }
    memcpy(r->data, s, len + 1);
    r->length = len;
    r->capacity = len + 1;
    return r;
}

elx_string_t *elx_str_from_n(const char *s, size_t n) {
    elx_string_t *r = (elx_string_t *)malloc(sizeof(elx_string_t));
    if (!r) return NULL;
    r->data = (char *)malloc(n + 1);
    if (!r->data) { free(r); return NULL; }
    memcpy(r->data, s, n);
    r->data[n] = '\0';
    r->length = n;
    r->capacity = n + 1;
    return r;
}

elx_string_t *elx_str_dup(const char *s) { return elx_str_from(s); }

void elx_str_free(elx_string_t *s) {
    if (!s) return;
    free(s->data);
    free(s);
}

void elx_str_clear(elx_string_t *s) {
    if (!s) return;
    s->data[0] = '\0';
    s->length = 0;
}

void elx_str_reserve(elx_string_t *s, size_t additional) {
    if (!s) return;
    size_t needed = s->length + additional + 1;
    if (needed <= s->capacity) return;
    char *nd = (char *)realloc(s->data, needed);
    if (nd) { s->data = nd; s->capacity = needed; }
}

void elx_str_append(elx_string_t *s, const char *data) {
    if (!s || !data) return;
    size_t len = strlen(data);
    elx_str_reserve(s, len);
    memcpy(s->data + s->length, data, len);
    s->length += len;
    s->data[s->length] = '\0';
}

void elx_str_append_n(elx_string_t *s, const char *data, size_t n) {
    if (!s || !data) return;
    elx_str_reserve(s, n);
    memcpy(s->data + s->length, data, n);
    s->length += n;
    s->data[s->length] = '\0';
}

void elx_str_append_char(elx_string_t *s, char c) {
    elx_str_reserve(s, 1);
    s->data[s->length++] = c;
    s->data[s->length] = '\0';
}

void elx_str_append_fmt(elx_string_t *s, const char *fmt, ...) {
    if (!s || !fmt) return;
    va_list args;
    va_start(args, fmt);
    va_list args2;
    va_copy(args2, args);
    int needed = vsnprintf(NULL, 0, fmt, args);
    if (needed < 0) { va_end(args); va_end(args2); return; }
    elx_str_reserve(s, (size_t)needed);
    vsnprintf(s->data + s->length, (size_t)needed + 1, fmt, args2);
    s->length += (size_t)needed;
    va_end(args);
    va_end(args2);
}

int elx_str_compare(const elx_string_t *a, const elx_string_t *b) {
    if (!a || !b) return (a ? 1 : (b ? -1 : 0));
    size_t ml = a->length < b->length ? a->length : b->length;
    int r = memcmp(a->data, b->data, ml);
    if (r != 0) return r;
    return (int)(a->length - b->length);
}

int elx_str_compare_c(const elx_string_t *a, const char *b) {
    if (!a || !b) return (a ? 1 : (b ? -1 : 0));
    return strcmp(a->data, b);
}

bool elx_str_equals(const elx_string_t *a, const char *b) {
    return elx_str_compare_c(a, b) == 0;
}

bool elx_str_starts_with(const elx_string_t *s, const char *prefix) {
    if (!s || !prefix) return false;
    size_t pl = strlen(prefix);
    return s->length >= pl && memcmp(s->data, prefix, pl) == 0;
}

bool elx_str_ends_with(const elx_string_t *s, const char *suffix) {
    if (!s || !suffix) return false;
    size_t sl = strlen(suffix);
    return s->length >= sl && memcmp(s->data + s->length - sl, suffix, sl) == 0;
}

char *elx_str_find(const elx_string_t *s, const char *needle) {
    if (!s || !s->data || !needle) return NULL;
    return strstr(s->data, needle);
}

char *elx_str_find_char(const elx_string_t *s, char c) {
    if (!s || !s->data) return NULL;
    return memchr(s->data, c, s->length);
}

char *elx_str_rfind_char(const elx_string_t *s, char c) {
    if (!s || !s->data || s->length == 0) return NULL;
    for (size_t i = s->length; i > 0; i--) {
        if (s->data[i - 1] == c) return s->data + i - 1;
    }
    return NULL;
}

elx_string_t *elx_str_slice(const elx_string_t *s, size_t start, size_t end) {
    if (!s || start > end || end > s->length) return NULL;
    return elx_str_from_n(s->data + start, end - start);
}

elx_string_t *elx_str_trim(const elx_string_t *s) {
    if (!s) return NULL;
    size_t start = 0, end = s->length;
    while (start < end && (s->data[start] == ' ' || s->data[start] == '\t' || s->data[start] == '\n' || s->data[start] == '\r')) start++;
    while (end > start && (s->data[end - 1] == ' ' || s->data[end - 1] == '\t' || s->data[end - 1] == '\n' || s->data[end - 1] == '\r')) end--;
    return elx_str_slice(s, start, end);
}

elx_string_t *elx_str_trim_left(const elx_string_t *s) {
    if (!s) return NULL;
    size_t start = 0;
    while (start < s->length && (s->data[start] == ' ' || s->data[start] == '\t' || s->data[start] == '\n' || s->data[start] == '\r')) start++;
    return elx_str_slice(s, start, s->length);
}

elx_string_t *elx_str_trim_right(const elx_string_t *s) {
    if (!s) return NULL;
    size_t end = s->length;
    while (end > 0 && (s->data[end - 1] == ' ' || s->data[end - 1] == '\t' || s->data[end - 1] == '\n' || s->data[end - 1] == '\r')) end--;
    return elx_str_slice(s, 0, end);
}

elx_string_t *elx_str_to_lower(const elx_string_t *s) {
    if (!s) return NULL;
    elx_string_t *r = elx_str_new();
    if (!r) return NULL;
    elx_str_reserve(r, s->length);
    for (size_t i = 0; i < s->length; i++) {
        char c = s->data[i];
        if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
        elx_str_append_char(r, c);
    }
    return r;
}

elx_string_t *elx_str_to_upper(const elx_string_t *s) {
    if (!s) return NULL;
    elx_string_t *r = elx_str_new();
    if (!r) return NULL;
    elx_str_reserve(r, s->length);
    for (size_t i = 0; i < s->length; i++) {
        char c = s->data[i];
        if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
        elx_str_append_char(r, c);
    }
    return r;
}

elx_string_t *elx_str_replace(const elx_string_t *s, const char *old, const char *new_str) {
    if (!s || !old || !new_str) return NULL;
    size_t ol = strlen(old);
    if (ol == 0) return elx_str_dup(s->data);
    elx_string_t *r = elx_str_new();
    if (!r) return NULL;
    size_t i = 0;
    while (i < s->length) {
        if (i + ol <= s->length && memcmp(s->data + i, old, ol) == 0) {
            elx_str_append(r, new_str);
            i += ol;
        } else {
            elx_str_append_char(r, s->data[i]);
            i++;
        }
    }
    return r;
}

elx_string_t *elx_str_join(const char *sep, const char **parts, size_t count) {
    if (!parts || count == 0) return elx_str_new();
    elx_string_t *r = elx_str_new();
    if (!r) return NULL;
    for (size_t i = 0; i < count; i++) {
        if (parts[i]) elx_str_append(r, parts[i]);
        if (i + 1 < count && sep) elx_str_append(r, sep);
    }
    return r;
}

elx_string_t *elx_str_format(const char *fmt, ...) {
    if (!fmt) return elx_str_new();
    va_list args;
    va_start(args, fmt);
    va_list args2;
    va_copy(args2, args);
    int needed = vsnprintf(NULL, 0, fmt, args);
    if (needed < 0) { va_end(args); va_end(args2); return elx_str_new(); }
    elx_string_t *r = elx_str_new();
    if (!r) { va_end(args); va_end(args2); return NULL; }
    elx_str_reserve(r, (size_t)needed);
    vsnprintf(r->data + r->length, (size_t)needed + 1, fmt, args2);
    r->length = (size_t)needed;
    va_end(args);
    va_end(args2);
    return r;
}

size_t elx_str_hash(const elx_string_t *s) {
    if (!s || !s->data) return 0;
    size_t h = 5381;
    for (size_t i = 0; i < s->length; i++) h = ((h << 5) + h) + (size_t)(unsigned char)s->data[i];
    return h;
}

size_t elx_str_hash_c(const char *s) {
    if (!s) return 0;
    size_t h = 5381;
    while (*s) h = ((h << 5) + h) + (size_t)(unsigned char)*(s++);
    return h;
}

void elx_str_split(const elx_string_t *s, char delimiter, char ***out_parts, size_t *out_count) {
    if (!s || !out_parts || !out_count) return;
    size_t count = 1;
    for (size_t i = 0; i < s->length; i++) {
        if (s->data[i] == delimiter) count++;
    }
    char **parts = (char **)calloc(count, sizeof(char *));
    if (!parts) { *out_parts = NULL; *out_count = 0; return; }
    size_t start = 0, pi = 0;
    for (size_t i = 0; i <= s->length; i++) {
        if (i == s->length || s->data[i] == delimiter) {
            size_t len = i - start;
            parts[pi] = (char *)malloc(len + 1);
            if (parts[pi]) {
                memcpy(parts[pi], s->data + start, len);
                parts[pi][len] = '\0';
            }
            pi++;
            start = i + 1;
        }
    }
    *out_parts = parts;
    *out_count = count;
}

void elx_str_free_parts(char **parts, size_t count) {
    if (!parts) return;
    for (size_t i = 0; i < count; i++) free(parts[i]);
    free(parts);
}

size_t elx_str_count(const elx_string_t *s, char c) {
    if (!s || !s->data) return 0;
    size_t count = 0;
    for (size_t i = 0; i < s->length; i++) {
        if (s->data[i] == c) count++;
    }
    return count;
}

bool elx_str_is_empty(const elx_string_t *s) {
    return !s || s->length == 0;
}

const char *elx_str_c_str(const elx_string_t *s) {
    return s ? s->data : "";
}

size_t elx_str_capacity(const elx_string_t *s) {
    return s ? s->capacity : 0;
}

void elx_str_shrink_to_fit(elx_string_t *s) {
    if (!s || s->length == 0) return;
    char *nd = (char *)realloc(s->data, s->length + 1);
    if (nd) { s->data = nd; s->capacity = s->length + 1; }
}
