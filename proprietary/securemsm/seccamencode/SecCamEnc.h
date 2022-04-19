/**
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef SEC_CAM_ENC_H
#define SEC_CAM_ENC_H

#include <pthread.h>
#include "app_main.h"

typedef void *sce_handle;

typedef enum sce_result_e {
    SCE_SUCCESS = 0,
    SCE_ERROR_COPY_FAILED = 1,
    SCE_ERROR_INIT_FAILED = 2,
    SCE_ERROR_TERMINATE_FAILED = 3,
    SCE_ERROR_INVALID_PARAMS = 4,
    SCE_ERROR_BUFFER_TOO_SHORT = 5,
    SCE_ERROR_FEATURE_NOT_SUPPORTED = 6,
    SCE_FAILURE = 0x7FFFFFFF
} sce_result_t;

// TZ App/Client commands
typedef enum { SCE_CMD01_COPY_IF_ALLOWED = 1 } sce_cmd_type;

// TZ send command structures
typedef struct tz_sc_copy_req_s {
    /** First 4 bytes should always be command id */
    uint32 cmd_id;
    uint32 flags;
    uint64 non_sec_buf_handle;
    uint32 non_sec_buf_len;
    tz_buf_64bit_array_s sec_buf_handle;
    uint32 sec_buf_len;
} __attribute__((packed)) tz_sc_copy_req_t;

typedef struct tz_sc_copy_rsp_s {
    /** First 4 bytes should always be command id */
    uint32 cmd_id;
    uint32 sec_buf_len;
    long ret;
} __attribute__((packed)) tz_sc_copy_rsp_t;

/*
  This API loads the secure app and returns the handle to the client.

  @return
  SCE_SUCCESS                  - Success.
  SCE_ERROR_INIT_FAILED        - Failure.
  SCE_ERROR_TERMINATE_FAILED   - Failure, Invalid parameter

  @param[out]       l_handle      pointer to the handle

  @dependencies
  This function should be invoked by the client before starting any secure
  session
  using secure_camera_encode_copy().

*/
sce_result_t secure_camera_encode_init(sce_handle *l_handle,
                                       const char *app_name);

/*
  This API unloads the secure app corresponding to the client handle.

  @return
  SCE_SUCCESS                  - Success.
  SCE_ERROR_TERMINATE_FAILED   - Failure.
  SCE_ERROR_TERMINATE_FAILED   - Failure, Invalid parameter

  @param[in]    l_handle      pointer to the handle

  @dependencies
  This function should be invoked by the client on properly initialized sessoins
  and
  after copy operations are complete.

*/
sce_result_t secure_camera_encode_terminate(sce_handle *l_handle);

/*
  This API copies the content present in the secure buffer, referenced by
  the sec_buf_handle parameter, into the non-secure buffer referenced
  by the non_sec_buf_handle if allowed.

  @return
  SCE_SUCCESS                - Success.
  SCE_ERROR_COPY_FAILED      - Failure.
  SCE_ERROR_TERMINATE_FAILED - Failure, Invalid parameter

  @param[in]       l_handle               pointer to the handle
  @param[in]       non_sec_buf_handle     non-secure buffer fd
  @param[in]       non_sec_buf_len        number of bytes in the non-secure
  buffer
  @param[in]       sec_buf_handle         reference to the secure buffer
  @param[in/out]   sec_buf_len            in: secure buffer lenth,
                                          out: number of bytes written into the
  non secure buffer
  @param[in]       flags                  custom flags

  @dependencies
  This function must only be invoked after using secure_camera_encode_init()

*/
sce_result_t secure_camera_encode_copy(sce_handle l_handle,
                                       uint32 non_sec_buf_handle,
                                       const uint32 non_sec_buf_len,
                                       uint32 sec_buf_handle,
                                       uint32 *sec_buf_len, uint32 flags);

#endif /* SEC_CAM_ENC_H */
