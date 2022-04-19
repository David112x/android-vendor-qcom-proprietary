/* q3a_platform.h
 *                                                                   .
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __Q3A_PLATFORM_H__
#define __Q3A_PLATFORM_H__

/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include "stats_debug.h"
#include "cam_types.h"
#include "mtype.h"


/* ==========================================================================
                       Preprocessor Definitions and Constants
========================================================================== */

#if !defined(uint8) && !defined(_UINT8_DEFINED)
typedef uint8_t     uint8;
#define _UINT8_DEFINED
#endif

#if !defined(int8) && !defined(_INT8_DEFINED)
typedef int8_t      int8;
#define _INT8_DEFINED
#endif

#if !defined(uint16) && !defined(_UINT16_DEFINED)
typedef uint16_t    uint16;
#define _UINT16_DEFINED
#endif

#if !defined(int16) && !defined(_INT16_DEFINED)
typedef int16_t     int16;
#define _INT16_DEFINED
#endif

#if !defined(uint32) && !defined(_UINT32_DEFINED)
typedef uint32_t            uint32;
#define _UINT32_DEFINED
#endif

#if !defined(int32) && !defined(_INT32_DEFINED)
typedef int32_t              int32;
#define _INT32_DEFINED
#endif

#if !defined(uint64) && !defined(_UINT64_DEFINED)
typedef uint64_t    uint64;
#define _UINT64_DEFINED
#endif

#if !defined(int64) && !defined(_INT64_DEFINED)
typedef int64_t     int64;
#define _INT64_DEFINED
#endif

#ifndef        TRUE
#define        TRUE         1
#endif
#ifndef        FALSE
#define        FALSE        0
#endif


#define STATS_TEST(_test) (stats_debug_test == _test)

/**
 * Allocate and free macros.
 */
#define AWB_MALLOC(numBytes)                malloc(numBytes)
#define AWB_FREE(ptr)                       free(ptr)

#define AEC_MALLOC(numBytes)                malloc(numBytes)
#define AEC_FREE(ptr)                       free(ptr)

#define AF_MALLOC(numBytes)                 malloc(numBytes)
#define AF_FREE(ptr)                        free(ptr)

#define AFD_MALLOC(numBytes)                malloc(numBytes)
#define AFD_FREE(ptr)                       free(ptr)

/**
 * Math macros.
 */
#define Q3A_LOG2                            log2

/**
* Sleep macro.
*/
#define Q3A_SLEEP(value)                    sleep(value / 1000)

/**
 * Memory macros.
 */
#define Q3A_MEMSET(ptr, value, num_bytes)       memset(ptr, value, num_bytes)
#define Q3A_MEMCPY(to, from, length)            memcpy(to, from, length)
#define Q3A_MEMCMP(pSource1, pSource2, nLength) memcmp(pSource1, pSource2, nLength)
#define Q3A_STRCPY(dest,src,len)                strlcpy(dest,src,len)
#define Q3A_STRLEN(string)                      strlen(string)

#define Q3A_MEMCPY_S(dest, destLength, source, sourceLength) Q3A_MEMCPY(dest, source, sourceLength)
/**
 * kpi macros.
 */
#define Q3A_ATRACE_INT KPI_ATRACE_INT
#define Q3A_ATRACE_INT_SNPRINTF ATRACE_INT_SNPRINTF
#define Q3A_ATRACE_END ATRACE_END
#define Q3A_ATRACE_INT_IF Q3A_TRACE_INT
#define Q3A_ATRACE_BEGIN_SNPRINTF ATRACE_BEGIN_SNPRINTF

/**
 * HAL macros dependencies
 */
#define MAX_STATS_ROI_NUM (10)

/**
 * Logging macros.
 */
#undef LOG_TAG
#define LOG_TAG "mm-3a-core"

#define AWB_MSG_ERROR(fmt, args...) \
    CLOGE(CAM_STATS_AWB_MODULE,  fmt, ##args)

#define AWB_MSG_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_AWB_MODULE,  fmt, ##args)

#define AWB_MSG_LOW(fmt, args...) \
    CLOGL(CAM_STATS_AWB_MODULE,  fmt, ##args)

#define AWB_MSG_INFO(fmt, args...) \
    CLOGI(CAM_STATS_AWB_MODULE,  fmt, ##args)


#define AEC_MSG_ERROR(fmt, args...) \
    CLOGE(CAM_STATS_AEC_MODULE,  fmt, ##args)

#define AEC_MSG_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_AEC_MODULE,  fmt, ##args)

#define AEC_MSG_LOW(fmt, args...) \
    CLOGL(CAM_STATS_AEC_MODULE,  fmt, ##args)

#define AEC_MSG_INFO(fmt, args...) \
    CLOGI(CAM_STATS_AEC_MODULE,  fmt, ##args)


#define AF_MSG_ERROR(fmt, args...) \
    CLOGE(CAM_STATS_AF_MODULE,  fmt, ##args)

#define AF_MSG_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_AF_MODULE,  fmt, ##args)

#define AF_MSG_LOW(fmt, args...) \
    CLOGL(CAM_STATS_AF_MODULE,  fmt, ##args)

#define AF_MSG_INFO(fmt, args...) \
    CLOGI(CAM_STATS_AF_MODULE,  fmt, ##args)


#define HAF_MSG_ERROR(fmt, args...) \
    CLOGE(CAM_STATS_HAF_MODULE,  fmt, ##args)

#define HAF_MSG_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_HAF_MODULE,  fmt, ##args)

#define HAF_MSG_LOW(fmt, args...) \
    CLOGL(CAM_STATS_HAF_MODULE,  fmt, ##args)

#define HAF_MSG_INFO(fmt, args...) \
    CLOGI(CAM_STATS_HAF_MODULE,  fmt, ##args)

#define CAF_SCAN_MSG(fmt, args...) \
    CLOGL(CAM_STATS_CAF_SCAN_MODULE,  fmt, ##args)

#define Q3A_MSG_ERROR(fmt, args...) \
    CLOGE(CAM_STATS_Q3A_MODULE,  fmt, ##args)

#define Q3A_MSG_HIGH(fmt, args...) \
    CLOGH(CAM_STATS_Q3A_MODULE,  fmt, ##args)

#define Q3A_MSG_INFO(fmt, args...) \
    CLOGI(CAM_STATS_Q3A_MODULE,  fmt, ##args)



/**
 * Max value of floating point number.
 */
#define Q3A_FLT_MAX FLT_MAX

/**
 * Assert macro.
 */
#define Q3A_ASSERT assert

/**
 * MAX and MIN macros.
 */
#undef MAX
#define MAX(x, y)       (((x) > (y)) ? (x) : (y))

#undef MIN
#define MIN(x, y)       (((x) < (y)) ? (x) : (y))


#ifndef ABS
#define ABS(x)            (((x) < 0) ? -(x) : (x))
#endif


#define VSNPRINT(buf, size, fmt, arg) vsnprintf(buf, size, fmt, arg)
/* ==========================================================================
                Functions
========================================================================== */
/**
* Returns the current time in ms.
*/
uint32 q3a_get_time_ms();
void q3a_core_set_log_level ();
void q3a_algo_set_log_level(
  uint32_t stats_haf_api_debug_mask_algo,
  uint32_t stats_haf_input_debug_mask_algo,
  char * stats_haf_node_debug_mask_algo,
  uint32_t stats_log_mask_algo,
  uint32_t global_log_level_algo,
  uint32_t kpi_debug_level_algo,
  uint32_t stats_debug_mask_algo);
int q3a_prop_get(char* propName, char *defVal);
#endif // __Q3A_PLATFORM_H__

