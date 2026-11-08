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

#ifndef ELX_ELX1_H
#define ELX_ELX1_H

#include "elx_config.h"
#include "elx_std.h"
#include "elx_diagnostics.h"
#include "elx_spec_freeze.h"
#include "elx_compat.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_ELX1_VERSION "1.0.0"
#define ELX_RC_VERSION "1.0.0-rc.1"
#define ELX_MAX_STAGE 4

typedef enum {
    ELX_STAGE_DEV = 0,
    ELX_STAGE_RC,
    ELX_STAGE_RELEASE,
    ELX_STAGE_STABLE,
} elx_stage_t;

typedef enum {
    ELX_STABILIZATION_PENDING = 0,
    ELX_STABILIZATION_IN_PROGRESS,
    ELX_STABILIZATION_COMPLETE,
    ELX_STABILIZATION_FAILED,
} elx_stabilization_t;

typedef struct {
    char version[ELX_FREEZE_MAX_VERSION];
    elx_stage_t stage;
    char build_date[32];
    char features[ELX_FREEZE_MAX_FEATURES][ELX_FREEZE_MAX_SPEC];
    size_t feature_count;
    elx_stabilization_t stabilization;
    bool release_ready;
    bool freeze_passed;
    bool compat_passed;
    bool diagnostics_passed;
    bool tooling_passed;
} elx_elx1_status_t;

const char *elx_elx1_version(void);
const char *elx_elx1_rc_version(void);

elx_elx1_status_t *elx_elx1_status_new(void);
void elx_elx1_status_free(elx_elx1_status_t *status);

elx_stage_t elx_elx1_stage(const elx_elx1_status_t *status);
const char *elx_elx1_stage_name(elx_stage_t stage);

bool elx_elx1_check_diagnostics(const elx_elx1_status_t *status,
                                    const elx_diagnostics_t *diag, bool *passed);
bool elx_elx1_check_tooling(const elx_elx1_status_t *status, bool *passed);
bool elx_elx1_check_freeze(const elx_elx1_status_t *status, bool *passed);
bool elx_elx1_check_compatibility(const elx_elx1_status_t *status, bool *passed);
bool elx_elx1_check_stabilization(const elx_elx1_status_t *status,
                                      elx_stabilization_t *state);

bool elx_elx1_release_candidate(elx_elx1_status_t *status);
bool elx_elx1_stabilize(elx_elx1_status_t *status);
bool elx_elx1_release(elx_elx1_status_t *status);
bool elx_elx1_print_report(const elx_elx1_status_t *status, FILE *stream);
bool elx_elx1_print_checklist(const elx_elx1_status_t *status, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif
