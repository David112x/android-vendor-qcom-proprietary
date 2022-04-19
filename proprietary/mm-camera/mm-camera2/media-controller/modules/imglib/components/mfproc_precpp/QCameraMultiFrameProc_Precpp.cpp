/**********************************************************************
*  Copyright (c) 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#define LOG_TAG "QCameraMFProcPrecpp"

#include "QCameraMultiFrameProc_Precpp.h"
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
  #include "libmfproc_dummy.h"
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

#define DUMMY_LIB_NAME "libmfproc_dummy_daemon.so"

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

  QCameraMFProcPrecpp *pMFPrecpp;
  pMFPrecpp = new QCameraMFProcPrecpp;
  CDBG("new QCameraMFProcPrecpp %p", pMFPrecpp);
  if (pMFPrecpp) {
    *p_context = pMFPrecpp;
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

  QCameraMFProcPrecpp* pMFPrecpp = (QCameraMFProcPrecpp*) p_context;
  delete pMFPrecpp;

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

  //TO DO: OEM will call library deinit here

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
  CDBG("MultiFrame Process Precpp ProcessFrame %d, %d", in_frame_cnt,
    out_frame_cnt);

  CDBG(": E");
  int ret = IMG_SUCCESS;
  if (p_context != NULL) {
    QCameraMFProcPrecpp* pMFPrecpp = (QCameraMFProcPrecpp*) p_context;
    ret = pMFPrecpp->processData(p_in_frame, in_frame_cnt, p_out_frame,
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
  QCameraMFProcPrecpp* instance = (QCameraMFProcPrecpp*) p_context;
  instance->setFrameOps(p_ops);
  CDBG(": X");
  return ret;
}

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
QCameraMFProcPrecpp::QCameraMFProcPrecpp()
{
  CDBG_ERROR(": E");

  memset(&mFrameOps, 0x0, sizeof(mFrameOps));
  mBufCnt = 0;

#ifdef DUMMY_DYNAMIC_LOAD
  //TO-DO: OEM will load their dynamic library(Custom Algorithm)
  mLibPtr = dlopen(DUMMY_LIB_NAME, RTLD_NOW);
  if (!mLibPtr) {
    CDBG_ERROR("could not load %s", DUMMY_LIB_NAME);
    mMFProcPrecppInitialized = false;
    return;
  }
  CDBG_ERROR("lib %s -- mLibPtr %p", DUMMY_LIB_NAME, mLibPtr);

  //TO-DO: OEM will link their library function
  mDummyProc = NULL;
  mDummyProc =
    (void (*)(unsigned char*, unsigned short*, int, int, int, int, int, int,
    int, int*))
    dlsym(mLibPtr, "DummyProc" );

  if (!mDummyProc) {
    CDBG_ERROR("Error linking DummyProc");
    dlclose(mLibPtr);
    mMFProcPrecppInitialized = false;
    mLibPtr = NULL;
    return;
  }

  CDBG_ERROR("lib %s -- mLibPtr %p , func %p", DUMMY_LIB_NAME, mLibPtr,
    mDummyProc);
#endif//  #ifdef DUMMY_DYNAMIC_LOAD

  mMFProcPrecppInitialized = true;

  CDBG_ERROR(": X");
}

/**
 * Function: ~QCameraMFProcPrecpp
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
QCameraMFProcPrecpp::~QCameraMFProcPrecpp()
{
#ifdef DUMMY_DYNAMIC_LOAD
  if (mMFProcPrecppInitialized) {
    //TO-DO: OEM will close their dynamic library(Custom Algorithm)
    CDBG_ERROR("closing %s", DUMMY_LIB_NAME);
    dlclose(mLibPtr);
    mLibPtr = NULL;
    mMFProcPrecppInitialized = false;
    return;
  }
#else //#ifdef DUMMY_DYNAMIC_LOAD
  mMFProcPrecppInitialized = false;
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
int8_t QCameraMFProcPrecpp::processData(img_frame_t *inFrame[],
  int inFrameCnt, img_frame_t *outFrame[], int outFrameCnt,
  img_meta_t *p_meta)
{
  CDBG(": E");
  int ret = IMG_SUCCESS;
  img_meta_t *p_frame_meta;
  IMG_UNUSED(outFrameCnt);

  CDBG(" out buffer %p in buffer %p , width %d , height %d,"
    "inFrameCnt %d, frame_length %d",
    outFrame[0]->frame[0].plane[0].addr,
    inFrame[0]->frame[0].plane[0].addr,
    inFrame[0]->frame[0].plane[0].width,
    inFrame[0]->frame[0].plane[0].height,
    inFrameCnt, IMG_FRAME_LEN(inFrame[inFrameCnt-1]));
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

  mFrameOps.dump_frame(outFrame[0], "MultiFrameProc_Precpp_output", 0, NULL);

  CDBG(": X ret %d", ret);
  return IMG_SUCCESS;
}
