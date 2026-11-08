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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "elx_std_env.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
const char *elx_std_env_version(void) { return "0.1.0"; }
const char *elx_env_get(const char *key) { if (!key) return NULL; return getenv(key); }
bool elx_env_set(const char *key, const char *value) {
    if (!key) return false;
    if (value) return setenv(key, value, 1) == 0;
    return unsetenv(key) == 0;
}
bool elx_env_unset(const char *key) { if (!key) return false; return unsetenv(key) == 0; }
size_t elx_env_count(void) { char **env = environ; size_t c = 0; while (env && *env) { c++; env++; } return c; }
const char *elx_env_get_at(size_t index, const char **out_key) {
    char **env = environ;
    for (size_t i = 0; i < index && env && *env; i++) env++;
    if (!env || !*env) return NULL;
    if (out_key) *out_key = *env;
    return *env;
}
const char *elx_env_get_home(size_t *out_length) {
    const char *home = getenv("HOME");
#if defined(_WIN32)
    if (!home) home = getenv("USERPROFILE");
#endif
    if (home && out_length) *out_length = strlen(home);
    return home;
}
const char *elx_env_get_temp(size_t *out_length) {
    const char *temp = getenv("TMPDIR");
    if (!temp) temp = getenv("TEMP");
    if (!temp) temp = getenv("TMP");
    if (!temp) temp = "/tmp";
    if (out_length && temp) *out_length = strlen(temp);
    return temp;
}
const char *elx_env_get_path(size_t *out_length) {
    const char *path = getenv("PATH");
    if (path && out_length) *out_length = strlen(path);
    return path;
}
const char *elx_env_get_user(size_t *out_length) {
    const char *user = getenv("USER");
#if defined(_WIN32)
    if (!user) user = getenv("USERNAME");
#endif
    if (user && out_length) *out_length = strlen(user);
    return user;
}
const char *elx_env_get_hostname(size_t *out_length) { (void)out_length; return NULL; }
bool elx_env_has(const char *key) { return elx_env_get(key) != NULL; }
bool elx_env_is_empty(const char *key) { const char *v = elx_env_get(key); return !v || strlen(v) == 0; }
void elx_env_foreach(elx_env_foreach_fn callback, void *context) {
    if (!callback) return;
    char **env = environ;
    while (env && *env) {
        char *eq = strchr(*env, '=');
        if (eq) {
            size_t key_len = (size_t)(eq - *env);
            char key[256];
            if (key_len < sizeof(key)) {
                memcpy(key, *env, key_len); key[key_len] = '\0';
                callback(key, eq + 1, context);
            }
        }
        env++;
    }
}
bool elx_env_get_bool(const char *key, bool default_value) {
    const char *value = elx_env_get(key);
    if (!value) return default_value;
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "yes") == 0) return true;
    if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0 || strcmp(value, "no") == 0) return false;
    return default_value;
}
int elx_env_get_int(const char *key, int default_value) {
    const char *value = elx_env_get(key);
    if (!value) return default_value;
    return (int)strtol(value, NULL, 10);
}
long elx_env_get_long(const char *key, long default_value) {
    const char *value = elx_env_get(key);
    if (!value) return default_value;
    return (long)strtol(value, NULL, 10);
}
double elx_env_get_float(const char *key, double default_value) {
    const char *value = elx_env_get(key);
    if (!value) return default_value;
    return strtod(value, NULL);
}
bool elx_env_set_bool(const char *key, bool value) { return elx_env_set(key, value ? "true" : "false"); }
bool elx_env_set_int(const char *key, int value) { char buf[32]; snprintf(buf, sizeof(buf), "%d", value); return elx_env_set(key, buf); }
bool elx_env_set_long(const char *key, long value) { char buf[64]; snprintf(buf, sizeof(buf), "%ld", value); return elx_env_set(key, buf); }
bool elx_env_set_float(const char *key, double value) { char buf[64]; snprintf(buf, sizeof(buf), "%f", value); return elx_env_set(key, buf); }
