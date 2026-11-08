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
#include "elx_std_time.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if !defined(_WIN32)
#include <sys/time.h>
#endif
const char *elx_std_time_version(void) { return "0.1.0"; }
elx_timestamp_t elx_time_now(void) {
    elx_timestamp_t ts;
#if defined(_WIN32)
    FILETIME ft; GetSystemTimeAsFileTime(&ft);
    ULONGLONG ull = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    ts.epoch_ns = (ull/100)*1000000; ts.epoch_us = ts.epoch_ns/1000; ts.epoch_ms = ts.epoch_us/1000;
#else
    struct timeval tv; gettimeofday(&tv, NULL);
    ts.epoch_ms = (uint64_t)tv.tv_sec*1000 + tv.tv_usec/1000;
    ts.epoch_us = (uint64_t)tv.tv_sec*1000000 + tv.tv_usec;
    ts.epoch_ns = ts.epoch_us*1000;
#endif
    return ts;
}
elx_timestamp_t elx_time_epoch(void) { return (elx_timestamp_t){0,0,0}; }
static void tm_to_elx_time(const struct tm *tm, elx_time_t *out) {
    if (!tm || !out) return;
    out->year=tm->tm_year+1900; out->month=tm->tm_mon+1; out->day=tm->tm_mday;
    out->hour=tm->tm_hour; out->minute=tm->tm_min; out->second=tm->tm_sec;
    out->millisecond=0; out->weekday=tm->tm_wday;
}
elx_time_t elx_time_local(void) {
    struct tm tm; time_t t=time(NULL);
#if defined(_WIN32)
    localtime_s(&tm,&t);
#else
    localtime_r(&t,&tm);
#endif
    elx_time_t r; tm_to_elx_time(&tm,&r); return r;
}
elx_time_t elx_time_utc(void) {
    struct tm tm; time_t t=time(NULL);
#if defined(_WIN32)
    gmtime_s(&tm,&t);
#else
    gmtime_r(&t,&tm);
#endif
    elx_time_t r; tm_to_elx_time(&tm,&r); return r;
}
elx_timestamp_t elx_time_from_string(const char *f, const char *s) { (void)f;(void)s; return elx_time_now(); }
char *elx_time_to_string(const char *format, elx_timestamp_t ts, size_t *ol) {
    (void)ts; time_t t=time(NULL); struct tm tm;
#if defined(_WIN32)
    localtime_s(&tm,&t);
#else
    localtime_r(&t,&tm);
#endif
    char buffer[256];
    int len=strftime(buffer,sizeof(buffer),format?format:"%Y-%m-%d %H:%M:%S",&tm);
    if(len<=0){if(ol)*ol=0;return NULL;}
    char *result=(char*)malloc((size_t)len+1);
    if(!result){if(ol)*ol=0;return NULL;}
    memcpy(result,buffer,(size_t)len+1);
    if(ol)*ol=(size_t)len;
    return result;
}
char *elx_time_to_iso8601(elx_timestamp_t ts, size_t *ol) {
    (void)ts; elx_time_t lt=elx_time_local();
    char *result=(char*)malloc(32);
    if(!result){if(ol)*ol=0;return NULL;}
    int len=snprintf(result,32,"%04d-%02d-%02dT%02d:%02d:%02d",lt.year,lt.month,lt.day,lt.hour,lt.minute,lt.second);
    if(ol)*ol=(size_t)len;
    return result;
}
char *elx_time_to_rfc3339(elx_timestamp_t ts, size_t *ol) { return elx_time_to_iso8601(ts,ol); }
elx_duration_t elx_time_diff(elx_timestamp_t s, elx_timestamp_t e) {
    elx_duration_t d; uint64_t dm=(e.epoch_ms>s.epoch_ms)?(e.epoch_ms-s.epoch_ms):(s.epoch_ms-e.epoch_ms);
    d.days=(int)(dm/(24*60*60*1000)); dm-=(uint64_t)d.days*24*60*60*1000;
    d.hours=(int)(dm/(60*60*1000)); dm-=(uint64_t)d.hours*60*60*1000;
    d.minutes=(int)(dm/(60*1000)); dm-=(uint64_t)d.minutes*60*1000;
    d.seconds=(int)(dm/1000); dm-=(uint64_t)d.seconds*1000;
    d.milliseconds=(int)dm; return d;
}
uint64_t elx_time_elapsed_ms(void) { return elx_time_now().epoch_ms; }
uint64_t elx_time_elapsed_us(void) { return elx_time_now().epoch_us; }
void elx_sleep_ms(uint32_t ms) {
#if defined(_WIN32)
    Sleep(ms);
#else
    usleep((useconds_t)ms*1000);
#endif
}
void elx_sleep_us(uint32_t us) {
#if defined(_WIN32)
    Sleep(us/1000);
#else
    usleep((useconds_t)us);
#endif
}
bool elx_time_is_leap_year(int y) { return (y%4==0&&y%100!=0)||(y%400==0); }
int elx_time_days_in_month(int y, int m) {
    static const int d[]={31,28,31,30,31,30,31,31,30,31,30,31};
    if(m<1||m>12) return 0;
    if(m==2&&elx_time_is_leap_year(y)) return 29;
    return d[m-1];
}
uint64_t elx_time_monotonic_ms(void) {
#if defined(_WIN32)
    return GetTickCount64();
#else
    struct timeval tv; gettimeofday(&tv,NULL);
    return (uint64_t)tv.tv_sec*1000+tv.tv_usec/1000;
#endif
}
uint64_t elx_time_perf_counter_freq(void) {
#if defined(_WIN32)
    LARGE_INTEGER f; QueryPerformanceFrequency(&f); return (uint64_t)f.QuadPart;
#else
    return 1000000;
#endif
}
double elx_time_perf_counter(void) {
#if defined(_WIN32)
    LARGE_INTEGER f,c; QueryPerformanceFrequency(&f); QueryPerformanceCounter(&c);
    return (double)c.QuadPart/(double)f.QuadPart;
#else
    struct timeval tv; gettimeofday(&tv,NULL);
    return (double)tv.tv_sec+(double)tv.tv_usec/1000000.0;
#endif
}
