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

#ifndef ELX_STD_TIME_H
#define ELX_STD_TIME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int millisecond;
    int weekday;
} elx_time_t;

typedef struct {
    uint64_t epoch_ms;
    uint64_t epoch_us;
    uint64_t epoch_ns;
} elx_timestamp_t;

typedef struct {
    int days;
    int hours;
    int minutes;
    int seconds;
    int milliseconds;
} elx_duration_t;

const char *elx_std_time_version(void);

elx_timestamp_t elx_time_now(void);
elx_timestamp_t elx_time_epoch(void);
elx_time_t elx_time_local(void);
elx_time_t elx_time_utc(void);
elx_timestamp_t elx_time_from_string(const char *format, const char *string);
char *elx_time_to_string(const char *format, elx_timestamp_t timestamp, size_t *out_length);
char *elx_time_to_iso8601(elx_timestamp_t timestamp, size_t *out_length);
char *elx_time_to_rfc3339(elx_timestamp_t timestamp, size_t *out_length);

elx_duration_t elx_time_diff(elx_timestamp_t start, elx_timestamp_t end);
uint64_t elx_time_elapsed_ms(void);
uint64_t elx_time_elapsed_us(void);
void elx_sleep_ms(uint32_t milliseconds);
void elx_sleep_us(uint32_t microseconds);
bool elx_time_is_leap_year(int year);
int elx_time_days_in_month(int year, int month);

uint64_t elx_time_monotonic_ms(void);
uint64_t elx_time_perf_counter_freq(void);
double elx_time_perf_counter(void);

#ifdef __cplusplus
}
#endif

#endif
