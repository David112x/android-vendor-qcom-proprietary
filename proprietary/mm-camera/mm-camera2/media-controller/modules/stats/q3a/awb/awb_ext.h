/* awb_ext.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AWB_EXT_H__
#define __AWB_EXT_H__

#include "awb_port.h"
#include "awb.h"
#include "stats_util.h"

#define MAX_CNT_RESTORE 15

#define IS_HISTORY_H_TO_L(_history) \
  (_history.prev == TRUE && _history.crnt == FALSE)
#define IS_HISTORY_L_TO_H(_history) \
  (_history.prev == FALSE && _history.crnt == TRUE)

#define IS_FLASH_ON(_ext_param) \
  (_ext_param->led_state.crnt | _ext_param->use_led_estimation.crnt)
#define IS_OFFGAIN_SAVED(_ext_param) (_ext_param->is_off_saved == TRUE)
#define IS_USE_LED_EST(_ext_param)    (_ext_param->use_led_estimation.crnt == TRUE)

#define IS_EST_ALGO_DONE(_ext_param) (_ext_param->led_calc_done.crnt)
#define IS_EST_PORT_DONE(_ext_param) (_ext_param->is_awb_est_done)
#define SET_EST_DONE(_ext_param)    (_ext_param->is_awb_est_done = TRUE)
#define CLEAR_EST_DONE(_ext_param)  (_ext_param->is_awb_est_done = FALSE)

#define IS_AF_UPDATE_VALID(_stats_update) \
  (stats_update && stats_update->flag == STATS_UPDATE_AF)
#define IS_AF_STATUS_DONE(_af_status) \
  (_af_status == AF_STATUS_FOCUSED || _af_status == AF_STATUS_UNKNOWN)
#define IS_AF_DONE(_ext_param)      (_ext_param->is_af_done)
#define SET_AF_DONE(_ext_param)     (_ext_param->is_af_done = TRUE)
#define CLEAR_AF_DONE(_ext_param)   (_ext_param->is_af_done = FALSE)

#define IS_UNIFIED_SAVED(_ext_param) (_ext_param->is_fi_saved == TRUE)
#define SET_UNIFIED(_ext_param)     (_ext_param->is_fi_saved = TRUE)
#define CLEAR_UNIFIED(_ext_param)   (_ext_param->is_fi_saved = FALSE)

#define IS_LED_NEEDED(_ext_param)        (ext_param->led_needed)
#define IS_LED_STATE(_ext_param, _enum)  (_ext_param->led_sm == _enum)
#define SET_LED_STATE(_ext_param, _enum) (_ext_param->led_sm = _enum)
#define IS_NO_LED(_ext_param) \
  ((ext_param->led_mode == LED_MODE_OFF) || \
  (ext_param->led_mode == LED_MODE_AUTO && !IS_LED_NEEDED(ext_param)))

#define IS_LOCKED_HAL(_ext_param)    (_ext_param->lock.hal == TRUE)
#define IS_LOCKED_ALGO(_ext_param)    (_ext_param->lock.algo == TRUE)

typedef enum {
  AWB_EXT_LED_STATUS_OFF = 0,
  AWB_EXT_LED_STATUS_PREFLASH_AEC,
  AWB_EXT_LED_STATUS_PREFLASH_AF,
  AWB_EXT_LED_STATUS_PREFLASH_AWB,
  AWB_EXT_LED_STATUS_PREFLASH_LOCKED,
  AWB_EXT_LED_STATUS_RESTORE_OFFGAIN,
  AWB_EXT_NO_LED_STATUS,
} awb_ext_led_state_machine;

/**
 * Custom output mask.
 */
typedef enum {
  AWB_OUTPUT_CUSTOM_MAX
} awb_ext_output_mask_type;

/**
 * Custom output structure. External core should define
 * the parameters here.
 */
typedef struct {
  uint32_t mask;
  /* Add parameters here */
} awb_ext_output_t;

typedef struct {
  boolean hal;  /* lock from HAL */
  boolean algo; /* lock in Algorithm */
} awb_ext_lock_t;

typedef struct {
  int prev;
  int crnt;
} awb_ext_state_history_t;

typedef struct _awb_ext_param {
  awb_ext_output_t custom_output;

  awb_ext_lock_t lock;
  awb_ext_state_history_t use_led_estimation;
  awb_ext_state_history_t led_state;
  int led_needed;
  boolean is_af_done;
  boolean is_awb_est_done;
  awb_bestshot_mode_type_t awb_best_shot;
  awb_config3a_wb_t awb_current_wb;

  awb_gain_t gain;
  awb_ccm_type ccm;
  int color_temp;

  awb_gain_t off_gain;
  awb_ccm_type off_ccm;
  int off_color_temp;

  awb_frame_batch_t fi;
  boolean is_fi_saved;
  boolean is_off_saved;
  awb_ext_state_history_t led_calc_done;

  int led_mode;

  awb_ext_led_state_machine led_sm;
  int cnt_restore;
} awb_ext_param_t;


boolean awb_port_ext_is_extension_required(void **awb_libptr,
  cam_position_t cam_position, boolean *use_default_algo);
boolean awb_port_ext_update_func_table(void *ptr);
void * awb_port_ext_load_function(awb_ops_t *awb_ops, void *awb_libptr,
  cam_position_t cam_position, boolean use_default_algo);
void awb_port_ext_unload_function(awb_port_private_t *private);



#endif /*__AWB_EXT_H__*/
