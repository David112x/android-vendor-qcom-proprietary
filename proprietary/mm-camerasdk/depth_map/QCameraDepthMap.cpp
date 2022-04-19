/**********************************************************************
*  Copyright (c) 2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#define LOG_TAG "QCameraDepthMap"

#include <system/graphics.h>
#include "QCameraDepthMap.h"


 /**
  * Function: img_algo_init
  *
  * Description: algorirhm initialization
  *
  * Arguments:
  *   @pp_context: return context [Output parameter]
  *   @p_params: init params
  *
  * Return values:
  *     none
  *
  * Notes: none
  **/
extern "C" int img_algo_init(void **p_context, img_init_params_t *p_params)
{
  CDBG(": E");
  int ret = IMG_SUCCESS;
  IMG_UNUSED(p_params);

  QCameraDepthMap *pDepthMap = NULL;
  pDepthMap = new QCameraDepthMap;
  if (pDepthMap) {
    *p_context = pDepthMap;
  } else {
    ret = IMG_ERR_GENERAL;
  }
  CDBG(": X");
  return ret;
}

 /**
  * Function: img_algo_set_frame_ops
  *
  * Description: Function to set frame operations
  *
  * Arguments:
  *   @p_context: context
  *   @p_ops: frame operations pointer
  *
  * Return values:
  *   IMGLIB error codes
  *
  * Notes: none
  **/
 extern "C" int img_algo_set_frame_ops(void *p_context, img_frame_ops_t *p_ops)
 {
   CDBG(": E");
   int ret = IMG_SUCCESS;
   assert(NULL != p_context);
   assert(NULL != p_ops);
   QCameraDepthMap* instance = (QCameraDepthMap*) p_context;
   instance->setFrameOps(p_ops);
   CDBG(": X");
   return ret;
 }

 /**
  * Function: img_algo_set_lib_config
  *
  * Description: Set algo config param
  *
  * Arguments:
  *   @p_context: class pointer
  *   @user_data: Payload data to be set to algo
  *
  * Returns:
  *   IMGLIB return codes
  *
  * Notes: none
  **/
extern "C" int img_algo_set_lib_config(void *p_context, void* user_data)
{
  CDBG(": E");
  int ret = IMG_SUCCESS;

  img_lib_depth_map_config_t *p_lib_config =
    (img_lib_depth_map_config_t *)user_data;

  QCameraDepthMap* pDepthMap = (QCameraDepthMap*) p_context;
  if(!p_lib_config || !p_lib_config ||!pDepthMap) {
    return IMG_ERR_INVALID_INPUT;
  }

  img_lib_param_t  lib_param = p_lib_config->lib_param;
  switch(lib_param) {
  case IMG_ALGO_PRESTART:

    CDBG_ERROR("img_algo_set_lib_config img format =%d, min_fps= %.2f,\
      max_fps=%.2f, video_min_fps= %.2f, video_max_fps = %.2f,\
      exposure mode = %d exposure val = %lld",
      p_lib_config->depth_map_sensor_img_format,
      p_lib_config->depth_map_fps_range.min_fps,
      p_lib_config->depth_map_fps_range.max_fps,
      p_lib_config->depth_map_fps_range.video_min_fps,
      p_lib_config->depth_map_fps_range.video_max_fps,
      p_lib_config->depth_map_exp_mode,
      p_lib_config->depth_map_exp_val);

    pDepthMap->setDepthMapConfig(p_lib_config->depth_map_sensor_img_format,
      p_lib_config->depth_map_exp_mode,
      p_lib_config->depth_map_exp_val,
      p_lib_config->depth_map_fps_range);

    break;
  default:
    ret= IMG_ERR_INVALID_INPUT;
    break;
  }
  CDBG(": X");
  return ret;
}

 /**
  * Function: img_algo_deinit
  *
  * Description: algorithm de-initialization
  *
  * Arguments:
  *   @arg1:
  *
  * Return values:
  *     none
  *
  * Notes: none
 **/
extern "C" int img_algo_deinit(void *p_context)
{
  CDBG(": E");
  assert(NULL != p_context);

  QCameraDepthMap *pDepthMap = (QCameraDepthMap*) p_context;
  delete pDepthMap;
  pDepthMap  = NULL;

  CDBG(": X");
  return IMG_SUCCESS;
}

 /**
  * Function: img_algo_process
  *
  * Description: algorithm main body
  *
  * Arguments:
  *   @arg1: p_context: class pointer
  *   @arg2: p_in_frame: input frame handler
  *   @arg3: in_frame_cnt: number of input frames
  *   @arg4: p_out_frame: output frame handler
  *   @arg5: out_frame_cnt: number of output frames
  *   @arg6: p_meta: frame meta data
  *   @arg7: meta_cnt: number of meta data array
  *
  * Return values:
  *   IMG_SUCCESS or IMG_ERR_GENERAL
  *
  * Notes: none
  **/
extern "C" int img_algo_process(void *p_context, img_frame_t *p_in_frame[],
  int in_frame_cnt,img_frame_t *p_out_frame[],
  int out_frame_cnt, img_meta_t *p_meta[], int meta_cnt)
{
  CDBG("DepthMap ProcessFrame %d, %d", in_frame_cnt, out_frame_cnt);

  CDBG_ERROR(": E");
  int ret = IMG_SUCCESS;
  if (p_context != NULL) {
    QCameraDepthMap *pDepthMap = (QCameraDepthMap*) p_context;
    ret = pDepthMap->processData(p_in_frame, in_frame_cnt, p_out_frame,
      out_frame_cnt, (p_meta && (meta_cnt > 0)) ? p_meta[0] : NULL);
  } else {
    CDBG_ERROR(": img_algo_process failed");
    ret = IMG_ERR_GENERAL;
  }

  CDBG(": X ret %d ", ret );

  return ret;
}

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
QCameraDepthMap::QCameraDepthMap():
  raw_width(-1),
  raw_height(-1),
  depth_map_width(DEPTH_MAP_WIDTH),
  depth_map_height(DEPTH_MAP_HEIGHT),
  wrapper_depth_format(WRAPPER_CAM_FORMAT_NONE),
  wrapper_exp_mode(WRAPPER_EXP_MODE_NONE),
  wrapper_me_val(-1),
  in_image_lenth(-1),
  in_image(NULL),
  out_image_lenth(-1),
  out_depth_image(NULL),
  out_pcloud_image(NULL),
  count_pt_cloud(0),
  last_ae_val(-1),
  mpDepthMapWrapper(NULL)
{
  wrapper_fps_range = {0.0f, 0.0f, 0.0f, 0.0f};
  CDBG(": X");
}

/**
  * Function: setSensorConfig
  *
  * Description: Sets sensor Config
  *
  * Arguments:
  *   @arg1: img_format
  *   @arg2: mode
  *   @arg3: exp_val
  * Return values:
  *     none
  *
  * Notes: none
**/
void QCameraDepthMap::setDepthMapConfig(cam_format_t img_format ,
  cam_ae_mode_type sensor_mode, int64_t exp_val, cam_fps_range_t fps_range) {

  sensor_img_format = img_format;
  switch (sensor_img_format) {
    case CAM_FORMAT_DEPTH8:
      wrapper_depth_format = WRAPPER_CAM_FORMAT_IR;
      break;
    case CAM_FORMAT_DEPTH16:
      wrapper_depth_format = WRAPPER_CAM_FORMAT_DEPTH16;
      break;
    case CAM_FORMAT_DEPTH_POINT_CLOUD:
      wrapper_depth_format = WRAPPER_CAM_FORMAT_DEPTH_POINT_CLOUD;
      break;
    default:
      wrapper_depth_format = WRAPPER_CAM_FORMAT_NONE;
      break;
  }

  switch (sensor_mode) {
    case CAM_AE_MODE_OFF:
      wrapper_exp_mode = WRAPPER_EXP_MODE_ME;
    break;
    case CAM_AE_MODE_ON:
      wrapper_exp_mode = WRAPPER_EXP_MODE_AE;
    break;
    default:
      wrapper_exp_mode = WRAPPER_EXP_MODE_NONE;
    break;
  }

  wrapper_me_val = exp_val;

  wrapper_fps_range.max_fps       = fps_range.max_fps;
  wrapper_fps_range.min_fps       = fps_range.min_fps;
  wrapper_fps_range.video_min_fps = fps_range.video_min_fps;
  wrapper_fps_range.video_max_fps = fps_range.video_max_fps;

  CDBG_ERROR("setDepthMapConfig img format =%d, min_fps= %.2f , max_fps=%.2f,\
    video_min_fps= %.2f, video_max_fps = %.2f, exposure mode = %d \
    exposure val = %lld",
    sensor_img_format,
    wrapper_fps_range.min_fps,
    wrapper_fps_range.max_fps,
    wrapper_fps_range.video_min_fps,
    wrapper_fps_range.video_max_fps,
    wrapper_exp_mode,
    wrapper_me_val);
}

 /**
  * Function: ~QCameraDepthMap
  *
  * Description: destructor of class
  *
  * Arguments:
   *   @arg1:
  *
  * Return values:
  *     none
  *
  * Notes: none
 **/
QCameraDepthMap::~QCameraDepthMap()
{
  CDBG(": E");
  if (mpDepthMapWrapper) {
    delete mpDepthMapWrapper;
    mpDepthMapWrapper = NULL;
  }
  CDBG(": X");
}

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
  *   IMG_SUCCESS, IMG_ERR_GENERAL or IMG_ERR_NOT_SUPPORTED
  *
  * Notes: none
  **/
int8_t QCameraDepthMap::processData(img_frame_t *inFrame[],
  int inFrameCnt, img_frame_t *outFrame[], int outFrameCnt,
  img_meta_t *p_meta)
{
  CDBG(": E");
  int8_t ret = IMG_SUCCESS;
  IMG_UNUSED(inFrameCnt);
  IMG_UNUSED(outFrameCnt);
  IMG_UNUSED(p_meta);
  bool rc = true;
  WrapperDepthCamConfig pDepthMapConfig;
  memset(&pDepthMapConfig, 0x0, sizeof(WrapperDepthCamConfig));

  depth_map_width  = QIMG_WIDTH(outFrame[0], 0);
  depth_map_height = QIMG_HEIGHT(outFrame[0], 0);

  if (!mpDepthMapWrapper) {
    //do init
    raw_width =  QIMG_WIDTH(inFrame[0], 0);
    raw_height = QIMG_HEIGHT(inFrame[0], 0);

    mpDepthMapWrapper = new DepthMapWrapper(raw_width,
      raw_height,
      depth_map_width,
      depth_map_height);

     if (!mpDepthMapWrapper) {
       CDBG_ERROR("DepthMapWrapper: Initialization failed %p", mpDepthMapWrapper);
       ret = IMG_ERR_GENERAL;
       return ret;
    }
    CDBG_ERROR("mpDepthMapWrapper %p", mpDepthMapWrapper);
  }

  int depth_map_frame_cnt =
    inFrame[DEPTH_MAP_INPUT_FRAME_IDX]->frame[0].plane_cnt;

  CDBG_ERROR("plane_cnt %d",depth_map_frame_cnt);

  //inFrame Sanity check
  if (depth_map_frame_cnt > DEPTH_MAP_PLANE_CNT) {
    CDBG_ERROR("plane_cnt > 1 .. Not Raw Frame");
    ret = IMG_ERR_NOT_SUPPORTED;
    return ret;
  }

  in_image =
    inFrame[DEPTH_MAP_INPUT_FRAME_IDX]->frame[0].plane[0].addr;
  out_depth_image =
    outFrame[DEPTH_MAP_INPUT_FRAME_IDX]->frame[0].plane[0].addr;

  switch (wrapper_depth_format) {
    case WRAPPER_CAM_FORMAT_IR:
      out_image_lenth = raw_width*raw_height*sizeof(uint16_t);
      break;
    case WRAPPER_CAM_FORMAT_DEPTH16:
      out_image_lenth = depth_map_width*depth_map_height*sizeof(uint16_t);
      break;
    case WRAPPER_CAM_FORMAT_DEPTH_POINT_CLOUD:
      out_image_lenth = depth_map_width*depth_map_height*sizeof(pc_pkt_t);
      out_pcloud_image =
        (pc_pkt_t *)((android_depth_points *)out_depth_image)->xyzc_points;
      break;
    default:
      break;
  }

  pDepthMapConfig.in_depth_map_exp_mode = wrapper_exp_mode;
  pDepthMapConfig.in_depth_map_fps_range = wrapper_fps_range;
  pDepthMapConfig.in_depth_map_me_val = wrapper_me_val;
  pDepthMapConfig.out_depth_map_ae_val = -1;
  pDepthMapConfig.out_depth_map_laser_strength_val = -1;

  rc = mpDepthMapWrapper->processFrame( wrapper_depth_format,
    (const char *)in_image,
    (char *)out_depth_image,
    out_image_lenth,
    out_pcloud_image,
    &count_pt_cloud,
    &pDepthMapConfig);

  if (wrapper_exp_mode == WRAPPER_EXP_MODE_AE &&
      pDepthMapConfig.out_depth_map_ae_val != -1) {
      if (pDepthMapConfig.out_depth_map_ae_val != last_ae_val) {
        CDBG_ERROR("last_ae_exp_val: %lld current ae_exp_val : %lld", last_ae_val,
         pDepthMapConfig.out_depth_map_ae_val);
         last_ae_val = pDepthMapConfig.out_depth_map_ae_val;
         rc = mFrameOps.get_data_lib(mFrameOps.p_appdata, DEPTH_AE_VAL,
          (void *)&last_ae_val);
         if (false == rc ) {
            ret = IMG_ERR_GENERAL;
            return ret;
         }
      } else {
        CDBG_ERROR("No change in AE val");
      }
  } else {
    CDBG_ERROR("ME mode or AE val not set");
  }

  if (wrapper_depth_format == WRAPPER_CAM_FORMAT_DEPTH_POINT_CLOUD) {
    (( android_depth_points *)out_depth_image)->num_points = count_pt_cloud;
  }

  CDBG(": X ret %d", ret);
  return ret;
}
