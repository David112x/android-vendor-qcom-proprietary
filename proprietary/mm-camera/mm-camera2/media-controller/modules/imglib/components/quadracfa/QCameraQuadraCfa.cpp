/**********************************************************************
*  Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#define LOG_TAG "QCameraQuadraCfa"

#include "QCameraQuadraCfa.h"
#include <sys/types.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <utils/Log.h>
#include <assert.h>
#include <sys/time.h>
#include <fcntl.h>
#include <cutils/properties.h>
#include <android/log.h>

extern "C" {
  #include "libremosaiclib.h"
}

#define LOG_DEBUG

#undef CDBG
#undef CDBG_ERROR
#ifdef _ANDROID_
  #define CDBG_ERROR(fmt, args...) \
    ALOGE("%s:%d " fmt "\n", __func__, __LINE__, ##args)
  #ifdef LOG_DEBUG
  #define CDBG(fmt, args...) \
    ALOGD("%s:%d " fmt "\n", __func__, __LINE__, ##args)
  #else
  #define CDBG(fmt, args...) do {} while(0)
  #endif
#else
  #define CDBG_ERROR(fmt, args...) \
    fprintf(stderr, "%s:%d "fmt"\n", __func__, __LINE__, ##args)
  #ifdef LOG_DEBUG
  #define CDBG(fmt, args...) \
    fprintf(stderr, "%s:%d "fmt"\n", __func__, __LINE__, ##args)
  #else
  #define CDBG(fmt, args...) do {} while(0)
  #endif
#endif

#define REMOSAIC_LIB_NAME "libremosaiclib.so"

#define EEPROM_BUF_SIZE 2048

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

  QCameraQuadraCfa *pQuadraCfa;
  pQuadraCfa = new QCameraQuadraCfa;
  CDBG("new QCameraQuadraCfa %p", pQuadraCfa);
  if (pQuadraCfa) {
    *p_context = pQuadraCfa;
  } else {
    ret = IMG_ERR_GENERAL;
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

  QCameraQuadraCfa* pQuadraCfa = (QCameraQuadraCfa*) p_context;
  delete pQuadraCfa;

  CDBG(": X");
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_preload
 *
 * Description: algorithm pre-allocations
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
  CDBG(": E");
  QCameraQuadraCfa::PresetConfig(p_ops, p_params);
  CDBG(": X");
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_shutdown
 *
 * Description: algorithm cleanup of preallocated resources
 *
 * Arguments:
 *  @p_ops: base Ops
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_shutdown(img_frame_ops_t *p_ops)
{
  CDBG(": E");
  IMG_UNUSED(p_ops);

  remosaic_deinit();

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
  CDBG("QuadraCFA ProcessFrame %d, %d", in_frame_cnt, out_frame_cnt);

  CDBG(": E");
  int ret = IMG_SUCCESS;
  if (p_context != NULL) {
    QCameraQuadraCfa* pQuadraCfa = (QCameraQuadraCfa*) p_context;
    ret = pQuadraCfa->processData(p_in_frame, in_frame_cnt, p_out_frame,
      out_frame_cnt, (p_meta && (meta_cnt > 0)) ? p_meta[0] : NULL);
  } else {
    CDBG_ERROR(": img_algo_process failed");
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
 *   @p_context: ptr to instance
 *   @p_ops: ptr to frame ops
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_set_frame_ops(void *p_context, img_frame_ops_t *p_ops)
{
  CDBG(": E");
  int ret = IMG_SUCCESS;
  assert(NULL != p_context);
  assert(NULL != p_ops);
  QCameraQuadraCfa* instance = (QCameraQuadraCfa*) p_context;
  instance->setFrameOps(p_ops);
  CDBG(": X");
  return ret;
}

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
QCameraQuadraCfa::QCameraQuadraCfa()
{
  CDBG_ERROR(": E");

  memset(&mFrameOps, 0x0, sizeof(mFrameOps));
  mBufCnt = 0;

#ifdef REMOSAIC_DYNAMIC_LOAD
  mLibPtr = dlopen(REMOSAIC_LIB_NAME, RTLD_NOW);
  if (!mLibPtr) {
    CDBG_ERROR("could not load %s", REMOSAIC_LIB_NAME);
    mQuadraCfaInitialized = false;
    return;
  }
  CDBG_ERROR("lib %s -- mLibPtr %p", REMOSAIC_LIB_NAME, mLibPtr);
  mRun4PixelProc = NULL;
  mRun4PixelProc =
    (void (*)(unsigned char*, unsigned short*, int, int, int, int, int, int,
    int, int*))
    dlsym(mLibPtr, "Run4PixelProc" );

  if (!mRun4PixelProc) {
    CDBG_ERROR("Error linking Run4PixelProc");
    dlclose(mLibPtr);
    mQuadraCfaInitialized = false;
    mLibPtr = NULL;
    return;
  }

  CDBG_ERROR("lib %s -- mLibPtr %p , func %p", REMOSAIC_LIB_NAME, mLibPtr,
    mRun4PixelProc);
#endif//  #ifdef REMOSAIC_DYNAMIC_LOAD

  mQuadraCfaInitialized = true;

  CDBG_ERROR(": X");
}

/**
 * Function: ~QCameraQuadraCfa
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
QCameraQuadraCfa::~QCameraQuadraCfa()
{
#ifdef REMOSAIC_DYNAMIC_LOAD
  if (mQuadraCfaInitialized) {
    CDBG_ERROR("closing %s", REMOSAIC_LIB_NAME);
    dlclose(mLibPtr);
    mLibPtr = NULL;
    mQuadraCfaInitialized = false;
    return;
  }
#endif //#ifdef REMOSAIC_DYNAMIC_LOAD
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
 *   IMG_SUCCESS or IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int8_t QCameraQuadraCfa::processData(img_frame_t *inFrame[],
  int inFrameCnt, img_frame_t *outFrame[], int outFrameCnt,
  img_meta_t *p_meta)
{
  CDBG(": E");
  int ret = IMG_SUCCESS;
  img_meta_t *p_frame_meta;
  IMG_UNUSED(inFrameCnt);
  IMG_UNUSED(outFrameCnt);

  img_awb_info_t *p_awb_info = (img_awb_info_t *)mFrameOps.get_meta(p_meta,
    IMG_META_AWB_INFO);
  if (p_awb_info) {
    CDBG("QCameraQuadraCfa: Recieved valid meta IMG_META_AWB_INFO %f %f %f",
      p_awb_info->r_gain, p_awb_info->g_gain, p_awb_info->b_gain);
  } else {
    CDBG_ERROR("QCameraQuadraCfa: failed to get IMG_META_AWB_INFO");
  }

  CDBG(" I[%p, %d %d, %d %d, %d, type %d] "
    "O[%p, %d %d, %d %d, %d, type %d]",
    QIMG_ADDR(inFrame[0], 0), QIMG_WIDTH(inFrame[0], 0),
    QIMG_HEIGHT(inFrame[0], 0), QIMG_STRIDE(inFrame[0], 0),
    QIMG_SCANLINE(inFrame[0], 0), QIMG_LEN(inFrame[0], 0),
    QIMG_PL_TYPE(inFrame[0], 0), QIMG_ADDR(outFrame[0], 0),
    QIMG_WIDTH(outFrame[0], 0), QIMG_HEIGHT(outFrame[0], 0),
    QIMG_STRIDE(outFrame[0], 0), QIMG_SCANLINE(outFrame[0], 0),
    QIMG_LEN(outFrame[0], 0), QIMG_PL_TYPE(outFrame[0], 0));

  CDBG(" out buffer %p in buffer %p , width %d , height %d  func %p",
    outFrame[0]->frame[0].plane[0].addr,
    inFrame[0]->frame[0].plane[0].addr,
    inFrame[0]->frame[0].plane[0].width,
    inFrame[0]->frame[0].plane[0].height,
    mRun4PixelProc);

  int wb_grgain = 1024;
  int wb_rgain = 1024;
  int wb_bgain = 1024;
  int wb_gbgain = 1024;
  if (p_awb_info) {
    int wb_grgain = p_awb_info->g_gain * 1024;
    int wb_rgain = p_awb_info->r_gain*1024;
    int wb_bgain = p_awb_info->b_gain*1024;
    int wb_gbgain = p_awb_info->g_gain*1024;
  }
  int analog_gain = 1;
  int pedestal = 64;
  int mode = 0;
  int* xtalk_gain_map = NULL;
  int port0 = 0;
  int port1 = 0;
  int port2 = 0;
  int port3 = 0;
  int byr_order = 0;

  int lib_status = 0;

  char prop[PROPERTY_VALUE_MAX];
  int temp = 0;
  property_get("persist.vendor.camera.quad.anagain", prop, "0");
  temp = (int)atoi(prop);
  if (temp > 0 ) {
    analog_gain = temp;
    CDBG_ERROR(":quadcfa: + analog_gain %d", analog_gain);
  }
  property_get("persist.vendor.camera.quad.byrord", prop, "0");
  temp = (int)atoi(prop);
  if (temp > 0 ) {
    byr_order = temp;
    CDBG_ERROR(":quadcfa: + byr_order %d", byr_order);
  }
  property_get("persist.vendor.camera.quad.rem.mode", prop, "0");
  temp = (int)atoi(prop);
  if (temp > 0 ) {
    mode = temp;
    CDBG_ERROR(":quadcfa: + mode %d", mode);
  }

  mFrameOps.dump_frame(inFrame[0], "Quadracfa_input", 0, NULL);

  #ifndef REMOSAIC_DYNAMIC_LOAD

  st_remosaic_param param;
  param.wb_r_gain       = wb_grgain;
  param.wb_gr_gain      = wb_rgain;
  param.wb_gb_gain      = wb_bgain;
  param.wb_b_gain       = wb_gbgain;
  param.analog_gain     = analog_gain;

  remosaic_process_param_set(&param);

  remosaic_process(inFrame[0]->frame[0].plane[0].fd,
    inFrame[0]->frame[0].plane[0].length,
    outFrame[0]->frame[0].plane[0].fd,
    outFrame[0]->frame[0].plane[0].length);

  property_get("persist.vendor.camera.quad.dummy", prop, "0");
  temp = (int)atoi(prop);
  if (temp > 0 ) {
    CDBG_ERROR("QUADCFA: Dummy Unpacking");
    uint8_t* pOut = (uint8_t*)(outFrame[0]->frame[0].plane[0].addr);
    uint8_t* pIn = (uint8_t*)(inFrame[0]->frame[0].plane[0].addr);
    uint32_t count = (outFrame[0]->frame[0].plane[0].width *
      outFrame[0]->frame[0].plane[0].height * 5) / 4;
    for (uint32_t i = 0; i < count; i = i+5) {
      *pOut = ((*(pIn+4)) & 0b00000011) |  ((*pIn & 0b00111111)<<2);
      pOut ++;
      *pOut = ((*pIn) & 0b11000000) >> 6;
      pOut ++;

      *pOut = (((*(pIn+4)) & 0b00001100) >> 2) |  ((*(pIn+1) & 0b00111111)<<2);
      pOut ++;
      *pOut = ((*(pIn+1)) & 0b11000000) >> 6;
      pOut ++;

      *pOut = (((*(pIn+4)) & 0b00110000) >> 4) |  ((*(pIn+2) & 0b00111111)<<2);
      pOut ++;
      *pOut = ((*(pIn+2)) & 0b11000000) >> 6;
      pOut ++;

      *pOut = (((*(pIn+4)) & 0b11000000) >> 6) |  ((*(pIn+3) & 0b00111111)<<2);
      pOut ++;
      *pOut = ((*(pIn+3)) & 0b11000000) >> 6;
      pOut ++;

      pIn = pIn + 5;
    }
    /* Sleep here to mimic actual remosaic as actual remosaic lib will take time*/
    usleep(500000);
  }

//Run4PixelProc((unsigned char*)inFrame[0]->frame[0].plane[0].addr,
//  (unsigned short*) outFrame[0]->frame[0].plane[0].addr,
//  inFrame[0]->frame[0].plane[0].width,
//  inFrame[0]->frame[0].plane[0].height,
//  wb_grgain,
//  wb_rgain,
//  wb_bgain,
//  wb_gbgain,
//  analog_gain,
//  pedestal,
//  byr_order,
//  xtalk_gain_map,
//  mode,
//  &lib_status,
//  &port0,
//  &port1,
//  &port2,
//  &port3);

  #else
  mRun4PixelProc( inFrame[0]->frame[0].plane[0].addr,
    (unsigned short*) outFrame[0]->frame[0].plane[0].addr,
    inFrame[0]->frame[0].plane[0].width,
    inFrame[0]->frame[0].plane[0].height,
    wb_grgain, wb_rgain,  wb_bgain, wb_gbgain, analog_gain, &port0);
  #endif

  mFrameOps.dump_frame(outFrame[0], "Quadracfa_output", 0, NULL);

  CDBG(": X ret %d", ret);
  return IMG_SUCCESS;
}

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
int32_t QCameraQuadraCfa::PresetConfig(img_base_ops_t *aOps, void *p_params)
{
  CDBG(": E");
  IMG_UNUSED(aOps);

  img_preload_param_t *p_preload_param = (img_preload_param_t*)p_params;
  CDBG(": custom_cal_data  %0x", p_preload_param->custom_cal_data);
  CDBG(": full_height  %0x", p_preload_param->full_s_dim.height);
  CDBG(": full_width  %0x", p_preload_param->full_s_dim.width);
  CDBG(": filter_arrangement  %0x", p_preload_param->filter_arrangement);
  CDBG(": pedestal  %0x", p_preload_param->pedestal);

  remosaic_init(p_preload_param->full_s_dim.width,
    p_preload_param->full_s_dim.height,
    (e_remosaic_bayer_order)p_preload_param->filter_arrangement,
    p_preload_param->pedestal);

  size_t eeprom_size = EEPROM_BUF_SIZE;
  uint8_t temp_custom_data[EEPROM_BUF_SIZE];
  if (NULL == p_preload_param->custom_cal_data) {
    p_preload_param->custom_cal_data = (uint8_t*)temp_custom_data;
  }
  remosaic_gainmap_gen(p_preload_param->custom_cal_data, eeprom_size);

  CDBG(": X");
  return IMG_SUCCESS;

}
