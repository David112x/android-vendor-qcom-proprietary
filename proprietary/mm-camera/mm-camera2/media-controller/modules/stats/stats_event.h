/* stats_event.h
 *
 * Copyright (c) 2013 - 2015, 2019 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __STATS_EVENT_H__
#define __STATS_EVENT_H__
#include "mct_event_stats.h"

/* Size base in recomended size by the hardware API */
#define STATS_GROUP_0 0
#define STATS_GROUP_1 1

/** _mct_event_gravity_vector_enable
 *  @enable: boolean flag - true if enable false if disable.
 *  @report_period: how often to get gravity data, units of
 *    seconds, Q16
 *  @sample_rate_valid: true if sampling rate is passed.
 *  @sample_rate: sample rate in Hz, Q16. If less than report
 *    period, it is set to report rate.
 *
 *  This structure is used to enable/disable gravity vector.
 *
 **/
typedef struct _mct_event_gravity_vector_enable {
  boolean enable;
  uint32_t report_period;
  uint8_t sample_rate_valid;
  uint32_t sample_rate;
} mct_event_gravity_vector_enable_t;

/** _mct_event_light_sensor_update
 *  @lux:  light sensor value is returned in SI lux units.
 *
 *  This structure is used to store light sensor data
 *
 **/
typedef struct _mct_event_light_sensor_update {
  float lux;
} mct_event_light_sensor_update_t;

typedef struct _mct_event_stats_ext_t {
  pthread_mutex_t*        stats_mutex;
  int32_t                 ref_count;
  mct_event_stats_isp_t*  stats_data;
} mct_event_stats_ext_t;

typedef struct {
  uint32_t is_gyro_sampling_rate;
  uint32_t rs_interval_offset;
  uint32_t s3d_interval_offset;
} mct_event_sns_config_request_t;

typedef struct {
  boolean  is_af_hj_enabled;
  uint32_t af_hj_gyro_sampling_rate;
  uint32_t af_hj_gyro_report_rate;
} mct_event_af_gyro_config_request_t;

#endif /* __STATS_EVENT_H__ */
