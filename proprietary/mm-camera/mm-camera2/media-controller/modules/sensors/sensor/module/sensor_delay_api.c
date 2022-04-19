/* sensor_delay_api.c
 *
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include "sensor.h"
#include "sensor_delay_api.h"

static const boolean delay_data_type[SENSOR_DELAY_MAX] = {
  [SENSOR_DELAY_EXPOSURE]            = FALSE,
  [SENSOR_DELAY_ANALOG_SENSOR_GAIN]  = FALSE,
  [SENSOR_DELAY_DIGITAL_SENSOR_GAIN] = TRUE,
  [SENSOR_DELAY_ISP_GAIN]            = TRUE,
};


static inline void sensor_delay_add_new_entry_data(sen_delay_api_t *delay_api,
  sensor_exposure_info_t *in, uint32_t idx)
{
  SET(idx, SENSOR_DELAY_ANALOG_SENSOR_GAIN, delay_api, in->reg_gain);
  SET(idx, SENSOR_DELAY_EXPOSURE, delay_api, in->line_count);
  SET(idx, SENSOR_DELAY_ISP_GAIN, delay_api, in->digital_gain);
  SET(idx, SENSOR_DELAY_DIGITAL_SENSOR_GAIN, delay_api, in->sensor_digital_gain);
  return;
}

static inline void sensor_delay_get_data(sen_delay_api_t *delay_api,
  sensor_exposure_info_t *in)
{
  in->reg_gain = GET(SENSOR_DELAY_ANALOG_SENSOR_GAIN, delay_api, in->reg_gain);
  in->line_count = GET(SENSOR_DELAY_EXPOSURE, delay_api, in->line_count);
  in->digital_gain = GET(SENSOR_DELAY_ISP_GAIN, delay_api, in->digital_gain);
  in->sensor_digital_gain =
    GET(SENSOR_DELAY_DIGITAL_SENSOR_GAIN, delay_api, in->sensor_digital_gain);
  return;
}

static inline void sensor_delay_data_fill(sen_delay_api_t *delay_api,
  sensor_exposure_info_t *in)
{
  uint32_t i;
  for (i = 0; i < SEN_MAX_DATA_DELAY; i++)
    sensor_delay_add_new_entry_data(delay_api, in, i);
  return;
}

/** sensor_delay_get_max: Return max sensor delay value
 *
 *  @sctrl: sensor handle
 *
 *  Return: TRUE for success and FALSE on failure
 **/
int32_t sensor_delay_get_max(void *sctrl)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_t    *sensor_lib_ptr;
  int32_t max_delay = 0, i;

  RETURN_ERR_ON_NULL(ctrl, 0);
  RETURN_ERR_ON_NULL(ctrl->lib_params, 0);
  sensor_lib_ptr = ctrl->lib_params->sensor_lib_ptr;
  RETURN_ERR_ON_NULL(sensor_lib_ptr, 0);

  for (i = 0; i < SENSOR_DELAY_MAX; i++) {
    if (sensor_lib_ptr->app_delay[i] > max_delay)
      max_delay = sensor_lib_ptr->app_delay[i];
  }
  return max_delay;
}

/** sensor_delay_init: Init delay api
 *
 *  @sctrl: sensor handle
 *
 *  Return: TRUE for success and FALSE on failure
 **/
int32_t sensor_delay_init(void *sctrl)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sen_delay_api_t *delay_api;
  sensor_lib_t    *sensor_lib_ptr;
  int32_t i,j;


  RETURN_ERR_ON_NULL(ctrl, -EINVAL);
  delay_api = &ctrl->delay_api;
  RETURN_ERR_ON_NULL(ctrl->lib_params, -EINVAL);
  sensor_lib_ptr = ctrl->lib_params->sensor_lib_ptr;
  RETURN_ERR_ON_NULL(sensor_lib_ptr, -EINVAL);

  SLOW("Enter");
  memset(delay_api, 0 , sizeof(sen_delay_api_t));

  for (i = 0; i < SEN_MAX_DATA_DELAY; i++) {
    for (j = 0; j < SENSOR_DELAY_MAX; j++) {
      if (sensor_lib_ptr->app_delay[j] < SEN_MAX_DATA_DELAY)
        delay_api->delay_val[j] = sensor_lib_ptr->app_delay[j] ;
      else {
        delay_api->delay_val[j] = 0 ;
        SHIGH("Invalid delay: param:%d delay:%d",
          j, sensor_lib_ptr->app_delay[j]);
      }
      delay_api->delay.data[i].s[j].is_float = delay_data_type[j];
    }
  }
  SLOW("Exit");
  return 0;
}

/** sensor_delay_in: Input sensor parameters
 * Adding current sensor data in "delay api array".
 *
 *  @sctrl: sensor handle
 *  @exp_info: sensor parameters
 *
 *  Return: TRUE for success and FALSE on failure
 **/
int32_t sensor_delay_in(void *sctrl, sensor_exposure_info_t *exp_info)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sen_delay_api_t *delay_api;
  sen_delay_t *delay;

  SLOW("Enter");
  delay_api = &ctrl->delay_api;
  delay = &delay_api->delay;

  RETURN_ERR_ON_NULL(exp_info, -EINVAL);

  PTHREAD_MUTEX_LOCK(&ctrl->s_data->mutex);
  if (!delay_api->init) {
    sensor_delay_data_fill(delay_api, exp_info);
    delay_api->init = 1;
    PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);
    SLOW("Exit");
    return 0;
  }

  delay->in_idx++;
  delay->in_idx %= SEN_MAX_DATA_DELAY;
  sensor_delay_add_new_entry_data(delay_api, exp_info, delay->in_idx);
  PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);
  SLOW("Exit");
  return 0;
}
/** sensor_delay_out: Output sensor parameters
 * Getting sensor parameters like expo/gain with including delays
 *
 *  @sctrl: sensor handle
 *  @exp_info: sensor parameters
 *
 *  Return: TRUE for success and FALSE on failure
 **/
int32_t sensor_delay_out(void *sctrl, sensor_exposure_info_t *exp_info)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sen_delay_api_t *delay_api;
  sen_delay_t *delay;

  SLOW("Enter");
  delay_api = &ctrl->delay_api;
  delay = &delay_api->delay;

  RETURN_ERR_ON_NULL(exp_info, -EINVAL);

  if (!delay_api->init) {
    SERR("Exit");
    return 0;
  }

  PTHREAD_MUTEX_LOCK(&ctrl->s_data->mutex);
  sensor_delay_get_data(delay_api, exp_info);
  PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);

  SLOW("Exit");
  return 0;
}

