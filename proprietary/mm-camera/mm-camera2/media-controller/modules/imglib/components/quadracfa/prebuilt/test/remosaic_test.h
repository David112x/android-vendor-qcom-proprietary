/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef _REMOSAIC_TEST_H_
#define _REMOSAIC_TEST_H_

#define REMOSAIC_DEBUG_TEST 1

#if defined (_ANDROID_)
#include <android/log.h>
#define  EXAMPLE_LOG_TAG    "quadtest"
#define RT_LOGE(...) \
  do { if (REMOSAIC_DEBUG_TEST) __android_log_print( \
  ANDROID_LOG_ERROR,EXAMPLE_LOG_TAG,__VA_ARGS__); \
  } while (0)
#define RT_LOGI(...) \
  do { if (REMOSAIC_DEBUG_TEST) __android_log_print( \
  ANDROID_LOG_INFO,EXAMPLE_LOG_TAG,__VA_ARGS__); \
  } while (0)
#else
#define RT_LOGE(...) \
  do { if (REMOSAIC_DEBUG_TEST) \
  fprintf(stderr,__VA_ARGS__); \
  } while (0)
#define RT_LOGI(...) \
  do { if (REMOSAIC_DEBUG_TEST) \
  fprintf(stderr,__VA_ARGS__); \
  } while (0)
#endif

#define uint16 unsigned short
#include <stdio.h>

#endif
