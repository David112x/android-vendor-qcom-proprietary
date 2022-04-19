/**********************************************************************
*  Copyright (c) 2013, 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/


#ifndef __MODULE_DENOISE_DBG_H__
#define __MODULE_DENOISE_DBG_H__

/** MODULE_DENOISE_NAME:
 *
 * Defines denoise module name
 *
 * Returns denoise module name
 **/
#define MODULE_DENOISE_NAME "denoise"

#include "camera_dbg.h"
#include "img_dbg.h"

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_DENOISE_SHIFT

#endif //__MODULE_DENOISE_DBG_H__
