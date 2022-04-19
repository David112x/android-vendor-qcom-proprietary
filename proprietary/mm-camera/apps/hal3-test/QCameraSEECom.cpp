////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  QCameraSEECom.cpp
/// @brief Load QSeeCom API to Interact with TA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "QCameraSEECom.h"

#define TZ_APP_NAME             "seccamdemo64"
#define TZ_APP_BUFFER_SIZE      (8*1024)
#define TZ_APP_PATH             "/vendor/firmware_mnt/image"
#define MAX_FNAME               64
#define MAX_FNAME_EXT           4
#define MAX_PATH_NAME           80

QCameraSEECom::QCameraSEECom()
   :mLibHandle(dlopen("libQSEEComAPI.so", RTLD_NOW)),
    l_QSEEComHandle(NULL)
{
    ALOGD("E");
    int     ret = 0;
    if (NULL != mLibHandle)
    {
        ALOGD("libQSEEComAPI.so open success");
        mQSEECom_start_app = (QSEECom_start_app_func)dlsym(mLibHandle, "QSEECom_start_app");
        mQSEECom_shutdown_app = (QSEECom_shutdown_app_func)dlsym(mLibHandle, "QSEECom_shutdown_app");
        mQSEECom_start_app_V2 = (QSEECom_start_app_V2_func)dlsym(mLibHandle, "QSEECom_start_app_V2");
        mQSEECom_send_modified_cmd = (QSEECom_send_modified_cmd_func)dlsym(mLibHandle, "QSEECom_send_modified_cmd");
        mQSEECom_send_modified_cmd_64 = (QSEECom_send_modified_cmd_64_func)dlsym(mLibHandle, "QSEECom_send_modified_cmd_64");
        mQSEECom_set_bandwidth = (QSEECom_set_bandwidth_func)dlsym(mLibHandle, "QSEECom_set_bandwidth");
        mQSEECom_get_app_info = (QSEECom_get_app_info_func)dlsym(mLibHandle, "QSEECom_get_app_info");

    }
    if ((NULL != mQSEECom_start_app) &&
        (NULL != mQSEECom_start_app_V2) &&
        (NULL != mQSEECom_shutdown_app) &&
        (NULL != mQSEECom_send_modified_cmd) &&
        (NULL != mQSEECom_get_app_info) &&
        (NULL != mQSEECom_set_bandwidth) &&
        (NULL != mQSEECom_send_modified_cmd_64)
        )
    {
#if 1
        ret = mQSEECom_start_app(&l_QSEEComHandle, TZ_APP_PATH, TZ_APP_NAME, TZ_APP_BUFFER_SIZE);
#else
        ret = start_app_v2(&l_QSEEComHandle, TZ_APP_PATH, TZ_APP_NAME, TZ_APP_BUFFER_SIZE);
#endif
        if(ret != 0)
        {
            ALOGE("%s Error: Failed to load TA image (%s).", __FUNCTION__, TZ_APP_NAME);
        }
    }
    ALOGD("X");
}

QCameraSEECom::~QCameraSEECom()
{
    int     ret = 0;
    ALOGD("E");
    if(l_QSEEComHandle == NULL)
    {
      ALOGE("%s Error: QSEECom_start_app failed", __FUNCTION__);
    }
    else
    {
        //shutdown sample app
        ret = mQSEECom_shutdown_app(&l_QSEEComHandle);
        if(ret != 0)
        {
          ALOGE("%s Error: QSEECom_shutdown_app failed", __FUNCTION__);
        }
    }
    ALOGD("X");
}

int QCameraSEECom::start_app_v2(struct QSEECom_handle **l_QSEEComHandle,
                                   const char *path, const char* appname, int buf_size)
{
   int32_t ret = 0;
   char temp_fname[MAX_PATH_NAME + MAX_FNAME + MAX_FNAME_EXT];

   int32_t fd = -1;
   struct stat info;
   unsigned char *app_buf = NULL;

   if (!appname || !path) {
      ALOGV("appname or path is NULL");
      ret = -1;
      goto end;
   }

   snprintf(temp_fname, sizeof(temp_fname), "%s/%s.mbn", path, appname);
   fd = open(temp_fname, O_RDONLY);
   if (fd  == -1) {
      ALOGE("Cannot open file %s/%s.mbn, errno = %d\n", path, appname, errno);
      ret = -1;
      goto end;
   }
   ALOGD("Succeed to open file %s/%s.mbn\n", path, appname);

   /* Grab the file size information */
   if (fstat(fd, &info) == -1) {
      ALOGE("fstat failed, errno = %d\n", errno);
      ret = -1;
      goto end;
   }

   app_buf = (unsigned char *)malloc(info.st_size);
   if (!app_buf) {
      ALOGE("Malloc failed with size %d\n", (int)info.st_size);
      ret = -1;
      goto end;
   }

   /* Read the file contents starting at into buffer */
   ret = read(fd, app_buf, info.st_size);
   if (ret == -1) {
      ALOGE("Error::reading from image %s.mbn failed.\n", appname);
      goto end;
   }
   /* Call QSEECom_start_app_v2 */
   ret = mQSEECom_start_app_V2(l_QSEEComHandle, appname, app_buf, info.st_size, buf_size);
   if(ret) {
      ALOGE("Error::Starting App %s failed.\n", appname);
      goto end;
   }
   ALOGD("Starting App %s Success.\n", appname);
   ret = 0;
end:
   if(app_buf) {
      free(app_buf);
   }

   if(fd >= 0) {
      close(fd);
   }

   return ret;
}

int QCameraSEECom::check_if_version(unsigned int* if_version_maj, unsigned int* if_version_min)
{
    
    struct sec_app_if_version_req_t cmd;
    struct sec_app_if_version_rsp_t rsp;
    int32_t cmd_len, rsp_len;
    int32_t ret = 0;
    struct QSEECom_ion_fd_info    ion_fd_info;

    if ((if_version_maj == NULL) || (if_version_min == NULL))
    {
        ALOGE("Bad parameters");
        return -1;
    }
    cmd_len = sizeof(struct sec_app_if_version_req_t);
    rsp_len = sizeof(struct sec_app_if_version_rsp_t);
    cmd.cmd_id = TZ_APP_IF_CMD_GET_IF_VERSION;
    memset(&ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));

    ret = mQSEECom_send_modified_cmd(l_QSEEComHandle, (void*)&cmd, cmd_len,
                                    (void*)&rsp, rsp_len, &ion_fd_info);
    if (ret < 0) {
        ALOGE("Unable to get if version info, rc=%d", ret);
    }
    else {
        if (rsp.rc < 0) {
            ALOGD("TZ App error, rc=%d", rsp.rc);
            ret = -1;
        } else {
            *if_version_maj = rsp.if_version_maj;
            *if_version_min = rsp.if_version_min;
            ALOGD("TZ If version %d.%d",
                *if_version_maj,
                *if_version_min);
        }
    }
    return ret;
}

QCameraSEECom * QCameraSEECom::GetInstance()
{
        static QCameraSEECom mInstance;
        return &mInstance;
}
