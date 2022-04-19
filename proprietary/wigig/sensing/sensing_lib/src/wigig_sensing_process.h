/******************************************************************************
  @file    wigig_sensing_process.h
  @brief   The WiGig Sensing QMI Library Events Handling Header file.

  DESCRIPTION
  APIS for handling decoded wigig sensing protobuf events

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include "sns_wigig.pb.h"
#include "wigig_sensing_lib.h"

uint32_t wigig_sensing_get_curr_req();

void wigig_sensing_process_generic_event
(
    uint32_t error,
    bool match_curr_req
);

void wigig_sensing_process_burst_size_event
(
    sns_wigig_burst_size_event *event_msg,
    bool match_curr_req
);

void wigig_sensing_process_get_params_resp
(
    uint32_t   error,
    const void *params_buf,
    uint32_t   params_buf_len,
    bool match_curr_req
);

void wigig_sensing_process_debug_resp
(
    uint32_t   error,
    uint32_t   req_id,
    const void *debug_buf,
    uint32_t   debug_buf_len,
    bool match_curr_req
);

void wigig_sensing_process_get_mode_resp
(
    sns_wigig_get_mode_resp *event_msg,
    bool match_curr_req
);

void wigig_sensing_process_data_ready_event
(
    uint64_t timestamp,
    bool MF,
    uint32_t data_len,
    uint8_t *data
);

void wigig_sensing_set_fw_ready
(
    bool ready
);