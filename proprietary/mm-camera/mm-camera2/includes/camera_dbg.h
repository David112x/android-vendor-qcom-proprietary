/**
 * Copyright (c) 2012-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/

/**
 * @file  camera_dgb.h
 * @brief macros for trace/debug logging
 **/

#ifndef __CAMERA_DBG_H__
#define __CAMERA_DBG_H__

#include "cam_types.h"

#ifndef MM_DEBUG
#define LOG_DEBUG
#endif

#if defined(LOG_DEBUG)

  typedef enum {
    CAM_NO_MODULE,
    CAM_MCT_MODULE,
    CAM_SENSOR_MODULE,
    CAM_IFACE_MODULE,
    CAM_ISP_MODULE,
    CAM_PPROC_MODULE,
    CAM_IMGLIB_MODULE,
    CAM_CPP_MODULE,
    CAM_HAL_MODULE,
    CAM_JPEG_MODULE,
    CAM_C2D_MODULE,
    CAM_STATS_MODULE,
    CAM_STATS_AF_MODULE,
    CAM_STATS_AEC_MODULE,
    CAM_STATS_AWB_MODULE,
    CAM_STATS_ASD_MODULE,
    CAM_STATS_AFD_MODULE,
    CAM_STATS_Q3A_MODULE,
    CAM_STATS_IS_MODULE,
    CAM_STATS_HAF_MODULE,
    CAM_STATS_CAF_SCAN_MODULE,
    CAM_SHIM_LAYER,
    CAM_LAST_MODULE
  } cam_modules_t;

  /* values that persist.vendor.camera.global.debug can be set to */
  /* all camera modules need to map their internal debug levels to this range */
  typedef enum {
    CAM_GLBL_DBG_NONE  = 0,
    CAM_GLBL_DBG_ERR   = 1,
    CAM_GLBL_DBG_WARN  = 2,
    CAM_GLBL_DBG_HIGH  = 3,
    CAM_GLBL_DBG_DEBUG = 4,
    CAM_GLBL_DBG_LOW   = 5,
    CAM_GLBL_DBG_INFO  = 6
  } cam_global_debug_level_t;

  /* current trace logging configuration */
  /* g_cam_log[cam_modules_t][cam_global_debug_level_t] */
  extern int g_cam_log[CAM_LAST_MODULE][CAM_GLBL_DBG_INFO + 1];

  /* logging macros */
  #undef CLOGx
  #define CLOGx(module, level, fmt, args...)                         \
    if (g_cam_log[module][level]) {                                  \
      cam_debug_log(module, level, __func__, __LINE__, fmt, ##args); \
    }

  #undef CLOGI
  #define CLOGI(module, fmt, args...)                \
      CLOGx(module, CAM_GLBL_DBG_INFO, fmt, ##args)
  #undef CLOGD
  #define CLOGD(module, fmt, args...)                \
      CLOGx(module, CAM_GLBL_DBG_DEBUG, fmt, ##args)
  #undef CLOGL
  #define CLOGL(module, fmt, args...)                \
      CLOGx(module, CAM_GLBL_DBG_LOW, fmt, ##args)
  #undef CLOGW
  #define CLOGW(module, fmt, args...)                \
      CLOGx(module, CAM_GLBL_DBG_WARN, fmt, ##args)
  #undef CLOGH
  #define CLOGH(module, fmt, args...)                \
      CLOGx(module, CAM_GLBL_DBG_HIGH, fmt, ##args)
  #undef CLOGE
  #define CLOGE(module, fmt, args...)                \
      CLOGx(module, CAM_GLBL_DBG_ERR, fmt, ##args)

  /* Assert macros */
  #undef CAM_DBG_ASSERT
  #define CAM_DBG_ASSERT(module, x, fmt, args...)                \
      cam_assert_log(module, x, __func__, __LINE__, fmt, ##args)
  #undef CAM_DBG_ASSERT_ALWAYS
  #define CAM_DBG_ASSERT_ALWAYS(module, fmt, args...)            \
      cam_assert_log(module, 0, __func__, __LINE__, fmt, ##args)

  /* Module level logging macros, deprecated */
  #undef MDBG
  #define MDBG(module, fmt, args...) CLOGD(module,  fmt, ##args)
  #undef MDBG_LOW
  #define MDBG_LOW(module, fmt, args...) CLOGL(module,  fmt, ##args)
  #undef MDBG_WARN
  #define MDBG_WARN(module, fmt, args...) CLOGW(module,  fmt, ##args)
  #undef MDBG_HIGH
  #define MDBG_HIGH(module, fmt, args...) CLOGH(module,  fmt, ##args)
  #undef MDBG_ERROR
  #define MDBG_ERROR(module, fmt, args...) CLOGE(module,  fmt, ##args)
  #undef MDBG_INFO
  #define MDBG_INFO(module, fmt, args...) CLOGI(module,  fmt, ##args)

  /* Global logging macros, deprecated */
  #undef CDBG
  #define CDBG(fmt, args...) CLOGD(CAM_NO_MODULE,  fmt, ##args)
  #undef CDBG_LOW
  #define CDBG_LOW(fmt, args...) CLOGL(CAM_NO_MODULE,  fmt, ##args)
  #undef CDBG_WARN
  #define CDBG_WARN(fmt, args...) CLOGW(CAM_NO_MODULE,  fmt, ##args)
  #undef CDBG_HIGH
  #define CDBG_HIGH(fmt, args...) CLOGH(CAM_NO_MODULE,  fmt, ##args)
  #undef CDBG_ERROR
  #define CDBG_ERROR(fmt, args...) CLOGE(CAM_NO_MODULE,  fmt, ##args)
  #undef LOGE
  #define LOGE(fmt, args...) CLOGE(CAM_NO_MODULE,  fmt, ##args)

#ifdef __cplusplus
extern "C" {
#endif

  /* generic logger function */
  void cam_debug_log(const cam_modules_t module,
                     const cam_global_debug_level_t level,
                     const char *func, const int line, const char *fmt, ...);
  /* assert logging */
  void cam_assert_log(const cam_modules_t module, const unsigned char cond,
                      const char *func, const int line, const char *fmt, ...);

  /* reads and updates camera logging properties */
  void cam_set_dbg_log_properties(void);

  /* reads debug configuration file */
  void cam_debug_open(void);

    /* flush logging file at camera close */
  void cam_debug_flush(void);

  /* debug logging cleanup */
  void cam_debug_close(void);

#ifdef __cplusplus
}
#endif

#else /* LOG_DEBUG */

  /* logging macros */
  #undef CLOGI
  #define CLOGI(fmt, args...)
  #undef CLOGD
  #define CLOGD(fmt, args...)
  #undef CLOGL
  #define CLOGL(fmt, args...)
  #undef CLOGW
  #define CLOGW(fmt, args...)
  #undef CLOGH
  #define CLOGH(fmt, args...)
  #undef CLOGE
  #define CLOGE(fmt, args...)

  /* Assert macros */
  #undef CAM_DBG_ASSERT
  #define CAM_DBG_ASSERT(module, x, fmt, args...)
  #undef CAM_DBG_ASSERT_ALWAYS
  #define CAM_DBG_ASSERT_ALWAYS(module, fmt, args...)

  /* Module level logging macros, deprecated */
  #undef MDBG_LOW
  #define MDBG_LOW(module, fmt, args...)
  #undef MDBG_WARN
  #define MDBG_WARN(module, fmt, args...)
  #undef MDBG
  #define MDBG(module, fmt, args...)
  #undef MDBG_HIGH
  #define MDBG_HIGH(module, fmt, args...)
  #undef MDBG_ERROR
  #define MDBG_ERROR(module, fmt, args...)
  #undef MDBG_INFO
  #define MDBG_INFO(module, fmt, args...)

  /* Global logging macros, deprecated */
  #undef CDBG_LOW
  #define CDBG_LOW(fmt, args...)
  #undef CDBG_WARN
  #define CDBG_WARN(fmt, args...)
  #undef CDBG
  #define CDBG(fmt, args...)
  #undef CDBG_HIGH
  #define CDBG_HIGH(fmt, args...)
  #undef CDBG_ERROR
  #define CDBG_ERROR(fmt, args...)
#endif  /* LOG_DEBUG */

/* cam_setting_get:
 *
 *  @key:           name of the setting to get
 *  @value:         pointer to returned setting value
 *  @default_value: default setting value, if any
 *
 *  Gets the current value for a setting
 *
 *  Return: the length of the setting which will never be greater than a system
 *          dependent max value - 1 and will always be zero terminated.
 *          (the length does not include the terminating zero)
 *
 *          If the setting read fails or returns an empty value, the default
 *          setting value is used (if non null).
 */
int cam_setting_get(const char *key, char *value, const char *default_value);

/* cam_setting_set:
 *
 *  @key:    name of the setting to set
 *  @value:  new setting value
 *
 *  Sets a new value for a setting
 *
 *  Return: 0 on success, < 0 on failure
 */
int cam_setting_set(const char *key, const char *value);

#define CAM_UNUSED_PARAM(x) (void)x

#ifndef ABS
#define ABS(x)            (((x) < 0) ? -(x) : (x))
#endif

#define PRINT_2D_MATRIX(X,Y,M) ({             \
  int i, j;                                   \
  for(i=0; i<X; i++) {                        \
    CLOGH(CAM_NO_MODULE, "\n%s: ", __func__); \
    for(j=0; j<Y; j++) {                      \
      CLOGH(CAM_NO_MODULE, "%lf ", M[i][j]);  \
    }                                         \
  }                                           \
})

#define PRINT_1D_MATRIX(X,Y,M) ({               \
  for(i=0; i<X; i++) {                          \
    CLOGH(CAM_NO_MODULE, "\n%s: ", __func__);   \
    for(j=0; j<Y; j++) {                        \
      CLOGH(CAM_NO_MODULE, "%lf ", M[j+(i*X)]); \
    }                                           \
  }                                             \
})

#undef STATS_TEST_MASK
#undef STATS_DBG_MASK
#undef STATS_GLOBAL_DBG_MASK
#undef STATS_GYRO_USE_ANDROID_API
#ifdef _ANDROID_
#include <utils/Log.h>
#include <cutils/properties.h>

#define STATS_TEST_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.test", prop, "0"); \
                                  mask = atoi (prop);};

/* Test 3A extension framework for two outputs algo, for dual camera use case*/

/* Main Switch, request HAL to set 3A sync mode as CAM_3A_SYNC_ALGO_CTRL.
 * Send 2 output structures to the 3As algorithms */
#define STATS_TEST_DUAL_OUTPUT(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.test.2outs", prop, "0"); \
                                  mask = atoi (prop);};

/* Decides if independent algorithm instances will be use or ports share algo
 * 1: One algo instance is use for AEC & AWB, the algo provides same output for
 *    both cameras, output from camera index 0 is copy to camera index 1.
 * 0: Each port uses an independent algo instance */
#define STATS_TEST_SHARE_AECAWB_ALGO(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.test.1aec", prop, "0"); \
                                  mask = atoi (prop);};

/* Allows user to set a specific line-count for camera listed as index 1 by the
 * thread (secondary).
 * Values: 0 or min linecount to max linecount
 * lc  :: linecount to be apply on camera with index 1, this overwrite any
 *        value previously copy from camera 0.
 * 0  :: Do not overwrite lc value while sending output to camera index 1. */
#define STATS_TEST_AEC_SECONDARY_LC(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.test.lc_sc", prop, "0"); \
                                  mask = atoi (prop);};

/* Decides if independent algorithm instances will be use or ports share algo
 * 1: One algo instance is use for AF, the algo provides same output for
 *    both cameras, output from camera index 0 is copy to camera index 1.
 * 0: Each port uses an independent algo instance */
#define STATS_TEST_SHARE_AF_ALGO(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.test.1af", prop, "0"); \
                                  mask = atoi (prop);};

/**********     End of properties to test dual camera extension     **********/

#define STATS_DBG_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.debug", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_DBG_AF_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.af.debug", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_DBG_AEC_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.aec.debug", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_DBG_AWB_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.awb.debug", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_DBG_ASD_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.asd.debug", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_DBG_AFD_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.afd.debug", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_DBG_Q3A_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.q3a.debug", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_DBG_IS_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.is.debug", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_DBG_HAF_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.haf.debug", prop, "0"); \
                                  mask = atoi (prop);};
#define STATS_HAF_API_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.hafapi", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_HAF_INPUT_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.hafinput", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_HAF_NODE_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.hafnode", prop, "0"); \
                                  memcpy(mask,prop,strlen(mask)); \
                                  *(mask + strlen(prop)) = '\0';};

#define STATS_DBG_CAF_SCAN_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.cafscan", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_EXIF_DBG_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.debugexif", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_GLOBAL_DBG_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.global.debug", prop, "0"); \
                                  mask = atoi (prop);};

/*
  SENSOR_ANDROID_FMWK_DISABLE = 0,
  SENSOR_ANDROID_FMWK_ENABLE_ALL = (1 << 0),
  SENSOR_ANDROID_FMWK_LASER = (1 << 1),
  SENSOR_ANDROID_FMWK_GYRO = (1 << 2),
  SENSOR_ANDROID_FMWK_ACCELEROMETER = (1 << 3),
  SENSOR_ANDROID_FMWK_GRAVITY_VECTOR = (1 << 4),
  SENSOR_ANDROID_FMWK_MAGNETOMETER = (1 << 5),
  SENSOR_ANDROID_FMWK_LINEAR_ACCEL = (1 << 6),
  SENSOR_ANDROID_FMWK_LIGHT = (1 << 7),
*/
#define STATS_GYRO_USE_ANDROID_API(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.gyro.android", prop, "0");\
                                  mask = atoi(prop);};

#define STATS_DISABLE_HAF_ALGO_MASK(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.disablehaf", prop, "0"); \
                                  mask = atoi (prop);};

#define STATS_ENABLE_TUNING_DEBUG(value) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.stats.TuningDbg", prop, "0"); \
                                  value = atoi (prop);};


#define STATS_KPI_MASK(mask) {\
                      char prop[PROPERTY_VALUE_MAX];\
                      property_get("persist.vendor.camera.kpi.debug", prop, "0"); \
                      mask = atoi(prop);};

#define STATS_DISABLE_GYRO_MODULE(mask) {\
                      char prop[PROPERTY_VALUE_MAX];\
                      property_get("persist.vendor.camera.gyro.disable", prop, "0"); \
                      mask = atoi(prop);};

#define STATS_DEBUG_DATA_LEVEL_MASK(mask) {\
                      char prop[PROPERTY_VALUE_MAX];\
                      property_get("persist.vendor.camera.stats.exiflevel", prop, "0"); \
                      mask = atoi(prop);};

#define STATS_TOF_INTERFACE_CHOICE(mask) {\
                      char prop[PROPERTY_VALUE_MAX];\
                      property_get("persist.vendor.camera.tof.direct", prop, "0"); \
                      mask = (boolean)atoi(prop);};
/* Enable/disable FOVC feature:
   0 - disable
   1 - enable CPP mode
   2 - enable ISP mode */
#define STATS_ENABLE_FOVC(mask) {\
                      char prop[PROPERTY_VALUE_MAX];\
                      property_get("persist.vendor.camera.af.fovc.enable", prop, "0"); \
                      mask = (boolean)atoi(prop);};
#define STATS_GYRO_THREAD_ENQUEUE_EVERY_FRAME(mask) {\
                                  char prop[PROPERTY_VALUE_MAX];\
                                  property_get("persist.vendor.camera.gyro.perframe", prop, "0");\
                                  mask = atoi(prop);};

#define STATS_ENABLE_MULTIWINDOW(mask) {\
                      char prop[PROPERTY_VALUE_MAX];\
                      property_get("persist.vendor.camera.mw.enable", prop, "0"); \
                      mask = (boolean)atoi(prop);};

/* Enable/disable Hand Jitter Resistant AF feature:
   0 - disable
   1 - enable  */
#define STATS_ENABLE_AF_HJ(mask) {\
                      char prop[PROPERTY_VALUE_MAX];\
                      property_get("persist.vendor.camera.af_hj.enable", prop, "0"); \
                      mask = (boolean)atoi(prop);};

#else
#define STATS_TEST_MASK(mask) {mask = 0;}
#define STATS_TEST_DUAL_OUTPUT(mask) {mask = 0;}
#define STATS_TEST_SHARE_AECAWB_ALGO(mask) {mask = 0;}
#define STATS_TEST_AEC_SECONDARY_LC(mask) {mask = 0;}
#define STATS_TEST_SHARE_AF_ALGO(mask) {mask = 0;}
#define STATS_DBG_MASK(mask) {mask = 0;}
#define STATS_EXIF_DBG_MASK(mask) {mask = 0;}
#define STATS_GLOBAL_DBG_MASK(mask) {mask = 0;}
#define STATS_DISABLE_HAF_ALGO_MASK(mask) {mask = 0;}
#define STATS_KPI_MASK(mask) {mask = 0;}
#define STATS_DEBUG_DATA_LEVEL_MASK(mask) {mask = 0;}

#define STATS_GYRO_USE_ANDROID_API(use_android) {use_android = 0;}
#define STATS_DISABLE_GYRO_MODULE(mask) {mask = 0;}
#define STATS_TOF_INTERFACE_CHOICE(mask) {mask = 0;}
#define STATS_ENABLE_FOVC(mask) {mask = 0;}
#define STATS_ENABLE_AF_HJ(mask) {mask = 0;}
#define STATS_GYRO_THREAD_ENQUEUE_EVERY_FRAME(mask) {mask = 0;}

#endif

#ifndef KPI_DEBUG
#define KPI_DEBUG
#ifdef _ANDROID_
#include <cutils/trace.h>
#include "camscope_packet_type.h"

extern volatile uint32_t kpi_debug_level;
extern volatile uint32_t malloc_track_size;

#undef ATRACE_BEGIN
#undef ATRACE_INT
#undef ATRACE_END

#define KPI_ONLY 1
#define KPI_DBG 2
#define MCT_KPI_DBG 3

#define KPI_TRACE_DBG ((kpi_debug_level >= KPI_DBG) ? 1 : 0)
#define MCT_TRACE_DBG ((kpi_debug_level >= MCT_KPI_DBG) ? 1 : 0)

#define ATRACE_BEGIN_SNPRINTF(buf_size, fmt_str, ...) \
 if (kpi_debug_level >= KPI_DBG) { \
   char trace_tag[buf_size]; \
   snprintf(trace_tag, buf_size, fmt_str, ##__VA_ARGS__); \
   ATRACE_BEGIN(trace_tag); \
}

#define ATRACE_INT_SNPRINTF(value, buf_size, fmt_str, ...) \
 if (kpi_debug_level >= KPI_DBG) { \
   char trace_tag[buf_size]; \
   snprintf(trace_tag, buf_size, fmt_str, ##__VA_ARGS__); \
   ATRACE_INT(trace_tag,value); \
}

//to enable only KPI logs
#define KPI_ATRACE_BEGIN(name) ({\
if (kpi_debug_level >= KPI_ONLY) { \
     atrace_begin(ATRACE_TAG_ALWAYS, name); \
}\
})

#define KPI_ATRACE_END() ({\
if (kpi_debug_level >= KPI_ONLY) { \
     atrace_end(ATRACE_TAG_ALWAYS); \
}\
})

#define KPI_ATRACE_INT(name,val) ({\
if (kpi_debug_level >= KPI_ONLY) { \
     atrace_int(ATRACE_TAG_ALWAYS, name, val); \
}\
})

#define ATRACE_BEGIN_DBG(name) ({\
if (kpi_debug_level >= KPI_DBG) { \
     atrace_begin(ATRACE_TAG_ALWAYS, name); \
}\
})

#define ATRACE_END_DBG() ({\
if (kpi_debug_level >= KPI_DBG) { \
     atrace_end(ATRACE_TAG_ALWAYS); \
}\
})

#define ATRACE_INT_DBG(name,val) ({\
if (kpi_debug_level >= KPI_DBG) { \
     atrace_int(ATRACE_TAG_ALWAYS, name, val); \
}\
})

#define Q3A_TRACE_INT(kpi_dbg_level, name, val) ({\
if (kpi_dbg_level >= KPI_ONLY) { \
     atrace_int(ATRACE_TAG_ALWAYS, name, val); \
}\
})

#define ATRACE_BEGIN ATRACE_BEGIN_DBG
#define ATRACE_INT ATRACE_INT_DBG
#define ATRACE_END ATRACE_END_DBG

#define CAMSCOPE_MAX_STRING_LENGTH 64

/* Initializes CameraScope tool */
void camscope_init(camscope_section_type camscope_section);

/* Cleans up CameraScope tool */
void camscope_destroy(camscope_section_type camscope_section);

/* Reserves a number of bytes on the memstore flushing to the
 * file system if remaining space is insufficient */
uint32_t camscope_reserve(camscope_section_type camscope_section,
                          uint32_t num_bytes_to_reserve);

/* Store the data to the memstore and calculate remaining space */
void camscope_store_data(camscope_section_type camscope_section,
                         void* data, uint32_t size);

/* Lock the camscope mutex lock for the given camscope section */
void camscope_mutex_lock(camscope_section_type camscope_section);

/* Unlock the camscope mutex lock for the given camscope section */
void camscope_mutex_unlock(camscope_section_type camscope_section);

#define CAMSCOPE_SYSTRACE_TIME_MARKER() { \
    if (kpi_camscope_frame_count != 0) { \
        if (kpi_camscope_flags & CAMSCOPE_ON_FLAG) { \
            struct timeval t_domain; \
            char trace_time_conv[CAMSCOPE_MAX_STRING_LENGTH]; \
            gettimeofday(&t_domain, NULL); \
            snprintf(trace_time_conv, sizeof(trace_time_conv), \
                     "_CAMSCOPE_TIME_CONV_:%ld:%ld", t_domain.tv_sec, \
                     t_domain.tv_usec); \
            atrace_int(ATRACE_TAG_ALWAYS, trace_time_conv, 0); \
        } \
    } \
}

#define CAMSCOPE_MASK(mask) { \
    char prop[PROPERTY_VALUE_MAX]; \
    property_get("persist.vendor.camera.kpi.camscope", prop, "0"); \
    mask = atoi(prop); \
}

#define CAMSCOPE_FRAME_COUNT_MASK(mask) { \
    char prop[PROPERTY_VALUE_MAX]; \
    property_get("persist.vendor.camera.kpi.camscope_cnt", prop, "0"); \
    mask = atoi(prop); \
}

#define CAMSCOPE_UPDATE_FLAGS(camscope_section, camscope_prop) { \
    if (kpi_camscope_frame_count != 0) { \
        static uint32_t camscope_frame_counter = 0; \
        if (camscope_frame_counter >= kpi_camscope_frame_count) { \
            uint32_t prev_prop = camscope_prop; \
            CAMSCOPE_MASK(camscope_prop); \
            uint32_t is_prev_prop_on = (prev_prop & CAMSCOPE_ON_FLAG) \
                                        ? 1 : 0; \
            uint32_t is_prop_on = (camscope_prop & CAMSCOPE_ON_FLAG) \
                                   ? 1 : 0; \
            if (is_prev_prop_on ^ is_prop_on) { \
                if (is_prop_on) { \
                    camscope_init(camscope_section); \
                } else { \
                    camscope_destroy(camscope_section); \
                } \
            } \
            CAMSCOPE_SYSTRACE_TIME_MARKER(); \
            camscope_frame_counter = 0; \
        } \
        else { \
            ++camscope_frame_counter; \
        } \
    } \
}

#define CAMSCOPE_INIT(camscope_section) { \
    CAMSCOPE_FRAME_COUNT_MASK(kpi_camscope_frame_count); \
    if (kpi_camscope_frame_count != 0) { \
        CAMSCOPE_MASK(kpi_camscope_flags); \
        if (kpi_camscope_flags & CAMSCOPE_ON_FLAG) { \
            camscope_init(camscope_section); \
            CAMSCOPE_SYSTRACE_TIME_MARKER(); \
        } \
    } \
}

#define CAMSCOPE_DESTROY(camscope_section) { \
    if (kpi_camscope_frame_count != 0) { \
        if (kpi_camscope_flags & CAMSCOPE_ON_FLAG) { \
            camscope_destroy(camscope_section); \
        } \
    } \
}

#define ATRACE_CAMSCOPE_BEGIN(camscope_name) ({\
if (camscope_name < CAMSCOPE_EVENT_NAME_SIZE && \
    camscope_name >= 0) { \
    ATRACE_BEGIN_DBG(camscope_atrace_names[camscope_name]); \
} \
camscope_sw_base_log(CAMSCOPE_SECTION_MMCAMERA, \
                     CAMSCOPE_KPI_DBG_MASK, \
                     CAMSCOPE_SYNC_BEGIN, \
                     camscope_name); \
})

#define ATRACE_CAMSCOPE_END(camscope_name) ({\
ATRACE_END_DBG(); \
camscope_sw_base_log(CAMSCOPE_SECTION_MMCAMERA, \
                     CAMSCOPE_KPI_DBG_MASK, \
                     CAMSCOPE_SYNC_END, \
                     camscope_name); \
})

#define MALLOC_TRACK_TAG "MALLOC:"
#define CAM_MALLOC(size) \
malloc(size); \
do { \
    if ((malloc_track_size > 0) && ((uint32_t)size >= malloc_track_size)) \
    { \
        __android_log_print(ANDROID_LOG_ERROR, MALLOC_TRACK_TAG, \
               ":%s,%s,%d,%u\n", \
               __FILE__, __func__, __LINE__, (unsigned int)size); \
    } \
} while(0)

#define CALLOC_TRACK_TAG "CALLOC:"
#define CAM_CALLOC(arg, size) \
calloc(arg, size); \
do { \
    if ((malloc_track_size > 0) && ((uint32_t)size >= malloc_track_size)) \
    { \
        __android_log_print(ANDROID_LOG_ERROR, CALLOC_TRACK_TAG, \
               ":%s,%s,%d,%d,%u\n", \
               __FILE__, __func__, __LINE__, arg, (unsigned int)size); \
    } \
} while(0)


#else
#define PROPERTY_GET(property_name, init_value) do {} while (0)
#define ATRACE_BEGIN_SNPRINTF(buf_size, fmt_str, ...) do {} while (0)
#define ATRACE_INT_SNPRINTF(value, buf_size, fmt_str, ...) do {} while (0)
#define KPI_ATRACE_BEGIN(name) do {} while (0)
#define KPI_ATRACE_END() do {} while (0)
#define KPI_ATRACE_INT(name,val) do {} while (0)
#define ATRACE_BEGIN(name) do {} while (0)
#define ATRACE_END() do {} while (0)
#define ATRACE_INT(name, value) do {} while (0)
#define Q3A_TRACE_INT(kpi_dbg_level, name, val) do {} while (0)
#define CAMSCOPE_MASK(mask) do {} while (0)
#define CAMSCOPE_FRAME_COUNT_MASK(mask) do {} while (0)
#define CAMSCOPE_UPDATE_FLAGS(camscope_section, camscope_prop) do {} while (0)
#define CAMSCOPE_INIT(camscope_section) do {} while (0)
#define CAMSCOPE_DESTROY(camscope_section) do {} while (0)
#define CAMSCOPE_SYSTRACE_TIME_MARKER() do {} while (0)
#define ATRACE_CAMSCOPE_BEGIN(name, camscope_name) do {} while (0)
#define ATRACE_CAMSCOPE_END(camscope_name) do {} while (0)
#define CAM_MALLOC(size) malloc(size)
#define CAM_CALLOC(arg, size) calloc(arg, size)

#endif /* _ANDROID_*/

#endif /*KPI_DEBUG*/
#endif /* __CAMERA_DBG_H__ */
