#define _GNU_SOURCE
/* ELX Standard Library - Native implementations
 *
 * These functions are registered as native functions in the ELX runtime
 * and can be called from ELX bytecode via the OP_CALL opcode fallback.
 *
 * Functions use the elx_value_t calling convention:
 *   elx_value_t fn(elx_value_t *args, uint32_t count)
 */
#include "elx_runtime.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

/* ===========================
 * Math module
 * =========================== */

static elx_value_t std_math_abs(elx_value_t *args, uint32_t count) {
    if (count < 1) return elx_value_int(0);
    if (args[0].type == ELX_VAL_INT)
        return elx_value_int(llabs(args[0].integer));
    return elx_value_float(fabs(args[0].real));
}

static elx_value_t std_math_sqrt(elx_value_t *args, uint32_t count) {
    (void)count;
    return elx_value_float(sqrt(args[0].real));
}

static elx_value_t std_math_pow(elx_value_t *args, uint32_t count) {
    (void)count;
    return elx_value_float(pow(args[0].real, args[1].real));
}

static elx_value_t std_math_pi(elx_value_t *args, uint32_t count) {
    (void)args; (void)count;
    return elx_value_float(M_PI);
}

static elx_value_t std_math_floor(elx_value_t *args, uint32_t count) {
    (void)count;
    return elx_value_float(floor(args[0].real));
}

static elx_value_t std_math_ceil(elx_value_t *args, uint32_t count) {
    (void)count;
    return elx_value_float(ceil(args[0].real));
}

static elx_value_t std_math_round(elx_value_t *args, uint32_t count) {
    (void)count;
    return elx_value_float(round(args[0].real));
}

static elx_value_t std_math_min(elx_value_t *args, uint32_t count) {
    if (count < 2) return args[0];
    if (args[0].type == ELX_VAL_INT && args[1].type == ELX_VAL_INT)
        return elx_value_int(args[0].integer < args[1].integer ? args[0].integer : args[1].integer);
    return elx_value_float(args[0].real < args[1].real ? args[0].real : args[1].real);
}

static elx_value_t std_math_max(elx_value_t *args, uint32_t count) {
    if (count < 2) return args[0];
    if (args[0].type == ELX_VAL_INT && args[1].type == ELX_VAL_INT)
        return elx_value_int(args[0].integer > args[1].integer ? args[0].integer : args[1].integer);
    return elx_value_float(args[0].real > args[1].real ? args[0].real : args[1].real);
}

/* ===========================
 * String module
 * =========================== */

static elx_value_t std_str_len(elx_value_t *args, uint32_t count) {
    (void)count;
    if (args[0].type == ELX_VAL_STRING)
        return elx_value_int((int64_t)strlen(args[0].string ? args[0].string : ""));
    return elx_value_int(0);
}

static elx_value_t std_str_upper(elx_value_t *args, uint32_t count) {
    (void)count;
    const char *s = args[0].string ? args[0].string : "";
    size_t len = strlen(s);
    char *buf = (char *)malloc(len + 1);
    size_t i;
    for (i = 0; i < len; i++) buf[i] = (char)toupper((unsigned char)s[i]);
    buf[len] = '\0';
    return elx_value_string(buf);
}

static elx_value_t std_str_lower(elx_value_t *args, uint32_t count) {
    (void)count;
    const char *s = args[0].string ? args[0].string : "";
    size_t len = strlen(s);
    char *buf = (char *)malloc(len + 1);
    size_t i;
    for (i = 0; i < len; i++) buf[i] = (char)tolower((unsigned char)s[i]);
    buf[len] = '\0';
    return elx_value_string(buf);
}

static elx_value_t std_str_concat(elx_value_t *args, uint32_t count) {
    if (count < 2) return args[0];
    const char *a = args[0].string ? args[0].string : "";
    const char *b = args[1].string ? args[1].string : "";
    size_t la = strlen(a), lb = strlen(b);
    char *buf = (char *)malloc(la + lb + 1);
    memcpy(buf, a, la);
    memcpy(buf + la, b, lb);
    buf[la + lb] = '\0';
    return elx_value_string(buf);
}

static elx_value_t std_str_substr(elx_value_t *args, uint32_t count) {
    const char *s = args[0].string ? args[0].string : "";
    int64_t start = count > 1 ? args[1].integer : 0;
    int64_t length = count > 2 ? args[2].integer : (int64_t)strlen(s);
    size_t slen = strlen(s);
    if (start < 0) start = 0;
    if (start >= (int64_t)slen) start = slen;
    if (length < 0) length = 0;
    if (start + length > (int64_t)slen) length = slen - start;
    char *buf = (char *)malloc(length + 1);
    memcpy(buf, s + start, (size_t)length);
    buf[length] = '\0';
    return elx_value_string(buf);
}

static elx_value_t std_str_repeat(elx_value_t *args, uint32_t count) {
    (void)count;
    const char *s = args[0].string ? args[0].string : "";
    int64_t times = args[1].integer;
    if (times <= 0) return elx_value_string("");
    size_t slen = strlen(s);
    size_t total = slen * (size_t)times;
    char *buf = (char *)malloc(total + 1);
    for (int64_t i = 0; i < times; i++)
        memcpy(buf + i * slen, s, slen);
    buf[total] = '\0';
    return elx_value_string(buf);
}

/* ===========================
 * List module
 * =========================== */

static int64_t g_list_next_id = 1;

typedef struct {
    elx_value_t *items;
    size_t count;
    size_t capacity;
    int64_t id;
} elx_list_t;

static elx_list_t *g_lists = NULL;
static size_t g_list_count = 0;
static size_t g_list_capacity = 0;

static elx_list_t *find_list(int64_t id) {
    for (size_t i = 0; i < g_list_count; i++) {
        if (g_lists[i].id == id) return &g_lists[i];
    }
    return NULL;
}

static elx_value_t std_list_new(elx_value_t *args, uint32_t count) {
    (void)args; (void)count;
    elx_list_t *lst = NULL;
    if (g_list_count >= g_list_capacity) {
        size_t new_cap = g_list_capacity ? g_list_capacity * 2 : 4;
        elx_list_t *new_arr = (elx_list_t *)realloc(g_lists, new_cap * sizeof(elx_list_t));
        if (!new_arr) return elx_value_int(-1);
        g_lists = new_arr;
        g_list_capacity = new_cap;
    }
    lst = &g_lists[g_list_count];
    lst->items = NULL;
    lst->count = 0;
    lst->capacity = 0;
    lst->id = g_list_next_id++;
    g_list_count++;
    return elx_value_int(lst->id);
}

static elx_value_t std_list_push(elx_value_t *args, uint32_t count) {
    if (count < 2) return elx_value_nil();
    elx_list_t *lst = find_list(args[0].integer);
    if (!lst) return elx_value_nil();
    if (lst->count >= lst->capacity) {
        size_t new_cap = lst->capacity ? lst->capacity * 2 : 4;
        elx_value_t *new_items = (elx_value_t *)realloc(lst->items, new_cap * sizeof(elx_value_t));
        if (!new_items) return elx_value_nil();
        lst->items = new_items;
        lst->capacity = new_cap;
    }
    lst->items[lst->count++] = args[1];
    return elx_value_int((int64_t)lst->count);
}

static elx_value_t std_list_get(elx_value_t *args, uint32_t count) {
    if (count < 2) return elx_value_nil();
    elx_list_t *lst = find_list(args[0].integer);
    if (!lst) return elx_value_nil();
    int64_t idx = args[1].integer;
    if (idx < 0 || idx >= (int64_t)lst->count) return elx_value_nil();
    return elx_value_copy(lst->items[idx]);
}

static elx_value_t std_list_len(elx_value_t *args, uint32_t count) {
    (void)count;
    elx_list_t *lst = find_list(args[0].integer);
    if (!lst) return elx_value_int(0);
    return elx_value_int((int64_t)lst->count);
}

/* ===========================
 * File I/O module
 * =========================== */

static elx_value_t std_file_read(elx_value_t *args, uint32_t count) {
    (void)count;
    const char *path = args[0].string;
    if (!path) return elx_value_string("");
    FILE *f = fopen(path, "rb");
    if (!f) return elx_value_string("");
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(sz + 1);
    if (!buf) { fclose(f); return elx_value_string(""); }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    buf[rd] = '\0';
    fclose(f);
    return elx_value_string(buf);
}

static elx_value_t std_file_write(elx_value_t *args, uint32_t count) {
    (void)count;
    const char *path = args[0].string;
    const char *content = count > 1 ? (args[1].string ? args[1].string : "") : "";
    if (!path) return elx_value_int(0);
    FILE *f = fopen(path, "w");
    if (!f) return elx_value_int(0);
    size_t written = fwrite(content, 1, strlen(content), f);
    fclose(f);
    return elx_value_int((int64_t)written);
}

static elx_value_t std_file_exists(elx_value_t *args, uint32_t count) {
    (void)count;
    const char *path = args[0].string;
    if (!path) return elx_value_bool(false);
    FILE *f = fopen(path, "r");
    if (!f) return elx_value_bool(false);
    fclose(f);
    return elx_value_bool(true);
}

static elx_value_t std_file_remove(elx_value_t *args, uint32_t count) {
    (void)count;
    const char *path = (count > 0 && args) ? args[0].string : NULL;
    if (!path) return elx_value_bool(false);
    if (remove(path) == 0) return elx_value_bool(true);
    return elx_value_bool(false);
}

/* ===========================
 * Time module
 * =========================== */

static elx_value_t std_time_now(elx_value_t *args, uint32_t count) {
    (void)args; (void)count;
    return elx_value_int((int64_t)time(NULL));
}

static elx_value_t std_time_to_iso8601(elx_value_t *args, uint32_t count) {
    (void)count;
    if (count == 0 || !args) return elx_value_nil();
    time_t ts = (time_t)args[0].integer;
    struct tm *tm = gmtime(&ts);
    if (!tm) return elx_value_nil();
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", tm);
    return elx_value_string(strdup(buf));
}

/* ===========================
 * Process module
 * =========================== */

static elx_value_t std_process_exit(elx_value_t *args, uint32_t count) {
    (void)count;
    int code = count > 0 ? (int)args[0].integer : 0;
    exit(code);
    return elx_value_nil();
}

static elx_value_t std_process_get_id(elx_value_t *args, uint32_t count) {
    (void)args; (void)count;
    return elx_value_int((int64_t)getpid());
}

static elx_value_t std_env_get_hostname(elx_value_t *args, uint32_t count) {
    (void)args; (void)count;
    char buf[256];
    if (gethostname(buf, sizeof(buf)) == 0) {
        return elx_value_string(strdup(buf));
    }
    return elx_value_nil();
}

/* ===========================
 * JSON module (basic serialization)
 * =========================== */

static elx_value_t std_json_stringify(elx_value_t *args, uint32_t count) {
    (void)count;
    if (!args || args[0].type != ELX_VAL_STRING) return elx_value_string("");
    const char *s = args[0].string ? args[0].string : "";
    size_t len = strlen(s);
    char *buf = (char *)malloc(len + 3);
    if (!buf) return elx_value_string("");
    buf[0] = '"';
    memcpy(buf + 1, s, len);
    buf[1 + len] = '"';
    buf[2 + len] = '\0';
    return elx_value_string(buf);
}

/* ===========================
 * Registration
 * =========================== */

void elx_stdlib_register(elx_runtime_t *runtime) {
    elx_native_register(runtime, "print", 1, native_print);
    elx_native_register(runtime, "printn", 1, native_print);
    elx_native_register(runtime, "clock", 0, native_clock);

    elx_native_register(runtime, "math.abs", 1, std_math_abs);
    elx_native_register(runtime, "math.sqrt", 1, std_math_sqrt);
    elx_native_register(runtime, "math.pow", 2, std_math_pow);
    elx_native_register(runtime, "math.pi", 0, std_math_pi);
    elx_native_register(runtime, "math.floor", 1, std_math_floor);
    elx_native_register(runtime, "math.ceil", 1, std_math_ceil);
    elx_native_register(runtime, "math.round", 1, std_math_round);
    elx_native_register(runtime, "math.min", 2, std_math_min);
    elx_native_register(runtime, "math.max", 2, std_math_max);

    elx_native_register(runtime, "str.len", 1, std_str_len);
    elx_native_register(runtime, "str.upper", 1, std_str_upper);
    elx_native_register(runtime, "str.lower", 1, std_str_lower);
    elx_native_register(runtime, "str.concat", 2, std_str_concat);
    elx_native_register(runtime, "str.substr", 3, std_str_substr);
    elx_native_register(runtime, "str.repeat", 2, std_str_repeat);

    elx_native_register(runtime, "list.new", 0, std_list_new);
    elx_native_register(runtime, "list.push", 2, std_list_push);
    elx_native_register(runtime, "list.get", 2, std_list_get);
    elx_native_register(runtime, "list.len", 1, std_list_len);

    elx_native_register(runtime, "file.read", 1, std_file_read);
    elx_native_register(runtime, "file.write", 2, std_file_write);
    elx_native_register(runtime, "file.exists", 1, std_file_exists);

    elx_native_register(runtime, "time.now", 0, std_time_now);

    elx_native_register(runtime, "process.exit", 1, std_process_exit);
    elx_native_register(runtime, "std.process.get_id", 0, std_process_get_id);
    elx_native_register(runtime, "std.env.get_hostname", 0, std_env_get_hostname);
    elx_native_register(runtime, "std.time.now", 0, std_time_now);
    elx_native_register(runtime, "std.time.to_iso8601", 1, std_time_to_iso8601);

    elx_native_register(runtime, "json.stringify", 1, std_json_stringify);

    /* std.* aliases for module-imported calls */
    elx_native_register(runtime, "std.io.print", 1, native_print);
    elx_native_register(runtime, "std.fs.read_text", 1, std_file_read);
    elx_native_register(runtime, "std.fs.write_text", 2, std_file_write);
    elx_native_register(runtime, "std.fs.exists", 1, std_file_exists);
    elx_native_register(runtime, "std.fs.remove", 1, std_file_remove);
    elx_native_register(runtime, "std.time.now", 0, std_time_now);
    elx_native_register(runtime, "std.math.abs", 1, std_math_abs);
    elx_native_register(runtime, "std.math.sqrt", 1, std_math_sqrt);
    elx_native_register(runtime, "std.str.len", 1, std_str_len);
    elx_native_register(runtime, "std.str.upper", 1, std_str_upper);
    elx_native_register(runtime, "std.str.lower", 1, std_str_lower);
    elx_native_register(runtime, "std.str.concat", 2, std_str_concat);
    elx_native_register(runtime, "std.list.new", 0, std_list_new);
    elx_native_register(runtime, "std.list.push", 2, std_list_push);
    elx_native_register(runtime, "std.list.get", 2, std_list_get);
    elx_native_register(runtime, "std.list.len", 1, std_list_len);
}
