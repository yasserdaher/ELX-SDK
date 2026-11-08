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

#ifndef ELX_STD_ENV_H
#define ELX_STD_ENV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *elx_std_env_version(void);

const char *elx_env_get(const char *key);
bool elx_env_set(const char *key, const char *value);
bool elx_env_unset(const char *key);
size_t elx_env_count(void);
const char *elx_env_get_at(size_t index, const char **out_key);
const char *elx_env_get_home(size_t *out_length);
const char *elx_env_get_temp(size_t *out_length);
const char *elx_env_get_path(size_t *out_length);
const char *elx_env_get_user(size_t *out_length);
const char *elx_env_get_hostname(size_t *out_length);

bool elx_env_has(const char *key);
bool elx_env_is_empty(const char *key);

typedef void (*elx_env_foreach_fn)(const char *key, const char *value, void *context);
void elx_env_foreach(elx_env_foreach_fn callback, void *context);

bool elx_env_get_bool(const char *key, bool default_value);
int elx_env_get_int(const char *key, int default_value);
long elx_env_get_long(const char *key, long default_value);
double elx_env_get_float(const char *key, double default_value);
bool elx_env_set_bool(const char *key, bool value);
bool elx_env_set_int(const char *key, int value);
bool elx_env_set_long(const char *key, long value);
bool elx_env_set_float(const char *key, double value);

#ifdef __cplusplus
}
#endif

#endif
