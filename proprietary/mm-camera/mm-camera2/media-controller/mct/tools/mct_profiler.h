/* mct_profile.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef MCT_PROFILER_H
#define MCT_PROFILER_H
#include "camera_dbg.h"
#include <stdio.h>
#include <stdarg.h>

//#define PROF_LOG_USE

#define PROF_MCT_STREAMON                             "MCT_STREAMON"
#define PROF_MCT_STREAMOFF                            "MCT_STREAMOFF"
#define PROF_MCT_STREAMADD                            "MCT_STREAMADD"
#define PROF_MCT_STREAMDEL                            "MCT_STREAMDEL"
#define PROF_MCT_CAM_PRIV_PARAM                       "MCT_CAM_PRIV_PARAM"
#define PROF_MCT_CAM_STREAM_PARAM                     "MCT_CAM_STREAM_PARAM"
#define PROF_MCT_CAM_COMMAND                          "MCT_CAM_COMMAND"
#define PROF_MCT_STREAM_PARAM                         "MCT_STREAM_PARAM"
#define PROF_MCT_STREAM_LINK                          "MCT_STREAM_LINK"
#define PROF_MCT_MSG_PROC_SET                         "MCT_MSG_PROC_SET"
#define PROF_MCT_MSG_PROC_GET                         "MCT_MSG_PROC_GET"
#define PROF_MCT_POST_BUS_MSG                         "MCT_POST_BUS_MSG"
#define PROF_MCT_EVENT_DOWNSTREAM                     "MCT_EVENT_DOWNSTREAM"
#define PROF_MCT_EVENT_UPSTREAM                       "MCT_EVENT_UPSTREAM"
#define PROF_MCT_EVENT_LINK_OFFLINE                   "MCT_EVENT_LINK_OFFLINE"
#define PROF_SRV_PROC_MSG                             "SRV_PROC_MSG"
#define PROF_SRV_RET_MSG                              "SRV_PROC_RET_MSG"
#define PROF_SENSOR_INIT                              "SENSOR_INIT"
#define PROF_SENSOR_DEINIT                            "SENSOR_DEINIT"
#define PROF_SENSOR_STREAM_ON                         "SENSOR_STREAM_ON"
#define PROF_SENSOR_STREAM_OFF                        "SENSOR_STREAM_OFF"
#define PROF_SENSOR_INIT_SEN_MODULES                  "SENSOR_INIT_SEN_MODULES"
#define PROF_SENSOR_INIT_PORT_CREATE                  "SENSOR_INIT_PORT_CREATE"
#define PROF_SENSOR_INIT_EEPROM_INIT                  "SENSOR_INIT_EEPROM_INIT"
#define PROF_SENSOR_INIT_FILL_ALL_SENSORS             "SENSOR_INIT_FILL_ALL_SENSORS"
#define PROF_SENSOR_FIND_OTHER_SUB                    "SENSOR_FIND_OTHER_SUB"
#define PROF_SENSOR_PROCESS_EVENT                     "SENSOR_PROCESS_EVENT"
#define PROF_SENSOR_PROCESS_EVENT_SET_PARM            "SENSOR_PROCESS_EVENT_SET_PARM"
#define PROF_SENSOR_PROCESS_EVENT_PARM_STREAM_BUF     "SENSOR_PROCESS_EVENT_PARM_STREAM_BUF"
#define PROF_SENSOR_PROCESS_EVENT_START_ZSL_SNAPSHOT  "SENSOR_PROCESS_EVENT_START_ZSL_SNAPSHOT"
#define PROF_SENSOR_PROCESS_EVENT_STOP_ZSL_SNAPSHOT   "SENSOR_PROCESS_EVENT_STOP_ZSL_SNAPSHOT"
#define PROF_SENSOR_START_SESSION                     "SENSOR_START_SESSION"
#define PROF_SENSOR_STOP_SESSION                      "SENSOR_STOP_SESSION"
#define PROF_SENSOR_SET_MOD                           "SENSOR_SET_MODE"
#define PROF_SENSOR_MODULE_CREATE                     "SENSOR_MODULE_CREATE"
#define PROF_PORT_PROCESS_EVENT                       "PORT_PROCESS_EVENT"
#define PROF_SENSOR_CHROMATIX                         "SENSOR_CHROMATIX"
#define PROF_SENSOR_LIVE_SNAPSHOT_CHROMATIX           "SENSOR_LIVE_SNAPSHOT_CHROMATIX"
#define PROF_SENSOR_STREAMON_CFG                      "SENSOR_STREAMON_CFG"
#define PROF_IFACE_INIT                               "IFACE_INIT"
#define PROF_IFACE_DEINIT                             "IFACE_DEINIT"
#define PROF_IFACE_START_SESSION                      "IFACE_START_SESSION"
#define PROF_IFACE_STOP_SESSION                       "IFACE_STOP_SESSION"
#define PROF_IFACE_STREAM_ON                          "IFACE_STREAM_ON"
#define PROF_IFACE_STREAM_OFF                         "IFACE_STREAM_OFF"
#define PROF_ISP_INIT                                 "ISP_INIT"
#define PROF_ISP_DEINIT                               "ISP_DEINIT"
#define PROF_ISP_START_SESSION                        "ISP_START_SESSION"
#define PROF_ISP_STOP_SESSION                         "ISP_STOP_SESSION"
#define PROF_ISP_STREAM_ON                            "ISP_STREAM_ON"
#define PROF_ISP_STREAM_OFF                           "ISP_STREAM_OFF"

#define PROF_MCT_SP                                   "MCT_SP"
#define PROF_MCT_SOF                                  "MCT_SOF"
#define PROF_MCT_SPL_EVT                              "MCT_SEVT"

#define PROF_SENSOR_SP                                "SENSOR_SP"
#define PROF_SENSOR_SOF                               "SENSOR_SOF"

#define PROF_IFACE_SP                                 "IFACE_SP"
#define PROF_IFACE_SOF                                "IFACE_SOF"

#define PROF_ISP_SP                                   "ISP_SP"
#define PROF_ISP_SOF                                  "ISP_SOF"

#define PROF_3A_SP                                    "3A_SP"
#define PROF_3A_SOF                                   "3A_SOF"

#define PROF_C2D_SP                                   "C2D_SP"
#define PROF_C2D_SOF                                  "C2D_SOF"

#define PROF_CPP_SP                                   "CPP_SP"
#define PROF_CPP_SOF                                  "CPP_SOF"

#define PROF_FACEPROC_SP                              "FACEPROC_SP"
#define PROF_FACEPROC_SOF                             "FACEPROC_SOF"

#define PROF_IMGLIB_SP                                "IMGLIB_SP"
#define PROF_IMGLIB_SOF                               "IMGLIB_SOF"

#define PROF_SNAP_REQUEST                             "SNAP_REQUEST"
#define PROF_ISP_EPOCH                                "ISP_EPOCH"
#define PROF_ISP_AWB                                  "ISP_AWB"
#define PROF_IFACE_AXI_SOF                            "IFACE_AXI_SOF"

#define PROF_ISP_SOF                                  "ISP_SOF"


#ifdef PROF_LOG_USE
void mct_profiler_add(const char *string, ...);

#define MCT_PROF_LOG_BEG_T(a, ...)    mct_profiler_add(a, ##__VA_ARGS__)
#define MCT_PROF_LOG_BEG(...) \
 if (kpi_debug_level >= MCT_KPI_DBG) { \
   MCT_PROF_LOG_BEG_T( __VA_ARGS__, NULL); \
}

#define MCT_PROF_LOG_END() \
 if (kpi_debug_level >= MCT_KPI_DBG) { \
   ATRACE_END(); \
}

#else
  #define MCT_PROF_LOG_BEG(...)
  #define MCT_PROF_LOG_END()
#endif


#endif // MCT_PROFILER_H
