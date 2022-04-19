/******************************************************************************
  @file    wigig_sensing_slpi.h
  @brief   The WiGig Sensing Library SLPI Flow Header File.

  DESCRIPTION
  APIs used for SLPI flow in sensing lib

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include "wigig_sensing_data.h"
#include "wigig_sensing_encode_decode.h"

void wigig_sensing_lib_unregister_slpi();

wigig_sensing_lib_error_type
wigig_sensing_lib_change_mode_slpi
(
    wigig_sensing_lib_mode_type     mode,
    wigig_sensing_channel           channel,
    uint32_t                        *burst_size
);

wigig_sensing_lib_error_type
wigig_sensing_lib_get_parameters_slpi
(
    void                            *params_buf,
    uint32_t                        params_buf_len,
    uint32_t                        *params_recv_len
);

wigig_sensing_lib_error_type
wigig_sensing_lib_send_debug_msg_slpi
(
    uint32_t                        req_id,
    bool                            data_valid,
    void                            *req_buf,
    uint32_t                        req_buf_len,
    void                            *resp_buf,
    uint32_t                        resp_buf_len,
    uint32_t                        *resp_buf_recv_len,
    uint32_t                        timeout_msecs
);

wigig_sensing_lib_error_type
wigig_sensing_lib_read_data_slpi
(
    uint32_t                        *data_buf,
    uint32_t                        data_buf_len,
    uint32_t                        *data_buf_recv_len,
    uint64_t                        *dri_tsf_arr,
    uint32_t                        dri_tsf_arr_size,
    uint32_t                        *num_of_bursts
);


wigig_sensing_lib_error_type
wigig_sensing_lib_get_mode_slpi
(
    wigig_sensing_lib_mode_type     *mode
);


wigig_sensing_lib_error_type
wigig_sensing_lib_register_slpi
(
    bool                            auto_recovery
);

wigig_sensing_lib_error_type
wigig_sensing_lib_read_stats_slpi
(
    uint32_t                        *dropped_cnt_from_last_read,
    uint32_t                        *remaining_bursts
);

wigig_sensing_lib_error_type
wigig_sensing_lib_cancel_read_slpi();