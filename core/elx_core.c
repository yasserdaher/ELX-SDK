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

const char *elx_core_version(void) { return ELX_CORE_VERSION; }

elx_target_t elx_target_from_name(const char *name) {
    if (!name) return ELX_TARGET_ELVI;
    if (strcmp(name, "windows") == 0) return ELX_TARGET_WINDOWS;
    if (strcmp(name, "linux") == 0) return ELX_TARGET_LINUX;
    if (strcmp(name, "macos") == 0) return ELX_TARGET_MACOS;
    if (strcmp(name, "arm64") == 0) return ELX_TARGET_ARM64;
    if (strcmp(name, "elvi") == 0) return ELX_TARGET_ELVI;
    return ELX_TARGET_ELVI;
}

const char *elx_target_name(elx_target_t target) {
    switch (target) {
        case ELX_TARGET_ELVI: return "elvi";
        case ELX_TARGET_WINDOWS: return "windows";
        case ELX_TARGET_LINUX: return "linux";
        case ELX_TARGET_MACOS: return "macos";
        case ELX_TARGET_ARM64: return "arm64";
        default: return "unknown";
    }
}

elx_backend_t elx_backend_from_name(const char *name) {
    if (!name) return ELX_BACKEND_ELBC;
    if (strcmp(name, "elbc") == 0) return ELX_BACKEND_ELBC;
    if (strcmp(name, "native-c") == 0 || strcmp(name, "c") == 0) return ELX_BACKEND_NATIVE_C;
    if (strcmp(name, "native-asm") == 0) return ELX_BACKEND_NATIVE_ASM;
    return ELX_BACKEND_ELBC;
}

const char *elx_backend_name(elx_backend_t backend) {
    switch (backend) {
        case ELX_BACKEND_ELBC: return "elbc";
        case ELX_BACKEND_NATIVE_C: return "native-c";
        case ELX_BACKEND_NATIVE_ASM: return "native-asm";
        default: return "unknown";
    }
}
