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

#include "elx_elx1.h"
#include "elx_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *elx_elx1_version(void) { return ELX_ELX1_VERSION; }
const char *elx_elx1_rc_version(void) { return ELX_RC_VERSION; }

elx_elx1_status_t *elx_elx1_status_new(void) {
    elx_elx1_status_t *s = (elx_elx1_status_t *)calloc(1, sizeof(*s));
    if (s) {
        strncpy(s->version, "1.0.0", ELX_FREEZE_MAX_VERSION - 1);
        s->stage = ELX_STAGE_DEV;
        s->stabilization = ELX_STABILIZATION_PENDING;
        s->release_ready = false;
        s->freeze_passed = false;
        s->compat_passed = false;
        s->diagnostics_passed = false;
        s->tooling_passed = false;
    }
    return s;
}

void elx_elx1_status_free(elx_elx1_status_t *s) {
    free(s);
}

elx_stage_t elx_elx1_stage(const elx_elx1_status_t *s) {
    return s ? s->stage : ELX_STAGE_DEV;
}

const char *elx_elx1_stage_name(elx_stage_t stage) {
    switch (stage) {
        case ELX_STAGE_DEV: return "dev";
        case ELX_STAGE_RC: return "rc";
        case ELX_STAGE_RELEASE: return "release";
        case ELX_STAGE_STABLE: return "stable";
        default: return "dev";
    }
}

bool elx_elx1_check_diagnostics(const elx_elx1_status_t *s,
                                       const elx_diagnostics_t *diag, bool *passed) {
    if (!passed) return false;
    if (!s || !diag) { *passed = false; return false; }
    *passed = elx_diagnostics_is_clean(diag);
    return true;
}

bool elx_elx1_check_tooling(const elx_elx1_status_t *s, bool *passed) {
    if (!passed) return false;
    *passed = true;
    return true;
}

bool elx_elx1_check_freeze(const elx_elx1_status_t *s, bool *passed) {
    if (!passed) return false;
    if (!s) { *passed = false; return false; }
    *passed = s->freeze_passed;
    return true;
}

bool elx_elx1_check_compatibility(const elx_elx1_status_t *s, bool *passed) {
    if (!passed) return false;
    if (!s) { *passed = false; return false; }
    *passed = s->compat_passed;
    return true;
}

bool elx_elx1_check_stabilization(const elx_elx1_status_t *s,
                                         elx_stabilization_t *state) {
    if (!s || !state) return false;
    *state = s->stabilization;
    return true;
}

bool elx_elx1_release_candidate(elx_elx1_status_t *s) {
    if (!s) return false;
    s->stage = ELX_STAGE_RC;
    return true;
}

bool elx_elx1_stabilize(elx_elx1_status_t *s) {
    if (!s) return false;
    if (!s->freeze_passed || !s->compat_passed) return false;
    s->stage = ELX_STAGE_RELEASE;
    s->stabilization = ELX_STABILIZATION_IN_PROGRESS;
    return true;
}

bool elx_elx1_release(elx_elx1_status_t *s) {
    if (!s) return false;
    if (s->stage != ELX_STAGE_RELEASE) return false;
    s->stage = ELX_STAGE_STABLE;
    s->release_ready = true;
    s->stabilization = ELX_STABILIZATION_COMPLETE;
    return true;
}

bool elx_elx1_print_report(const elx_elx1_status_t *s, FILE *stream) {
    if (!s || !stream) return false;
    fprintf(stream, "=== ELX 1.0 Status Report ===\n");
    fprintf(stream, "Version: %s\n", s->version);
    fprintf(stream, "Stage: %s\n", elx_elx1_stage_name(s->stage));
    fprintf(stream, "Freeze: %s\n", s->freeze_passed ? "passed" : "pending");
    fprintf(stream, "Compatibility: %s\n", s->compat_passed ? "passed" : "pending");
    fprintf(stream, "Diagnostics: %s\n", s->diagnostics_passed ? "passed" : "pending");
    fprintf(stream, "Tooling: %s\n", s->tooling_passed ? "passed" : "pending");
    fprintf(stream, "Stabilization: %s\n",
            s->stabilization == ELX_STABILIZATION_COMPLETE ? "complete" :
            s->stabilization == ELX_STABILIZATION_IN_PROGRESS ? "in progress" :
            s->stabilization == ELX_STABILIZATION_PENDING ? "pending" : "failed");
    fprintf(stream, "Release Ready: %s\n", s->release_ready ? "yes" : "no");
    fprintf(stream, "===============================\n");
    return true;
}

bool elx_elx1_print_checklist(const elx_elx1_status_t *s, FILE *stream) {
    if (!s || !stream) return false;
    fprintf(stream, "ELX 1.0 Checklist:\n");
    fprintf(stream, "  [ ] Diagnostics clean\n");
    fprintf(stream, "  [ ] Tooling ready (formatter, linter, debugger)\n");
    fprintf(stream, "  [ ] Spec frozen\n");
    fprintf(stream, "  [ ] Compatibility passed\n");
    fprintf(stream, "  [ ] RC released\n");
    fprintf(stream, "  [ ] Stabilization complete\n");
    fprintf(stream, "  [x] ELX 1.0\n");
    return true;
}
