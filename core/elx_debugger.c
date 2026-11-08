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

#define _POSIX_C_SOURCE 200809L
#include "elx_debugger.h"
#include "elx_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *elx_debugger_version(void) { return "0.1.0"; }

elx_debugger_t *elx_debugger_new(void) {
    elx_debugger_t *d = (elx_debugger_t *)calloc(1, sizeof(*d));
    if (d) {
        d->state = ELX_DEBUG_STATE_STOPPED;
        d->stop_on_error = true;
    }
    return d;
}

void elx_debugger_free(elx_debugger_t *d) {
    if (!d) return;
    free(d);
}

bool elx_debugger_load(elx_debugger_t *d, const char *elbc_path) {
    (void)d; (void)elbc_path;
    return false;
}

bool elx_debugger_load_source(elx_debugger_t *d, const char *source, size_t length) {
    (void)d; (void)source; (void)length;
    return false;
}

bool elx_debugger_set_breakpoint(elx_debugger_t *d, const char *file, size_t line) {
    if (!d || !file || d->breakpoint_count >= ELX_DEBUG_MAX_BREAKPOINTS) return false;
    elx_breakpoint_t *bp = &d->breakpoints[d->breakpoint_count];
    bp->file = file;
    bp->line = line;
    bp->enabled = true;
    bp->condition_set = false;
    d->breakpoint_count++;
    return true;
}

bool elx_debugger_remove_breakpoint(elx_debugger_t *d, const char *file, size_t line) {
    if (!d || !file) return false;
    for (size_t i = 0; i < d->breakpoint_count; i++) {
        if (d->breakpoints[i].enabled &&
            d->breakpoints[i].line == line &&
            strcmp(d->breakpoints[i].file, file) == 0) {
            d->breakpoints[i].enabled = false;
            return true;
        }
    }
    return false;
}

bool elx_debugger_toggle_breakpoint(elx_debugger_t *d, const char *file, size_t line) {
    if (!d || !file) return false;
    for (size_t i = 0; i < d->breakpoint_count; i++) {
        if (d->breakpoints[i].line == line &&
            strcmp(d->breakpoints[i].file, file) == 0) {
            d->breakpoints[i].enabled = !d->breakpoints[i].enabled;
            return true;
        }
    }
    return elx_debugger_set_breakpoint(d, file, line);
}

bool elx_debugger_set_breakpoint_condition(elx_debugger_t *d, const char *file,
                                                    size_t line, const char *condition) {
    if (!d || !file || !condition) return false;
    for (size_t i = 0; i < d->breakpoint_count; i++) {
        if (d->breakpoints[i].line == line &&
            strcmp(d->breakpoints[i].file, file) == 0) {
            strncpy(d->breakpoints[i].condition, condition, 255);
            d->breakpoints[i].condition_set = true;
            return true;
        }
    }
    return false;
}

bool elx_debugger_enable_breakpoint(elx_debugger_t *d, size_t index) {
    if (!d || index >= d->breakpoint_count) return false;
    d->breakpoints[index].enabled = true;
    return true;
}

bool elx_debugger_disable_breakpoint(elx_debugger_t *d, size_t index) {
    if (!d || index >= d->breakpoint_count) return false;
    d->breakpoints[index].enabled = false;
    return true;
}

elx_breakpoint_t *elx_debugger_get_breakpoint(const elx_debugger_t *d, size_t index) {
    if (!d || index >= d->breakpoint_count) return NULL;
    return (elx_breakpoint_t *)&d->breakpoints[index];
}

size_t elx_debugger_breakpoint_count(const elx_debugger_t *d) {
    return d ? d->breakpoint_count : 0;
}

bool elx_debugger_breakpoint_at(const elx_debugger_t *d, const char *file, size_t line) {
    if (!d || !file) return false;
    for (size_t i = 0; i < d->breakpoint_count; i++) {
        if (d->breakpoints[i].enabled &&
            d->breakpoints[i].line == line &&
            strcmp(d->breakpoints[i].file, file) == 0) {
            return true;
        }
    }
    return false;
}

bool elx_debugger_set_watchpoint(elx_debugger_t *d, const char *expression) {
    if (!d || !expression || d->watchpoint_count >= ELX_DEBUG_MAX_WATCHPOINTS) return false;
    d->watchpoints[d->watchpoint_count].expression = expression;
    d->watchpoint_count++;
    return true;
}

bool elx_debugger_remove_watchpoint(elx_debugger_t *d, size_t index) {
    (void)d; (void)index;
    return false;
}

elx_watchpoint_t *elx_debugger_get_watchpoint(const elx_debugger_t *d, size_t index) {
    if (!d || index >= d->watchpoint_count) return NULL;
    elx_watchpoint_t *wp = (elx_watchpoint_t *)&d->watchpoints[index];
    return wp;
}

size_t elx_debugger_watchpoint_count(const elx_debugger_t *d) {
    return d ? d->watchpoint_count : 0;
}

bool elx_debugger_set_variable(elx_debugger_t *d, const char *name, const char *value) {
    if (!d || !name || !value) return false;
    for (size_t i = 0; i < d->variable_count; i++) {
        if (strcmp(d->variables[i].name, name) == 0) {
            strncpy(d->variables[i].value, value, ELX_DEBUG_MAX_VARIABLES - 1);
            return true;
        }
    }
    if (d->variable_count >= ELX_DEBUG_MAX_VARIABLES) return false;
    elx_variable_t *v = &d->variables[d->variable_count];
    v->name = name;
    v->type = "unknown";
    strncpy(v->value, value, ELX_DEBUG_MAX_VARIABLES - 1);
    v->address = 0;
    v->is_local = false;
    v->is_mutable = true;
    d->variable_count++;
    return true;
}

elx_variable_t *elx_debugger_get_variable(const elx_debugger_t *d, const char *name) {
    if (!d || !name) return NULL;
    for (size_t i = 0; i < d->variable_count; i++) {
        if (strcmp(d->variables[i].name, name) == 0) {
            elx_variable_t *v = (elx_variable_t *)&d->variables[i];
            return v;
        }
    }
    return NULL;
}

elx_variable_t *elx_debugger_get_variable_at(const elx_debugger_t *d, size_t index) {
    if (!d || index >= d->variable_count) return NULL;
    elx_variable_t *v = (elx_variable_t *)&d->variables[index];
    return v;
}

size_t elx_debugger_variable_count(const elx_debugger_t *d) {
    return d ? d->variable_count : 0;
}

bool elx_debugger_step(elx_debugger_t *d, elx_debug_step_t step) {
    if (!d) return false;
    d->step_count++;
    d->state = ELX_DEBUG_STATE_PAUSED;
    (void)step;
    return true;
}

bool elx_debugger_run(elx_debugger_t *d) {
    if (!d) return false;
    d->state = ELX_DEBUG_STATE_RUNNING;
    return true;
}

bool elx_debugger_pause(elx_debugger_t *d) {
    if (!d) return false;
    d->state = ELX_DEBUG_STATE_PAUSED;
    return true;
}

bool elx_debugger_continue(elx_debugger_t *d) {
    if (!d) return false;
    d->state = ELX_DEBUG_STATE_RUNNING;
    return true;
}

bool elx_debugger_stop(elx_debugger_t *d) {
    if (!d) return false;
    d->state = ELX_DEBUG_STATE_STOPPED;
    return true;
}

bool elx_debugger_reset(elx_debugger_t *d) {
    if (!d) return false;
    d->state = ELX_DEBUG_STATE_STOPPED;
    d->step_count = 0;
    d->hit_count = 0;
    d->breakpoint_count = 0;
    d->variable_count = 0;
    d->watchpoint_count = 0;
    d->stack.frame_count = 0;
    return true;
}

elx_stack_t *elx_debugger_stack(const elx_debugger_t *d) {
    if (!d) return NULL;
    return (elx_stack_t *)&d->stack;
}

elx_variable_t *elx_debugger_locals(const elx_debugger_t *d) {
    if (!d) return NULL;
    return (elx_variable_t *)d->variables;
}

elx_variable_t *elx_debugger_globals(const elx_debugger_t *d) {
    if (!d) return NULL;
    return (elx_variable_t *)d->variables;
}

bool elx_debugger_set_event_handler(elx_debugger_t *d, elx_debug_event_fn handler,
                                               void *user) {
    (void)d; (void)handler; (void)user;
    return false;
}

bool elx_debugger_set_stop_on_error(elx_debugger_t *d, bool stop) {
    if (!d) return false;
    d->stop_on_error = stop;
    return true;
}

bool elx_debugger_set_verbose(elx_debugger_t *d, bool verbose) {
    if (!d) return false;
    d->verbose = verbose;
    return true;
}

elx_debug_state_t elx_debugger_state(const elx_debugger_t *d) {
    return d ? d->state : ELX_DEBUG_STATE_STOPPED;
}

uint64_t elx_debugger_step_count(const elx_debugger_t *d) {
    return d ? d->step_count : 0;
}

uint64_t elx_debugger_hit_count(const elx_debugger_t *d) {
    return d ? d->hit_count : 0;
}

bool elx_debugger_print_stack(const elx_debugger_t *d, FILE *stream) {
    if (!d || !stream) return false;
    fprintf(stream, "Stack (%zu frames):\n", d->stack.frame_count);
    for (size_t i = 0; i < d->stack.frame_count; i++) {
        fprintf(stream, "  #%zu %s() at %s:%zu (depth=%zu)\n",
                i, d->stack.frames[i].function ? d->stack.frames[i].function : "?",
                d->stack.frames[i].file ? d->stack.frames[i].file : "?",
                d->stack.frames[i].line, d->stack.frames[i].depth);
    }
    return true;
}

bool elx_debugger_print_variables(const elx_debugger_t *d, FILE *stream) {
    if (!d || !stream) return false;
    fprintf(stream, "Variables (%zu):\n", d->variable_count);
    for (size_t i = 0; i < d->variable_count; i++) {
        fprintf(stream, "  %s: %s = %s\n",
                d->variables[i].name ? d->variables[i].name : "?",
                d->variables[i].type ? d->variables[i].type : "?",
                d->variables[i].value);
    }
    return true;
}

bool elx_debugger_print_breakpoints(const elx_debugger_t *d, FILE *stream) {
    if (!d || !stream) return false;
    fprintf(stream, "Breakpoints (%zu):\n", d->breakpoint_count);
    for (size_t i = 0; i < d->breakpoint_count; i++) {
        if (!d->breakpoints[i].enabled) continue;
        fprintf(stream, "  #%zu %s:%zu%s\n", i,
                d->breakpoints[i].file ? d->breakpoints[i].file : "?",
                d->breakpoints[i].line,
                d->breakpoints[i].condition_set ?
                    " (condition: " : "");
        if (d->breakpoints[i].condition_set) {
            fprintf(stream, "%s)", d->breakpoints[i].condition);
        }
        fprintf(stream, "\n");
    }
    return true;
}
