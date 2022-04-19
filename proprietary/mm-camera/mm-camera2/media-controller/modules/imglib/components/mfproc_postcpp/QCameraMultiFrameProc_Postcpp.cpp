/**********************************************************************
*  Copyright (c) 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#define LOG_TAG "QCameraMFProcPostcpp"

#include "QCameraMultiFrameProc_Postcpp.h"
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
  #include "libmfpostdummy.h"
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

#define DUMMY_LIB_NAME "libmfpostdummy_daemon.so"

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
 *   IMG_SUCCESS or IMG_ERR_GENERAL
 *
 * Notes: none
 **/
extern "C" int img_algo_init(void **p_context, img_init_params_t *p_params)
{
  CDBG(": E");
  int ret = IMG_SUCCESS;
  IMG_UNUSED(p_params);

  QCameraMFProcPostcpp *pMFPostcpp;
  pMFPostcpp = new QCameraMFProcPostcpp;
  CDBG("new QCameraMFProcPostcpp %p", pMFPostcpp);
  if (pMFPostcpp) {
    *p_context = pMFPostcpp;
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
 *   IMG_SUCCESS
 *
 * Notes: none
 **/
extern "C" int img_algo_deinit(void *p_context)
{
  CDBG(": E");
  assert(NULL != p_context);

  QCameraMFProcPostcpp* pMFPostcpp = (QCameraMFProcPostcpp*) p_context;
  delete pMFPostcpp;

  CDBG(": X");
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_shutdown
 *
 * Description: algorithm cleanup of preallocated resources
 *
 * Arguments:
 *   @p_ops: base Ops
 *
 * Return values:
 *   IMG_SUCCESS
 *
 * Notes: none
 **/
extern "C" int img_algo_shutdown(img_frame_ops_t *p_ops)
{
  CDBG(": E");
  IMG_UNUSED(p_ops);

  //TO DO: OEM will call their library deinit here

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
  CDBG("MultiFrameProc Postcpp ProcessFrame %d, %d", in_frame_cnt, out_frame_cnt);

  CDBG(": E");
  int ret = IMG_SUCCESS;
  if (p_context != NULL) {
    QCameraMFProcPostcpp* pMFPostcpp = (QCameraMFProcPostcpp*) p_context;
    ret = pMFPostcpp->processData(p_in_frame, in_frame_cnt, p_out_frame, out_frame_cnt,
        (p_meta && (meta_cnt > 0)) ? p_meta[0] : NULL);
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
 *   IMG_SUCCESS or IMG_ERR_GENERAL
 *
 * Notes: none
 **/
extern "C" int img_algo_set_frame_ops(void *p_context, img_frame_ops_t *p_ops)
{
  CDBG(": E");
  int ret = IMG_SUCCESS;
  assert(NULL != p_context);
  assert(NULL != p_ops);
  QCameraMFProcPostcpp* instance = (QCameraMFProcPostcpp*) p_context;
  instance->setFrameOps(p_ops);
  CDBG(": X");
  return ret;
}

/**
 * Function: QCameraMFProcPostcpp
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
QCameraMFProcPostcpp::QCameraMFProcPostcpp()
{
  CDBG_ERROR(": E");

  memset(&mFrameOps, 0x0, sizeof(mFrameOps));

#ifdef DUMMY_DYNAMIC_LOAD
  mLibPtr = dlopen(DUMMY_LIB_NAME, RTLD_NOW);
  if (!mLibPtr) {
    CDBG_ERROR("could not load %s", DUMMY_LIB_NAME);
    mMFProcPostcppInitialized = false;
    return;
  }
  CDBG_ERROR("lib %s -- mLibPtr %p", DUMMY_LIB_NAME, mLibPtr);
  mDummyProc = NULL;
  mDummyProc =
    (void (*)(unsigned char*, unsigned short*, int, int, int, int, int, int,
    int, int*))
    dlsym(mLibPtr, "mfpostDummyProc" );

  if (!mDummyProc) {
    CDBG_ERROR("Error linking mDummyProc");
    dlclose(mLibPtr);
    mMFProcPostcppInitialized = false;
    mLibPtr = NULL;
    return;
  }

  CDBG_ERROR("lib %s -- mLibPtr %p , func %p", DUMMY_LIB_NAME, mLibPtr,
    mDummyProc);
#endif//  #ifdef DUMMY_DYNAMIC_LOAD

  mMFProcPostcppInitialized = true;

  CDBG_ERROR(": X");
}

/**
 * Function: ~QCameraMFProcPostcpp
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
QCameraMFProcPostcpp::~QCameraMFProcPostcpp()
{
#ifdef DUMMY_DYNAMIC_LOAD
  if (mMFProcPostcppInitialized) {
    CDBG_ERROR("closing %s", DUMMY_LIB_NAME);
    dlclose(mLibPtr);
    mLibPtr = NULL;
    mMFProcPostcppInitialized = false;
    return;
  }
#else //#ifdef DUMMY_DYNAMIC_LOAD
    mMFProcPostcppInitialized = false;
#endif
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
 *   @arg5: p_meta: frame meta data
 *
 * Return values:
 *   IMG_SUCCESS or IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int8_t QCameraMFProcPostcpp::processData(img_frame_t *inFrame[],
  int inFrameCnt, img_frame_t *outFrame[], int outFrameCnt,
  img_meta_t *p_meta)
{
  CDBG(": E");
  int ret = IMG_SUCCESS;
  img_meta_t *p_frame_meta;
  IMG_UNUSED(outFrameCnt);

  CDBG(" out buffer %p in buffer %p , width %d , height %d  inFrameCnt %d",
    outFrame[0]->frame[0].plane[0].addr,
    inFrame[0]->frame[0].plane[0].addr,
    inFrame[0]->frame[0].plane[0].width,
    inFrame[0]->frame[0].plane[0].height,
    inFrameCnt);
#ifndef DUMMY_DYNAMIC_LOAD
  //Dummy process for demo
  //memcpy last input frame to output frame
  memcpy(IMG_ADDR(outFrame[0]), IMG_ADDR(inFrame[inFrameCnt-1]),
    IMG_FRAME_LEN(inFrame[inFrameCnt-1]));
#else
  //TO-DO: OEM will call their process function from their
  //library (Custom Algorithm)
  //Call algo function for frame process
#endif

  mFrameOps.dump_frame(outFrame[0], "MultiFrameProc_Postcpp_output", 0, NULL);

  CDBG(": X ret %d", ret);
  return IMG_SUCCESS;
}
