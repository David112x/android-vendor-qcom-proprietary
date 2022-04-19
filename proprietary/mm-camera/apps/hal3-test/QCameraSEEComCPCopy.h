////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  QCameraSEEComCPCopy.h
/// @brief Copy Secure Content
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __QCAMERA_SEECOM_CP_COPY__
#define __QCAMERA_SEECOM_CP_COPY__

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
#include "cp_utils.h"
#include <dlfcn.h>
#include "content_protection_copy.h"

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

class QCameraSEEComCPCopy {
    void *                      mLibHandle;
    struct QSEECom_handle *     l_QSEEComHandle;

    QCameraSEEComCPCopy();
    ~QCameraSEEComCPCopy();

public:

    typedef SampleClientResult (*Content_Protection_Set_AppName_Func)
                                (const char*);

    typedef SampleClientResult (*Content_Protection_Copy_Init_Func)
                                (struct QSEECom_handle**) ;

    typedef SampleClientResult (*Content_Protection_Copy_Terminate_Func)
                                (struct QSEECom_handle**);

    typedef SampleClientResult (*Content_Protection_Copy_Func)
                                (struct QSEECom_handle*, uint8*, const uint32,
                                 uint32, uint32, uint32*, uint32);

    Content_Protection_Set_AppName_Func         mContent_Protection_Set_AppName;
    Content_Protection_Copy_Init_Func           mContent_Protection_Copy_Init;
    Content_Protection_Copy_Terminate_Func      mContent_Protection_Copy_Terminate;
    Content_Protection_Copy_Func                mContent_Protection_Copy;

    QCameraSecureCopyResult Secure_to_Non_Secure(
                                void *non_secure_buf_ptr,
                                unsigned int BuffLen,
                                uint32 Sec_Ion_fd);

    static QCameraSEEComCPCopy * GetInstance();
};

#endif
