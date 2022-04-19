/*============================================================================
  @file sns_low_lat_stream.h

  @brief
  API functions, data structures, and constants used by the Low Latency Stream
  Library

  @note
  File line length should generally be limited to <= 80 columns.

  Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ===========================================================================*/

#ifndef _SNS_LOW_LAT_STREAM_H
#define _SNS_LOW_LAT_STREAM_H
#include "AEEStdDef.h"
#ifndef __QAIC_HEADER
#define __QAIC_HEADER(ff) ff
#endif //__QAIC_HEADER

#ifndef __QAIC_HEADER_EXPORT
#define __QAIC_HEADER_EXPORT
#endif // __QAIC_HEADER_EXPORT

#ifndef __QAIC_HEADER_ATTRIBUTE
#define __QAIC_HEADER_ATTRIBUTE
#endif // __QAIC_HEADER_ATTRIBUTE

#ifndef __QAIC_IMPL
#define __QAIC_IMPL(ff) ff
#endif //__QAIC_IMPL

#ifndef __QAIC_IMPL_EXPORT
#define __QAIC_IMPL_EXPORT
#endif // __QAIC_IMPL_EXPORT

#ifndef __QAIC_IMPL_ATTRIBUTE
#define __QAIC_IMPL_ATTRIBUTE
#endif // __QAIC_IMPL_ATTRIBUTE
#ifdef __cplusplus
extern "C" {
#endif
__QAIC_HEADER_EXPORT int __QAIC_HEADER(sns_low_lat_stream_init)(int fd, unsigned int size, unsigned int offset, int* handle) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(sns_low_lat_stream_start)(int handle, const uint64* sensor_suid, int sensor_suidLen, unsigned int sample_period_us, unsigned int flags) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(sns_low_lat_stream_stop)(int handle) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(sns_low_lat_stream_deinit)(int handle) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(sns_low_lat_stream_poll)(int handle) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(sns_low_lat_stream_forward_block)(int sleep_in_ms) __QAIC_HEADER_ATTRIBUTE;
#ifdef __cplusplus
}
#endif
#endif //_SNS_LOW_LAT_STREAM_H
