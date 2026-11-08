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

#include "elx_std_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
const char *elx_std_io_version(void) { return "0.1.0"; }
bool elx_print(const char *format, ...) { va_list a; va_start(a, format); int r = vprintf(format, a); va_end(a); return r >= 0; }
bool elx_println(const char *format, ...) { va_list a; va_start(a, format); int r = vprintf(format, a); va_end(a); if (r >= 0) putchar('\n'); return r >= 0; }
bool elx_print_err(const char *format, ...) { va_list a; va_start(a, format); int r = vfprintf(stderr, format, a); va_end(a); return r >= 0; }
bool elx_println_err(const char *format, ...) { va_list a; va_start(a, format); int r = vfprintf(stderr, format, a); va_end(a); if (r >= 0) fputc('\n', stderr); return r >= 0; }
bool elx_read_line(char *buffer, size_t max_length, size_t *bytes_read) {
    if (!buffer || max_length == 0) return false;
    if (fgets(buffer, (int)max_length, stdin) == NULL) return false;
    size_t len = strlen(buffer); if (bytes_read) *bytes_read = len;
    if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = '\0';
    return true;
}
char *elx_read_stdin(size_t *out_length) {
    size_t cap = 256, len = 0; char *buffer = (char *)malloc(cap);
    if (!buffer) return NULL;
    int ch;
    while ((ch = getchar()) != EOF && ch != '\n') {
        if (len + 1 >= cap) { cap *= 2; char *n = (char *)realloc(buffer, cap); if (!n) { free(buffer); return NULL; } buffer = n; }
        buffer[len++] = (char)ch;
    }
    buffer[len] = '\0'; if (out_length) *out_length = len;
    if (len == 0 && ch == EOF && feof(stdin)) { free(buffer); return NULL; }
    return buffer;
}
bool elx_file_open(elx_file_t *file, const char *path, const char *mode) { if (!file || !path || !mode) return false; file->file = fopen(path, mode); file->owner = true; return file->file != NULL; }
bool elx_file_close(elx_file_t *file) { if (!file || !file->file) return false; int r = fclose(file->file); file->file = NULL; file->owner = false; return r == 0; }
size_t elx_file_read(void *buffer, size_t element_size, size_t count, elx_file_t *file) { if (!file || !file->file || !buffer) return 0; return fread(buffer, element_size, count, file->file); }
size_t elx_file_write(const void *buffer, size_t element_size, size_t count, elx_file_t *file) { if (!file || !file->file || !buffer) return 0; return fwrite(buffer, element_size, count, file->file); }
bool elx_file_seek(elx_file_t *file, long offset, int origin) { if (!file || !file->file) return false; return fseek(file->file, offset, origin) == 0; }
long elx_file_tell(elx_file_t *file) { if (!file || !file->file) return -1; return ftell(file->file); }
bool elx_file_flush(elx_file_t *file) { if (!file || !file->file) return false; return fflush(file->file) == 0; }
bool elx_stdout_write(const void *buffer, size_t length) { if (!buffer) return false; return fwrite(buffer, 1, length, stdout) == length; }
bool elx_stderr_write(const void *buffer, size_t length) { if (!buffer) return false; return fwrite(buffer, 1, length, stderr) == length; }
bool elx_stdin_read(void *buffer, size_t length, size_t *bytes_read) { if (!buffer || length == 0) return false; size_t r = fread(buffer, 1, length, stdin); if (bytes_read) *bytes_read = r; return r > 0 || (r == 0 && !feof(stdin)); }
int elx_get_char(void) { return fgetc(stdin); }
int elx_put_char(int character) { return fputc(character, stdout); }
bool elx_flush_stdout(void) { return fflush(stdout) == 0; }
