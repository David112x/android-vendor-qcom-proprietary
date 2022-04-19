/*==========================================================
  *Copyright (c) 2014-2018 Qualcomm Technologies, Inc.
  *All Rights Reserved.
  *Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================*/
#ifndef __DEBUG_LIB_H__
#define __DEBUG_LIB_H__


#ifdef _ANDROID_
  #include <stdlib.h>
  #include <utils/Log.h>
  #include <cutils/properties.h>

  #undef SLOG_HIGH
  #undef SLOG_LOW
  #undef SERR
  #undef SLOW
  #undef SHIGH

  typedef enum {
      SENSOR_DBG_NONE   = 0,
      SENSOR_DBG_ERR    = 1,
      SENSOR_DBG_WARN   = 2,
      SENSOR_DBG_HIGH   = 3,
      SENSOR_DBG_LOW    = 4,
      SENSOR_DBG_DEBUG  = 5,
      SENSOR_DBG_INFO   = 6,
      SENSOR_DBG_MAX
  } sensor_debug_level_t;

  #undef LOG_NIDEBUG
  #define LOG_NIDEBUG 0
  #undef LOG_TAG
  #define LOG_TAG "mm-camera"

  typedef enum {
      CAM_GLBL_DBG_NONE  = 0,
      CAM_GLBL_DBG_ERR   = 1,
      CAM_GLBL_DBG_WARN  = 2,
      CAM_GLBL_DBG_HIGH  = 3,
      CAM_GLBL_DBG_LOW   = 4,
      CAM_GLBL_DBG_DEBUG = 5,
      CAM_GLBL_DBG_INFO  = 6
  } cam_global_debug_level_t;

  #define INVALID_LOG_LEVEL (-1)

  typedef struct {
    int g_dbg_level;
    int s_dbg_level;
  } sensor_global_map_t;

  static volatile int sdk_log_level = INVALID_LOG_LEVEL;

  static  const sensor_global_map_t g_map[] = {
      {
        .g_dbg_level = CAM_GLBL_DBG_NONE,
        .s_dbg_level = SENSOR_DBG_NONE,
      },
      {
        .g_dbg_level = CAM_GLBL_DBG_ERR,
        .s_dbg_level = SENSOR_DBG_ERR,
      },
      {
        .g_dbg_level = CAM_GLBL_DBG_WARN,
        .s_dbg_level = SENSOR_DBG_WARN,
      },
      {
        .g_dbg_level = CAM_GLBL_DBG_HIGH,
        .s_dbg_level = SENSOR_DBG_HIGH,
      },
      {
        .g_dbg_level = CAM_GLBL_DBG_LOW,
        .s_dbg_level = SENSOR_DBG_LOW,
      },
      {
        .g_dbg_level = CAM_GLBL_DBG_DEBUG,
        .s_dbg_level = SENSOR_DBG_DEBUG,
      },
      {
        .g_dbg_level = CAM_GLBL_DBG_INFO,
        .s_dbg_level = SENSOR_DBG_INFO,
      }
    };

  #define SMAX(x,y) (((x)>(y)) ? (x) : (y))
  #define S_ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))

  static int sdk_getLoglevel_(void){
      char prop[PROPERTY_VALUE_MAX];
      int i = 0;
      int global_debug_level, sensor_debug_level = SENSOR_DBG_ERR;
      property_get("persist.vendor.camera.global.debug", prop, "0");
      global_debug_level = atoi(prop);
      property_get("persist.vendor.camera.sensor.debug", prop, "0");
      sensor_debug_level = atoi(prop);
      for (i = 0; i < (int)S_ARRAYSIZE(g_map); i++) {
        if (global_debug_level == g_map[i].g_dbg_level) {
          return SMAX(sensor_debug_level,g_map[i].s_dbg_level);
        }
      }
    return SENSOR_DBG_ERR;
  }

  static int sdk_getLoglevel(void){
    if(sdk_log_level == INVALID_LOG_LEVEL) {
      sdk_log_level = sdk_getLoglevel_();
      return sdk_log_level;
    } else {
      return sdk_log_level;
    }
  }

  #define SERR(fmt, args...) \
    ALOGE("<SENSOR><ERROR> %s: %d: " fmt"\n", __func__, __LINE__, ##args)

  #define SHIGH(fmt, args...) \
    ALOGD_IF(sdk_getLoglevel() >= SENSOR_DBG_HIGH, \
    "<SENSOR>< HIGH> %s: %d: " fmt"\n", __func__, __LINE__, ##args)

  #define SWARN(fmt, args...) \
    ALOGW_IF(sdk_getLoglevel() >= SENSOR_DBG_WARN, \
    "<SENSOR>< WARN> %s: %d: " fmt"\n", __func__, __LINE__, ##args)

  #define SLOW(fmt, args...) \
    ALOGD_IF(sdk_getLoglevel() >= SENSOR_DBG_LOW, \
    "<SENSOR><  LOW> %s: %d: " fmt"\n", __func__, __LINE__, ##args)

  #define SDBG(fmt, args...) \
    ALOGD_IF(sdk_getLoglevel() >= SENSOR_DBG_DEBUG, \
    "<SENSOR><  DBG> %s: %d: " fmt"\n", __func__, __LINE__, ##args)

  #define SINFO(fmt, args...) \
    ALOGV("<SENSOR><  INFO> %s:%d " fmt"\n", __func__, __LINE__, ##args)
#else
  #include <stdio.h>
  #if defined(SLOW_DEBUG)
    #define SERR(fmt, args...) fprintf(stderr, fmt, ##args)
    #define SLOW(fmt, args...) fprintf(stderr, fmt, ##args)
    #define SHIGH(fmt, args...) fprintf(stderr, fmt, ##args)
  #elif defined(SLOG_HIGH)
    #define SERR(fmt, args...) fprintf(stderr, fmt, ##args)
    #define SLOW(fmt, args...) do{}while(0)
    #define SHIGH(fmt, args...) fprintf(stderr, fmt, ##args)
  #else
    #define SERR(fmt, args...) fprintf(stderr, fmt, ##args)
    #define SLOW(fmt, args...) do{}while(0)
    #define SHIGH(fmt, args...) do{}while(0)
  #endif
#endif

/* This macro will return if evaluated expression returned NULL.*/
#define RETURN_VOID_ON_NULL(expr)         \
    if ((expr) == NULL) {            \
        SERR("failed NULL pointer detected " #expr); \
        return;                \
     }
/*This macro will return FALSE to ret if evaluated expression returned NULL.*/
#define RETURN_ON_NULL(expr)         \
    if ((expr) == NULL) {            \
        SERR("failed NULL pointer detected " #expr); \
        return FALSE;                \
     }
/* This macro will return errval if evaluated expression returned NULL */
#define RETURN_ERR_ON_NULL(expr, ret, args...)                \
    if ((expr) == NULL) {                                     \
        SERR("failed NULL pointer detected: " #expr " : " args); \
        return ret;                                           \
    }
/* This macro will return SENSOR_FAILURE if evaluated expression returned NULL */
#define RETURN_ERROR_ON_NULL(expr)                         \
    if ((expr) == NULL) {                            \
        SERR("failed evaluated expression is NULL"); \
        return SENSOR_FAILURE;                                 \
     }
#endif /* __DEBUG_LIB_H__ */
