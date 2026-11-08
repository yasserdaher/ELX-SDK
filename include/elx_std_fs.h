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

#ifndef ELX_STD_FS_H
#define ELX_STD_FS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_FS_MAX_PATH 4096

typedef enum {
    ELX_FS_FILE = 0,
    ELX_FS_DIR,
    ELX_FS_LINK,
    ELX_FS_UNKNOWN,
} elx_fs_entry_type_t;

typedef struct {
    char path[ELX_FS_MAX_PATH];
    char name[ELX_FS_MAX_PATH];
    elx_fs_entry_type_t type;
    size_t size;
    uint64_t modified_time;
    bool exists;
    bool is_readable;
    bool is_writable;
    bool is_executable;
} elx_fs_entry_t;

typedef struct {
    void *impl;
    size_t index;
    bool valid;
} elx_fs_dir_t;

const char *elx_std_fs_version(void);

bool elx_fs_exists(const char *path);
bool elx_fs_is_file(const char *path);
bool elx_fs_is_dir(const char *path);
bool elx_fs_is_link(const char *path);
elx_fs_entry_type_t elx_fs_type(const char *path);
size_t elx_fs_size(const char *path);

bool elx_fs_read_file(const char *path, void **out_buffer, size_t *out_length);
bool elx_fs_write_file(const char *path, const void *buffer, size_t length);
bool elx_fs_append_file(const char *path, const void *buffer, size_t length);
char *elx_fs_read_text(const char *path, size_t *out_length);
bool elx_fs_write_text(const char *path, const char *text, size_t length);

bool elx_fs_mkdir(const char *path);
bool elx_fs_mkdirs(const char *path);
bool elx_fs_remove(const char *path);
bool elx_fs_rename(const char *old_path, const char *new_path);
bool elx_fs_copy(const char *source, const char *destination);

bool elx_fs_current_dir(char *buffer, size_t buffer_size, size_t *out_length);
bool elx_fs_change_dir(const char *path);
bool elx_fs_resolve_path(const char *relative, char *absolute, size_t absolute_size);

elx_fs_dir_t *elx_fs_open_dir(const char *path);
bool elx_fs_dir_next(elx_fs_dir_t *dir, elx_fs_entry_t *entry);
void elx_fs_close_dir(elx_fs_dir_t *dir);

bool elx_fs_path_join(const char *base, const char *relative, char *result, size_t result_size);
void elx_fs_path_normalize(char *path);
void elx_fs_path_get_directory(const char *path, char *directory, size_t directory_size);
void elx_fs_path_get_filename(const char *path, char *filename, size_t filename_size);
void elx_fs_path_get_extension(const char *path, char *extension, size_t extension_size);

#ifdef __cplusplus
}
#endif

#endif
