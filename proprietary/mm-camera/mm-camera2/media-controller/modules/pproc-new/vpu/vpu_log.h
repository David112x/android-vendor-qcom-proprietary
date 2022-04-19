/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef VPU_LOG_H
#define VPU_LOG_H

#include "camera_dbg.h"

/* ------------- change this macro to change the logging level ------------- /
 * SILENT     0
 * NORMAL     1
 * DEBUG      2
 * VERBOSE    3
 * -------------------------------------------------------------------------*/
#define VPU_LOG_LEVEL       2

/* -------------- change this macro to enable profiling logs  --------------*/
#define VPU_DEBUG_PROFILE   0

/* ------- change this macro to enable mutex debugging for deadlock --------*/
#define VPU_DEBUG_MUTEX     0

/* -------------------------------------------------------------------------*/

#undef LOG_TAG
#define LOG_TAG "mm-camera-vpu"

#define VPU_ERR(fmt, args...) \
    CDBG_ERROR("%s:%d, ERROR "fmt"", __func__, __LINE__, ##args);

#define VPU_HIGH(fmt, args...) \
    if(VPU_LOG_LEVEL > 0) { \
      CDBG_ERROR("%s:%d, INFO "fmt"", __func__, __LINE__, ##args); \
    }

#define VPU_DBG(fmt, args...) \
    if(VPU_LOG_LEVEL > 1) { \
      CDBG_ERROR("%s:%d, DEBUG "fmt"", __func__, __LINE__, ##args); \
    }

#define VPU_LOW(fmt, args...) \
    if(VPU_LOG_LEVEL > 2) { \
      CDBG_ERROR("%s:%d, INFO "fmt"", __func__, __LINE__, ##args); \
    }

/* macro for profiling logs */
#undef VPU_PROFILE
#if (VPU_DEBUG_PROFILE == 1)
  #define VPU_PROFILE VPU_HIGH
#else
  #define VPU_PROFILE(fmt, args...) do {} while(0)
#endif

#undef PTHREAD_MUTEX_LOCK
#undef PTHREAD_MUTEX_UNLOCK

#if (VPU_DEBUG_MUTEX)
  #define PTHREAD_MUTEX_LOCK(m) do { \
    VPU_HIGH("[vpu_mutex_log] before pthread_mutex_lock(%p)", m); \
    pthread_mutex_lock(m); \
    VPU_HIGH("[vpu_mutex_log] after pthread_mutex_lock(%p)", m); \
  } while(0)

  #define PTHREAD_MUTEX_UNLOCK(m) do { \
    VPU_HIGH("[vpu_mutex_log] before pthread_mutex_unlock(%p)\n", m); \
    pthread_mutex_unlock(m); \
    VPU_HIGH("[vpu_mutex_log] after pthread_mutex_unlock(%p)\n", m); \
  } while(0)
#else
  #define PTHREAD_MUTEX_LOCK(m)   pthread_mutex_lock(m)
  #define PTHREAD_MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#endif

#endif
