/**********************************************************************
* Copyright (c) 2016-2017, 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#define LOG_TAG "QCameraSATDummy"
#include <sys/types.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <assert.h>
#include <errno.h>
#include "QCameraSATDummy.h"

#ifdef _ANDROID_
#include <utils/Log.h>
#include <cutils/properties.h>
#endif

#define TRANSITION_START 1.59
#define TRANSITION_END 2.44

/**
 * Macro to start timer
 */
#define ALGO_START_TIME \
  struct timeval start_time, end_time; \
  if (mPerfMeasure) \
    gettimeofday(&start_time, NULL);  \

/**
 * Macro to get the duration
 */
#define ALGO_GET_TIME ({ \
  uint64_t time = 0ULL; \
  if (mPerfMeasure) { \
    gettimeofday(&end_time, NULL); \
   time = ((end_time.tv_sec * 1000) + (end_time.tv_usec / 1000)) - \
    ((start_time.tv_sec * 1000) + (start_time.tv_usec / 1000)); \
  } \
  time; \
})

/**
 *  dynamic loglevel
 **/
volatile uint32_t g_imgloglevel = 2;
volatile uint32_t g_imgLogModuleMask = 65535;
static int count = 0;

/**
 *   Flag to indicate whether to measure latency
 */
bool QCameraSATDummy::mPerfMeasure;

/**
 *   Dummy test algo called by dummy wrapper for completion,
 *   should not be used.
 */
int32_t DummyAlgo(algo_frame_t input1, algo_frame_t input2,
  algo_frame_t output,
  img_spatial_transform_result_t *p_sat_wide_result,
  img_spatial_transform_result_t *p_sat_tele_result,
  img_meta_t *p_mWideMeta, img_meta_t *p_mTeleMeta,
  uint8_t mWidelpm, uint8_t mTelelpm);

/**
 * Function: img_algo_get_caps
 *
 * Description: This is a query capability function and will be called
 *   before algo init function call. It returns capability of algo like
 *   crop capability, which core the algo will run on, buffer access mode,
 *   and execution mode.
 *
 * Arguments:
 *   @p_ops: base Ops
 *   @p_params: extra params for query capability
 *
 * Return values:
 *   IMG_SUCCESS or IMG_ERR_INVALID_INPUT
 *
 * Notes: p_ops will be NULL as this function is called befor algo init.
 **/
extern "C" int img_algo_get_caps(img_base_ops_t *p_ops, void *p_params)
{
  int32_t rc = IMG_SUCCESS;
  IMG_UNUSED(p_ops);

  if (p_params != NULL) {
    img_algo_caps_t *lib_caps =
      (img_algo_caps_t *)(p_params);

    lib_caps->core_type = IMG_CORE_ARM;
    lib_caps->exec_mode = IMG_EXECUTION_SW;
    lib_caps->buff_access_mode = IMG_ACCESS_READ_WRITE;
    lib_caps->crop_caps = IMG_CROP_APPLIED;
  } else {
    rc = IMG_ERR_INVALID_INPUT;
    IDBG_ERROR("Invalid input");
  }
  return rc;
}

/**
 * Function: img_algo_preload
 *
 * Description: This is a preload function and will be called
 * before Init process. Wrapper can take advantage of this
 * function to pre-allocate resources and get algo capabilities.
 *
 * Arguments:
 *   @p_ops: base Ops
 *   @p_params: extra params for preload
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_preload(img_base_ops_t *p_ops, void *p_params)
{
  IMG_UNUSED(p_ops);
  IMG_UNUSED(p_params);
  int32_t rc = IMG_SUCCESS;
  IDBG_MED("E");

  IDBG_MED("rc %d X", rc);
  return rc;
}

/**
 * Function: img_algo_shutdown
 *
 * Description: algorithm cleanup of pre-allocated resources
 *
 * Arguments:
 *   @p_ops: base Ops
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_shutdown(img_base_ops_t *p_ops)
{
  IMG_UNUSED(p_ops);
  int32_t rc = IMG_SUCCESS;
  IDBG_MED("E");

  IDBG_MED("rc %d X", rc);
  return rc;
}

/**
 * Function: img_algo_init
 *
 * Description: algorithm initialization
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
extern "C" int img_algo_init(void **p_context,
  img_multicam_init_params_t *p_params)
{
  QCameraSATDummy *SATDummy;
  int rc = IMG_SUCCESS;

  IDBG_MED("E");
  if (p_context == NULL || p_params == NULL) {
    IDBG_ERROR("Error invalid input %p %p", p_context, p_params);
    rc = IMG_ERR_INVALID_INPUT;
    goto end;
  }

  SATDummy = new QCameraSATDummy;
  if (!SATDummy) {
    IDBG_ERROR("Cannot create st dummy object");
    rc = IMG_ERR_NO_MEMORY;
    goto end;
  }

  rc = SATDummy->Init(p_params);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Algo init falied");
    delete SATDummy;
    SATDummy = NULL;
  }

  *p_context = SATDummy;

end:
  IDBG_MED("rc %d X", rc);
  return rc;
}

/**
 * Function: img_algo_deinit
 *
 * Description: algorithm de-initialization
 *
 * Arguments:
 *   @p_context: pointer to the context
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_deinit(void *p_context)
{
  IDBG_MED("E");
  if (p_context != NULL) {
    QCameraSATDummy* SATDummy =
      (QCameraSATDummy*) p_context;
    SATDummy->DeInit();
    delete SATDummy;
  }

  IDBG_MED("X");
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
extern "C" int img_algo_process(void *p_context,
  img_frame_t *p_in_frame[],
  int in_frame_cnt,
  img_frame_t *p_out_frame[],
  int out_frame_cnt,
  img_meta_t *p_meta[],
  int meta_cnt)
{
  int rc = IMG_SUCCESS;

  IDBG_HIGH("E");
  if (p_context != NULL) {
    QCameraSATDummy* SATDummy =
      (QCameraSATDummy*) p_context;

    rc = SATDummy->Execute(p_in_frame,
      in_frame_cnt,
      p_out_frame,
      out_frame_cnt,
      p_meta,
      meta_cnt);
  } else {
    IDBG_ERROR("Error invalid context");
    rc = IMG_ERR_INVALID_INPUT;
  }

  IDBG_HIGH("rc %d X", rc);
  return rc;
}

/**
 * Function: img_algo_set_frame_ops
 *
 * Description: Function to set frame operations
 *
 * Arguments:
 *   none
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_set_frame_ops(void *p_context, img_frame_ops_t *p_ops)
{
  int rc = IMG_SUCCESS;

  IDBG_MED("E");
  if (p_context != NULL && p_ops != NULL) {
    QCameraSATDummy* SATDummy =
      (QCameraSATDummy*) p_context;

    SATDummy->SetFrameOps(p_ops);
  } else {
    IDBG_ERROR("Error invalid input %p %p", p_context, p_ops);
    rc = IMG_ERR_INVALID_INPUT;
  }

  IDBG_MED("rc %d X", rc);
  return rc;
}

/**
 * Function: img_algo_get_lib_config
 *
 * Description: algorirhm get config
 *
 * Arguments:
 *   @p_context: lib context
 *   @p_data: data for config
 *
 * Return values:
 *     IMG_SUCCESS or IMG_ERR_
 *
 * Notes: This function will be called before wrapper init hence
 * p_context will be NULL. Please do not use p_context.
 **/
extern "C" int img_algo_get_lib_config(void *p_context, void *p_data)
{
  IMG_UNUSED(p_context);
  img_lib_config_t *p_lib_config = (img_lib_config_t *)p_data;
  img_lib_param_t lib_param_type;
  int32_t lib_ret = IMG_SUCCESS;

  if(!p_lib_config || !p_lib_config->lib_data) {
    IDBG_ERROR("Invalid input %p ", p_lib_config);
    return IMG_ERR_INVALID_INPUT;
  }

  lib_param_type = p_lib_config->lib_param;
  switch (lib_param_type) {
  case IMG_ALGO_IN_FRAME_DIM: {
    img_multicam_dim_info_t *p_dim_info =
      (img_multicam_dim_info_t *) p_lib_config->lib_data;
    if (p_dim_info->output_size.width >= 3840 ||
      p_dim_info->output_size.width >= 2160) {
      p_dim_info->input_margin.w_margin = 0.0;
      p_dim_info->input_margin.h_margin = 0.0;
    } else {
      p_dim_info->input_margin.w_margin = 0.3;
      p_dim_info->input_margin.h_margin = 0.3;
    }
    p_dim_info->input_size.width = p_dim_info->output_size.width +
      (p_dim_info->output_size.width * p_dim_info->input_margin.w_margin);
    p_dim_info->input_size.height = p_dim_info->output_size.height +
      (p_dim_info->output_size.height * p_dim_info->input_margin.h_margin);
    p_dim_info->input_size.stride = p_dim_info->input_size.width;
    p_dim_info->input_size.scanline = p_dim_info->input_size.height;
  }
  break;
  default:
    IDBG_ERROR("Invalid param type");
    lib_ret = IMG_ERR_INVALID_INPUT;
  }

  return lib_ret;
}

/**
 * Function: FillAlgoInitCfg
 *
 * Description: Fill the algo initalization params.
 *
 * Arguments:
 *   @p_aImglibParams: Imagelib init parameters
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void QCameraSATDummy::FillAlgoInitCfg(img_multicam_init_params_t *p_aParams)
{
  uint32_t i =0;
  img_multicam_dim_info_t multicam_dim_info;
  for (i = 0; i < p_aParams->num_of_sessions; i++) {
  multicam_dim_info = p_aParams->multicam_dim_info[i];
  IDBG_MED("cam role %d, Input: wxh %dx%d stxsl %dx%d,"
    " Output: wxh %dx%d stxsl %dx%d",
    multicam_dim_info.cam_role,
    multicam_dim_info.input_size.width,
    multicam_dim_info.input_size.height,
    multicam_dim_info.input_size.stride,
    multicam_dim_info.input_size.scanline,
    multicam_dim_info.output_size.width,
    multicam_dim_info.output_size.height,
    multicam_dim_info.output_size.stride,
    multicam_dim_info.output_size.scanline);
  }

/* Fill algo init params here*/
}

/**
 * Function: FillAlgoFrameCfg
 *
 * Description: fill the algo frame configuration
 *
 * Arguments:
 *   @aImglibFrame: Imagelib frame
 *   @aType: Frame type
 *
 * Return values:
 *   IMG_SUCCESS or IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t QCameraSATDummy::FillAlgoFrameCfg(img_frame_t *p_aImgFrame,
  algo_frame_type_t aType)
{
  algo_frame_t *pAlgoFrame;
  switch (aType) {
  case WIDE_FRAME:
    pAlgoFrame = &mAlgoFrameSet.algoInWide;
    break;
  case TELE_FRAME:
    pAlgoFrame = &mAlgoFrameSet.algoInTele;
    break;
  case OUT_FRAME:
    pAlgoFrame = &mAlgoFrameSet.algoOutput;
    break;
  default:
    IDBG_ERROR("Error invalid frame type");
    return IMG_ERR_INVALID_INPUT;
  }

  /* Retreve frame information */
  pAlgoFrame->frameid = p_aImgFrame->frame_id;
  pAlgoFrame->vaddr = QIMG_ADDR(p_aImgFrame, 0);
  pAlgoFrame->fd = QIMG_FD(p_aImgFrame, 0);
  pAlgoFrame->width = QIMG_WIDTH(p_aImgFrame, 0);
  pAlgoFrame->height = QIMG_HEIGHT(p_aImgFrame, 0);
  pAlgoFrame->stride = QIMG_STRIDE(p_aImgFrame, 0);
  pAlgoFrame->scanline = QIMG_SCANLINE(p_aImgFrame, 0);

  IDBG_INFO("frame %d addr %p  fd %d wxh %dx%d stxsl %dx%d",
    aType, pAlgoFrame->vaddr, pAlgoFrame->fd,
    pAlgoFrame->width, pAlgoFrame->height,
    pAlgoFrame->stride, pAlgoFrame->scanline);

  return IMG_SUCCESS;
}

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
int32_t QCameraSATDummy::FillAlgoMetaCfg(img_meta_t *p_aMeta, uint8_t aIdx)
{
  /* Save imglib meta pointer to write algo output info*/
  if (!p_aMeta) {
    return IMG_SUCCESS;
  }
  switch (p_aMeta->qdcc_cfg.sat_cfg.camera_role) {
  case IMG_CAM_ROLE_WIDE:
    mWidelpm = p_aMeta->qdcc_cfg.sat_cfg.low_power_mode;
    p_mWideMeta = p_aMeta;
    mWideIdx = aIdx;
    break;
  case IMG_CAM_ROLE_TELE:
    mTelelpm = p_aMeta->qdcc_cfg.sat_cfg.low_power_mode;
    p_mTeleMeta = p_aMeta;
    mTeleIdx = aIdx;

    break;
  default:
    IDBG_ERROR("Error invalid type %d", p_aMeta->qdcc_cfg.sat_cfg.camera_role);
    return IMG_ERR_INVALID_INPUT;
  }

  /* Wrapper can retrive following information from imglib meta struct
   *   AEC info              : p_aMeta->aec_info;
   *   AWB info              : p_aMeta->awb_info;
   *   AF info               : p_aMeta->af_info;
   *   Zoom factor           : p_aMeta->zoom_factor
   *   Per module FOV params : p_aMeta->sat_cfg.fov_params[IMG_MAX_FOV]
   *     here IMG_MAX_FOV = 4; 0: Sensor, 1: IFACE, 2:CAMIF, 3:ISP
   *   Frame rate            : p_aMeta->sat_cfg.frameRate
   */
  PrintMetaInfo(p_aMeta);
  return IMG_SUCCESS;
}

/**
 * Function: FillAlgoOutputInfo
 *
 * Description: Return algo output info by filling imglib meta
 * out params
 *
 * Arguments:
 *   @aImglibMeta: Imglib meta
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void QCameraSATDummy::FillAlgoOutputInfo(img_meta_t *p_aMeta)
{
  IMG_UNUSED(p_aMeta);
  /* Fill algo output info to following out params
   *   p_aMeta->qdcc_result.sat_result.camera_role;
   *   p_aMeta->qdcc_result.sat_result.output_shift;
   */
}

/**
 * Function: MapToImgError
 *
 * Description: Mapping error from algorithm to imglib errors
 *
 * Arguments:
 *   @aAlgoError: algorithm error
 *
 * Return values:
 *     imaging errors
 *
 * Notes: none
 **/
int32_t QCameraSATDummy::MapToImgError(int32_t aAlgoError)
{
  int32_t rc;
  switch (aAlgoError) {
  case 0:
    rc = IMG_SUCCESS;
    break;
  default:
    rc = IMG_ERR_GENERAL;
  }
  return rc;
}

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
void QCameraSATDummy::PrintMetaInfo(img_meta_t *p_aMeta)
{
  uint8_t i;
  IDBG_LOW("Meta for camera role %d", p_aMeta->qdcc_cfg.sat_cfg.camera_role);
  IDBG_LOW("frame id %d", p_aMeta->frame_id);
  IDBG_LOW("Frame rate %f", p_aMeta->qdcc_cfg.sat_cfg.frameRate);
  IDBG_LOW("zoom factor %f", p_aMeta->zoom_factor);

  for (i = 0; i < IMG_MAX_FOV; i++) {
    img_fov_t fov = p_aMeta->qdcc_cfg.sat_cfg.fov_params[i];
    IDBG_LOW("Module %d: input_width %d", fov.module, fov.input_width);
    IDBG_LOW("Module %d: input_height %d", fov.module, fov.input_height);
    IDBG_LOW("Module %d: offset_x %d", fov.module, fov.offset_x);
    IDBG_LOW("Module %d: offset_y %d", fov.module, fov.offset_y);
    IDBG_LOW("Module %d: fetch_window_width %d", fov.module,
      fov.fetch_window_width);
    IDBG_LOW("Module %d: fetch_window_height %d", fov.module,
      fov.fetch_window_height);
    IDBG_LOW("Module %d: offset_y %d", fov.module, fov.output_window_width);
    IDBG_LOW("Module %d: offset_x %d", fov.module, fov.output_window_height);
  }

  IDBG_LOW("AEC info: aec_status %d", p_aMeta->aec_info.aec_status);
  IDBG_LOW("AEC info: exp_time %f", p_aMeta->aec_info.exp_time);
  IDBG_LOW("AEC info: ios %d", p_aMeta->aec_info.iso);
  IDBG_LOW("AEC info: linecount %d", p_aMeta->aec_info.linecount);
  IDBG_LOW("AEC info: lux_idx %f", p_aMeta->aec_info.lux_idx);
  IDBG_LOW("AEC info: real_gain %f", p_aMeta->aec_info.real_gain);

  IDBG_LOW("AWB info: awb_status %d", p_aMeta->awb_info.awb_status);
  IDBG_LOW("AWB info: r_gain %f", p_aMeta->awb_info.r_gain);
  IDBG_LOW("AWB info: g_gain %f", p_aMeta->awb_info.g_gain);
  IDBG_LOW("AWB info: b_gain %f", p_aMeta->awb_info.b_gain);
  IDBG_LOW("AWB info: ccm %p", p_aMeta->awb_info.ccm);
  IDBG_LOW("AWB info: ccm_size %d", p_aMeta->awb_info.ccm_size);
  IDBG_LOW("AWB info: color_temp %d", p_aMeta->awb_info.color_temp);

  IDBG_LOW("AF info: af_status %d", p_aMeta->af_info.af_status);
  IDBG_LOW("AF info: far_field %f", p_aMeta->af_info.far_field);
  IDBG_LOW("AF info: roi x %d", p_aMeta->af_info.focused_roi.pos.x);
  IDBG_LOW("AF info: roi y %d", p_aMeta->af_info.focused_roi.pos.y);
  IDBG_LOW("AF info: roi width %d", p_aMeta->af_info.focused_roi.size.width);
  IDBG_LOW("AF info: roi height %d", p_aMeta->af_info.focused_roi.size.height);
  IDBG_LOW("AF info: lens_shift %f", p_aMeta->af_info.lens_shift);
  IDBG_LOW("AF info: near_field %f", p_aMeta->af_info.near_field);
  IDBG_LOW("AF info: object_distance %f", p_aMeta->af_info.object_distance);
}

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
int32_t QCameraSATDummy::Init(img_multicam_init_params_t *p_params)
{
  int rc = IMG_SUCCESS;

  /* Fill algorithum init config*/
  FillAlgoInitCfg(p_params);

  /* call algo init here */

  return rc;
}

/**
 * Function: Deinitialize
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
int32_t QCameraSATDummy::DeInit()
{
  int rc = IMG_SUCCESS;

  /* call algo deinit here */

  return rc;
}

/**
 * Function: QCameraSATDummy
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
QCameraSATDummy::QCameraSATDummy()
{
  IDBG_LOW("E");
  memset(&mFrameOps, 0x0, sizeof(mFrameOps));
  memset(&mAlgoFrameSet, 0x0, sizeof(algo_frame_set_t));
  p_mWideMeta = NULL;
  p_mTeleMeta = NULL;
  IDBG_LOW("X");
}

/**
 * Function: ~QCameraSATDummy
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
QCameraSATDummy::~QCameraSATDummy()
{
  IDBG_LOW("E");
  IDBG_LOW("X");
}

/**
 * Function: Execute
 *
 * Description: main processing body
 *
 * Arguments:
 *   @aInFrame: input frame handler
 *   @aInFrameCnt: number of input frames
 *   @aOutFrame: output frame handler
 *   @aOutFrameCnt: number of output frames
 *   @aMeta: frame meta data
 *   @aMetaCount: number of meta
 *
 * Return values:
 *   IMG_SUCCESS or IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t QCameraSATDummy::Execute(img_frame_t *aInFrame[],
  int aInFrameCnt,
  img_frame_t *aOutFrame[],
  int aOutFrameCnt,
  img_meta_t* aMeta[],
  int aMetaCnt)
{
  int i = 0;
  algo_frame_t *p_algoInWide;
  algo_frame_t *p_algoInTele;
  algo_frame_t *p_algoOut;
  img_spatial_transform_result_t lSatWideResult;
  img_spatial_transform_result_t lSatTeleResult;
  int32_t rc = IMG_SUCCESS;

  IDBG_MED("E");
  IMG_UNUSED(aInFrameCnt);
  IMG_UNUSED(aOutFrameCnt);
#if 0
  if (aInFrameCnt != ALGO_NUM_INPUT || aInFrame == NULL ||
    aOutFrameCnt != ALGO_NUM_OUTPUT || aOutFrame == NULL ||
    aMetaCnt != ALGO_NUM_META || aMeta == NULL) {
    IDBG_ERROR("Error invalid input In %p %d Out %p %d Meta %p %d",
      aInFrame, aInFrameCnt, aOutFrame, aOutFrameCnt, aMeta, aMetaCnt);
    rc = IMG_ERR_INVALID_INPUT;
    return rc;
  }

  if (aInFrame[0] == NULL || aInFrame[1] == NULL) {
    IDBG_ERROR("Error NULL pointer, Input frame %p %p",
      aInFrame[0], aInFrame[1]);
    return IMG_ERR_INVALID_INPUT;
  }

  if (aOutFrame[0] == NULL) {
    IDBG_ERROR("Error NULL pointer, Output frame %p", aOutFrame);
    return IMG_ERR_INVALID_INPUT;
  }

  if (aMeta[0] == NULL || aMeta[1] == NULL) {
    IDBG_ERROR("Error NULL pointer, Meta %p %p",
      aMeta[0], aMeta[1]);
    return IMG_ERR_INVALID_INPUT;
  }
#endif
  memset(&mAlgoFrameSet, 0x0, sizeof(algo_frame_set_t));
  memset(&lSatWideResult, 0x0, sizeof(img_spatial_transform_result_t));
  memset(&lSatTeleResult, 0x0, sizeof(img_spatial_transform_result_t));

  /* configre meta data for wide and tele frames*/
  IDBG_INFO("Meta count %d", aMetaCnt);
  for (i = 0; i < aMetaCnt; i++) {
    rc = FillAlgoMetaCfg(aMeta[i], i);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Error during meta config");
      return rc;
    }
  }


  if (aMetaCnt == 1) {
    if (aMeta[0]->qdcc_cfg.sat_cfg.camera_role == IMG_CAM_ROLE_WIDE)
        mTelelpm = 1;
    else
        mWidelpm = 1;
  }
  IDBG_INFO("Meta count %d mtelelpm %d widelpm %d p_mTeleMeta %p p_mWideMeta %p ",
    aMetaCnt, mTelelpm, mWidelpm, p_mTeleMeta, p_mWideMeta);

  /* configure Wide frame */
  if (!mWidelpm) {
    rc = FillAlgoFrameCfg(aInFrame[mWideIdx], WIDE_FRAME);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Error during Wide frame confg ");
      return rc;
    }
  }

  /* configure Tele frame */
  if (!mTelelpm) {
    rc = FillAlgoFrameCfg(aInFrame[mTeleIdx], TELE_FRAME);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Error during Tele frame config");
      return rc;
    }
  }

  /* configure output frame */
  rc = FillAlgoFrameCfg(aOutFrame[0], OUT_FRAME);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Error during Output frame config");
    return rc;
  }

  /* Call dummy algo process function*/
  ALGO_START_TIME;
  IDBG_MED("Start Copy");
  rc = DummyAlgo(mAlgoFrameSet.algoInWide, mAlgoFrameSet.algoInTele,
    mAlgoFrameSet.algoOutput, &lSatWideResult, &lSatTeleResult, p_mWideMeta,
    p_mTeleMeta, mWidelpm, mTelelpm);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Error during DummyAlgo config");
    return rc;
  }

  IDBG_MED("End Copy");
  ALGO_GET_TIME;

  /* return algo output info for wide frames and tele to imglib in metadata */
  if (!mWidelpm) {
    rc = mFrameOps.set_meta(p_mWideMeta, IMG_META_SPATIAL_TRANSFORM_RESULT,
      (void *)&lSatWideResult);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Set Meta for wide failed");
      return rc;
    }
  }

  if (!mTelelpm) {
    rc = mFrameOps.set_meta(p_mTeleMeta, IMG_META_SPATIAL_TRANSFORM_RESULT,
      (void *)&lSatTeleResult);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Set Meta for tele failed");
      return rc;
    }
  }

  IDBG_MED("X");
  return IMG_SUCCESS;
}

/************************ DUMMY TEST ALGO **************************/
/* This is just a dummy test code that was used for the completion
   of this wrapper, please use do not use this code. Instead call
   spatial transform solution library APIs*/

/**
 * Function: LineCopyImageBuf
 *
 * Description: Dummy algo helper function
 *
 * Arguments:
 *   none
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void LineCopyImageBuf(uint8_t *a_pSrc, uint8_t *a_pDst, uint32_t w_off,
  uint32_t h_off, uint32_t aCpyWidth, uint32_t aCpyHeight,
  uint32_t aSrcStride, uint32_t aDstStride)
{
  uint32_t i;
  uint8_t *l_pSrc = a_pSrc + (aSrcStride * h_off) + w_off;
  uint8_t *l_pDst = a_pDst;
  for (i = 0; i < aCpyHeight; i++) {
    memcpy(l_pDst, l_pSrc, aCpyWidth);
    l_pDst += aDstStride;
    l_pSrc += aSrcStride;
  }
}

/**
 * Function: LineCopyImageBuf
 *
 * Description: Dummy algo, copies input buffer to output buffer
 *
 * Arguments:
 *   none
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
int32_t DummyAlgo(algo_frame_t input1, algo_frame_t input2,
  algo_frame_t output,
  img_spatial_transform_result_t *p_sat_wide_result,
  img_spatial_transform_result_t *p_sat_tele_result,
  img_meta_t *p_mWideMeta, img_meta_t *p_mTeleMeta,
  uint8_t mWidelpm, uint8_t mTelelpm)
{
  uint32_t in_cr_offset = 0;
  uint32_t out_cr_offset = 0;
  uint32_t w_offset = 0;
  uint32_t h_offset = 0;
  float user_zoom = 1.0;
  uint32_t wide_fallback_enable = 0;

  IDBG_MED("p_mWideMeta=%p, p_mTeleMeta=%p, mWidelpm=%d, mTelelpm=%d",
    p_mWideMeta, p_mTeleMeta, mWidelpm, mTelelpm);

  if (mWidelpm || mTelelpm) {
    if (mWidelpm && !p_mTeleMeta) {
      IDBG_ERROR("WideMeta is %p while tele %p lpm", p_mWideMeta, p_mTeleMeta);
      return IMG_ERR_GENERAL;
    } else if (mTelelpm && !p_mWideMeta) {
      IDBG_ERROR("TeleMeta is %p while wide %p lpm", p_mWideMeta, p_mWideMeta);
      return IMG_ERR_GENERAL;
    }
  } else if (!p_mWideMeta || !p_mTeleMeta) {
    IDBG_ERROR("Transition mode :WideMeta %p or TeleMeta %p are NULL",
      p_mWideMeta,p_mTeleMeta);
    return IMG_ERR_GENERAL;
  }

  if (p_mWideMeta) {
   p_sat_wide_result->is_master_flag_valid = 1;
   p_sat_wide_result->is_master = p_mWideMeta->qdcc_cfg.sat_cfg.is_master;
  }

  if (p_mTeleMeta) {
   p_sat_tele_result->is_master_flag_valid = 1;
   p_sat_tele_result->is_master = p_mTeleMeta->qdcc_cfg.sat_cfg.is_master;
  }

  //LPM ON
  if (mWidelpm || mTelelpm) {
    if (mWidelpm && p_mTeleMeta->qdcc_cfg.sat_cfg.is_master) {
      p_sat_tele_result->is_camera_role_valid = true;
      p_sat_tele_result->camera_role = IMG_CAM_ROLE_TELE;
      p_sat_wide_result->is_camera_role_valid = true;
      p_sat_wide_result->camera_role = IMG_CAM_ROLE_TELE;

      user_zoom = p_mTeleMeta->user_zoom_factor;
      p_sat_tele_result->is_lpm_cmd_valid = TRUE;
      p_sat_tele_result->lpm_cmd[1].camera_role = IMG_CAM_ROLE_TELE;
      p_sat_tele_result->lpm_cmd[1].low_power_mode =  0;
      p_sat_tele_result->lpm_cmd[0].camera_role = IMG_CAM_ROLE_WIDE;
      p_sat_tele_result->lpm_cmd[0].low_power_mode =  1;
      IDBG_INFO("wide lpm on");
      if (user_zoom <= TRANSITION_END) {
        p_sat_tele_result->lpm_cmd[0].low_power_mode =  0;
        IDBG_INFO("request wide lpm off");
      } else if (user_zoom <= TRANSITION_START) {
        IDBG_HIGH("wide fallback enable in widelpm %d telelpm %d", mWidelpm, mTelelpm);
        wide_fallback_enable = 1;
      }
    } else if(mTelelpm && p_mWideMeta->qdcc_cfg.sat_cfg.is_master){
      p_sat_wide_result->is_camera_role_valid = true;
      p_sat_wide_result->camera_role = IMG_CAM_ROLE_WIDE;
      p_sat_tele_result->is_camera_role_valid = true;
      p_sat_tele_result->camera_role = IMG_CAM_ROLE_WIDE;
      user_zoom = p_mWideMeta->user_zoom_factor;
      p_sat_wide_result->is_lpm_cmd_valid = TRUE;
      p_sat_wide_result->lpm_cmd[0].camera_role = IMG_CAM_ROLE_WIDE;
      p_sat_wide_result->lpm_cmd[0].low_power_mode = 0;
      p_sat_wide_result->lpm_cmd[1].camera_role = IMG_CAM_ROLE_TELE;
      p_sat_wide_result->lpm_cmd[1].low_power_mode = 1;
      IDBG_INFO("tele lpm on");
      if (user_zoom >= TRANSITION_START) {
        p_sat_wide_result->lpm_cmd[1].low_power_mode = 0;
        IDBG_INFO("request tele lpm off");
      }
    }
  } else {
     //Transition zone
    p_sat_wide_result->is_camera_role_valid = true;
    p_sat_tele_result->is_camera_role_valid = true;
    if (p_mWideMeta->qdcc_cfg.sat_cfg.is_master) {
      p_sat_wide_result->camera_role = IMG_CAM_ROLE_WIDE;
      p_sat_tele_result->camera_role = IMG_CAM_ROLE_WIDE;
      user_zoom = p_mWideMeta->user_zoom_factor;
    } else {
      p_sat_wide_result->camera_role = IMG_CAM_ROLE_TELE;
      p_sat_tele_result->camera_role = IMG_CAM_ROLE_TELE;
      user_zoom = p_mTeleMeta->user_zoom_factor;
    }

    p_sat_tele_result->is_lpm_cmd_valid = TRUE;
    p_sat_tele_result->lpm_cmd[0].camera_role = IMG_CAM_ROLE_WIDE;
    p_sat_tele_result->lpm_cmd[0].low_power_mode = 0;
    p_sat_tele_result->lpm_cmd[1].camera_role = IMG_CAM_ROLE_TELE;
    p_sat_tele_result->lpm_cmd[1].low_power_mode = 0;

    p_sat_wide_result->is_lpm_cmd_valid = TRUE;
    p_sat_wide_result->lpm_cmd[0].camera_role = IMG_CAM_ROLE_WIDE;
    p_sat_wide_result->lpm_cmd[0].low_power_mode = 0;
    p_sat_wide_result->lpm_cmd[1].camera_role = IMG_CAM_ROLE_TELE;
    p_sat_wide_result->lpm_cmd[1].low_power_mode = 0;

    if (user_zoom >= TRANSITION_END) {
      p_sat_tele_result->lpm_cmd[0].camera_role = IMG_CAM_ROLE_WIDE;
      p_sat_tele_result->lpm_cmd[0].low_power_mode = 1;
      p_sat_wide_result->lpm_cmd[0].camera_role = IMG_CAM_ROLE_WIDE;
      p_sat_wide_result->lpm_cmd[0].low_power_mode = 1;
      IDBG_INFO("Request wide lpm on");
    }

    if (user_zoom <= TRANSITION_START) {
      p_sat_tele_result->lpm_cmd[1].low_power_mode = 1;
      p_sat_wide_result->lpm_cmd[1].low_power_mode = 1;
      IDBG_INFO("Request tele lpm on");
    }

    if (p_mWideMeta->qdcc_cfg.sat_cfg.is_master &&
      p_mTeleMeta->qdcc_cfg.sat_cfg.is_master) {
      IDBG_ERROR ("Both Wide/Tele as Master Invalid scenario");
      return IMG_ERR_FRAME_DROP;
    } else if (!p_mWideMeta->qdcc_cfg.sat_cfg.is_master &&
      !p_mTeleMeta->qdcc_cfg.sat_cfg.is_master) {
      IDBG_ERROR ("Both Wide/Tele are not Master Invalid scenario");
      return IMG_ERR_FRAME_DROP;
    }
  }

  if (user_zoom > 2.3) {
    p_sat_wide_result->is_recommended_role_valid = TRUE;
    p_sat_wide_result->recommended_role = IMG_CAM_ROLE_TELE;
    p_sat_tele_result->is_recommended_role_valid = TRUE;
    p_sat_tele_result->recommended_role = IMG_CAM_ROLE_TELE;
    IDBG_INFO("Zoom on, Hinting cam role change to %d",
      p_sat_wide_result->recommended_role);
  } else {
    if (wide_fallback_enable) {
      p_sat_wide_result->is_recommended_role_valid = TRUE;
      p_sat_wide_result->recommended_role = IMG_CAM_ROLE_WIDE_FALLBACK;
      p_sat_tele_result->is_recommended_role_valid = TRUE;
      p_sat_tele_result->recommended_role = IMG_CAM_ROLE_WIDE_FALLBACK;
      IDBG_INFO("Hinting cam role change to %d",
        p_sat_wide_result->recommended_role);
    } else {
      p_sat_wide_result->is_recommended_role_valid = TRUE;
      p_sat_wide_result->recommended_role = IMG_CAM_ROLE_WIDE;
      p_sat_tele_result->is_recommended_role_valid = TRUE;
      p_sat_tele_result->recommended_role = IMG_CAM_ROLE_WIDE;
      IDBG_INFO("Hinting cam role change to %d",
        p_sat_wide_result->recommended_role);
    }
  }

  switch (p_sat_wide_result->camera_role) {
  case IMG_CAM_ROLE_WIDE:
    if (input1.width > output.width) {
      w_offset = (input1.width - output.width) / 2;
      if (w_offset%2 == 1) {
        w_offset += 1;
        IDBG_LOW(" Make w_offset even no:%d ", w_offset);
      }
    }
    if (input1.height > output.height) {
      h_offset = (input1.height - output.height) / 2;
      if (h_offset%2 == 1) {
        h_offset += 1;
        IDBG_LOW(" Make h_offset even no:%d ", h_offset);
      }
    }

    LineCopyImageBuf((uint8_t *)input1.vaddr ,
      (uint8_t*)output.vaddr, w_offset, h_offset,
      output.width, output.height, input1.stride,
      output.stride);

    in_cr_offset = input1.stride * input1.scanline;
    out_cr_offset = output.stride * output.scanline;

    LineCopyImageBuf((uint8_t *)input1.vaddr + in_cr_offset,
      (uint8_t*)output.vaddr + out_cr_offset, w_offset, h_offset / 2,
      output.width, output.height / 2, input1.stride, output.stride);
    IDBG_INFO("Chosen frame/role %d, zoom factor %f",
      p_sat_wide_result->camera_role, user_zoom);
    break;

  case IMG_CAM_ROLE_TELE:
    if (input2.width > output.width) {
      w_offset = (input2.width - output.width) / 2;
      if (w_offset%2 == 1) {
        w_offset += 1;
        IDBG_LOW(" Make w_offset even no:%d ", w_offset);
      }
    }
    if (input2.height > output.height) {
      h_offset = (input2.height - output.height) / 2;
      if (h_offset%2 == 1) {
        h_offset += 1;
        IDBG_LOW(" Make h_offset even no:%d ", h_offset);
      }
    }
    LineCopyImageBuf((uint8_t *)input2.vaddr ,
      (uint8_t*)output.vaddr, w_offset, h_offset,
      output.width, output.height, input2.stride,
      output.stride);

    in_cr_offset = input2.stride * input2.scanline;
    out_cr_offset = output.stride * output.scanline;

    LineCopyImageBuf((uint8_t *)input2.vaddr + in_cr_offset,
      (uint8_t*)output.vaddr + out_cr_offset, w_offset, h_offset / 2,
      output.width, output.height / 2, input2.stride, output.stride);

    IDBG_INFO("Chosen frame/role %d, zoom factor %f",
      p_sat_tele_result->camera_role, user_zoom);
    break;

  default:
    IDBG_ERROR("Error invalid camera role %d", p_sat_wide_result->camera_role);
  }
  return IMG_SUCCESS;
}

/************************ DUMMY TEST ALGO **************************/
