/******************************************************************************
  @file    wigig_sensing_data.h
  @brief   The WiGig Sensing QMI Library Data Handling Header file.

  DESCRIPTION
  APIs for wigig sensing data handling

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/

#pragma once
#include "wigig_sensing_internal.h"

typedef struct
{
    void        *cyclic_buffer;
    uint32_t    cyclic_buffer_size;
    void        *burst_buffer;
    uint32_t    burst_buffer_size;
    uint64_t    burst_timestamp;
    uint32_t    burst_size;
    uint32_t    max_num_bursts_in_buff;
    uint32_t    dropped_cnt_from_last_read;
    uint32_t    curr_buf_size;
    uint32_t    read_pointer;
    uint32_t    write_pointer;
    uint64_t    *dri_tsf_arr;
    uint32_t    tsf_arr_size;
}wigig_sensing_data;

int wigig_sensing_process_read
(
    void            *data_buf,
    uint32_t        data_buf_len,
    uint32_t        *data_buf_recv_len,
    uint64_t        *dri_tsf_arr,
    uint32_t        dri_tsf_arr_size,
    uint32_t        *num_of_bursts
);

int wigig_sensing_process_stats
(
    uint32_t        *dropped_cnt_from_last_read,
    uint32_t        *remaining_bursts
);

void wigig_sensing_release_buffers();

void wigig_sensing_process_single_burst
(
    uint64_t timestamp,
    uint32_t data_len,
    uint8_t *data
);

int wigig_sensing_process_data_ready();

int allocate_cyclic_buffer
(
    uint32_t                    burst_size
);

void wigig_sensing_lib_get_avail_bursts_slpi
(
    uint32_t    *remaining_bursts
);

void wigig_sensing_set_burst_size
(
    uint32_t    burst_size
);

void wigig_sensing_dump_buff
(
    uint8_t *data,
    uint32_t data_len,
    uint32_t num_lines
);

int wigig_sensing_data_mutex_init();

int wigig_sensing_data_mutex_uninit();

wigig_sensing_lib_error_type
wigig_sensing_lib_clear_data_slpi();

uint32_t wigig_sensing_get_buffer_size();