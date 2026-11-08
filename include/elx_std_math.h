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

#ifndef ELX_STD_MATH_H
#define ELX_STD_MATH_H

#include "elx_std.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_PI 3.14159265358979323846
#define ELX_E 2.71828182845904523536

double elx_math_abs(double x);
int elx_math_sign(double x);
double elx_math_sqrt(double x);
double elx_math_cbrt(double x);
double elx_math_pow(double base, double exp);
double elx_math_exp(double x);
double elx_math_exp2(double x);
double elx_math_log(double x);
double elx_math_log2(double x);
double elx_math_log10(double x);
double elx_math_sin(double x);
double elx_math_cos(double x);
double elx_math_tan(double x);
double elx_math_asin(double x);
double elx_math_acos(double x);
double elx_math_atan(double x);
double elx_math_atan2(double y, double x);
double elx_math_sinh(double x);
double elx_math_cosh(double x);
double elx_math_tanh(double x);
double elx_math_floor(double x);
double elx_math_ceil(double x);
double elx_math_round(double x);
double elx_math_trunc(double x);
double elx_math_fmod(double x, double y);
double elx_math_remainder(double x, double y);
double elx_math_hypot(double x, double y);
double elx_math_fmax(double x, double y);
double elx_math_fmin(double x, double y);
double elx_math_fdim(double x, double y);
double elx_math_nearbyint(double x);
double elx_math_rint(double x);
long elx_math_lround(double x);
long long elx_math_llround(double x);
int elx_math_is_nan(double x);
int elx_math_is_inf(double x);
int elx_math_is_finite(double x);
int elx_math_is_normal(double x);
double elx_math_fabs(double x);
double elx_math_fadd(double x, double y);
double elx_math_fsub(double x, double y);
double elx_math_fmul(double x, double y);
double elx_math_fdiv(double x, double y);
double elx_math_fmadd(double x, double y, double z);
double elx_math_fmsub(double x, double y, double z);
const char *elx_math_version(void);

#ifdef __cplusplus
}
#endif

#endif
