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

#ifndef ELX_STD_IO_H
#define ELX_STD_IO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    FILE *file;
    bool owner;
} elx_file_t;

typedef enum {
    ELX_IO_STDIN = 0,
    ELX_IO_STDOUT,
    ELX_IO_STDERR,
    ELX_IO_FILE,
} elx_io_stream_t;

const char *elx_std_io_version(void);

bool elx_print(const char *format, ...);
bool elx_println(const char *format, ...);
bool elx_print_err(const char *format, ...);
bool elx_println_err(const char *format, ...);
bool elx_read_line(char *buffer, size_t max_length, size_t *bytes_read);
char *elx_read_stdin(size_t *out_length);

bool elx_file_open(elx_file_t *file, const char *path, const char *mode);
bool elx_file_close(elx_file_t *file);
size_t elx_file_read(void *buffer, size_t element_size, size_t count, elx_file_t *file);
size_t elx_file_write(const void *buffer, size_t element_size, size_t count, elx_file_t *file);
bool elx_file_seek(elx_file_t *file, long offset, int origin);
long elx_file_tell(elx_file_t *file);
bool elx_file_flush(elx_file_t *file);

bool elx_stdout_write(const void *buffer, size_t length);
bool elx_stderr_write(const void *buffer, size_t length);
bool elx_stdin_read(void *buffer, size_t length, size_t *bytes_read);

int elx_get_char(void);
int elx_put_char(int character);
bool elx_flush_stdout(void);

#ifdef __cplusplus
}
#endif

#endif
