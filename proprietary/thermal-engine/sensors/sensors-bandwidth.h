/*===========================================================================
  copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#ifndef __BANDWIDTH_SENSOR_H__
#define __BANDWIDTH_SENSOR_H__

#include "sensors_manager_internal.h"

int bw_sensors_setup(struct sensor_info *sensor);
void bw_sensors_shutdown(struct sensor_info *sensor);
int bw_sensor_get_temperature(struct sensor_info *sensor);
void bw_sensor_interrupt_wait(struct sensor_info *sensor);
int get_bw_client_sensors_list(struct sensor_info ** sensors);

#endif  /* __BANDWIDTH_SENSOR_H__ */
