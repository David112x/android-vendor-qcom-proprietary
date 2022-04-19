/******************************************************************************
  @file    wigig_sensing_lib.h
  @brief   The WiGig Sensing library C file.

  DESCRIPTION
  WiGig Sensing client routines.
  WiGig Sensing clients will be built on top of these routines for communicating
  with SLPI driver, which involves initializing, sending and receiving messages.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  wigig_sensing_lib_register() needs to be called before sending or receiving any
  service specific messages

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/

#if APPS_LIB
#include "wigig_sensing_apps.h"
#else
#include "wigig_sensing_slpi.h"
#endif

struct wigig_sensing_funcs
{
    wigig_sensing_lib_error_type(*register_func) (bool auto_recovery);
    wigig_sensing_lib_error_type(*get_mode_func) (wigig_sensing_lib_mode_type *mode);
    wigig_sensing_lib_error_type(*change_mode_func) (wigig_sensing_lib_mode_type mode, wigig_sensing_channel channel, uint32_t *burst_size);
    wigig_sensing_lib_error_type(*get_params_func) (void *params_buf, uint32_t params_buf_len, uint32_t *params_recv_len);
    wigig_sensing_lib_error_type(*send_debug_func) (uint32_t req_id, bool data_valid, void *req_buf, uint32_t req_buf_len, void *resp_buf,
        uint32_t resp_buf_len, uint32_t *resp_buf_recv_len, uint32_t timeout_msecs);
    wigig_sensing_lib_error_type(*read_data_func) (uint32_t *data_buf, uint32_t data_buf_len, uint32_t *data_buf_recv_len, uint64_t *dri_tsf_arr,
        uint32_t dri_tsf_arr_size, uint32_t *num_of_bursts);
    wigig_sensing_lib_error_type(*read_stats) (uint32_t *dropped_cnt_from_last_read, uint32_t *remaining_bursts);
    wigig_sensing_lib_error_type(*cancel_read) ();
    void(*get_avail_bursts) (uint32_t *num_avail_bursts);
    wigig_sensing_lib_error_type(*clear_data) ();
    void(*unregister_func) ();
    /* TODO: add rest of the APIs */
};

static struct wigig_sensing_funcs sensing_funcs;
struct wigig_sensing_lib sensing_lib = { 0 };

wigig_sensing_lib_error_type
wigig_sensing_lib_unregister
(
    wigig_sensing_lib_client_type   user_handle
)
{
    if (user_handle != (wigig_sensing_lib_client_type)&sensing_lib) {
        syslog(LOG_ERR, "%s: invalid handle given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    if (!sensing_lib.is_registered) {
        syslog(LOG_INFO, "%s: already unregistered\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_NO_ERR;
    }

    sensing_funcs.cancel_read();

    sensing_funcs.unregister_func();
    memset(&sensing_lib, 0, sizeof(sensing_lib));

    syslog(LOG_INFO, "%s: finished\n", __FUNCTION__);
    return WIGIG_SENSING_LIB_NO_ERR;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_change_mode
(
    wigig_sensing_lib_client_type   user_handle,
    wigig_sensing_lib_mode_type     mode,
    wigig_sensing_channel           channel,
    uint32_t                        *burst_size
)
{
    if (user_handle != (wigig_sensing_lib_client_type)&sensing_lib) {
        syslog(LOG_ERR, "%s: invalid handle given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    syslog(LOG_INFO, "sending change_mode new mode %d channel %d\n", mode, channel);

    return sensing_funcs.change_mode_func(mode, channel, burst_size);
}

wigig_sensing_lib_error_type
wigig_sensing_lib_get_parameters
(
    wigig_sensing_lib_client_type   user_handle,
    void                            *params_buf,
    uint32_t                        params_buf_len,
    uint32_t                        *params_recv_len
)
{
    if (user_handle != (wigig_sensing_lib_client_type)&sensing_lib) {
        syslog(LOG_ERR, "%s: invalid handle given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    if (!params_buf || !params_buf_len) {
        syslog(LOG_ERR, "%s: invalid params buf given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    return sensing_funcs.get_params_func(params_buf, params_buf_len, params_recv_len);
}

wigig_sensing_lib_error_type
wigig_sensing_lib_send_debug_msg
(
    wigig_sensing_lib_client_type   user_handle,
    uint32_t                        req_id,
    bool                            data_valid,
    void                            *req_buf,
    uint32_t                        req_buf_len,
    void                            *resp_buf,
    uint32_t                        resp_buf_len,
    uint32_t                        *resp_buf_recv_len,
    uint32_t                        timeout_msecs
)
{
    if (user_handle != (wigig_sensing_lib_client_type)&sensing_lib) {
        syslog(LOG_ERR, "%s: invalid handle given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    if (!resp_buf || !resp_buf_len) {
        syslog(LOG_ERR, "%s: invalid resp buf given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    return sensing_funcs.send_debug_func(req_id, data_valid, req_buf, req_buf_len, resp_buf, resp_buf_len, resp_buf_recv_len, timeout_msecs);
}

wigig_sensing_lib_error_type
wigig_sensing_lib_get_num_avail_bursts
(
    wigig_sensing_lib_client_type   user_handle,
    uint32_t                    *num_avail_bursts
)
{
    if (user_handle != (wigig_sensing_lib_client_type)&sensing_lib) {
        syslog(LOG_ERR, "%s: invalid handle given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    if (!num_avail_bursts) {
        syslog(LOG_ERR, "%s: invalid num_avail_bursts given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    sensing_funcs.get_avail_bursts(num_avail_bursts);

    return WIGIG_SENSING_LIB_NO_ERR;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_read_data
(
    wigig_sensing_lib_client_type   user_handle,
    void                            *data_buf,
    uint32_t                        data_buf_len,
    uint32_t                        *data_buf_recv_len,
    uint64_t                        *dri_tsf_arr,
    uint32_t                        dri_tsf_arr_size,
    uint32_t                        *num_of_bursts
)
{
    if (user_handle != (wigig_sensing_lib_client_type)&sensing_lib) {
        syslog(LOG_ERR, "%s: invalid handle given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    syslog(LOG_DEBUG, "wigig_sensing_lib_read_data buf_len=%d\n", data_buf_len);
    if (!data_buf || !data_buf_len || !dri_tsf_arr || !dri_tsf_arr_size) {
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    if (!sensing_lib.is_registered) {
        syslog(LOG_ERR, "%s: Sensing lib was not registered \n", __FUNCTION__);
        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }

    if (sensing_lib.current_mode == WIGIG_SENSING_LIB_STOP_MODE || sensing_lib.burst_size <= 0) {
        syslog(LOG_ERR, "%s: invalid state to read data, current mode=%d, burst size=%d \n", __FUNCTION__, sensing_lib.current_mode, sensing_lib.current_mode);
        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }

    wigig_sensing_lib_error_type rc = WIGIG_SENSING_LIB_NO_ERR;

    rc = sensing_funcs.read_data_func((uint32_t*)data_buf, data_buf_len, data_buf_recv_len, dri_tsf_arr, dri_tsf_arr_size, num_of_bursts);

    syslog(LOG_DEBUG, "Returning data_buf_recv_len=%d num_of_bursts=%d status=%d\n",
        *data_buf_recv_len, *num_of_bursts, rc);

    return rc;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_read_stats
(
    wigig_sensing_lib_client_type   user_handle,
    uint32_t                        *dropped_cnt_from_last_read,
    uint32_t                        *remaining_bursts
)
{
    if (user_handle != (wigig_sensing_lib_client_type)&sensing_lib) {
        syslog(LOG_ERR, "%s: invalid handle given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    wigig_sensing_lib_error_type rc = sensing_funcs.read_stats(dropped_cnt_from_last_read, remaining_bursts);

    syslog(LOG_DEBUG, "Returning dropped_cnt_from_last_read=%d remaining_bursts=%d status=%d\n",
        *dropped_cnt_from_last_read, *remaining_bursts, rc);

    return rc;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_get_mode
(
    wigig_sensing_lib_client_type   user_handle,
    wigig_sensing_lib_mode_type     *mode
)
{
    if (user_handle != (wigig_sensing_lib_client_type)&sensing_lib) {
        syslog(LOG_ERR, "%s: invalid handle given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    if (!mode) {
        syslog(LOG_ERR, "%s: invalid mode given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    return sensing_funcs.get_mode_func(mode);
}

extern const char*
wigig_sensing_lib_translate_error
(
    uint32_t                        lib_error
)
{
    switch (lib_error)
    {
    case (WIGIG_SENSING_LIB_INTERNAL_ERR):
        return "WiGig Sensing lib internal error received in communication with SLPI";
    case (WIGIG_SENSING_LIB_SERVICE_ERR):
        return "WiGig Sensing lib service error, QMI server cannot be reached";
    case (WIGIG_SENSING_LIB_TIMEOUT_ERR):
        return "WiGig Sensing lib timeout error";
    case (WIGIG_SENSING_LIB_SENSING_DISABLED):
        return "WiGig Sensing lib sensing disabled";
    case (WIGIG_SENSING_LIB_PORT_NOT_OPEN_ERR):
        return "WiGig Sensing lib port not open error";
    case (WIGIG_SENSING_LIB_MEMORY_ERROR):
        return "WiGig Sensing lib memory error related to malloc or memcopy";
    case (WIGIG_SENSING_LIB_TRANSPORT_ERR):
        return "WiGig Sensing lib transport error";
    case (WIGIG_SENSING_LIB_PARAM_ERR):
        return "WiGig Sensing lib invalid parameters error";
    case (WIGIG_SENSING_LIB_INVALID_CLNT):
        return "WiGig Sensing lib invalid client";
    case (WIGIG_SENSING_LIB_CLIENT_FW_NOT_UP):
        return "WiGig Sensing lib FW is down";
    case (WIGIG_SENSING_LIB_XPORT_BUSY_ERR):
        return "WiGig Sensing lib XPORT busy";
    case (WIGIG_SENSING_LIB_SENSING_FW_RESET):
        return "WiGig Sensing lib FW is not ready";
    case (WIGIG_SENSING_LIB_NO_DATA):
        return "WiGig Sensing lib no data available";
    case (WIGIG_SENSING_LIB_INVALID_BURST_SIZE):
        return "WiGig Sensing lib invalid burst size or never got burzt size indication";
    case (WIGIG_SENSING_LIB_MUTEX_ERR):
        return "WiGig Sensing lib error on mutex usage";
    case (WIGIG_SENSING_LIB_NO_ERR):
        return "Success";
    default:
        return "Unknown error";
    }
}

wigig_sensing_lib_error_type
wigig_sensing_lib_clear_data
(
    wigig_sensing_lib_client_type   user_handle
)
{
    if (user_handle != (wigig_sensing_lib_client_type)&sensing_lib) {
        syslog(LOG_ERR, "%s: invalid handle given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    sensing_funcs.clear_data();

    syslog(LOG_INFO, "%s finished\n", __FUNCTION__);
    return WIGIG_SENSING_LIB_NO_ERR;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_cancel_read(
    wigig_sensing_lib_client_type   user_handle
)
{
    if (user_handle != (wigig_sensing_lib_client_type)&sensing_lib) {
        syslog(LOG_ERR, "%s: invalid handle given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    return sensing_funcs.cancel_read();
}

wigig_sensing_lib_error_type
wigig_sensing_lib_set_timeout(
    wigig_sensing_lib_client_type   user_handle,
    int                             timeout_ms
)
{
    if (user_handle != (wigig_sensing_lib_client_type)&sensing_lib) {
        syslog(LOG_ERR, "%s: invalid handle given to lib\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    if (timeout_ms < 0) {
        syslog(LOG_ERR, "%s: invalid timeout\n", __FUNCTION__);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    sensing_lib.cmd_timeout_ms = timeout_ms;

    return WIGIG_SENSING_LIB_NO_ERR;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_register
(
    wigig_sensing_lib_client_type   *user_handle,
    bool                            auto_recovery
)
{
    if (sensing_lib.is_registered) {
        syslog(LOG_ERR, "wigig_sensing_lib_register called while sensing lib is already registered. abort.\n");
        return WIGIG_SENSING_LIB_SERVICE_ERR;
    }

    sensing_lib.cmd_timeout_ms = DEFAULT_TIMEOUT_MS;

#if APPS_LIB /* TODO: compile based on APPS_LIB flag*/
    sensing_funcs = (struct wigig_sensing_funcs) {
        .register_func = wigig_sensing_lib_register_apps,
        .get_mode_func = wigig_sensing_lib_get_mode_apps,
        .change_mode_func = wigig_sensing_lib_change_mode_apps,
        .read_data_func = wigig_sensing_lib_read_data_apps,
        .read_stats = wigig_sensing_lib_read_stats_apps,
        .cancel_read = wigig_sensing_lib_cancel_read_apps,
        .get_avail_bursts = wigig_sensing_lib_get_avail_bursts_apps,
        .clear_data = wigig_sensing_lib_clear_data_apps,
        .unregister_func = wigig_sensing_lib_unregister_apps,
    };

    syslog(LOG_INFO, "Sensing lib starting in APPS flow\n");

#else
    sensing_funcs = (struct wigig_sensing_funcs) {
        .register_func = wigig_sensing_lib_register_slpi,
        .get_mode_func = wigig_sensing_lib_get_mode_slpi,
        .change_mode_func = wigig_sensing_lib_change_mode_slpi,
        .read_data_func = wigig_sensing_lib_read_data_slpi,
        .read_stats = wigig_sensing_lib_read_stats_slpi,
        .cancel_read = wigig_sensing_lib_cancel_read_slpi,
        .get_avail_bursts = wigig_sensing_lib_get_avail_bursts_slpi,
        .clear_data = wigig_sensing_lib_clear_data_slpi,
        .unregister_func = wigig_sensing_lib_unregister_slpi,
    };

    syslog(LOG_INFO, "Sensing lib starting in SLPI flow\n");
#endif

    wigig_sensing_lib_error_type rc = sensing_funcs.register_func(auto_recovery);
    if (rc)
        return rc;

    *user_handle = (wigig_sensing_lib_client_type)&sensing_lib;

    wigig_sensing_lib_mode_type mode;
    rc = sensing_funcs.get_mode_func(&mode);
    if (rc)
        return rc;

    sensing_lib.is_registered = true;
    sensing_lib.is_fw_ready = true;

    syslog(LOG_INFO, "%s success with initial mode: %d\n", __FUNCTION__, mode);

    return WIGIG_SENSING_LIB_NO_ERR;
}
