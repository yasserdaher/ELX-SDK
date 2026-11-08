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
#include "elx_std_fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#define FS_ACCESS _access
#define FS_MKDIR _mkdir
#define FS_STAT _stat
#define FS_CWD _getcwd
#define FS_CHDIR _chdir
#define FS_REMOVE _unlink
#define FS_RENAME _rename
#else
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#define FS_ACCESS access
#define FS_MKDIR mkdir
#define FS_STAT stat
#define FS_CWD getcwd
#define FS_CHDIR chdir
#define FS_REMOVE unlink
#define FS_RENAME rename
#endif
const char *elx_std_fs_version(void) { return "0.1.0"; }
bool elx_fs_exists(const char *path) { if (!path) return false; return FS_ACCESS(path, 0) == 0; }
bool elx_fs_is_file(const char *path) {
    if (!path) return false;
#if defined(_WIN32)
    struct _stat st; if (_stat(path, &st) != 0) return false; return (st.st_mode & _S_IFMT) == _S_IFREG;
#else
    struct stat st; if (stat(path, &st) != 0) return false; return S_ISREG(st.st_mode);
#endif
}
bool elx_fs_is_dir(const char *path) {
    if (!path) return false;
#if defined(_WIN32)
    struct _stat st; if (_stat(path, &st) != 0) return false; return (st.st_mode & _S_IFMT) == _S_IFDIR;
#else
    struct stat st; if (stat(path, &st) != 0) return false; return S_ISDIR(st.st_mode);
#endif
}
bool elx_fs_is_link(const char *path) { (void)path; return false; }
elx_fs_entry_type_t elx_fs_type(const char *path) { if (elx_fs_is_dir(path)) return ELX_FS_DIR; if (elx_fs_is_file(path)) return ELX_FS_FILE; return ELX_FS_UNKNOWN; }
size_t elx_fs_size(const char *path) {
    if (!path) return 0;
#if defined(_WIN32)
    struct _stat st; if (_stat(path, &st) != 0) return 0; return (size_t)st.st_size;
#else
    struct stat st; if (stat(path, &st) != 0) return 0; return (size_t)st.st_size;
#endif
}
static bool read_whole_file(const char *path, void **out_buffer, size_t *out_length) {
    FILE *file = fopen(path, "rb");
    if (!file) return false;
    fseek(file, 0, SEEK_END); long size = ftell(file); fseek(file, 0, SEEK_SET);
    if (size < 0) { fclose(file); return false; }
    char *buffer = (char *)malloc((size_t)size + 1);
    if (!buffer) { fclose(file); return false; }
    size_t read = fread(buffer, 1, (size_t)size, file);
    fclose(file); buffer[read] = '\0';
    if (out_buffer) *out_buffer = buffer;
    if (out_length) *out_length = read;
    return true;
}
bool elx_fs_read_file(const char *path, void **out_buffer, size_t *out_length) { return read_whole_file(path, out_buffer, out_length); }
bool elx_fs_write_file(const char *path, const void *buffer, size_t length) {
    if (!path || !buffer) return false;
    FILE *file = fopen(path, "wb");
    if (!file) return false;
    size_t w = fwrite(buffer, 1, length, file); fclose(file);
    return w == length;
}
bool elx_fs_append_file(const char *path, const void *buffer, size_t length) {
    if (!path || !buffer) return false;
    FILE *file = fopen(path, "ab");
    if (!file) return false;
    size_t w = fwrite(buffer, 1, length, file); fclose(file);
    return w == length;
}
char *elx_fs_read_text(const char *path, size_t *out_length) {
    void *buffer = NULL; size_t length = 0;
    if (!read_whole_file(path, &buffer, &length)) return NULL;
    if (out_length) *out_length = length;
    return (char *)buffer;
}
bool elx_fs_write_text(const char *path, const char *text, size_t length) { return elx_fs_write_file(path, text, length); }
bool elx_fs_mkdir(const char *path) { if (!path) return false;
#if defined(_WIN32)
    return FS_MKDIR(path) == 0;
#else
    return FS_MKDIR(path, 0755) == 0;
#endif
}
bool elx_fs_mkdirs(const char *path) {
    if (!path) return false;
    char *temp = (char *)malloc(strlen(path) + 1);
    if (!temp) return false;
    strcpy(temp, path); char *p = temp;
#if defined(_WIN32)
    if (p[0] && p[1] == ':') p += 2;
    if (p[0] == '\\' || p[0] == '/') p++;
#else
    if (p[0] == '/') p++;
#endif
    while (*p) {
        while (*p && *p != '\\' && *p != '/') p++;
        if (*p) { char saved = *p; *p = '\0';
#if defined(_WIN32)
            _mkdir(temp);
#else
            mkdir(temp, 0755);
#endif
            *p = saved;
        }
        if (*p) p++;
    }
    int result = mkdir(path, 0755); free(temp);
    return result == 0;
}
bool elx_fs_remove(const char *path) { if (!path) return false; return FS_REMOVE(path) == 0; }
bool elx_fs_rename(const char *old_path, const char *new_path) { if (!old_path || !new_path) return false; return FS_RENAME(old_path, new_path) == 0; }
bool elx_fs_copy(const char *source, const char *destination) {
    if (!source || !destination) return false;
    void *buffer = NULL; size_t length = 0;
    if (!elx_fs_read_file(source, &buffer, &length)) return false;
    bool result = elx_fs_write_file(destination, buffer, length);
    free(buffer); return result;
}
bool elx_fs_current_dir(char *buffer, size_t buffer_size, size_t *out_length) {
    if (!buffer || buffer_size == 0) return false;
    char *result = FS_CWD(buffer, (int)buffer_size);
    if (!result) return false;
    if (out_length) *out_length = strlen(buffer);
    return true;
}
bool elx_fs_change_dir(const char *path) { if (!path) return false; return FS_CHDIR(path) == 0; }
bool elx_fs_resolve_path(const char *relative, char *absolute, size_t absolute_size) {
    if (!relative || !absolute || absolute_size == 0) return false;
    if (!elx_fs_current_dir(absolute, absolute_size, NULL)) return false;
    size_t cwd_len = strlen(absolute);
    const char *separator = (cwd_len > 0 && absolute[cwd_len-1] != '/'
#if defined(_WIN32)
        && absolute[cwd_len-1] != '\\'
#endif
        ) ? "/" : "";
    if (strlen(relative) + strlen(absolute) + strlen(separator) + 1 > absolute_size) return false;
    strcat(absolute, separator); strcat(absolute, relative);
    return true;
}
elx_fs_dir_t *elx_fs_open_dir(const char *path) {
    if (!path) return NULL;
    elx_fs_dir_t *dir = (elx_fs_dir_t *)calloc(1, sizeof(*dir));
    if (!dir) return NULL;
#if !defined(_WIN32)
    dir->impl = opendir(path);
    if (!dir->impl) { free(dir); return NULL; }
    dir->valid = true;
#endif
    return dir;
}
bool elx_fs_dir_next(elx_fs_dir_t *dir, elx_fs_entry_t *entry) {
    if (!dir || !dir->valid || !entry) return false;
#if defined(_WIN32)
    (void)dir; (void)entry; return false;
#else
    struct dirent *e = readdir((DIR *)dir->impl);
    if (!e) return false;
    strncpy(entry->name, e->d_name, ELX_FS_MAX_PATH - 1);
    entry->name[ELX_FS_MAX_PATH - 1] = '\0';
    snprintf(entry->path, ELX_FS_MAX_PATH, "%s/%s", "?", e->d_name);
    entry->size = 0;
    entry->type = (e->d_type == DT_DIR) ? ELX_FS_DIR : ELX_FS_FILE;
    return true;
#endif
}
void elx_fs_close_dir(elx_fs_dir_t *dir) {
    if (!dir) return;
#if !defined(_WIN32)
    if (dir->impl) closedir((DIR *)dir->impl);
#endif
    free(dir);
}
void elx_fs_path_get_directory(const char *path, char *directory, size_t directory_size) {
    if (!path || !directory || directory_size == 0) return;
    const char *last_sep = strrchr(path, '/');
#if defined(_WIN32)
    char *ws = strrchr(path, '\\');
    if (ws && (!last_sep || ws > last_sep)) last_sep = ws;
#endif
    if (!last_sep) { strncpy(directory, ".", directory_size - 1); directory[directory_size-1] = '\0'; return; }
    size_t prefix = (size_t)(last_sep - path + 1);
    if (prefix > directory_size - 1) prefix = directory_size - 1;
    strncpy(directory, path, prefix); directory[prefix] = '\0';
}
void elx_fs_path_get_filename(const char *path, char *filename, size_t filename_size) {
    if (!path || !filename || filename_size == 0) return;
    const char *last_sep = strrchr(path, '/');
#if defined(_WIN32)
    char *ws = strrchr(path, '\\');
    if (ws && (!last_sep || ws > last_sep)) last_sep = ws;
#endif
    const char *start = last_sep ? last_sep + 1 : path;
    size_t len = strlen(start);
    if (len >= filename_size) len = filename_size - 1;
    strncpy(filename, start, len); filename[len] = '\0';
}
void elx_fs_path_get_extension(const char *path, char *extension, size_t extension_size) {
    if (!path || !extension || extension_size == 0) return;
    extension[0] = '\0';
    const char *dot = strrchr(path, '.');
    const char *sep = strrchr(path, '/');
#if defined(_WIN32)
    char *ws = strrchr(path, '\\');
    if (ws && (!sep || ws > sep)) sep = ws;
#endif
    if (sep && dot <= sep) return;
    if (!dot) return;
    const char *ext_start = dot + 1;
    size_t ext_len = strlen(ext_start);
    if (ext_len >= extension_size) ext_len = extension_size - 1;
    strncpy(extension, ext_start, ext_len); extension[ext_len] = '\0';
}
bool elx_fs_path_join(const char *base, const char *relative, char *result, size_t result_size) {
    if (!base || !relative || !result || result_size == 0) return false;
    size_t base_len = strlen(base);
    const char *separator = (base_len > 0 && base[base_len-1] != '/'
#if defined(_WIN32)
        && base[base_len-1] != '\\'
#endif
        ) ? "/" : "";
    if (base_len + strlen(relative) + strlen(separator) + 1 > result_size) { result[0] = '\0'; return false; }
    strcpy(result, base); strcat(result, separator); strcat(result, relative);
    return true;
}
void elx_fs_path_normalize(char *path) {
    if (!path) return;
    for (char *p = path; *p; p++) { if (*p == '\\') *p = '/'; }
    char *src = path, *dst = path; bool prev_slash = false;
    while (*src) {
        if (*src == '/') {
            if (!prev_slash || (dst > path && dst[-1] != '/')) { *dst++ = *src; prev_slash = true; }
        } else { *dst++ = *src; prev_slash = false; }
        src++;
    }
    *dst = '\0';
    if (dst > path && dst[-1] == '/') dst[-1] = '\0';
}
