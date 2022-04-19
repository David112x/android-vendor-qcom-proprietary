/**********************************************************************
*  Copyright (c) 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __QCAMERA_THIRDPARTYEIS_H__
#define __QCAMERA_THIRDPARTYEIS_H__

extern "C" {
#include "img_common.h"
#include "img_meta.h"
#include "img_queue.h"
#include "img_buffer.h"
#include "img_dbg.h"
}


/*========QCameraThirdPartyEIS====================================================
 * Class: QCameraThirdPartyEIS
 *
 * Description: This class represents a temple implementation
 *
 * Notes: none
 *==========================================================================*/
class QCameraThirdPartyEIS
{
public:
 /**
  * Function: QCameraThirdPartyEIS
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
  QCameraThirdPartyEIS();

 /**
  * Function: ~QCameraThirdPartyEIS
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
  ~QCameraThirdPartyEIS();

 /**
  * Function: Init
  *
  * Description: Initialization for the class
  *
  * Arguments:
  *   none
  *
  * Return values:
  *     True/False: Status of  initialization
  *
  * Notes: none
  **/
bool Init();

 /**
  * Function: DeInit
  *
  * Description: Deinitialization for the class
  *
  * Arguments:
  *   none
  *
  * Return values:
  *     True/False: Status of  initialization
  *
  * Notes: none
  **/
bool DeInit();

/**
 * Returns deinit state
 **/

bool IsDeinitStarted() {
  return bIsDeinitStarted;
}

/**
 * Sets deinit state
 **/

inline void setDeinitStart(bool bVal) {
  bIsDeinitStarted = bVal;
}

void Flush();

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
  *   @arg5: p_meta: frame meta data, containing ROI info
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

  int32_t thirdpartyeisInit(int32_t in_width, int32_t in_height,
    int32_t out_width, int32_t out_height, uint32_t fps, int32_t stream_type);

  void thirdpartyeisDeInit();

  /**
  * Frame ops
  */
  img_frame_ops_t mFrameOps;

private:

  typedef struct {
  } thirdpartyeis_lib_t;


  /*
   * Thread context Data
   */
  typedef struct _thirdpartyeis_thread_data_t{
    thirdpartyeis_lib_t    thirdpartyeis_lib;
  }thirdpartyeis_thread_data_t;

public:
  int (*callback)(void *user_data);
  pthread_t threadId;
  thirdpartyeis_thread_data_t *m_pThreadData = NULL;
  bool bIsDeinitStarted;
  img_queue_t callBackQ;
};

#endif
