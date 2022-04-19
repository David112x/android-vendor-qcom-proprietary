/*******************************************************************************
* Copyright (c) 2012-2015 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*******************************************************************************/

#ifndef __QIDBG_H__
#define __QIDBG_H__

#include <stdio.h>
#define ATRACE_TAG ATRACE_TAG_CAMERA
#include <cutils/trace.h>

#define MMSTILL_USE_DYNAMIC_LOGGING

#ifndef QIDBG_LOG_LEVEL
#define QIDBG_LOG_LEVEL 3
#endif

/** Loglevels
 * g_mmstillloglevel = 1 -> only error logs
 * g_mmstillloglevel = 2 -> error and high
 * g_mmstillloglevel = 3 -> error, high, medium
 * g_mmstillloglevel = 4 -> error, high, medium and low
 */
extern volatile uint32_t g_mmstillloglevel;

#define ATRACE_BEGIN_SNPRINTF(buf_size, fmt_str, ...) { \
  char trace_tag[buf_size]; \
  snprintf(trace_tag, buf_size, fmt_str, ##__VA_ARGS__); \
  ATRACE_BEGIN(trace_tag); \
}

#undef QIDBG
#ifdef MMSTILL_USE_DYNAMIC_LOGGING
    #ifdef _ANDROID_
        #undef LOG_NIDEBUG
        #undef LOG_TAG
        #define LOG_NIDEBUG 0
        #define LOG_TAG "mm-still"
        #include <utils/Log.h>
        #define QIDBG_INFO(fmt, args...) ALOGI(fmt, ##args)
        #define QIDBG_HIGH(fmt, args...) ALOGD_IF(g_mmstillloglevel >= 2, fmt, ##args)
        #define QIDBG_MED(fmt, args...) ALOGD_IF(g_mmstillloglevel >= 3, fmt, ##args)
        #define QIDBG_LOW(fmt, args...) ALOGD_IF(g_mmstillloglevel >= 4, fmt, ##args)
    #else
        #define QIDBG_INFO(fmt, args...) fprintf(stderr, fmt, ##args)
        #define QIDBG_HIGH(fmt, args...) fprintf(stderr, fmt, ##args)
        #define QIDBG_MED(fmt, args...) fprintf(stderr, fmt, ##args)
        #define QIDBG_LOW(fmt, args...) fprintf(stderr, fmt, ##args)
    #endif
#elif (QIDBG_LOG_LEVEL > 0)
    #ifdef _ANDROID_
        #undef LOG_NIDEBUG
        #undef LOG_TAG
        #define LOG_NIDEBUG 0
        #define LOG_TAG "mm-still"
        #include <utils/Log.h>
        #define QIDBG(fmt, args...) ALOGE(fmt, ##args)
        #define QINDBG(fmt, args...) do{}while(0)
    #else
        #define QIDBG(fmt, args...) fprintf(stderr, fmt, ##args)
    #endif
#else
    #define QIDBG(fmt, args...) do{}while(0)
    #define QINDBG(fmt, args...) do{}while(0)
#endif

#define QIDBG_ERROR(fmt, args...) ALOGE(fmt, ##args)

#ifndef MMSTILL_USE_DYNAMIC_LOGGING
#if (QIDBG_LOG_LEVEL >= 4)
  #define QIDBG_HIGH(...)   QIDBG(__VA_ARGS__)
  #define QIDBG_MED(...)    QIDBG(__VA_ARGS__)
  #define QIDBG_LOW(...)    QIDBG(__VA_ARGS__)
#elif (QIDBG_LOG_LEVEL == 3)
  #define QIDBG_HIGH(...)   QIDBG(__VA_ARGS__)
  #define QIDBG_MED(...)    QIDBG(__VA_ARGS__)
  #define QIDBG_LOW(...)    QINDBG(__VA_ARGS__)
#elif (QIDBG_LOG_LEVEL == 2)
  #define QIDBG_HIGH(...)   QIDBG(__VA_ARGS__)
  #define QIDBG_MED(...)    QINDBG(__VA_ARGS__)
  #define QIDBG_LOW(...)    QINDBG(__VA_ARGS__)
#elif (QIDBG_LOG_LEVEL == 1)
  #define QIDBG_HIGH(...)   QINDBG(__VA_ARGS__)
  #define QIDBG_MED(...)    QINDBG(__VA_ARGS__)
  #define QIDBG_LOW(...)    QINDBG(__VA_ARGS__)
#else
  #define QIDBG_HIGH(...)   QINDBG(__VA_ARGS__)
  #define QIDBG_MED(...)    QINDBG(__VA_ARGS__)
  #define QIDBG_LOW(...)    QINDBG(__VA_ARGS__)
#endif
#endif
#endif /* __QIDBG_H__ */
