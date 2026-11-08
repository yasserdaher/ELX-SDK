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

#ifndef ELX_SDK_SECURITY_H
#define ELX_SDK_SECURITY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "elx_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ELX_SEC_OK = 0,
    ELX_SEC_ERROR,
    ELX_SEC_SIGNATURE_INVALID,
    ELX_SEC_SIGNATURE_MISSING,
    ELX_SEC_PERMISSION_DENIED,
    ELX_SEC_SANDBOX_VIOLATION,
    ELX_SEC_PATH_TRAVERSAL,
    ELX_SEC_PERMISSION_NOT_DECLARED,
} elx_sec_status_t;

typedef enum {
    ELX_PERM_FILE_READ = 1,
    ELX_PERM_FILE_WRITE = 2,
    ELX_PERM_FILE_EXECUTE = 4,
    ELX_PERM_NETWORK_CONNECT = 8,
    ELX_PERM_NETWORK_BIND = 16,
    ELX_PERM_NETWORK_LISTEN = 32,
    ELX_PERM_PROCESS_SPAWN = 64,
    ELX_PERM_PROCESS_KILL = 128,
    ELX_PERM_ENV_READ = 256,
    ELX_PERM_ENV_WRITE = 512,
    ELX_PERM_ALL = 0x1ff,
} elx_permission_t;

typedef struct {
    elx_permission_t permission;
    bool required;
    const char *resource_pattern;
} elx_perm_constraint_t;

typedef struct {
    elx_perm_constraint_t *constraints;
    size_t count;
    size_t capacity;
} elx_sec_policy_t;

typedef struct {
    unsigned char hash[32];
    size_t hash_len;
    char *signed_by;
    char *signature;
    size_t signature_len;
    bool valid;
} elx_module_signature_t;

const char *elx_sdk_security_version(void);

elx_sec_status_t elx_sec_policy_create(elx_sec_policy_t *policy);
void elx_sec_policy_destroy(elx_sec_policy_t *policy);
elx_sec_status_t elx_sec_policy_add_constraint(elx_sec_policy_t *policy,
                                                elx_permission_t perm,
                                                bool required,
                                                const char *resource_pattern);
bool elx_sec_policy_check_permission(const elx_sec_policy_t *policy,
                                      elx_permission_t perm,
                                      const char *resource);

elx_sec_status_t elx_sec_sign_module(const char *module_path,
                                      const char *private_key_path,
                                      const char *output_path);
elx_sec_status_t elx_sec_verify_module(const char *module_path,
                                        const char *public_key_path);
elx_sec_status_t elx_sec_verify_module_signature(const char *module_path);

elx_sec_status_t elx_sec_sandbox_init(const char *module_path,
                                       const elx_sec_policy_t *policy);
elx_sec_status_t elx_sec_sandbox_check_path(const char *path,
                                             elx_permission_t perm);
elx_sec_status_t elx_sec_sandbox_check_network(const char *host,
                                                 uint16_t port,
                                                 elx_permission_t perm);
void elx_sec_sandbox_destroy(void);

elx_sec_status_t elx_sec_validate_manifest(const char *manifest_path,
                                            const char *manifest_json);
bool elx_sec_manifest_has_permission(const char *manifest_json,
                                      elx_permission_t perm);

elx_sec_status_t elx_sec_hash_file_sha256(const char *path,
                                          unsigned char *out_hash,
                                          size_t hash_size);

const char *elx_sec_status_string(elx_sec_status_t status);

bool elx_sec_is_path_safe(const char *path);
bool elx_sec_sanitize_path(char *path, size_t max_len);

bool elx_sec_is_compiler_trusted(void);
bool elx_sec_is_runtime_trusted(void);

#ifdef __cplusplus
}
#endif

#endif
