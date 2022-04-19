////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  QCameraSECCamEncode.h
/// @brief Copy Secure Content
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __QCAMERA_SEECAM_ENCODE__
#define __QCAMERA_SEECAM_ENCODE__

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <linux/msm_ion.h>

#if TARGET_ION_ABI_VERSION >= 2
//#include <ion/ion.h>
#endif
#include <linux/dma-buf.h>

#ifdef ANDROID
#include <utils/Log.h>
#endif
#include "QSEEComAPI.h"
#include <sys/mman.h>
#include <getopt.h>
#include <dlfcn.h>
#include "SecCamEnc.h"
#include "BufferManager.h"

#define SCE_COPY_SECURE_TO_NONSECURE 0

typedef enum QCameraSecureCopyResult {
  QCAMERA_SECCOPY_SUCCESS                           = 0,
  QCAMERA_SECCOPY_ERROR_COPY_FAILED                 = 1,
  QCAMERA_SECCOPY_ERROR_INIT_FAILED                 = 2,
  QCAMERA_SECCOPY_ERROR_TERMINATE_FAILED            = 3,
  QCAMERA_SECCOPY_ERROR_ION_MALLOC_FAILED           = 4,
  QCAMERA_SECCOPY_ERROR_ION_FREE_FAILED             = 5,
  QCAMERA_SECCOPY_ERROR_NSS_COPY_FAILED             = 6,
  QCAMERA_SECCOPY_ERROR_SNS_COPY_FAILED             = 7,
  QCAMERA_SECCOPY_ERROR_MEM_SEG_COPY_FAILED         = 8,
  QCAMERA_SECCOPY_ERROR_INVALID_PARAMS              = 9,
  QCAMERA_SECCOPY_ERROR_FEATURE_NOT_SUPPORT         = 10,
  QCAMERA_SECCOPY_ERROR_BUFFER_TOO_SHORT            = 11,
  QCAMERA_SECCOPY_ERROR_SECURE_ION_MALLOC_FAILED    = 12,
  QCAMERA_SECCOPY_ERROR_FEATURE_NOT_SUPPORTED       = 13,
  QCAMERA_SECCOPY_FAILURE = 0x7FFFFFFF
} QCameraSecureCopyResult;

class QCameraSECCamEncode {
    void *                      mLibHandle;
    void *     l_QSEEComHandle;

    QCameraSECCamEncode();
    ~QCameraSECCamEncode();

public:

    typedef long (*Secure_Camera_Encode_Init)(void **, const char*  appName);
    typedef long (*Secure_Camera_Encode_Terminate)(void **);
    typedef long (*Secure_Camera_Encode_ION_Secure_Malloc)(int*,
                        struct ion_fd_data*,
                        uint32);
    typedef long (*Secure_Camera_Encode_Copy)(void *,
                        uint32,
                        const uint32,
                        uint32,
                        uint32*,
                        uint32);

    Secure_Camera_Encode_Init                   mSecure_Camera_Encode_Init;
    Secure_Camera_Encode_Terminate              mSecure_Camera_Encode_Terminate;
    Secure_Camera_Encode_Copy                   mSecure_Camera_Encode_Copy;

    QCameraSecureCopyResult Secure_to_Non_Secure(
                        BufferInfo * pNonSecBufferinfo,
                        BufferInfo * pSecBufferinfo);

    static QCameraSECCamEncode * GetInstance();
};


#endif
