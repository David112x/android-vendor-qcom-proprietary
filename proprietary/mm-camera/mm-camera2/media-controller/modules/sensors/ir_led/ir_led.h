/* ir_led.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __IR_LED_H__
#define __IR_LED_H__

#include "sensor_common.h"

#define IR_LED_FREQ_HZ             10000    /*10 KHz*/
#define ONE_SEC_TO_NS              1000000000
#define IR_LED_PERIOD_NS           (ONE_SEC_TO_NS / IR_LED_FREQ_HZ)
#define IR_LED_DUTY_NS(intensity_percentage)  \
   ((IR_LED_PERIOD_NS * intensity_percentage) / 100.00)

typedef struct {
  int            fd;
} sensor_ir_led_data_t;

#endif
