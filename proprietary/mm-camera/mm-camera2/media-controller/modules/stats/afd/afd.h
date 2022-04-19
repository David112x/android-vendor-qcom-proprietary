/* afd.h
 *
 * Copyright (c) 2013-2016, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AFD_H__
#define __AFD_H__

#include "q3a_common_stats.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CAFD_DELAY 120

typedef enum {
  AFD_STATUS_OFF = 0,
  AFD_STATUS_REGULAR_EXPOSURE_TABLE,   /* 1 */
  AFD_STATUS_60HZ_EXPOSURE_TABLE,      /* 2 */
  AFD_STATUS_50HZ_EXPOSURE_TABLE,      /* 3 */
  AFD_STATUS_50HZ_AUTO_EXPOSURE_TABLE, /* 4 */
  AFD_STATUS_60HZ_AUTO_EXPOSURE_TABLE, /* 5 */
} afd_status_t;

typedef enum {
#if 1
  AFD_TYPE_OFF,
  AFD_TYPE_60HZ,
  AFD_TYPE_50HZ,
  AFD_TYPE_AUTO,
  AFD_TYPE_AUTO_50HZ,
  AFD_TYPE_AUTO_60HZ,
  AFD_TYPE_MAX,
#else
  CAMERA_ANTIBANDING_OFF,
  CAMERA_ANTIBANDING_60HZ,
  CAMERA_ANTIBANDING_50HZ,
  CAMERA_ANTIBANDING_AUTO,
  CAMERA_ANTIBANDING_AUTO_50HZ,
  CAMERA_ANTIBANDING_AUTO_60HZ,
  CAMERA_MAX_ANTIBANDING,
#endif
} afd_type_t;

typedef enum {
  AFD_OFF_VALUE = 0,
  AFD_60HZ_VALUE = 60,
  AFD_50HZ_VALUE = 50,
} afd_flicker_freq_t;

typedef enum {
  AFD_STATE_INIT = 0,
  AFD_STATE_ACTIVE,
  AFD_STATE_INACTIVE,
  AFD_STATE_INVALID_STATE,
  AFD_STATE_OFF,
  AFD_STATE_ON,
  AFD_STATE_BUSY,
  AFD_STATE_DONE,
} afd_state_t;

typedef enum {
  AFD_CB_OUTPUT = 1,
  AFD_CB_STATS_CONFIG = (1 << 1),
} afd_cb_type_t;

typedef struct {
  boolean trigger;
  int frame_skip_cnt;
  int status;
  int conti_afd_delay;
} afd_conti_t;

typedef enum {
  AFD_SET_PARAM_INIT_CHROMATIX   = 1,
  AFD_SET_AEC_PARAM,
  AFD_SET_SENSOR_PARAM,
  AFD_SET_AF_PARAM,
  AFD_SET_ENABLE,
  /* TODO Either Actually send the event from camera hal and mm-camerea or remove it
      enabling only at customer side is risky
   */
  AFD_SET_RESET,
  AFD_SET_STATS_DEBUG_MASK,
  AFD_SET_SOF,
  AFD_SET_PARAM_STATS_DEPTH,
  AFD_SET_PARAM_ROLE_SWITCH,
  AFD_SET_PARAM_3HDR_ENABLE,
  AFD_SET_PARAM_MAX
} afd_set_parameter_type;


/** afd_role_switch_params_t:
 *    @master: If AFD is master/slave
 *    @afd_enable:
 *    @afd_type:
 *    @afd_monitor:
 *    @afd_exec_once:
 *
 * Stores the current AFD information of the master and
 * which will be used in next  Master camera session
 * as a starting point to improve the convergence.
 **/
typedef struct _afd_role_switch_params_t {
  boolean     master;
  boolean     afd_enable;
  afd_type_t  afd_type;
  boolean     afd_monitor;
  boolean     afd_exec_once;
}afd_role_switch_params_t;

typedef struct {
  /* static level 3 slope check threshold */
  int     static_slope_steep_thr;
  int     static_slope_not_steep_thr;
  /* static level 2 confidence calculation */
  int     static_confidence_level_h;
  int     static_confidence_level_l;
  float   static_confidence_level_h_ratio;
  float   static_confidence_level_l_ratio;
  float   static_confidence_level_h_ll_ratio;
  float   static_confidence_level_l_ll_ratio;
  float   static_confidence_level_slope_ratio;
  /* low light static tuning parameters */
  float   static_lux_idx_lowlight_threshold;
  float   static_threshold_level_compensation;
  int     static_row_sum_lowlight_thr;
  int     static_row_sum_lowlight_comp;
} afd_extended_parameters_t;

typedef struct {
  void *chromatix;
  afd_extended_parameters_t extended_params;
} afd_set_parameter_init_t;

typedef struct {
  boolean afd_enable;
  afd_type_t afd_mode;
} afd_set_enable_t;

/* Below should come from AEC update */
typedef struct {
  boolean aec_settled;

  int  max_line_cnt;

  uint32_t preview_fps;
  uint32_t max_preview_fps;
  uint32_t cur_line_cnt;
  float    band_50hz_gap;
  afd_type_t  aec_atb;
  uint32_t preview_linesPerFrame;
  uint32_t sen_dim_height;

  boolean  af_fixed_lens;

  boolean  af_active;
  boolean  cont_af_enabled;
  float    exp_time;
  float    real_gain;
  float    lux_idx;
  uint32_t max_sensor_preview_fps;
} afd_data_from_aec_and_af_t;

typedef struct _afd_set_parameter {
  afd_set_parameter_type type;

  union {
    unsigned int set_sof_id;
    afd_set_parameter_init_t   init_param;
    afd_set_enable_t           set_enable;
    afd_data_from_aec_and_af_t aec_af_data;
    uint32_t                   stats_debug_mask;
    uint32_t                   stats_depth;
    afd_role_switch_params_t   role_switch;
    boolean                    HDR_enable;
  } u;
} afd_set_parameter_t;

typedef enum {
  AFD_GET_STATS_CONFIG,
  AFD_GET_MAX
} afd_get_parameter_type;


typedef struct _afd_get_parameter {
  afd_get_parameter_type type;
  uint32_t  stats_hnum;
  uint32_t  stats_vnum;
} afd_get_parameter_t;

typedef struct {
  afd_data_from_aec_and_af_t aec_af_data;
} afd_process_data_t;


/** afd_output_data
 *
 **/
typedef struct {
  afd_cb_type_t type;
  boolean      afd_enable;
  boolean      afd_monitor;
  boolean      afd_exec_once;
  boolean      flicker_detect; /* afd->flicker */
  boolean      eztune_enabled;
  int          flicker_freq;
  int          actual_peaks;
  int          multiple_peak_algo; /* afd->num_peaks */
  int          std_width;

  afd_state_t  afd_state;
  afd_status_t afd_status;
  afd_type_t   afd_atb;
  uint32_t     sof_id;
  uint32_t     max_algo_hnum;
  uint32_t     max_algo_vnum;
} afd_output_data_t;

void *afd_init(void);
int afd_set_parameters(afd_set_parameter_t *param,
  void *afd, afd_output_data_t *output);
boolean afd_get_parameters(afd_get_parameter_t *param, void *afd);
boolean afd_process(q3a_core_bg_stats_type *bg_stats,
                    q3a_core_row_sum_stats_type *row_sum_stats,
                    void *afd, afd_output_data_t *output);
void afd_destroy(void *afd);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* __AFD_H__ */
