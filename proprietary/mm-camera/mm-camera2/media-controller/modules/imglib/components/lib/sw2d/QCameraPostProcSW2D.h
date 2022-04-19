/**********************************************************************
*  Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __QCAMERAPOSTPROCSW2D_H__
#define __QCAMERAPOSTPROCSW2D_H__

#ifdef USE_FASTCV_OPT
#include <fastcv/fastcv.h>
#endif

extern "C" {
#include "img_common.h"
#include "img_meta.h"
#include "img_buffer.h"
}
#include "img_thread.h"

/*===========================================================================
 * Class: QCameraPostProcSW2D
 *
 * Description: This class represents the class of processing sw2d
 *   algorithm
 *
 * Notes: none
 *==========================================================================*/
class QCameraPostProcSW2D
{
public:

/**
 * Function: QCameraPostProcSW2D
 *
 * Description: constructor of class
 *
 * Arguments:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
QCameraPostProcSW2D();

/**
 * Function: QCameraPostProcSW2D
 *
 * Description: destructor of class
 *
 * Arguments:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
~QCameraPostProcSW2D();

/**
 * Function: processData
 *
 * Description: main processing body
 *
 * Arguments:
 *   @arg1: inFrame: input frames
 *   @arg2: outFrame: output frame
 *   @arg3: p_meta: frame meta data
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
int8_t processData(img_frame_t *inFrame[],
  img_frame_t *outFrame, img_meta_t *p_meta);

/**
 * Function: deinterleaveFrame
 *
 * Description: Deinterleaves input frame to semi-planar if the
 *   input is a single plane YUV format
 *
 * Arguments:
 *   @inFrame: input frames
 *   @pOutFrame: output frame. if NULL, allocate internal ION
 *     buffer
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
int8_t deinterleaveFrame(img_frame_t *inFrame, uint8_t *pOutFrame);

/**
 * Function: setFrameOps
 *
 * Description: set frame ops
 *
 * Arguments:
 *   @pFrameOps: pointer to frame ops
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
inline void setFrameOps(img_frame_ops_t *pFrameOps) {mFrameOps = *pFrameOps;}

private:
  img_frame_ops_t mFrameOps; /* frameops used for intermediate buffer */

  img_mem_handle_t mBufMemHandle; /* image memory handle for internal buf */

  bool mBufAllocDone; /* indicates if internal buf is allocated */
};
#endif
