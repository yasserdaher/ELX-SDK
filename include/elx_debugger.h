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

#ifndef ELX_DEBUGGER_H
#define ELX_DEBUGGER_H

#include "elx_config.h"
#include "elx_std.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_DEBUG_MAX_BREAKPOINTS 256
#define ELX_DEBUG_MAX_WATCHPOINTS 64
#define ELX_DEBUG_MAX_VARIABLES 1024
#define ELX_DEBUG_MAX_FRAMES 128
#define ELX_DEBUG_MAX_STEP_COUNT 1000000

typedef enum {
    ELX_DEBUG_STATE_STOPPED = 0,
    ELX_DEBUG_STATE_RUNNING,
    ELX_DEBUG_STATE_PAUSED,
    ELX_DEBUG_STATE_EXITED,
    ELX_DEBUG_STATE_ERROR,
} elx_debug_state_t;

typedef enum {
    ELX_DEBUG_STEP_OVER = 0,
    ELX_DEBUG_STEP_IN,
    ELX_DEBUG_STEP_OUT,
    ELX_DEBUG_STEP_CONTINUE,
} elx_debug_step_t;

typedef struct {
    const char *file;
    size_t line;
    bool enabled;
    bool condition_set;
    char condition[256];
} elx_breakpoint_t;

typedef struct {
    const char *expression;
    char last_value[ELX_DEBUG_MAX_VARIABLES];
    bool has_changed;
} elx_watchpoint_t;

typedef struct {
    const char *name;
    const char *type;
    char value[ELX_DEBUG_MAX_VARIABLES];
    size_t address;
    bool is_local;
    bool is_mutable;
} elx_variable_t;

typedef struct {
    const char *function;
    const char *file;
    size_t line;
    size_t depth;
} elx_stack_frame_t;

typedef struct {
    elx_stack_frame_t frames[ELX_DEBUG_MAX_FRAMES];
    size_t frame_count;
} elx_stack_t;

typedef struct {
    elx_debug_state_t state;
    elx_breakpoint_t breakpoints[ELX_DEBUG_MAX_BREAKPOINTS];
    size_t breakpoint_count;
    elx_watchpoint_t watchpoints[ELX_DEBUG_MAX_WATCHPOINTS];
    size_t watchpoint_count;
    elx_variable_t variables[ELX_DEBUG_MAX_VARIABLES];
    size_t variable_count;
    elx_stack_t stack;
    uint64_t step_count;
    uint64_t hit_count;
    bool stop_on_error;
    bool verbose;
} elx_debugger_t;

typedef void (*elx_debug_event_fn)(elx_debug_state_t state, const char *event,
                                       void *user);

const char *elx_debugger_version(void);

elx_debugger_t *elx_debugger_new(void);
void elx_debugger_free(elx_debugger_t *dbg);

bool elx_debugger_load(elx_debugger_t *dbg, const char *elbc_path);
bool elx_debugger_load_source(elx_debugger_t *dbg, const char *source, size_t length);

bool elx_debugger_set_breakpoint(elx_debugger_t *dbg, const char *file, size_t line);
bool elx_debugger_remove_breakpoint(elx_debugger_t *dbg, const char *file, size_t line);
bool elx_debugger_toggle_breakpoint(elx_debugger_t *dbg, const char *file, size_t line);
bool elx_debugger_set_breakpoint_condition(elx_debugger_t *dbg, const char *file,
                                               size_t line, const char *condition);
bool elx_debugger_enable_breakpoint(elx_debugger_t *dbg, size_t index);
bool elx_debugger_disable_breakpoint(elx_debugger_t *dbg, size_t index);

elx_breakpoint_t *elx_debugger_get_breakpoint(const elx_debugger_t *dbg, size_t index);
size_t elx_debugger_breakpoint_count(const elx_debugger_t *dbg);
bool elx_debugger_breakpoint_at(const elx_debugger_t *dbg, const char *file, size_t line);

bool elx_debugger_set_watchpoint(elx_debugger_t *dbg, const char *expression);
bool elx_debugger_remove_watchpoint(elx_debugger_t *dbg, size_t index);
elx_watchpoint_t *elx_debugger_get_watchpoint(const elx_debugger_t *dbg, size_t index);
size_t elx_debugger_watchpoint_count(const elx_debugger_t *dbg);

bool elx_debugger_set_variable(elx_debugger_t *dbg, const char *name, const char *value);
elx_variable_t *elx_debugger_get_variable(const elx_debugger_t *dbg, const char *name);
elx_variable_t *elx_debugger_get_variable_at(const elx_debugger_t *dbg, size_t index);
size_t elx_debugger_variable_count(const elx_debugger_t *dbg);

bool elx_debugger_step(elx_debugger_t *dbg, elx_debug_step_t step);
bool elx_debugger_run(elx_debugger_t *dbg);
bool elx_debugger_pause(elx_debugger_t *dbg);
bool elx_debugger_continue(elx_debugger_t *dbg);
bool elx_debugger_stop(elx_debugger_t *dbg);
bool elx_debugger_reset(elx_debugger_t *dbg);

elx_stack_t *elx_debugger_stack(const elx_debugger_t *dbg);
elx_variable_t *elx_debugger_locals(const elx_debugger_t *dbg);
elx_variable_t *elx_debugger_globals(const elx_debugger_t *dbg);

bool elx_debugger_set_event_handler(elx_debugger_t *dbg, elx_debug_event_fn handler,
                                        void *user);
bool elx_debugger_set_stop_on_error(elx_debugger_t *dbg, bool stop);
bool elx_debugger_set_verbose(elx_debugger_t *dbg, bool verbose);

elx_debug_state_t elx_debugger_state(const elx_debugger_t *dbg);
uint64_t elx_debugger_step_count(const elx_debugger_t *dbg);
uint64_t elx_debugger_hit_count(const elx_debugger_t *dbg);

bool elx_debugger_print_stack(const elx_debugger_t *dbg, FILE *stream);
bool elx_debugger_print_variables(const elx_debugger_t *dbg, FILE *stream);
bool elx_debugger_print_breakpoints(const elx_debugger_t *dbg, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif
