/**********************************************************************
*  Copyright (c) 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __QCAMERA_MFPROC_PRECPP_H__
#define __QCAMERA_MFPROC_PRECPP_H__

extern "C" {
  #include "img_common.h"
  #include "img_meta.h"
}

/*========QCameraMFProcPrecpp==================================================
 * Class: QCameraMFProcPrecpp
 *
 * Description: This class represents a temple implementation
 *
 * Notes: none
 *==========================================================================*/
class QCameraMFProcPrecpp
{
public:
 /**
  * Function: QCameraMFProcPrecpp
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
  QCameraMFProcPrecpp();

 /**
  * Function: ~QCameraMFProcPrecpp
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
  ~QCameraMFProcPrecpp();

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
 *   @arg5: p_meta: frame meta data
  *
  * Return values:
  *   IMG_SUCCESS or IMG_ERR_GENERAL
  *
  * Notes: none
  **/
  int8_t processData(img_frame_t *inFrame[], int inFrameCnt,
    img_frame_t *outFrame[], int outFrameCnt, img_meta_t* p_meta);

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
  inline bool IsInitialized() {return mMFProcPrecppInitialized;}

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
  bool  mMFProcPrecppInitialized;

  /**
  * pointer to MultiFrame Process Precpp algorithm implementation.
  */
  void*  mLibPtr;

  /**
  * function pointer to dummy function.
  */
  void (*mDummyProc)(unsigned char*, unsigned short*, int, int, int, int,
    int, int, int, int*);

public:

};

#endif
