/******************************************************************************
  @file    wigig_sensing_internal.h
  @brief   The WiGig Sensing Library Internal Strucs Header File.

  DESCRIPTION
  Internal structs and defines used for both SPI over APPS and SLPI flow in sensing lib

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/

#pragma once
#include "wigig_sensing_lib.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unistd.h>
#include <endian.h>
#include "stddef.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <syslog.h>
#include <math.h>
#include <inttypes.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <byteswap.h>

#ifndef APPS_LIB
#include "qmi_client.h"
#include "sns_client_api_v01.h"
#include "sns_std.pb.h"
#include "sns_std_sensor.pb.h"
#include "sns_client.pb.h"
#endif

#define DEFAULT_TIMEOUT_MS 500
#define FLIPPED_BURST_MAGIC_NUMBER 0x4030201

struct wigig_sensing_lib_debug_msg
{
    uint32_t                        req_id;
    void                            *buf;
    uint32_t                        buf_len;
    uint32_t                        recv_len;
};

struct wigig_sensing_lib_params_msg
{
    void                            *buf;
    uint32_t                        buf_len;
    uint32_t                        recv_len;
};

struct wigig_sensing_lib
{
    bool                            is_registered;
#ifndef APPS_LIB
    qmi_client_type                 qmi_user_handle;
    qmi_client_error_type           qmi_error;
    pthread_cond_t                  async_response_cond_var; /* used to wait for response */
    pthread_mutex_t                 async_response_mutex;
    bool                            async_response_wait_flag;
    uint32_t                        current_req;
    pthread_cond_t                  data_response_cond_var;
    pthread_mutex_t                 data_response_mutex;
    bool                            data_response_wait_flag;
#endif
    uint32_t                        last_error;
    uint32_t                        burst_size; /* as received from driver */
    wigig_sensing_lib_mode_type     current_mode;
    struct wigig_sensing_lib_params_msg    params_buf;
    struct wigig_sensing_lib_debug_msg     debug_buf;
    bool                            is_fw_ready;
    int                             cmd_timeout_ms;
    int                             wigig_fd;
    int                             abort_pipe[2];
};