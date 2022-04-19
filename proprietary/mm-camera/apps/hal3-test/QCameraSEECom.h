////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  QCameraSEECom.h
/// @brief Load QSeeCom API
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __QCAMERA_SEECOM__
#define __QCAMERA_SEECOM__

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
#include <errno.h>
#include <linux/msm_ion.h>
#include <utils/Log.h>
#include <dlfcn.h>
#include "QSEEComAPI.h"

typedef struct sec_app_if_generic_req_t {
    uint32_t cmd_id;
} sec_app_if_generic_req_t;

#define sec_app_if_version_req_t sec_app_if_generic_req_t

typedef struct sec_app_if_version_rsp_t {
    uint32_t rc;
    uint32_t if_version_maj;
    uint32_t if_version_min;
} sec_app_if_version_rsp_t;

#ifndef TZ_APP_IF_CMD_OFFSET
    #define TZ_APP_IF_CMD_OFFSET 56000
#endif

typedef enum tz_app_if_cmd_id_t {
    TZ_APP_IF_CMD_NONE = TZ_APP_IF_CMD_OFFSET,
    TZ_APP_IF_CMD_GET_IF_VERSION,
    TZ_APP_IF_CMD_POWER_UP,
    TZ_APP_IF_CMD_POWER_DOWN,
    TZ_APP_IF_CMD_CCI_GENERIC,
    TZ_APP_IF_CMD_CCI_READ,
    TZ_APP_IF_CMD_CCI_READ_SEQ,
    TZ_APP_IF_CMD_CCI_WRITE,
    TZ_APP_IF_CMD_CCI_WRITE_SEQ,
    TZ_APP_IF_CMD_CCI_WRITE_TABLE_ASYNC,
    TZ_APP_IF_CMD_CCI_WRITE_TABLE_SYNC,
    TZ_APP_IF_CMD_CCI_WRITE_TABLE_SYNC_BLOCK,
    TZ_APP_IF_CMD_CCI_WRITE_TABLE,
    TZ_APP_IF_CMD_CCI_WRITE_SEQ_TABLE,
    TZ_APP_IF_CMD_CCI_WRITE_TABLE_W_MICRODELAY,
    TZ_APP_IF_CMD_CCI_POLL,
    TZ_APP_IF_CMD_CCI_WRITE_CONF_TBL,
    TZ_APP_IF_CMD_CCI_UTIL,
    TZ_APP_IF_CMD_SET_MODE,
    TZ_APP_IF_CMD_FRAME_NOTIFICATION,
    TZ_APP_IF_CMD_REG_READ,
    TZ_APP_IF_CMD_REG_WRITE,
    TZ_APP_IF_CMD_REG_WRITE_BULK,
    TZ_APP_IF_CMD_RESET_HW_BLOCK,
} tz_app_if_cmd_id_t;

class QCameraSEECom {
    void *                      mLibHandle;
    struct QSEECom_handle *    l_QSEEComHandle;

    QCameraSEECom();
    ~QCameraSEECom();
public:
        typedef int (*QSEECom_app_load_query_func)(struct QSEECom_handle *handle, char *app_name);
        typedef int (*QSEECom_clear_key_func)(enum QSEECom_key_management_usage_type usage);
        typedef int (*QSEECom_get_app_info_func)(struct QSEECom_handle *handle, struct qseecom_app_info* info);
        typedef int (*QSEECom_load_external_elf_func)(struct QSEECom_handle **clnt_handle,
                                                            const char *path, const char *fname);
        typedef int (*QSEECom_receive_req_func)(struct QSEECom_handle *handle, void * rcv_buf, uint32_t len);
        typedef int (*QSEECom_start_app_func)(struct QSEECom_handle **clnt_handle, const char *path,
                                                const char *fname, uint32_t sb_length);
        typedef int (*QSEECom_start_app_V2_func)(struct QSEECom_handle** clnt_handle, const char* fname,
                                                 unsigned char* trustlet, uint32_t tlen, uint32_t sb_length);
        typedef int (*QSEECom_shutdown_app_func)(struct QSEECom_handle **handle);
        typedef int (*QSEECom_set_bandwidth_func)(struct QSEECom_handle *handle, bool high);
        typedef int (*QSEECom_send_modified_cmd_func)(struct QSEECom_handle *handle, void *send_buf,
                                                            uint32_t sbuf_len, void *resp_buf, uint32_t rbuf_len,
                                                            struct QSEECom_ion_fd_info *ifd);
        typedef int (*QSEECom_send_modified_cmd_64_func)(struct QSEECom_handle *handle, void *send_buf,
                                                            uint32_t sbuf_len, void *resp_buf, uint32_t rbuf_len,
                                                            struct QSEECom_ion_fd_info *ifd);

        QSEECom_app_load_query_func         mQSEECom_app_load_query;
        QSEECom_clear_key_func              mQSEECom_clear_key;
        QSEECom_get_app_info_func           mQSEECom_get_app_info;
        QSEECom_load_external_elf_func      mQSEECom_load_external_elf;
        QSEECom_receive_req_func            mQSEECom_receive_req;
        QSEECom_start_app_func              mQSEECom_start_app;
        QSEECom_shutdown_app_func           mQSEECom_shutdown_app;
        QSEECom_set_bandwidth_func          mQSEECom_set_bandwidth;
        QSEECom_send_modified_cmd_func      mQSEECom_send_modified_cmd;
        QSEECom_send_modified_cmd_64_func   mQSEECom_send_modified_cmd_64;
        QSEECom_start_app_V2_func           mQSEECom_start_app_V2;

#if 0
        QSEECom_register_listener
        QSEECom_scale_bus_bandwidth
        QSEECom_send_cmd
        QSEECom_send_modified_resp
        QSEECom_send_modified_resp_64
        QSEECom_send_resp
        QSEECom_send_service_cmd
        QSEECom_unload_external_elf
        QSEECom_unregister_listener
        QSEECom_update_key_user_info
        QSEECom_wipe_key
#endif

    static QCameraSEECom * GetInstance();
    int start_app_v2(struct QSEECom_handle **l_QSEEComHandle,
                                       const char *path, const char* appname, int buf_size);
    int check_if_version(unsigned int* if_version_maj, unsigned int* if_version_min);
};

#endif
