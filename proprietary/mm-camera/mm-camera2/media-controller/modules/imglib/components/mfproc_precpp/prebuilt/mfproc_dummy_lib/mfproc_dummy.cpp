/**********************************************************************
*  Copyright (c) 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <stdint.h>
extern "C" {
  #include "libmfproc_dummy.h"
}

#if defined (_ANDROID_)
#include <android/log.h>
#define  QUADDUMMY_LOG_TAG    "mfproc_dummy"
#define QUADDUMMY_DEBUG_TEST 1
#define QUADDUMMY_LOGE(...) \
  do { if (QUADDUMMY_DEBUG_TEST) __android_log_print(ANDROID_LOG_ERROR, \
  QUADDUMMY_LOG_TAG, __VA_ARGS__); } while (0)
#define QUADDUMMY_LOGI(...) \
  do { if (QUADDUMMY_DEBUG_TEST) __android_log_print(ANDROID_LOG_INFO, \
  QUADDUMMY_LOG_TAG, __VA_ARGS__); } while (0)
#else
#define QUADDUMMY_LOGE(...) \
  do { if (QUADDUMMY_DEBUG_TEST) fprintf(stderr, __VA_ARGS__); } while (0)
#define QUADDUMMY_LOGI(...) \
  do { if (QUADDUMMY_DEBUG_TEST) fprintf(stderr, __VA_ARGS__); } while (0)
#endif

#define VAR_UNUSED(x) (void)(x)

void mDummyProc()
{
  QUADDUMMY_LOGE("inside dummyProc");
  return;
}

void dummy_mfproc_init()
{
  QUADDUMMY_LOGE("inside dummy mfproc_init");
  return;
}

int32_t dummy_mfproc_process()
{
  QUADDUMMY_LOGE("inside dummy mfproc_process");
  return 0;
}

void dummy_mfproc_deinit()
{
  QUADDUMMY_LOGE("inside dummy mfproc_deinit");
  return ;
}

