/*============================================================================

  Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __LED_FLASH_H__
#define __LED_FLASH_H__

#include "sensor_common.h"
#include "flash_lib.h"

// Red Eye Reduction (RER) Timing Limitations
#define PREFLASH_CYCLES_MIN       (1)       // [times]
#define PREFLASH_CYCLES_MAX       (200)     // [times]
#define LED_ON_MS_MIN             (1)       // [ms]
#define LED_ON_MS_MAX             (200)     // [ms]
#define LED_OFF_MS_MIN            (1)       // [ms]
#define LED_OFF_MS_MAX            (200)     // [ms]
#define RER_DURATION_MS_MIN       (10)      // [ms]
#define RER_DURATION_MS_MAX       (2000)    // [ms]
#define RER_PUPIL_CONTRACT_TIME   (300)     // [ms]

typedef enum {
  RER_START = 0,
  RER_WAIT_PUPIL_CONTRACT,
  RER_DONE,
} rer_status_t;

typedef struct {
  red_eye_reduction_type    *cfg;
  struct timeval            rer_start_ts;
  struct timeval            last_rer_flash_ts;
  rer_status_t              status;
} rer_cfg_t;

typedef struct {
  int32_t low_setting[MAX_LED_TRIGGERS];  /* current values for pre-flash */
  int32_t high_setting[MAX_LED_TRIGGERS];  /* crrent values for main-flash */
} dual_led_settings_t;

typedef struct {
  int32_t              fd;
  rer_cfg_t            *rer;
  int32_t              flash_max_duration[MAX_LED_TRIGGERS];
  int32_t              flash_max_current[MAX_LED_TRIGGERS];
  dual_led_settings_t  dual_led_setting;
  void                 *driver_lib_handle;
  flash_lib_t          *driver_lib_data;
} sensor_flash_data_t;

int32_t flash_rer_set_current(
  dual_led_settings_t *flash_update,
  dual_led_settings_t *dualled_setting);

int32_t flash_rer_set_chromatix(
  rer_cfg_t               *rer,
  red_eye_reduction_type  *rer_chromatix);

int32_t flash_rer_sequence_process(
  rer_cfg_t               *rer,
  module_sensor_params_t  *led_module_params);

int32_t flash_rer_wait_pupil_contract(
  rer_cfg_t              *rer,
  module_sensor_params_t *led_module_params);

int32_t flash_rer_set_parm(
  rer_cfg_t               *rer,
  int32_t                  mode);

int32_t flash_set_current(
  dual_led_settings_t *flash_update,
  dual_led_settings_t *dualled_setting);

int32_t flash_get_frame_skip_timing_params(
    module_sensor_bundle_info_t* s_bundle,
    sensor_flash_data_t*         flash_ctrl);

#endif
