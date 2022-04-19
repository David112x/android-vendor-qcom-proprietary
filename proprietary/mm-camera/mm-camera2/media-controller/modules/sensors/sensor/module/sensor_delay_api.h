/* sensor_delay_api.h
 *
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SENSOR_DELAY_API_H__
#define __SENSOR_DELAY_API_H__

#define SEN_MAX_DATA_DELAY (7)

#define IDX(_delay, _p) \
  ((_p->delay.in_idx + SEN_MAX_DATA_DELAY) - _p->delay_val[_delay]) % \
  SEN_MAX_DATA_DELAY

#define GET(_delay, _p, _type) \
  _p->delay.data[IDX(_delay, _p)].s[_delay].is_float ? \
  (typeof(_type))_p->delay.data[IDX(_delay, _p)].s[_delay].u.float_val : \
  (typeof(_type))_p->delay.data[IDX(_delay, _p)].s[_delay].u.int_val

#define SET(_idx, _delay, _p, _val) \
  (_p->delay.data[_idx].s[_delay].is_float ? \
  (_p->delay.data[_idx].s[_delay].u.float_val = _val) : \
  (_p->delay.data[_idx].s[_delay].u.int_val = _val))

typedef struct {
  struct {
    boolean is_float;
    union {
      uint32_t int_val;
      float float_val;
    } u;
  }s[SENSOR_DELAY_MAX];
}sen_delay_data_t;

typedef struct {
  uint32_t in_idx;
  sen_delay_data_t data[SEN_MAX_DATA_DELAY];
}sen_delay_t;

typedef struct {
  uint8_t delay_val[SENSOR_DELAY_MAX];
  sen_delay_t delay;
  boolean init;
}sen_delay_api_t;

int32_t sensor_delay_init(void *sctrl);
int32_t sensor_delay_in(void *sctrl, sensor_exposure_info_t *exp_info);
int32_t sensor_delay_out(void *sctrl, sensor_exposure_info_t *exp_info);
int32_t sensor_delay_get_max(void *sctrl);

#endif //__SENSOR_DELAY_API_H__
