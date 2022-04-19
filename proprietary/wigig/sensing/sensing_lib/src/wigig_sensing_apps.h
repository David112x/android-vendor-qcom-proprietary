/******************************************************************************
  @file    wigig_sensing_apps.h
  @brief   The WiGig Sensing Library APPS Flow Header File.

  DESCRIPTION
  APIs used for SPI over APPS flow in sensing lib

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/
#pragma once
#include "wigig_sensing_internal.h"
#include <wigig_sensing_uapi.h>

wigig_sensing_lib_error_type wait_for_device_apps();

void wigig_sensing_lib_unregister_apps();

wigig_sensing_lib_error_type
wigig_sensing_lib_change_mode_apps
(
    wigig_sensing_lib_mode_type     mode,
    wigig_sensing_channel           channel,
    uint32_t                        *burst_size
);

wigig_sensing_lib_error_type
wigig_sensing_lib_read_data_apps
(
    uint32_t                        *data_buf,
    uint32_t                        data_buf_len,
    uint32_t                        *data_buf_recv_len,
    uint64_t                        *dri_tsf_arr,
    uint32_t                        dri_tsf_arr_size,
    uint32_t                        *num_of_bursts
);

wigig_sensing_lib_error_type
wigig_sensing_lib_get_mode_apps
(
    wigig_sensing_lib_mode_type     *mode
);

wigig_sensing_lib_error_type
wigig_sensing_lib_register_apps
(
    bool                            auto_recovery
);

wigig_sensing_lib_error_type
wigig_sensing_lib_read_stats_apps
(
    uint32_t                        *dropped_cnt_from_last_read,
    uint32_t                        *remaining_bursts
);

void wigig_sensing_lib_get_avail_bursts_apps
(
    uint32_t    *num_avail_cirs
);

wigig_sensing_lib_error_type
wigig_sensing_lib_cancel_read_apps();

wigig_sensing_lib_error_type
wigig_sensing_lib_clear_data_apps();