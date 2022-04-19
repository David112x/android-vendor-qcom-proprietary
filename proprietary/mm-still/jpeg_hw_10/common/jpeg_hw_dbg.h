/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __JPEG_HW_DBG_H__
#define __JPEG_HW_DBG_H__

#define JPEGHW_USE_DYNAMIC_LOGGING

#define LOG_DEBUG
#define _ANDROID_

/** Loglevels
 * g_jpeghwloglevel <= 1 -> only error logs
 * g_jpeghwloglevel = 2 -> error and dbg
 */
extern volatile uint32_t g_jpeghwloglevel;

#undef JPEG_HW_DBG
#undef JPEG_HW_PR_ERR

#ifdef JPEGHW_USE_DYNAMIC_LOGGING
  #ifdef _ANDROID_
      #undef LOG_NIDEBUG
      #undef LOG_TAG
      #define LOG_NIDEBUG 0
      #define LOG_TAG "jpeg_hw"
      #include <utils/Log.h>
      #define JPEG_HW_PR_ERR(fmt, args...) ALOGE(fmt, ##args)
      #define JPEG_HW_INFO(fmt, args...) ALOGI(fmt, ##args)
      #define JPEG_HW_DBG(fmt, args...) ALOGD_IF(g_jpeghwloglevel >= 2, fmt, ##args)
  #else
      #define JPEG_HW_PR_ERR(fmt, args...) fprintf(stderr, fmt, ##args)
      #define JPEG_HW_INFO(fmt, args...) fprintf(stderr, fmt, ##args)
      #define JPEG_HW_DBG(fmt, args...) fprintf(stderr, fmt, ##args)
  #endif
#else
#ifndef LOG_DEBUG
#ifdef _ANDROID_
#define LOG_NIDEBUG 0
#define LOG_TAG "jpeg_hw"
#include <utils/Log.h>
#define JPEG_HW_DBG(fmt, args...) do{}while(0)
#ifdef NEW_LOG_API
  #define JPEG_HW_PR_ERR(fmt, args...) ALOGE(fmt, ##args)
#else
  #define JPEG_HW_PR_ERR(fmt, args...) LOGE(fmt, ##args)
#endif
#else
#define JPEG_HW_DBG(fmt, args...) do{}while(0)
#define JPEG_HW_PR_ERR(fmt, args...) fprintf(stderr, fmt, ##args)
#endif
#else
#ifdef _ANDROID_
#define LOG_NIDEBUG 0
#define LOG_TAG "jpeg_hw"
#include <utils/Log.h>
#ifdef NEW_LOG_API
  #define JPEG_HW_DBG(fmt, args...) ALOGE(fmt, ##args)
  #define JPEG_HW_PR_ERR(fmt, args...) ALOGE(fmt, ##args)
#else
  #define JPEG_HW_DBG(fmt, args...) LOGE(fmt, ##args)
  #define JPEG_HW_PR_ERR(fmt, args...) LOGE(fmt, ##args)
#endif
#else
/*     #define GMN_DBG(fmt, args...) do{}while(0) */
#define JPEG_HW_DBG(fmt, args...) fprintf(fmt, ##args)
#define JPEG_HW_PR_ERR(fmt, args...) fprintf(stderr, fmt, ##args)
#endif
#endif
#endif

#endif /* __JPEG_HW_DBG_H__ */
