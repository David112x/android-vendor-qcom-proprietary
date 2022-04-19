/******************************************************************************
  @file    client.h
  @brief   Android performance PerfLock library

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2014,2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __PERFCLIENT_H__
#define __PERFCLIENT_H__

#define QC_DEBUG_ERRORS 1
#ifdef QC_DEBUG_ERRORS
#define QLOGE(...)    ALOGE(__VA_ARGS__)
#else
#define QLOGE(...)
#endif

#if QC_DEBUG
#define QLOGW(...)    ALOGW(__VA_ARGS__)
#define QLOGI(...)    ALOGI(__VA_ARGS__)
#define QLOGV(...)    ALOGV(__VA_ARGS__)
#define QCLOGE(...)   ALOGE(__VA_ARGS__)
#else
#define QLOGW(...)
#define QLOGI(...)
#define QLOGV(...)
#define QCLOGE(...)
#endif
#define PROP_VAL_LENGTH 128
typedef struct {
    char value[PROP_VAL_LENGTH];
} PropVal;

int perf_lock_acq(int, int, int[], int);
int perf_hint(int, const char *, int, int);
int perf_get_feedback(int, const char *);
int perf_lock_rel(int);
void perf_lock_reset(void);
int perf_lock_use_profile(int, int);
void perf_lock_cmd(int);
PropVal perf_get_prop(const char *, const char *);
PropVal perf_wait_get_prop(const char * , const char *);
#endif

#ifdef __cplusplus
}
#endif
