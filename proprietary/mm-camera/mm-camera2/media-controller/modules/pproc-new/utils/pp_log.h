/*============================================================================

 Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef PP_LOG_H
#define PP_LOG_H

#include "camera_dbg.h"

#define PP_LOG_LEVEL 1

#undef LOG_TAG
#define LOG_TAG "mm-camera-pp"

#define PP_ERR(fmt, args...) MDBG_ERROR(CAM_PPROC_MODULE, "%s:%d, " fmt"", __func__, __LINE__, ##args);
#define PP_WARN(fmt, args...) MDBG_WARN(CAM_PPROC_MODULE, "%s:%d, " fmt"", __func__, __LINE__, ##args);

#define PP_HIGH(fmt, args...) \
      MDBG_HIGH(CAM_PPROC_MODULE, "%s:%d, " fmt"", __func__, __LINE__, ##args); \

#define PP_DBG(fmt, args...) \
      MDBG(CAM_PPROC_MODULE, "%s:%d, " fmt"", __func__, __LINE__, ##args); \

#define PP_LOW(fmt, args...) \
      MDBG_LOW(CAM_PPROC_MODULE, "%s:%d, " fmt"", __func__, __LINE__, ##args); \

#define PP_INFO(fmt, args...) CLOGI(CAM_PPROC_MODULE, fmt, ##args)
#endif
