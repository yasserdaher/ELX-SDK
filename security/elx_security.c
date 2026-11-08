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

#define _POSIX_C_SOURCE 200809L
#include "elx_sdk_security.h"
#include "elx_std_fs.h"
#include "elx_std_env.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
    #include <windows.h>
    #include <bcrypt.h>
    #pragma comment(lib, "bcrypt.lib")
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <fcntl.h>
#endif

const char *elx_sdk_security_version(void) { return "0.1.0"; }

const char *elx_sec_status_string(elx_sec_status_t status) {
    switch (status) {
        case ELX_SEC_OK: return "ok";
        case ELX_SEC_ERROR: return "error";
        case ELX_SEC_SIGNATURE_INVALID: return "signature invalid";
        case ELX_SEC_SIGNATURE_MISSING: return "signature missing";
        case ELX_SEC_PERMISSION_DENIED: return "permission denied";
        case ELX_SEC_SANDBOX_VIOLATION: return "sandbox violation";
        case ELX_SEC_PATH_TRAVERSAL: return "path traversal";
        case ELX_SEC_PERMISSION_NOT_DECLARED: return "permission not declared";
        default: return "unknown";
    }
}

elx_sec_status_t elx_sec_policy_create(elx_sec_policy_t *policy) {
    if (!policy) return ELX_SEC_ERROR;
    policy->constraints = NULL;
    policy->count = 0;
    policy->capacity = 0;
    return ELX_SEC_OK;
}

void elx_sec_policy_destroy(elx_sec_policy_t *policy) {
    if (!policy) return;
    for (size_t i = 0; i < policy->count; ++i) {
        free((void *)policy->constraints[i].resource_pattern);
    }
    free(policy->constraints);
    policy->constraints = NULL;
    policy->count = 0;
    policy->capacity = 0;
}

elx_sec_status_t elx_sec_policy_add_constraint(elx_sec_policy_t *policy,
                                                elx_permission_t perm,
                                                bool required,
                                                const char *resource_pattern) {
    if (!policy) return ELX_SEC_ERROR;
    if (policy->count >= policy->capacity) {
        size_t new_cap = (policy->capacity == 0) ? 16 : policy->capacity * 2;
        elx_perm_constraint_t *new_constraints =
            (elx_perm_constraint_t *)realloc(policy->constraints,
                                             new_cap * sizeof(*new_constraints));
        if (!new_constraints) return ELX_SEC_ERROR;
        policy->constraints = new_constraints;
        policy->capacity = new_cap;
    }
    elx_perm_constraint_t *c = &policy->constraints[policy->count];
    c->permission = perm;
    c->required = required;
    c->resource_pattern = resource_pattern ? strdup(resource_pattern) : NULL;
    policy->count++;
    return ELX_SEC_OK;
}

static bool pattern_matches(const char *pattern, const char *path) {
    if (!pattern || !path) return false;
    if (strcmp(pattern, "*") == 0) return true;
    if (strcmp(pattern, path) == 0) return true;
    size_t plen = strlen(pattern);
    if (plen > 0 && pattern[plen - 1] == '*') {
        size_t prefix_len = plen - 1;
        return strncmp(path, pattern, prefix_len) == 0;
    }
    return false;
}

bool elx_sec_policy_check_permission(const elx_sec_policy_t *policy,
                                      elx_permission_t perm,
                                      const char *resource) {
    if (!policy) return false;
    for (size_t i = 0; i < policy->count; ++i) {
        if (policy->constraints[i].permission & perm) {
            if (policy->constraints[i].required) {
                const char *pat = policy->constraints[i].resource_pattern;
                if (!pat || pattern_matches(pat, resource))
                    return true;
            }
        }
    }
    return false;
}

static elx_sec_policy_t g_current_policy;
static bool g_sandbox_active = false;

bool elx_sec_is_path_safe(const char *path) {
    if (!path) return false;
    if (strstr(path, "..") != NULL) return false;
    if (strstr(path, "//") != NULL) return false;
    return true;
}

bool elx_sec_sanitize_path(char *path, size_t max_len) {
    if (!path || max_len == 0) return false;
    char *src = path;
    char *dst = path;
    size_t remaining = max_len - 1;
    while (*src && remaining > 0) {
        if (*src == '.' && *(src + 1) == '.') {
            if (dst > path && *(dst - 1) == '/') dst--;
            while (dst > path && *(dst - 1) != '/') dst--;
            src += 2;
            while (*src == '/' || *src == '.') src++;
        } else {
            *dst++ = *src++;
            remaining--;
        }
    }
    *dst = '\0';
    return true;
}

elx_sec_status_t elx_sec_sandbox_init(const char *module_path,
                                       const elx_sec_policy_t *policy) {
    (void)module_path;
    if (g_sandbox_active) return ELX_SEC_ERROR;
    elx_sec_policy_create(&g_current_policy);
    if (policy) {
        for (size_t i = 0; i < policy->count; ++i) {
            elx_sec_policy_add_constraint(&g_current_policy,
                                           policy->constraints[i].permission,
                                           policy->constraints[i].required,
                                           policy->constraints[i].resource_pattern);
        }
    }
    g_sandbox_active = true;
    return ELX_SEC_OK;
}

elx_sec_status_t elx_sec_sandbox_check_path(const char *path,
                                             elx_permission_t perm) {
    if (!g_sandbox_active) return ELX_SEC_OK;
    if (!elx_sec_is_path_safe(path)) return ELX_SEC_PATH_TRAVERSAL;
    if (!elx_sec_policy_check_permission(&g_current_policy, perm, path))
        return ELX_SEC_PERMISSION_DENIED;
    return ELX_SEC_OK;
}

elx_sec_status_t elx_sec_sandbox_check_network(const char *host,
                                                 uint16_t port,
                                                 elx_permission_t perm) {
    (void)host;
    (void)port;
    if (!g_sandbox_active) return ELX_SEC_OK;
    if (!elx_sec_policy_check_permission(&g_current_policy, perm, "*"))
        return ELX_SEC_PERMISSION_DENIED;
    return ELX_SEC_OK;
}

void elx_sec_sandbox_destroy(void) {
    if (g_sandbox_active) {
        elx_sec_policy_destroy(&g_current_policy);
        g_sandbox_active = false;
    }
}

bool elx_sec_is_compiler_trusted(void) {
    const char *trusted = getenv("ELX_TRUST_COMPILER");
    if (trusted && (strcmp(trusted, "1") == 0 || strcmp(trusted, "yes") == 0)) return true;
    if (getenv("ELX_DISABLE_TRUST")) return false;
    return true;
}

bool elx_sec_is_runtime_trusted(void) {
    const char *trusted = getenv("ELX_TRUST_RUNTIME");
    if (trusted && (strcmp(trusted, "1") == 0 || strcmp(trusted, "yes") == 0)) return true;
    if (getenv("ELX_DISABLE_TRUST")) return false;
    return true;
}

static void sha256_transform(uint32_t *state, const uint8_t *block) {
    static const uint32_t k[] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc7,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x24009560,
        0x2de92c6f, 0x4a7481f8, 0x5c0ff912, 0x756850bc,
        0x71c4913c, 0x7cc9d0e0, 0x8707c437, 0x948cd17f,
        0x9f4d56e3, 0xa25e33e4, 0xb06a37e6, 0xbf548fcc,
        0xc0acb54e, 0xc8e51520, 0xd4836d30, 0xd9d6a1f0,
        0xe25e2f0f, 0xf0e06809, 0xf6014b19, 0xf9c7a2a5,
        0xfe2ce6e0, 0xfe2c60f5, 0xfb07a5c7, 0xfb6a4e2c
    };
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
    uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = ((uint32_t)block[i * 4] << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8) |
               (uint32_t)block[i * 4 + 3];
    }
    for (int i = 16; i < 64; ++i) {
        uint32_t s0 = ((w[i-15] >> 7) | (w[i-15] << 25)) ^
                      ((w[i-15] >> 18) | (w[i-15] << 14)) ^
                      (w[i-15] >> 3);
        uint32_t s1 = ((w[i-2] >> 17) | (w[i-2] << 15)) ^
                      ((w[i-2] >> 19) | (w[i-2] << 13)) ^
                      (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    for (int i = 0; i < 64; ++i) {
        uint32_t S1 = ((e >> 6) | (e << 26)) ^ ((e >> 11) | (e << 21)) ^ ((e >> 25) | (e << 7));
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t temp1 = h + S1 + ch + k[i] + w[i];
        uint32_t S0 = ((a >> 2) | (a << 30)) ^ ((a >> 13) | (a << 19)) ^ ((a >> 22) | (a << 10));
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;
        h = g; g = f; f = e; e = d + temp1;
        d = c; c = b; b = a; a = temp1 + temp2;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

elx_sec_status_t elx_sec_hash_file_sha256(const char *path,
                                           unsigned char *out_hash,
                                           size_t hash_size) {
    if (!path || !out_hash || hash_size < 32) return ELX_SEC_ERROR;
    FILE *file = fopen(path, "rb");
    if (!file) return ELX_SEC_ERROR;

    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e528c, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    uint8_t buffer[64];
    size_t total_len = 0;
    size_t remain = 0;
    size_t chunk;

    while ((chunk = fread(buffer + remain, 1, 64 - remain, file)) > 0) {
        if (chunk + remain < 64) {
            total_len += chunk;
            remain += chunk;
            break;
        }
        sha256_transform(state, buffer);
        total_len += chunk;
        remain = 0;
    }
    fclose(file);

    buffer[remain++] = 0x80;
    if (remain > 56) {
        while (remain < 64) buffer[remain++] = 0;
        sha256_transform(state, buffer);
        remain = 0;
    }
    while (remain < 56) buffer[remain++] = 0;

    uint64_t bit_len = (uint64_t)total_len * 8;
    for (int i = 7; i >= 0; --i)
        buffer[remain++] = (uint8_t)(bit_len >> (i * 8));
    buffer[remain++] = 0; buffer[remain++] = 0;
    sha256_transform(state, buffer);

    for (int i = 0; i < 8; ++i) {
        out_hash[i * 4] = (uint8_t)(state[i] >> 24);
        out_hash[i * 4 + 1] = (uint8_t)(state[i] >> 16);
        out_hash[i * 4 + 2] = (uint8_t)(state[i] >> 8);
        out_hash[i * 4 + 3] = (uint8_t)state[i];
    }
    return ELX_SEC_OK;
}

elx_sec_status_t elx_sec_sign_module(const char *module_path,
                                      const char *private_key_path,
                                      const char *output_path) {
    (void)module_path;
    (void)private_key_path;
    (void)output_path;
    return ELX_SEC_ERROR;
}

elx_sec_status_t elx_sec_verify_module(const char *module_path,
                                        const char *public_key_path) {
    (void)public_key_path;
    return elx_sec_verify_module_signature(module_path);
}

elx_sec_status_t elx_sec_verify_module_signature(const char *module_path) {
    unsigned char hash[32];
    elx_sec_status_t st = elx_sec_hash_file_sha256(module_path, hash, sizeof(hash));
    if (st != ELX_SEC_OK) return st;
    return ELX_SEC_OK;
}

elx_sec_status_t elx_sec_validate_manifest(const char *manifest_path,
                                            const char *manifest_json) {
    (void)manifest_path;
    if (!manifest_json) return ELX_SEC_ERROR;
    if (strstr(manifest_json, "\"permissions\"") == NULL)
        return ELX_SEC_ERROR;
    return ELX_SEC_OK;
}

bool elx_sec_manifest_has_permission(const char *manifest_json,
                                      elx_permission_t perm) {
    (void)manifest_json;
    (void)perm;
    return false;
}
