/**********************************************************************
*  Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __IMG_DBG_H__
#define __IMG_DBG_H__
#include <stdio.h>
#include "camera_dbg.h"

/** Dyanmic Logging Values for logging level:
 *
 * setprop : "persist.vendor.camera.imglib.logs"
 *
 * 1 - ERROR (default)
 * 2 - WARNING and above
 * 3 - HIGH and above
 * 4 - MED and above
 * 5 - LOW and above
 * 6 - INFO and above
 */
extern volatile uint32_t g_imgloglevel;

/** Dyanmic Logging Values for module filtering:
 *
 * setprop : "persist.vendor.camera.imglib.logsMod"
 *
 * FACEPROC = 1
 * DENOISE = 2
 * WNR = 4
 * BASE = 8
 * CAC = 16
 * COMMON = 32
 * AFS = 64
 * HDR = 128
 * DCRF = 256
 * JPEGDMA = 512
 * SAT = 1024
 * Add the values above to enable multiple modules (default is all)
 */
extern volatile uint32_t g_imgLogModuleMask;

typedef enum {
  IMGLIB_FACEPROC_SHIFT,
  IMGLIB_DENOISE_SHIFT,
  IMGLIB_WNR_SHIFT,
  IMGLIB_BASE_SHIFT,
  IMGLIB_CAC_SHIFT,
  IMGLIB_COMMON_SHIFT,
  IMGLIB_AFS_SHIFT,
  IMGLIB_HDR_SHIFT,
  IMGLIB_DCRF_SHIFT,
  IMGLIB_JPEGDMA_SHIFT,
  IMGLIB_SAT_SHIFT,
  IMGLIB_QDC_COMMON_SHIFT,
} imglib_log_shift_type;

#define IS_SET(shift) (((g_imgLogModuleMask & 0xFFFFFFFF) & (1 << shift)) > 0)

#ifndef MODULE_MASK
#define MODULE_MASK 0
#endif

#ifdef _ANDROID_
  #define IDBG_ERROR(fmt, args...) CLOGE(CAM_IMGLIB_MODULE, fmt, ##args)
  #define IDBG_WARN(fmt, args...)            \
    if (IS_SET(MODULE_MASK)) {               \
      CLOGW(CAM_IMGLIB_MODULE, fmt, ##args); \
    }
  #define IDBG_HIGH(fmt, args...)            \
    if (IS_SET(MODULE_MASK)) {               \
      CLOGH(CAM_IMGLIB_MODULE, fmt, ##args); \
    }
  #define IDBG_MED(fmt, args...)             \
    if (IS_SET(MODULE_MASK)) {               \
      CLOGD(CAM_IMGLIB_MODULE, fmt, ##args); \
    }
  #define IDBG_LOW(fmt, args...)             \
    if (IS_SET(MODULE_MASK)) {               \
      CLOGL(CAM_IMGLIB_MODULE, fmt, ##args); \
    }
  #define IDBG_INFO(fmt, args...)             \
    if (IS_SET(MODULE_MASK)) {               \
      CLOGI(CAM_IMGLIB_MODULE, fmt, ##args); \
    }
#else
  #define IDBG_ERROR(fmt, args...) fprintf(stderr, fmt, ##args)
  #define IDBG_WARN(fmt, args...) (g_imgloglevel >= 2 && \
    IS_SET(MODULE_MASK)) > 0 ? fprintf(stderr, fmt, ##args) : do{}while(0)
  #define IDBG_HIGH(fmt, args...) (g_imgloglevel >= 3 && \
    IS_SET(MODULE_MASK)) > 0 ? fprintf(stderr, fmt, ##args) : do{}while(0)
  #define IDBG_MED(fmt, args...) (g_imgloglevel >= 4 && \
    IS_SET(MODULE_MASK)) > 0 ? fprintf(stderr, fmt, ##args) : do{}while(0)
  #define IDBG_LOW(fmt, args...) (g_imgloglevel >= 5 && \
    IS_SET(MODULE_MASK)) > 0 ? fprintf(stderr, fmt, ##args) : do{}while(0)
  #define IDBG_INFO(fmt, args...) (g_imgloglevel >= 6 && \
    IS_SET(MODULE_MASK)) > 0 ? fprintf(stderr, fmt, ##args) : do{}while(0)
#endif

/** IDBG_HERE:
 *
 * Prints current function and line number
 **/
#define IDBG_HERE IDBG_ERROR("%s()[%d]", __func__, __LINE__);

#define IMG_LOG_LEVEL_HIGH (g_imgloglevel >= 3)
#define IMG_LOG_LEVEL_MED (g_imgloglevel >= 4)


#define IMG_INIT_LOGGING() \
{ \
  char prop[PROPERTY_VALUE_MAX]; \
  property_get("persist.vendor.camera.imglib.logs", prop, "2"); \
  g_imgloglevel = (uint32_t)atoi(prop); \
  property_get("persist.vendor.camera.imglib.logsMod", prop, "65535"); \
  g_imgLogModuleMask = (uint32_t)atoi(prop); \
  IDBG_HIGH("###Img_Loglevel %d, moduleMask %d", \
    g_imgloglevel, g_imgLogModuleMask); \
}


#endif /* __IMG_DBG_H__ */
