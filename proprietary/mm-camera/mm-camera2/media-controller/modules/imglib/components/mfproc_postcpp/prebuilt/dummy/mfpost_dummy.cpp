/**********************************************************************
*  Copyright (c) 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <stdint.h>
extern "C" {
  #include "libmfpostdummy.h"
}

#if defined (_ANDROID_)
#include <android/log.h>
#define  QUADDUMMY_LOG_TAG    "mfpost_dummy"
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

void mfpostDummyProc()
{
  QUADDUMMY_LOGE("inside dummy PostProc");
  return;
}

void mfpostdummy_init()
{
  QUADDUMMY_LOGE("inside dummy mfpostdummy_init");
  return;
}

int32_t mfpostdummy_process()
{
  QUADDUMMY_LOGE("inside dummy postproc process");
  return 0;
}

void mfpostdummy_deinit()
{
  QUADDUMMY_LOGE("inside dummy mfpostdummy_deinit");
  return;
}
