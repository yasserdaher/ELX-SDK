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

#include "elx_std_math.h"

const char *elx_math_version(void) { return "0.1.0"; }

double elx_math_abs(double x) { return fabs(x); }
int elx_math_sign(double x) { return (x > 0) - (x < 0); }
double elx_math_sqrt(double x) { return sqrt(x); }
double elx_math_cbrt(double x) { return cbrt(x); }
double elx_math_pow(double base, double exp) { return pow(base, exp); }
double elx_math_exp(double x) { return exp(x); }
double elx_math_exp2(double x) { return exp2(x); }
double elx_math_log(double x) { return log(x); }
double elx_math_log2(double x) { return log2(x); }
double elx_math_log10(double x) { return log10(x); }
double elx_math_sin(double x) { return sin(x); }
double elx_math_cos(double x) { return cos(x); }
double elx_math_tan(double x) { return tan(x); }
double elx_math_asin(double x) { return asin(x); }
double elx_math_acos(double x) { return acos(x); }
double elx_math_atan(double x) { return atan(x); }
double elx_math_atan2(double y, double x) { return atan2(y, x); }
double elx_math_sinh(double x) { return sinh(x); }
double elx_math_cosh(double x) { return cosh(x); }
double elx_math_tanh(double x) { return tanh(x); }
double elx_math_floor(double x) { return floor(x); }
double elx_math_ceil(double x) { return ceil(x); }
double elx_math_round(double x) { return round(x); }
double elx_math_trunc(double x) { return trunc(x); }
double elx_math_fmod(double x, double y) { return fmod(x, y); }
double elx_math_remainder(double x, double y) { return remainder(x, y); }
double elx_math_hypot(double x, double y) { return hypot(x, y); }
double elx_math_fmax(double x, double y) { return fmax(x, y); }
double elx_math_fmin(double x, double y) { return fmin(x, y); }
double elx_math_fdim(double x, double y) { return fdim(x, y); }
double elx_math_nearbyint(double x) { return nearbyint(x); }
double elx_math_rint(double x) { return rint(x); }
long elx_math_lround(double x) { return lround(x); }
long long elx_math_llround(double x) { return llround(x); }
int elx_math_is_nan(double x) { return isnan(x); }
int elx_math_is_inf(double x) { return isinf(x); }
int elx_math_is_finite(double x) { return isfinite(x); }
int elx_math_is_normal(double x) { return isnormal(x); }
double elx_math_fabs(double x) { return fabs(x); }
double elx_math_fadd(double x, double y) { return x + y; }
double elx_math_fsub(double x, double y) { return x - y; }
double elx_math_fmul(double x, double y) { return x * y; }
double elx_math_fdiv(double x, double y) { return x / y; }
double elx_math_fmadd(double x, double y, double z) { return fma(x, y, z); }
double elx_math_fmsub(double x, double y, double z) { return x * y - z; }
