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

#define _GNU_SOURCE
#include "elx_std_str.h"
#include "elx_std_list.h"
#include "elx_std_map.h"
#include "elx_std_set.h"
#include "elx_std_math.h"
#include "elx_std_error.h"
#include "elx_std_net.h"
#include "elx_module.h"
#include "elx_package.h"
#include "elx_diagnostics.h"
#include "elx_formatter.h"
#include "elx_linter.h"
#include "elx_debugger.h"
#include "elx_spec_freeze.h"
#include "elx_compat.h"
#include "elx_elx1.h"
#include "elx_std_io.h"
#include "elx_std_fs.h"
#include "elx_std_process.h"
#include "elx_std_env.h"
#include "elx_std_time.h"
#include "elx_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static int tests_passed = 0;
static int tests_failed = 0;

static void map_foreach_print(const char *key, void *value, size_t index, void *user) {
    (void)key; (void)value; (void)index; (void)user;
}

static void check(bool cond, const char *name) {
    if (cond) { tests_passed++; }
    else { tests_failed++; fprintf(stderr, "FAIL: %s\n", name); }
}

static void test_string(void) { /* same as before */
    printf("=== String ===\n");
    elx_string_t *s = elx_str_new();
    check(s != NULL, "str_new");
    check(elx_str_is_empty(s), "str_is_empty");
    elx_str_append(s, "Hello");
    check(elx_str_equals(s, "Hello"), "str_append+equals");
    check(s->length == 5, "str_length");
    elx_str_append(s, " World");
    check(elx_str_equals(s, "Hello World"), "str_append2");
    check(s->length == 11, "str_length2");
    elx_string_t *s2 = elx_str_from("Test");
    check(s2 != NULL && elx_str_equals(s2, "Test"), "str_from");
    elx_string_t *s3 = elx_str_dup("dup");
    check(s3 != NULL && elx_str_equals(s3, "dup"), "str_dup");
    elx_str_append_char(s, '!');
    check(elx_str_equals(s, "Hello World!"), "str_append_char");
    elx_string_t *lower = elx_str_to_lower(s);
    check(lower != NULL && elx_str_equals(lower, "hello world!"), "str_to_lower");
    elx_str_free(lower);
    elx_string_t *upper = elx_str_to_upper(s);
    check(upper != NULL && elx_str_equals(upper, "HELLO WORLD!"), "str_to_upper");
    elx_str_free(upper);
    elx_string_t *trimmed = elx_str_trim(elx_str_from("  hello  "));
    check(trimmed != NULL && elx_str_equals(trimmed, "hello"), "str_trim");
    elx_str_free(trimmed);
    elx_string_t *replaced = elx_str_replace(elx_str_from("aab"), "ab", "XY");
    check(replaced != NULL && elx_str_equals(replaced, "aXY"), "str_replace");
    elx_str_free(replaced);
    elx_string_t *joined = elx_str_join(",", (const char *[]){"a", "b", "c"}, 3);
    check(joined != NULL && elx_str_equals(joined, "a,b,c"), "str_join");
    elx_str_free(joined);
    elx_string_t *fmt = elx_str_format("test %d %s", 42, "hello");
    check(fmt != NULL && elx_str_equals(fmt, "test 42 hello"), "str_format");
    elx_str_free(fmt);
    elx_string_t *sliced = elx_str_slice(s, 0, 5);
    check(sliced != NULL && elx_str_equals(sliced, "Hello"), "str_slice");
    elx_str_free(sliced);
    check(elx_str_starts_with(s, "Hello"), "str_starts_with");
    check(elx_str_ends_with(s, "!"), "str_ends_with");
    check(elx_str_find(s, "World") != NULL, "str_find");
    check(elx_str_count(s, 'l') == 3, "str_count");
    char **parts; size_t count;
    elx_str_split(elx_str_from("a,b,c"), ',', &parts, &count);
    check(count == 3 && strcmp(parts[0], "a") == 0 && strcmp(parts[1], "b") == 0 && strcmp(parts[2], "c") == 0, "str_split");
    elx_str_free_parts(parts, count);
    elx_str_free(s); elx_str_free(s2); elx_str_free(s3);
}

static void test_list(void) {
    printf("=== List ===\n");
    elx_list_t *l = elx_list_new(NULL);
    check(l != NULL && elx_list_is_empty(l), "list_new");
    check(elx_list_count(l) == 0, "list_count");
    int a = 1, b = 2, c = 3;
    check(elx_list_push_back(l, &a), "list_push_back");
    check(elx_list_push_back(l, &b), "list_push_back2");
    check(elx_list_push_front(l, &c), "list_push_front");
    check(elx_list_count(l) == 3, "list_count3");
    check(*(int *)elx_list_get(l, 0) == 3, "list_get_front");
    check(*(int *)elx_list_get(l, 1) == 1, "list_get_mid");
    check(*(int *)elx_list_get(l, 2) == 2, "list_get_back");
    check(*(int *)elx_list_pop_front(l) == 3, "list_pop_front");
    check(elx_list_count(l) == 2, "list_count2");
    check(*(int *)elx_list_pop_back(l) == 2, "list_pop_back");
    check(elx_list_count(l) == 1, "list_count1");
    int d = 4;
    elx_list_set(l, 0, &d);
    check(*(int *)elx_list_get(l, 0) == 4, "list_set");
    check(elx_list_remove(l, 0) == &d, "list_remove");
    check(elx_list_is_empty(l), "list_empty_after_remove");
    elx_list_push_back(l, &a); elx_list_push_back(l, &b); elx_list_push_back(l, &c);
    size_t idx = elx_list_index_of(l, &b, (int (*)(const void *, const void *))strcmp);
    check(idx == 1, "list_index_of");
    check(elx_list_contains(l, &a, (int (*)(const void *, const void *))strcmp), "list_contains");
    elx_list_clear(l);
    check(elx_list_is_empty(l), "list_clear");
    elx_list_free(l);
}

static void test_map(void) {
    printf("=== Map ===\n");
    elx_map_t *m = elx_map_new(NULL, NULL);
    check(m != NULL && elx_map_is_empty(m), "map_new");
    int x = 10, y = 20;
    check(elx_map_set(m, "a", &x), "map_set");
    check(elx_map_set(m, "b", &y), "map_set2");
    check(elx_map_count(m) == 2, "map_count");
    void *v;
    check(elx_map_get(m, "a", &v) && *(int *)v == 10, "map_get");
    check(elx_map_get(m, "b", &v) && *(int *)v == 20, "map_get2");
    check(!elx_map_get(m, "missing", &v), "map_get_missing");
    check(elx_map_has(m, "a"), "map_has");
    check(!elx_map_has(m, "nope"), "map_has_missing");
    check(elx_map_remove(m, "a"), "map_remove");
    check(!elx_map_has(m, "a"), "map_has_after_remove");
    check(elx_map_count(m) == 1, "map_count_after_remove");
    elx_map_foreach(m, map_foreach_print, NULL);
    elx_map_clear(m);
    check(elx_map_is_empty(m), "map_clear");
    elx_map_free(m);
}

static void test_set(void) {
    printf("=== Set ===\n");
    elx_set_t *s = elx_set_new();
    check(s != NULL && elx_set_is_empty(s), "set_new");
    check(elx_set_add(s, "a"), "set_add");
    check(elx_set_add(s, "b"), "set_add2");
    check(elx_set_add(s, "a"), "set_add_duplicate");
    check(elx_set_count(s) == 2, "set_count");
    check(elx_set_has(s, "a"), "set_has");
    check(!elx_set_has(s, "c"), "set_has_missing");
    check(elx_set_remove(s, "a"), "set_remove");
    check(!elx_set_has(s, "a"), "set_has_after_remove");
    char **vals = elx_set_values(s, NULL);
    check(vals != NULL, "set_values");
    free(vals);
    elx_set_clear(s);
    check(elx_set_is_empty(s), "set_clear");
    elx_set_free(s);
}

static void test_math(void) {
    printf("=== Math ===\n");
    check(fabs(elx_math_abs(-5.0) - 5.0) < 1e-9, "math_abs");
    check(elx_math_sign(-3.0) == -1, "math_sign_neg");
    check(elx_math_sign(0.0) == 0, "math_sign_zero");
    check(elx_math_sign(3.0) == 1, "math_sign_pos");
    check(fabs(elx_math_sqrt(16.0) - 4.0) < 1e-9, "math_sqrt");
    check(fabs(elx_math_pow(2.0, 3.0) - 8.0) < 1e-9, "math_pow");
    check(fabs(elx_math_exp(0.0) - 1.0) < 1e-9, "math_exp");
    check(fabs(elx_math_log(M_E) - 1.0) < 1e-9, "math_log");
    check(fabs(elx_math_sin(0.0)) < 1e-9, "math_sin");
    check(fabs(elx_math_cos(0.0) - 1.0) < 1e-9, "math_cos");
    check(fabs(elx_math_tan(0.0)) < 1e-9, "math_tan");
    check(elx_math_floor(3.7) == 3.0, "math_floor");
    check(elx_math_ceil(3.2) == 4.0, "math_ceil");
    check(elx_math_round(3.5) == 4.0, "math_round");
    check(elx_math_is_nan(NAN) == 1, "math_is_nan");
    check(elx_math_is_inf(INFINITY) == 1, "math_is_inf");
    check(elx_math_is_finite(1.0) == 1, "math_is_finite");
    check(fabs(elx_math_hypot(3.0, 4.0) - 5.0) < 1e-9, "math_hypot");
    check(elx_math_llround(3.6) == 4, "math_llround");
}

static void test_error(void) {
    printf("=== Error ===\n");
    elx_error_t *e = elx_error_new(ELX_ERR_INVALID_ARGUMENT, "bad arg");
    check(e != NULL, "err_new");
    check(e->code == ELX_ERR_INVALID_ARGUMENT, "err_code");
    check(strcmp(e->message, "bad arg") == 0, "err_message");
    check(!elx_error_is_success(e), "err_is_failure");
    elx_error_free(e);
    check(elx_error_is_success(NULL), "err_null_is_success");
    check(strcmp(elx_error_code_name(ELX_ERR_FILE_NOT_FOUND), "FileNotFound") == 0, "err_code_name");
    check(strcmp(elx_error_code_message(ELX_ERR_IO), "I/O error") == 0, "err_code_message");
    elx_error_list_t *l = elx_error_list_new();
    check(l != NULL && elx_error_list_count(l) == 0, "err_list_new");
    e = elx_error_new(ELX_ERR_RUNTIME, "rt");
    elx_error_list_add(l, e);
    check(elx_error_list_count(l) == 1, "err_list_add");
    check(elx_error_list_has_error(l), "err_list_has_error");
    check(elx_error_list_has_code(l, ELX_ERR_RUNTIME), "err_list_has_code");
    check(elx_error_list_get(l, 0) != NULL, "err_list_get");
    check(elx_error_list_first(l) != NULL, "err_list_first");
    elx_error_list_free(l);
}

static void test_net(void) {
    printf("=== Network ===\n");
    check(strcmp(elx_net_version(), "0.1.0") == 0, "net_version");
    elx_net_socket_t *s = elx_net_create_socket(ELX_NET_FAMILY_IPV4, ELX_NET_SOCK_STREAM);
    check(s == NULL, "net_create_returns_null");
    check(elx_net_resolve_host("localhost", ELX_NET_FAMILY_IPV4) == NULL, "net_resolve_null");
}

static void test_other_std(void) {
    printf("=== Other Std ===\n");
    check(strcmp(elx_std_io_version(), "0.1.0") == 0, "io_version");
    check(strcmp(elx_std_fs_version(), "0.1.0") == 0, "fs_version");
    check(strcmp(elx_std_process_version(), "0.1.0") == 0, "process_version");
    check(strcmp(elx_std_env_version(), "0.1.0") == 0, "env_version");
    check(strcmp(elx_std_time_version(), "0.1.0") == 0, "time_version");
}

static void test_module(void) {
    printf("=== Module ===\n");
    check(strcmp(elx_module_version(), "0.1.0") == 0, "module_version");
    check(elx_module_type_from_string("stdlib") == ELX_MODULE_TYPE_STDLIB, "module_type_stdlib");
    check(elx_module_type_from_string("local") == ELX_MODULE_TYPE_LOCAL, "module_type_local");
    check(elx_module_type_from_string("package") == ELX_MODULE_TYPE_PACKAGE, "module_type_package");
    check(elx_module_type_from_string("builtin") == ELX_MODULE_TYPE_BUILTIN, "module_type_builtin");
    check(strcmp(elx_module_type_name(ELX_MODULE_TYPE_STDLIB), "stdlib") == 0, "module_type_name_stdlib");
    check(strcmp(elx_module_type_name(ELX_MODULE_TYPE_LOCAL), "local") == 0, "module_type_name_local");
    elx_module_info_t info;
    check(elx_module_info_init(&info), "module_info_init");
    check(strcmp(info.name, "unknown") == 0, "module_info_default_name");
    check(strcmp(info.version, "0.1.0") == 0, "module_info_default_version");
    check(info.type == ELX_MODULE_TYPE_LOCAL, "module_info_default_type");
    elx_module_info_clear(&info);
    elx_module_cache_t *cache = elx_module_cache_new();
    check(cache != NULL, "module_cache_new");
    check(elx_module_cache_is_empty(cache), "module_cache_empty");
    elx_module_info_init(&info);
    strncpy(info.name, "testmod", ELX_MODULE_MAX_NAME - 1);
    strncpy(info.version, "1.0.0", ELX_MODULE_MAX_VERSION - 1);
    strncpy(info.path, "/test/path", ELX_MODULE_MAX_PATH - 1);
    info.type = ELX_MODULE_TYPE_PACKAGE;
    check(elx_module_cache_add(cache, &info), "module_cache_add");
    check(!elx_module_cache_is_empty(cache), "module_cache_not_empty");
    check(elx_module_cache_count(cache) == 1, "module_cache_count");
    elx_module_entry_t *found = elx_module_cache_find(cache, "testmod");
    check(found != NULL, "module_cache_find");
    check(strcmp(found->info.name, "testmod") == 0, "module_cache_find_name");
    elx_module_entry_t *get = elx_module_cache_get(cache, 0);
    check(get != NULL, "module_cache_get");
    check(strcmp(get->info.name, "testmod") == 0, "module_cache_get_name");
    elx_module_info_t resolved;
    elx_module_info_init(&resolved);
    check(!elx_module_resolve(NULL, "testmod", &resolved), "module_resolve_null_cache");
    check(!elx_module_resolve(cache, NULL, &resolved), "module_resolve_null_name");
    elx_module_cache_clear(cache);
    check(elx_module_cache_is_empty(cache), "module_cache_cleared");
    elx_module_cache_free(cache);
}

static void test_package(void) {
    printf("=== Package ===\n");
    check(strcmp(elx_package_version(), "0.1.0") == 0, "package_version");
    check(elx_package_license_from_string("MIT") == ELX_PACKAGE_LICENSE_MIT, "license_mit");
    check(elx_package_license_from_string("Apache-2.0") == ELX_PACKAGE_LICENSE_APACHE2, "license_apache");
    check(elx_package_license_from_string("unknown") == ELX_PACKAGE_LICENSE_CUSTOM, "license_unknown");
    check(strcmp(elx_package_license_name(ELX_PACKAGE_LICENSE_MIT), "MIT") == 0, "license_name_mit");
    check(strcmp(elx_package_license_name(ELX_PACKAGE_LICENSE_UNKNOWN), "Unknown") == 0, "license_name_unknown");
    check(elx_package_status_from_string("public") == ELX_PACKAGE_STATUS_PUBLIC, "status_public");
    check(elx_package_status_from_string("verified") == ELX_PACKAGE_STATUS_VERIFIED, "status_verified");
    check(elx_package_status_from_string("unknown") == ELX_PACKAGE_STATUS_PRIVATE, "status_private");
    check(strcmp(elx_package_status_name(ELX_PACKAGE_STATUS_PUBLIC), "public") == 0, "status_name_public");
    elx_package_info_t pkg;
    check(elx_package_info_init(&pkg), "package_info_init");
    check(strcmp(pkg.version, "0.1.0") == 0, "package_info_default_version");
    check(pkg.sandbox == true, "package_info_default_sandbox");
    elx_package_info_clear(&pkg);
    elx_package_registry_t *reg = elx_package_registry_new();
    check(reg != NULL, "package_registry_new");
    check(elx_package_registry_is_empty(reg), "package_registry_empty");
    check(strcmp(elx_package_registry_url(reg), "https://elx.packages") == 0, "package_registry_url");
    elx_package_info_init(&pkg);
    strncpy(pkg.name, "testpkg", ELX_PACKAGE_MAX_NAME - 1);
    strncpy(pkg.version, "1.0.0", ELX_PACKAGE_MAX_VERSION - 1);
    check(elx_package_registry_add(reg, &pkg), "package_registry_add");
    check(!elx_package_registry_is_empty(reg), "package_registry_not_empty");
    check(elx_package_registry_count(reg) == 1, "package_registry_count");
    elx_package_entry_t *found = elx_package_registry_find(reg, "testpkg");
    check(found != NULL, "package_registry_find");
    check(strcmp(found->info.name, "testpkg") == 0, "package_registry_find_name");
    elx_package_entry_t *get = elx_package_registry_get(reg, 0);
    check(get != NULL, "package_registry_get");
    char error[256];
    check(elx_package_validate(&pkg, error, sizeof(error)), "package_validate_ok");
    elx_package_info_t badpkg;
    elx_package_info_init(&badpkg);
    check(!elx_package_validate(&badpkg, error, sizeof(error)), "package_validate_empty");
    check(elx_package_install(reg, "sample", NULL, NULL, NULL), "package_install");
    check(elx_package_registry_count(reg) == 2, "package_count_after_install");
    check(elx_package_update(reg, "testpkg", NULL, NULL), "package_update");
    check(elx_package_remove(reg, "sample"), "package_remove");
    check(elx_package_registry_count(reg) == 1, "package_count_after_remove");
    elx_package_registry_free(reg);
}

static void test_diagnostics(void) {
    printf("=== Diagnostics ===\n");
    check(strcmp(elx_diagnostics_version(), "0.1.0") == 0, "diag_version");
    check(elx_diag_code_from_string("parse") == ELX_DIAG_CODE_PARSE, "diag_code_parse");
    check(elx_diag_code_from_string("unknown") == ELX_DIAG_CODE_UNKNOWN, "diag_code_unknown");
    check(strcmp(elx_diag_code_name(ELX_DIAG_CODE_PARSE), "parse") == 0, "diag_code_name");
    check(strcmp(elx_diag_code_description(ELX_DIAG_CODE_TYPE), "Type mismatch or error") == 0, "diag_code_desc");
    check(strlen(elx_diag_code_suggestion(ELX_DIAG_CODE_PARSE)) > 0, "diag_code_suggestion");
    check(elx_diag_severity_from_string("warning") == ELX_DIAG_WARNING, "diag_severity_warning");
    check(elx_diag_severity_from_string("info") == ELX_DIAG_INFO, "diag_severity_info");
    check(strcmp(elx_diag_severity_name(ELX_DIAG_WARNING), "warning") == 0, "diag_severity_name");
    elx_diagnostics_t *diag = elx_diagnostics_new();
    check(diag != NULL && elx_diagnostics_is_clean(diag), "diag_new_clean");
    elx_diagnostic_t *d = elx_diag_create("test.c", 10, 5, ELX_DIAG_CODE_PARSE,
                                                ELX_DIAG_ERROR, "test error");
    check(d != NULL, "diag_create");
    check(strcmp(d->message, "test error") == 0, "diag_message");
    check(d->location.line == 10, "diag_line");
    check(d->location.column == 5, "diag_column");
    elx_diag_add_label(d, "main");
    check(d->label_count == 1, "diag_label");
    elx_diag_add_note(d, "note1", "detail", "more info");
    check(d->note_count == 1, "diag_note");
    elx_diag_add_suggestion(d, "fix", "do this");
    check(d->suggestion_count == 1, "diag_suggestion");
    elx_diag_mark_fatal(d);
    check(d->is_fatal == true, "diag_fatal");
    elx_diagnostics_add(diag, d);
    free(d);
    check(elx_diagnostics_count(diag) == 1, "diag_count");
    check(elx_diagnostics_has_errors(diag), "diag_has_errors");
    check(elx_diagnostics_error_count(diag) == 1, "diag_error_count");
    elx_diagnostic_t *get = elx_diagnostics_get(diag, 0);
    check(get != NULL, "diag_get");
    check(strcmp(get->message, "test error") == 0, "diag_get_message");
    elx_diag_set_location(get, "other.c", 20, 10, 20, 15);
    check(get->location.line == 20, "diag_set_location");
    check(elx_diagnostics_is_clean(diag) == false, "diag_not_clean");
    elx_diagnostics_clear(diag);
    check(elx_diagnostics_is_clean(diag), "diag_cleared");
    elx_diagnostics_free(diag);
}

static void test_formatter(void) {
    printf("=== Formatter ===\n");
    check(strcmp(elx_formatter_version(), "0.1.0") == 0, "fmt_version");
    elx_fmt_config_t *config = elx_fmt_config_new();
    check(config != NULL, "fmt_config_new");
    check(elx_fmt_config_set_indent(config, 2), "fmt_config_indent");
    check(elx_fmt_config_set_use_tabs(config, true), "fmt_config_tabs");
    check(elx_fmt_config_set_max_line_length(config, 120), "fmt_config_max");
    check(elx_fmt_config_set_style(config, ELX_FMT_STYLE_COMPACT), "fmt_config_style");
    const char *source = "fn main() { print(\"hello\"); }\n";
    elx_fmt_result_t *result = elx_fmt_format(source, strlen(source), config);
    check(result != NULL, "fmt_format");
    check(result->success, "fmt_format_success");
    check(result->length > 0, "fmt_format_length");
    free(result->formatted);
    free(result);
    check(elx_fmt_can_format(source, strlen(source)), "fmt_can_format");
    check(elx_fmt_line_count(source, strlen(source)) > 0, "fmt_line_count");
    char *line = elx_fmt_extract_line(source, strlen(source), 1);
    check(line != NULL, "fmt_extract_line");
    free(line);
    elx_fmt_config_free(config);
}

static void test_linter(void) {
    printf("=== Linter ===\n");
    check(strcmp(elx_linter_version(), "0.1.0") == 0, "lint_version");
    elx_lint_config_t *config = elx_lint_config_new();
    check(config != NULL, "lint_config_new");
    check(elx_lint_config_set_global_level(config, ELX_LINT_WARN), "lint_config_level");
    check(elx_lint_config_enable_rule(config, ELX_LINT_TYPE_CHECK, ELX_LINT_ERROR), "lint_config_enable");
    check(elx_lint_config_disable_rule(config, ELX_LINT_NAMING), "lint_config_disable");
    char name[ELX_LINT_MAX_RULE_NAME];
    check(elx_lint_rule_name(ELX_LINT_PARSE, name, sizeof(name)), "lint_rule_name");
    const elx_lint_rule_desc_t *rule = elx_lint_config_get_rule(config, ELX_LINT_PARSE);
    check(rule != NULL, "lint_config_get_rule");
    check(rule->enabled == true, "lint_rule_enabled");
    const char *desc = elx_lint_rule_description(ELX_LINT_TYPE_CHECK);
    check(strlen(desc) > 0, "lint_rule_desc");
    const char *source = "fn Main() { x }";
    elx_lint_result_t *result = elx_lint_source(source, strlen(source), config);
    check(result != NULL, "lint_source");
    check(result->linted_source != NULL, "lint_source_buf");
    elx_diagnostics_free(result->diagnostics);
    free(result->linted_source);
    free(result);
    elx_lint_config_free(config);
}

static void test_debugger(void) {
    printf("=== Debugger ===\n");
    check(strcmp(elx_debugger_version(), "0.1.0") == 0, "dbg_version");
    elx_debugger_t *dbg = elx_debugger_new();
    check(dbg != NULL, "dbg_new");
    check(elx_debugger_state(dbg) == ELX_DEBUG_STATE_STOPPED, "dbg_stopped");
    check(elx_debugger_breakpoint_count(dbg) == 0, "dbg_no_bps");
    check(elx_debugger_set_breakpoint(dbg, "test.c", 10), "dbg_set_bp");
    check(elx_debugger_breakpoint_count(dbg) == 1, "dbg_bp_count");
    check(elx_debugger_breakpoint_at(dbg, "test.c", 10), "dbg_bp_at");
    check(!elx_debugger_breakpoint_at(dbg, "test.c", 99), "dbg_bp_not_at");
    check(elx_debugger_get_breakpoint(dbg, 0) != NULL, "dbg_get_bp");
    elx_debugger_set_variable(dbg, "x", "42");
    check(elx_debugger_get_variable(dbg, "x") != NULL, "dbg_get_var");
    check(elx_debugger_variable_count(dbg) == 1, "dbg_var_count");
    check(elx_debugger_step(dbg, ELX_DEBUG_STEP_OVER), "dbg_step");
    check(elx_debugger_state(dbg) == ELX_DEBUG_STATE_PAUSED, "dbg_paused_after_step");
    check(elx_debugger_run(dbg), "dbg_run");
    check(elx_debugger_pause(dbg), "dbg_pause");
    check(elx_debugger_continue(dbg), "dbg_continue");
    check(elx_debugger_stop(dbg), "dbg_stop");
    check(elx_debugger_reset(dbg), "dbg_reset");
    check(elx_debugger_breakpoint_count(dbg) == 0, "dbg_reset_bps");
    elx_debugger_free(dbg);
}

static void test_spec_freeze(void) {
    printf("=== Spec Freeze ===\n");
    check(strcmp(elx_spec_freeze_version(), "0.1.0") == 0, "freeze_version");
    elx_freeze_registry_t *reg = elx_freeze_registry_new();
    check(reg != NULL, "freeze_new");
    check(!elx_freeze_registry_is_locked(reg), "freeze_not_locked");
    check(elx_freeze_registry_state(reg) == ELX_FREEZE_STATE_DRAFT, "freeze_draft");
    elx_freeze_registry_add_rule(reg, "import", "import X from Y", "resolve module", true, true);
    check(reg->rule_count == 1, "freeze_rule_added");
    elx_freeze_registry_lock(reg, "0.4.0-phase4");
    check(elx_freeze_registry_is_locked(reg), "freeze_locked");
    check(elx_freeze_registry_state(reg) == ELX_FREEZE_STATE_FROZEN, "freeze_frozen");
    check(strcmp(elx_freeze_current_version(reg), "0.4.0-phase4") == 0, "freeze_version_str");
    char error[256];
    check(elx_freeze_validate_version("0.4.0-phase4", error, sizeof(error)), "freeze_version_valid");
    check(!elx_freeze_validate_version("", error, sizeof(error)), "freeze_version_invalid");
    char changelog[ELX_FREEZE_MAX_CHANGELOG];
    check(elx_freeze_get_changelog(reg, changelog, sizeof(changelog)), "freeze_changelog");
    elx_freeze_registry_unlock(reg);
    check(!elx_freeze_registry_is_locked(reg), "freeze_unlocked");
    elx_freeze_registry_free(reg);
}

static void test_compat(void) {
    printf("=== Compatibility ===\n");
    check(strcmp(elx_compat_version(), "0.1.0") == 0, "compat_version");
    elx_compat_suite_t *suite = elx_compat_suite_new();
    check(suite != NULL, "compat_new");
    elx_compat_suite_add_module(suite, "moduleA", "1.0.0");
    elx_compat_suite_add_module(suite, "moduleB", "2.0.0");
    check(elx_compat_suite_graph(suite) != NULL, "compat_graph");
    elx_compat_suite_add_dependency(suite, "moduleA", "moduleB", ">=1.0");
    char cyclic[ELX_COMPAT_MAX_GRAPH][ELX_MODULE_MAX_NAME];
    size_t cycle_count = 0;
    elx_compat_detect_cycles(suite, cyclic, &cycle_count);
    check(cycle_count == 0, "compat_no_cycles");
    char missing[ELX_COMPAT_MAX_GRAPH][ELX_MODULE_MAX_NAME];
    size_t missing_count = 0;
    elx_compat_check_dependencies(suite, missing, &missing_count);
    check(missing_count == 0 || missing_count > 0, "compat_deps_check");
    elx_package_info_t pkg;
    elx_package_info_init(&pkg);
    strncpy(pkg.name, "test", ELX_PACKAGE_MAX_NAME - 1);
    strncpy(pkg.version, "1.0.0", ELX_PACKAGE_MAX_VERSION - 1);
    char perr[256];
    check(elx_compat_validate_manifest(&pkg, perr, sizeof(perr)), "compat_manifest_ok");
    elx_compat_suite_free(suite);
}

static void test_elx1(void) {
    printf("=== ELX 1.0 ===\n");
    check(strcmp(elx_elx1_version(), "1.0.0") == 0, "elx1_version");
    check(strcmp(elx_elx1_rc_version(), "1.0.0-rc.1") == 0, "elx1_rc_version");
    check(strcmp(elx_elx1_stage_name(ELX_STAGE_DEV), "dev") == 0, "elx1_stage_dev");
    check(strcmp(elx_elx1_stage_name(ELX_STAGE_RC), "rc") == 0, "elx1_stage_rc");
    check(strcmp(elx_elx1_stage_name(ELX_STAGE_RELEASE), "release") == 0, "elx1_stage_release");
    check(strcmp(elx_elx1_stage_name(ELX_STAGE_STABLE), "stable") == 0, "elx1_stage_stable");
    elx_elx1_status_t *status = elx_elx1_status_new();
    check(status != NULL, "elx1_status_new");
    check(elx_elx1_stage(status) == ELX_STAGE_DEV, "elx1_initial_stage");
    bool passed = false;
    elx_elx1_check_diagnostics(status, NULL, &passed);
    check(!passed, "elx1_diag_no_data");
    elx_elx1_check_tooling(status, &passed);
    check(passed, "elx1_tooling");
    elx_elx1_status_free(status);
}

int main(void) {
    printf("=== ELX Standard Library Tests ===\n");
    test_string();
    test_list();
    test_map();
    test_set();
    test_math();
    test_error();
    test_net();
    test_other_std();
    test_module();
    test_package();
    test_diagnostics();
    test_formatter();
    test_linter();
    test_debugger();
    test_spec_freeze();
    test_compat();
    test_elx1();
    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}