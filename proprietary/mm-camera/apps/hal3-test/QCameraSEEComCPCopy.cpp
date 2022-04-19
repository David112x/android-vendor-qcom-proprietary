////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  QCameraSEEComCPCopy.cpp
/// @brief Copy Secure Content
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "QCameraSEEComCPCopy.h"

const char* CP_COPY_LIB  = "libcontentcopy.so";
QCameraSEEComCPCopy::QCameraSEEComCPCopy()
    :mLibHandle(dlopen(CP_COPY_LIB, RTLD_NOW)),
     l_QSEEComHandle(NULL)
{
    SampleClientResult ret = SAMPLE_CLIENT_SUCCESS;
    const char * appname = "smpcpyap";
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

        mContent_Protection_Set_AppName =
            (Content_Protection_Set_AppName_Func)dlsym(mLibHandle, "Content_Protection_Set_AppName");
        ALOGD("%s: %p mContent_Protection_Set_AppName = %p", __FUNCTION__, &mContent_Protection_Set_AppName, mContent_Protection_Set_AppName);

        if(NULL == mContent_Protection_Set_AppName)
        {
            ALOGE("Error: loading symbol \
                 Content_Protection_Set_AppName failed . exit !");
            break;
        }

        mContent_Protection_Copy_Init =
            (Content_Protection_Copy_Init_Func)dlsym(mLibHandle, "Content_Protection_Copy_Init");
        if(NULL == mContent_Protection_Copy_Init)
        {
            ALOGE("Error: loading symbol \
                 Content_Protection_Copy_Init failed . exit !");
            break;
        }

        mContent_Protection_Copy_Terminate =
            (Content_Protection_Copy_Terminate_Func)dlsym(mLibHandle, "Content_Protection_Copy_Terminate");
        if(NULL == mContent_Protection_Copy_Terminate)
        {
            ALOGE("Error: loading symbol \
                 Content_Protection_Copy_Terminate failed . exit !");
            break;
        }

        mContent_Protection_Copy =
            (Content_Protection_Copy_Func)dlsym(mLibHandle, "Content_Protection_Copy");
        if(NULL == mContent_Protection_Copy)
        {
            ALOGE("Error: loading symbol \
                  Content_Protection_Copy failed . exit !");
            break;
        }

        ret = mContent_Protection_Set_AppName(appname);
        if(ret != SAMPLE_CLIENT_SUCCESS)
        {
            ALOGE("%s Error: Content_Protection_Set_AppName failed with ret = 0x%x", __FUNCTION__, ret);
            break;
        }

        ret = mContent_Protection_Copy_Init(&l_QSEEComHandle);
        if(ret != SAMPLE_CLIENT_SUCCESS)
        {
            ALOGE("%s Error: Content_Protection_Copy_Init failed with ret = 0x%x", __FUNCTION__, ret);
            break;
        }
    } while(0);
    ALOGD("%s:X", __FUNCTION__);
}

QCameraSEEComCPCopy::~QCameraSEEComCPCopy()
{
    SampleClientResult ret = SAMPLE_CLIENT_SUCCESS;
    ALOGD("%s:E", __FUNCTION__);
    /* terminate app */
    if (l_QSEEComHandle != NULL)
    {
        ret = mContent_Protection_Copy_Terminate(&l_QSEEComHandle);
        if(ret != SAMPLE_CLIENT_SUCCESS)
        {
            LOGE("%s Error: Content_Protection_Copy_Terminate failed with ret = 0x%x", __FUNCTION__, ret);
        }
    }
    else
    {
        ALOGE("%s Error: Invalid QSEECom handle", __FUNCTION__);
    }
    ALOGD("%s:X", __FUNCTION__);
}

QCameraSEEComCPCopy * QCameraSEEComCPCopy::GetInstance()
{
        static QCameraSEEComCPCopy mInstance;
        return &mInstance;
}

QCameraSecureCopyResult QCameraSEEComCPCopy::Secure_to_Non_Secure(void *non_secure_buf_ptr,
                                                        unsigned int BuffLen,
                                                        uint32 Sec_Ion_fd)
{
    unsigned int outBuffLen = 0;
    SampleClientResult ret = SAMPLE_CLIENT_SUCCESS;
    ret = mContent_Protection_Copy(l_QSEEComHandle,
           (uint8 *)non_secure_buf_ptr, BuffLen, Sec_Ion_fd, 0,
           &outBuffLen, SAMPLECLIENT_COPY_SECURE_TO_NONSECURE);
    if (ret != SAMPLE_CLIENT_SUCCESS)
    {
        LOGE("%s Error: Content_Protection_Copy secure to non-secure failed.", __FUNCTION__);
        ret = SAMPLE_CLIENT_ERROR_COPY_FAILED;
    }
    return (QCameraSecureCopyResult)ret;
}
