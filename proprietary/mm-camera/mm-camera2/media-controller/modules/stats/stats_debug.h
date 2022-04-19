/* stats_debug.h
 *
 * Copyright (c) 2014-2018 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __STATS_DEBUG_H__
#define __STATS_DEBUG_H__

#include "camera_dbg.h"
#include "mct_event.h"

#define STATS_USE_DYNAMIC_LOGGING
#undef LOG_TAG
#define LOG_TAG "mm-camera"

#define STATS_DEBUG         1
#define MAX(x,y) (((x)>(y)) ? (x) : (y))
#define STATS_UNUSED(x) (void)(x)

#define TRACE_BUF_SIZE 30


/* adb shell setprop persist.vendor.camera.stats.test */
typedef enum {
  STATS_DBG_TEST_NONE          = 0,
  STATS_DBG_TEST_AEC           = 1,
  STATS_DBG_TEST_AF            = 2,
  STATS_DBG_TEST_BRIGHTNES     = 3,
  STATS_DBG_DISABLE_PDAF       = 4,
  STATS_DBG_DISABLE_HAF_STATUS = 5,
  STATS_DBG_TEST_DRC_FIXED     = 6,
  STATS_DBG_TEST_DRC_TOGGLE    = 7,
  STATS_DBG_TEST_FIXED_3A      = 8,
} stats_debug_test_t;

typedef enum {
  DISABLE_STATS_NONE = 0,
  DISABLE_STATS_PDAF_MASK = 1<<0,
  DISABLE_STATS_TOF_MASK = 1<<1,
  DISABLE_STATS_DCIAF_MASK = 1<<2,
}stats_disable_mask;


extern volatile uint32_t stats_debug_data_log_level;
extern volatile uint32_t stats_debug_test;
extern volatile uint32_t stats_exif_debug_mask;

#if (STATS_DEBUG)

/* Always print err without checking for debug level */
#define AEC_ERR(fmt, args...) \
    CLOGE(CAM_STATS_AEC_MODULE,  fmt, ##args)

#define AEC_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_AEC_MODULE,  fmt, ##args)

#define AEC_LOW(fmt, args...) \
    CLOGL(CAM_STATS_AEC_MODULE,  fmt, ##args)

#define AEC_INFO(fmt, args...) \
    CLOGI(CAM_STATS_AEC_MODULE,  fmt, ##args)


#define AWB_ERR(fmt, args...) \
    CLOGE(CAM_STATS_AWB_MODULE,  fmt, ##args)

#define AWB_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_AWB_MODULE,  fmt, ##args)

#define AWB_LOW(fmt, args...) \
    CLOGL(CAM_STATS_AWB_MODULE,  fmt, ##args)

#define AWB_INFO(fmt, args...) \
    CLOGI(CAM_STATS_AWB_MODULE,  fmt, ##args)


#define AF_ERR(fmt, args...) \
    CLOGE(CAM_STATS_AF_MODULE,  fmt, ##args)

#define AF_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_AF_MODULE,  fmt, ##args)

#define AF_LOW(fmt, args...) \
    CLOGL(CAM_STATS_AF_MODULE,  fmt, ##args)

#define AF_INFO(fmt, args...) \
    CLOGI(CAM_STATS_AF_MODULE,  fmt, ##args)


#define ASD_ERR(fmt, args...) \
    CLOGE(CAM_STATS_ASD_MODULE,  fmt, ##args)

#define ASD_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_ASD_MODULE,  fmt, ##args)

#define ASD_LOW(fmt, args...) \
    CLOGL(CAM_STATS_ASD_MODULE,  fmt, ##args)

#define ASD_INFO(fmt, args...) \
    CLOGI(CAM_STATS_ASD_MODULE,  fmt, ##args)


#define AFD_ERR(fmt, args...) \
    CLOGE(CAM_STATS_AFD_MODULE,  fmt, ##args)

#define AFD_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_AFD_MODULE,  fmt, ##args)

#define AFD_LOW(fmt, args...) \
    CLOGL(CAM_STATS_AFD_MODULE,  fmt, ##args)


#define AFD_INFO(fmt, args...) \
    CLOGI(CAM_STATS_AFD_MODULE,  fmt, ##args)


#define Q3A_ERR(fmt, args...) \
    CLOGE(CAM_STATS_Q3A_MODULE,  fmt, ##args)

#define Q3A_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_Q3A_MODULE,  fmt, ##args)

#define Q3A_LOW(fmt, args...) \
    CLOGL(CAM_STATS_Q3A_MODULE,  fmt, ##args)

#define Q3A_INFO(fmt, args...) \
    CLOGI(CAM_STATS_Q3A_MODULE,  fmt, ##args)


#define STATS_ERR(fmt, args...) \
    CLOGE(CAM_STATS_MODULE,  fmt, ##args)

#define STATS_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_MODULE,  fmt, ##args)

#define STATS_LOW(fmt, args...) \
    CLOGL(CAM_STATS_MODULE,  fmt, ##args)

#define STATS_INFO(fmt, args...) \
    CLOGI(CAM_STATS_MODULE,  fmt, ##args)

#define STATS_DBG(fmt, args...) \
    CLOGD(CAM_STATS_MODULE,  fmt, ##args)


#define IS_ERR(fmt, args...) \
    CLOGE(CAM_STATS_IS_MODULE,  fmt, ##args)

#define IS_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_IS_MODULE,  fmt, ##args)

#define IS_LOW(fmt, args...) \
    CLOGL(CAM_STATS_IS_MODULE,  fmt, ##args)

#define IS_INFO(fmt, args...) \
    CLOGI(CAM_STATS_IS_MODULE,  fmt, ##args)


#else
/* If simply do nothing in the while loop, the unused variable
   warning will be generated, put PPRINT in the while loop has
   no effect, but suppresses the compiler warnings.
*/
#define AEC_ERR(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define AEC_HIGH(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define AEC_LOW(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define AWB_ERR(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define AWB_HIGH(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define AWB_LOW(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define AF_ERR(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define AF_HIGH(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define AF_LOW(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define AFD_ERR(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define AFD_HIGH(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define AFD_LOW(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define ASD_ERR(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define ASD_HIGH(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define ASD_LOW(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define Q3A_ERR(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define Q3A_HIGH(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define Q3A_LOW(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define STATS_ERR(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define STATS_HIGH(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define STATS_LOW(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define STATS_DBG(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define IS_ERR(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define IS_HIGH(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define IS_LOW(fmt, args...) do {PPRINT (fmt, ##args)} while(0)

#endif


#undef LOG_TAG
#define LOG_TAG "mm-camera-CORE"

#undef CDBG_ASD
#undef CDBG_AFD

#if (STATS_DEBUG)
#define ASD_MSG_ERROR(fmt, args...) CLOGE(CAM_STATS_ASD_MODULE, fmt, ##args)
#define ASD_MSG_HIGH(fmt, args...)  CLOGH(CAM_STATS_ASD_MODULE, fmt, ##args)
#define ASD_MSG_LOW(fmt, args...)   CLOGL(CAM_STATS_ASD_MODULE, fmt, ##args)
#define ASD_MSG_INFO(fmt, args...)  CLOGI(CAM_STATS_ASD_MODULE, fmt, ##args)

#define AFD_MSG_ERROR(fmt, args...) CLOGE(CAM_STATS_AFD_MODULE, fmt, ##args)
#define AFD_MSG_HIGH(fmt, args...)  CLOGH(CAM_STATS_AFD_MODULE, fmt, ##args)
#define AFD_MSG_LOW(fmt, args...)   CLOGL(CAM_STATS_AFD_MODULE, fmt, ##args)
#define AFD_MSG_INFO(fmt, args...)  CLOGI(CAM_STATS_AFD_MODULE, fmt, ##args)

#else
#define ASD_MSG_ERROR(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define ASD_MSG_HIGH(fmt, args...)  do {PPRINT (fmt, ##args)} while(0)
#define ASD_MSG_LOW(fmt, args...)   do {PPRINT (fmt, ##args)} while(0)
#define ASD_MSG_INFO(fmt, args...)  do {PPRINT (fmt, ##args)} while(0)

#define AFD_MSG_ERROR(fmt, args...) do {PPRINT (fmt, ##args)} while(0)
#define AFD_MSG_HIGH(fmt, args...)  do {PPRINT (fmt, ##args)} while(0)
#define AFD_MSG_LOW(fmt, args...)   do {PPRINT (fmt, ##args)} while(0)
#define AFD_MSG_INFO(fmt, args...)  do {PPRINT (fmt, ##args)} while(0)



#endif // STATS_DEBUG

char * stats_port_get_mct_event_ctrl_string(enum _mct_event_control_type eventId);
char * stats_port_get_mct_event_module_string(enum _mct_event_module_type eventId);

#endif // __STATS_DBG_H__
