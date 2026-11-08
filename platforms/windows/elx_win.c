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

#include "elx_win.h"
#include "elx_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const char *elx_win_version(void) { return "0.1.0"; }
bool elx_win_init(void) { return true; }
void elx_win_shutdown(void) { }
bool elx_win_console_init(elx_win_console_t *c) {
    if (!c) return false;
    c->console_in = GetStdHandle(STD_INPUT_HANDLE);
    c->console_out = GetStdHandle(STD_OUTPUT_HANDLE);
    c->initialized = true;
    return true;
}
void elx_win_console_destroy(elx_win_console_t *c) { (void)c; }
bool elx_win_console_write(elx_win_console_t *c, const void *buffer, size_t length) {
    (void)c; DWORD w;
    return WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buffer, (DWORD)length, &w, NULL) != 0;
}
bool elx_win_console_read(elx_win_console_t *c, void *buffer, size_t length, size_t *bytes_read) {
    (void)c; DWORD r;
    bool res = ReadConsoleA(GetStdHandle(STD_INPUT_HANDLE), buffer, (DWORD)length, &r, NULL) != 0;
    if (bytes_read) *bytes_read = (size_t)r;
    return res;
}
bool elx_win_console_set_size(int w, int h) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(h, &csbi)) return false;
    COORD size = {(short)w, (short)h};
    return SetConsoleScreenBufferSize(h, size) != 0;
}
void elx_win_console_get_size(int *width, int *height) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(h, &csbi)) {
        *width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
}
bool elx_win_console_set_text_color(uint16_t color) {
    return SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color) != 0;
}
bool elx_win_console_set_background(uint16_t color) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(h, &csbi)) return false;
    return SetConsoleTextAttribute(h, (csbi.wAttributes & 0xF0) | (color & 0x0F)) != 0;
}
bool elx_win_console_clear(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(h, &csbi)) return false;
    DWORD w; FillConsoleOutputCharacterA(h, ' ', csbi.dwSize.X * csbi.dwSize.Y, (COORD){0,0}, &w);
    SetConsoleCursorPosition(h, (COORD){0,0});
    return true;
}
bool elx_win_console_cursor_visible(bool visible) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); CONSOLE_CURSOR_INFO cci;
    if (!GetConsoleCursorInfo(h, &cci)) return false;
    cci.bVisible = visible ? TRUE : FALSE;
    return SetConsoleCursorInfo(h, &cci) != 0;
}
bool elx_win_console_cursor_position(int x, int y) {
    return SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){(short)x,(short)y}) != 0;
}
bool elx_win_file_exists(const char *path) { if (!path) return false; return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES; }
bool elx_win_is_file(const char *path) { if (!path) return false; DWORD a = GetFileAttributesA(path); return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY); }
bool elx_win_is_dir(const char *path) { if (!path) return false; DWORD a = GetFileAttributesA(path); return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY); }
uint64_t elx_win_file_size(const char *path) {
    if (!path) return 0; WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &fad)) return 0;
    return ((uint64_t)fad.nFileSizeHigh << 32) | fad.nFileSizeLow;
}
uint64_t elx_win_file_modified(const char *path) {
    if (!path) return 0; WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &fad)) return 0;
    FILETIME ft = fad.ftLastWriteTime;
    ULONGLONG ull = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    return (ull / 100) * 1000000 / 1000;
}
bool elx_win_read_file(const char *path, void **out_buffer, size_t *out_length) {
    if (!path || !out_buffer) return false;
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) return false;
    DWORD size = GetFileSize(h, NULL);
    if (size == INVALID_FILE_SIZE) { CloseHandle(h); return false; }
    char *buffer = (char *)malloc(size + 1);
    if (!buffer) { CloseHandle(h); return false; }
    DWORD br; bool r = ReadFile(h, buffer, size, &br, NULL) && br == size;
    buffer[size] = '\0'; CloseHandle(h);
    if (!r) { free(buffer); return false; }
    if (out_length) *out_length = size;
    *out_buffer = buffer;
    return true;
}
bool elx_win_write_file(const char *path, const void *buffer, size_t length) {
    if (!path || !buffer) return false;
    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return false;
    DWORD bw; bool r = WriteFile(h, buffer, (DWORD)length, &bw, NULL) && bw == (DWORD)length;
    CloseHandle(h); return r;
}
bool elx_win_append_file(const char *path, const void *buffer, size_t length) {
    if (!path || !buffer) return false;
    HANDLE h = CreateFileA(path, FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return false;
    DWORD bw; bool r = WriteFile(h, buffer, (DWORD)length, &bw, NULL) && bw == (DWORD)length;
    CloseHandle(h); return r;
}
bool elx_win_mkdir(const char *path) { if (!path) return false; return _mkdir(path) == 0; }
bool elx_win_mkdirs(const char *path) {
    if (!path) return false;
    char *temp = (char *)malloc(strlen(path) + 1);
    if (!temp) return false;
    strcpy(temp, path); char *p = temp;
    if (p[0] && p[1] == ':') p += 2;
    if (p[0] == '\\' || p[0] == '/') p++;
    while (*p) {
        while (*p && *p != '\\' && *p != '/') p++;
        if (*p) { char saved = *p; *p = '\0'; _mkdir(temp); *p = saved; }
        if (*p) p++;
    }
    int result = _mkdir(path); free(temp);
    return result == 0;
}
bool elx_win_remove(const char *path) { if (!path) return false; return RemoveFileA(path) != 0 || RemoveDirectoryA(path) != 0; }
bool elx_win_rename(const char *old_path, const char *new_path) { if (!old_path || !new_path) return false; return MoveFileExA(old_path, new_path, MOVEFILE_REPLACE_EXISTING) != 0; }
bool elx_win_copy(const char *source, const char *destination) { if (!source || !destination) return false; return CopyFileA(source, destination, FALSE) != 0; }
elx_win_dir_t *elx_win_dir_open(const char *path) {
    if (!path) return NULL;
    elx_win_dir_t *dir = (elx_win_dir_t *)calloc(1, sizeof(*dir));
    if (!dir) return NULL;
    char sp[ELX_FS_MAX_PATH]; snprintf(sp, sizeof(sp), "%s\\*", path);
    dir->find_handle = FindFirstFileA(sp, &dir->data);
    if (dir->find_handle == INVALID_HANDLE_VALUE) { free(dir); return NULL; }
    dir->valid = true;
    return dir;
}
bool elx_win_dir_next(elx_win_dir_t *dir, char *name_buffer, size_t name_buffer_size) {
    if (!dir || !dir->valid || !dir->find_handle) return false;
    if (!FindNextFileA(dir->find_handle, &dir->data)) return false;
    strncpy(name_buffer, dir->data.cFileName, name_buffer_size - 1);
    name_buffer[name_buffer_size - 1] = '\0';
    return true;
}
void elx_win_dir_close(elx_win_dir_t *dir) {
    if (!dir) return;
    if (dir->find_handle && dir->valid) FindClose(dir->find_handle);
    free(dir);
}
bool elx_win_get_current_dir(char *buffer, size_t buffer_size, size_t *out_length) {
    if (!buffer || buffer_size == 0) return false;
    DWORD len = GetCurrentDirectoryA((DWORD)buffer_size, buffer);
    if (len == 0 || len >= buffer_size) return false;
    if (out_length) *out_length = len;
    return true;
}
bool elx_win_change_dir(const char *path) { if (!path) return false; return SetCurrentDirectoryA(path) != 0; }
bool elx_win_resolve_path(const char *relative, char *absolute, size_t absolute_size) {
    if (!relative || !absolute || absolute_size == 0) return false;
    return GetFullPathNameA(relative, (DWORD)absolute_size, absolute, NULL) != 0;
}
void elx_win_path_join(const char *base, const char *relative, char *result, size_t result_size) {
    if (!base || !relative || !result || result_size == 0) return;
    size_t bl = strlen(base); size_t rl = strlen(relative);
    const char *sep = (bl > 0 && base[bl-1] != '\\' && base[bl-1] != '/') ? "\\" : "";
    if (bl + rl + strlen(sep) + 1 > result_size) { result[0] = '\0'; return; }
    strcpy(result, base); strcat(result, sep); strcat(result, relative);
}
void elx_win_path_normalize(char *path) { if (!path) return; for (char *p = path; *p; p++) { if (*p == '/') *p = '\\'; } }
void elx_win_path_get_directory(const char *path, char *dir, size_t dir_size) {
    if (!path || !dir || dir_size == 0) return;
    const char *ls = strrchr(path, '\\'); if (!ls) ls = strrchr(path, '/');
    if (!ls) { strncpy(dir, ".", dir_size - 1); dir[dir_size-1] = '\0'; return; }
    size_t prefix = (size_t)(ls - path + 1);
    if (prefix > dir_size - 1) prefix = dir_size - 1;
    strncpy(dir, path, prefix); dir[prefix] = '\0';
}
void elx_win_path_get_filename(const char *path, char *name, size_t name_size) {
    if (!path || !name || name_size == 0) return;
    const char *ls = strrchr(path, '\\'); if (!ls) ls = strrchr(path, '/');
    const char *start = ls ? ls + 1 : path;
    size_t len = strlen(start); if (len >= name_size) len = name_size - 1;
    strncpy(name, start, len); name[len] = '\0';
}
void elx_win_path_get_extension(const char *path, char *ext, size_t ext_size) {
    if (!path || !ext || ext_size == 0) return; ext[0] = '\0';
    const char *dot = strrchr(path, '.'); const char *sep = strrchr(path, '\\');
    if (sep && dot <= sep) return; if (!dot) return;
    const char *es = dot + 1; size_t elen = strlen(es);
    if (elen >= ext_size) elen = ext_size - 1;
    strncpy(ext, es, elen); ext[elen] = '\0';
}
bool elx_win_env_get(const char *key, char *buffer, size_t buffer_size, size_t *out_length) {
    if (!key || !buffer || buffer_size == 0) return false;
    DWORD len = GetEnvironmentVariableA(key, buffer, (DWORD)buffer_size);
    if (len == 0 || len >= buffer_size) return false;
    if (out_length) *out_length = len;
    return true;
}
bool elx_win_env_set(const char *key, const char *value) { if (!key) return false; return SetEnvironmentVariableA(key, value) != 0; }
bool elx_win_env_unset(const char *key) { if (!key) return false; return SetEnvironmentVariableA(key, NULL) != 0; }
size_t elx_win_env_count(void) { size_t c = 0; char **env = _environ; while (env && *env) { c++; env++; } return c; }
bool elx_win_env_get_at(size_t index, char *kb, size_t ks, char *vb, size_t vs) {
    char **env = _environ;
    for (size_t i = 0; i < index && env && *env; i++) env++;
    if (!env || !*env) return false;
    char *eq = strchr(*env, '=');
    if (!eq) return false;
    size_t kl = (size_t)(eq - *env); if (kl >= ks) kl = ks - 1;
    memcpy(kb, *env, kl); kb[kl] = '\0';
    const char *value = eq + 1; size_t vl = strlen(value); if (vl >= vs) vl = vs - 1;
    memcpy(vb, value, vl); vb[vl] = '\0';
    return true;
}
bool elx_win_env_get_home(char *buffer, size_t buffer_size, size_t *out_length) {
    const char *home = getenv("USERPROFILE"); if (!home) home = getenv("HOME");
    if (!home || !buffer || buffer_size == 0) return false;
    size_t len = strlen(home); if (len >= buffer_size) len = buffer_size - 1;
    memcpy(buffer, home, len); buffer[len] = '\0';
    if (out_length) *out_length = len;
    return true;
}
bool elx_win_env_get_temp(char *buffer, size_t buffer_size, size_t *out_length) {
    DWORD len = GetTempPathA((DWORD)buffer_size, buffer);
    if (len == 0 || len >= buffer_size) return false;
    if (out_length) *out_length = len;
    return true;
}
bool elx_win_env_get_path(char *buffer, size_t buffer_size, size_t *out_length) {
    const char *path = getenv("PATH"); if (!path || !buffer || buffer_size == 0) return false;
    size_t len = strlen(path); if (len >= buffer_size) len = buffer_size - 1;
    memcpy(buffer, path, len); buffer[len] = '\0';
    if (out_length) *out_length = len;
    return true;
}
bool elx_win_env_get_username(char *buffer, size_t buffer_size, size_t *out_length) {
    const char *user = getenv("USERNAME"); if (!user || !buffer || buffer_size == 0) return false;
    size_t len = strlen(user); if (len >= buffer_size) len = buffer_size - 1;
    memcpy(buffer, user, len); buffer[len] = '\0';
    if (out_length) *out_length = len;
    return true;
}
bool elx_win_env_get_computername(char *buffer, size_t buffer_size, size_t *out_length) {
    DWORD len = GetComputerNameA(buffer, (DWORD *)buffer_size);
    if (!len) return false;
    if (out_length) *out_length = len;
    return true;
}
bool elx_win_process_spawn(const char *command_line, const char *working_dir, bool hide_window, elx_win_process_t *process) {
    if (!command_line || !process) return false;
    ZeroMemory(&process->startup_info, sizeof(process->startup_info));
    process->startup_info.cb = sizeof(process->startup_info);
    if (hide_window) { process->startup_info.dwFlags = STARTF_USESHOWWINDOW; process->startup_info.wShowWindow = SW_HIDE; }
    char cmd_copy[4096]; strncpy(cmd_copy, command_line, sizeof(cmd_copy) - 1); cmd_copy[sizeof(cmd_copy) - 1] = '\0';
    BOOL result = CreateProcessA(NULL, cmd_copy, NULL, NULL, FALSE, hide_window ? CREATE_NO_WINDOW : 0, NULL, working_dir, &process->startup_info, &process->process_info);
    process->spawned = result != 0;
    return result != 0;
}
bool elx_win_process_wait(elx_win_process_t *process, DWORD *out_exit_code) {
    if (!process || !process->spawned) return false;
    WaitForSingleObject(process->process_info.hProcess, INFINITE);
    if (out_exit_code) GetExitCodeProcess(process->process_info.hProcess, out_exit_code);
    return true;
}
bool elx_win_process_terminate(elx_win_process_t *process, UINT exit_code) {
    if (!process || !process->spawned) return false;
    return TerminateProcess(process->process_info.hProcess, exit_code) != 0;
}
bool elx_win_process_close(elx_win_process_t *process) {
    if (!process) return false;
    if (process->spawned) { CloseHandle(process->process_info.hProcess); CloseHandle(process->process_info.hThread); }
    process->spawned = false;
    return true;
}
bool elx_win_process_get_id(elx_win_process_t *process) { (void)process; return false; }
elx_win_lib_t *elx_win_lib_load(const char *path) {
    if (!path) return NULL;
    elx_win_lib_t *lib = (elx_win_lib_t *)calloc(1, sizeof(*lib));
    if (!lib) return NULL;
    lib->library = LoadLibraryA(path);
    if (!lib->library) { free(lib); return NULL; }
    return lib;
}
void *elx_win_lib_symbol(elx_win_lib_t *library, const char *symbol_name) {
    if (!library || !library->library || !symbol_name) return NULL;
    return (void *)GetProcAddress(library->library, symbol_name);
}
void elx_win_lib_close(elx_win_lib_t *library) {
    if (!library) return;
    if (library->library) FreeLibrary(library->library);
    free(library);
}
elx_win_dialog_fn elx_win_get_dialog_box(void) { return NULL; }
