/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __QCAMERA_QUADRACFA_H__
#define __QCAMERA_QUADRACFA_H__

extern "C" {
#include "img_common.h"
#include "img_meta.h"
}

/*========QCameraQuadraCfa==================================================
 * Class: QCameraQuadraCfa
 *
 * Description: This class represents a temple implementation
 *
 * Notes: none
 *==========================================================================*/
class QCameraQuadraCfa
{
public:
 /**
  * Function: QCameraQuadraCfa
  *
  * Description: constructor of class
  *
  * Arguments:
  *   none
  *
  * Return values:
  *     none
  *
  * Notes: none
  **/
  QCameraQuadraCfa();

 /**
  * Function: ~QCameraQuadraCfa
  *
  * Description: destructor of class
  *
  * Arguments:
  *   none
  *
  * Return values:
  *     none
  *
  * Notes: none
  **/
  ~QCameraQuadraCfa();

 /**
  * Function: processData
  *
  * Description: main processing body
  *
  * Arguments:
  *   @arg1: p_in_frame: input frame handler
  *   @arg2: in_frame_cnt: number of input frames
  *   @arg3: p_out_frame: output frame handler
  *   @arg4: out_frame_cnt: number of output frames
  *   @arg3: p_meta: frame meta data, containing ROI info
  *
  * Return values:
  *   IMG_SUCCESS or IMG_ERR_GENERAL
  *
  * Notes: none
  **/
  int8_t processData(img_frame_t *inFrame[], int inFrameCnt,
    img_frame_t *outFrame[], int outFrameCnt,
    img_meta_t* p_meta);

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

 /**
  * Function: IsInitialized
  *
  * Description: return if library is loaded and linked
  *
  * Arguments:
  *
  * Return values:
  *   bool
  *
  * Notes: none
  **/
  inline bool IsInitialized() {return mQuadraCfaInitialized;}

  /**
   * Function: PresetConfig
   *
   * Description: set config for algo in advance
   *
   * Arguments:
   *   @aOps :base ops
   *   @p_params : preset config params
   *
   * Return values:
   *   IMG_SUCCESS or imaging errors
   *
   * Notes: none
   **/
  static int32_t PresetConfig(img_base_ops_t *aOps, void *p_params);

private:

 /**
  * Frame ops
  */
  img_frame_ops_t mFrameOps;

 /**
  * Buffer count
  */
  int32_t mBufCnt;

 /**
  * flag to check if module is initialized and avoid multiple initialization
  */
  bool  mQuadraCfaInitialized;

  /**
  * pointer to remosaic algorithm implementation.
  */
  void*  mLibPtr;

  /**
  * function pointer to remosaic function.
  */
  void (*mRun4PixelProc)(unsigned char*, unsigned short*, int, int, int, int,
    int, int, int, int*);

public:

};

#endif
