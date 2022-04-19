/**********************************************************************
*  Copyright (c) 2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/


#ifndef __QCAMERA_DEPTHMAP_H__
#define __QCAMERA_DEPTHMAP_H__

#include <vector>
#include <cam_types.h>
#include <sys/types.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <utils/Log.h>
#include <assert.h>
#include <sys/time.h>
#include <fcntl.h>
#include <cutils/properties.h>
#include "DepthMapWrapper.h"


extern "C" {
#include "img_common.h"
#include "img_meta.h"
}

#define DEPTH_MAP_WIDTH                 2688
#define DEPTH_MAP_HEIGHT                1520
#define DEPTH_MAP_INPUT_FRAME_IDX   0
#define DEPTH_MAP_PLANE_CNT         1

#undef CDBG
#undef CDBG_ERROR
#undef CDBG_INFO

#ifdef _ANDROID_
  #define CDBG_ERROR(fmt, args...) \
    ALOGE("%s:%d " fmt "\n", __func__, __LINE__, ##args)
  #define CDBG_INFO(fmt, args...) \
    ALOGI("%s:%d " fmt "\n", __func__, __LINE__, ##args)
  #ifdef LOG_DEBUG
  #define CDBG(fmt, args...) \
    ALOGD("%s:%d " fmt "\n", __func__, __LINE__, ##args)
  #else
  #define CDBG(fmt, args...) do {} while(0)
  #endif
#else
  #define CDBG_ERROR(fmt, args...) \
    fprintf(stderr, "%s:%d "fmt"\n", __func__, __LINE__, ##args)
  #define CDBG_INFO(fmt, args...) \
    fprintf(stderr, "%s:%d "fmt"\n", __func__, __LINE__, ##args)
  #ifdef LOG_DEBUG
  #define CDBG(fmt, args...) \
    fprintf(stderr, "%s:%d "fmt"\n", __func__, __LINE__, ##args)
  #else
  #define CDBG(fmt, args...) do {} while(0)
  #endif
#endif

/*===========================================================================
 * Class: QCameraDepthMap
 *
 * Description: This class represents the class of processing DepthMap Algorithm
 *
 * Notes: none
 *==========================================================================*/
class QCameraDepthMap
{
public:
 /**
  * Function: QCameraDepthMap
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
  QCameraDepthMap();

 /**
  * Function: ~QCameraDepthMap
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
  ~QCameraDepthMap();

 /**
  * Function: setDepthMapConfig
  *
  * Description: Sets sensor Config
  *
  * Arguments:
  *   @arg1: img_format
  *   @arg2: mode
  *   @arg3: exp_val
  *   @arg3: fps_range
  *
  * Return values:
  *     none
  *
  * Notes: none
 **/
 void setDepthMapConfig(cam_format_t img_format , cam_ae_mode_type sensor_mode,
   int64_t exp_val,  cam_fps_range_t fps_range);

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
  * Frame ops
  */
 img_frame_ops_t mFrameOps;

private:

 /**
  * parameters for initialization
  */
  int raw_width;
  int raw_height;
  int depth_map_width;
  int depth_map_height;
  cam_format_t sensor_img_format;
  WrapperDepthFormat wrapper_depth_format;
  WrapperDepthExpModeType wrapper_exp_mode;
  WrapperDepthCamFpsRange wrapper_fps_range;
  int64_t wrapper_me_val;

/**
 * parameters for processData
 */
  uint32_t in_image_lenth;
  uint8_t *in_image;
  uint32_t out_image_lenth;
  uint8_t *out_depth_image;
  pc_pkt_t *out_pcloud_image;
  uint32_t count_pt_cloud;
  int64_t  last_ae_val;

/**
 * DepthMapWrapper Handle
 */
  DepthMapWrapper *mpDepthMapWrapper;
};

#endif//__QCAMERA_DEPTHMAP_H__
