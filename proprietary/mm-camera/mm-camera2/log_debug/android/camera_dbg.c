/**
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/

/**
 * @file  camera_dgb.c
 * @brief functions for trace/debug logging
 **/

#include "camera_dbg.h"

/* This log debug file is for Android */
#if defined(LOG_DEBUG) && defined (_ANDROID_)

  #include <stdarg.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <sys/stat.h>
  #include <pthread.h>
  #include <cutils/log.h>
  #include <cutils/properties.h>

  #undef LOG_TAG
  #define LOG_TAG "mm-camera"
  #define CDBG_MAX_STR_LEN 1024
  #define CDBG_MAX_LINE_LENGTH 256

  volatile uint32_t kpi_debug_level = 0;
  volatile uint32_t malloc_track_size = 0;

  /* current trace loggin permissions
   * {NONE, ERR, WARN, HIGH, DEBUG, LOW, INFO} */
  int g_cam_log[CAM_LAST_MODULE][CAM_GLBL_DBG_INFO + 1] = {
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_NO_MODULE     */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_MCT_MODULE    */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_SENSOR_MODULE */
      {0, 1, 1, 0, 0, 0, 1}, /* CAM_IFACE_MODULE  */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_ISP_MODULE    */
      {0, 1, 1, 0, 0, 0, 1}, /* CAM_PPROC_MODULE  */
      {0, 1, 1, 0, 0, 0, 1}, /* CAM_IMGLIB_MODULE */
      {0, 1, 1, 0, 0, 0, 1}, /* CAM_CPP_MODULE    */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_HAL_MODULE    */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_JPEG_MODULE   */
      {0, 1, 1, 0, 0, 0, 1}, /* CAM_C2D_MODULE    */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_STATS_MODULE  */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_STATS_AF_MODULE   */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_STATS_AEC_MODULE  */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_STATS_AWB_MODULE  */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_STATS_ASD_MODULE  */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_STATS_AFD_MODULE  */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_STATS_Q3A_MODULE  */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_STATS_IS_MODULE   */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_STATS_HAF_MODULE  */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_STATS_CAFSCAN_MODULE  */
      {0, 1, 0, 0, 0, 0, 1}, /* CAM_SHIM_LAYER */
    };

  /* string representation for logging level */
  static const char *cam_dbg_level_to_str[] = {
       "",        /* CAM_GLBL_DBG_NONE  */
       "<ERROR>", /* CAM_GLBL_DBG_ERR   */
       "< WARN>", /* CAM_GLBL_DBG_WARN  */
       "< HIGH>", /* CAM_GLBL_DBG_HIGH  */
       "<  DBG>", /* CAM_GLBL_DBG_DEBUG */
       "<  LOW>", /* CAM_GLBL_DBG_LOW   */
       "< INFO>"  /* CAM_GLBL_DBG_INFO  */
    };

  /* current trace logging configuration */
  typedef struct {
     cam_global_debug_level_t  level;
     int                       initialized;
     const char               *name;
     const char               *prop;
  } module_debug_t;

  static module_debug_t cam_loginfo[(int)CAM_LAST_MODULE] = {
    {CAM_GLBL_DBG_ERR, 1,
        "",         "persist.vendor.camera.global.debug"     }, /* CAM_NO_MODULE     */
    {CAM_GLBL_DBG_ERR, 1,
        "<MCT   >", "persist.vendor.camera.mct.debug"        }, /* CAM_MCT_MODULE    */
    {CAM_GLBL_DBG_ERR, 1,
        "<SENSOR>", "persist.vendor.camera.sensor.debug"     }, /* CAM_SENSOR_MODULE */
    {CAM_GLBL_DBG_WARN, 1,
        "<IFACE >", "persist.vendor.camera.iface.logs"       }, /* CAM_IFACE_MODULE  */
    {CAM_GLBL_DBG_ERR, 1,
        "<ISP   >", "persist.vendor.camera.isp.debug"        }, /* CAM_ISP_MODULE    */
    {CAM_GLBL_DBG_ERR, 1,
        "<PPROC >", "persist.vendor.camera.pproc.debug.mask" }, /* CAM_PPROC_MODULE  */
    {CAM_GLBL_DBG_WARN, 1,
        "<IMGLIB>", "persist.vendor.camera.imglib.logs"      }, /* CAM_IMGLIB_MODULE */
    {CAM_GLBL_DBG_WARN, 1,
        "<CPP   >", "persist.vendor.camera.cpp.debug.mask"   }, /* CAM_CPP_MODULE    */
    {CAM_GLBL_DBG_ERR, 1,
        "<HAL   >", "persist.vendor.camera.hal.debug"        }, /* CAM_HAL_MODULE    */
    {CAM_GLBL_DBG_ERR, 1,
        "<JPEG  >", "persist.vendor.camera.mmstill.logs"     }, /* CAM_JPEG_MODULE   */
    {CAM_GLBL_DBG_WARN, 1,
        "<C2D   >", "persist.vendor.camera.c2d.debug.mask"   }, /* CAM_C2D_MODULE    */
    {CAM_GLBL_DBG_ERR, 1,
        "<STATS >", "persist.vendor.camera.stats.debug" }, /* CAM_STATS_MODULE  */
    {CAM_GLBL_DBG_ERR, 1,
        "<STATS_AF >", "persist.vendor.camera.stats.af.debug"    }, /* CAM_STATS_AF_MODULE   */
    {CAM_GLBL_DBG_ERR, 1,
        "<STATS_AEC >", "persist.vendor.camera.stats.aec.debug"  }, /* CAM_STATS_AEC_MODULE  */
    {CAM_GLBL_DBG_ERR, 1,
        "<STATS_AWB >", "persist.vendor.camera.stats.awb.debug"  }, /* CAM_STATS_AWB_MODULE  */
    {CAM_GLBL_DBG_ERR, 1,
        "<STATS_ASD >", "persist.vendor.camera.stats.asd.debug"  }, /* CAM_STATS_ASD_MODULE  */
    {CAM_GLBL_DBG_ERR, 1,
        "<STATS_AFD >", "persist.vendor.camera.stats.afd.debug"  }, /* CAM_STATS_AFD_MODULE  */
    {CAM_GLBL_DBG_ERR, 1,
        "<STATS_Q3A >", "persist.vendor.camera.stats.q3a.debug"  }, /* CAM_STATS_Q3A_MODULE  */
    {CAM_GLBL_DBG_ERR, 1,
        "<STATS_AIS >", "persist.vendor.camera.stats.is.debug"   }, /* CAM_STATS_IS_MODULE   */
    {CAM_GLBL_DBG_ERR, 1,
        "<STATS_HAF >", "persist.vendor.camera.stats.haf.debug"  }, /* CAM_STATS_HAF_MODULE  */
    {CAM_GLBL_DBG_ERR, 1,
        "<STATS_CAFSCAN >", "persist.vendor.camera.stats.cafscan"  }, /* CAM_STATS_CAFSCAN_MODULE  */
    {CAM_GLBL_DBG_ERR, 1,
        "<SHIM  >", "persist.vendor.camera.shim.debug"          }, /* CAM_SHIM_LAYER    */

  };

  pthread_mutex_t dbg_log_mutex;

  static int         cam_soft_assert     = 0;
  static FILE       *cam_log_fd          = NULL;
  static char       *g_log_cache_buffer  = NULL;
  static int         g_log_cache_buffer_size = 0;
  static int         g_log_cache_buffer_offset = 0;
  static const char *cam_config_filename = QCAMERA_DUMP_FRM_LOCATION"camera_dbg.txt";
  static const char *cam_log_filename    = QCAMERA_DUMP_FRM_LOCATION"cam_dbg_log.txt";

  /** cam_get_dbg_level
   *
   *    @module: module name
   *    @level:  module debug logging level
   *
   *  Maps debug log string to value.
   *
   *  Return: logging level
   **/
  static cam_global_debug_level_t cam_get_dbg_level(const char *module,
    char *pValue) {

    cam_global_debug_level_t rc = CAM_GLBL_DBG_NONE;

    if (!strcmp(pValue, "none")) {
      rc = CAM_GLBL_DBG_NONE;
    } else if (!strcmp(pValue, "warn")) {
      rc = CAM_GLBL_DBG_WARN;
    } else if (!strcmp(pValue, "debug")) {
      rc = CAM_GLBL_DBG_DEBUG;
    } else if (!strcmp(pValue, "error")) {
      rc = CAM_GLBL_DBG_ERR;
    } else if (!strcmp(pValue, "low")) {
      rc = CAM_GLBL_DBG_LOW;
    } else if (!strcmp(pValue, "high")) {
      rc = CAM_GLBL_DBG_HIGH;
    } else if (!strcmp(pValue, "info")) {
      rc = CAM_GLBL_DBG_INFO;
    } else {
      CDBG_ERROR("Invalid %s debug log level %s\n", module, pValue);
    }

    CDBG("%s debug log level: %s\n", module, cam_dbg_level_to_str[rc]);

    return rc;
  }

  /** cam_vsnprintf
   *    @pdst:   destination buffer pointer
   *    @size:   size of destination buffer
   *    @pfmt:   string format
   *    @argptr: variabkle length argument list
   *
   *  Processes variable length argument list to a formatted string.
   *
   *  Return: n/a
   **/
  static void cam_vsnprintf(char* pdst, unsigned int size,
                            const char* pfmt, va_list argptr) {
    int num_chars_written = 0;

    pdst[0] = '\0';
    num_chars_written = vsnprintf(pdst, size, pfmt, argptr);

    if ((num_chars_written >= (int)size) && (size > 0)) {
       /* Message length exceeds the buffer limit size */
       num_chars_written = size - 1;
       pdst[size - 1] = '\0';
    }
  }

  /** cam_debug_log
   *    @module: origin or log message
   *    @level:  logging level
   *    @func:   caller function name
   *    @line:   caller line number
   *    @fmt:    log message formatting string
   *    @...:    variable argument list
   *
   *  Generig logger method.
   *
   *  Return: N/A
   **/
  void cam_debug_log(const cam_modules_t module,
                     const cam_global_debug_level_t level,
                     const char *func, const int line, const char *fmt, ...) {
    char    str_buffer[CDBG_MAX_STR_LEN];
    va_list args;

    va_start(args, fmt);
    cam_vsnprintf(str_buffer, CDBG_MAX_STR_LEN, fmt, args);
    va_end(args);

    switch (level) {
    case CAM_GLBL_DBG_WARN:
      ALOGW("%s%s %d: %s: %s", cam_loginfo[module].name,
        cam_dbg_level_to_str[level], line, func, str_buffer);
      break;
    case CAM_GLBL_DBG_ERR:
      ALOGE("%s%s %d: %s: %s", cam_loginfo[module].name,
        cam_dbg_level_to_str[level], line, func, str_buffer);
      break;
    case CAM_GLBL_DBG_INFO:
      ALOGI("%s%s %d: %s: %s", cam_loginfo[module].name,
        cam_dbg_level_to_str[level], line, func, str_buffer);
      break;
    case CAM_GLBL_DBG_HIGH:
    case CAM_GLBL_DBG_DEBUG:
    case CAM_GLBL_DBG_LOW:
    default:
      ALOGD("%s%s %d: %s: %s", cam_loginfo[module].name,
        cam_dbg_level_to_str[level], line, func, str_buffer);
    }

    if (cam_log_fd != NULL) {
      char new_str_buffer[CDBG_MAX_STR_LEN] = {0};
      struct timeval tv;
      struct timezone tz;
      struct tm *now;

      pthread_mutex_lock(&dbg_log_mutex);
      gettimeofday(&tv, &tz);
      now = gmtime((time_t *)&tv.tv_sec);
      if (now != NULL)
      {
          snprintf(new_str_buffer, CDBG_MAX_STR_LEN, "%2d %02d:%02d:%02d.%03ld %d:%d Camera%s%s %d: %s: %s",
               now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, tv.tv_usec,getpid(),gettid(),
               cam_dbg_level_to_str[level], cam_loginfo[module].name,
               line, func, str_buffer);
      }
      int len = strlen(new_str_buffer);

      /* flush the cached buffer to file if not enough space */
      if ((g_log_cache_buffer_offset + len) > g_log_cache_buffer_size) {
        if (g_log_cache_buffer_offset > 0) {
          g_log_cache_buffer[g_log_cache_buffer_offset] = '\0';
          fprintf(cam_log_fd, "%s", g_log_cache_buffer);
          g_log_cache_buffer_offset = 0;
        }
      }
      /* still need to check space just in case it is a big single string */
      if ((g_log_cache_buffer_offset + len) <= g_log_cache_buffer_size) {
        if (g_log_cache_buffer != NULL) {
          memcpy(g_log_cache_buffer + g_log_cache_buffer_offset,
                 new_str_buffer, len);
          g_log_cache_buffer[g_log_cache_buffer_offset + len] = '\n';
          g_log_cache_buffer_offset += len + 1;
        }
      }
      /* all right,just write this big string then */
      else {
        fprintf(cam_log_fd, "%s", new_str_buffer);
      }
      pthread_mutex_unlock(&dbg_log_mutex);
    }
  }

  /** cam_assert_log
   *    @module: origin or log message
   *    @cond:   will assert if cond == 0
   *    @func:   caller function name
   *    @line:   caller line number
   *    @fmt:    log message formatting string
   *    @...:    variable argument list
   *
   *  Assert logger method.
   *
   *  Return: N/A
   **/
  void cam_assert_log(const cam_modules_t module, const unsigned char cond,
                      const char *func, const int line, const char *fmt, ...) {
    char    str_buffer[CDBG_MAX_STR_LEN];
    va_list args;

    if (cond == 0) {
      va_start(args, fmt);
      cam_vsnprintf(str_buffer, CDBG_MAX_STR_LEN, fmt, args);
      va_end(args);

      /* Always write assert message at least to stderr */
      ALOGE("Camera[ASSERT]%s %s: %d: %s", cam_loginfo[module].name,
          func, line, str_buffer);

      if (cam_log_fd != NULL) {
        pthread_mutex_lock(&dbg_log_mutex);
        fprintf(cam_log_fd, "Camera[ASSERT]%s %s: %d: %s",
            cam_loginfo[module].name, func, line, str_buffer);
        pthread_mutex_unlock(&dbg_log_mutex);
      }

      if (cam_soft_assert) {
        raise(SIGTRAP);
      }
    }
  }

  /** cam_set_dbg_log_properties
   *
   *  Set global and module log level properties.
   *
   *  Return: N/A
   **/
  void cam_set_dbg_log_properties(void) {
    int          i;
    unsigned int j;
    char         property_value[PROPERTY_VALUE_MAX] = {0};
    char         default_value[PROPERTY_VALUE_MAX]  = {0};

    /* set global and individual module logging levels */
    pthread_mutex_lock(&dbg_log_mutex);
    for (i = CAM_NO_MODULE; i < CAM_LAST_MODULE; i++) {
      cam_global_debug_level_t log_level;
      snprintf(default_value, PROPERTY_VALUE_MAX, "%d", (int)cam_loginfo[i].level);
      cam_setting_get(cam_loginfo[i].prop, property_value, default_value);
      log_level = (cam_global_debug_level_t)atoi(property_value);

      /* fix KW warnings */
      if (log_level > CAM_GLBL_DBG_INFO) {
         log_level = CAM_GLBL_DBG_INFO;
      }

      cam_loginfo[i].level = log_level;

      /* The logging macros will produce a log message when logging level for
       * a module is less or equal to the level specified in the property for
       * the module, or less or equal the level specified by the global logging
       * property. Currently we don't allow INFO logging to be turned off */
      for (j = CAM_GLBL_DBG_ERR; j <= CAM_GLBL_DBG_LOW; j++) {
        g_cam_log[i][j] = (cam_loginfo[CAM_NO_MODULE].level != CAM_GLBL_DBG_NONE)     &&
                          (cam_loginfo[i].level             != CAM_GLBL_DBG_NONE)     &&
                          ((j                                <= cam_loginfo[i].level) ||
                           (j                                <= cam_loginfo[CAM_NO_MODULE].level));
      }
    }
    cam_setting_get("persist.vendor.camera.kpi.debug", property_value, "0");
    kpi_debug_level = atoi(property_value);

    cam_setting_get("persist.vendor.camera.malloctrk.size", property_value, "0");
    malloc_track_size = atoi(property_value);

    pthread_mutex_unlock(&dbg_log_mutex);
  }

   /** cam_debug_open
   *
   *  Reads debug configuration file from /data/vendor/camera/camera_dbg.txt
   *
   *  Return: N/A
   **/
  void cam_debug_open(void) {
    static int   boot_init = 1;
    struct stat  file_stat;
    int          i;
    char         buffer[CDBG_MAX_LINE_LENGTH];
    char         property_value[PROPERTY_VALUE_MAX] = {0};
    FILE        *fd = NULL;

    if (boot_init) {
      cam_set_dbg_log_properties();
      boot_init = 0;
      pthread_mutex_init(&dbg_log_mutex, 0);
    }

    /* configure asserts */
    cam_setting_get("persist.vendor.camera.debug.assert", property_value, "0");
    cam_soft_assert = atoi(property_value);

    /* open default log file according to property setting */
    if (cam_log_fd == NULL) {
      cam_setting_get("persist.vendor.camera.debug.logfile", property_value, "0");
      if (atoi(property_value)) {
        /* we always put the current process id at end of log file name */
        char pid_str[255] = {0};
        char new_log_file_name[1024] = {0};
        snprintf(pid_str, 255, "_%d", getpid());
        strlcpy(new_log_file_name, cam_log_filename, sizeof(new_log_file_name));
        strlcat(new_log_file_name, pid_str, sizeof(new_log_file_name));
        cam_log_fd = fopen(new_log_file_name, "a");
        if (cam_log_fd == NULL) {
          CLOGE(CAM_NO_MODULE, "Failed to create debug log file %s\n",
              new_log_file_name);
        } else {
          CLOGD(CAM_NO_MODULE, "Debug log file %s open\n", new_log_file_name);
        }
      } else {
        CLOGD(CAM_NO_MODULE, "Debug log file is not enabled");
        return;
      }

      /* allocate a buffer to cache the log to be written to the file */
      cam_setting_get("persist.vendor.camera.debug.buffersize", property_value, "0");
      g_log_cache_buffer_size = atoi(property_value);
      if (g_log_cache_buffer_size > 0) {
        /* it is ok to fail allocation, then we just don't use the buffer */
        g_log_cache_buffer = malloc(g_log_cache_buffer_size);
        if (g_log_cache_buffer == NULL) {
          CLOGE(CAM_NO_MODULE, "Failed to allocate log buffer with size %d\n",
                g_log_cache_buffer_size);
        }
      }
    }

    /* run with default logging settings if there is no config file present */
    fd = fopen(cam_config_filename, "r");
    if (fd == NULL) {
      cam_set_dbg_log_properties();
      return;
    }

    if (stat(cam_config_filename, &file_stat)) {
       CAM_DBG_ASSERT_ALWAYS(CAM_NO_MODULE,
           "Should always be able to stat() a config file we have open!");
    }

    while (fgets(buffer, sizeof(buffer), fd)) {
      char *pToken, *pValue, *pSave;

      /* Get next token/value pair from the config file */
      pToken = strtok_r(buffer, " =\n\r\t", &pSave);
      pValue = strtok_r(NULL,   " =\n\r\t", &pSave);

      /* Avoid crashes when reading blank lines or incomplete lines */
      if (NULL == pToken || NULL == pValue) {
        continue;
      }

      /* Discard this line if the first token starts with a semicolon, */
      /* because this is a comment line. */
      if (*pToken == ';') {
        continue;
      }

      /* Parse the config settings. */
      if (!strcmp(pToken, "logfile"))
      {
        if (cam_log_fd == NULL) {
          cam_log_fd = fopen(pValue, "a");
          if (cam_log_fd == NULL) {
            CLOGE(CAM_NO_MODULE, "Failed to create debug log file %s\n",
                pValue);
          } else {
            CLOGD(CAM_NO_MODULE, "Debug log file %s open\n", pValue);
          }
        }
      } else if (!strcmp(pToken, "cam_dbglevel")) {
        cam_loginfo[CAM_NO_MODULE].level =
            cam_get_dbg_level(cam_loginfo[CAM_NO_MODULE].name, pValue);
        cam_loginfo[CAM_NO_MODULE].initialized = 1;
      } else if (!strcmp(pToken, "mct_dbglevel")) {
        cam_loginfo[CAM_MCT_MODULE].level =
            cam_get_dbg_level(cam_loginfo[CAM_MCT_MODULE].name, pValue);
        cam_loginfo[CAM_MCT_MODULE].initialized = 1;
      } else if (!strcmp(pToken, "sensor_dbglevel")) {
        cam_loginfo[CAM_SENSOR_MODULE].level =
            cam_get_dbg_level(cam_loginfo[CAM_SENSOR_MODULE].name, pValue);
        cam_loginfo[CAM_SENSOR_MODULE].initialized = 1;
      } else if (!strcmp(pToken, "iface_dbglevel")) {
        cam_loginfo[CAM_IFACE_MODULE].level =
            cam_get_dbg_level(cam_loginfo[CAM_IFACE_MODULE].name, pValue);
        cam_loginfo[CAM_IFACE_MODULE].initialized = 1;
      } else if (!strcmp(pToken, "isp_dbglevel")) {
        cam_loginfo[CAM_ISP_MODULE].level =
            cam_get_dbg_level(cam_loginfo[CAM_ISP_MODULE].name, pValue);
        cam_loginfo[CAM_ISP_MODULE].initialized = 1;
      } else if (!strcmp(pToken, "stats_dbglevel")) {
        cam_loginfo[CAM_STATS_MODULE].level =
            cam_get_dbg_level(cam_loginfo[CAM_STATS_MODULE].name, pValue);
        cam_loginfo[CAM_STATS_MODULE].initialized = 1;
      } else if (!strcmp(pToken, "pproc_dbglevel")) {
        cam_loginfo[CAM_PPROC_MODULE].level =
            cam_get_dbg_level(cam_loginfo[CAM_PPROC_MODULE].name, pValue);
        cam_loginfo[CAM_PPROC_MODULE].initialized = 1;
      } else if (!strcmp(pToken, "imglib_dbglevel")) {
        cam_loginfo[CAM_IMGLIB_MODULE].level =
             cam_get_dbg_level(cam_loginfo[CAM_IMGLIB_MODULE].name, pValue);
        cam_loginfo[CAM_IMGLIB_MODULE].initialized = 1;
      } else if (!strcmp(pToken, "cpp_dbglevel")) {
        cam_loginfo[CAM_CPP_MODULE].level =
             cam_get_dbg_level(cam_loginfo[CAM_CPP_MODULE].name, pValue);
        cam_loginfo[CAM_CPP_MODULE].initialized = 1;
      } else if (!strcmp(pToken, "hal_dbglevel")) {
        cam_loginfo[CAM_HAL_MODULE].level =
             cam_get_dbg_level(cam_loginfo[CAM_HAL_MODULE].name, pValue);
        cam_loginfo[CAM_HAL_MODULE].initialized = 1;
      } else if (!strcmp(pToken, "jpeg_dbglevel")) {
        cam_loginfo[CAM_JPEG_MODULE].level =
             cam_get_dbg_level(cam_loginfo[CAM_JPEG_MODULE].name, pValue);
        cam_loginfo[CAM_JPEG_MODULE].initialized = 1;
      } else if (!strcmp(pToken, "c2d_dbglevel")) {
        cam_loginfo[CAM_C2D_MODULE].level =
             cam_get_dbg_level(cam_loginfo[CAM_C2D_MODULE].name, pValue);
        cam_loginfo[CAM_C2D_MODULE].initialized = 1;
      } else if (!strcmp(pToken, "shim_dbglevel")) {
        cam_loginfo[CAM_SHIM_LAYER].level =
             cam_get_dbg_level(cam_loginfo[CAM_SHIM_LAYER].name, pValue);
        cam_loginfo[CAM_SHIM_LAYER].initialized = 1;

      } else if (!strcmp(pToken, "cam_assert")) {
        cam_soft_assert = atoi(pValue);
      } else {
        CLOGE(CAM_NO_MODULE, "Invalid token %s\n", pToken);
      }
    }

    fclose(fd);
    cam_set_dbg_log_properties();
  }

  /** cam_debug_flush
   *
   *  Flush logging file when camera closes.
   *
   *  Return: N/A
   **/
  void cam_debug_flush(void) {
    if (cam_log_fd != NULL) {
      fflush(cam_log_fd);
      cam_log_fd = NULL;
    }
  }

   /** cam_debug_close
   *
   *  Release logging resources.
   *
   *  Return: N/A
   **/
  void cam_debug_close(void) {
    if (cam_log_fd != NULL) {
      if (g_log_cache_buffer != NULL) {
        if (g_log_cache_buffer_offset > 0) {
          fprintf(cam_log_fd, "%s", g_log_cache_buffer);
          fflush(cam_log_fd);
          g_log_cache_buffer_offset = 0;
        }
        free(g_log_cache_buffer);
      }
      fclose(cam_log_fd);
      cam_log_fd = NULL;
    }
    pthread_mutex_destroy(&dbg_log_mutex);
  }

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
int cam_setting_get(const char *key, char *value, const char *default_value) {
  return property_get(key, value, default_value);
}

/* cam_setting_set:
 *
 *  @key:    name of the setting to set
 *  @value:  new setting value
 *
 *  Sets a new value for a setting
 *
 *  Return: 0 on success, < 0 on failure
 */
int cam_setting_set(const char *key, const char *value) {
  return property_set(key, value);
}

#endif  /* LOG_DEBUG */ /* _ANDROID_ */

#if defined(KPI_DEBUG) && defined(_ANDROID_)

#define CAMSCOPE_MEMSTORE_SIZE 0x00100000 // 1MB

volatile uint32_t kpi_camscope_flags = 0;
volatile uint32_t kpi_camscope_frame_count = 0;

static const char * camscope_filenames[CAMSCOPE_SECTION_SIZE] = {
    QCAMERA_DUMP_FRM_LOCATION"camscope_mmcamera.bin",
    QCAMERA_DUMP_FRM_LOCATION"camscope_hal.bin",
    QCAMERA_DUMP_FRM_LOCATION"camscope_jpeg.bin"
};

static FILE * camscope_fd[CAMSCOPE_SECTION_SIZE];
static uint32_t camscope_num_bytes_stored[CAMSCOPE_SECTION_SIZE];
static char * camscope_memstore[CAMSCOPE_SECTION_SIZE];
static pthread_mutex_t camscope_mutex[CAMSCOPE_SECTION_SIZE];

/* camscope_init:
 *
 *  @camscope_section: camscope section where this function is occurring
 *
 *  Initializes the CameraScope tool functionality
 *
 *  Return: N/A
 */
void camscope_init(camscope_section_type camscope_section) {
    pthread_mutex_init(&(camscope_mutex[camscope_section]), NULL);
    if (camscope_fd[camscope_section] == NULL) {
        if(camscope_memstore[camscope_section] == NULL) {
            camscope_memstore[camscope_section] =
                malloc(CAMSCOPE_MEMSTORE_SIZE);
            if (camscope_memstore[camscope_section] == NULL) {
              CLOGE(CAM_NO_MODULE, "Failed to allocate camscope memstore"
                    "with size %d\n", CAMSCOPE_MEMSTORE_SIZE);
            }
        }
        camscope_fd[camscope_section] =
            fopen(camscope_filenames[camscope_section], "ab");
    }
}

/* camscope_flush:
 *
 *  @camscope_section: camscope section where this function is occurring
 *
 *  Flushes the camscope memstore to the file system
 *
 *  Return: N/A
 */
static void camscope_flush(camscope_section_type camscope_section) {
    if (camscope_fd[camscope_section] != NULL &&
        camscope_memstore[camscope_section] != NULL) {
        fwrite(camscope_memstore[camscope_section], sizeof(char),
               camscope_num_bytes_stored[camscope_section],
               camscope_fd[camscope_section]);
        camscope_num_bytes_stored[camscope_section] = 0;
    }
}

/* camscope_destroy:
 *
 *  @camscope_section: camscope section where this function is occurring
 *
 *  Flushes any remaining data to the file system and cleans up CameraScope
 *
 *  Return: N/A
 */
void camscope_destroy(camscope_section_type camscope_section) {
    if (camscope_fd[camscope_section] != NULL) {
        pthread_mutex_lock(&(camscope_mutex[camscope_section]));
        if(camscope_memstore[camscope_section] != NULL) {
            camscope_flush(camscope_section);
            free(camscope_memstore[camscope_section]);
            camscope_memstore[camscope_section] = NULL;
        }
        fclose(camscope_fd[camscope_section]);
        camscope_fd[camscope_section] = NULL;
        pthread_mutex_unlock(&(camscope_mutex[camscope_section]));
    }
    pthread_mutex_destroy(&(camscope_mutex[camscope_section]));
}

/* camscope_reserve:
 *
 *  @camscope_section:     camscope section where this function is occurring
 *  @num_bytes_to_reserve: number in bytes to reserve on the memstore
 *
 *  Reserves a number of bytes on the memstore flushing to the
 *  file system if remaining space is insufficient
 *
 *  Return: number of bytes successfully reserved on the memstore
 */
uint32_t camscope_reserve(camscope_section_type camscope_section,
                                 uint32_t num_bytes_to_reserve) {
    uint32_t bytes_reserved = 0;
    if (camscope_fd[camscope_section] != NULL &&
        num_bytes_to_reserve <= CAMSCOPE_MEMSTORE_SIZE) {
        int32_t size = CAMSCOPE_MEMSTORE_SIZE -
               camscope_num_bytes_stored[camscope_section] -
               num_bytes_to_reserve;
        if (size < 0) {
            camscope_flush(camscope_section);
        }
        bytes_reserved = num_bytes_to_reserve;
    }
    return bytes_reserved;
}

/* camscope_store_data:
 *
 *  @camscope_section: camscope section where this function is occurring
 *  @data:             data to be stored
 *  @size:             size of data to be stored
 *
 *  Store the data to the memstore and calculate remaining space
 *
 *  Return: N/A
 */
void camscope_store_data(camscope_section_type camscope_section,
                       void* data, uint32_t size) {
    if(camscope_memstore[camscope_section] != NULL) {
        memcpy(camscope_memstore[camscope_section] +
               camscope_num_bytes_stored[camscope_section],
               (char*)data, size);
        camscope_num_bytes_stored[camscope_section] += size;
    }
}

/* camscope_mutex_lock:
 *
 *  @camscope_section: camscope section where this function is occurring
 *
 *  Lock the camscope mutex lock for the given camscope section
 *
 *  Return: N/A
 */
void camscope_mutex_lock(camscope_section_type camscope_section) {
    pthread_mutex_lock(&(camscope_mutex[camscope_section]));
}

/* camscope_mutex_unlock:
 *
 *  @camscope_section: camscope section where this function is occurring
 *
 *  Unlock the camscope mutex lock for the given camscope section
 *
 *  Return: N/A
 */
void camscope_mutex_unlock(camscope_section_type camscope_section) {
    pthread_mutex_unlock(&(camscope_mutex[camscope_section]));
}

#endif /* KPI_DEBUG */ /* _ANDROID_ */
