/*============================================================================

  Copyright (c) 2013-2016 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef C2D_LOG_H
#define C2D_LOG_H

#include "camera_dbg.h"

#define C2D_LOG_SILENT   0
#define C2D_LOG_NORMAL   1
#define C2D_LOG_DEBUG    2
#define C2D_LOG_VERBOSE  3


/* ------- change this macro to enable mutex debugging for deadlock --------*/
#define C2D_DEBUG_MUTEX  0

extern volatile int32_t gcam_c2d_loglevel;
extern volatile int32_t gcam_pp_feat_mask;
/* -------------------------------------------------------------------------*/

#define IS_VALID_MASK(value, mask) ((value & (1 << mask)) ? 1 : 0)

/* Use bit 31 in the gcam_c2d_loglevelto enable the log to verify EIS is on */
#define C2D_EIS_LOG_BIT 0

#define C2D_LOW(fmt, args...) CLOGL(CAM_C2D_MODULE, fmt, ##args)
#define C2D_HIGH(fmt, args...) CLOGH(CAM_C2D_MODULE, fmt, ##args)
#define C2D_WARN(fmt, args...) CLOGW(CAM_C2D_MODULE, fmt, ##args)
#define C2D_ERR(fmt, args...) CLOGE(CAM_C2D_MODULE, fmt, ##args)
#define C2D_DBG(fmt, args...) CLOGD(CAM_C2D_MODULE, fmt, ##args)
#define C2D_INFO(fmt, args...) CLOGI(CAM_C2D_MODULE, fmt, ##args)
#define C2D_PROFILE(fmt, args...) C2D_DBG(CAM_C2D_MODULE, fmt, ##args)

#define C2D_FEATURE_INFO(fmt, args...) \
 if (IS_VALID_MASK(gcam_pp_feat_mask, C2D_EIS_LOG_BIT)) { \
   CLOGI(CAM_C2D_MODULE, "[FEATURE_INFO:] " fmt, ##args) \
 } \

#undef PTHREAD_MUTEX_LOCK
#undef PTHREAD_MUTEX_UNLOCK

#if (C2D_DEBUG_MUTEX == 1)
  #define PTHREAD_MUTEX_LOCK(m) do { \
    C2D_DBG("[c2d_mutex_log] before pthread_mutex_lock(%p)\n", \
      m); \
    pthread_mutex_lock(m); \
    C2D_DBG("[c2d_mutex_log] after pthread_mutex_lock(%p)\n", \
      m); \
  } while(0)

  #define PTHREAD_MUTEX_UNLOCK(m) do { \
    C2D_DBG("[c2d_mutex_log] before pthread_mutex_unlock(%p)\n", \
      m); \
    pthread_mutex_unlock(m); \
    C2D_DBG("[c2d_mutex_log] after pthread_mutex_unlock(%p)\n", \
      m); \
  } while(0)
#else
  #define PTHREAD_MUTEX_LOCK(m)   pthread_mutex_lock(m)
  #define PTHREAD_MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#endif
#endif
