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

#ifndef ELX_NATIVE_INTERNAL_H
#define ELX_NATIVE_INTERNAL_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elx_config.h"

static inline int elx_native_vsnprintf(char *buffer, size_t size, const char *format, va_list args) {
    int result = vsnprintf(buffer, size, format, args);
    if (result < 0) return -1;
    if ((size_t)result >= size) return (int)size - 1;
    return result;
}

static inline void elx_native_c_escape(FILE *out, const char *s) {
    if (!s) { fputs("\\0", out); return; }
    for (; *s; s++) {
        switch (*s) {
            case '\\': fputs("\\\\", out); break;
            case '"':  fputs("\\\"", out); break;
            case '\n': fputs("\\n", out); break;
            case '\r': fputs("\\r", out); break;
            case '\t': fputs("\\t", out); break;
            case '\0': fputs("\\0", out); break;
            default:
                if ((unsigned char)*s < 0x20 || (unsigned char)*s == 0x7f)
                    fprintf(out, "\\x%02x", (unsigned char)*s);
                else fputc(*s, out);
                break;
        }
    }
}
#endif
