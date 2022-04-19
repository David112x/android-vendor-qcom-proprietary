/*********************************************************************
 Copyright (c) 2019 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>

#include "wigig_sensing_lib.h"

#define READ_LOOP_ITERATIONS 10
#define NUM_OF_BURSTS_TO_READ 2 /* read up to 2 bursts at once, can be more */
#define SENSING_MAGIC_NUMBER 0x01020304 /*TODO: data is swapped, need to swap back to 01020304? */

#define LOG_TAG "sensing_sample_app"

int check_data(uint32_t* data_buf, uint32_t num_of_bursts, uint32_t burst_size, uint32_t burst_number) {
    if (num_of_bursts == 0) {
        syslog(LOG_INFO, "No data was available, skipping");
        return 0;
    }

    for(int i = 0; i < num_of_bursts; i++){
        uint32_t burst_magic_number = *(data_buf + i * burst_size / sizeof(uint32_t));
        printf("got magic number: %x for burst %d\n", burst_magic_number, burst_number + i);
        if (burst_magic_number != SENSING_MAGIC_NUMBER) {
            printf("incorrect magic number!\n");
            return -1;
        }
    }

    return 0;
}

int read_loop(wigig_sensing_lib_client_type userHandle, uint32_t burst_size) {
    int rc = 0;
    uint32_t data_buf_len = burst_size * NUM_OF_BURSTS_TO_READ;
    uint32_t *data_buf = malloc(data_buf_len);
    if (data_buf == NULL) {
        syslog(LOG_ERR, "Data buf allocation failed: %d", errno);
        return -errno;
    }

    uint32_t data_buf_recv_len = 0;
    uint32_t dri_tsf_arr_size = NUM_OF_BURSTS_TO_READ * sizeof(uint64_t);
    uint64_t *dri_tsf_arr = (uint64_t*)malloc(dri_tsf_arr_size);
    if (dri_tsf_arr == NULL) {
        syslog(LOG_ERR, "Dri tsf buf allocation failed: %d", errno);
        free(data_buf);
        return -errno;
    }

    uint32_t num_of_bursts = 0;
    uint32_t total_bursts_read = 0;
    for(int i = 0; i < READ_LOOP_ITERATIONS; i++) {
        rc = wigig_sensing_lib_read_data
        (
            userHandle,
            data_buf,
            data_buf_len,
            &data_buf_recv_len,
            dri_tsf_arr,
            dri_tsf_arr_size,
            &num_of_bursts
        );
        if (rc)
            goto exit;

        if(check_data(data_buf, num_of_bursts, burst_size, total_bursts_read)) {
            rc = -1;
            goto exit;
        }

        total_bursts_read += num_of_bursts;
    }

exit:
    free(data_buf);
    free(dri_tsf_arr);
    return rc;
}

int main(int argc, char** argv) {
    printf("Sensing sample app is starting up...\n");
    int rc = 0;
    int unreg = 0;
    wigig_sensing_lib_client_type userHandle;
    uint32_t default_channel = 7; /* channel 10 in CB2 */

    rc = wigig_sensing_lib_register(&userHandle, false);

    if (rc) {
        printf("register failed, error: %d", rc);
        return rc;
    }

    uint32_t burst_size = 0;

    for(int retries = 0; retries <5; retries++) {
        /* chang_mode includes a wait_for_fw_ready() that waits for ready if system is in assert */
        rc = wigig_sensing_lib_change_mode(userHandle, WIGIG_SENSING_LIB_SEARCH_MODE, default_channel, &burst_size);
        if (rc) {
            printf("change mode failed, error: %d", rc);
            goto exit;
        }
        /* at this point after switching to search mode, WiGig device starts sending data */

        wigig_sensing_lib_mode_type mode;
        rc = wigig_sensing_lib_get_mode(userHandle, &mode);
        if (rc) {
            printf("get mode failed: %d", rc);
            goto exit;
        }
        printf("got mode = %d\n", mode);


        rc = read_loop(userHandle, burst_size);
        if (rc == WIGIG_SENSING_LIB_SENSING_FW_RESET)
            continue;
        else
            break;
    }

    rc = wigig_sensing_lib_change_mode(userHandle, WIGIG_SENSING_LIB_STOP_MODE, 0, &burst_size);
    if (rc)
        printf("change mode failed, error: %d", rc);

exit:
    unreg = wigig_sensing_lib_unregister(userHandle);

    if (rc || unreg)
        printf("FAIL\n");
    else
        printf("PASS\n");

    return 0;
}
