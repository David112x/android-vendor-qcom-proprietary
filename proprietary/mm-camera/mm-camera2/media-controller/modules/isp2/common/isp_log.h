/* isp_log.h
 *
 * Copyright (c) 2012-2014, 2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ISP_LOG_H__
#define __ISP_LOG_H__

#include "camera_dbg.h"
#include "isp_common.h"

#undef ISPLOG_HIGH
#undef ISPLOG_DBG
#undef ISP_ERR
#undef ISP_DBG
#undef ISP_HIGH

/* Enable ISPLOG_HIGH to print ISP_HIGH */
//#define ISPLOG_HIGH

/* Enable ISPLOG_DBG to print all log levels - ISP_DBG and ISP_HIGH */
//#define ISPLOG_DBG

/* ------- change this macro to enable mutex debugging for deadlock --------*/
#define ISP_DEBUG_MUTEX             0

/* ------- change this macro to enable hw update list debugging --------*/
#define ISP_DEBUG_HW_UPDATE_LIST    0

/* ------- change this macro to enable hw update list debugging --------*/
#define ISP_DEBUG_BUCKET_SWAP       0

#define ISP_HIGH_MOD(mod, fmt, args...) \
  if (isp_modules_loglevel[mod] >= 3) {CLOGH(CAM_ISP_MODULE, fmt, ##args);}
#define ISP_DBG_MOD(mod, fmt, args...) \
  if (isp_modules_loglevel[mod] >= 4) {CLOGD(CAM_ISP_MODULE, fmt, ##args);}
#define ISP_DBG_NO_FUNC_MOD(mod, fmt, args...) \
  if (isp_modules_loglevel[mod] >= 4) {CLOGI(CAM_ISP_MODULE, fmt, ##args);}

extern uint32_t isp_modules_loglevel[ISP_LOG_MAX];

#if defined(ISPLOG_DBG)
  #ifdef _ANDROID_
    #undef LOG_NIDEBUG
    #undef LOG_TAG
    #define LOG_NIDEBUG 0
    #define LOG_TAG "mm-camera-isp2"
    #include <utils/Log.h>
    #define ISP_ERR(fmt, args...) \
      CLOGE(CAM_ISP_MODULE, fmt, ##args)
    #define ISP_DBG(fmt, args...) \
      CLOGD(CAM_ISP_MODULE, fmt, ##args)
    #define ISP_DBG_NO_FUNC(fmt, args...) \
      CLOGD(CAM_ISP_MODULE, fmt, ##args)
    #define ISP_HIGH(fmt, args...) \
      CLOGH(CAM_ISP_MODULE, fmt, ##args)
    #define ISP_INFO(fmt, args...) \
      CLOGI(CAM_ISP_MODULE, fmt, ##args)\
    #define ISP_WARN(fmt, args...) \
      CLOGW(CAM_ISP_MODULE, fmt, ##args)
  #else
    #include <stdio.h>
    #define ISP_ERR(fmt, args...) fprintf(stderr, fmt, ##args)
    #define ISP_DBG(fmt, args...) fprintf(stderr, fmt, ##args)
    #define ISP_DBG_NO_FUNC(fmt, args...) fprintf(stderr, fmt, ##args)
    #define ISP_HIGH(fmt, args...) fprintf(stderr, fmt, ##args)
    #define ISP_INFO(fmt, args...) fprintf(stderr, fmt, ##args)
    #define ISP_WARN(fmt, args...) fprintf(stderr, fmt, ##args)
  #endif
#elif defined(ISPLOG_HIGH)
  #ifdef _ANDROID_
    #undef LOG_NIDEBUG
    #undef LOG_TAG
    #define LOG_NIDEBUG 0
    #define LOG_TAG "mm-camera-isp2"
    #include <utils/Log.h>
    #define ISP_ERR(fmt, args...) \
      CLOGE(CAM_ISP_MODULE, fmt, ##args)
    #define ISP_DBG(fmt, args...) do{}while(0)
    #define ISP_DBG_NO_FUNC(fmt, args...) do{}while(0)
    #define ISP_HIGH(fmt, args...) \
      CLOGH(CAM_ISP_MODULE, fmt, ##args)
    #define ISP_INFO(fmt, args...) \
      CLOGI(CAM_ISP_MODULE, fmt, ##args)
    #define ISP_WARN(fmt, args...) \
      CLOGW(CAM_ISP_MODULE, fmt, ##args)
  #else
    #include <stdio.h>
    #define ISP_ERR(fmt, args...) fprintf(stderr, fmt, ##args)
    #define ISP_DBG(fmt, args...) do{}while(0)
    #define ISP_DBG_NO_FUNC(fmt, args...) do{}while(0)
    #define ISP_HIGH(fmt, args...) fprintf(stderr, fmt, ##args)
    #define ISP_INFO(fmt, args...) fprintf(stderr, fmt, ##args)
    #define ISP_WARN(fmt, args...) fprintf(stderr, fmt, ##args)
  #endif
#else
  #ifdef _ANDROID_
    #undef LOG_NIDEBUG
    #undef LOG_TAG
    #define LOG_NIDEBUG 0
    #define LOG_TAG "mm-camera-isp2"
    #include <utils/Log.h>
    #define ISP_WARN(fmt, args...) CLOGW(CAM_ISP_MODULE, fmt, ##args)
    #define ISP_ERR(fmt, args...) CLOGE(CAM_ISP_MODULE, fmt, ##args)
    #define ISP_DBG(fmt, args...) do{}while(0)
    #define ISP_DBG_NO_FUNC(fmt, args...) do{}while(0)
    #define ISP_HIGH(fmt, args...) do{}while(0)
    #define ISP_INFO(fmt, args...) \
      CLOGI(CAM_ISP_MODULE, fmt, ##args)
  #else
    #include <stdio.h>
    #define ISP_ERR(fmt, args...) fprintf(stderr, fmt, ##args)
    #define ISP_WARN(fmt, args...) do{}while(0)
    #define ISP_DBG(fmt, args...) do{}while(0)
    #define ISP_DBG_NO_FUNC(fmt, args...) do{}while(0)
    #define ISP_HIGH(fmt, args...) do{}while(0)
    #define ISP_INFO(fmt, args...) do{}while(0)
  #endif
#endif

#if (ISP_DEBUG_MUTEX == 1)
  #define PTHREAD_MUTEX_LOCK(m) do { \
    ISP_HIGH("[isp_mutex_log] before pthread_mutex_lock(%p)", m); \
    pthread_mutex_lock(m); \
    ISP_HIGH("[isp_mutex_log] after pthread_mutex_lock(%p)", m); \
  } while(0)

  #define PTHREAD_MUTEX_UNLOCK(m) do { \
    ISP_HIGH("[isp_mutex_log] before pthread_mutex_unlock(%p)", m); \
    pthread_mutex_unlock(m); \
    ISP_HIGH("[isp_mutex_log] after pthread_mutex_unlock(%p)", m); \
  } while(0)
#else
  #define PTHREAD_MUTEX_LOCK(m)   pthread_mutex_lock(m)
  #define PTHREAD_MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#endif

#if (ISP_DEBUG_HW_UPDATE_LIST == 1)
  #ifdef _ANDROID_
    #undef LOG_NIDEBUG
    #undef LOG_TAG
    #define LOG_NIDEBUG 0
    #define LOG_TAG "mm-camera-isp2"
    #include <utils/Log.h>
    #define ISP_LOG_LIST(fmt, args...) \
      CLOGI(CAM_ISP_MODULE, " HWUPDATEDEBUG "fmt, ##args)
  #else
    #include <stdio.h>
    #define ISP_LOG_LIST(fmt, args...) fprintf(stderr, fmt, ##args)
  #endif
#else
  #define ISP_LOG_LIST(fmt, args...) do{}while(0)
#endif

#if (ISP_DEBUG_BUCKET_SWAP == 1)
  #ifdef _ANDROID_
    #undef LOG_NIDEBUG
    #undef LOG_TAG
    #define LOG_NIDEBUG 0
    #define LOG_TAG "mm-camera-isp2"
    #include <utils/Log.h>
    #define ISP_DBG_BUCKET_SWAP(fmt, args...) \
      CLOGI(CAM_ISP_MODULE, " BUCKET_SWAP "fmt, ##args)
  #else
    #include <stdio.h>
    #define ISP_DBG_BUCKET_SWAP(fmt, args...) fprintf(stderr, fmt, ##args)
  #endif
#else
  #define ISP_DBG_BUCKET_SWAP(fmt, args...) do{}while(0)
#endif

#endif /* __ISP_LOG_H__ */
