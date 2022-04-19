/* media_controller.h
 *                               .
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MEDIA_CONTROLLER_H__
#define __MEDIA_CONTROLLER_H__

#include "mct_list.h"
#include "mct_util.h"
#include "mtype.h"

/* put mct forward declarations here due to cross dependencies */
typedef struct _mct_controller mct_controller_t;
typedef struct _mct_object mct_object_t;
typedef struct _mct_event mct_event_t;
typedef struct _mct_bus mct_bus_t;
typedef struct _mct_pipeline mct_pipeline_t;
typedef struct _mct_stream mct_stream_t;
typedef struct _mct_module mct_module_t;
typedef struct _mct_port mct_port_t;

#define MCT_MODULE_CAST(obj) ((mct_module_t *)(obj))
#define MCT_PORT_CAST(obj)   ((mct_port_t *)(obj))

#define pack_identity(sessionid, streamid) \
  ((sessionid & 0x0000FFFF) << 16) | (streamid & 0x0000FFFF)
#define UNPACK_STREAM_ID(identity) (identity & 0x0000FFFF)
#define UNPACK_SESSION_ID(identity) ((identity & 0xFFFF0000) >> 16)

typedef enum _mct_status_ret_type {
  MCT_STATUS_RET_SUCCESS,
  MCT_STATUS_RET_FAILURE,
  MCT_STATUS_RET_BUSY
} mct_status_ret_type;

#ifdef _ANDROID_
enum {
MCT_DEBUG_PRIO_ERROR = 1,
MCT_DEBUG_PRIO_HIGH  = 2,
MCT_DEBUG_PRIO_LOW   = 4,
};

enum module {
MODULE_MCT = 1 << 0,
MODULE_SENSOR = 1 << 1,
MODULE_ISP2 = 1 << 2,
MODULE_PPROC_NEW = 1 << 3,
MODULE_STATS = 1 << 4,
};

extern volatile uint32_t mct_debug_level;
extern volatile uint32_t mct_debug_module;

#define cam_module(n) ({ \
        cam_modules_t __m = CAM_NO_MODULE; \
        switch (n) { \
        case MODULE_MCT: \
             __m = CAM_MCT_MODULE; \
        break; \
        case MODULE_SENSOR: \
            __m = CAM_SENSOR_MODULE; \
        break; \
        case MODULE_ISP2:\
            __m = CAM_ISP_MODULE; \
        break; \
        case MODULE_PPROC_NEW:\
            __m = CAM_PPROC_MODULE; \
        break;\
        case MODULE_STATS:\
            __m = CAM_STATS_MODULE; \
        break; \
        default:\
            __m = CAM_NO_MODULE; \
        break; \
        } \
            __m;\
        })

#undef MCT_DBG_ERROR
#define MCT_DBG_ERROR(module, fmt, args...) \
  MDBG_ERROR(cam_module(module), fmt, ##args)
#undef MCT_DBG_HIGH
#define MCT_DBG_HIGH(module, fmt, args...)  \
  MDBG_HIGH(cam_module(module), fmt, ##args)
#undef MCT_DBG_LOW
#define MCT_DBG_LOW(module, fmt, args...)   \
  MDBG_LOW(cam_module(module), fmt, ##args)
#else
#define MCT_DBG_ERROR printf
#define MCT_DBG_HIGH printf
#define MCT_DBG printf
#endif /* _ADNROID_ */

#endif /* __MEDIA_CONTROLLER_H__ */
