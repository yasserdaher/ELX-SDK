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

#ifndef ELX_CONFIG_H
#define ELX_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ELX_SDK_VERSION
#define ELX_SDK_VERSION "0.4.1-phase4"
#endif
#ifndef ELX_SDK_VERSION_MAJOR
#define ELX_SDK_VERSION_MAJOR 1
#endif
#ifndef ELX_SDK_VERSION_MINOR
#define ELX_SDK_VERSION_MINOR 0
#endif
#ifndef ELX_SDK_VERSION_PATCH
#define ELX_SDK_VERSION_PATCH 0
#endif

#ifndef ELX_SDK_SECURITY_VERSION
#define ELX_SDK_SECURITY_VERSION "0.1.0"
#endif

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
    #define ELX_PLATFORM_WINDOWS 1
    #define ELX_PLATFORM_NAME "windows"
#elif defined(__linux__) || defined(linux) || defined(__linux)
    #define ELX_PLATFORM_LINUX 1
    #define ELX_PLATFORM_NAME "linux"
#elif defined(__APPLE__) && defined(__MACH__)
    #define ELX_PLATFORM_MACOS 1
    #define ELX_PLATFORM_NAME "macos"
#elif defined(__sun) || defined(sun)
    #define ELX_PLATFORM_SOLARIS 1
    #define ELX_PLATFORM_NAME "solaris"
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
    #define ELX_PLATFORM_FREEBSD 1
    #define ELX_PLATFORM_NAME "freebsd"
#else
    #define ELX_PLATFORM_UNKNOWN 1
    #define ELX_PLATFORM_NAME "unknown"
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
    #define ELX_ARCH_X86_64 1
    #define ELX_ARCH_NAME "x86_64"
#elif defined(__i386__) || defined(_M_IX86) || defined(__i686__)
    #define ELX_ARCH_X86 1
    #define ELX_ARCH_NAME "x86"
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(__arm64__)
    #define ELX_ARCH_ARM64 1
    #define ELX_ARCH_NAME "arm64"
#elif defined(__arm__) || defined(_M_ARM)
    #define ELX_ARCH_ARM 1
    #define ELX_ARCH_NAME "arm"
#else
    #define ELX_ARCH_UNKNOWN 1
    #define ELX_ARCH_NAME "unknown"
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define ELX_COMPILER GCC_LIKE
#elif defined(_MSC_VER)
    #define ELX_COMPILER MSVC
#else
    #define ELX_COMPILER UNKNOWN
#endif

#if defined(__ELVI_OS__)
    #define ELX_TARGET_ELVI 1
#else
    #define ELX_TARGET_NATIVE 1
#endif

#ifndef ELX_API
    #if defined(_WIN32) && defined(__GNUC__)
        #define ELX_API __attribute__((dllimport))
    #elif defined(_WIN32) && defined(__clang__)
        #define ELX_API __attribute__((dllimport))
    #elif defined(_WIN32)
        #define ELX_API __declspec(dllimport)
    #elif defined(__GNUC__) || defined(__clang__)
        #define ELX_API __attribute__((visibility("default")))
    #else
        #define ELX_API
    #endif
#endif

#ifndef ELX_EXPORT
    #define ELX_EXPORT
#endif

#ifndef ELX_CALLCONV
    #if defined(_WIN32)
        #define ELX_CALLCONV __stdcall
    #else
        #define ELX_CALLCONV
    #endif
#endif

static inline const char *elx_platform_name(void) { return ELX_PLATFORM_NAME; }
static inline const char *elx_arch_name(void) { return ELX_ARCH_NAME; }
const char *elx_sdk_version(void);

#ifdef __cplusplus
}
#endif

#endif
