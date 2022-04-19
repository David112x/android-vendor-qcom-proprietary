#ifndef __TH_LOG_H__
#define __TH_LOG_H__

/*========================================================================

*//** @file log.h

FILE SERVICES:
      Common log handling code to be used by all Thread Services.

      Detailed description

Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                      Edit History

when       who    what, where, why
--------   ---    -------------------------------------------------------
11/7/14   rg     Initial Version.

========================================================================== */

/*========================================================================*/
/* INCLUDE FILES */
/*==  =====================================================================*/
/* std headers */
#include <utils/Log.h>

/* local headers */

/*========================================================================*/
/* DEFINITIONS AND DECLARATIONS */
/*========================================================================*/
#if defined( __cplusplus )
extern "C"
{
#endif /* end of macro __cplusplus */


#define TH_DEBUG_LEVEL_ALL   0x8
#define TH_DEBUG_LEVEL_MED   0x4
#define TH_DEBUG_LEVEL_HIGH  0x2
#define TH_DEBUG_LEVEL_ERR   0x1
#define TH_DEBUG_LEVEL_NONE  0x0

#define TH_DEBUG_LEVEL_CURR TH_DEBUG_LEVEL_ERR //<--- set your log level here

#undef LOG_NIDEBUG
#undef LOG_TAG
#define LOG_NIDEBUG 0
#define LOG_TAG "mm-camera-th"

#define DBG_ON(fmt, args...) ALOGE("%s:%d "fmt"\n", __func__, __LINE__, ##args)
#define DBG_OFF(fmt, args...) do{}while(0)

#if (TH_DEBUG_LEVEL_CURR == TH_DEBUG_LEVEL_ALL)
  #define TH_LOG_ERR(...) DBG_ON(__VA_ARGS__)
  #define TH_LOG_HIGH(...) DBG_ON(__VA_ARGS__)
  #define TH_LOG_MED(...) DBG_ON(__VA_ARGS__)
  #define TH_LOG_LOW(...) DBG_ON(__VA_ARGS__)
#elif (TH_DEBUG_LEVEL_CURR == TH_DEBUG_LEVEL_MED)
  #define TH_LOG_ERR(...) DBG_ON(__VA_ARGS__)
  #define TH_LOG_HIGH(...) DBG_ON(__VA_ARGS__)
  #define TH_LOG_MED(...) DBG_ON(__VA_ARGS__)
  #define TH_LOG_LOW(...) DBG_OFF(__VA_ARGS__)
#elif (TH_DEBUG_LEVEL_CURR == TH_DEBUG_LEVEL_HIGH)
  #define TH_LOG_ERR(...) DBG_ON(__VA_ARGS__)
  #define TH_LOG_HIGH(...) DBG_ON(__VA_ARGS__)
  #define TH_LOG_MED(...) DBG_OFF(__VA_ARGS__)
  #define TH_LOG_LOW(...) DBG_OFF(__VA_ARGS__)
#elif (TH_DEBUG_LEVEL_CURR == TH_DEBUG_LEVEL_ERR)
  #define TH_LOG_ERR(...) DBG_ON(__VA_ARGS__)
  #define TH_LOG_HIGH(...) DBG_OFF(__VA_ARGS__)
  #define TH_LOG_MED(...) DBG_OFF(__VA_ARGS__)
  #define TH_LOG_LOW(...) DBG_OFF(__VA_ARGS__)
#else
  #define TH_LOG_ERR(...) DBG_OFF(__VA_ARGS__)
  #define TH_LOG_HIGH(...) DBG_OFF(__VA_ARGS__)
  #define TH_LOG_MED(...) DBG_OFF(__VA_ARGS__)
  #define TH_LOG_LOW(...) DBG_OFF(__VA_ARGS__)
#endif


#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */

#endif /* end of macro __TH_LOG_H__ */

