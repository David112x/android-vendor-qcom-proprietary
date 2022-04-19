/***************************************************************************
* Copyright (c) 2017-2019 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

extern "C" {
#include "img_common.h"
#include "img_mem_ops.h"
#include "img_buffer.h"
}

#ifdef _ANDROID_
#include <utils/Log.h>
#include <cutils/properties.h>
#endif

#include "img_dbg.h"
#include "QCameraBokeh.h"
#include "QCameraBufferMap.h"

#ifdef ENABLE_QC_BOKEH
#include "videobokeh_wrapper.h"
#endif

#include "RenderTexture.h"
#include "Effects.h"
#include "SpatiallyVaryingBlur.h"
#include "Texture.h"

#define DEPTHMAP_LATENCY 65
#define BOKEH_MONO_RESOLUTION_SUPPORTED 3
pthread_mutex_t QCameraBokeh::m_lock = PTHREAD_MUTEX_INITIALIZER;

enum slave_aspect_ratio{
  RATIO_4_3,
  RATIO_16_9,
  RATIO_3_2
};

slave_bokeh_dim mono_bokeh_dim[BOKEH_MONO_RESOLUTION_SUPPORTED] = {
  {640,480},
  {1280,720},
  {480,320}
};

slave_bokeh_dim wide_bokeh_dim[BOKEH_MONO_RESOLUTION_SUPPORTED] = {
  {640,480},
  {1280,720},
  {480,320}
};

EGLConfig GetEglConfig(EGLDisplay display,
                       const EGLint* attrib_list,
                       Config* cfg)
{
  EGLConfig config = NULL;
  EGLConfig* configs;
  EGLint numConfigs;
  int i;

  eglChooseConfig(display, attrib_list, NULL, 0, &numConfigs);
  configs = (EGLConfig*)malloc(sizeof(EGLConfig) * numConfigs);
  if (NULL == configs)
  {
    IDBG_ERROR("Failed to allocate memory for egl config");
    return 0;
  }

  eglChooseConfig(display, attrib_list, configs, numConfigs, &numConfigs);
  // parse all returned configurations and select the one that we really want
  for ( i = 0; i < numConfigs; ++i )
  {
    int red;
    int green;
    int blue;
    int alpha;
    int depth;
    int stencil;
    int sampleBuffers;
    int samples;
    eglGetConfigAttrib(display, configs[i], EGL_RED_SIZE, &red);
    eglGetConfigAttrib(display, configs[i], EGL_GREEN_SIZE, &green);
    eglGetConfigAttrib(display, configs[i], EGL_BLUE_SIZE, &blue);
    eglGetConfigAttrib(display, configs[i], EGL_ALPHA_SIZE, &alpha);
    eglGetConfigAttrib(display, configs[i], EGL_DEPTH_SIZE, &depth);
    eglGetConfigAttrib(display, configs[i], EGL_STENCIL_SIZE, &stencil);
    eglGetConfigAttrib(display, configs[i], EGL_SAMPLE_BUFFERS, &sampleBuffers);
    eglGetConfigAttrib(display, configs[i], EGL_SAMPLES, &samples);
    if ( red == cfg->rBits && green == cfg->gBits && blue == cfg->bBits &&
        (!cfg->aBits || alpha == cfg->aBits) && depth == cfg->depthBits &&
        (!cfg->stencilBits || stencil == cfg->stencilBits) &&
      sampleBuffers == cfg->sampleBuffers && samples == cfg->samples )
    {
      config = configs[i];
      break;
    }
  }
  free(configs);
  return config;
}

void CreateContext(Config* cfg,
                   EGLDisplay *display,
                   EGLContext *context,
                   EGLSurface *surface)
{
  EGLint PbufferConfigAttribs[] =
  {
    EGL_RED_SIZE,       8,
    EGL_GREEN_SIZE,     8,
    EGL_BLUE_SIZE,      8,
    EGL_ALPHA_SIZE,     8,
    EGL_DEPTH_SIZE,     24,
    EGL_STENCIL_SIZE,   8,
    EGL_SURFACE_TYPE,   EGL_PBUFFER_BIT,
    EGL_SAMPLE_BUFFERS, 0,
    EGL_SAMPLES,        0,
    EGL_NONE
  };

  static const EGLint ContextAttribs[] =
  {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };

  static EGLint s_pbufferAttribs[] =
  {
    EGL_WIDTH,  0,
    EGL_HEIGHT, 0,
    EGL_NONE
  };

  EGLint      majorVersion;
  EGLint      minorVersion;
  EGLConfig   config;

  s_pbufferAttribs[1] = cfg->winW;
  s_pbufferAttribs[3] = cfg->winH;

  *display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglBindAPI(EGL_OPENGL_ES_API);
  eglInitialize(*display, &majorVersion, &minorVersion);

  IDBG_LOW("EGL Vendor: %s ", eglQueryString(*display, EGL_VENDOR));
  IDBG_LOW("EGL Version: %d:%d ", majorVersion, minorVersion);

  config = GetEglConfig(*display, PbufferConfigAttribs, cfg);

  *context = eglCreateContext(*display, config, NULL, ContextAttribs);
  if (*context == EGL_NO_CONTEXT)
  {
    IDBG_ERROR("Unable to create EGL context");
    return;
  }

  *surface = eglCreatePbufferSurface(*display, config, s_pbufferAttribs);

  if (*surface == EGL_NO_SURFACE )
  {
    IDBG_ERROR("Unable to create EGL surface");
    return;
  }
  return;
}


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
extern "C" int img_algo_init(void **p_context,
                             img_multicam_init_params_t *p_params)
{
  IDBG_LOW(": E");
  int ret = IMG_SUCCESS;
  QCameraBokeh *pCamBokeh;
  pCamBokeh = new QCameraBokeh();
  if (pCamBokeh)
  {
    *p_context = pCamBokeh;
  }
  else
  {
    ret = IMG_ERR_GENERAL;
  }
  if (pCamBokeh)
  {
    pCamBokeh->Init(p_params);
  }

  IDBG_LOW(": X");
  return ret;
}


/**
 * Function: img_algo_abort
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
extern "C" int img_algo_abort(void *p_context)
{
  IDBG_LOW(": E img_algo_abort called");
  int status;
  assert(NULL != p_context);
  QCameraBokeh* pCamBokeh = (QCameraBokeh*) p_context;
  status = pCamBokeh->Abort() ? IMG_SUCCESS: IMG_ERR_GENERAL;
  IDBG_LOW(": X");
  return status;
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
  IDBG_LOW(": E img_algo_deinit called");
  assert(NULL != p_context);
  QCameraBokeh* pCamBokeh = (QCameraBokeh*) p_context;
  pCamBokeh->Flush();
  pCamBokeh->DeInit();
  delete pCamBokeh;
  pCamBokeh = NULL;

  IDBG_LOW(": X");
  return IMG_SUCCESS;
}

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

  if (NULL != p_params)
  {
    img_algo_caps_t *lib_caps = (img_algo_caps_t *)(p_params);

    lib_caps->core_type = IMG_CORE_GPU;
    lib_caps->exec_mode = IMG_EXECUTION_SW_HW;
    lib_caps->buff_access_mode = IMG_ACCESS_READ_WRITE;
    lib_caps->crop_caps = IMG_CROP_UPDATED;
  }
  else
  {
    rc = IMG_ERR_INVALID_INPUT;
    IDBG_ERROR("Invalid input");
  }
  return rc;
}

extern "C" int get_aspect_ratio (uint32_t width, uint32_t height) {
  if ((3 * width) == (4 * height))
  {
    return RATIO_4_3;
  }
  else if ((9 * width) == (16 * height))
  {
    return RATIO_16_9;
  }
  else if ((2 * width) == (3 * height))
  {
    return RATIO_3_2;
  }
  else
  {
    return -1;
  }
}

extern "C" int img_algo_get_lib_config(void *p_context, void *p_data)
{
  IDBG_LOW(": E");
  IMG_UNUSED(p_context);
  img_lib_config_t *p_lib_config = (img_lib_config_t *)p_data;
  img_lib_param_t lib_param_type;
  int32_t lib_ret = IMG_SUCCESS;
  int32_t slave_aspect_ratio;

  if (!p_lib_config || !p_lib_config->lib_data)
  {
    IDBG_ERROR("Invalid input %p", p_lib_config);
    return IMG_ERR_INVALID_INPUT;
  }

  lib_param_type = p_lib_config->lib_param;
  switch (lib_param_type)
  {
    case IMG_ALGO_IN_FRAME_DIM:
    {
      img_multicam_dim_info_t *p_dim_info =
        (img_multicam_dim_info_t *) p_lib_config->lib_data;
      if (p_dim_info->cam_role == IMG_CAM_ROLE_WIDE ||
          p_dim_info->cam_role == IMG_CAM_ROLE_MONO)
      {
        p_dim_info->input_margin.w_margin = 0.0;
        p_dim_info->input_margin.h_margin = 0.0;
        slave_aspect_ratio = get_aspect_ratio(p_dim_info->output_size.width,
                               p_dim_info->output_size.height);

        if (slave_aspect_ratio >= 0)
        {
          p_dim_info->input_size.width = mono_bokeh_dim[slave_aspect_ratio].width;
          p_dim_info->input_size.height= mono_bokeh_dim[slave_aspect_ratio].height;
        }
        else
        {
          IDBG_ERROR("Invalid Aspect ratio for Mono Camera, Use 4:3 or 16:9"
            "aspect ratio only");
          lib_ret = IMG_ERR_INVALID_INPUT;
        }
        p_dim_info->input_size.stride = p_dim_info->input_size.width;
        p_dim_info->input_size.scanline = p_dim_info->input_size.height;
        IDBG_MED("Mono input camera wxh %dx%d stxsc %dx%d slave_aspect_ratio %d",
          p_dim_info->input_size.width,p_dim_info->input_size.height,
          p_dim_info->input_size.stride,p_dim_info->input_size.scanline,
          slave_aspect_ratio);
        if ((p_dim_info->output_pad.w_pad != BOKEH_OUTPUT_WIDTH_ALIGN) ||
          (p_dim_info->output_pad.h_pad != BOKEH_OUTPUT_HEIGHT_ALIGN)) {
          p_dim_info->output_pad.w_pad = BOKEH_OUTPUT_WIDTH_ALIGN;
          p_dim_info->output_pad.h_pad = BOKEH_OUTPUT_HEIGHT_ALIGN;
          p_dim_info->output_size_changed = true;
        }
      }
      else
      {
        p_dim_info->input_margin.w_margin = 0.0;
        p_dim_info->input_margin.h_margin = 0.0;
        p_dim_info->input_size.width = p_dim_info->output_size.width +
          (p_dim_info->output_size.width * p_dim_info->input_margin.w_margin);
        p_dim_info->input_size.height = p_dim_info->output_size.height +
          (p_dim_info->output_size.height * p_dim_info->input_margin.h_margin);
        p_dim_info->input_size.scanline = p_dim_info->input_size.height;
        p_dim_info->input_size.stride = p_dim_info->input_size.width;
        if ((p_dim_info->output_pad.w_pad != BOKEH_OUTPUT_WIDTH_ALIGN) ||
          (p_dim_info->output_pad.h_pad != BOKEH_OUTPUT_HEIGHT_ALIGN)) {
          p_dim_info->output_pad.w_pad = BOKEH_OUTPUT_WIDTH_ALIGN;
          p_dim_info->output_pad.h_pad = BOKEH_OUTPUT_HEIGHT_ALIGN;
          p_dim_info->output_size_changed = true;
        }
        IDBG_MED("Bayer input camera wxh %dx%d stxsc %dx%d",
          p_dim_info->input_size.width,p_dim_info->input_size.height,
          p_dim_info->input_size.stride,p_dim_info->input_size.scanline);
      }
      break;
    }
    default:
      IDBG_ERROR("Invalid param type");
      lib_ret = IMG_ERR_INVALID_INPUT;
  }
  IDBG_LOW(": X");
  return lib_ret;
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
 *   @arg6: p_meta: frame pMeta data
 *   @arg7: meta_cnt: number of pMeta data array
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
  IDBG_LOW(": E Bokeh Process %d, %d", in_frame_cnt, out_frame_cnt);

  int ret = IMG_SUCCESS;
  if (NULL != p_context && NULL != p_meta)
  {
    QCameraBokeh* pCamBokeh = (QCameraBokeh*) p_context;
    ret = pCamBokeh->Execute(p_in_frame,
                             in_frame_cnt,
                             p_out_frame,
                             out_frame_cnt,
                             p_meta,
                             meta_cnt);
  }
  else
  {
    IDBG_ERROR("img_algo_process failed");
    ret = IMG_ERR_GENERAL;
  }

  IDBG_LOW(": X");

  return ret;
}

/**
 * Function: img_algo_set_frame_ops
 *
 * Description: Function to set frame operations
 *
 * Arguments:
 *   @p_context: ptr to instance
 *   @p_ops: ptr to frame ops
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_set_frame_ops(void *p_context,
                                      img_frame_ops_t *p_ops)
{
  IDBG_LOW(": E");
  int ret = IMG_SUCCESS;
  assert(NULL != p_context);
  assert(NULL != p_ops);
  QCameraBokeh* instance = (QCameraBokeh*) p_context;
  instance->setFrameOps(p_ops);
  IDBG_LOW(": X");
  return ret;
}

/**
 * Function: QCameraBokeh
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
QCameraBokeh::QCameraBokeh()
{
  IDBG_LOW(": E");
  memset(&m_frameOps, 0x0, sizeof(m_frameOps));
  m_threadExit = false;
  m_threadCreated = false;
  m_InitthreadCreated = false;
  m_wrapperInitdone = false;
  m_blurTextureData = NULL;

#ifdef ENABLE_QC_BOKEH
  m_pBokehWrapper = NULL;
#endif

  m_pBokehEffects = NULL;
  m_pMasterMeta = NULL;
  m_pSlaveMeta = NULL;
  m_masterIndex = 0;
  m_slaveIndex = 0;
  m_masterSensorWidth = 0;
  m_masterSensorHeight = 0;
  m_pInputColorMap = NULL;
  m_pInputBlurRadiusMap = NULL;
  m_context = EGL_NO_CONTEXT;
  m_surface = EGL_NO_SURFACE;
  m_display = EGL_NO_DISPLAY;
  m_isDepthAlgoDisabled = FALSE;
  m_dumpNow = FALSE;
  m_dumpCount = 0;
  m_dumpingEnabled = FALSE;
  m_threadLock = PTHREAD_MUTEX_INITIALIZER;
  m_threadCond = PTHREAD_COND_INITIALIZER;
  IDBG_LOW(": X");
}

/**
 * Function: ~QCameraBokeh
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
QCameraBokeh::~QCameraBokeh()
{
}

/**
 * Function: AlgoThreadCanWait
 *
 * Description: Function to check if thread can wait
 *
 * Arguments:
 *   @p_userdata: pointer to user data
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 **/
int QCameraBokeh::AlgoThreadCanWait(void *p_userdata)
{
  QCameraBokeh *p_bokeh_obj = (QCameraBokeh *) p_userdata;
  return !p_bokeh_obj->m_threadExit;
}

/**
 * Function: AlgoThreadloop
 *
 * Description: Algo thread loop used for async calls
 *
 * Arguments:
 *   @data: input data
 *
 * Return values:
 *   None
 *
 * Notes: none
 **/
void* QCameraBokeh::AlgoThreadloop(void* data)
{
  QCameraBokeh *p_bokeh_obj = (QCameraBokeh *) data;
  QIMG_LOCK(&p_bokeh_obj->m_threadLock);
  p_bokeh_obj->m_threadCreated = TRUE;
  pthread_cond_signal(&p_bokeh_obj->m_threadCond);
  QIMG_UNLOCK(&p_bokeh_obj->m_threadLock);
  ddm_algo_job_t* pJob;
  uint32_t i, j;
#ifdef ENABLE_QC_BOKEH
  qrcp::VideoBokehWrapperStatus returnStatus;
#endif

  while (TRUE)
  {
    pJob = (ddm_algo_job_t*) img_q_wait(&p_bokeh_obj->m_jobQueue,
                                        QCameraBokeh::AlgoThreadCanWait,
                                        p_bokeh_obj);
    QIMG_LOCK(&p_bokeh_obj->m_threadLock);
    if (!QCameraBokeh::AlgoThreadCanWait(p_bokeh_obj))
    {
      IDBG_ERROR("ddm thread exiting");
      pthread_cond_signal(&p_bokeh_obj->m_threadCond);
      QIMG_UNLOCK(&p_bokeh_obj->m_threadLock);
      break;
    }
    else if (NULL == pJob)
    {
      QIMG_UNLOCK(&p_bokeh_obj->m_threadLock);
      continue;
    }
    QIMG_UNLOCK(&p_bokeh_obj->m_threadLock);
    if ((NULL != pJob->slaveDepthAddr) &&
        (0 != pJob->slaveDepthStride) &&
        (NULL != pJob->masterDepthAddr) &&
        (0 != pJob->masterDepthStride) &&
        (NULL != pJob->outputBuffer) &&
        (0 != pJob->outputWidth) &&
        (0 != pJob->outputHeight) &&
        (0 != pJob->textureId))
    {
#ifdef ENABLE_QC_BOKEH
      if (NULL != p_bokeh_obj->m_pBokehWrapper)
      {
        IDBG_MED("DDM processFrame "
                 "masterFrameAddr = %p masterFrameStride = %d "
                 "slaveFrameAddr = %p slaveFrameStride = %d "
                 "outputDDMAddr = %p outputDDMWidth = %d "
                 "focusROIX = %d focusROIY = %d "
                 "blur intensity = %f "
                 "masterFocalLengthRatio = %f "
                 "slaveFocalLengthRatio = %f",
                 pJob->masterDepthAddr,
                 pJob->masterDepthStride,
                 pJob->slaveDepthAddr,
                 pJob->slaveDepthStride,
                 pJob->outputBuffer,
                 pJob->outputWidth,
                 pJob->focusROIX,
                 pJob->focusROIY,
                 pJob->blurIntensity,
                 pJob->masterFocalLengthRatio,
                 pJob->slaveFocalLengthRatio);

        if (0 != p_bokeh_obj->m_tracingEnabled)
        {
          atrace_begin(ATRACE_TAG_ALWAYS, "bokeh_ddm");
        }

        returnStatus = p_bokeh_obj->m_pBokehWrapper->processFrame(
          pJob->masterDepthAddr,
          pJob->masterDepthStride,
          pJob->slaveDepthAddr,
          pJob->slaveDepthStride,
          pJob->outputBuffer,
          pJob->outputWidth,
          pJob->focusROIX,
          pJob->focusROIY,
          pJob->blurIntensity,
          pJob->masterFocalLengthRatio,
          pJob->slaveFocalLengthRatio);

        if (0 != p_bokeh_obj->m_tracingEnabled)
        {
          atrace_end(ATRACE_TAG_ALWAYS);
        }

        QIMG_LOCK(&p_bokeh_obj->m_lock);
        if (!returnStatus.ok())
        {
          if (returnStatus.getErrorCode() ==
               qrcp::VideoBokehWrapperStatus::ErrorCode::TOO_NEAR)
          {
            IDBG_HIGH("DDM Calculation unsuccessful, error code %d msg: %s",
                      returnStatus.getErrorCode(),
                      returnStatus.getErrorMessage().c_str());
            p_bokeh_obj->m_depthStatus = CAM_RTB_MSG_TOO_NEAR;
          }
          else if (returnStatus.getErrorCode() ==
               qrcp::VideoBokehWrapperStatus::ErrorCode::TOO_FAR)
          {
            IDBG_HIGH("DDM Calculation unsuccessful, error code %d msg: %s",
                      returnStatus.getErrorCode(),
                      returnStatus.getErrorMessage().c_str());
            p_bokeh_obj->m_depthStatus = CAM_RTB_MSG_TOO_FAR;
          }
          else
          {
            IDBG_ERROR("DDM Calculation unsuccessful, error code %d msg: %s",
                       returnStatus.getErrorCode(),
                       returnStatus.getErrorMessage().c_str());
            p_bokeh_obj->m_depthStatus = CAM_RTB_MSG_NO_DEPTH_EFFECT;
          }
          // DDM Failed for calcuate for this frame, clear the depth map
          memset(pJob->outputBuffer,
                 0,
                 pJob->outputWidth * pJob->outputHeight);
        }
        else
        {
          IDBG_MED("DDM processFrame success");
          p_bokeh_obj->m_depthStatus = CAM_RTB_MSG_DEPTH_EFFECT_SUCCESS;
        }
        QIMG_UNLOCK(&p_bokeh_obj->m_lock);
      }
#endif
      QIMG_LOCK(&p_bokeh_obj->m_lock);
      if (0 != p_bokeh_obj->m_tracingEnabled)
      {
        atrace_begin(ATRACE_TAG_ALWAYS, "bokeh_DDM_copy");
      }

      eglMakeCurrent(p_bokeh_obj->m_display,
                     p_bokeh_obj->m_surface,
                     p_bokeh_obj->m_surface,
                     p_bokeh_obj->m_context);
      glBindTexture(GL_TEXTURE_2D, pJob->textureId);

      glTexSubImage2D(GL_TEXTURE_2D,
                      0,
                      0,
                      0,
                      pJob->outputWidth,
                      pJob->outputHeight,
                      GL_RED,
                      GL_UNSIGNED_BYTE,
                      pJob->outputBuffer);

      eglMakeCurrent(p_bokeh_obj->m_display, EGL_NO_SURFACE,
        EGL_NO_SURFACE, EGL_NO_CONTEXT);

      if (0 != p_bokeh_obj->m_tracingEnabled)
      {
        atrace_end(ATRACE_TAG_ALWAYS);
      }
      QIMG_UNLOCK(&p_bokeh_obj->m_lock);
    }
    IDBG_MED("DDM finished job for frame id %d", pJob->frameId);
    free(pJob);
  }
  eglReleaseThread();
  return NULL;
}

void QCameraBokeh::fillOTPCalibrationData(
  cam_related_system_calibration_data_t * p_calib_data)
{
  cam_related_sensor_calibration_data_t main_calib =
    p_calib_data->main_cam_specific_calibration;
  cam_related_sensor_calibration_data_t aux_calib =
    p_calib_data->aux_cam_specific_calibration;
  uint32_t flipSetting = FLIP_NONE;
  uint32_t sensorMountAngle = 270;

  char prop[PROPERTY_VALUE_MAX];
  // Assuming Wide-Master Tele-Slave following swap is required
  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.rtb.optswap", prop, "0");
  optmasterswap = atoi(prop);

  // Assuming Wide-Master Tele-Slave following swap is required
  if(optmasterswap == 1)
  {
    // Todo: Swapping the main/aux as current W+T sensor modules
    //       are reversed to what the ddm algo is expecting.
    //       Once new sensor modules arrive, this will no longer be required
    main_calib = p_calib_data->aux_cam_specific_calibration;
    aux_calib = p_calib_data->main_cam_specific_calibration;
  }
  if (0 == isBayerMono)
  {
    // Todo: Need to make this Generic, instead of hardcoding.
    sensorMountAngle = 90;
  }

  snprintf(m_pCalibrationData,
    2048,
    "Sensor default mirror flip setting = %d\n"
    "Sensor mount angle =  %d\n"
    "Calibration OTP format version = %d\n"
    "Main Normalized Focal length =  %fpx\n"
    "Main Native Sensor Resolution width = %dpx\n"
    "Main Native Sensor Resolution height = %dpx\n"
    "Main Calibration Resolution width = %dpx\n"
    "Main Calibration Resolution height = %dpx\n"
    "Main Focal length ratio = %f\n"
    "Aux Normalized Focal length =  %fpx\n"
    "Aux Native Sensor Resolution width = %dpx\n"
    "Aux Native Sensor Resolution height = %dpx\n"
    "Aux Calibration Resolution width = %dpx\n"
    "Aux Calibration Resolution height = %dpx\n"
    "Aux Focal length ratio = %f\n"
    "Relative Rotation matrix [0] through [8] = "
    "%f,%f,%f,%f,%f,%f,%f,%f,%f\n"
    "Relative Geometric surface parameters [0] through [31] = "
    "%f,%f,%f,%f,%f,%f,%f,%f,"
    "%f,%f,%f,%f,%f,%f,%f,%f,"
    "%f,%f,%f,%f,%f,%f,%f,%f,"
    "%f,%f,%f,%f,%f,%f,%f,%f\n"
    "Relative Principal point X axis offset (ox) = %fpx\n"
    "Relative Principal point Y axis offset (oy) = %fpx\n"
    "Relative position flag = %d\n"
    "Baseline distance = %fmm\n"
    "Main sensor mirror and flip setting = %d\n"
    "Aux sensor mirror and flip setting = %d\n"
    "Module orientation during calibration = %d\n"
    "Rotation flag = %d\n",
    flipSetting,
    sensorMountAngle,
    p_calib_data->calibration_format_version,
    main_calib.normalized_focal_length,
    main_calib.native_sensor_resolution_width,
    main_calib.native_sensor_resolution_height,
    main_calib.calibration_sensor_resolution_width,
    main_calib.calibration_sensor_resolution_height,
    main_calib.focal_length_ratio,
    aux_calib.normalized_focal_length,
    aux_calib.native_sensor_resolution_width,
    aux_calib.native_sensor_resolution_height,
    aux_calib.calibration_sensor_resolution_width,
    aux_calib.calibration_sensor_resolution_height,
    aux_calib.focal_length_ratio,
    p_calib_data->relative_rotation_matrix[0],
    p_calib_data->relative_rotation_matrix[1],
    p_calib_data->relative_rotation_matrix[2],
    p_calib_data->relative_rotation_matrix[3],
    p_calib_data->relative_rotation_matrix[4],
    p_calib_data->relative_rotation_matrix[5],
    p_calib_data->relative_rotation_matrix[6],
    p_calib_data->relative_rotation_matrix[7],
    p_calib_data->relative_rotation_matrix[8],
    p_calib_data->relative_geometric_surface_parameters[0],
    p_calib_data->relative_geometric_surface_parameters[1],
    p_calib_data->relative_geometric_surface_parameters[2],
    p_calib_data->relative_geometric_surface_parameters[3],
    p_calib_data->relative_geometric_surface_parameters[4],
    p_calib_data->relative_geometric_surface_parameters[5],
    p_calib_data->relative_geometric_surface_parameters[6],
    p_calib_data->relative_geometric_surface_parameters[7],
    p_calib_data->relative_geometric_surface_parameters[8],
    p_calib_data->relative_geometric_surface_parameters[9],
    p_calib_data->relative_geometric_surface_parameters[10],
    p_calib_data->relative_geometric_surface_parameters[11],
    p_calib_data->relative_geometric_surface_parameters[12],
    p_calib_data->relative_geometric_surface_parameters[13],
    p_calib_data->relative_geometric_surface_parameters[14],
    p_calib_data->relative_geometric_surface_parameters[15],
    p_calib_data->relative_geometric_surface_parameters[16],
    p_calib_data->relative_geometric_surface_parameters[17],
    p_calib_data->relative_geometric_surface_parameters[18],
    p_calib_data->relative_geometric_surface_parameters[19],
    p_calib_data->relative_geometric_surface_parameters[20],
    p_calib_data->relative_geometric_surface_parameters[21],
    p_calib_data->relative_geometric_surface_parameters[22],
    p_calib_data->relative_geometric_surface_parameters[23],
    p_calib_data->relative_geometric_surface_parameters[24],
    p_calib_data->relative_geometric_surface_parameters[25],
    p_calib_data->relative_geometric_surface_parameters[26],
    p_calib_data->relative_geometric_surface_parameters[27],
    p_calib_data->relative_geometric_surface_parameters[28],
    p_calib_data->relative_geometric_surface_parameters[29],
    p_calib_data->relative_geometric_surface_parameters[30],
    p_calib_data->relative_geometric_surface_parameters[31],
    p_calib_data->relative_principle_point_x_offset,
    p_calib_data->relative_principle_point_y_offset,
    p_calib_data->relative_position_flag,
    p_calib_data->relative_baseline_distance,
    p_calib_data->main_sensor_mirror_flip_setting,
    p_calib_data->aux_sensor_mirror_flip_setting,
    p_calib_data->module_orientation_during_calibration,
    p_calib_data->rotation_flag);
}

void QCameraBokeh::fillROICropData(img_meta_t *p_aMeta, char* buf)
{
  uint32_t i;
  img_fov_t fov_sensor;
  img_fov_t fov_ispif;
  img_fov_t fov_camif;
  img_fov_t fov_isp_out;
  img_fov_t fov_cpp;

  memset(&fov_sensor, 0, sizeof(img_fov_t));
  memset(&fov_ispif, 0, sizeof(img_fov_t));
  memset(&fov_camif, 0, sizeof(img_fov_t));
  memset(&fov_isp_out, 0, sizeof(img_fov_t));
  memset(&fov_cpp, 0, sizeof(img_fov_t));

  for (i = 0; i < IMG_MAX_FOV; i++)
  {
    switch(p_aMeta->qdcc_cfg.rtb_cfg.fov_params[i].module)
    {
      case SENSOR_FOV:
        fov_sensor = p_aMeta->qdcc_cfg.rtb_cfg.fov_params[i];
        if (FALSE != p_aMeta->qdcc_cfg.rtb_cfg.is_master)
        {
          m_masterSensorWidth = fov_sensor.fetch_window_width;
          m_masterSensorHeight = fov_sensor.fetch_window_height;
          IDBG_MED("Sensor dim of master %dx%d",
                  m_masterSensorWidth,
                  m_masterSensorHeight);
        }
        break;
      case ISPIF_FOV:
        fov_ispif = p_aMeta->qdcc_cfg.rtb_cfg.fov_params[i];
        break;
      case CAMIF_FOV:
        fov_camif = p_aMeta->qdcc_cfg.rtb_cfg.fov_params[i];
        break;
      case ISP_OUT_FOV:
        fov_isp_out = p_aMeta->qdcc_cfg.rtb_cfg.fov_params[i];
        break;
      default:
        IDBG_ERROR("Unknown img fov type");
    }
  }
  // Because CPP will not be doing any scale or cropping to
  // the frame, we can set the scale/crop data to match ISP output
  fov_cpp.input_width = fov_isp_out.output_window_width;
  fov_cpp.input_height = fov_isp_out.output_window_height;
  fov_cpp.offset_x = 0;
  fov_cpp.offset_y = 0;
  fov_cpp.fetch_window_width = fov_isp_out.output_window_width;
  fov_cpp.fetch_window_height = fov_isp_out.output_window_height;
  fov_cpp.output_window_width = fov_isp_out.output_window_width;
  fov_cpp.output_window_height = fov_isp_out.output_window_height;
  fov_cpp.module = ISP_OUT_FOV;

  snprintf(buf,
    1024,
    "Sensor Crop left = %d\n"
    "Sensor Crop top = %d\n"
    "Sensor Crop width = %d\n"
    "Sensor Crop height = %d\n"
    "Sensor ROI Map left = %d\n"
    "Sensor ROI Map top = %d\n"
    "Sensor ROI Map width = %d\n"
    "Sensor ROI Map height = %d\n"
    "CAMIF Crop left = %d\n"
    "CAMIF Crop top = %d\n"
    "CAMIF Crop width = %d\n"
    "CAMIF Crop height = %d\n"
    "CAMIF ROI Map left = %d\n"
    "CAMIF ROI Map top = %d\n"
    "CAMIF ROI Map width = %d\n"
    "CAMIF ROI Map height = %d\n"
    "ISP Crop left = %d\n"
    "ISP Crop top = %d\n"
    "ISP Crop width = %d\n"
    "ISP Crop height = %d\n"
    "ISP ROI Map left = %d\n"
    "ISP ROI Map top = %d\n"
    "ISP ROI Map width = %d\n"
    "ISP ROI Map height = %d\n"
    "CPP Crop left = %d\n"
    "CPP Crop top = %d\n"
    "CPP Crop width = %d\n"
    "CPP Crop height = %d\n"
    "CPP ROI Map left = %d\n"
    "CPP ROI Map top = %d\n"
    "CPP ROI Map width = %d\n"
    "CPP ROI Map height = %d\n"
    "DDM Crop left = 0000\n"
    "DDM Crop top = 0000\n"
    "DDM Crop width = 0000\n"
    "DDM Crop height = 0000\n"
    "Focal length Ratio = 1.0\n"
    "Current pipeline mirror flip setting = 0\n"
    "Current pipeline rotation setting = 0\n",
    fov_sensor.offset_x,
    fov_sensor.offset_y,
    fov_sensor.fetch_window_width,
    fov_sensor.fetch_window_height,
    0,
    0,
    fov_sensor.output_window_width,
    fov_sensor.output_window_height,
    fov_ispif.offset_x,
    fov_ispif.offset_y,
    fov_ispif.fetch_window_width,
    fov_ispif.fetch_window_height,
    0,
    0,
    fov_ispif.output_window_width,
    fov_ispif.output_window_height,
    fov_isp_out.offset_x,
    fov_isp_out.offset_y,
    fov_isp_out.fetch_window_width,
    fov_isp_out.fetch_window_height,
    0,
    0,
    fov_camif.output_window_width,
    fov_camif.output_window_height,
    fov_cpp.offset_x,
    fov_cpp.offset_y,
    fov_cpp.fetch_window_width,
    fov_cpp.fetch_window_height,
    0,
    0,
    fov_cpp.output_window_width,
    fov_cpp.output_window_height);
}

/**
 * Function: QCameraBokeh: Init
 *
 * Description: Init of class
 *
 * Arguments:
 *   @arg1:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
bool QCameraBokeh::Init(img_multicam_init_params_t *p_params)
{
  IDBG_LOW(": E");
  bool ret = FALSE;
  cam_related_system_calibration_data_t *p_calib_data;
  img_multicam_dim_info_t multicam_dim_info;
  uint32_t i;
  m_DDMObjInitialized = FALSE;
  m_frameNum = 0;
  m_depthStatus = 0;
  if (p_params == NULL)
  {
    IDBG_ERROR("Invalid params");
    return IMG_ERR_INVALID_INPUT;
  }

  for (i = 0; i < p_params->num_of_sessions; i++)
  {
    multicam_dim_info = p_params->multicam_dim_info[i];
    switch (multicam_dim_info.cam_role)
    {
      case IMG_CAM_ROLE_WIDE:
      case IMG_CAM_ROLE_MONO:
        m_slaveIndex = i;
        break;
      case IMG_CAM_ROLE_TELE:
      case IMG_CAM_ROLE_BAYER:
        m_masterIndex = i;
        break;
      default:
        IDBG_ERROR("Invalid cam role");
        return IMG_ERR_INVALID_INPUT;
    }
    if(multicam_dim_info.cam_role == IMG_CAM_ROLE_BAYER) {
      isBayerMono = 1;
    } else if (multicam_dim_info.cam_role == IMG_CAM_ROLE_TELE) {
      isBayerMono = 0;
    }
  }

  multicam_dim_info = p_params->multicam_dim_info[m_masterIndex];
  img_opaque_data_set_t sensor_calib = multicam_dim_info.sensor_calib_data;
  m_focalRatio = p_params->multicam_dim_info[m_masterIndex].focal_length;

  if (sizeof(cam_related_system_calibration_data_t) != sensor_calib.data_size)
  {
    IDBG_ERROR("Sensor Calibration data size mismatch");
    return IMG_ERR_INVALID_INPUT;
  }
  p_calib_data = (cam_related_system_calibration_data_t*) sensor_calib.p_data;

  if (p_calib_data == NULL)
  {
    IDBG_ERROR("Sensor Calibration data NULL");
    return IMG_ERR_INVALID_INPUT;
  }

  fillOTPCalibrationData(p_calib_data);

  m_bufferMap.Init();
  Config cfg =
  {
    (int)multicam_dim_info.input_size.width,
    (int)multicam_dim_info.input_size.height,
    8,
    8,
    8,
    8,
    32,
    24,
    8,
    0,
    0
  };
  CreateContext(&cfg, &m_display, &m_context, &m_surface);
  m_bokehInitialized = FALSE;

  m_pBokehEffects = new Effects();

  img_q_init(&m_jobQueue, (char *)"Bokeh_Depth_Job_Q");
  QIMG_LOCK(&m_threadLock);
  ret = pthread_create(&m_threadId, NULL, QCameraBokeh::AlgoThreadloop, this);
  if (0 != ret)
  {
    IDBG_ERROR("Error in thread creation");
    ret = IMG_ERR_GENERAL;
    QIMG_UNLOCK(&m_threadLock);
    return ret;
  }
  pthread_setname_np(m_threadId, "CAM_bokeh_DDM");
  ret = pthread_cond_wait(&m_threadCond, &m_threadLock);
  if (FALSE == m_threadCreated)
  {
    IDBG_ERROR("pthread cond wait Error %d %s", ret, strerror(ret));
    ret = IMG_ERR_GENERAL;
    QIMG_UNLOCK(&m_threadLock);
    return ret;
  }

  char prop[PROPERTY_VALUE_MAX];
  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.rtb.ddm_off", prop, "0");
  m_isDepthAlgoDisabled = atoi(prop);

  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.rtb.tracing", prop, "0");
  m_tracingEnabled = atoi(prop);

  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.imglib.dump", prop, "0");
  m_dumpingEnabled = atoi(prop);

  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.rtb.lux_index", prop, "400");
  lux_index = (float)atoi(prop);

  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.rtb.dummy_algo", prop, "1");
  m_isRTBAlgoDummy = atoi(prop);

  QIMG_UNLOCK(&m_threadLock);
  IDBG_LOW(": X");
  return ret;
}

/**
 * Function: QCameraBokeh::Flush
 *
 * Description: Flush function of class
 *
 * Arguments:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void QCameraBokeh::Flush()
{
  IDBG_LOW(": E");
  eglMakeCurrent(m_display, m_surface, m_surface, m_context);
  m_bufferMap.FlushMaps();
  eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
    EGL_NO_CONTEXT);

  IDBG_LOW("Last frame: Done Flushing Map");
  IDBG_LOW(": X");
}

/**
 * Function: QCameraBokeh::Abort
 *
 * Description: Abort DDM processing
 *
 * Arguments:
 *	 none
 *
 * Return values:
 *	   True/False: Status of DDM abort
 *
 * Notes: none
 **/
bool QCameraBokeh::Abort()
{
  bool ret = TRUE;
  int rc;
  IDBG_LOW(": E");
  if (m_threadExit) {
    IDBG_ERROR("ddm already aborted");
    return ret;
  }
  QIMG_LOCK(&m_threadLock);
  img_q_flush_and_destroy(&m_jobQueue);
  m_threadExit = true;
  img_q_signal(&m_jobQueue);
  //rc = img_wait_for_completion(&m_threadCond, &m_threadLock, 500);
  //if ((rc == IMG_ERR_TIMEOUT) || (rc == IMG_ERR_GENERAL)) {
   //IDBG_ERROR("ddm thread exit error");
   //ret = FALSE;
  //}
  rc = pthread_cond_wait(&m_threadCond, &m_threadLock);

  // wait until initthread is done when created
  if (m_InitthreadCreated) {
    if (!pthread_equal(m_initthreadId, pthread_self())) {
      pthread_join(m_initthreadId, NULL);
    }
    IDBG_INFO("ddm init thread exited");
  }

  IDBG_INFO("ddm abort success");
  QIMG_UNLOCK(&m_threadLock);
  IDBG_LOW(": X");
  return ret;
}


/**
 * Function: QCameraBokeh::DeInit
 *
 * Description: DeInit function of class
 *
 * Arguments:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
bool QCameraBokeh::DeInit()
{
  IDBG_LOW(": E");
  bool ret = FALSE;
  int rc;

  // ddm thread join
  if (!pthread_equal(m_threadId, pthread_self())) {
    pthread_join(m_threadId, NULL);
  }

  img_q_deinit(&m_jobQueue);
  m_depthStatus = 0;
  m_frameNum = 0;
  m_threadCreated = FALSE;
  if (NULL != m_blurTextureData)
  {
    free(m_blurTextureData);
  }

#ifdef ENABLE_QC_BOKEH
  if (NULL != m_pBokehWrapper)
  {
    delete m_pBokehWrapper;
    m_DDMObjInitialized = FALSE;
  }
#endif

  eglMakeCurrent(m_display, m_surface, m_surface, m_context);

  if (NULL != m_pBokehEffects)
  {
    m_pBokehEffects->Deinitialize();
    delete m_pBokehEffects;
    m_pBokehEffects = NULL;
    m_bokehInitialized = false;
  }

  if(m_pInputColorMap != NULL)
  {
    delete m_pInputColorMap;
    m_pInputColorMap = NULL;
  }

  if(m_pInputBlurRadiusMap != NULL)
  {
    delete m_pInputBlurRadiusMap;
    m_pInputBlurRadiusMap = NULL;
  }

  m_bufferMap.DeInit();

  if (m_surface != EGL_NO_SURFACE)
  {
    eglDestroySurface(m_display, m_surface);
  }

  if (m_context != EGL_NO_CONTEXT)
  {
    eglDestroyContext(m_display, m_context);
  }

  if (m_display != EGL_NO_DISPLAY)
  {
    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
      EGL_NO_CONTEXT);
    eglReleaseThread();
    eglTerminate(m_display);
  }
  m_context = EGL_NO_CONTEXT;
  m_surface = EGL_NO_SURFACE;
  m_display = EGL_NO_DISPLAY;

  IDBG_LOW(": X");
  return ret;
}

static void copy_input_output(img_frame_t *pInFrame[],
  img_frame_t *pOutFrame[], uint32_t master_index)
{
  uint32_t i,j,h,w,s_st,d_st;
  unsigned char *src_ptr;
  unsigned char *dst_ptr;
  uint32_t outSize = QIMG_LEN(pOutFrame[0], 0) +
                       QIMG_LEN(pOutFrame[0], 1);
  uint32_t inSize = QIMG_LEN(pInFrame[master_index], 0) +
                       QIMG_LEN(pInFrame[master_index], 1);

  if (outSize == inSize) {
    memcpy(QIMG_ADDR(pOutFrame[0], 0),
      QIMG_ADDR(pInFrame[master_index], 0),inSize);
  } else {
    for (j = 0; j < pOutFrame[0]->frame[0].plane_cnt; j++) {
      h = QIMG_HEIGHT(pOutFrame[0], j);
      w = QIMG_WIDTH(pOutFrame[0], j);
      s_st = QIMG_STRIDE(pInFrame[master_index], j);
      d_st = QIMG_STRIDE(pOutFrame[0], j);
      IDBG_MED("j %d w %d h %d s_st %d d_st %d", j, w, h, s_st, d_st);
      src_ptr = QIMG_ADDR(pInFrame[master_index], j);
      dst_ptr = QIMG_ADDR(pOutFrame[0], j);
      for (i = 0; i < h; i++) {
        memcpy(dst_ptr, src_ptr, w);
        src_ptr += s_st;
        dst_ptr += d_st;
      }
    }
  }
}

/**
 * Function: BlurInit
 *
 * Description: Function to init blur wrapper
 *
 * Arguments:
 *    @inputWidth: Input frame width
 *    @inputHeight: Input frame height
 *    @p_obj: pointer to user data
 *
 * Return values:
 *   0 on success, -ve on failure
 *
 * Notes: none
 **/
int32_t QCameraBokeh::BlurInit(uint32_t inputWidth,
  uint32_t inputHeight, void *p_obj)
{
  QCameraBokeh *p_bkhobj = (QCameraBokeh *)p_obj;
  p_bkhobj->m_width = inputWidth;
  p_bkhobj->m_height = inputHeight;
  if (FALSE != p_bkhobj->m_isDepthAlgoDisabled) {
    p_bkhobj->m_blurSizeWidth = BOKEH_DDM_TARGET_WIDTH;
    p_bkhobj->m_blurSizeHeight = BOKEH_DDM_TARGET_HEIGHT;
    p_bkhobj->m_goodROISrcX = 0;
    p_bkhobj->m_goodROISrcY = 0;
    p_bkhobj->m_goodROISrcWidth = inputWidth;
    p_bkhobj->m_goodROISrcHeight = inputHeight;
  }
  IDBG_MED("DDM buffer dim %dx%d roi xy %dx%d roi wxh %dx%d",
    p_bkhobj->m_blurSizeWidth,
    p_bkhobj->m_blurSizeHeight,
    p_bkhobj->m_goodROISrcX,
    p_bkhobj->m_goodROISrcY,
    p_bkhobj->m_goodROISrcWidth,
    p_bkhobj->m_goodROISrcHeight);

  p_bkhobj->m_blurSize = p_bkhobj->m_blurSizeWidth * p_bkhobj->m_blurSizeHeight;
  p_bkhobj->m_blurTextureData = (unsigned char*)malloc(p_bkhobj->m_blurSize);
  if (!p_bkhobj->m_blurTextureData){
    IDBG_ERROR("ddm output alloc failed");
    return IMG_ERR_NO_MEMORY;
  }
  if (FALSE != p_bkhobj->m_isDepthAlgoDisabled) {
    size_t blursize = p_bkhobj->m_blurSize;
    memset(p_bkhobj->m_blurTextureData, 0, blursize/ 4);
    memset(p_bkhobj->m_blurTextureData + (blursize / 4), 12, blursize/ 4);
    memset(p_bkhobj->m_blurTextureData + 2 * (blursize / 4), 20, blursize / 4);
    memset(p_bkhobj->m_blurTextureData + 3 * (blursize / 4), 45, blursize / 4);
  }

  glGenTextures(1, &p_bkhobj->m_rgbOutTextureId);
  glBindTexture(GL_TEXTURE_2D, p_bkhobj->m_rgbOutTextureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,
    0,
    GL_RGB,
    inputWidth,
    inputHeight,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    0);

  glGenTextures(1, &p_bkhobj->m_blurTextureId);
  glBindTexture(GL_TEXTURE_2D, p_bkhobj->m_blurTextureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D,
    0,
    GL_R8,
    p_bkhobj->m_blurSizeWidth,
    p_bkhobj->m_blurSizeHeight,
    0,
    GL_RED,
    GL_UNSIGNED_BYTE,
    p_bkhobj->m_blurTextureData);

  p_bkhobj->m_bokehInitialized = p_bkhobj->m_pBokehEffects->Initialize(
                                                 inputWidth,
                                                 inputHeight,
                                                 p_bkhobj->m_goodROISrcX,
                                                 p_bkhobj->m_goodROISrcY,
                                                 p_bkhobj->m_goodROISrcWidth,
                                                 p_bkhobj->m_goodROISrcHeight,
                                                 p_bkhobj->m_rgbOutTextureId);

  if (FALSE == p_bkhobj->m_bokehInitialized) {
    IDBG_ERROR("Effects Initialization failed");
    return IMG_ERR_GENERAL;
  }
  p_bkhobj->m_pInputColorMap = new Texture(p_bkhobj->m_rgbOutTextureId,
                                   inputWidth, inputHeight,
                                   1);
  p_bkhobj->m_pInputBlurRadiusMap = new Texture(p_bkhobj->m_blurTextureId,
                                      p_bkhobj->m_blurSizeWidth,
                                      p_bkhobj->m_blurSizeHeight,
                                      0);

  IDBG_MED("Blur Init Success");
  return IMG_SUCCESS;
}


/**
 * Function: DDMInit
 *
 * Description: Function to init DDM library wrapper
 *
 * Arguments:
*    @pInFrame: Input frame bundle
 *   @p_userdata: pointer to user data
 *
 * Return values:
 *   0 on success, -ve on failure
 *
 * Notes: none
 **/
 int32_t QCameraBokeh::DDMInit(bokeh_dim *bokeh_dim,
  void *p_obj)
{
  char prop[PROPERTY_VALUE_MAX];
  uint32_t dumpCalib = 0;
  QCameraBokeh *p_bokeh_obj = (QCameraBokeh *)p_obj;
  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.rtb.dump", prop, "0");
  dumpCalib = atoi(prop);

  IDBG_MED("E");
  p_bokeh_obj->fillROICropData(p_bokeh_obj->m_pSlaveMeta,
    p_bokeh_obj->m_pSlaveROICrop);
  p_bokeh_obj->fillROICropData(p_bokeh_obj->m_pMasterMeta,
    p_bokeh_obj->m_pMasterROICrop);

  if (0 != dumpCalib) {
    char roi_w[64];
    char roi_t[64];
    char calib[64];
    snprintf(roi_w, 64, QCAMERA_DUMP_FRM_LOCATION"aux_roi_crop.txt");
    snprintf(roi_t, 64, QCAMERA_DUMP_FRM_LOCATION"primary_roi_crop.txt");
    snprintf(calib, 64, QCAMERA_DUMP_FRM_LOCATION"calib_data.txt");

    FILE *fout_w = fopen(roi_w, "wb");
    FILE *fout_t = fopen(roi_t, "wb");
    FILE *fout_c = fopen(calib, "wb");
    if (NULL != fout_w) {
      fprintf(fout_w, "%s", p_bokeh_obj->m_pSlaveROICrop);
      fclose(fout_w);
    }
    if (NULL != fout_t) {
      fprintf(fout_t, "%s", p_bokeh_obj->m_pMasterROICrop);
      fclose(fout_t);
    }
    if (NULL != fout_c) {
      fprintf(fout_c, "%s", p_bokeh_obj->m_pCalibrationData);
      fclose(fout_c);
    }
  }

  p_bokeh_obj->enableHVX = false;

#ifdef ENABLE_QC_BOKEH
  p_bokeh_obj->m_pBokehWrapper = new qrcp::VideoBokehWrapper(
    bokeh_dim->master_width,
    bokeh_dim->master_height,
    bokeh_dim->slave_width,
    bokeh_dim->slave_height,
    p_bokeh_obj->m_pMasterROICrop,
    strlen(p_bokeh_obj->m_pMasterROICrop),
    p_bokeh_obj->m_pSlaveROICrop,
    strlen(p_bokeh_obj->m_pSlaveROICrop),
    p_bokeh_obj->m_pCalibrationData,
    strlen(p_bokeh_obj->m_pCalibrationData),
    p_bokeh_obj->m_focalRatio,
    p_bokeh_obj->enableHVX);

  IDBG_MED("DDM Wrapper Create "
    "Master wxh %dx%d "
    "Slave wxh %d %d "
    "Master crop data %p %d "
    "Slave crop data %p %d "
    "OTP calib data %p %d "
    "Master focal ratio %f "
    "goodROISrc %d %d %d %d",
    bokeh_dim->master_width,
    bokeh_dim->master_height,
    bokeh_dim->slave_width,
    bokeh_dim->slave_height,
    p_bokeh_obj->m_pMasterROICrop,
    strlen(p_bokeh_obj->m_pMasterROICrop),
    p_bokeh_obj->m_pSlaveROICrop,
    strlen(p_bokeh_obj->m_pSlaveROICrop),
    p_bokeh_obj->m_pCalibrationData,
    strlen(p_bokeh_obj->m_pCalibrationData),
    p_bokeh_obj->m_focalRatio,
    p_bokeh_obj->m_goodROISrcX,
    p_bokeh_obj->m_goodROISrcY,
    p_bokeh_obj->m_goodROISrcWidth,
    p_bokeh_obj->m_goodROISrcHeight);
  if (p_bokeh_obj->m_pBokehWrapper == NULL) {
    IDBG_ERROR("Unable to create DDM Wrapper");
    return IMG_ERR_GENERAL;
  } else {
    bool isSuccess = p_bokeh_obj->m_pBokehWrapper->getRenderParams(
      p_bokeh_obj->m_blurSizeWidth,p_bokeh_obj->m_blurSizeHeight,
      p_bokeh_obj->m_goodROISrcX,p_bokeh_obj->m_goodROISrcY,
      p_bokeh_obj->m_goodROISrcWidth, p_bokeh_obj->m_goodROISrcHeight);
    if (FALSE == isSuccess) {
      IDBG_ERROR("Unable to get DDM destination buffer size");
      return IMG_ERR_GENERAL;
    } else {
      p_bokeh_obj->m_DDMObjInitialized = TRUE;
      IDBG_MED("DDM wrapper created, buffer dim %dx%d goodROISrc %d %d %d %d",
        p_bokeh_obj->m_blurSizeWidth,
        p_bokeh_obj->m_blurSizeHeight,
        p_bokeh_obj->m_goodROISrcX, p_bokeh_obj->m_goodROISrcY,
        p_bokeh_obj->m_goodROISrcWidth, p_bokeh_obj->m_goodROISrcHeight);
    }
  }
#endif
  IDBG_MED("DDM Init Success");
  return IMG_SUCCESS;

}

/**
 * Function: DDMBlurInitThread
 *
 * Description: DDM and blur wrapper init thread
 * to do initialize asynchronously. Until initialization
 * is done, input master frames will be copied to output
 *
 * Arguments:
 *   @data: input data
 *
 * Return values:
 *   None
 *
 * Notes: none
 **/
void* QCameraBokeh::DDMBlurInitThread(void *arg)
{
  int32_t rc;
  bokeh_dim bokeh_dim;
  ddmblur_algo_init_t *param = (ddmblur_algo_init_t *)arg;
  QCameraBokeh *p_bokeh_obj = (QCameraBokeh *)param->pBokehObj;
  img_frame_t **pInFrame = param->pFrame;

  QIMG_LOCK(&p_bokeh_obj->m_threadLock);
  bokeh_dim.master_width = QIMG_WIDTH(pInFrame[p_bokeh_obj->m_masterIndex], 0);
  bokeh_dim.master_height = QIMG_HEIGHT(pInFrame[p_bokeh_obj->m_masterIndex], 0);
  bokeh_dim.slave_width = QIMG_WIDTH(pInFrame[p_bokeh_obj->m_slaveIndex], 0);
  bokeh_dim.slave_height = QIMG_HEIGHT(pInFrame[p_bokeh_obj->m_slaveIndex], 0);
  p_bokeh_obj->m_InitthreadCreated = TRUE;
  pthread_cond_signal(&p_bokeh_obj->m_threadCond);
  QIMG_UNLOCK(&p_bokeh_obj->m_threadLock);

  IDBG_MED("E %d", p_bokeh_obj->m_InitthreadCreated);

  p_bokeh_obj->m_wrapperInitStatus = IMG_SUCCESS;
  p_bokeh_obj->m_wrapperInitdone = false;
  if (FALSE == p_bokeh_obj->m_isDepthAlgoDisabled &&
    FALSE == p_bokeh_obj->m_DDMObjInitialized ) {
    rc = QCameraBokeh::DDMInit(&bokeh_dim, (void *)p_bokeh_obj);
    if (IMG_ERROR(rc)) {
      p_bokeh_obj->m_wrapperInitStatus = rc;
      p_bokeh_obj->m_wrapperInitdone = true;
      IDBG_ERROR("DDMInit failed");
      return NULL;
    }
  }

  eglMakeCurrent(p_bokeh_obj->m_display, p_bokeh_obj->m_surface,
    p_bokeh_obj->m_surface, p_bokeh_obj->m_context);
  if ((FALSE == p_bokeh_obj->m_bokehInitialized) && (NULL != p_bokeh_obj->m_pBokehEffects)) {
    rc = QCameraBokeh::BlurInit(bokeh_dim.master_width,
           bokeh_dim.master_height, (void *)p_bokeh_obj);
    if (IMG_ERROR(rc)) {
      p_bokeh_obj->m_wrapperInitStatus = rc;
      p_bokeh_obj->m_wrapperInitdone = true;
      eglMakeCurrent(p_bokeh_obj->m_display, EGL_NO_SURFACE,
        EGL_NO_SURFACE, EGL_NO_CONTEXT);
      IDBG_ERROR("BlurInit failed");
      return NULL;
    }
  }
  eglMakeCurrent(p_bokeh_obj->m_display, EGL_NO_SURFACE,
    EGL_NO_SURFACE, EGL_NO_CONTEXT);
  QIMG_LOCK(&p_bokeh_obj->m_lock);
  p_bokeh_obj->m_wrapperInitdone = true;
  QIMG_UNLOCK(&p_bokeh_obj->m_lock);
  eglReleaseThread();
  IDBG_MED("DDMBlurWrapperInit success");
  return NULL;
}

/**
 * Function: Execute
 *
 * Description: main processing body
 *
 * Arguments:
 *   @pInFrame: input frame handler
 *   @inFrameCount: number of input frames
 *   @pOutFrame: output frame handler
 *   @outFrameCount: number of output frames
 *   @pMeta: frame pMeta data
 *   @metaCount: number of pMeta
 *
 * Return values:
 *   IMG_SUCCESS or IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t QCameraBokeh::Execute(img_frame_t *pInFrame[],
  int inFrameCount,
  img_frame_t *pOutFrame[],
  int outFrameCount,
  img_meta_t* pMeta[],
  int metaCount)
{
  int index = 0;
  uint32_t i = 0;
  uint32_t j = 0;
  uint32_t inputWidth = 0;
  uint32_t inputHeight = 0;
  uint32_t outputWidth = 0;
  uint32_t outputHeight = 0;
  uint32_t depthmap_latency = 0;
  int32_t rc = IMG_SUCCESS;
  int32_t update_format;
  GLuint sourceTexture = 0;
  GLuint frameBuffer = 0;
  img_realtime_bokeh_result_t masterRTBResult;
  img_realtime_bokeh_result_t slaveRTBResult;
  img_fov_t bokeh_crop;
  memset(&bokeh_crop, 0x0, sizeof(bokeh_crop));
  memset(&masterRTBResult, 0x0, sizeof(img_realtime_bokeh_result_t));
  memset(&slaveRTBResult, 0x0, sizeof(img_realtime_bokeh_result_t));

  if (0 != m_tracingEnabled)
  {
    atrace_begin(ATRACE_TAG_ALWAYS, "bokeh_process");
  }
  IDBG_LOW(": E");
  // Bokeh did not receive necessary frame data
  // to process bokeh
  if ((NULL == pMeta) || (0 == metaCount) ||
      (NULL == pInFrame) || (0 == inFrameCount) ||
      (NULL == pOutFrame) || (0 == outFrameCount))
  {
    IDBG_ERROR("No input frame data");
    return IMG_SUCCESS;
  }

  for (index = 0; index < metaCount; index++)
  {
    if (!pMeta[index])
    {
      return IMG_SUCCESS;
    }
    switch (pMeta[index]->qdcc_cfg.rtb_cfg.camera_role)
    {
      case IMG_CAM_ROLE_WIDE:
      case IMG_CAM_ROLE_MONO:
        m_pSlaveMeta = pMeta[index];
        m_slaveIndex = index;
        if (m_pSlaveMeta == NULL)
        {
          IDBG_ERROR("Invalid Meta for WIDE/Mono");
          return IMG_ERR_INVALID_INPUT;
        }
        if (m_pSlaveMeta->qdcc_cfg.rtb_cfg.is_master)
        {
          slaveRTBResult.camera_role =
            pMeta[index]->qdcc_cfg.rtb_cfg.camera_role;
          slaveRTBResult.is_camera_role_valid = true;
          masterRTBResult.camera_role =
            pMeta[index]->qdcc_cfg.rtb_cfg.camera_role;
          masterRTBResult.is_camera_role_valid = true;
          slaveRTBResult.is_master = true;
          slaveRTBResult.is_master_flag_valid = true;
          masterRTBResult.is_master = false;
          masterRTBResult.is_master_flag_valid = true;
          IDBG_ERROR("RTB has the incorrect master set");
        }
        break;
      case IMG_CAM_ROLE_TELE:
      case IMG_CAM_ROLE_BAYER:
        m_pMasterMeta = pMeta[index];
        m_masterIndex = index;
        if (m_pMasterMeta == NULL)
        {
          IDBG_ERROR("Invalid Meta for Bayer/Tele");
          return IMG_ERR_INVALID_INPUT;
        }
        if (m_pMasterMeta->qdcc_cfg.rtb_cfg.is_master)
        {
          slaveRTBResult.camera_role =
            pMeta[index]->qdcc_cfg.rtb_cfg.camera_role;
          slaveRTBResult.is_camera_role_valid = true;
          masterRTBResult.camera_role =
            pMeta[index]->qdcc_cfg.rtb_cfg.camera_role;
          masterRTBResult.is_camera_role_valid = true;
          slaveRTBResult.is_master = false;
          slaveRTBResult.is_master_flag_valid = true;
          masterRTBResult.is_master = true;
          masterRTBResult.is_master_flag_valid = true;
          IDBG_MED("RTB camera role %d is set as the Master",
                   pMeta[index]->qdcc_cfg.rtb_cfg.camera_role);
        }
        break;
      default:
        IDBG_ERROR("Error invalid type %d",
                   pMeta[index]->qdcc_cfg.rtb_cfg.camera_role);
        return IMG_ERR_INVALID_INPUT;
    }
  }

  IDBG_MED("BOKEH_CROP x,y = %dx%d crop_out x,y = %dx%d",
    pMeta[m_masterIndex]->l_stream_crop.pos.x,
    pMeta[m_masterIndex]->l_stream_crop.pos.y,
    pMeta[m_masterIndex]->l_stream_crop.size.width,
    pMeta[m_masterIndex]->l_stream_crop.size.height);

  if (m_pMasterMeta == NULL)
  {
    IDBG_ERROR("m_pMasterMeta Meta result is NULL");
    return IMG_ERR_INVALID_INPUT;
  }

  rc = img_common_get_frame_fmt(pOutFrame[0]->info.fmt,&update_format);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Invalid format = %d",pOutFrame[0]->info.fmt);
    return IMG_ERR_INVALID_INPUT;
  }
  // Bokeh received 1 input frame, copy input to output
  // and log error
  if (1 == metaCount && 1 == inFrameCount && 1 == outFrameCount)
  {
    index = m_masterIndex;

    uint32_t outSize = QIMG_LEN(pOutFrame[0], 0) + QIMG_LEN(pOutFrame[0], 1);
    uint32_t inSize = QIMG_LEN(pInFrame[index], 0) + QIMG_LEN(pInFrame[index], 1);

    if(pMeta[m_masterIndex]->l_stream_crop.pos.x > 0 ||
      pMeta[m_masterIndex]->l_stream_crop.pos.y > 0 ||
      (pMeta[m_masterIndex]->l_stream_crop.size.width <
      QIMG_WIDTH(pInFrame[m_masterIndex], 0)) ||
      (pMeta[m_masterIndex]->l_stream_crop.size.height <
      QIMG_HEIGHT(pInFrame[m_masterIndex], 0)))
    {
      IDBG_LOW("Swtiching with RTB Zoom Enabled Drop frame");
      return IMG_ERR_FRAME_DROP;
    } else {
      copy_input_output(pInFrame, pOutFrame, index);
    }

    if (m_pSlaveMeta)
    {
      rc = m_frameOps.set_meta(m_pSlaveMeta,
                               IMG_META_BOKEH_RESULT,
                               (void *)&slaveRTBResult);
      if (IMG_ERROR(rc))
      {
        IDBG_ERROR("Set Meta result for wide failed");
        return rc;
      }
    }
    if (m_pMasterMeta)
    {
      rc = m_frameOps.set_meta(m_pMasterMeta,
                               IMG_META_BOKEH_RESULT,
                               (void *)&masterRTBResult);
      if (IMG_ERROR(rc))
      {
        IDBG_ERROR("Set Meta result for tele failed");
        return rc;
      }
    }

    if (0 != img_q_count(&m_jobQueue))
    {
      IDBG_ERROR("invalid bundle, flush ddm queue");
      img_q_flush_and_destroy(&m_jobQueue);
    }

    IDBG_ERROR("Bokeh requires 2 input frames");
    return IMG_SUCCESS;
  }


  if (m_isRTBAlgoDummy) {
    /* For DummyAlgo setting status as success because
     * HAL can take bokeh snapshot based on RTB status.
     */
    m_depthStatus = CAM_RTB_MSG_DEPTH_EFFECT_SUCCESS;
    if (m_pSlaveMeta)
    {
     slaveRTBResult.depth_effect_msg = m_depthStatus;
     rc = m_frameOps.set_meta(m_pSlaveMeta,
                               IMG_META_BOKEH_RESULT,
                               (void *)&slaveRTBResult);
      if (IMG_ERROR(rc))
      {
        IDBG_ERROR("Set Meta result for slave failed");
      }
    }
    if (m_pMasterMeta)
    {
      masterRTBResult.depth_effect_msg = m_depthStatus;
      rc = m_frameOps.set_meta(m_pMasterMeta,
                               IMG_META_BOKEH_RESULT,
                               (void *)&masterRTBResult);
      if (IMG_ERROR(rc))
      {
        IDBG_ERROR("Set Meta result for master failed");
      }
    }
    copy_input_output(pInFrame, pOutFrame, m_masterIndex);
    return IMG_SUCCESS;
  }

  if ((FALSE == m_DDMObjInitialized) && (FALSE == m_bokehInitialized) &&
    !m_InitthreadCreated) {
    ddmblur_algo_init_t arg;
    QIMG_LOCK(&m_threadLock);
    arg.pFrame = pInFrame;
    arg.pBokehObj = (void *)this;
    rc = pthread_create(&m_initthreadId, NULL, QCameraBokeh::DDMBlurInitThread,
      (void *)&arg);
    if (0 != rc) {
      IDBG_ERROR("Error in ddmblur init thread creation");
      rc = IMG_ERR_GENERAL;
      QIMG_UNLOCK(&m_threadLock);
      return rc;
    }
    pthread_setname_np(m_initthreadId, "CAM_BokehDDM_Init");
    rc = pthread_cond_wait(&m_threadCond, &m_threadLock);
    if (!m_InitthreadCreated) {
      IDBG_ERROR("pthread cond wait Error %d %s", rc, strerror(rc));
      rc = IMG_ERR_GENERAL;
      QIMG_UNLOCK(&m_threadLock);
      return rc;
    }
    QIMG_UNLOCK(&m_threadLock);
  }

  if (m_wrapperInitdone && IMG_ERROR(m_wrapperInitStatus)) {
      IDBG_ERROR("Init failed %d %d", m_DDMObjInitialized, m_bokehInitialized);
      return m_wrapperInitStatus;
  }

  inputWidth = QIMG_WIDTH(pInFrame[m_masterIndex], 0);
  inputHeight = QIMG_HEIGHT(pInFrame[m_masterIndex], 0);
  outputWidth = QIMG_WIDTH(pOutFrame[0], 0);
  outputHeight = QIMG_HEIGHT(pOutFrame[0], 0);
  IDBG_MED("Master wxh %dx%d stxsc %dx%d "
    "Slave wxh %dx%d stxsc %dx%d "
    "Output wxh %dx%d stxsc %dx%d",
    QIMG_WIDTH(pInFrame[m_masterIndex], 0),
    QIMG_HEIGHT(pInFrame[m_masterIndex], 0),
    QIMG_STRIDE(pInFrame[m_masterIndex], 0),
    QIMG_SCANLINE(pInFrame[m_masterIndex], 0),
    QIMG_WIDTH(pInFrame[m_slaveIndex], 0),
    QIMG_HEIGHT(pInFrame[m_slaveIndex], 0),
    QIMG_STRIDE(pInFrame[m_slaveIndex], 0),
    QIMG_SCANLINE(pInFrame[m_slaveIndex], 0),
    QIMG_WIDTH(pOutFrame[0], 0),
    QIMG_HEIGHT(pOutFrame[0], 0),
    QIMG_STRIDE(pOutFrame[0], 0),
    QIMG_SCANLINE(pOutFrame[0], 0));

  QIMG_LOCK(&m_lock);
  if (FALSE != m_bokehInitialized && NULL != m_pBokehEffects
    && m_wrapperInitdone)
  {
    //TODO: Need to modify the parameters for RTB Zoom usecase
    bokeh_crop.offset_x = m_goodROISrcX;
    bokeh_crop.offset_y = m_goodROISrcY;
    bokeh_crop.output_window_width = m_goodROISrcWidth;
    bokeh_crop.output_window_height = m_goodROISrcHeight;
    bokeh_crop.module = BOKEH_FOV;

    rc = m_frameOps.set_meta(m_pMasterMeta,
      IMG_META_QDC_CROP, &bokeh_crop);
    if (IMG_ERROR(rc))
    {
      IDBG_ERROR("Set Meta result for IMG_META_QDC_CROP failed");
      return rc;
    }

    eglMakeCurrent(m_display, m_surface, m_surface, m_context);
    depthmap_latency = (pInFrame[m_masterIndex]->timestamp
      - algoframe_timestamp)  / 1000;
    if (FALSE == m_isDepthAlgoDisabled &&
      (m_pMasterMeta->aec_info.lux_idx < lux_index))
    {
      if(depthmap_latency > DEPTHMAP_LATENCY) {
        ddm_algo_job_t* job;
        float min_level;
        float max_level;
        float level;
        uint32_t sensorWidth = m_masterSensorWidth;
        uint32_t sensorHeight = m_masterSensorHeight;
        if (0 == sensorWidth)
        {
            sensorWidth = 4000; // Set default value
        }
        if (0 == sensorHeight)
        {
            sensorHeight = 3000; // Set default value
        }
        job = (ddm_algo_job_t *)calloc(1, sizeof(ddm_algo_job_t));
        if (job == NULL)
        {
          IDBG_ERROR("Error no memory");
          QIMG_UNLOCK(&m_lock);
          return IMG_ERR_NO_MEMORY;
        }
        job->frameId = pMeta[m_masterIndex]->frame_id;
        job->slaveDepthAddr = QIMG_ADDR(pInFrame[m_slaveIndex], 0);
        job->slaveDepthStride = QIMG_STRIDE(pInFrame[m_slaveIndex], 0);
        job->masterDepthAddr = QIMG_ADDR(pInFrame[m_masterIndex], 0);
        job->masterDepthStride = QIMG_STRIDE(pInFrame[m_masterIndex], 0);
        job->outputWidth = m_blurSizeWidth;
        job->outputHeight = m_blurSizeHeight;
        job->outputBuffer = m_blurTextureData;
        job->textureId = m_blurTextureId;
        job->slaveFocalLengthRatio =
          (m_pSlaveMeta->af_info.focal_length_ratio) >= 1.0f ?
          (m_pSlaveMeta->af_info.focal_length_ratio) : 1.0f;
        job->masterFocalLengthRatio =
          (m_pMasterMeta->af_info.focal_length_ratio) >= 1.0f ?
          (m_pMasterMeta->af_info.focal_length_ratio) : 1.0f;
        if (m_pMasterMeta->af_info.af_supported) {
          job->focusROIX =
            ((m_pMasterMeta->af_info.focused_roi.size.width / 2 +
            (m_pMasterMeta->af_info.focused_roi.pos.x)) *
            inputWidth) / sensorWidth;
           job->focusROIY =
            ((m_pMasterMeta->af_info.focused_roi.size.height / 2 +
            (m_pMasterMeta->af_info.focused_roi.pos.y)) *
            inputHeight) / sensorHeight;
        } else {
          job->focusROIX =
            ((m_pSlaveMeta->af_info.focused_roi.size.width / 2 +
            (m_pSlaveMeta->af_info.focused_roi.pos.x)) *
            inputWidth) / sensorWidth;
          job->focusROIY =
            ((m_pSlaveMeta->af_info.focused_roi.size.height / 2 +
            (m_pSlaveMeta->af_info.focused_roi.pos.y)) *
            inputHeight) / sensorHeight;
       }

        min_level = m_pMasterMeta->qdcc_cfg.rtb_cfg.blur_min_value;
        max_level = m_pMasterMeta->qdcc_cfg.rtb_cfg.blur_max_value;
        level = m_pMasterMeta->qdcc_cfg.rtb_cfg.blur_level;
        level = (level < min_level) ? min_level : level;
        level = (level > max_level) ? max_level : level;
        if (max_level == min_level)
        {
          job->blurIntensity = 0.5f;
        }
        else
        {
          job->blurIntensity = (level - min_level) / (max_level - min_level);
        }
        if (0 != img_q_count(&m_jobQueue))
        {
          IDBG_MED("Flushing job queue, too many jobs queued");
          img_q_flush_and_destroy(&m_jobQueue);
        }
        int rc = img_q_enqueue(&m_jobQueue, job);
        if (0 != m_tracingEnabled)
        {
          atrace_int(ATRACE_TAG_ALWAYS, "bokeh_ddm_enqueue", 1);
          atrace_int(ATRACE_TAG_ALWAYS, "bokeh_ddm_enqueue", 0);
        }
        IDBG_MED("Enqueued job for frameid %d", job->frameId);
        if (IMG_ERROR(rc))
        {
          IDBG_ERROR("Cannot enqueue DDM job");
        }
        else
        {
          img_q_signal(&m_jobQueue);
          algoframe_timestamp = pInFrame[m_masterIndex]->timestamp;
        }
      }
    } else {
      // Low Light Condition, no need to process bokeh algo
      if (0 != img_q_count(&m_jobQueue)) {
        IDBG_MED("Flushing job queue, too many jobs queued");
        img_q_flush_and_destroy(&m_jobQueue);
      }
      IDBG_MED("Low light Condition dont process bokeh algo");
      m_depthStatus = CAM_RTB_MSG_LOW_LIGHT;
      memset(m_blurTextureData, 0, m_blurSizeWidth * m_blurSizeHeight);
      glBindTexture(GL_TEXTURE_2D, m_blurTextureId);
      glTexSubImage2D(GL_TEXTURE_2D,
                      0,
                      0,
                      0,
                      m_blurSizeWidth,
                      m_blurSizeHeight,
                      GL_RED,
                      GL_UNSIGNED_BYTE,
                      m_blurTextureData);
    }
    sourceTexture = m_bufferMap.GetSourceTexture(
      pInFrame[m_masterIndex]->idx,
      IMG_FD(pInFrame[m_masterIndex]),
      QIMG_LEN(pInFrame[m_masterIndex], 0) + QIMG_LEN(pInFrame[m_masterIndex], 1),
      update_format,
      QIMG_WIDTH(pInFrame[m_masterIndex], 0),
      QIMG_HEIGHT(pInFrame[m_masterIndex], 0),
      QIMG_STRIDE(pInFrame[m_masterIndex], 0),
      QIMG_SCANLINE(pInFrame[m_masterIndex], 0));
    frameBuffer = m_bufferMap.GetFrameBuffer(
      pOutFrame[0]->idx ,
      IMG_FD(pOutFrame[0]),
      QIMG_LEN(pOutFrame[0], 0) + QIMG_LEN(pOutFrame[0], 1),
      update_format,
      QIMG_WIDTH(pOutFrame[0], 0),
      QIMG_HEIGHT(pOutFrame[0], 0),
      QIMG_STRIDE(pOutFrame[0], 0),
      QIMG_SCANLINE(pOutFrame[0], 0));
    if (sourceTexture == 0 || frameBuffer == 0)
    {
      IDBG_ERROR("sourceTexture and FrameBuffer are not created properly");
      flushmaps(pMeta,m_masterIndex);
      copy_input_output(pInFrame, pOutFrame, m_masterIndex);
      QIMG_UNLOCK(&m_lock);
      return IMG_SUCCESS;
    }
    if (0 != m_tracingEnabled)
    {
      atrace_begin(ATRACE_TAG_ALWAYS, "bokeh_render");
    }

    m_pBokehEffects->CropParamters(
      inputWidth,
      inputHeight,
      pMeta[m_masterIndex]->l_stream_crop.pos.x,
      pMeta[m_masterIndex]->l_stream_crop.pos.y,
      pMeta[m_masterIndex]->l_stream_crop.size.width,
      pMeta[m_masterIndex]->l_stream_crop.size.height);

    m_pBokehEffects->ConvertYUV2RGB(sourceTexture);
    RenderTexture* finalResult = SpatiallyVaryingBlur(m_pBokehEffects,
                                                      m_pInputColorMap,
                                                      m_pInputBlurRadiusMap,
                                                      47);
    if (finalResult == NULL)
    {
      IDBG_ERROR("Internal problem in SpatiallyVaryingBlur");
      flushmaps(pMeta,m_masterIndex);
      return IMG_ERR_FRAME_DROP;
    }
    m_pBokehEffects->ConvertRGB2YUV(finalResult->texture, frameBuffer);
    glFinish();
    if (0 != m_tracingEnabled)
    {
      atrace_end(ATRACE_TAG_ALWAYS);
    }
    flushmaps(pMeta,m_masterIndex);
    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
      EGL_NO_CONTEXT);
  }
  else
  {
    if(pMeta[m_masterIndex]->l_stream_crop.pos.x > 0 ||
      pMeta[m_masterIndex]->l_stream_crop.pos.y > 0 ||
      (pMeta[m_masterIndex]->l_stream_crop.size.width <
      QIMG_WIDTH(pInFrame[m_masterIndex], 0)) ||
      (pMeta[m_masterIndex]->l_stream_crop.size.height <
      QIMG_HEIGHT(pInFrame[m_masterIndex], 0)))
    {
      IDBG_LOW("Switching with RTB Zoom enabled drop frame");
      QIMG_UNLOCK(&m_lock);
      return IMG_ERR_FRAME_DROP;
    } else {
      copy_input_output(pInFrame, pOutFrame, m_masterIndex);
    }
  }


  if (FALSE != m_dumpingEnabled)
  {
    char prop[PROPERTY_VALUE_MAX];
    property_get("persist.vendor.camera.rtb.dump", prop, "0");
    m_dumpNow = atoi(prop);
    if (FALSE != m_dumpNow && m_dumpCount < 10)
    {
      char name_w[32];
      char name_t[32];
      char name_o[32];
      char focal_w[64];
      char focal_t[64];
      snprintf(name_w,
               32,
               "aux_%dx%d_%03d",
               pInFrame[m_slaveIndex]->frame[0].plane[0].stride,
               pInFrame[m_slaveIndex]->frame[0].plane[0].scanline,
               m_dumpCount);
      snprintf(name_t,
               32,
               "primary_%dx%d_%03d",
               pInFrame[m_masterIndex]->frame[0].plane[0].stride,
               pInFrame[m_masterIndex]->frame[0].plane[0].scanline,
               m_dumpCount);
      snprintf(name_o,
               32,
               "output_%dx%d_%03d",
               pOutFrame[0]->frame[0].plane[0].stride,
               pOutFrame[0]->frame[0].plane[0].scanline,
               m_dumpCount);
      m_frameOps.dump_frame(pInFrame[m_slaveIndex], name_w, 0, NULL);
      m_frameOps.dump_frame(pInFrame[m_masterIndex], name_t, 0, NULL);
      m_frameOps.dump_frame(pOutFrame[0], name_o, 0, NULL);

      if (m_pSlaveMeta && m_pMasterMeta)
      {
        snprintf(focal_w,
                 64,
                 QCAMERA_DUMP_FRM_LOCATION"focal_length_aux_%03d.txt",
                 m_dumpCount);
        snprintf(focal_t,
                 64,
                 QCAMERA_DUMP_FRM_LOCATION"focal_length_primary_%03d.txt",
                 m_dumpCount);
        FILE *fout_w = fopen(focal_w, "wb");
        FILE *fout_t = fopen(focal_t, "wb");
        if (NULL != fout_w)
        {
            fprintf(fout_w, "%f", m_pSlaveMeta->af_info.focal_length_ratio);
            fclose(fout_w);
        }
        if (NULL != fout_t)
        {
            fprintf(fout_t, "%f", m_pMasterMeta->af_info.focal_length_ratio);
            fclose(fout_t);
        }
      }
      m_dumpCount++;
    }
    else if (FALSE == m_dumpNow)
    {
      m_dumpCount = 0;
    }
  }

  if (m_pSlaveMeta)
  {
    slaveRTBResult.depth_effect_msg = m_depthStatus;
    rc = m_frameOps.set_meta(m_pSlaveMeta,
                             IMG_META_BOKEH_RESULT,
                             (void *)&slaveRTBResult);
    if (IMG_ERROR(rc))
    {
      IDBG_ERROR("Set Meta result for wide failed");
      return rc;
    }
  }
  if (m_pMasterMeta)
  {
    masterRTBResult.depth_effect_msg = m_depthStatus;
    rc = m_frameOps.set_meta(m_pMasterMeta,
                             IMG_META_BOKEH_RESULT,
                             (void *)&masterRTBResult);
    if (IMG_ERROR(rc))
    {
      IDBG_ERROR("Set Meta result for tele failed");
      return rc;
    }
  }

  m_frameNum++;
  QIMG_UNLOCK(&m_lock);
  IDBG_LOW(": X ret");

  if (0 != m_tracingEnabled)
  {
    atrace_end(ATRACE_TAG_ALWAYS);
  }
  return IMG_SUCCESS;
}
