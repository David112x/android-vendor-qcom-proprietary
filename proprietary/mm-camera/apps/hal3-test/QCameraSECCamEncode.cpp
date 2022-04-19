////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  QCameraSECCamEncode.cpp
/// @brief Copy Secure Content
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "QCameraSECCamEncode.h"

const char* SECCAM_ENCODE_LIB  = "libseccamenc.so";

QCameraSECCamEncode::QCameraSECCamEncode()
    :mLibHandle(dlopen(SECCAM_ENCODE_LIB, RTLD_NOW)),
     l_QSEEComHandle(NULL)
{
    long ret = SCE_SUCCESS;
    ALOGD("%s:E", __FUNCTION__);

     /* Initialize functions */
    do
    {
        ALOGD("%s: Opening API libraries", __FUNCTION__);
        if(NULL == mLibHandle)
        {
             ALOGE("Error: Opening copy libs failed. exits !");
             break;
        }

        mSecure_Camera_Encode_Init =
            (Secure_Camera_Encode_Init)dlsym(mLibHandle, "secure_camera_encode_init");
        if(NULL == mSecure_Camera_Encode_Init)
        {
            ALOGE("Error: loading symbol \
                 Secure_Camera_Encode_Init failed . exit !");
            break;
        }

        mSecure_Camera_Encode_Terminate =
            (Secure_Camera_Encode_Terminate)dlsym(mLibHandle, "secure_camera_encode_terminate");
        if(NULL == mSecure_Camera_Encode_Terminate)
        {
            ALOGE("Error: loading symbol \
                 Secure_Camera_Encode_Terminate failed . exit !");
            break;
        }

        mSecure_Camera_Encode_Copy =
            (Secure_Camera_Encode_Copy)dlsym(mLibHandle, "secure_camera_encode_copy");
        if(NULL == mSecure_Camera_Encode_Copy)
        {
            ALOGE("Error: loading symbol \
                  Secure_Camera_Encode_Copy failed . exit !");
            break;
        }

        ret = mSecure_Camera_Encode_Init(&l_QSEEComHandle, "seccamenc");
        if(ret != SCE_SUCCESS)
        {
            ALOGE("%s Error: Content_Protection_Copy_Init failed with ret = 0x%x", __FUNCTION__, ret);
            break;
        }
    } while(0);
    ALOGD("%s:X", __FUNCTION__);
}

QCameraSECCamEncode::~QCameraSECCamEncode()
{
    long ret = SCE_SUCCESS;
    ALOGD("%s:E", __FUNCTION__);
    /* terminate app */
    if (l_QSEEComHandle != NULL)
    {
        ret = mSecure_Camera_Encode_Terminate(&l_QSEEComHandle);
        if(ret != SCE_SUCCESS)
        {
            ALOGE("%s Error: Content_Protection_Copy_Terminate failed with ret = 0x%x", __FUNCTION__, ret);
        }
    }
    else
    {
        ALOGE("%s Error: Invalid QSEECom handle", __FUNCTION__);
    }
    ALOGD("%s:X", __FUNCTION__);
}

QCameraSECCamEncode * QCameraSECCamEncode::GetInstance()
{
        static QCameraSECCamEncode mInstance;
        return &mInstance;
}

QCameraSecureCopyResult QCameraSECCamEncode::Secure_to_Non_Secure(
                                    BufferInfo * pNonSecBufferinfo,
                                    BufferInfo * pSecBufferinfo)
{
    unsigned int outBuffLen = 0;
    long ret = SCE_SUCCESS;
    ALOGI("BuffLen = %d", pNonSecBufferinfo->size);
    ALOGI("Sec_Ion_fd = %d", pSecBufferinfo->fd);
    ALOGI("non_secure_fd = %d", pNonSecBufferinfo->fd);
    outBuffLen = pNonSecBufferinfo->size;
    if (mSecure_Camera_Encode_Copy != NULL && l_QSEEComHandle != NULL)
    {
        ret = mSecure_Camera_Encode_Copy(l_QSEEComHandle, pNonSecBufferinfo->fd,
                pNonSecBufferinfo->size, pSecBufferinfo->fd, &outBuffLen, SCE_COPY_SECURE_TO_NONSECURE);
        if (ret != SCE_SUCCESS)
        {
            ALOGE("%s Error: Secure_Camera_Encode_Copy secure to non-secure failed.", __FUNCTION__);
            ret = SCE_ERROR_COPY_FAILED;
        }
    }
    else
    {
        ret = SCE_ERROR_INIT_FAILED;
    }
    ALOGI("outBuffLen = %d", outBuffLen);

    return (QCameraSecureCopyResult)ret;
}
