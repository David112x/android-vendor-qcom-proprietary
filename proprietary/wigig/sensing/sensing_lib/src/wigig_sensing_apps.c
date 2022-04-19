/******************************************************************************
  @file    wigig_sensing_apps.c
  @brief   The WiGig Sensing Library APPS Flow C File.

  DESCRIPTION
  Implementation of APIs used for SPI over APPS flow in sensing lib

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include "wigig_sensing_apps.h"

extern struct wigig_sensing_lib sensing_lib;

wigig_sensing_lib_error_type wait_for_device_apps()
{
    if (!sensing_lib.is_registered) {
        syslog(LOG_ERR, "%s: Sensing lib was not registered \n", __FUNCTION__);
        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }

    if (sensing_lib.is_fw_ready) {
        syslog(LOG_INFO, "fw is ready, no need to wait\n");
        return WIGIG_SENSING_LIB_NO_ERR;
    }

    struct pollfd poll_list[1];
    poll_list[0].fd = sensing_lib.wigig_fd;
    poll_list[0].events = POLLPRI;

    int rc = 0;
    int current_timeout = sensing_lib.cmd_timeout_ms;
    while (current_timeout > 0) {
        struct timeval stop, start;
        gettimeofday(&start, NULL);

        rc = poll(poll_list, 1, current_timeout);
        syslog(LOG_INFO, "%s: polling, POLLPRI is set\n", __FUNCTION__);

        gettimeofday(&stop, NULL);
        int poll_time_ms = (stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000; /* convert to ms */
        current_timeout -= poll_time_ms;
        syslog(LOG_INFO, "Took %d ms to return from poll \n", poll_time_ms);

        if (rc < 0) {
            syslog(LOG_ERR, "Wait for fw ready error = %d \n", errno);
            return WIGIG_SENSING_LIB_SENSING_FW_RESET;
        }
        if (rc == 0) {
            syslog(LOG_ERR, "Polling on file descriptors timed out \n");
            return WIGIG_SENSING_LIB_TIMEOUT_ERR;
        }

        if ((poll_list[0].revents&POLLPRI) == POLLPRI) {
            enum wigig_sensing_event evt;
            rc = ioctl(sensing_lib.wigig_fd, WIGIG_SENSING_IOC_GET_EVENT, &evt);

            /* check only the latest event read */
            if (evt == WIGIG_SENSING_EVENT_FW_READY) {
                syslog(LOG_INFO, "fw is ready\n");
                return WIGIG_SENSING_LIB_NO_ERR;
            }
        }
    }
    syslog(LOG_INFO, "fw is not ready\n");
    return WIGIG_SENSING_LIB_SENSING_FW_RESET;
}

static wigig_sensing_lib_error_type send_stop_mode_apps()
{
    if (sensing_lib.current_mode == WIGIG_SENSING_LIB_STOP_MODE) {
        syslog(LOG_INFO, "Already in stop mode\n");
        return WIGIG_SENSING_LIB_NO_ERR;
    }

    struct wigig_sensing_change_mode change_req = { 0 };
    change_req.mode = WIGIG_SENSING_MODE_STOP;

    int rc = ioctl(sensing_lib.wigig_fd, WIGIG_SENSING_IOC_CHANGE_MODE, &change_req);

    if (rc < 0) {
        syslog(LOG_ERR, "%s: stop mode ioctl failed, errno = %d\n", __FUNCTION__, errno);

        if (errno == ENODEV) {
            syslog(LOG_ERR, "%s: fw in sysassert\n", __FUNCTION__);
            return WIGIG_SENSING_LIB_SENSING_FW_RESET;
        }

        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }

    if (change_req.burst_size != 0) {
        syslog(LOG_ERR, "%s: expected 0 for burst size, instead got = %d\n", __FUNCTION__, change_req.burst_size);
        return WIGIG_SENSING_LIB_INVALID_BURST_SIZE;
    }

    sensing_lib.burst_size = change_req.burst_size;
    sensing_lib.current_mode = WIGIG_SENSING_LIB_STOP_MODE;

    return WIGIG_SENSING_LIB_NO_ERR;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_change_mode_apps
(
    wigig_sensing_lib_mode_type     mode,
    wigig_sensing_channel           channel,
    uint32_t                        *burst_size
)
{
    wigig_sensing_lib_error_type res = wait_for_device_apps();
    if (res)
        return res;

    if (mode == WIGIG_SENSING_MODE_STOP) {
        res = send_stop_mode_apps();
        *burst_size = 0;
        syslog(LOG_INFO, "change_mode to stop complete status = %d\n", res);
        return res;
    }

    struct wigig_sensing_change_mode change_req = { 0 };
    change_req.mode = (enum wigig_sensing_mode)mode;
    if (channel >= WIGIG_SENSING_CHANNEL_1 || channel <= WIGIG_SENSING_CHANNEL_12) {
        change_req.has_channel = true;
        change_req.channel = channel;
    }

    int rc = ioctl(sensing_lib.wigig_fd, WIGIG_SENSING_IOC_CHANGE_MODE, &change_req);
    syslog(LOG_INFO, "%s: got burst size %d\n", __FUNCTION__, change_req.burst_size);

    if (rc < 0) {
        syslog(LOG_ERR, "%s: change mode ioctl failed, errno = %d\n", __FUNCTION__, errno);
        *burst_size = 0;

        if (errno == ENODEV) {
            syslog(LOG_ERR, "%s: fw in sysassert\n", __FUNCTION__);
            return WIGIG_SENSING_LIB_SENSING_FW_RESET;
        }

        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }

    sensing_lib.burst_size = change_req.burst_size;
    *burst_size = sensing_lib.burst_size;
    sensing_lib.current_mode = mode;

    syslog(LOG_INFO, "%s succeeded\n", __FUNCTION__);
    return WIGIG_SENSING_LIB_NO_ERR;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_read_data_apps
(
    uint32_t                        *data_buf,
    uint32_t                        data_buf_len,
    uint32_t                        *data_buf_recv_len,
    uint64_t                        *dri_tsf_arr,
    uint32_t                        dri_tsf_arr_size,
    uint32_t                        *num_of_bursts
)
{
    *data_buf_recv_len = 0;
    *num_of_bursts = 0;

    struct pollfd poll_list[2];
    poll_list[0].fd = sensing_lib.wigig_fd;
    poll_list[0].events = POLLIN | POLLPRI; /* poll for both data and events*/
    poll_list[1].fd = sensing_lib.abort_pipe[0];
    poll_list[1].events = POLLIN; /* poll abort*/

    int rc = poll(poll_list, 2, sensing_lib.cmd_timeout_ms);
    syslog(LOG_INFO, "%s: polling, POLLPRI and POLLIN is set\n", __FUNCTION__);

    if (rc < 0) {
        syslog(LOG_ERR, "Error polling on file descriptors = %d \n", errno);
        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }
    if (rc == 0) {
        syslog(LOG_ERR, "Polling on file descriptors timed out \n");
        return WIGIG_SENSING_LIB_TIMEOUT_ERR;
    }

    if (poll_list[1].revents&POLLIN) {
        syslog(LOG_ERR, "Aborting read \n");
        char buf[strlen("x")];
        read(sensing_lib.abort_pipe[0], buf, strlen("x"));
        *data_buf_recv_len = 0;
        *num_of_bursts = 0;
        return WIGIG_SENSING_LIB_NO_ERR;
    }
    if ((poll_list[0].revents&POLLPRI) == POLLPRI) {
        enum wigig_sensing_event evt;
        int ioctl_err = 0;
        while (!ioctl_err) {
            ioctl_err = ioctl(sensing_lib.wigig_fd, WIGIG_SENSING_IOC_GET_EVENT, &evt);
        }
        /* check only the latest event read */
        if (evt == WIGIG_SENSING_EVENT_RESET)
            return WIGIG_SENSING_LIB_SENSING_FW_RESET;
    }

    if ((poll_list[0].revents&POLLIN) == POLLIN) {
        syslog(LOG_INFO, "Got POLLIN\n");
        int recv_len = read(sensing_lib.wigig_fd, data_buf, data_buf_len);
        if (recv_len > 0) {
            *data_buf_recv_len = recv_len;
            *num_of_bursts = recv_len / sensing_lib.burst_size;
        } else {
            syslog(LOG_ERR, "Error trying to read data,recv_len=%d ,errno= %d \n", recv_len, errno);
            *data_buf_recv_len = 0;
            *num_of_bursts = 0;
            return WIGIG_SENSING_LIB_INTERNAL_ERR;
        }
    }
    return WIGIG_SENSING_LIB_NO_ERR;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_get_mode_apps
(
    wigig_sensing_lib_mode_type     *mode
)
{
    sensing_lib.current_mode = ioctl(sensing_lib.wigig_fd, WIGIG_SENSING_IOC_GET_MODE);
    syslog(LOG_INFO, "sensing_lib.current_mode: %d", sensing_lib.current_mode);
    if (sensing_lib.current_mode < 0) {
        syslog(LOG_INFO, "%s ioctl failed: %d", __FUNCTION__, errno);
        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }

    *mode = sensing_lib.current_mode;

    return WIGIG_SENSING_LIB_NO_ERR;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_register_apps
(
    bool                            auto_recovery
)
{
    sensing_lib.wigig_fd = open("/dev/wigig_sensing", O_RDWR);
    if (sensing_lib.wigig_fd < 0) {
        syslog(LOG_ERR, "/dev/wigig_sensing open failed: %d", errno);
        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }

    int rc = pipe(sensing_lib.abort_pipe);
    if (rc) {
        syslog(LOG_ERR, "create temporary pipe to cancel read failed: %d", errno);
        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }

    return WIGIG_SENSING_LIB_NO_ERR;
}

void wigig_sensing_lib_unregister_apps()
{
    wigig_sensing_lib_error_type stop_res = send_stop_mode_apps();
    if (stop_res)
        syslog(LOG_ERR, "%s: change to stop failed = %d, continue to unregister..\n", __FUNCTION__, stop_res);

    close(sensing_lib.wigig_fd);
    close(sensing_lib.abort_pipe[0]);
    close(sensing_lib.abort_pipe[1]);
}

wigig_sensing_lib_error_type
wigig_sensing_lib_read_stats_apps
(
    uint32_t                        *dropped_cnt_from_last_read,
    uint32_t                        *remaining_bursts
)
{
    *dropped_cnt_from_last_read = ioctl(sensing_lib.wigig_fd, WIGIG_SENSING_IOC_GET_NUM_DROPPED_BURSTS);
    /*  TODO: return remaining_bursts */
    *remaining_bursts = 0;

    return WIGIG_SENSING_LIB_NO_ERR;
}

void wigig_sensing_lib_get_avail_bursts_apps
(
    uint32_t    *num_avail_cirs
)
{
    *num_avail_cirs = ioctl(sensing_lib.wigig_fd, WIGIG_SENSING_IOC_GET_NUM_AVAIL_BURSTS);
}


wigig_sensing_lib_error_type
wigig_sensing_lib_cancel_read_apps()
{
    const char* buf = "x"; /* TODO: what to write to cancel read */

    write(sensing_lib.abort_pipe[1], buf, strlen(buf));

    syslog(LOG_INFO, "%s finished", __FUNCTION__);
    return WIGIG_SENSING_LIB_NO_ERR;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_clear_data_apps()
{
    int rc = ioctl(sensing_lib.wigig_fd, WIGIG_SENSING_IOC_CLEAR_DATA);
    if (rc) {
        syslog(LOG_INFO, "%s ioctl failed: %d", __FUNCTION__, errno);
        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }

    return WIGIG_SENSING_LIB_NO_ERR;
}