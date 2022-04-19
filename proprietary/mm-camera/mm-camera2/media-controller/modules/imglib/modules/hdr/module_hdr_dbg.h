/**********************************************************************
*  Copyright (c) 2013, 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __MODULE_HDR_DBG_H__
#define __MODULE_HDR_DBG_H__

/** MODULE_HDR_NAME:
 *
 * Defines hdr module name
 *
 * Returns hdr module name
 **/
#define MODULE_HDR_NAME "hdr"

#include "camera_dbg.h"
#include "img_dbg.h"

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_HDR_SHIFT

#endif //__MODULE_HDR_DBG_H__
