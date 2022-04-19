#ifndef SNS_LOW_LAT_H_
#define SNS_LOW_LAT_H_

/*============================================================================
  @file sns_low_lat.h

  @brief
  API functions, data structures, and constants used by the Low Latency Stream
  Library

  @note
  File line length should generally be limited to <= 80 columns.

  Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ===========================================================================*/

#include <sys/types.h>

#define SNS_LOW_LAT_START_FLAG_INTERRUPT_EN       0x00000001

int sns_low_lat_enable(void* output_buf,
                       unsigned int output_buf_size,
                       unsigned int output_buf_offset,
                       const uint64* sensor_suid,
                       int sensor_suidLen,
                       unsigned int sample_period_us,
                       unsigned int flags,
                       int* stream_id);

int sns_low_lat_disable(int stream_id);

int sns_low_lat_poll(int stream_id);

#endif /* SNS_LOW_LAT_H_ */