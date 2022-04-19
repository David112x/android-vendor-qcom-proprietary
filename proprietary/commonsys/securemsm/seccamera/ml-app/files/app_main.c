/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "seccamlib.h"

pthread_t hlos_rcv_thread;

// If the frame processing time is high,
// it is recommended that the frame is copied to another buffer.
mlvmapp_error_t process_frame(const mlvmapp_frame_info_t* frame_info,
                              uint8_t *camera_buffer,
                              uint32_t cam_buf_size,
                              const mlvmapp_buffer_info_t* cam_buf_info,
                              const void* custom_req,
                              uint32_t custom_req_size,
                              void* custom_rsp,
                              uint32_t custom_rsp_size) {
    mlvmapp_error_t ret = MLVM_APP_E_SUCCESS;
    int i = 0;

    printf("\nNew frame_event received: cam_id:%u, cbuf:%p, cbuf_size: %d\n",
           frame_info->cam_id, camera_buffer, cam_buf_size);

    if (camera_buffer) {
        while (i++ < 10) {
            printf("0x%x ", *(camera_buffer + i));
        }
    }

    return ret;
}

// Called on receiving INIT cmd from HLOS
mlvmapp_error_t init_cmd_handler(void *rcv_buf,
                                 uint32_t rcv_size,
                                 void *rsp_buf,
                                 uint32_t *rsp_size) {
    mlvmapp_error_t ret = MLVM_APP_E_SUCCESS;

    printf("App init cmd received\n");

    return ret;
}

// Called on receiving DEINIT cmd from HLOS
mlvmapp_error_t deinit_cmd_handler(void *rcv_buf,
                                   uint32_t rcv_size,
                                   void *rsp_buf,
                                   uint32_t *rsp_size) {
    mlvmapp_error_t ret = MLVM_APP_E_SUCCESS;

    printf("App de-init received\n");

    return MLVM_APP_E_DEINIT_COMMAND;
}

// Called on receiving STOP cmd from HLOS
mlvmapp_error_t stop_cmd_handler(void *rcv,
                                 uint32_t rcv_size,
                                 void *rsp,
                                 uint32_t *rsp_size) {
    mlvmapp_error_t ret = MLVM_APP_E_SUCCESS;
    mlvmapp_generic_rsp_t *rsp_buf = rsp;

    // TODO: Add cleanup code here, i.e before notify_hyp_done call

    ret = notify_hyp_done();
    if (ret != MLVM_APP_E_SUCCESS) {
        printf("Failed to send done notification to hyp\n");
    }

    rsp_buf->ret = ret;
    *rsp_size = sizeof(*rsp_buf);

    return ret;
}

// Called on receiving STOP cmd from HLOS
mlvmapp_error_t start_cmd_handler(void *rcv,
                                  uint32_t rcv_size,
                                  void *rsp,
                                  uint32_t *rsp_size) {
    mlvmapp_error_t ret = MLVM_APP_E_SUCCESS;
    mlvmapp_generic_rsp_t *rsp_buf = rsp;

    printf("%s -Enter\n", __func__);

    // TODO: Add init code here, i.e before notify_hyp_done call

    ret = wait_for_hyp_notification();
    if (ret != MLVM_APP_E_SUCCESS){
        printf("Failed to receive start/open scbuff\n");
    }

    rsp_buf->ret = ret;
    *rsp_size = sizeof(*rsp_buf);

    printf("%s -Exit\n", __func__);

    return ret;
}

mlvmapp_error_t MLVM_app_receive_cmds(void) {
    mlvmapp_error_t ret = MLVM_APP_E_SUCCESS;
    int status = 0;

    ret = wait_for_hlos_app_init_cmd();
    if (ret != MLVM_APP_E_SUCCESS) {
       goto EXIT;
    }
    status = pthread_create(&hlos_rcv_thread, NULL, hlos_rcv_cmds, NULL);
    if (status) {
        printf("failed to start pthread hlos_rcv: %d\n", status);
        ret = MLVM_APP_E_FAIL;
        goto EXIT;
    }
    pthread_join(hlos_rcv_thread, NULL);

EXIT:
    return ret;
}

mlvmapp_error_t MLVM_app_shutdown(void) {
    return sock_deinit();
}

mlvmapp_error_t MLVM_app_start(void) {
    mlvmapp_error_t ret = MLVM_APP_E_SUCCESS;

    // Establish socket communication with HLOS
    ret = sock_init();
    if (ret != 0) {
        printf("%s: sock_init failed %d\n", __func__, ret);
    }
    return ret;
}

int main(int argc, char const **argv) {
    mlvmapp_error_t ret = MLVM_APP_E_SUCCESS;

    ret = MLVM_app_start();
    if (ret != MLVM_APP_E_SUCCESS) {
        printf("MLVM_app_start failed %d\n", ret);
         goto EXIT;
    }

    ret = MLVM_app_receive_cmds();
    if (ret != MLVM_APP_E_SUCCESS) {
        printf("MLVM_app_receive_cmds failed %d\n", ret);
        goto EXIT;
    }

    ret = MLVM_app_shutdown();
    if (ret != MLVM_APP_E_SUCCESS) {
        printf("MLVM_app_shutdown failed %d\n", ret);
        goto EXIT;
    }

EXIT:
    return 0;
}
