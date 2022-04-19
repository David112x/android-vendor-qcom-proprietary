/**********************************************************************
* Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#ifndef __QCAMERASATDUMMY_H__
#define __QCAMERASATDUMMY_H__

#include <stdbool.h>

extern "C" {
#include "img_common.h"
#include "img_meta.h"
}
#include "img_dbg.h"
/**
 *  MACROS and CONSTANTS
 **/

/* Number input frames required for algo*/
#define ALGO_NUM_INPUT  (2)
/* Number output frames required for algo*/
#define ALGO_NUM_OUTPUT (1)
/* Number meta frames required for algo*/
#define ALGO_NUM_META   (2)

typedef enum{
  WIDE_FRAME,
  TELE_FRAME,
  OUT_FRAME,
} algo_frame_type_t;

/* algo_frame_t */
typedef struct
{
  void *vaddr;
  int fd;
  uint32_t width;
  uint32_t height;
  uint32_t stride;
  uint32_t scanline;
  uint32_t frameid;
} algo_frame_t;

/* algo_frame_set_t */
typedef struct
{
  algo_frame_t algoInWide;
  algo_frame_t algoInTele;
  algo_frame_t algoOutput;
} algo_frame_set_t;

/*===========================================================================
 * Class: QCameraSATDummy
 *
 * Description: Dummy wrapper class
 *
 * Notes: none
 *==========================================================================*/
class QCameraSATDummy
{
public:
 /**
  * Function: QCameraSATDummy
  *
  * Description: constructor
  *
  * Arguments:
  *   none
  *
  * Return values:
  *     none
  *
  * Notes: none
  **/
  QCameraSATDummy();

 /**
  * Function: ~QCameraSATDummy
  *
  * Description: destructor
  *
  * Arguments:
  *   none
  *
  * Return values:
  *     none
  *
  * Notes: none
  **/
  ~QCameraSATDummy();

  /**
   * Function: Initialize
   *
   * Description: initialize the algorithm
   *
   * Arguments:
   *   @p_params: pointer to init params
   *
   * Return values:
   *   IMG_SUCCESS or imaging errors
   *
   * Notes: none
   **/
  int32_t Init(img_multicam_init_params_t *p_aParams);

 /**
  * Function: Execute
  *
  * Description: main processing body
  *
  * Arguments:
  *   @p_aInFrame: input frame handler
  *   @aInFrameCnt: number of input frames
  *   @p_aOutFrame: output frame handler
  *   @aOutFrameCnt: number of output frames
  *   @p_aMeta: frame meta data
  *   @aMetaCount: number of metadata
  *
  * Return values:
  *   IMG_SUCCESS or IMG_ERR_GENERAL
  *
  * Notes: none
  **/
  int32_t Execute(img_frame_t *p_aInFrame[],
    int aInFrameCnt,
    img_frame_t  *p_aOutFrame[],
    int aOutFrameCnt,
    img_meta_t *p_aMeta[],
    int aMetaCnt);

 /**
  * Function: De-Initialize
  *
  * Description: Deinitializ the algoritham
  *
  * Arguments:
  *   none
  *
  * Return values:
  *   IMG_SUCCESS or imaging errors
  *
  * Notes: none
  **/
  int32_t DeInit();

  /**
   * Function: SetFrameOps
   *
   * Description: set frame ops table
   *
   * Arguments:
   *   @pFrameOps: pointer to frame ops
   *
   * Return values:
   *   none
   *
   * Notes: img_frame_ops_t includes ops like
   *    @get_frame: The function pointer to get the frame
   *    @release_frame: The function pointer to release the frame
   *    @dump_frame: The function pointer to dump frame
   *    @get_meta: The function pointer to get meta
   *    @set_meta: The function pointer to set meta
   *    @image_copy: The function pointer to image copy
   *    @image_scale: image downscaling
   **/
  inline void SetFrameOps(img_frame_ops_t *p_aFrameOps) {mFrameOps = *p_aFrameOps;}

private:

  /**
   * Function: FillAlgoInitCfg
   *
   * Description: fill the dummy algo initalization param
   *   configuration
   *
   * Arguments:
   *   @p_params: Imagelib init parameters
   *
   * Return values:
   *     none
   *
   * Notes: none
   **/
  void FillAlgoInitCfg(img_multicam_init_params_t *p_aParams);

  /**
   * Function: FillAlgoFrameCfg
   *
   * Description: fill the dummy algo frame configuration
   *
   * Arguments:
   *   @p_aImgFrame: Imagelib frame
   *   @aType: Frame type
   *
   * Return values:
   *   IMG_SUCCESS or IMG_ERR_GENERAL
   *
   * Notes: none
   **/
  int32_t FillAlgoFrameCfg(img_frame_t *p_aImgFrame, algo_frame_type_t aType);

  /**
   * Function: FillAlgoMetaCfg
   *
   * Description: fill the dummy algo meta configuration
   *
   * Arguments:
   *   @aImglibMeta: Imagelib meta
   *   @aIdx: array index
   *
   * Return values:
   *   IMG_SUCCESS or IMG_ERR_GENERAL
   *
   * Notes: none
   **/
  int32_t FillAlgoMetaCfg(img_meta_t *p_aMeta, uint8_t aIdx);

  /**
   * Function: FillAlgoOutputInfo
   *
   * Description: Return algo output info by filling imglib meta
   * structure
   *
   * Arguments:
   *   @aImglibMeta: Imglib meta
   *
   * Return values:
   *     none
   *
   * Notes: none
   **/
  void FillAlgoOutputInfo(img_meta_t *p_aMeta);

  /**
   * Function: MapToImgError
   *
   * Description: mapping error from algorithm to imaging error
   *
   * Arguments:
   *   @aAlgoError: algorithm error
   *
   * Return values:
   *     imaging errors
   *
   * Notes: none
   **/
  int32_t MapToImgError(int32_t aAlgoError);

  /**
   * Function: PrintMetaInfo
   *
   * Description: Print Meta info
   *
   * Arguments:
   *   @p_aMeta: pointer to meta
   *
   * Return values:
   *     imaging errors
   *
   * Notes: none
   **/
  void PrintMetaInfo(img_meta_t *p_aMeta);

private:

  /**
   *  Algo frame info, input to algo
   */
  algo_frame_set_t mAlgoFrameSet;

  /**
   *  Pointer to Imglib wide frame meta, used to fill algo output
   *  info
   */
  img_meta_t *p_mWideMeta;

  /**
   *  Pointer to Imglib wide frame tele, used to fill algo output
   *  info
   */
  img_meta_t *p_mTeleMeta;

  /**
   *  Imglib frame operations pointer
   */
  img_frame_ops_t mFrameOps;

  /**
   *  Wide frame index for input frame and meta arrays
   */
  uint8_t mWideIdx;

  /**
   *  Tele frame index for input frame and meta arrays
   */
  uint8_t mTeleIdx;

  /**
   *  LPM enabled on wide
   */
  uint8_t mWidelpm;

    /**
   *  LPM enabled on tele
   */
  uint8_t mTelelpm;

public:

  /**
   * Flag to indicate whether to measure latency
   */
  static bool mPerfMeasure;
};
#endif /* __QCAMERASATDUMMY_H__ */
