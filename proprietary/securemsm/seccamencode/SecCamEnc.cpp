/**
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include "SecCamEnc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "QSEEComAPI.h"
#include "SecCamEncCommon.h"
#include "common_log.h"

#define TZ_OUT_BUF_MAX QSEECOM_MAX_SG_ENTRY
#define SCE_BUF_SIZE (100 * 1024)

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static const char *const app_paths[] = {"/vendor/firmware_mnt/image",
                                        "/vendor/firmware", "/firmware/image"};
static uint32_t nr_app_paths = NUM_ELEMS(app_paths);

/*
Descriptoin: This API loads the secure app and returns the handle to the client.

@return
SCE_SUCCESS                  - Success.
SCE_ERROR_INIT_FAILED        - Failure, in case of init failed.
SCE_ERROR_INVALID_PARAMS     - Failure, in case of invalid parameters.

@param[in]       l_handle      pointer to the handle
*/
sce_result_t secure_camera_encode_init(sce_handle *l_handle,
                                       const char *app_name)
{
    long ret = SCE_SUCCESS;
    long qsee_ret = 0;
    uint32_t i = 0;
    struct QSEECom_handle **l_QSEEComHandle = NULL;

    ENTER();
    // Sanity check
    CHECK_COND_MSG(l_handle == NULL || *l_handle != NULL,
                   SCE_ERROR_INVALID_PARAMS, "Invalid Handle");
    CHECK_COND_MSG(app_name == NULL, SCE_ERROR_INVALID_PARAMS,
                   "Invalid app name");

    l_QSEEComHandle = (struct QSEECom_handle **)l_handle;

    for (i = 0; i < nr_app_paths; i++) {
        ret = QSEECom_start_app(l_QSEEComHandle, app_paths[i], app_name,
                                SCE_BUF_SIZE);
        if (!ret) {
            break;
        }
    }
    CHECK_COND_MSG(ret != 0, SCE_ERROR_INIT_FAILED, "Failed to load app (%s).",
                   app_name);

exit:
    // Unload TA incase of error, if it is loaded already
    if (ret && l_QSEEComHandle && *l_QSEEComHandle) {
        qsee_ret = QSEECom_shutdown_app(l_QSEEComHandle);
        if (qsee_ret)
            LOGE("%s Error: unloading TA failed with error: %d", __FUNCTION__,
                 qsee_ret);
    }

    EXIT();
    return (sce_result_t)ret;
}

/*
Description: This API unloads the secure app corresponding to the client handle.

@return
SCE_SUCCESS                  - Success.
SCE_ERROR_TERMINATE_FAILED   - Failure, in case of termination failure.
SCE_ERROR_INVALID_PARAMS     - Failure, in case of invalid parameters.

@param[in]    l_handle      pointer to the handle
*/
sce_result_t secure_camera_encode_terminate(sce_handle *l_handle)
{
    long ret = 0;
    struct QSEECom_handle **l_QSEEComHandle = NULL;

    ENTER();
    // Sanity check
    CHECK_COND_MSG(l_handle == NULL || *l_handle == NULL,
                   SCE_ERROR_INVALID_PARAMS, "Invalid Handle");
    l_QSEEComHandle = (struct QSEECom_handle **)l_handle;

    // lock mutex
    SCE_LOCK_MUTEX(&g_mutex);

    // shutdown app
    ret = QSEECom_shutdown_app(l_QSEEComHandle);
    if (ret != 0) {
        LOGE("%s Error: unloading app returned = %d", __FUNCTION__, ret);
    }

    // unlock mutex
    SCE_UNLOCK_MUTEX(&g_mutex);

exit:
    EXIT();
    return (sce_result_t)ret;
}

/*
Description: This API copies the content present in the secure buffer,
referenced by
the sec_buf_handle parameter, into the non-secure buffer referenced
by the non_sec_buf_handle if allowed.

@return
SCE_SUCCESS              - Success.
SCE_ERROR_COPY_FAILED    - Failure, in case of copy is failed.
SCE_ERROR_INVALID_PARAMS - Failure, in case of invalid parameters.

@param[in]       l_handle               pointer to the handle
@param[in]       non_sec_buf_handle     reference to the non-secure buffer
@param[in]       non_sec_buf_len        number of bytes in the non-secure buffer
@param[in]       sec_buf_handle         reference to the secure buffer
@param[in/out]   sec_buf_len            in: secure buffer lenth,
                                        out: number of bytes written into the
non secure buffer
@param[in]       flags                  custom flags
*/
sce_result_t secure_camera_encode_copy(sce_handle l_handle,
                                       uint32 non_sec_buf_handle,
                                       const uint32 non_sec_buf_len,
                                       uint32 sec_buf_handle,
                                       uint32 *sec_buf_len, uint32 flags)
{
    long ret = SCE_SUCCESS;
    struct QSEECom_ion_fd_info ion_fd_info = {0};
    tz_sc_copy_rsp_t *resp = NULL;
    tz_sc_copy_req_t *cmd = NULL;
    int32_t cmd_len = 0;
    int32_t rsp_len = 0;
    struct QSEECom_handle *l_QSEEComHandle = NULL;

    ENTER();

    // Sanity check
    CHECK_COND_MSG(l_handle == NULL, SCE_ERROR_INVALID_PARAMS,
                   "handle is NULL");
    CHECK_COND_MSG(sec_buf_len == NULL, SCE_ERROR_INVALID_PARAMS,
                   "sec_buf_len ptr is NULL");
    CHECK_COND_MSG(*sec_buf_len == 0, SCE_ERROR_INVALID_PARAMS,
                   "sec_buf_len is 0");
    CHECK_COND_MSG(non_sec_buf_len == 0, SCE_ERROR_INVALID_PARAMS,
                   "non_sec_buf_len is 0");

    l_QSEEComHandle = (struct QSEECom_handle *)l_handle;

    cmd_len = QSEECOM_ALIGN(sizeof(*cmd));
    rsp_len = QSEECOM_ALIGN(sizeof(*resp));

    CHECK_COND_MSG((cmd_len + rsp_len) > SCE_BUF_SIZE, SCE_ERROR_COPY_FAILED,
                   "Insufficient buffer to accomodate cmd/resp");

    // mutex lock to prevent multithread access errors
    SCE_LOCK_MUTEX(&g_mutex);

    cmd = (tz_sc_copy_req_t *)l_QSEEComHandle->ion_sbuffer;
    CHECK_COND_MSG(NULL == cmd, SCE_ERROR_COPY_FAILED, "cmd is null");

    resp = (tz_sc_copy_rsp_t *)l_QSEEComHandle->ion_sbuffer + cmd_len;
    CHECK_COND_MSG(NULL == resp, SCE_ERROR_COPY_FAILED, "resp is null");

    // fill in request structure fields
    cmd->cmd_id = SCE_CMD01_COPY_IF_ALLOWED;
    cmd->flags = (uint32)flags;
    cmd->non_sec_buf_len = non_sec_buf_len;
    cmd->sec_buf_len = *sec_buf_len;

    // fill in ion handle info
    ion_fd_info.data[0].fd = non_sec_buf_handle;
    ion_fd_info.data[0].cmd_buf_offset =
        offsetof(tz_sc_copy_req_t, non_sec_buf_handle);
    ion_fd_info.data[1].fd = sec_buf_handle;
    ion_fd_info.data[1].cmd_buf_offset =
        offsetof(tz_sc_copy_req_t, sec_buf_handle);

    ret = QSEECom_send_modified_cmd_64(l_QSEEComHandle, cmd, cmd_len, resp,
                                       rsp_len, &ion_fd_info);
    CHECK_COND_MSG(ret != 0, SCE_ERROR_COPY_FAILED, "ioctl returned %d", ret);

    *sec_buf_len = resp->sec_buf_len;

    ret = (sce_result_t)resp->ret;
    if (ret != SCE_SUCCESS) {
        LOGE("%s Error: copy handler returned = %d", __FUNCTION__, ret);
    }

exit:
    // unlock mutex
    SCE_UNLOCK_MUTEX(&g_mutex);

    EXIT();
    return (sce_result_t)ret;
}

#ifdef __cplusplus
}

#endif  // __cplusplus
