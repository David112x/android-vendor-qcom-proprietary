/* sensor_debug.h
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __SENSOR_DEBUG_H__
#define __SENSOR_DEBUG_H__

#include "camera_dbg.h"

#undef SERR
#undef SHIGH
#undef SWARN
#undef SDBG
#undef SINFO
#undef SLOW

/* Find Max*/
#define SMAX(x,y) (((x)>(y)) ? (x) : (y))

/* Find array size*/
#define S_ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))

/*Use SERR for true error conditions that most likely need to be
fixed before product release*/
#define SERR(fmt, args...) CLOGE(CAM_SENSOR_MODULE, fmt, ##args)

/*Use SHIGH for high importance log messages*/
#define SHIGH(fmt, args...) CLOGH(CAM_SENSOR_MODULE, fmt, ##args)

/*Use SWARN to alert developer or tester of a potential problem which
needs to be looked at*/
#define SWARN(fmt, args...) CLOGW(CAM_SENSOR_MODULE, fmt, ##args)

/*Use SDBG for verbose logging during debugging*/
#define SDBG(fmt, args...) CLOGD(CAM_SENSOR_MODULE, fmt, ##args)

/*USE CLOGI for information which always need to be logged*/
#define SINFO(fmt, args...) CLOGI(CAM_SENSOR_MODULE, fmt, ##args)

/*Use SLOW for auxiliary log messages*/
#define SLOW(fmt, args...) CLOGL(CAM_SENSOR_MODULE, fmt, ##args)

/*  change this macro to enable mutex debugging for deadlock */
#define SENSOR_DEBUG_MUTEX    0

#if (SENSOR_DEBUG_MUTEX == 1)
  #define PTHREAD_MUTEX_LOCK(m) do { \
    SDBG("[sensor_mutex_log] before pthread_mutex_lock(%p)\n", m); \
    pthread_mutex_lock(m); \
    SDBG("[sensor_mutex_log] after pthread_mutex_lock(%p)\n", m); \
  } while(0)

  #define PTHREAD_MUTEX_UNLOCK(m) do { \
    SDBG("[sensor_mutex_log] before pthread_mutex_unlock(%p)\n", m); \
    pthread_mutex_unlock(m); \
    SDBG("[sensor_mutex_log] after pthread_mutex_unlock(%p)\n", m); \
  } while(0)
#else
  #define PTHREAD_MUTEX_LOCK(m)   pthread_mutex_lock(m)
  #define PTHREAD_MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#endif /* SENSOR_DEBUG_MUTEX */

#endif /* __SENSOR_DBG_H__ */
