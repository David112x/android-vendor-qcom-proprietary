/*============================================================================

  Copyright (c) 2013-2016 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef CPP_LOG_H
#define CPP_LOG_H

#include "camera_dbg.h"

#define CPP_LOG_SILENT     0
#define CPP_LOG_NORMAL     1
#define CPP_LOG_DEBUG      2
#define CPP_LOG_VERBOSE    3

/* -------------- change this macro to enable profiling logs  --------------*/
#define CPP_DEBUG_PROFILE  0

/* ------- change this macro to enable mutex debugging for deadlock --------*/
#define CPP_DEBUG_MUTEX    0

extern volatile int32_t gcam_cpp_loglevel;

/* default value 0xFF */
volatile uint32_t g_cpp_log_featureMask;

/*
 * WNR = 1
 * ASF = 2
 * TNR = 4
 * CROP = 8
 * PROFILE = 16
 * FRAME_MSG = 32
 * FRAME_INFO = 64
 * BUFFER LOG = 128
 * META DATA = 256
 * FEATURE = 512
 * STRIPE = 1024
 * STRIPE SCALE = 2048
 * STRIPE DSDN = 4096
 * STRIPE MMU PREFETCH = 8192
 * STRIPE ASF = 16384
 * STRIPE INPUT = 32768
 * STRIPE FETCH ENGINE = 65536
 * STRIPE WRITE ENGINE = 131072
 * STRIPE ROTATION = 262144
 * STRIPE WNR BIT = 524288
 * FEATURE_LOG_BIT = 1048576
 * CLOCK = 2097152
 */

typedef enum {
  CPP_WNR_BIT,
  CPP_ASF_BIT,
  CPP_TNR_BIT,
  CPP_CROP_BIT,
  CPP_PROFILE_BIT,
  CPP_FRAME_MSG_BIT,
  CPP_FRAME_INFO_BIT,
  CPP_BUF_BIT,
  CPP_PER_FRAME_BIT,
  CPP_METADATA_BIT,
  CPP_STRIPE_BIT,
  CPP_STRIPE_SCALE_BIT,
  CPP_STRIPE_DSDN_BIT,
  CPP_STRIPE_MMU_BIT,
  CPP_STRIPE_ASF_BIT,
  CPP_STRIPE_IP_BIT,
  CPP_STRIPE_FE_BIT,
  CPP_STRIPE_WE_BIT,
  CPP_STRIPE_ROT_BIT,
  CPP_STRIPE_WNR_BIT,
  CPP_FEATURE_BIT,
  CPP_CLOCK_BIT,
} cpp_log_fmask;

#define IS_VALID_MASK(value, mask) ((value & (1 << mask)) ? 1 : 0)

/* -------------------------------------------------------------------------*/
#define CPP_LOW(fmt, args...) CLOGL(CAM_CPP_MODULE, fmt, ##args)
#define CPP_HIGH(fmt, args...) CLOGH(CAM_CPP_MODULE, fmt, ##args)
#define CPP_ERR(fmt, args...) CLOGE(CAM_CPP_MODULE, fmt, ##args)
#define CPP_DBG(fmt, args...) CLOGD(CAM_CPP_MODULE, fmt, ##args)
#define CPP_INFO(fmt, args...) CLOGI(CAM_CPP_MODULE, fmt, ##args)
#define CPP_WARN(fmt, args...) CLOGW(CAM_CPP_MODULE, fmt, ##args)


/* Profile logs */
#define CPP_PROFILE(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_PROFILE_BIT)) { \
    CLOGD(CAM_CPP_MODULE, "[PROFILE:] " fmt, ##args) \
  } \

/* Frame message */
#define CPP_FRAME_MSG(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_FRAME_MSG_BIT)) { \
    CLOGD(CAM_CPP_MODULE, fmt, ##args) \
  } \

/*stripe related messgaes */
#define CPP_STRIPE(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[STRIPE:] " fmt, ##args) \
  } \

#define CPP_STRIPE_SCALE(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_SCALE_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[STRIPE:] " fmt, ##args) \
  } \

#define CPP_STRIPE_DSDN(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_DSDN_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[STRIPE:] " fmt, ##args) \
  } \

#define CPP_STRIPE_MMU(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_MMU_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[STRIPE:] " fmt, ##args) \
  } \

#define CPP_STRIPE_ASF(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_ASF_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[STRIPE:] " fmt, ##args) \
  } \

#define CPP_STRIPE_IP(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_IP_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[STRIPE:] " fmt, ##args) \
  } \

#define CPP_STRIPE_FE(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_FE_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[STRIPE:] " fmt, ##args) \
  } \

#define CPP_STRIPE_WE(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_WE_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[STRIPE:] " fmt, ##args) \
  } \

#define CPP_STRIPE_ROT(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_ROT_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[STRIPE:] " fmt, ##args) \
  } \

#define CPP_STRIPE_WNR(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_WNR_BIT)) { \
    CLOGL(CAM_CPP_MODULE, fmt, ##args) \
  } \
/* ASF related message */
#define CPP_ASF_LOW(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_ASF_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[ASF:] " fmt, ##args) \
  } \

#define CPP_ASF_HIGH(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_ASF_BIT)) { \
    CLOGH(CAM_CPP_MODULE, "[ASF:] " fmt, ##args) \
  } \

#define CPP_ASF_DBG(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_ASF_BIT)) { \
    CLOGD(CAM_CPP_MODULE, "[ASF:] " fmt, ##args) \
  } \

#define CPP_ASF_ERR(fmt, args...) \
    CLOGE(CAM_CPP_MODULE, "[ASF:] " fmt, ##args)

/* Prepare frame related messages */
#define CPP_FRAME_LOW(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_FRAME_INFO_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[PREPARE_FRAME:] " fmt, ##args) \
  } \

#define CPP_FRAME_HIGH(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_FRAME_INFO_BIT)) { \
    CLOGH(CAM_CPP_MODULE, "[PREPARE_FRAME:] " fmt, ##args) \
  } \

#define CPP_FRAME_DBG(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_FRAME_INFO_BIT)) { \
    CLOGD(CAM_CPP_MODULE, "[PREPARE_FRAME:] " fmt, ##args) \
  } \

#define CPP_FRAME_ERR(fmt, args...) \
    CLOGE(CAM_CPP_MODULE, "[PREPARE_FRAME:] " fmt, ##args)

/* Crop related messages */
#define CPP_CROP_LOW(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_CROP_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[CROP:] " fmt, ##args) \
  } \

#define CPP_CROP_HIGH(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_CROP_BIT)) { \
    CLOGH(CAM_CPP_MODULE, "[CROP:] " fmt, ##args) \
  } \

#define CPP_CROP_DBG(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_CROP_BIT)) { \
    CLOGD(CAM_CPP_MODULE, "[CROP:] " fmt, ##args) \
  } \

#define CPP_CROP_WARN(fmt, args...) \
    CLOGW(CAM_CPP_MODULE, "[CROP:] " fmt, ##args)

#define CPP_CROP_ERR(fmt, args...) \
    CLOGE(CAM_CPP_MODULE, "[CROP:] " fmt, ##args)

/* TNR related messages */
#define CPP_TNR_LOW(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_TNR_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[TNR:] " fmt, ##args) \
  } \

#define CPP_TNR_HIGH(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_TNR_BIT)) { \
    CLOGH(CAM_CPP_MODULE, "[TNR:] " fmt, ##args) \
  } \

#define CPP_TNR_DBG(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_TNR_BIT)) { \
    CLOGD(CAM_CPP_MODULE, "[TNR:] " fmt, ##args) \
  } \

#define CPP_TNR_ERR(fmt, args...) \
    CLOGE(CAM_CPP_MODULE, "[TNR:] " fmt, ##args)

/* BUF related messages */
#define CPP_BUF_LOW(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_BUF_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[CPP_BUF:] " fmt, ##args) \
  } \

#define CPP_BUF_HIGH(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_BUF_BIT)) { \
    CLOGH(CAM_CPP_MODULE, "[CPP_BUF:] " fmt, ##args) \
  } \

#define CPP_BUF_DBG(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_BUF_BIT)) { \
    CLOGD(CAM_CPP_MODULE, "[CPP_BUF:] " fmt, ##args) \
  } \

#define CPP_BUF_ERR(fmt, args...) \
    CLOGE(CAM_CPP_MODULE, "[CPP_BUF:] " fmt, ##args)

/* PER_FRAME related messages */
#define CPP_PER_FRAME_LOW(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_PER_FRAME_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[PER_FRAME:] " fmt, ##args) \
  } \

#define CPP_PER_FRAME_HIGH(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_PER_FRAME_BIT)) { \
    CLOGH(CAM_CPP_MODULE, "[PER_FRAME:] " fmt, ##args) \
  } \

#define CPP_PER_FRAME_DBG(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_PER_FRAME_BIT)) { \
    CLOGD(CAM_CPP_MODULE, "[PER_FRAME:] " fmt, ##args) \
  } \

#define CPP_PER_FRAME_ERR(fmt, args...) \
    CLOGE(CAM_CPP_MODULE, "[PER_FRAME:] " fmt, ##args)

/* Meta data related messages */
#define CPP_META_LOW(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_METADATA_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[METADATA:] " fmt, ##args) \
  } \

#define CPP_META_HIGH(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_METADATA_BIT)) { \
    CLOGH(CAM_CPP_MODULE, "[METADATA:] " fmt, ##args) \
  } \

#define CPP_META_DBG(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_METADATA_BIT)) { \
    CLOGD(CAM_CPP_MODULE, "[METADATA:] " fmt, ##args) \
  } \

#define CPP_META_ERR(fmt, args...) \
    CLOGE(CAM_CPP_MODULE, "[METADATA:] " fmt, ##args)

/* WNR related messages */
#define CPP_DENOISE_LOW(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_WNR_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[WNR:] " fmt, ##args) \
  } \

#define CPP_DENOISE_HIGH(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_WNR_BIT)) { \
    CLOGH(CAM_CPP_MODULE, "[WNR:] " fmt, ##args) \
  } \

#define CPP_DENOISE_DBG(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_WNR_BIT)) { \
    CLOGD(CAM_CPP_MODULE, "[WNR:] " fmt, ##args) \
  } \

#define CPP_DENOISE_ERR(fmt, args...) \
    CLOGE(CAM_CPP_MODULE, "[WNR:] " fmt, ##args)

#define CPP_FEATURE_INFO(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_FEATURE_BIT)) { \
    CLOGI(CAM_CPP_MODULE, "[FEATURE_INFO:] " fmt, ##args) \
  } \

/* CLOCK related messages */
#define CPP_CLOCK_LOW(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_CLOCK_BIT)) { \
    CLOGL(CAM_CPP_MODULE, "[CLOCK:] " fmt, ##args) \
  } \

#define CPP_CLOCK_HIGH(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_CLOCK_BIT)) { \
    CLOGH(CAM_CPP_MODULE, "[CLOCK:] " fmt, ##args) \
  } \

#define CPP_CLOCK_DBG(fmt, args...) \
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_CLOCK_BIT)) { \
    CLOGD(CAM_CPP_MODULE, "[CLOCK:] " fmt, ##args) \
  } \

#define CPP_CLOCK_ERR(fmt, args...) \
    CLOGE(CAM_CPP_MODULE, "[CLOCK:] " fmt, ##args)

#undef PTHREAD_MUTEX_LOCK
#undef PTHREAD_MUTEX_UNLOCK

#if (CPP_DEBUG_MUTEX)
  #define PTHREAD_MUTEX_LOCK(m) do { \
    CPP_HIGH("[cpp_mutex_log] before pthread_mutex_lock(%p)", m); \
    pthread_mutex_lock(m); \
    CPP_HIGH("[cpp_mutex_log] after pthread_mutex_lock(%p)", m); \
  } while(0)

  #define PTHREAD_MUTEX_UNLOCK(m) do { \
    CPP_HIGH("[cpp_mutex_log] before pthread_mutex_unlock(%p)\n", m); \
    pthread_mutex_unlock(m); \
    CPP_HIGH("[cpp_mutex_log] after pthread_mutex_unlock(%p)\n", m); \
  } while(0)
#else
  #define PTHREAD_MUTEX_LOCK(m)   pthread_mutex_lock(m)
  #define PTHREAD_MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#endif
#endif
