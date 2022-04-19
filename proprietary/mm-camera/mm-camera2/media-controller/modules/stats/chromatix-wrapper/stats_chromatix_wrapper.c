/* stats_chromatix_wrapper.c
 *
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "q3a_platform.h"
#include "stats_chromatix_wrapper.h"

#if 0
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wunused-parameter"
#pragma GCC diagnostic error "-Wunused-variable"
#pragma GCC diagnostic error "-Wsign-compare"
#pragma GCC diagnostic error "-Wunused-function"
#pragma GCC diagnostic error "-Wconversion"
#pragma GCC diagnostic error "-Wsign-conversion"
#endif

/* Since we don't have access to the C11 compile-time static_assert() method, this
   is a naive alternate implementation to do the same. The macro asserts the condition
   at compile time and enforces a compile error if assertion fails. Compilation errors
   such as below indicate that the compile-time assertion has failed and needs to be
   examined to determine the cause of error:
       error: size of array 'MY_ASSERT' is negative */
#define STATIC_ASSERT(cond) { char MY_ASSERT[ (!!(cond))*2 - 1 ]; }

/* This is a helper macro to memcopy an array while enforcing source
   and destination compile-time size checks. */
#define COPY_ARRAY(dest, source)                                               \
    STATIC_ASSERT(sizeof(dest) == sizeof(source));                             \
    Q3A_MEMCPY_S(dest, sizeof(dest), source, sizeof(source));

/* This is a helper macro to memcopy a single field while enforcing source
   and destination compile-time size checks. Similar to a safe-memcopy but
   with compile-time size checks rather than run-time size checks */
#define COPY_FIELD(dest, source)                                               \
    STATIC_ASSERT(sizeof(dest) == sizeof(source));                             \
    Q3A_MEMCPY_S(&dest, sizeof(dest), &source, sizeof(source));

/*=============================================================================
 *  Local AEC Definitions
 *=============================================================================*/

/* Warm start (store restore) exposure index range percentage */
#define WARM_START_EXP_RANGE_PERC                     (60)

/* Startup frame-skip in aggressive AEC convergence */
#define AGGRESSIVE_AEC_STARTUP_FRAME_SKIP              (3)

/* Frame-skip in aggressive AEC convergence */
#define AGGRESSIVE_AEC_FRAME_SKIP                      (2)

/* Aggressive AEC luma tolerance percentage */
#define AGGRESSIVE_AEC_LUMA_TOLERANCE_PCT              (10)

/* Max frame convergence threshold for AEC settle in aggressive AEC */
#define AGGRESSIVE_AEC_MAX_FRAME_CONVERGENCE_THRESHOLD (13)

/* Min settled frames needed to declare AEC is settled */
#define AGGRESSIVE_AEC_MIN_FRAME_SETTLE_THRESHOLD      (1)

/* Min settled frames needed to declare AWB is converged */
#define AGGRESSIVE_AEC_MIN_AWB_SETTLE_COUNT            (3)

/* This count determines how many frames to skip for fast AEC
 * convergence, ideally only 2 frames are needed */
#define FAST_AEC_FRAME_SKIP                       3

/* This count determines how many frames are to be skipped when
 * fast AEC is run the first time */
#define FAST_AEC_STARTUP_FRAME_SKIP               4

/* Luma settled count value for fast AEC */
#define FAST_AEC_MIN_FRAME_SETTLE_THRESHOLD       1

/* Default maximum ideal HDR gain */
#define MAX_IDEAL_HDR_GAIN                        8

/* Enable aec dynamic frame skip */
#define AEC_DYNAMIC_FRAME_SKIP_ENABLE (TRUE)

/* Exposure index tolerance for dynamic frame skip .*/
#define AEC_DYNAMIC_FRAME_SKIP_EXP_INDEX_TOLERANCE (1)

/* Min fps threshold considered for dynamic frame skip */
#define AEC_DYNAMIC_FRAME_SKIP_MIN_FPS_THRESHOLD (15)

/* Max number of frames to be skipped for stable scene */
#define AEC_DYNAMIC_FRAME_SKIP_DEFAULT_MAX_SKIP_COUNT  (1U)

/*=============================================================================
*  Local AWB Definitions
*=============================================================================*/
/* Exposure offset from maximum exposure index within which AWB white world
 * can be used. */
#define AWB_WHITEWORLD_DISABLE_LOWLIGHT   30

/*=============================================================================
 *  AF Methods
 *=============================================================================*/
void* af_get_algo_params(void);

static float af_pdaf_get_soft_scene_change_cnt(float soft_stable_cnt);
static int af_haf_fill_force_scan_cnt_th(int max_scan_cnt);


/** af_pdaf_get_soft_scene_change_cnt
* get scene change count from tuning
* default value is 1 and maximum threshold is 6
* @soft_scene_change_cnt: soft scene change count from tuning
*
*  Return: float
**/

static float af_pdaf_get_soft_scene_change_cnt(float soft_scene_change_cnt) {
  if (soft_scene_change_cnt < 1) {
    soft_scene_change_cnt = 1.0f;
  }
  else if (soft_scene_change_cnt > 6) {
    soft_scene_change_cnt = 6.0f;
  }
  return soft_scene_change_cnt;
}

static int af_haf_fill_force_scan_cnt_th(int max_scan_cnt){
  return (max_scan_cnt) ? max_scan_cnt : DEFAULT_MAX_SCAN_CNT_TH;
}

static void af_copy_single_threshold(single_threshold_adapter_t* dest, single_threshold_t* src) {
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  dest->change_ratio_inc   = src->change_ratio_inc;
  dest->change_ratio_dec   = src->change_ratio_dec;
#endif
  dest->flat_inc_thres     = src->flat_inc_thres;
  dest->flat_dec_thres     = src->flat_dec_thres;
  dest->macro_thres        = src->macro_thres;
  dest->drop_thres         = src->drop_thres;
  dest->hist_dec_dec_thres = src->hist_dec_dec_thres;
  dest->hist_inc_dec_thres = src->hist_inc_dec_thres;

  COPY_FIELD(dest->dec_dec_3frame, src->dec_dec_3frame);
  COPY_FIELD(dest->inc_dec_3frame, src->inc_dec_3frame);
  COPY_FIELD(dest->dec_dec, src->dec_dec);
  COPY_FIELD(dest->dec_dec_noise, src->dec_dec_noise);
  COPY_FIELD(dest->inc_dec, src->inc_dec);
  COPY_FIELD(dest->inc_dec_noise, src->inc_dec_noise);
  COPY_FIELD(dest->flat_threshold, src->flat_threshold);
}

static void af_copy_tuning_sad(af_tuning_sad_adapter_t* dest, af_tuning_sad_t* src) {
  dest->enable            = src->enable;
  dest->gain_min          = src->gain_min;
  dest->gain_max          = src->gain_max;
  dest->ref_gain_min      = src->ref_gain_min;
  dest->ref_gain_max      = src->ref_gain_max;
  dest->threshold_min     = src->threshold_min;
  dest->threshold_max     = src->threshold_max;
  dest->ref_threshold_min = src->ref_threshold_min;
  dest->ref_threshold_max = src->ref_threshold_max;
  dest->frames_to_wait    = src->frames_to_wait;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  dest->unstable_count_th     = src->unstable_count_th;
  dest->stable_count_th       = src->stable_count_th;
  dest->grid_region_start     = src->grid_region_start;
  dest->grid_region_end       = src->grid_region_end;
  dest->sadr_threshold_scaler = src->sadr_threshold_scaler;

  COPY_ARRAY(dest->aec_gain_input, src->aec_gain_input);
  COPY_ARRAY(dest->sadr_threshold_scaler_multiplier, src->sadr_threshold_scaler_multiplier);
#endif
}

boolean parse_AF_chromatix(void* af_tuning_info,
  af_algo_tune_parms_adapter_t* tuning_adapter_info)
{
  int i = 0, j = 0;
  af_algo_tune_parms_t* tuning_info = (af_algo_tune_parms_t*)af_tuning_info;

  if (tuning_info == NULL)
  {
    AF_ERR("error: tuning_info is NULL");
    return false;
  }

  if (tuning_adapter_info == NULL)
  {
    AF_ERR("error: tuning_adapter_info is NULL");
    return false;
  }

/******************************************************************************
** af_header_info_adapter_t: af_algo_tune_parms_t
******************************************************************************/
   STATIC_ASSERT(sizeof(af_header_info_adapter_t) == sizeof(af_header_info_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_header_info,
    sizeof(af_header_info_adapter_t),
    &tuning_info->af_header_info,
    sizeof(af_header_info_t));

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_caf.fv =
     tuning_info->af_monitor.monitor_algo_config.topo_caf.fv;
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_caf.gyro =
     tuning_info->af_monitor.monitor_algo_config.topo_caf.gyro;
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_caf.gyro_panning =
     tuning_info->af_monitor.monitor_algo_config.topo_caf.gyro_panning;
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_caf.profile_index =
     (af_vm_topo_profile_caf_adapter_enum)tuning_info->af_monitor.monitor_algo_config.topo_caf.profile_index;
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_caf.sad_panning =
     tuning_info->af_monitor.monitor_algo_config.topo_caf.sad_panning;
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_caf.sadr =
     tuning_info->af_monitor.monitor_algo_config.topo_caf.sadr;
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_caf.face_sc =
     tuning_info->af_monitor.monitor_algo_config.topo_caf.reserved_sens[0];

   tuning_adapter_info->af_monitor.monitor_algo_config.topo_pdaf.confidence_panning =
     tuning_info->af_monitor.monitor_algo_config.topo_pdaf.confidence_panning;
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_pdaf.defocus_high =
     tuning_info->af_monitor.monitor_algo_config.topo_pdaf.defocus_high;
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_pdaf.defocus_low =
     tuning_info->af_monitor.monitor_algo_config.topo_pdaf.defocus_low;
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_pdaf.profile_index =
     (af_vm_topo_profile_pdaf_adapter_enum)tuning_info->af_monitor.monitor_algo_config.topo_pdaf.profile_index;

   tuning_adapter_info->af_monitor.monitor_algo_config.topo_tof.distance =
     tuning_info->af_monitor.monitor_algo_config.topo_tof.distance;
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_tof.profile_index =
     (af_vm_topo_profile_tof_adapter_enum)tuning_info->af_monitor.monitor_algo_config.topo_tof.profile_index;

   /* modified/reserved parameters */
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_caf.caf_sc =
     tuning_info->af_monitor.monitor_algo_config.topo_caf.reserved_sens[1];

   tuning_adapter_info->af_monitor.monitor_algo_config.topo_tof.confidence_medium =
     tuning_info->af_monitor.monitor_algo_config.topo_tof.reserved_sens[0];
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_tof.confidence_high =
     tuning_info->af_monitor.monitor_algo_config.topo_tof.confidence;

   tuning_adapter_info->af_monitor.monitor_algo_config.topo_dciaf.confidence =
     tuning_info->af_monitor.monitor_algo_config.topo_dciaf.confidence;
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_dciaf.distance =
     tuning_info->af_monitor.monitor_algo_config.topo_dciaf.distance;
   tuning_adapter_info->af_monitor.monitor_algo_config.topo_dciaf.profile_index =
     (af_vm_topo_profile_dciaf_adapter_enum)tuning_info->af_monitor.monitor_algo_config.topo_dciaf.profile_index;

   /* reserved parameters */
   tuning_adapter_info->af_monitor.monitor_algo_config.medium_confidence_logic =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[0];
   tuning_adapter_info->af_monitor.monitor_algo_config.secondary_scene_change_cnt_thres =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[1];
   tuning_adapter_info->af_monitor.monitor_algo_config.secondary_unstable_cnt_thres =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[2];
   tuning_adapter_info->af_monitor.monitor_algo_config.caf_topology_profile_advanced =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[3];
   tuning_adapter_info->af_monitor.monitor_algo_config.pdaf_topology_profile_advanced =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[4];
   tuning_adapter_info->af_monitor.monitor_algo_config.tof_topology_profile_advanced =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[5];
   tuning_adapter_info->af_monitor.monitor_algo_config.dciaf_topology_profile_advanced =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[6];
   tuning_adapter_info->af_monitor.monitor_algo_config.enable_bemo_for_primary_algo =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[7];
   tuning_adapter_info->af_monitor.monitor_algo_config.enable_bemo_for_secondary_algo =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[8];

   /* reserved parameters for bemo (primary) */
   tuning_adapter_info->af_monitor.monitor_algo_config.bemo_primary_params.recover_time =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[9];
   tuning_adapter_info->af_monitor.monitor_algo_config.bemo_primary_params.lp_tolerance =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[10];
   tuning_adapter_info->af_monitor.monitor_algo_config.bemo_primary_params.frame_skip_thres =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[11];
   tuning_adapter_info->af_monitor.monitor_algo_config.bemo_primary_params.hammering_time =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[12];
   tuning_adapter_info->af_monitor.monitor_algo_config.bemo_primary_params.sens_dec_delta =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[13];
   tuning_adapter_info->af_monitor.monitor_algo_config.bemo_primary_params.sens_inc_delta =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[14];

   /* reserved parameters for bemo (secondary) */
   tuning_adapter_info->af_monitor.monitor_algo_config.bemo_secondary_params.recover_time =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[15];
   tuning_adapter_info->af_monitor.monitor_algo_config.bemo_secondary_params.lp_tolerance =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[16];
   tuning_adapter_info->af_monitor.monitor_algo_config.bemo_secondary_params.frame_skip_thres =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[17];
   tuning_adapter_info->af_monitor.monitor_algo_config.bemo_secondary_params.hammering_time =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[18];
   tuning_adapter_info->af_monitor.monitor_algo_config.bemo_secondary_params.sens_dec_delta =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[19];
   tuning_adapter_info->af_monitor.monitor_algo_config.bemo_secondary_params.sens_inc_delta =
        (int)tuning_info->af_monitor.monitor_algo_config.mixer_reserved[20];

   for (i = 0; i < Q3A_VM_SET_TYPE_MAX; i++) {
      tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].detection_type =
             (af_vm_detector_type_adapter_enum)tuning_info->af_monitor.value_monitor_bank.value_monitor[i].detection_type;

     COPY_FIELD(tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].absolute_mode_cfg,
               tuning_info->af_monitor.value_monitor_bank.value_monitor[i].absolute_mode_cfg);


     tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_low.trigger_threshold =
       tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_low.trigger_threshold;

     tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_low.consecutive_count_threshold =
       tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_low.consecutive_count_threshold;

     tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_low.frame2armed =
       tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_low.frame2armed;


     COPY_FIELD(tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_low.median_fltr_cfg,
               tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_low.median_fltr_cfg);

     COPY_FIELD(tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_low.mvavg_fltr_cfg,
               tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_low.mvavg_fltr_cfg);

     COPY_FIELD(tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_low.iir_fltr_cfg,
               tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_low.iir_fltr_cfg);

     tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_med.trigger_threshold =
       tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_med.trigger_threshold;

     tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_med.consecutive_count_threshold =
       tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_med.consecutive_count_threshold;

     tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_med.frame2armed =
       tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_med.frame2armed;

     COPY_FIELD(tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_med.median_fltr_cfg,
               tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_med.median_fltr_cfg);

     COPY_FIELD(tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_med.mvavg_fltr_cfg,
               tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_med.mvavg_fltr_cfg);

     COPY_FIELD(tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_med.iir_fltr_cfg,
               tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_med.iir_fltr_cfg);

     tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_high.trigger_threshold =
       tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_high.trigger_threshold;

     tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_high.consecutive_count_threshold =
       tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_high.consecutive_count_threshold;

     tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_high.frame2armed =
       tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_high.frame2armed;

     COPY_FIELD(tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_high.median_fltr_cfg,
               tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_high.median_fltr_cfg);

     COPY_FIELD(tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_high.mvavg_fltr_cfg,
               tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_high.mvavg_fltr_cfg);

     COPY_FIELD(tuning_adapter_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_high.iir_fltr_cfg,
               tuning_info->af_monitor.value_monitor_bank.value_monitor[i].sens_profile_high.iir_fltr_cfg);
    }
#endif

/******************************************************************************
** af_tuning_algo_adapter_t:af_algo_tune_parms_t
******************************************************************************/
   tuning_adapter_info->af_algo.af_process_type =
    (af_algo_adapter_t)tuning_info->af_algo.af_process_type;
  tuning_adapter_info->af_algo.position_near_end =
    tuning_info->af_algo.position_near_end;
  tuning_adapter_info->af_algo.position_default_in_macro =
    tuning_info->af_algo.position_default_in_macro;
  tuning_adapter_info->af_algo.position_boundary =
    tuning_info->af_algo.position_boundary;
  tuning_adapter_info->af_algo.position_default_in_normal =
    tuning_info->af_algo.position_default_in_normal;
  tuning_adapter_info->af_algo.position_far_end =
    tuning_info->af_algo.position_far_end;
  tuning_adapter_info->af_algo.position_normal_hyperfocal =
    tuning_info->af_algo.position_normal_hyperfocal;
  tuning_adapter_info->af_algo.position_macro_rgn =
    tuning_info->af_algo.position_macro_rgn;
  tuning_adapter_info->af_algo.undershoot_protect =
    tuning_info->af_algo.undershoot_protect;
  tuning_adapter_info->af_algo.undershoot_adjust =
    tuning_info->af_algo.undershoot_adjust;
  tuning_adapter_info->af_algo.min_max_ratio_th =
    tuning_info->af_algo.min_max_ratio_th;
  tuning_adapter_info->af_algo.led_af_assist_enable =
    tuning_info->af_algo.led_af_assist_enable;
  tuning_adapter_info->af_algo.led_af_assist_trigger_idx =
    tuning_info->af_algo.led_af_assist_trigger_idx;
  tuning_adapter_info->af_algo.lens_reset_frame_skip_cnt =
    tuning_info->af_algo.lens_reset_frame_skip_cnt;
  tuning_adapter_info->af_algo.low_light_gain_th =
    tuning_info->af_algo.low_light_gain_th;
  tuning_adapter_info->af_algo.base_delay_adj_th =
    tuning_info->af_algo.base_delay_adj_th;

  tuning_adapter_info->af_algo.af_haf.enable =
    tuning_info->af_algo.af_haf.enable;
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.algo_enable,
             tuning_info->af_algo.af_haf.algo_enable);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.stats_consume_enable,
             tuning_info->af_algo.af_haf.stats_consume_enable);

  tuning_adapter_info->af_algo.af_haf.lens_sag_comp_enable =
    tuning_info->af_algo.af_haf.lens_sag_comp_enable;
  tuning_adapter_info->af_algo.af_haf.hysteresis_comp_enable =
    tuning_info->af_algo.af_haf.hysteresis_comp_enable;
  tuning_adapter_info->af_algo.af_haf.actuator_shift_comp =
    tuning_info->af_algo.af_haf.actuator_shift_comp;
  tuning_adapter_info->af_algo.af_haf.actuator_hysteresis_comp =
    tuning_info->af_algo.af_haf.actuator_hysteresis_comp;
  tuning_adapter_info->af_algo.af_haf.actuator_sensitivity =
    tuning_info->af_algo.af_haf.actuator_sensitivity;
  tuning_adapter_info->af_algo.af_haf.stats_select =
    (af_fv_stats_adapter_t)tuning_info->af_algo.af_haf.stats_select;
  tuning_adapter_info->af_algo.af_haf.fine_srch_drop_thres =
    tuning_info->af_algo.af_haf.fine_srch_drop_thres;
  tuning_adapter_info->af_algo.af_haf.fine_step_size =
    tuning_info->af_algo.af_haf.fine_step_size;
  tuning_adapter_info->af_algo.af_haf.max_move_step =
    tuning_info->af_algo.af_haf.max_move_step;
  tuning_adapter_info->af_algo.af_haf.max_move_step_buffer =
    tuning_info->af_algo.af_haf.max_move_step_buffer;
  tuning_adapter_info->af_algo.af_haf.base_frame_delay =
    tuning_info->af_algo.af_haf.base_frame_delay;

/******************************************************************************
** af_tuning_pdaf_adapter_t:: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   STATIC_ASSERT(sizeof(af_tuning_pd_roi_adapter_t) == sizeof(af_tuning_pd_roi_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.pdaf.roi,
    sizeof(af_tuning_pd_roi_adapter_t),
    &tuning_info->af_algo.af_haf.pdaf.roi,
    sizeof(af_tuning_pd_roi_t));

  STATIC_ASSERT(sizeof(af_tuning_pd_focus_tbl_adapter_t) == sizeof(af_tuning_pd_focus_tbl_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.pdaf.focus_tbl,
    sizeof(af_tuning_pd_focus_tbl_adapter_t),
    &tuning_info->af_algo.af_haf.pdaf.focus_tbl,
    sizeof(af_tuning_pd_focus_tbl_t));

  STATIC_ASSERT(sizeof(af_tuning_pd_conf_tbl_adapter_t) == sizeof(af_tuning_pd_conf_tbl_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.pdaf.conf_tbl,
    sizeof(af_tuning_pd_conf_tbl_adapter_t),
    &tuning_info->af_algo.af_haf.pdaf.conf_tbl,
    sizeof(af_tuning_pd_conf_tbl_t));

  STATIC_ASSERT(sizeof(af_tuning_pd_noise_tbl_adapter_t) == sizeof(af_tuning_pd_noise_tbl_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.pdaf.noise_tbl,
    sizeof(af_tuning_pd_noise_tbl_adapter_t),
    &tuning_info->af_algo.af_haf.pdaf.noise_tbl,
    sizeof(af_tuning_pd_noise_tbl_t));

  STATIC_ASSERT(sizeof(af_tuning_pd_stable_tbl_adapter_t) == sizeof(af_tuning_pd_stable_tbl_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.pdaf.stable_tbl,
    sizeof(af_tuning_pd_stable_tbl_adapter_t),
    &tuning_info->af_algo.af_haf.pdaf.stable_tbl,
    sizeof(af_tuning_pd_stable_tbl_t));

  /* af_tuning_pdaf_t: af_tuning_pd_focus_scan_t*/
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.focus_conv_frame_skip =
    tuning_info->af_algo.af_haf.pdaf.focus_scan.focus_conv_frame_skip;
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.enable_fine_scan =
    tuning_info->af_algo.af_haf.pdaf.focus_scan.enable_fine_scan;
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.min_fine_scan_range =
    tuning_info->af_algo.af_haf.pdaf.focus_scan.min_fine_scan_range;
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.fine_scan_step_size =
    tuning_info->af_algo.af_haf.pdaf.focus_scan.fine_scan_step_size;
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.focus_done_threshold =
    tuning_info->af_algo.af_haf.pdaf.focus_scan.focus_done_threshold;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.enable_fine_scan_extension =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[2];
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.first_run_not_conf_cnt_add =
    (int)tuning_info->af_algo.af_haf.pdaf.reserve[9];
#else
    tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.enable_fine_scan_extension =
      tuning_info->af_algo.af_haf.pdaf.focus_scan.enable_fine_scan_extension;
    tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.first_run_not_conf_cnt_add =
      tuning_info->af_algo.af_haf.pdaf.focus_scan.first_run_not_conf_cnt_add;
#endif

  /* af_tuning_pdaf_t: af_tuning_pd_monitor_t*/
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.wait_after_focus_cnt =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.wait_after_focus_cnt;
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.wait_conf_recover_cnt =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.wait_conf_recover_cnt;
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.defocused_threshold =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.defocused_threshold;
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.depth_stable_threshold =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.depth_stable_threshold;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.defocus_stable_filter_len =
    (int)tuning_info->af_algo.af_haf.pdaf.reserve[0];
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.enable_trigger_suppr_by_scene_change =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[1];
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.defocus_dof_multiplier =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[8];
#else
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.defocus_stable_filter_len =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.defocus_stable_filter_len;
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.enable_trigger_suppr_by_scene_change =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.enable_trigger_suppr_by_scene_change;
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.defocus_dof_multiplier =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.defocus_dof_multiplier;
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.inc_defocus_thres_scaler =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.inc_defocus_thres_scaler;
#endif


/* af_tuning_pdaf_t: af_tuning_pd_pdlib_adapter_t,*/
/* af_tuning_pd_preproc_adapter_t, af_tuning_pd_internal_adapter_t*/
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.enable =
    tuning_info->af_algo.af_haf.pdaf.pd_lib.enable;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.max_shift =
    tuning_info->af_algo.af_haf.pdaf.pd_lib.max_shift;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.resize_ratio =
    tuning_info->af_algo.af_haf.pdaf.pd_lib.resize_ratio;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.confidence_scale =
    tuning_info->af_algo.af_haf.pdaf.pd_lib.confidence_scale;
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.smooth_filter,
             tuning_info->af_algo.af_haf.pdaf.pd_lib.smooth_filter);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.sobel_filter,
             tuning_info->af_algo.af_haf.pdaf.pd_lib.sobel_filter);
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.defocus_confidence_th =
    tuning_info->af_algo.af_haf.pdaf.pd_lib.defocus_confidence_th;


  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.enable =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.enable;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.enable_do_not_use_defocus_sample_by_pd =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.enable_do_not_use_defocus_sample_by_pd;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.enable_inc_min_conf_thres_by_pd =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.enable_inc_min_conf_thres_by_pd;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_max_limit_do_not_use_defocus_sample =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_max_limit_do_not_use_defocus_sample;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_max_limit_inc_min_conf_thres =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_max_limit_inc_min_conf_thres;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.inc_min_conf_thres_by_pd_scaler =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.inc_min_conf_thres_by_pd_scaler;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_std_Threshold =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_std_Threshold;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_defocushi_err_sc_check =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_defocushi_err_sc_check;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_avgdefocushi_err_sc_check =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_avgdefocushi_err_sc_check;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_lowlight_gain_thr_sc_check =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_lowlight_gain_thr_sc_check;
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_defocushi_sc_check,
             tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_defocushi_sc_check);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_avgdefocushi_sc_check,
             tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_avgdefocushi_sc_check);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_lowlight_defocushi_sc_check,
             tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_lowlight_defocushi_sc_check);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_lowlight_avgdefocushi_sc_check,
             tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_lowlight_avgdefocushi_sc_check);

  STATIC_ASSERT(sizeof(af_tuning_pd_internal_adapter_t) == sizeof(af_tuning_pd_internal_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.pdaf.pd_int,
    sizeof(af_tuning_pd_internal_adapter_t),
    &tuning_info->af_algo.af_haf.pdaf.pd_int,
    sizeof(af_tuning_pd_internal_t));
#endif

/* af_tuning_pdaf_t: reserved parameter and af_tuning_pd_extended_info_adapter_t*/
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.enable_do_not_use_defocus_sample_by_pd =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[3];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.enable_inc_min_conf_thres_by_pd =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[4];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_max_limit_do_not_use_defocus_sample =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[5];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_max_limit_inc_min_conf_thres =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[6];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.inc_min_conf_thres_by_pd_scaler =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[7];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_int.inc_defocus_thres_prev_error =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[10];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_rgn_thres_multiplier =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[11];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_int.fine_scan_range_enhance =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[12];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.enable_soft_conf_thresh =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[13];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_conf_thresh_delta =
    (int)tuning_info->af_algo.af_haf.pdaf.reserve[14];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_conf_focus_pnt =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[15];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.enable_soft_rgn_thres_high =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[16];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_rgn_thres_high_multiplier =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[17];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_scene_change_cnt =
    af_pdaf_get_soft_scene_change_cnt((float)tuning_info->af_algo.af_haf.pdaf.reserve[18]);
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.is_2PD_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[30];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_min_move_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[31];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_move_converge_suppress_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[32];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_stable_counter_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[33];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_move_thresh_x =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[34];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_move_thresh_y =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[35];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dpd_fallback_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[36];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.touch_pd_off_center_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[37];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conv_done_temporal_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[38];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conv_done_temporal_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[39];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conv_done_temporal_filter_len =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[40];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_temporal_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[41];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_temporal_len =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[42];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_temporal_conf_thres_high =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[43];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_fallback_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[45];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_fallback_roi_size =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[46];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.fast_enter_lsp_in_low_conf_and_hunting_enable=
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[47];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.fast_enter_lsp_in_low_conf_and_hunting_threshold=
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[48];

  /* reserve parameters 40 - 50 for conf_table_high */
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.num_entries =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[50];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[0].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[51];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[0].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[52];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[1].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[53];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[1].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[54];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[2].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[55];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[2].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[56];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[3].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[57];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[3].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[58];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[4].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[59];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[4].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[60];

#else
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_rgn_thres_multiplier =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[11];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.enable_soft_conf_thresh =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[13];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_conf_thresh_delta =
    (int)tuning_info->af_algo.af_haf.pdaf.reserve[14];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_conf_focus_pnt =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[15];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.enable_soft_rgn_thres_high =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[16];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_rgn_thres_high_multiplier =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[17];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_scene_change_cnt =
    af_pdaf_get_soft_scene_change_cnt((float)tuning_info->af_algo.af_haf.pdaf.reserve[18]);
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.PD_early_interrupt_enabled =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[21];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.sprs_pd_tf_conf_thr =
    (int)tuning_info->af_algo.af_haf.pdaf.reserve[22];
    tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.sprs_pd_tf_low_defocus_conf_thr =
    (int)tuning_info->af_algo.af_haf.pdaf.reserve[23];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.skip_search_if_scene_in_focus =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[24];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.is_2PD_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[30];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_min_move_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[31];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_move_converge_suppress_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[32];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_stable_counter_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[33];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_move_thresh_x =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[34];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_move_thresh_y =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[35];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dpd_fallback_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[36];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.touch_pd_off_center_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[37];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conv_done_temporal_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[38];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conv_done_temporal_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[39];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conv_done_temporal_filter_len =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[40];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_temporal_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[41];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_temporal_len =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[42];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_temporal_conf_thres_high =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[43];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_fallback_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[45];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_fallback_roi_size =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[46];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.fast_enter_lsp_in_low_conf_and_hunting_enable=
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[47];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.fast_enter_lsp_in_low_conf_and_hunting_threshold=
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[48];

  /* reserve parameters 40 - 50 for conf_table_high */
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.num_entries =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[50];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[0].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[51];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[0].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[52];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[1].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[53];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[1].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[54];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[2].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[55];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[2].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[56];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[3].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[57];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[3].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[58];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[4].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[59];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[4].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[60];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_change_refocus_trigger_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[61];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_roi_change_thresh_percentage =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[62];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_roi_size_stable_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[63];

  // reserve[64] - reserve[75] are empty, can be used

  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.led_pdaf_max_frame_allow_enable =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[76];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.led_pdaf_max_frame_cnt =
    (int)tuning_info->af_algo.af_haf.pdaf.reserve[77];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.mw_active_zoom_threshold =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[78];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.mw_active_zoom_control_enable =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[79];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.max_caf_stable_wait_cnt =
    (unsigned int)tuning_info->af_algo.af_haf.pdaf.reserve[80];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.max_caf_stable_wait_cnt_enable =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[81];

   /* reserve parameters 83 - 97 for dof_table */
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.num_entries =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[83];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[0].norm_lens_pos =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[84];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[0].dof =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[85];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[1].norm_lens_pos =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[86];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[1].dof =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[87];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[2].norm_lens_pos =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[88];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[2].dof =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[89];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[3].norm_lens_pos =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[90];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[3].dof =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[91];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[4].norm_lens_pos =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[92];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[4].dof =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[93];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[5].norm_lens_pos =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[94];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[5].dof =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[95];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[6].norm_lens_pos =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[96];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dof_table.entries[6].dof =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[97];


  AF_HIGH("PD_early_interrupt_enabled %d",
      (uint32)tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.PD_early_interrupt_enabled);
#endif


/******************************************************************************
** af_tuning_tof_adapter_t:: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   tuning_adapter_info->af_algo.af_haf.tof.outdoor_lux_idx =
    tuning_info->af_algo.af_haf.tof.outdoor_lux_idx;
  tuning_adapter_info->af_algo.af_haf.tof.jump_to_start_limit =
    tuning_info->af_algo.af_haf.tof.jump_to_start_limit;
  tuning_adapter_info->af_algo.af_haf.tof.tof_step_size =
    tuning_info->af_algo.af_haf.tof.tof_step_size;
  tuning_adapter_info->af_algo.af_haf.tof.normal_light_cnt =
    tuning_info->af_algo.af_haf.tof.normal_light_cnt;
  tuning_adapter_info->af_algo.af_haf.tof.lowlight_cnt =
    tuning_info->af_algo.af_haf.tof.lowlight_cnt;
  tuning_adapter_info->af_algo.af_haf.tof.num_monitor_samples =
    tuning_info->af_algo.af_haf.tof.num_monitor_samples;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  tuning_adapter_info->af_algo.af_haf.tof.tof_extended.resv_enabled =
    tuning_info->af_algo.af_haf.tof.reserve[0];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.num_near_steps_far_converge_point =
    (int)tuning_info->af_algo.af_haf.tof.reserve[1];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.num_far_steps_far_converge_point =
    (int)tuning_info->af_algo.af_haf.tof.reserve[2];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.panning_stable_cnt =
    (int)tuning_info->af_algo.af_haf.tof.reserve[3];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.panning_stable_thres =
    (int)tuning_info->af_algo.af_haf.tof.reserve[4];
  tuning_adapter_info->af_algo.af_haf.tof.tof_flag.enable_sad_when_tof_is_conf =
    (int)tuning_info->af_algo.af_haf.tof.reserve[5];
  tuning_adapter_info->af_algo.af_haf.tof.tof_extended.frames_to_wait_stable =
    tuning_info->af_algo.af_haf.tof.reserve[6];
  tuning_adapter_info->af_algo.af_haf.af_gyro.num_of_samples =
    (unsigned short)tuning_info->af_algo.af_haf.tof.reserve[7];
  tuning_adapter_info->af_algo.af_haf.tof.sensitivity =
    tuning_info->af_algo.af_haf.tof.reserve[10];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres[AF_DISTANCE_ENTRY_ZERO] =
    (int)tuning_info->af_algo.af_haf.tof.reserve[11];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres[AF_DISTANCE_ENTRY_ONE] =
    (int)tuning_info->af_algo.af_haf.tof.reserve[12];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres[AF_DISTANCE_ENTRY_TWO] =
    (int)tuning_info->af_algo.af_haf.tof.reserve[13];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres[AF_DISTANCE_ENTRY_THREE] =
   (int)tuning_info->af_algo.af_haf.tof.reserve[14];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres[AF_DISTANCE_ENTRY_FOUR] =
    (int)tuning_info->af_algo.af_haf.tof.reserve[15];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_std_dev_thres =
    (int)tuning_info->af_algo.af_haf.tof.reserve[16];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.enable =
    (unsigned char)tuning_info->af_algo.af_haf.tof.reserve[17];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.frames_to_wait =
    (unsigned short)tuning_info->af_algo.af_haf.tof.reserve[18];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.gain_max =
    (int)tuning_info->af_algo.af_haf.tof.reserve[19];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.gain_min =
    (int)tuning_info->af_algo.af_haf.tof.reserve[20];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.ref_gain_max =
    (int)tuning_info->af_algo.af_haf.tof.reserve[21];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.ref_gain_min =
    (int)tuning_info->af_algo.af_haf.tof.reserve[22];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.ref_threshold_max =
    (int)tuning_info->af_algo.af_haf.tof.reserve[23];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.ref_threshold_min =
    (int)tuning_info->af_algo.af_haf.tof.reserve[24];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.threshold_max =
    (int)tuning_info->af_algo.af_haf.tof.reserve[25];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.threshold_min =
    (int)tuning_info->af_algo.af_haf.tof.reserve[26];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.distance_region[AF_DISTANCE_ENTRY_ZERO] =
    (int)tuning_info->af_algo.af_haf.tof.reserve[27];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.distance_region[AF_DISTANCE_ENTRY_ONE] =
    (int)tuning_info->af_algo.af_haf.tof.reserve[28];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.distance_region[AF_DISTANCE_ENTRY_TWO] =
    (int)tuning_info->af_algo.af_haf.tof.reserve[29];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.distance_region[AF_DISTANCE_ENTRY_THREE] =
    (int)tuning_info->af_algo.af_haf.tof.reserve[30];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.distance_region[AF_DISTANCE_ENTRY_FOUR] =
    (int)tuning_info->af_algo.af_haf.tof.reserve[31];
  tuning_adapter_info->af_algo.af_haf.tof.focus_converge.far_converge_point =
    (int)tuning_info->af_algo.af_haf.tof.reserve[32];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.far_distance_fine_step_size =
    (int)tuning_info->af_algo.af_haf.tof.reserve[33];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.far_distance_unstable_cnt_factor =
    (int)tuning_info->af_algo.af_haf.tof.reserve[34];
  tuning_adapter_info->af_algo.af_haf.tof.focus_converge.jump_to_start_limit_low_light =
    (int)tuning_info->af_algo.af_haf.tof.reserve[35];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.tof_step_size_low_light =
    (int)tuning_info->af_algo.af_haf.tof.reserve[36];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.num_near_steps_near_direction =
    (int)tuning_info->af_algo.af_haf.tof.reserve[37];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.num_far_steps_near_direction =
    (int)tuning_info->af_algo.af_haf.tof.reserve[38];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.num_near_steps_far_direction =
    (int)tuning_info->af_algo.af_haf.tof.reserve[39];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.num_far_steps_far_direction =
    (int)tuning_info->af_algo.af_haf.tof.reserve[40];
  tuning_adapter_info->af_algo.af_haf.tof.focus_converge.start_pos_for_tof =
    (int)tuning_info->af_algo.af_haf.tof.reserve[41];
  tuning_adapter_info->af_algo.af_haf.tof.tof_extended.tof_delay_flag_enable =
    (int)tuning_info->af_algo.af_haf.tof.reserve[42];
  tuning_adapter_info->af_algo.af_haf.tof.frame_delay.frame_delay_focus_converge_bright_light =
    (int)tuning_info->af_algo.af_haf.tof.reserve[43];
  tuning_adapter_info->af_algo.af_haf.tof.frame_delay.frame_delay_focus_converge_low_light =
    (int)tuning_info->af_algo.af_haf.tof.reserve[44];
  tuning_adapter_info->af_algo.af_haf.tof.frame_delay.frame_delay_fine_search_bright_light =
    (int)tuning_info->af_algo.af_haf.tof.reserve[45];
  tuning_adapter_info->af_algo.af_haf.tof.frame_delay.frame_delay_fine_search_low_light =
    (int)tuning_info->af_algo.af_haf.tof.reserve[46];
  tuning_adapter_info->af_algo.af_haf.tof.frame_delay.frame_delay_outdoor_light =
    (int)tuning_info->af_algo.af_haf.tof.reserve[47];
  tuning_adapter_info->af_algo.af_haf.tof.tof_flag.aec_wait_check_flag_enable =
    (int)tuning_info->af_algo.af_haf.tof.reserve[48];
  tuning_adapter_info->af_algo.af_haf.tof.tof_flag.use_custom_sensitivity =
    (int)tuning_info->af_algo.af_haf.tof.reserve[49];
  tuning_adapter_info->af_algo.af_haf.tof.tof_filter.median_filter_enable =
    (int)tuning_info->af_algo.af_haf.tof.reserve[50];
  tuning_adapter_info->af_algo.af_haf.tof.tof_filter.median_filter_num_of_samples =
    (int)tuning_info->af_algo.af_haf.tof.reserve[51];
  tuning_adapter_info->af_algo.af_haf.tof.tof_filter.average_filter_enable =
    (int)tuning_info->af_algo.af_haf.tof.reserve[52];
  tuning_adapter_info->af_algo.af_haf.tof.tof_filter.average_filter_num_of_samples =
    (int)tuning_info->af_algo.af_haf.tof.reserve[53];
  tuning_adapter_info->af_algo.af_haf.tof.tof_flag.run_tof_in_outdoor_light_flag =
    (int)tuning_info->af_algo.af_haf.tof.reserve[54];
  tuning_adapter_info->af_algo.af_haf.tof.tof_flag.far_scene_coarse_srch_enable =
    (int)tuning_info->af_algo.af_haf.tof.far_scene_coarse_srch_enable;
  tuning_adapter_info->af_algo.af_haf.tof.focus_converge.jump_to_start_limit =
    (int)tuning_info->af_algo.af_haf.tof.jump_to_start_limit;
#else
  tuning_adapter_info->af_algo.af_haf.tof.sensitivity =
    tuning_info->af_algo.af_haf.tof.sensitivity;
  tuning_adapter_info->af_algo.af_haf.tof.jump_to_start_limit_low_light =
    tuning_info->af_algo.af_haf.tof.jump_to_start_limit_low_light;
  tuning_adapter_info->af_algo.af_haf.tof.start_pos_for_tof =
    tuning_info->af_algo.af_haf.tof.start_pos_for_tof;
  tuning_adapter_info->af_algo.af_haf.tof.far_converge_point =
    tuning_info->af_algo.af_haf.tof.far_converge_point;
  STATIC_ASSERT(sizeof(af_tof_filter_adapter_t) == sizeof(af_tof_filter_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.tof.tof_filter,
    sizeof(af_tof_filter_adapter_t),
    &tuning_info->af_algo.af_haf.tof.tof_filter,
    sizeof(af_tof_filter_t));
  STATIC_ASSERT(sizeof(af_tof_flag_adapter_t) == sizeof(af_tof_flag_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.tof.tof_flag,
    sizeof(af_tof_flag_adapter_t),
    &tuning_info->af_algo.af_haf.tof.tof_flag,
    sizeof(af_tof_flag_t));
  STATIC_ASSERT(sizeof(af_tof_frame_delay_adapter_t) == sizeof(af_tof_frame_delay_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.tof.frame_delay,
    sizeof(af_tof_frame_delay_adapter_t),
    &tuning_info->af_algo.af_haf.tof.frame_delay,
    sizeof(af_tof_frame_delay_t));

  af_copy_tuning_sad(&tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad,
                     &tuning_info->af_algo.af_haf.tof.scene_change.af_sad);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres,
             tuning_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres);
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_std_dev_thres =
    tuning_info->af_algo.af_haf.tof.scene_change.scene_change_distance_std_dev_thres;
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.tof.scene_change.distance_region,
             tuning_info->af_algo.af_haf.tof.scene_change.distance_region);
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.far_distance_unstable_cnt_factor =
    tuning_info->af_algo.af_haf.tof.scene_change.far_distance_unstable_cnt_factor;
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.panning_stable_cnt =
    tuning_info->af_algo.af_haf.tof.scene_change.panning_stable_cnt;
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.panning_stable_thres =
    tuning_info->af_algo.af_haf.tof.scene_change.panning_stable_thres;
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.normal_light_cnt =
    tuning_info->af_algo.af_haf.tof.scene_change.normal_light_cnt;
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.lowlight_cnt =
    tuning_info->af_algo.af_haf.tof.scene_change.lowlight_cnt;
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.num_monitor_samples =
    tuning_info->af_algo.af_haf.tof.scene_change.num_monitor_samples;

  STATIC_ASSERT(sizeof(af_tof_focus_converge_adapter_t) == sizeof(af_tof_focus_converge_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.tof.focus_converge,
    sizeof(af_tof_focus_converge_adapter_t),
    &tuning_info->af_algo.af_haf.tof.focus_converge,
    sizeof(af_tof_focus_converge_t));
  STATIC_ASSERT(sizeof(af_tof_fine_scan_adapter_t) == sizeof(af_tof_fine_scan_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.tof.fine_scan,
    sizeof(af_tof_fine_scan_adapter_t),
    &tuning_info->af_algo.af_haf.tof.fine_scan,
    sizeof(af_tof_fine_scan_t));
  tuning_adapter_info->af_algo.af_haf.tof.tof_extended.resv_enabled =
    tuning_info->af_algo.af_haf.tof.reserve[0];
  if (1 == tuning_adapter_info->af_algo.af_haf.tof.tof_extended.resv_enabled) {
    tuning_adapter_info->af_algo.af_haf.tof.tof_extended.tof_min_distance_range =
      tuning_info->af_algo.af_haf.tof.reserve[1];
  }
#endif

/******************************************************************************
** af_tuning_dciaf_adapter_t:: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   tuning_adapter_info->af_algo.af_haf.dciaf.monitor_enable =
    tuning_info->af_algo.af_haf.dciaf.monitor_enable;
  tuning_adapter_info->af_algo.af_haf.dciaf.monitor_freq =
    tuning_info->af_algo.af_haf.dciaf.monitor_freq;
  tuning_adapter_info->af_algo.af_haf.dciaf.search_freq =
    tuning_info->af_algo.af_haf.dciaf.search_freq;
  tuning_adapter_info->af_algo.af_haf.dciaf.baseline_mm =
    tuning_info->af_algo.af_haf.dciaf.baseline_mm;
  tuning_adapter_info->af_algo.af_haf.dciaf.aux_direction =
    (af_cam_orientation_adapter_t)tuning_info->af_algo.af_haf.dciaf.aux_direction;
  tuning_adapter_info->af_algo.af_haf.dciaf.macro_est_limit_cm =
    tuning_info->af_algo.af_haf.dciaf.macro_est_limit_cm;
  tuning_adapter_info->af_algo.af_haf.dciaf.alignment_check_enable =
    tuning_info->af_algo.af_haf.dciaf.alignment_check_enable;
  tuning_adapter_info->af_algo.af_haf.dciaf.jump_to_start_limit =
    tuning_info->af_algo.af_haf.dciaf.jump_to_start_limit;
  tuning_adapter_info->af_algo.af_haf.dciaf.num_near_steps =
    tuning_info->af_algo.af_haf.dciaf.num_near_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.num_far_steps =
    tuning_info->af_algo.af_haf.dciaf.num_far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_step_size =
    tuning_info->af_algo.af_haf.dciaf.dciaf_step_size;
  tuning_adapter_info->af_algo.af_haf.dciaf.motion_data_use_enable =
    tuning_info->af_algo.af_haf.dciaf.motion_data_use_enable;
  tuning_adapter_info->af_algo.af_haf.dciaf.scene_change_lens_pos_th =
    tuning_info->af_algo.af_haf.dciaf.scene_change_lens_pos_th;
  tuning_adapter_info->af_algo.af_haf.dciaf.panning_stable_lens_pos_th =
    tuning_info->af_algo.af_haf.dciaf.panning_stable_lens_pos_th;
  tuning_adapter_info->af_algo.af_haf.dciaf.panning_stable_duration_ms_bright =
    tuning_info->af_algo.af_haf.dciaf.panning_stable_duration_ms_bright;
  tuning_adapter_info->af_algo.af_haf.dciaf.panning_stable_duration_ms_low =
    tuning_info->af_algo.af_haf.dciaf.panning_stable_duration_ms_low;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x0309)
  tuning_adapter_info->af_algo.af_haf.dciaf.maxLowLightWaitMs =
    tuning_info->af_algo.af_haf.dciaf.maxLowLightWaitMs;
  tuning_adapter_info->af_algo.af_haf.dciaf.maxNormalLightWaitMs =
    tuning_info->af_algo.af_haf.dciaf.maxNormalLightWaitMs;
  tuning_adapter_info->af_algo.af_haf.dciaf.jumpWithNoSkip =
    tuning_info->af_algo.af_haf.dciaf.jumpWithNoSkip;
  tuning_adapter_info->af_algo.af_haf.dciaf.SkipframesBeforeFineSearch =
    tuning_info->af_algo.af_haf.dciaf.SkipframesBeforeFineSearch;
  tuning_adapter_info->af_algo.af_haf.dciaf.dist_thresh_macro_region_in_mm =
    tuning_info->af_algo.af_haf.dciaf.dist_thresh_macro_region_in_mm;
  tuning_adapter_info->af_algo.af_haf.dciaf.dist_thresh_non_macro_factor =
    tuning_info->af_algo.af_haf.dciaf.dist_thresh_non_macro_factor;
  tuning_adapter_info->af_algo.af_haf.dciaf.lowLightThreshold =
    tuning_info->af_algo.af_haf.dciaf.lowLightThreshold;
  tuning_adapter_info->af_algo.af_haf.dciaf.useFineSearchExtension =
    tuning_info->af_algo.af_haf.dciaf.useFineSearchExtension;
  tuning_adapter_info->af_algo.af_haf.dciaf.macroRegionThreshold_in_mm =
    tuning_info->af_algo.af_haf.dciaf.macroRegionThreshold_in_mm;
  tuning_adapter_info->af_algo.af_haf.dciaf.farRegionThresholdin_mm =
    tuning_info->af_algo.af_haf.dciaf.farRegionThresholdin_mm;

  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo,
             tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo);

  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo,
             tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo);
#endif

/* reserve/dciaf_extended : af_tuning_dciaf_adapter_t:: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t*/
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.enable =
    (boolean)tuning_info->af_algo.af_haf.dciaf.reserve[0];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.maxLowLightWaitMs =
    tuning_info->af_algo.af_haf.dciaf.reserve[1];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.maxNormalLightWaitMs =
    tuning_info->af_algo.af_haf.dciaf.reserve[2];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.jumpWithNoSkip =
    tuning_info->af_algo.af_haf.dciaf.reserve[3];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.SkipframesBeforeFineSearch =
    tuning_info->af_algo.af_haf.dciaf.reserve[4];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.dist_thresh_macro_region_in_mm =
    tuning_info->af_algo.af_haf.dciaf.reserve[5];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.dist_thresh_non_macro_factor =
    tuning_info->af_algo.af_haf.dciaf.reserve[6];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightThreshold =
    tuning_info->af_algo.af_haf.dciaf.reserve[7];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.useFineSearchExtension =
    (boolean)tuning_info->af_algo.af_haf.dciaf.reserve[8];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.macroRegionThreshold_in_mm =
    tuning_info->af_algo.af_haf.dciaf.reserve[9];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.farRegionThresholdin_mm =
    tuning_info->af_algo.af_haf.dciaf.reserve[10];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].stepSize =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[11];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].far_steps =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[12];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].near_steps =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[13];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].stepSize =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[14];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].far_steps =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[15];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].near_steps =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[16];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].stepSize =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[17];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].far_steps =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[18];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].near_steps =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[19];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].stepSize =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[20];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].far_steps =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[21];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].near_steps =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[22];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].stepSize =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[23];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].far_steps =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[24];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].near_steps =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[25];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].stepSize =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[26];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].far_steps =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[27];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].near_steps =
    (int)tuning_info->af_algo.af_haf.dciaf.reserve[28];
#else
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.enable =
    tuning_info->af_algo.af_haf.dciaf.reserve[0];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.maxLowLightWaitMs =
    tuning_info->af_algo.af_haf.dciaf.maxLowLightWaitMs;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.maxNormalLightWaitMs =
    tuning_info->af_algo.af_haf.dciaf.maxNormalLightWaitMs;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.jumpWithNoSkip =
    tuning_info->af_algo.af_haf.dciaf.jumpWithNoSkip;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.SkipframesBeforeFineSearch =
    tuning_info->af_algo.af_haf.dciaf.SkipframesBeforeFineSearch;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.dist_thresh_macro_region_in_mm =
    tuning_info->af_algo.af_haf.dciaf.dist_thresh_macro_region_in_mm;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.dist_thresh_non_macro_factor =
    tuning_info->af_algo.af_haf.dciaf.dist_thresh_non_macro_factor;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightThreshold =
    tuning_info->af_algo.af_haf.dciaf.lowLightThreshold;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.useFineSearchExtension =
    tuning_info->af_algo.af_haf.dciaf.useFineSearchExtension;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.macroRegionThreshold_in_mm =
    tuning_info->af_algo.af_haf.dciaf.macroRegionThreshold_in_mm;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.farRegionThresholdin_mm =
    tuning_info->af_algo.af_haf.dciaf.farRegionThresholdin_mm;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_MACRO_RANGE].stepSize;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_MACRO_RANGE].far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_MACRO_RANGE].near_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_MID_RANGE].stepSize;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_MID_RANGE].far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_MID_RANGE].near_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_FAR_RANGE].stepSize;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_FAR_RANGE].far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_FAR_RANGE].near_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_MACRO_RANGE].stepSize;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_MACRO_RANGE].far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_MACRO_RANGE].near_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_MID_RANGE].stepSize;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_MID_RANGE].far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_MID_RANGE].near_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_FAR_RANGE].stepSize;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_FAR_RANGE].far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_FAR_RANGE].near_steps;
#endif

/******************************************************************************
** af_fine_srch_extension_t: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
** af_tuning_sad_t: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
** af_tuning_gyro_t: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x0309)
  STATIC_ASSERT(sizeof(af_fine_srch_extension_adapter_t) == sizeof(af_fine_srch_extension_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.fine_srch_extension,
    sizeof(af_fine_srch_extension_adapter_t),
    &tuning_info->af_algo.af_haf.fine_srch_extension,
    sizeof(af_fine_srch_extension_t));

  af_copy_tuning_sad(&tuning_adapter_info->af_algo.af_haf.af_sad,
                     &tuning_info->af_algo.af_haf.af_sad);

  STATIC_ASSERT(sizeof(af_tuning_gyro_adapter_t) == sizeof(af_tuning_gyro_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.af_gyro,
    sizeof(af_tuning_gyro_adapter_t),
    &tuning_info->af_algo.af_haf.af_gyro,
    sizeof(af_tuning_gyro_t));
  tuning_adapter_info->af_algo.af_haf.scene_change_during_FS =
    tuning_info->af_algo.af_haf.scene_change_during_FS;
  tuning_adapter_info->af_algo.af_haf.face_caf_high_priority_enable =
    tuning_info->af_algo.af_haf.face_caf_high_priority_enable;
#endif

/******************************************************************************
** reserve: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  tuning_adapter_info->af_algo.af_haf.haf_extended.max_fine_srch_extension_cnt =
    tuning_info->af_algo.af_haf.reserve[0];
  tuning_adapter_info->af_algo.af_haf.haf_extended.num_near_steps =
    tuning_info->af_algo.af_haf.reserve[1];
  tuning_adapter_info->af_algo.af_haf.haf_extended.num_far_steps =
    tuning_info->af_algo.af_haf.reserve[2];
  tuning_adapter_info->af_algo.af_haf.haf_extended.step_size =
    tuning_info->af_algo.af_haf.reserve[3];
  tuning_adapter_info->af_algo.af_haf.haf_extended.decrease_drop_ratio =
    tuning_info->af_algo.af_haf.reserve[4];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_init_pos =
    (boolean)tuning_info->af_algo.af_haf.reserve[5];
  tuning_adapter_info->af_algo.af_haf.haf_extended.idx =
    tuning_info->af_algo.af_haf.reserve[6];
  tuning_adapter_info->af_algo.af_haf.haf_extended.depth_conf_cnt_th =
    tuning_info->af_algo.af_haf.reserve[7];
  tuning_adapter_info->af_algo.af_haf.haf_extended.spotlight_enabled =
    (boolean)tuning_info->af_algo.af_haf.reserve[10];
  tuning_adapter_info->af_algo.af_haf.haf_extended.AF_SPOT_LIGHT_INF_FLAT_THRS =
    tuning_info->af_algo.af_haf.reserve[11];
  tuning_adapter_info->af_algo.af_haf.haf_extended.AF_SPOT_LIGHT_MARCO_FLAT_THRS =
    tuning_info->af_algo.af_haf.reserve[12];
  tuning_adapter_info->af_algo.af_haf.haf_extended.AF_SPOT_LIGHT_INC_FLAT_THRS =
    tuning_info->af_algo.af_haf.reserve[13];
  tuning_adapter_info->af_algo.af_haf.haf_extended.AF_SPOT_LIGHT_DEC_FLAT_THRS =
    tuning_info->af_algo.af_haf.reserve[14];
  tuning_adapter_info->af_algo.af_haf.haf_extended.exp_index_th =
    tuning_info->af_algo.af_haf.reserve[15];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_bright_region_zone_start_h =
    tuning_info->af_algo.af_haf.reserve[16];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_bright_region_zone_end_h =
    tuning_info->af_algo.af_haf.reserve[17];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_bright_region_zone_start_v =
    tuning_info->af_algo.af_haf.reserve[18];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_bright_region_zone_end_v =
    tuning_info->af_algo.af_haf.reserve[19];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_background_zone_start_h =
    tuning_info->af_algo.af_haf.reserve[20];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_background_zone_end_h =
    tuning_info->af_algo.af_haf.reserve[21];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_background_zone_start_v =
    tuning_info->af_algo.af_haf.reserve[22];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_background_zone_end_v =
    tuning_info->af_algo.af_haf.reserve[23];
  tuning_adapter_info->af_algo.af_haf.haf_extended.bright_region_luma_delta_background =
    tuning_info->af_algo.af_haf.reserve[24];
  tuning_adapter_info->af_algo.af_haf.haf_extended.bright_region_luma_delta_spotlight_zone_low_grey =
    tuning_info->af_algo.af_haf.reserve[25];
  tuning_adapter_info->af_algo.af_haf.haf_extended.bright_region_luma_delta_spotlight_zone_dark_grey =
    tuning_info->af_algo.af_haf.reserve[26];
  tuning_adapter_info->af_algo.af_haf.haf_extended.region_low_grey_th =
    tuning_info->af_algo.af_haf.reserve[27];
  tuning_adapter_info->af_algo.af_haf.haf_extended.region_dark_grey_th =
    tuning_info->af_algo.af_haf.reserve[28];
  tuning_adapter_info->af_algo.af_haf.haf_extended.background_low_grey_pct_th =
    tuning_info->af_algo.af_haf.reserve[29];
  tuning_adapter_info->af_algo.af_haf.haf_extended.background_dark_grey_pct_th =
    tuning_info->af_algo.af_haf.reserve[30];
  tuning_adapter_info->af_algo.af_haf.haf_extended.background_luma_ave_th =
    tuning_info->af_algo.af_haf.reserve[31];
  tuning_adapter_info->af_algo.af_haf.haf_extended.square_cluster_increase_size =
    tuning_info->af_algo.af_haf.reserve[32];
  tuning_adapter_info->af_algo.af_haf.haf_extended.square_cluster_pct_th =
    tuning_info->af_algo.af_haf.reserve[33];
  tuning_adapter_info->af_algo.af_haf.haf_extended.vertical_cluster_increase_size =
    tuning_info->af_algo.af_haf.reserve[34];
  tuning_adapter_info->af_algo.af_haf.haf_extended.vertical_cluster_pct_th =
    tuning_info->af_algo.af_haf.reserve[35];
  tuning_adapter_info->af_algo.af_haf.haf_extended.horizontal_cluster_increase_size =
    tuning_info->af_algo.af_haf.reserve[36];
  tuning_adapter_info->af_algo.af_haf.haf_extended.horizontal_cluster_pct_th =
    tuning_info->af_algo.af_haf.reserve[37];
  tuning_adapter_info->af_algo.af_haf.haf_extended.spotlight_detect_cnt_th =
    tuning_info->af_algo.af_haf.reserve[38];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_enable =
    (int)tuning_info->af_algo.af_haf.reserve[40];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_h_num =
    tuning_info->af_algo.af_haf.reserve[41];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_v_num =
    tuning_info->af_algo.af_haf.reserve[42];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_center_roi_size =
    tuning_info->af_algo.af_haf.reserve[43];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_corner_roi_size =
    tuning_info->af_algo.af_haf.reserve[44];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_delay =
    tuning_info->af_algo.af_haf.reserve[45];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enableROIShifting =
    (boolean)tuning_info->af_algo.af_haf.reserve[46];
  tuning_adapter_info->af_algo.af_haf.haf_extended.roi_shift_type =
    (af_roi_shift_type)(int)tuning_info->af_algo.af_haf.reserve[47];
  tuning_adapter_info->af_algo.af_haf.follower.enable =
    (boolean)tuning_info->af_algo.af_haf.reserve[48];
  tuning_adapter_info->af_algo.af_haf.follower.gravity_comp_on =
    (boolean)tuning_info->af_algo.af_haf.reserve[49];
  tuning_adapter_info->af_algo.af_haf.follower.jump_to_start_limit =
    (int)tuning_info->af_algo.af_haf.reserve[50];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightThreshold =
    (int)tuning_info->af_algo.af_haf.reserve[51];
  tuning_adapter_info->af_algo.af_haf.follower.useFineSearchExtension =
    (boolean)tuning_info->af_algo.af_haf.reserve[52];
  tuning_adapter_info->af_algo.af_haf.follower.activeConvergenceFollow =
    (boolean)tuning_info->af_algo.af_haf.reserve[53];
  tuning_adapter_info->af_algo.af_haf.follower.lensPosThresholdForActiveConvergenceFollow =
    (int)tuning_info->af_algo.af_haf.reserve[54];
  tuning_adapter_info->af_algo.af_haf.follower.macroRegionThreshold_in_mm =
    (int)tuning_info->af_algo.af_haf.reserve[55];
  tuning_adapter_info->af_algo.af_haf.follower.farRegionThresholdin_mm =
    tuning_info->af_algo.af_haf.reserve[56];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[57];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[58];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[59];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[60];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[61];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[62];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[63];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[64];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[65];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[66];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[67];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[68];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[69];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[70];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[71];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[72];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[73];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[74];
  tuning_adapter_info->af_algo.af_haf.haf_extended.fine_start_move_buffer =
    (int)tuning_info->af_algo.af_haf.reserve[75];
  tuning_adapter_info->af_algo.af_haf.haf_extended.dof_multiplier =
    tuning_info->af_algo.af_haf.reserve[76];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_force_scan_end =
    (boolean)tuning_info->af_algo.af_haf.reserve[77];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_passive_scan_timeout =
    (boolean)tuning_info->af_algo.af_haf.reserve[78];
  tuning_adapter_info->af_algo.af_haf.haf_extended.max_scan_cnt_th =
    (int)af_haf_fill_force_scan_cnt_th((int)tuning_info->af_algo.af_haf.reserve[79]);
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_fine_search_at_roleSwitch =
    (boolean)tuning_info->af_algo.af_haf.reserve[84];
  tuning_adapter_info->af_algo.af_haf.haf_extended.roleswitch_fine_search_info.stepSize =
    (int)tuning_info->af_algo.af_haf.reserve[85];
  tuning_adapter_info->af_algo.af_haf.haf_extended.roleswitch_fine_search_info.far_steps =
    (int)tuning_info->af_algo.af_haf.reserve[86];
  tuning_adapter_info->af_algo.af_haf.haf_extended.roleswitch_fine_search_info.near_steps =
    (int)tuning_info->af_algo.af_haf.reserve[87];
  tuning_adapter_info->af_algo.af_haf.follower.searchStrategy =
    (af_follower_search_strategy)(int)tuning_info->af_algo.af_haf.reserve[88];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_dynamic_anchor_points =
    (boolean)tuning_info->af_algo.af_haf.reserve[89];
  tuning_adapter_info->af_algo.af_haf.haf_extended.anchor_pos1 =
    (af_single_index_adapter_t)(int)tuning_info->af_algo.af_haf.reserve[90];
  tuning_adapter_info->af_algo.af_haf.haf_extended.anchor_pos2 =
    (af_single_index_adapter_t)(int)tuning_info->af_algo.af_haf.reserve[91];
  tuning_adapter_info->af_algo.af_haf.haf_extended.minimum_focus_index =
    (af_single_index_adapter_t)(int)tuning_info->af_algo.af_haf.reserve[92];
#else
  tuning_adapter_info->af_algo.af_haf.haf_extended.max_fine_srch_extension_cnt =
    tuning_info->af_algo.af_haf.reserve[0];
  tuning_adapter_info->af_algo.af_haf.haf_extended.num_near_steps =
    tuning_info->af_algo.af_haf.reserve[1];
  tuning_adapter_info->af_algo.af_haf.haf_extended.num_far_steps =
    tuning_info->af_algo.af_haf.reserve[2];
  tuning_adapter_info->af_algo.af_haf.haf_extended.step_size =
    tuning_info->af_algo.af_haf.reserve[3];
  tuning_adapter_info->af_algo.af_haf.haf_extended.decrease_drop_ratio =
    tuning_info->af_algo.af_haf.reserve[4];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_init_pos =
    tuning_info->af_algo.af_haf.reserve[5];
  tuning_adapter_info->af_algo.af_haf.haf_extended.idx =
    tuning_info->af_algo.af_haf.reserve[6];
  tuning_adapter_info->af_algo.af_haf.haf_extended.depth_conf_cnt_th =
    tuning_info->af_algo.af_haf.reserve[7];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.HJ_Algorithm_enable =
    tuning_info->af_algo.af_haf.reserve[8];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.maxfv_in_beginning =
    tuning_info->af_algo.af_haf.reserve[9];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.coarse_min_sample_size =
    tuning_info->af_algo.af_haf.reserve[10];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.fine_min_sample_size =
    tuning_info->af_algo.af_haf.reserve[11];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.cur_gyro_HJ_thrs =
    tuning_info->af_algo.af_haf.reserve[12];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.cur_gyro_STD_HJ_Thrs =
    tuning_info->af_algo.af_haf.reserve[13];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.gyro_thrs_for_diff_ratio =
    tuning_info->af_algo.af_haf.reserve[14];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.gyro_diff_ratio_HJ_thrs =
    tuning_info->af_algo.af_haf.reserve[15];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.cur_gyro_HJ_thrs_fine =
    tuning_info->af_algo.af_haf.reserve[16];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.cur_gyro_STD_HJ_thrs_fine =
    tuning_info->af_algo.af_haf.reserve[17];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.gyro_thrs_for_diff_ratio_fine =
    tuning_info->af_algo.af_haf.reserve[18];
   tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.gyro_diff_ratio_HJ_thrs_fine =
    tuning_info->af_algo.af_haf.reserve[19];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.HJ_ReDo_rate_thrs =
    tuning_info->af_algo.af_haf.reserve[20];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.HJ_handling_need_cnt =
    tuning_info->af_algo.af_haf.reserve[21];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.HJ_reset_Data_Before_fine =
    tuning_info->af_algo.af_haf.reserve[22];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.HJ_high_gyro_diff_thrs =
    tuning_info->af_algo.af_haf.reserve[23];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.HJ_low_gyro_diff_thrs =
    tuning_info->af_algo.af_haf.reserve[24];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.HJ_high_gyro_Fv_weight =
    tuning_info->af_algo.af_haf.reserve[25];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.HJ_normalized_Fv_enable =
    tuning_info->af_algo.af_haf.reserve[26];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.HJ_algortihm_working_exp_idx =
    tuning_info->af_algo.af_haf.reserve[27];

  // Caf
   tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_HJ_Algorithm_enable =
    tuning_info->af_algo.af_haf.reserve[28];
  tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_min_sample_size =
    tuning_info->af_algo.af_haf.reserve[29];
  tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_cur_gyro_HJ_thrs =
    tuning_info->af_algo.af_haf.reserve[30];
  tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_cur_gyro_STD_HJ_Thrs =
    tuning_info->af_algo.af_haf.reserve[31];
  tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_gyro_thrs_for_diff_ratio =
    tuning_info->af_algo.af_haf.reserve[32];
  tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_HJ_handling_need_cnt =
    tuning_info->af_algo.af_haf.reserve[33];
  tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_gyro_diff_ratio_HJ_thrs =
    tuning_info->af_algo.af_haf.reserve[34];
  tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_HJ_high_gyro_diff_thrs =
    tuning_info->af_algo.af_haf.reserve[35];
  tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_HJ_low_gyro_diff_thrs =
    tuning_info->af_algo.af_haf.reserve[36];
  tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_HJ_high_gyro_Fv_weight =
    tuning_info->af_algo.af_haf.reserve[37];
  tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_HJ_normalized_Fv_enable =
    tuning_info->af_algo.af_haf.reserve[38];
   tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_HJ_algortihm_working_exp_idx =
    tuning_info->af_algo.af_haf.reserve[39];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_enable =
    (boolean)tuning_info->af_algo.af_haf.reserve[40];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_h_num =
    tuning_info->af_algo.af_haf.reserve[41];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_v_num =
    tuning_info->af_algo.af_haf.reserve[42];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_center_roi_size =
    tuning_info->af_algo.af_haf.reserve[43];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_corner_roi_size =
    tuning_info->af_algo.af_haf.reserve[44];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_delay =
    tuning_info->af_algo.af_haf.reserve[45];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enableROIShifting =
    tuning_info->af_algo.af_haf.reserve[46];
  tuning_adapter_info->af_algo.af_haf.haf_extended.roi_shift_type =
    (af_roi_shift_type)(int)tuning_info->af_algo.af_haf.reserve[47];
  tuning_adapter_info->af_algo.af_haf.follower.enable =
    tuning_info->af_algo.af_haf.reserve[48];
  tuning_adapter_info->af_algo.af_haf.follower.gravity_comp_on =
    tuning_info->af_algo.af_haf.reserve[49];
  tuning_adapter_info->af_algo.af_haf.follower.jump_to_start_limit =
    tuning_info->af_algo.af_haf.reserve[50];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightThreshold =
    tuning_info->af_algo.af_haf.reserve[51];
  tuning_adapter_info->af_algo.af_haf.follower.useFineSearchExtension =
    tuning_info->af_algo.af_haf.reserve[52];
  tuning_adapter_info->af_algo.af_haf.follower.activeConvergenceFollow =
    tuning_info->af_algo.af_haf.reserve[53];
  tuning_adapter_info->af_algo.af_haf.follower.lensPosThresholdForActiveConvergenceFollow =
    tuning_info->af_algo.af_haf.reserve[54];
  tuning_adapter_info->af_algo.af_haf.follower.macroRegionThreshold_in_mm =
    tuning_info->af_algo.af_haf.reserve[55];
  tuning_adapter_info->af_algo.af_haf.follower.farRegionThresholdin_mm =
    tuning_info->af_algo.af_haf.reserve[56];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[57];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[58];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[59];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[60];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[61];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[62];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[63];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[64];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_DISTANCE_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[65];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[66];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[67];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[68];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[69];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[70];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[71];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[72];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[73];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_DISTANCE_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[74];
  tuning_adapter_info->af_algo.af_haf.haf_extended.fine_start_move_buffer =
   (int)tuning_info->af_algo.af_haf.reserve[75];
  tuning_adapter_info->af_algo.af_haf.haf_extended.dof_multiplier =
    tuning_info->af_algo.af_haf.reserve[76];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_force_scan_end =
    (boolean)tuning_info->af_algo.af_haf.reserve[77];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_passive_scan_timeout =
    (boolean)tuning_info->af_algo.af_haf.reserve[78];
  tuning_adapter_info->af_algo.af_haf.haf_extended.max_scan_cnt_th =
    (int)af_haf_fill_force_scan_cnt_th((int)tuning_info->af_algo.af_haf.reserve[79]);
  tuning_adapter_info->af_algo.af_haf.haf_extended.fine_search_peakdown_thres =
    (int)tuning_info->af_algo.af_haf.reserve[80];
  tuning_adapter_info->af_algo.af_haf.haf_extended.face_far_end_enabled =
    (boolean)tuning_info->af_algo.af_haf.reserve[81];
  tuning_adapter_info->af_algo.af_haf.haf_extended.face_far_end =
    (int)tuning_info->af_algo.af_haf.reserve[82];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_fine_search_at_roleSwitch =
    (boolean)tuning_info->af_algo.af_haf.reserve[84];
  tuning_adapter_info->af_algo.af_haf.haf_extended.roleswitch_fine_search_info.stepSize =
    (int)tuning_info->af_algo.af_haf.reserve[85];
  tuning_adapter_info->af_algo.af_haf.haf_extended.roleswitch_fine_search_info.far_steps =
    (int)tuning_info->af_algo.af_haf.reserve[86];
  tuning_adapter_info->af_algo.af_haf.haf_extended.roleswitch_fine_search_info.near_steps =
    (int)tuning_info->af_algo.af_haf.reserve[87];
  tuning_adapter_info->af_algo.af_haf.follower.searchStrategy =
    (af_follower_search_strategy)(int)tuning_info->af_algo.af_haf.reserve[88];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_dynamic_anchor_points =
    (boolean)tuning_info->af_algo.af_haf.reserve[89];
  tuning_adapter_info->af_algo.af_haf.haf_extended.anchor_pos1 =
    (af_single_index_adapter_t)(int)tuning_info->af_algo.af_haf.reserve[90];
  tuning_adapter_info->af_algo.af_haf.haf_extended.anchor_pos2 =
    (af_single_index_adapter_t)(int)tuning_info->af_algo.af_haf.reserve[91];
  tuning_adapter_info->af_algo.af_haf.haf_extended.minimum_focus_index =
    (af_single_index_adapter_t)(int)tuning_info->af_algo.af_haf.reserve[92];
  tuning_adapter_info->af_algo.af_haf.haf_extended.hjr_info.HJ_normalized_weight_type =
    tuning_info->af_algo.af_haf.reserve[93];
  tuning_adapter_info->af_algo.af_haf.haf_extended.caf_hjr_info.caf_HJ_normalized_weight_type =
    tuning_info->af_algo.af_haf.reserve[94];

  tuning_adapter_info->af_algo.af_haf.haf_extended.fine_search_range_ratio =
    tuning_info->af_algo.af_haf.reserve[95];

  tuning_adapter_info->af_algo.af_haf.haf_extended.spd_stats_type =
    (int)tuning_info->af_algo.af_haf.reserve[97];
  tuning_adapter_info->af_algo.af_haf.haf_extended.spd_bg_stats_thresh_auto_convert_enable =
    (boolean)tuning_info->af_algo.af_haf.reserve[98];
  tuning_adapter_info->af_algo.af_haf.haf_extended.spd_bg_stats_bright_region_sat_ratio_thresh =
    (float)tuning_info->af_algo.af_haf.reserve[99];

#endif

/******************************************************************************
** af_tuning_continuous_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   tuning_adapter_info->af_algo.af_cont.enable =
    tuning_info->af_algo.af_cont.enable;
  tuning_adapter_info->af_algo.af_cont.scene_change_detection_ratio =
    tuning_info->af_algo.af_cont.scene_change_detection_ratio;
  tuning_adapter_info->af_algo.af_cont.panning_stable_fv_change_trigger =
    tuning_info->af_algo.af_cont.panning_stable_fv_change_trigger;
  tuning_adapter_info->af_algo.af_cont.panning_stable_fvavg_to_fv_change_trigger =
    tuning_info->af_algo.af_cont.panning_stable_fvavg_to_fv_change_trigger;
  tuning_adapter_info->af_algo.af_cont.panning_unstable_trigger_cnt =
    tuning_info->af_algo.af_cont.panning_unstable_trigger_cnt;
  tuning_adapter_info->af_algo.af_cont.panning_stable_trigger_cnt =
    tuning_info->af_algo.af_cont.panning_stable_trigger_cnt;
  tuning_adapter_info->af_algo.af_cont.downhill_allowance =
    tuning_info->af_algo.af_cont.downhill_allowance;
  tuning_adapter_info->af_algo.af_cont.uphill_allowance =
    tuning_info->af_algo.af_cont.uphill_allowance;
  tuning_adapter_info->af_algo.af_cont.base_frame_delay =
    tuning_info->af_algo.af_cont.base_frame_delay;
  tuning_adapter_info->af_algo.af_cont.scene_change_luma_threshold =
    tuning_info->af_algo.af_cont.scene_change_luma_threshold;
  tuning_adapter_info->af_algo.af_cont.luma_settled_threshold =
    tuning_info->af_algo.af_cont.luma_settled_threshold;
  tuning_adapter_info->af_algo.af_cont.noise_level_th =
    tuning_info->af_algo.af_cont.noise_level_th;
  tuning_adapter_info->af_algo.af_cont.search_step_size =
    tuning_info->af_algo.af_cont.search_step_size;
  tuning_adapter_info->af_algo.af_cont.init_search_type =
    (af_algo_adapter_t)tuning_info->af_algo.af_cont.init_search_type;
  tuning_adapter_info->af_algo.af_cont.search_type =
    (af_algo_adapter_t)tuning_info->af_algo.af_cont.search_type;
  tuning_adapter_info->af_algo.af_cont.low_light_wait =
    tuning_info->af_algo.af_cont.low_light_wait;
  tuning_adapter_info->af_algo.af_cont.max_indecision_cnt =
    tuning_info->af_algo.af_cont.max_indecision_cnt;
  tuning_adapter_info->af_algo.af_cont.flat_fv_confidence_level =
    tuning_info->af_algo.af_cont.flat_fv_confidence_level;

  af_copy_tuning_sad(&tuning_adapter_info->af_algo.af_cont.af_sad,
                     &tuning_info->af_algo.af_cont.af_sad);

  tuning_adapter_info->af_algo.af_cont.af_gyro.enable =
    tuning_info->af_algo.af_cont.af_gyro.enable;
  tuning_adapter_info->af_algo.af_cont.af_gyro.min_movement_threshold =
    tuning_info->af_algo.af_cont.af_gyro.min_movement_threshold;
  tuning_adapter_info->af_algo.af_cont.af_gyro.stable_detected_threshold =
    tuning_info->af_algo.af_cont.af_gyro.stable_detected_threshold;
  tuning_adapter_info->af_algo.af_cont.af_gyro.unstable_count_th =
    tuning_info->af_algo.af_cont.af_gyro.unstable_count_th;
  tuning_adapter_info->af_algo.af_cont.af_gyro.stable_count_th =
    tuning_info->af_algo.af_cont.af_gyro.stable_count_th;
  tuning_adapter_info->af_algo.af_cont.af_gyro.fast_pan_threshold =
    tuning_info->af_algo.af_cont.af_gyro.fast_pan_threshold;
  tuning_adapter_info->af_algo.af_cont.af_gyro.slow_pan_threshold =
    tuning_info->af_algo.af_cont.af_gyro.slow_pan_threshold;
  tuning_adapter_info->af_algo.af_cont.af_gyro.fast_pan_count_threshold =
    tuning_info->af_algo.af_cont.af_gyro.fast_pan_count_threshold;
  tuning_adapter_info->af_algo.af_cont.af_gyro.sum_return_to_orig_pos_threshold =
    tuning_info->af_algo.af_cont.af_gyro.sum_return_to_orig_pos_threshold;
  tuning_adapter_info->af_algo.af_cont.af_gyro.stable_count_delay =
    tuning_info->af_algo.af_cont.af_gyro.stable_count_delay;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  tuning_adapter_info->af_algo.af_cont.af_gyro.num_of_samples =
    tuning_info->af_algo.af_cont.af_gyro.num_of_samples;
#endif

  tuning_adapter_info->af_algo.af_cont.af_chroma.enable =
    tuning_info->af_algo.af_cont.af_chroma.enable;
  tuning_adapter_info->af_algo.af_cont.af_chroma.g_over_b_thres =
    tuning_info->af_algo.af_cont.af_chroma.g_over_b_thres;
  tuning_adapter_info->af_algo.af_cont.af_chroma.g_over_r_thres =
    tuning_info->af_algo.af_cont.af_chroma.g_over_r_thres;
  COPY_ARRAY(tuning_adapter_info->af_algo.af_cont.af_chroma.aec_gain_input,
             tuning_info->af_algo.af_cont.af_chroma.aec_gain_input);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_cont.af_chroma.g_over_b_threshold_scaler,
             tuning_info->af_algo.af_cont.af_chroma.g_over_b_threshold_scaler);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_cont.af_chroma.g_over_r_threshold_scaler,
             tuning_info->af_algo.af_cont.af_chroma.g_over_r_threshold_scaler);
  tuning_adapter_info->af_algo.af_cont.af_chroma.unstable_count_th =
    tuning_info->af_algo.af_cont.af_chroma.unstable_count_th;
  tuning_adapter_info->af_algo.af_cont.af_chroma.num_of_samples =
    tuning_info->af_algo.af_cont.af_chroma.num_of_samples;

  tuning_adapter_info->af_algo.af_cont.af_spot_light.enable =
    tuning_info->af_algo.af_cont.af_spot_light.enable;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.far_flat_th =
    tuning_info->af_algo.af_cont.af_spot_light.far_flat_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.marco_flat_th =
    tuning_info->af_algo.af_cont.af_spot_light.marco_flat_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.inc_flat_th =
    tuning_info->af_algo.af_cont.af_spot_light.inc_flat_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.dec_flat_th =
    tuning_info->af_algo.af_cont.af_spot_light.dec_flat_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.spotlight_zone_start_h =
    tuning_info->af_algo.af_cont.af_spot_light.spotlight_zone_start_h;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.spotlight_zone_end_h =
    tuning_info->af_algo.af_cont.af_spot_light.spotlight_zone_end_h;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.spotlight_zone_start_v =
    tuning_info->af_algo.af_cont.af_spot_light.spotlight_zone_start_v;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.spotlight_zone_end_v =
    tuning_info->af_algo.af_cont.af_spot_light.spotlight_zone_end_v;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_zone_start_h =
    tuning_info->af_algo.af_cont.af_spot_light.background_zone_start_h;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_zone_end_h =
    tuning_info->af_algo.af_cont.af_spot_light.background_zone_end_h;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_zone_start_v =
    tuning_info->af_algo.af_cont.af_spot_light.background_zone_start_v;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_zone_end_v =
    tuning_info->af_algo.af_cont.af_spot_light.background_zone_end_v;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.exposure_index_trigger_th =
    tuning_info->af_algo.af_cont.af_spot_light.exposure_index_trigger_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_luma_avg_th =
    tuning_info->af_algo.af_cont.af_spot_light.background_luma_avg_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.region_low_grey_th =
    tuning_info->af_algo.af_cont.af_spot_light.region_low_grey_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.region_low_dark_th =
    tuning_info->af_algo.af_cont.af_spot_light.region_low_dark_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_low_grey_pct_th =
    tuning_info->af_algo.af_cont.af_spot_light.background_low_grey_pct_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_dark_grey_pct_th =
    tuning_info->af_algo.af_cont.af_spot_light.background_dark_grey_pct_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.bright_region_luma_delta_background_zone =
    tuning_info->af_algo.af_cont.af_spot_light.bright_region_luma_delta_background_zone;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.bright_region_luma_delta_spotlight_zone_low_grey =
    tuning_info->af_algo.af_cont.af_spot_light.bright_region_luma_delta_spotlight_zone_low_grey;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.bright_region_luma_delta_spotlight_zone_dark_grey =
    tuning_info->af_algo.af_cont.af_spot_light.bright_region_luma_delta_spotlight_zone_dark_grey;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.square_cluster_inc_size =
    tuning_info->af_algo.af_cont.af_spot_light.square_cluster_inc_size;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.vertical_cluster_inc_size =
    tuning_info->af_algo.af_cont.af_spot_light.vertical_cluster_inc_size;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.horizontal_cluster_inc_size =
    tuning_info->af_algo.af_cont.af_spot_light.horizontal_cluster_inc_size;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.square_cluster_pct_th =
    tuning_info->af_algo.af_cont.af_spot_light.square_cluster_pct_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.vertical_cluster_pct_th =
    tuning_info->af_algo.af_cont.af_spot_light.vertical_cluster_pct_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.horizontal_cluster_pct_th =
    tuning_info->af_algo.af_cont.af_spot_light.horizontal_cluster_pct_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.spotlight_detect_cnt_th =
    tuning_info->af_algo.af_cont.af_spot_light.spotlight_detect_cnt_th;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x0309)
  tuning_adapter_info->af_algo.af_cont.af_caf_trigger_after_taf =
    tuning_info->af_algo.af_cont.af_caf_trigger_after_taf;
#endif

/******************************************************************************
** af_tuning_continuous_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   STATIC_ASSERT(sizeof(af_tuning_exhaustive_adapter_t) == sizeof(af_tuning_exhaustive_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_exh,
    sizeof(af_tuning_exhaustive_adapter_t),
    &tuning_info->af_algo.af_exh,
    sizeof(af_tuning_exhaustive_t));

/******************************************************************************
** af_tuning_fullsweep_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   tuning_adapter_info->af_algo.af_full.num_steps_between_stat_points =
    tuning_info->af_algo.af_full.num_steps_between_stat_points;
  tuning_adapter_info->af_algo.af_full.frame_delay_inf =
    tuning_info->af_algo.af_full.frame_delay_inf;
  tuning_adapter_info->af_algo.af_full.frame_delay_norm =
    tuning_info->af_algo.af_full.frame_delay_norm;
  tuning_adapter_info->af_algo.af_full.frame_delay_final =
    tuning_info->af_algo.af_full.frame_delay_final;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x0309)
  tuning_adapter_info->af_algo.af_full.PAAF_enable =
    tuning_info->af_algo.af_full.PAAF_enable;
#endif

/******************************************************************************
** af_tuning_sp_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   STATIC_ASSERT(sizeof(af_tuning_sp_adapter_t) == sizeof(af_tuning_sp_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_sp,
    sizeof(af_tuning_sp_adapter_t),
    &tuning_info->af_algo.af_sp,
    sizeof(af_tuning_sp_t));

/******************************************************************************
** af_tuning_single_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   COPY_ARRAY(tuning_adapter_info->af_algo.af_single.index,
             tuning_info->af_algo.af_single.index);
  tuning_adapter_info->af_algo.af_single.actuator_type =
    tuning_info->af_algo.af_single.actuator_type;
  tuning_adapter_info->af_algo.af_single.is_hys_comp_needed =
    tuning_info->af_algo.af_single.is_hys_comp_needed;
  tuning_adapter_info->af_algo.af_single.step_index_per_um =
    tuning_info->af_algo.af_single.step_index_per_um;
  STATIC_ASSERT(sizeof(af_step_size_table_adapter_t) == sizeof(step_size_table_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_single.TAF_step_table,
    sizeof(af_step_size_table_adapter_t),
    &tuning_info->af_algo.af_single.TAF_step_table,
    sizeof(step_size_table_t));
  STATIC_ASSERT(sizeof(af_step_size_table_adapter_t) == sizeof(step_size_table_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_single.CAF_step_table,
    sizeof(af_step_size_table_adapter_t),
    &tuning_info->af_algo.af_single.CAF_step_table,
    sizeof(step_size_table_t));
  tuning_adapter_info->af_algo.af_single.PAAF_enable =
    tuning_info->af_algo.af_single.PAAF_enable;
  af_copy_single_threshold(&tuning_adapter_info->af_algo.af_single.sw,
                           &tuning_info->af_algo.af_single.sw);
  af_copy_single_threshold(&tuning_adapter_info->af_algo.af_single.hw,
                           &tuning_info->af_algo.af_single.hw);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_single.BV_gain,
             tuning_info->af_algo.af_single.BV_gain);
  STATIC_ASSERT(sizeof(single_optic_adapter_t) == sizeof(single_optic_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_single.optics,
    sizeof(single_optic_adapter_t),
    &tuning_info->af_algo.af_single.optics,
    sizeof(single_optic_t));
  STATIC_ASSERT(sizeof(FVscore_threshold_adapter_t) == sizeof(FVscore_threshold_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_single.score_thresh,
    sizeof(FVscore_threshold_adapter_t),
    &tuning_info->af_algo.af_single.score_thresh,
    sizeof(FVscore_threshold_t));
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x0309)
  STATIC_ASSERT(sizeof(af_single_frame_delay_adapter_t) == sizeof(af_single_frame_delay_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_single.frame_delay,
    sizeof(af_single_frame_delay_adapter_t),
    &tuning_info->af_algo.af_single.frame_delay,
    sizeof(af_single_frame_delay_t));
#endif

/******************************************************************************
** af_shake_resistant_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   STATIC_ASSERT(sizeof(af_shake_resistant_adapter_t) == sizeof(af_shake_resistant_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_shake_resistant,
    sizeof(af_shake_resistant_adapter_t),
    &tuning_info->af_algo.af_shake_resistant,
    sizeof(af_shake_resistant_t));

/******************************************************************************
** af_motion_sensor_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   STATIC_ASSERT(sizeof(af_motion_sensor_adapter_t) == sizeof(af_motion_sensor_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_motion_sensor,
    sizeof(af_motion_sensor_adapter_t),
    &tuning_info->af_algo.af_motion_sensor,
    sizeof(af_motion_sensor_t));

/******************************************************************************
** af_fd_priority_caf_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   STATIC_ASSERT(sizeof(af_fd_priority_caf_adapter_t) == sizeof(af_fd_priority_caf_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.fd_prio,
    sizeof(af_fd_priority_caf_adapter_t),
    &tuning_info->af_algo.fd_prio,
    sizeof(af_fd_priority_caf_t));

/******************************************************************************
** af_tuning_lens_sag_comp_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   STATIC_ASSERT(sizeof(af_tuning_lens_sag_comp_adapter_t) == sizeof(af_tuning_lens_sag_comp_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.lens_sag_comp,
    sizeof(af_tuning_lens_sag_comp_adapter_t),
    &tuning_info->af_algo.lens_sag_comp,
    sizeof(af_tuning_lens_sag_comp_t));

/******************************************************************************
** af_tuning_vbt_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   STATIC_ASSERT(sizeof(af_tuning_vbt_adapter_t) == sizeof(af_tuning_vbt_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.vbt,
    sizeof(af_tuning_vbt_adapter_t),
    &tuning_info->af_algo.vbt,
    sizeof(af_tuning_vbt_t));

/******************************************************************************
** af_tuning_fovc_adapter: af_tuning_algo_adapter_t
******************************************************************************/
   tuning_adapter_info->af_algo.fovc.enable =
    tuning_info->af_algo.fovc.enable;
  tuning_adapter_info->af_algo.fovc.fudge_factor =
      tuning_info->af_algo.fovc.fudge_factor;
  tuning_adapter_info->af_algo.fovc.anchor_lens_pos1 =
      tuning_info->af_algo.fovc.anchor_lens_pos1;
  tuning_adapter_info->af_algo.fovc.anchor_lens_pos2 =
      tuning_info->af_algo.fovc.anchor_lens_pos2;
  tuning_adapter_info->af_algo.fovc.anchor_distance_mm_1 =
      tuning_info->af_algo.fovc.anchor_distance_mm_1;
  tuning_adapter_info->af_algo.fovc.anchor_distance_mm_2 =
      tuning_info->af_algo.fovc.anchor_distance_mm_2;
  tuning_adapter_info->af_algo.fovc.focus_converge_step_size_bright_light =
      tuning_info->af_algo.fovc.focus_converge_step_size_bright_light;
  tuning_adapter_info->af_algo.fovc.fine_search_step_size_bright_light =
      tuning_info->af_algo.fovc.fine_search_step_size_bright_light;
  tuning_adapter_info->af_algo.fovc.focus_converge_step_size_low_light =
      tuning_info->af_algo.fovc.focus_converge_step_size_low_light;
  tuning_adapter_info->af_algo.fovc.fine_search_step_size_low_light =
      tuning_info->af_algo.fovc.fine_search_step_size_low_light;

/******************************************************************************
** af_tuning_multiwindow_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
  tuning_adapter_info->af_algo.af_multiwindow.enable =
    tuning_info->af_algo.af_multiwindow.enable;
  tuning_adapter_info->af_algo.af_multiwindow.h_clip_ratio_normal_light =
    tuning_info->af_algo.af_multiwindow.h_clip_ratio_normal_light;
  tuning_adapter_info->af_algo.af_multiwindow.v_clip_ratio_normal_light =
    tuning_info->af_algo.af_multiwindow.v_clip_ratio_normal_light;
  tuning_adapter_info->af_algo.af_multiwindow.h_clip_ratio_low_light =
    tuning_info->af_algo.af_multiwindow.h_clip_ratio_low_light;
  tuning_adapter_info->af_algo.af_multiwindow.v_clip_ratio_low_light =
    tuning_info->af_algo.af_multiwindow.v_clip_ratio_low_light;
  tuning_adapter_info->af_algo.af_multiwindow.h_num_default =
    tuning_info->af_algo.af_multiwindow.h_num_default;
  tuning_adapter_info->af_algo.af_multiwindow.v_num_default =
    tuning_info->af_algo.af_multiwindow.v_num_default;
  tuning_adapter_info->af_algo.af_multiwindow.h_num_windows =
    tuning_info->af_algo.af_multiwindow.h_num_windows;
  tuning_adapter_info->af_algo.af_multiwindow.v_num_windows =
    tuning_info->af_algo.af_multiwindow.v_num_windows;

  /* multi-window reserved parameters */
  tuning_adapter_info->af_algo.af_multiwindow.pd_percentile =
    (float)tuning_info->af_algo.af_multiwindow.reserve[0];
  tuning_adapter_info->af_algo.af_multiwindow.defocus_th =
    (int)tuning_info->af_algo.af_multiwindow.reserve[1];
  tuning_adapter_info->af_algo.af_multiwindow.mw_depth_percentile =
    (float)tuning_info->af_algo.af_multiwindow.reserve[3];
  tuning_adapter_info->af_algo.af_multiwindow.mw_confidence_th_multiple =
    (float)tuning_info->af_algo.af_multiwindow.reserve[5];
  tuning_adapter_info->af_algo.af_multiwindow.mw_low_light_gain_th_low =
    (float)tuning_info->af_algo.af_multiwindow.reserve[6];
  tuning_adapter_info->af_algo.af_multiwindow.mw_low_light_gain_th_high =
    (float)tuning_info->af_algo.af_multiwindow.reserve[7];
  tuning_adapter_info->af_algo.af_multiwindow.mw_depth_detect_th_low =
    (float)tuning_info->af_algo.af_multiwindow.reserve[8];
  tuning_adapter_info->af_algo.af_multiwindow.mw_depth_detect_th_high =
    (float)tuning_info->af_algo.af_multiwindow.reserve[9];
  tuning_adapter_info->af_algo.af_multiwindow.lock_to_peripheral_during_search =
    (boolean)tuning_info->af_algo.af_multiwindow.reserve[10];
/******************************************************************************
** af_tuning_focus_mode_search_limit_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   COPY_ARRAY(tuning_adapter_info->af_algo.search_limit,
             tuning_info->af_algo.search_limit);

/******************************************************************************
** af_tuning_algo_extended_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  for (i = 0, j = 0; i < AF_FOCUS_MODE_MAX; i++) {
    j = i * 3; /* three parameters inside each limit structure*/
    tuning_adapter_info->af_algo.algo_extended.search_limit[i].near_end =
      (unsigned int)tuning_info->af_algo.reserve[j];
    tuning_adapter_info->af_algo.algo_extended.search_limit[i].far_end =
      (unsigned int)tuning_info->af_algo.reserve[j + 1];
    tuning_adapter_info->af_algo.algo_extended.search_limit[i].default_pos =
      (unsigned int)tuning_info->af_algo.reserve[j + 2];
  }
  tuning_adapter_info->af_algo.algo_extended.stats_comp_type =
    (af_stats_use_type)(int)tuning_info->af_algo.reserve[15];
  tuning_adapter_info->af_algo.algo_extended.hv_stats_ratio_enable =
    (boolean)tuning_info->af_algo.reserve[16];
  tuning_adapter_info->af_algo.algo_extended.gravity_x_land_thres =
  tuning_info->af_algo.reserve[17];
  tuning_adapter_info->af_algo.algo_extended.gravity_y_land_thres =
  tuning_info->af_algo.reserve[18];
  tuning_adapter_info->af_algo.algo_extended.gravity_x_port_thres =
  tuning_info->af_algo.reserve[19];
  tuning_adapter_info->af_algo.algo_extended.gravity_y_port_thres =
  tuning_info->af_algo.reserve[20];
  tuning_adapter_info->af_algo.algo_extended.hv_weight =
  tuning_info->af_algo.reserve[21];
  /* robust fd reserve param */
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.robust_fd_enable =
    (unsigned char)tuning_info->af_algo.reserve[64];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.noface_cnt_thr =
    (unsigned int)tuning_info->af_algo.reserve[65];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.stable_cnt_thr =
    (unsigned int)tuning_info->af_algo.reserve[66];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.fd_roi_pos_change_th =
    (float)tuning_info->af_algo.reserve[67];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.face_lost_trigger_enable =
    (unsigned char)tuning_info->af_algo.reserve[68];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.face_lost_stable_th =
    (unsigned int)tuning_info->af_algo.reserve[69];

  tuning_adapter_info->af_algo.algo_extended.en_ignore_first_invalid_frame =
    (boolean)tuning_info->af_algo.reserve[22];
  tuning_adapter_info->af_algo.algo_extended.step_back.enable =
    (boolean)tuning_info->af_algo.reserve[23];
  tuning_adapter_info->af_algo.algo_extended.step_back.start_inf =
    (int)tuning_info->af_algo.reserve[24];
  tuning_adapter_info->af_algo.algo_extended.step_back.start_mac =
    (int)tuning_info->af_algo.reserve[25];
  tuning_adapter_info->af_algo.algo_extended.step_back.reverse_inf =
    (int)tuning_info->af_algo.reserve[26];
  tuning_adapter_info->af_algo.algo_extended.step_back.reverse_mac =
    (int)tuning_info->af_algo.reserve[27];
  tuning_adapter_info->af_algo.algo_extended.scan_range.fratio_min =
    tuning_info->af_algo.reserve[28];
  tuning_adapter_info->af_algo.algo_extended.scan_range.fratio_max =
    tuning_info->af_algo.reserve[29];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_prev_y1 =
    tuning_info->af_algo.reserve[30];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_next_y1 =
    tuning_info->af_algo.reserve[31];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_prev_y2 =
    tuning_info->af_algo.reserve[32];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_next_y2 =
    tuning_info->af_algo.reserve[33];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_prev_y1 =
    tuning_info->af_algo.reserve[34];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_next_y1 =
    tuning_info->af_algo.reserve[35];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_prev_y2 =
    tuning_info->af_algo.reserve[36];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_next_y2 =
    tuning_info->af_algo.reserve[37];
  tuning_adapter_info->af_algo.algo_extended.flat_detection.enable =
    (boolean)tuning_info->af_algo.reserve[38];
  tuning_adapter_info->af_algo.algo_extended.flat_detection.threshold_bv =
    tuning_info->af_algo.reserve[39];
  tuning_adapter_info->af_algo.algo_extended.flat_threshold.increase =
    tuning_info->af_algo.reserve[40];
  tuning_adapter_info->af_algo.algo_extended.flat_threshold.decrease =
    tuning_info->af_algo.reserve[41];
  tuning_adapter_info->af_algo.algo_extended.en_detect_peak_in_decrease =
    (boolean)tuning_info->af_algo.reserve[42];
  tuning_adapter_info->af_algo.algo_extended.en_frame_skip_goto_peak =
    (boolean)tuning_info->af_algo.reserve[43];
  tuning_adapter_info->af_algo.algo_extended.en_fine_search_forcely =
    (boolean)tuning_info->af_algo.reserve[44];
  tuning_adapter_info->af_algo.algo_extended.hys_offset.enable =
    (boolean)tuning_info->af_algo.reserve[45];
  tuning_adapter_info->af_algo.algo_extended.hys_offset.offset =
    (int)tuning_info->af_algo.reserve[46];
  tuning_adapter_info->af_algo.algo_extended.fd_roi.enable =
    (boolean)tuning_info->af_algo.reserve[47];
  tuning_adapter_info->af_algo.algo_extended.fd_roi.ratio =
    (boolean)tuning_info->af_algo.reserve[48];
  tuning_adapter_info->af_algo.algo_extended.outdoor_roi.enable =
    (boolean)tuning_info->af_algo.reserve[49];
  tuning_adapter_info->af_algo.algo_extended.outdoor_roi.ratio_x =
    tuning_info->af_algo.reserve[50];
  tuning_adapter_info->af_algo.algo_extended.outdoor_roi.ratio_y =
    tuning_info->af_algo.reserve[51];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.enable =
    (boolean)tuning_info->af_algo.reserve[52];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_unstable_th =
    tuning_info->af_algo.reserve[53];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_stable_th =
    tuning_info->af_algo.reserve[54];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_trigger_cnt_th =
   (int)tuning_info->af_algo.reserve[55];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_unstable_th =
    tuning_info->af_algo.reserve[56];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_stable_th =
    tuning_info->af_algo.reserve[57];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_trigger_cnt_th =
   (boolean)tuning_info->af_algo.reserve[58];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_unstable_th =
    tuning_info->af_algo.reserve[59];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_stable_th =
    tuning_info->af_algo.reserve[60];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_trigger_cnt_th =
    (int)tuning_info->af_algo.reserve[61];
  tuning_adapter_info->af_algo.algo_extended.en_simple_prescan =
   (boolean)tuning_info->af_algo.reserve[62];
  tuning_adapter_info->af_algo.algo_extended.en_optimize_peak =
    (boolean)tuning_info->af_algo.reserve[63];
  tuning_adapter_info->af_algo.algo_extended.spot_light_filter.enable =
    (int)tuning_info->af_algo.reserve[70];
  /*Use reserve field index 71~80 for IIR 2nd order filter for spot light*/
  for(i = 0, j = 71; i< 10 ; i++, j++) {
    tuning_adapter_info->af_algo.algo_extended.spot_light_filter.iir_2tap_filter[i]  =
    (float) tuning_info->af_algo.reserve[j];
  }
  /*Use reserve field index 81~90 IIR 4th order filter for spot light*/
  for(i = 0, j = 81; i< 10 ; i++, j++) {
    tuning_adapter_info->af_algo.algo_extended.spot_light_filter.iir_4tap_filter[i]  =
    (float) tuning_info->af_algo.reserve[j];
  }

  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_extension =
   (int)tuning_info->af_algo.reserve[91];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_scale_x =
    (float)tuning_info->af_algo.reserve[92];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_scale_y =
    (float)tuning_info->af_algo.reserve[93];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_symmetric_x =
    (float)tuning_info->af_algo.reserve[94];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_symmetric_y =
    (float)tuning_info->af_algo.reserve[95];
  tuning_adapter_info->af_algo.algo_extended.face_trig_priority_enable =
    (boolean)tuning_info->af_algo.reserve[98];
  tuning_adapter_info->af_algo.algo_extended.exp_comp_enable =
    (boolean)tuning_info->af_algo.reserve[99];

#else
  for (i = 0, j = 0; i < AF_FOCUS_MODE_MAX; i++) {
    j = i * 3; /* three parameters inside each limit structure*/
    tuning_adapter_info->af_algo.algo_extended.search_limit[i].near_end =
      tuning_info->af_algo.reserve[j];
    tuning_adapter_info->af_algo.algo_extended.search_limit[i].far_end =
      tuning_info->af_algo.reserve[j + 1];
    tuning_adapter_info->af_algo.algo_extended.search_limit[i].default_pos =
      tuning_info->af_algo.reserve[j + 2];
  }
  tuning_adapter_info->af_algo.algo_extended.stats_comp_type =
    (af_stats_use_type)(int)tuning_info->af_algo.reserve[15];
  tuning_adapter_info->af_algo.algo_extended.hv_stats_ratio_enable =
    (boolean)tuning_info->af_algo.reserve[16];
  tuning_adapter_info->af_algo.algo_extended.gravity_x_land_thres =
  tuning_info->af_algo.reserve[17];
  tuning_adapter_info->af_algo.algo_extended.gravity_y_land_thres =
  tuning_info->af_algo.reserve[18];
  tuning_adapter_info->af_algo.algo_extended.gravity_x_port_thres =
  tuning_info->af_algo.reserve[19];
  tuning_adapter_info->af_algo.algo_extended.gravity_y_port_thres =
  tuning_info->af_algo.reserve[20];
  tuning_adapter_info->af_algo.algo_extended.hv_weight =
  tuning_info->af_algo.reserve[21];
  /* robust fd reserve param */
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.robust_fd_enable =
    (unsigned char)tuning_info->af_algo.reserve[64];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.noface_cnt_thr =
    (unsigned int)tuning_info->af_algo.reserve[65];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.stable_cnt_thr =
    (unsigned int)tuning_info->af_algo.reserve[66];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.fd_roi_pos_change_th =
    (float)tuning_info->af_algo.reserve[67];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.face_lost_trigger_enable =
    (unsigned char)tuning_info->af_algo.reserve[68];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.face_lost_stable_th =
    (unsigned int)tuning_info->af_algo.reserve[69];

  tuning_adapter_info->af_algo.algo_extended.en_ignore_first_invalid_frame =
    tuning_info->af_algo.reserve[22];
  tuning_adapter_info->af_algo.algo_extended.step_back.enable =
    tuning_info->af_algo.reserve[23];
  tuning_adapter_info->af_algo.algo_extended.step_back.start_inf =
    tuning_info->af_algo.reserve[24];
  tuning_adapter_info->af_algo.algo_extended.step_back.start_mac =
    tuning_info->af_algo.reserve[25];
  tuning_adapter_info->af_algo.algo_extended.step_back.reverse_inf =
    tuning_info->af_algo.reserve[26];
  tuning_adapter_info->af_algo.algo_extended.step_back.reverse_mac =
    tuning_info->af_algo.reserve[27];
  tuning_adapter_info->af_algo.algo_extended.scan_range.fratio_min =
    tuning_info->af_algo.reserve[28];
  tuning_adapter_info->af_algo.algo_extended.scan_range.fratio_max =
    tuning_info->af_algo.reserve[29];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_prev_y1 =
    tuning_info->af_algo.reserve[30];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_next_y1 =
    tuning_info->af_algo.reserve[31];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_prev_y2 =
    tuning_info->af_algo.reserve[32];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_next_y2 =
    tuning_info->af_algo.reserve[33];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_prev_y1 =
    tuning_info->af_algo.reserve[34];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_next_y1 =
    tuning_info->af_algo.reserve[35];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_prev_y2 =
    tuning_info->af_algo.reserve[36];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_next_y2 =
    tuning_info->af_algo.reserve[37];
  tuning_adapter_info->af_algo.algo_extended.flat_detection.enable =
    tuning_info->af_algo.reserve[38];
  tuning_adapter_info->af_algo.algo_extended.flat_detection.threshold_bv =
    tuning_info->af_algo.reserve[39];
  tuning_adapter_info->af_algo.algo_extended.flat_threshold.increase =
    tuning_info->af_algo.reserve[40];
  tuning_adapter_info->af_algo.algo_extended.flat_threshold.decrease =
    tuning_info->af_algo.reserve[41];
  tuning_adapter_info->af_algo.algo_extended.en_detect_peak_in_decrease =
    tuning_info->af_algo.reserve[42];
  tuning_adapter_info->af_algo.algo_extended.en_frame_skip_goto_peak =
    tuning_info->af_algo.reserve[43];
  tuning_adapter_info->af_algo.algo_extended.en_fine_search_forcely =
    tuning_info->af_algo.reserve[44];
  tuning_adapter_info->af_algo.algo_extended.hys_offset.enable =
    tuning_info->af_algo.reserve[45];
  tuning_adapter_info->af_algo.algo_extended.hys_offset.offset =
    tuning_info->af_algo.reserve[46];
  tuning_adapter_info->af_algo.algo_extended.fd_roi.enable =
    tuning_info->af_algo.reserve[47];
  tuning_adapter_info->af_algo.algo_extended.fd_roi.ratio =
    tuning_info->af_algo.reserve[48];
  tuning_adapter_info->af_algo.algo_extended.outdoor_roi.enable =
    tuning_info->af_algo.reserve[49];
  tuning_adapter_info->af_algo.algo_extended.outdoor_roi.ratio_x =
    tuning_info->af_algo.reserve[50];
  tuning_adapter_info->af_algo.algo_extended.outdoor_roi.ratio_y =
    tuning_info->af_algo.reserve[51];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.enable =
    tuning_info->af_algo.reserve[52];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_unstable_th =
    tuning_info->af_algo.reserve[53];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_stable_th =
    tuning_info->af_algo.reserve[54];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_trigger_cnt_th =
    tuning_info->af_algo.reserve[55];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_unstable_th =
    tuning_info->af_algo.reserve[56];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_stable_th =
    tuning_info->af_algo.reserve[57];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_trigger_cnt_th =
    tuning_info->af_algo.reserve[58];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_unstable_th =
    tuning_info->af_algo.reserve[59];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_stable_th =
    tuning_info->af_algo.reserve[60];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_trigger_cnt_th =
    tuning_info->af_algo.reserve[61];
  tuning_adapter_info->af_algo.algo_extended.en_simple_prescan =
    tuning_info->af_algo.reserve[62];
  tuning_adapter_info->af_algo.algo_extended.en_optimize_peak =
    tuning_info->af_algo.reserve[63];

  tuning_adapter_info->af_algo.algo_extended.spot_light_filter.enable =
    (int)tuning_info->af_algo.reserve[70];
  /*Use reserve field index 71~80 for IIR 2nd order filter for spot light*/
  for(i = 0, j = 71; i< 10 ; i++, j++) {
     tuning_adapter_info->af_algo.algo_extended.spot_light_filter.iir_2tap_filter[i]  =
     (float) tuning_info->af_algo.reserve[j];
  }
  /*Use reserve field index 81~90 IIR 4th order filter for spot light*/
  for(i = 0, j = 81; i< 10 ; i++, j++) {
    tuning_adapter_info->af_algo.algo_extended.spot_light_filter.iir_4tap_filter[i]  =
   (float) tuning_info->af_algo.reserve[j];
  }

  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_extension =
   (int)tuning_info->af_algo.reserve[91];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_scale_x =
    (float)tuning_info->af_algo.reserve[92];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_scale_y =
    (float)tuning_info->af_algo.reserve[93];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_symmetric_x =
    (float)tuning_info->af_algo.reserve[94];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_symmetric_y =
    (float)tuning_info->af_algo.reserve[95];
  tuning_adapter_info->af_algo.algo_extended.baf_exp_comp.enable =
    (boolean)tuning_info->af_algo.reserve[96];
  tuning_adapter_info->af_algo.algo_extended.baf_exp_comp.kickin_progress =
    (int)tuning_info->af_algo.reserve[97];

  tuning_adapter_info->af_algo.algo_extended.face_fv_weight_enable =
    (float)tuning_info->af_algo.reserve[98];
  tuning_adapter_info->af_algo.algo_extended.face_fv_h_weight =
    (float)tuning_info->af_algo.reserve[99];

#endif
/******************************************************************************
** af_tuning_sw_stats_adapter_t: af_algo_tune_parms_t
******************************************************************************/
   STATIC_ASSERT(sizeof(af_tuning_sw_stats_adapter_t) == sizeof(af_tuning_sw_stats_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_swaf_config,
    sizeof(af_tuning_sw_stats_adapter_t),
    &tuning_info->af_swaf_config,
    sizeof(af_tuning_sw_stats_t));

/******************************************************************************
** af_tuning_vfe_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   COPY_ARRAY(tuning_adapter_info->af_vfe,
             tuning_info->af_vfe);

/******************************************************************************
** af_tuning_baf_adapter_t: af_tuning_algo_adapter_t
******************************************************************************/
   COPY_ARRAY(tuning_adapter_info->af_baf.roi_config,
             tuning_info->af_baf.roi_config);
  COPY_ARRAY(tuning_adapter_info->af_baf.gamma,
             tuning_info->af_baf.gamma);
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  COPY_ARRAY(tuning_adapter_info->af_baf.filter,
             tuning_info->af_baf.filter);
#else
  {
  for (i = 0; i < Q3A_AF_SCENE_TYPE_MAX; i++) {
    COPY_ARRAY(tuning_adapter_info->af_baf.filter[i].HV_filter,
               tuning_info->af_baf.filter[i].HV_filter);

    COPY_ARRAY(tuning_adapter_info->af_baf.filter[i].HV_filter_2nd_tier,
               tuning_info->af_baf.filter[i].HV_filter_2nd_tier);

    COPY_FIELD(tuning_adapter_info->af_baf.filter[i].scaler,
               tuning_info->af_baf.filter[i].scaler);

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    COPY_FIELD(tuning_adapter_info->af_baf.filter[i].coefficient,
               tuning_info->af_baf.filter[i].coefficient);
#endif
  }
  }
#endif

  //get af algo params
  tuning_adapter_info->af_algo.algo_extended.af_algo_params = (af_algo_params*)af_get_algo_params();

  return true;
}


/*=============================================================================
 *  AEC Methods
 *=============================================================================*/

boolean parse_AEC_chromatix(void* aec_tuning_info, aec_algo_tuning_adapter* tuning_adapter)
{
  AEC_algo_struct_type* tuning_info = (AEC_algo_struct_type*)aec_tuning_info;

  if (tuning_info == NULL)
  {
    AEC_ERR("error: tuning_info is NULL");
    return false;
  }

  if (tuning_adapter == NULL)
  {
    AEC_ERR("error: tuning_adapter is NULL");
    return false;
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_generic
  *=============================================================================*/
  {
  aec_generic_tuning_adapter* dest = &tuning_adapter->aec_generic;
  aec_generic_tuning_type*    src  = &tuning_info->aec_generic;
  aec_face_priority_adapter* dest_face_AEC = &tuning_adapter->aec_face_detection;

  dest->aec_start_index =
    src->aec_start_index;
  dest->luma_tolerance =
    src->luma_tolerance;
  dest->pct_based_luma_tolerance_enable =
    src->pct_based_luma_tolerance_enable;
  dest->luma_tolerance_pct =
    src->luma_tolerance_pct;
  dest->frame_skip_startup =
    src->frame_skip_startup;
  dest->aggressiveness_startup =
    src->aggressiveness_startup;
  dest->exposure_index_adj_step =
    src->exposure_index_adj_step;
  dest->ISO100_gain =
    src->ISO100_gain;
  dest->antibanding_vs_ISO_priority =
    src->antibanding_vs_ISO_priority;
  dest->iso_quantization_enable =
    src->iso_quantization_enable;
  dest->max_snapshot_exposure_time_allowed =
    src->max_snapshot_exposure_time_allowed;
  dest->lux_index_ref =
    src->lux_index_ref;
  dest->aec_subsampling_factor =
    src->aec_subsampling_factor;

  dest->idx_comp.awb_exp_idx_comp_enable =
    src->idx_comp.awb_exp_idx_comp_enable;
  dest->idx_comp.lux_idx_comp_enable =
    src->idx_comp.lux_idx_comp_enable;
  dest->idx_comp.ref_sensitivity =
    src->idx_comp.ref_sensitivity;

  dest->aec_ev_table.steps_per_ev =
    src->aec_ev_table.steps_per_ev;
  dest->aec_ev_table.min_ev_val =
    src->aec_ev_table.min_ev_val;
  dest->aec_ev_table.max_ev_val =
    src->aec_ev_table.max_ev_val;

  COPY_ARRAY(dest->aec_ev_table.ev_comp_table,
             src->aec_ev_table.ev_comp_table);
  COPY_ARRAY(dest->aec_ev_table.bracketing_table,
             src->aec_ev_table.bracketing_table);

  dest->aec_luma_weight.r_weight =
    src->aec_luma_weight.r_weight;
  dest->aec_luma_weight.g_weight =
    src->aec_luma_weight.g_weight;
  dest->aec_luma_weight.b_weight =
    src->aec_luma_weight.b_weight;
  dest->aec_luma_weight.color_and_sensor_comp_enable =
    src->aec_luma_weight.color_and_sensor_comp_enable;
  dest->aec_luma_weight.ref_rGain =
    src->aec_luma_weight.ref_rGain;
  dest->aec_luma_weight.ref_gGain =
    src->aec_luma_weight.ref_gGain;
  dest->aec_luma_weight.ref_bGain =
    src->aec_luma_weight.ref_bGain;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && ((CHROMATIX_VERSION == 0x0309) || (CHROMATIX_VERSION == 0x0310))
  dest->dual_cam_exposure_mult = src->reserved[3];
  dest->dual_cam_comp_ratio[0] = src->reserved[4];
  dest->dual_cam_comp_ratio[1] = src->reserved[5];
  dest->dual_cam_comp_ratio[2] = src->reserved[6];
  dest->dual_cam_comp_ratio[3] = src->reserved[7];
  dest->dual_cam_comp_ratio[4] = src->reserved[8];
  dest->dual_cam_comp_ratio[5] = src->reserved[9];

  /* Fast AEC tuning parameters */
  dest->fast_aec.start_index        =
    (int)(((int)src->reserved[12] > 0) ? (int)(src->reserved[12]) : (int)(dest->aec_start_index));
  dest->fast_aec.startup_frame_skip =
    (int)(((int)src->reserved[13] > 0) ? (int)(src->reserved[13]) : (FAST_AEC_STARTUP_FRAME_SKIP));
  dest->fast_aec.frame_skip         =
    (int)(((int)src->reserved[14] > 0) ? (int)(src->reserved[14]) : (FAST_AEC_FRAME_SKIP));
  dest->fast_aec.luma_settled_count =
    (int)(((int)src->reserved[15] > 0) ? (int)(src->reserved[15]) : (FAST_AEC_MIN_FRAME_SETTLE_THRESHOLD));

  /* LLS: OIS feature related  */
  dest->low_light_shot.enable              = (uint32)src->reserved[20];
  dest->low_light_shot.luxindex_thr_500lux = src->reserved[21];
  dest->low_light_shot.luxindex_thr_200lux = src->reserved[22];
  dest->low_light_shot.luxindex_thr_100lux = src->reserved[23];
  dest->low_light_shot.luxindex_thr_050lux = src->reserved[24];
  dest->low_light_shot.luxindex_thr_010lux = src->reserved[25];
  dest->low_light_shot.req_exp_time_500lux = src->reserved[26];
  dest->low_light_shot.req_exp_time_200lux = src->reserved[27];
  dest->low_light_shot.req_exp_time_100lux = src->reserved[28];
  dest->low_light_shot.req_exp_time_050lux = src->reserved[29];
  dest->low_light_shot.req_exp_time_010lux = src->reserved[30];
  dest->low_light_shot.req_extend_max_gain = src->reserved[44];

  /* Additional flat scene parameters */
  dest->flat_non_grey_th_low  = (src->reserved[31] > 0 ? src->reserved[31] : 0.5f);
  dest->flat_non_grey_th_high = (src->reserved[32] > 0 ? src->reserved[32] : 0.8f);

  /* LV test enable flag */
  dest->enable_LV_test = (int)src->reserved[33];

  /* Temporal filtering of the lux index value */
  dest->lux_index_filter_factor = ((int)(src->reserved[34] * 100) > 0) ? (src->reserved[34]) : (LUX_INDEX_TEMPORAL_FILTER_FACTOR);

  /* Cap exposure index for flash capture */
  dest->cap_exp_index_ratio_for_flash_estimation = src->reserved[36];

  /* Warm Start Tuning Parameters */
  dest->enable_stored_params = (boolean)(((boolean)src->reserved[37] > 0) ? TRUE : FALSE);
  dest->warmstart_exp_range  =
    (float)((src->reserved[38] > 0.0f) ? (src->reserved[38]) : (WARM_START_EXP_RANGE_PERC)) / 100.0f;

  /* Aggressive AEC tuning parameters */
  dest->aggressive.startup_frame_skip  =
    (uint32)((src->reserved[39] > 0.0f) ? (src->reserved[39]) : (AGGRESSIVE_AEC_STARTUP_FRAME_SKIP));
  dest->aggressive.frame_skip          =
    (uint32)((src->reserved[40] > 0.0f) ? (src->reserved[40]) : (AGGRESSIVE_AEC_FRAME_SKIP));
  dest->aggressive.luma_tol_percentage =
    (float)((src->reserved[41] > 0.0f) ? (src->reserved[41]) : (AGGRESSIVE_AEC_LUMA_TOLERANCE_PCT)) / 100.0f;
  dest->aggressive.max_frame_converge  =
    (uint32)((src->reserved[42] > 0.0f) ? (src->reserved[42]) : (AGGRESSIVE_AEC_MAX_FRAME_CONVERGENCE_THRESHOLD));
  dest->aggressive.min_settled_frame   =
    (uint32)((src->reserved[43] > 0.0f) ? (src->reserved[43]) : (AGGRESSIVE_AEC_MIN_FRAME_SETTLE_THRESHOLD));
  dest->aggressive.min_awb_settle_count = AGGRESSIVE_AEC_MIN_AWB_SETTLE_COUNT;

  dest->fast_aec.use_digital_gain   = (((int)src->reserved[46] > 0) ? TRUE : FALSE);

  /* AEC HAL lock for preflash */
  dest->ignore_hal_lock_before_preflash = (uint32)src->reserved[47];

  dest->fast_aec_forced_cnt = (uint16_t)src->reserved[48];
  /* AEC requested region skip pattern */
  dest->requested_rgn_skip_pattern      = (int)src->reserved[49];

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  /* FACE AEC Max. & Min. weight for different trigger zones */
  dest_face_AEC->face_wt_zone.data[0].face_wt_max=src->reserved[50];
  dest_face_AEC->face_wt_zone.data[1].face_wt_max=src->reserved[51];
  dest_face_AEC->face_wt_zone.data[2].face_wt_max=src->reserved[52];
  dest_face_AEC->face_wt_zone.data[3].face_wt_max=src->reserved[53];
  dest_face_AEC->face_wt_zone.data[4].face_wt_max=src->reserved[54];
  dest_face_AEC->face_wt_zone.data[5].face_wt_max=src->reserved[55];
  dest_face_AEC->face_wt_zone.data[0].face_wt_min=src->reserved[56];
  dest_face_AEC->face_wt_zone.data[1].face_wt_min=src->reserved[57];
  dest_face_AEC->face_wt_zone.data[2].face_wt_min=src->reserved[58];
  dest_face_AEC->face_wt_zone.data[3].face_wt_min=src->reserved[59];
  dest_face_AEC->face_wt_zone.data[4].face_wt_min=src->reserved[60];
  dest_face_AEC->face_wt_zone.data[5].face_wt_min=src->reserved[61];
#endif

  /* AEC dynamic frame skip */
  dest->dynamic_frame_skip_config.enable_frame_skip = AEC_DYNAMIC_FRAME_SKIP_ENABLE;
  dest->dynamic_frame_skip_config.exp_index_tolerance = AEC_DYNAMIC_FRAME_SKIP_EXP_INDEX_TOLERANCE;
  dest->dynamic_frame_skip_config.max_frame_skip =
    (src->reserved[45] == 1.0f) ? (int)src->reserved[10] : AEC_DYNAMIC_FRAME_SKIP_DEFAULT_MAX_SKIP_COUNT;
  dest->dynamic_frame_skip_config.min_fps_threshold =
    (src->reserved[45] == 1.0f) ? (int)src->reserved[11] : AEC_DYNAMIC_FRAME_SKIP_MIN_FPS_THRESHOLD;

  /* Auto LLS feature */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->multi_frame_LLS_trigger_idx = (int)src->reserved[35];
#else
  dest->multi_frame_LLS_trigger_idx = (int)src->reserved[62];
#endif

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
#if 0
  dest->HDR_3exp_tuning.dark_percentage_threshold = src->reserved[66];
  dest->HDR_3exp_tuning.sat_percentage_threshold = src->reserved[67];
  dest->HDR_3exp_tuning.lux_adjusted_target_offset = src->reserved[68];
  dest->HDR_3exp_tuning.AE_hist_bin_weight_adjust_enable   = (((int)src->reserved[69] > 0) ? TRUE : FALSE);
  Q3A_MEMCPY_S(dest->HDR_3exp_tuning.AE_hist_bin_weight_adjust, sizeof(dest->HDR_3exp_tuning.AE_hist_bin_weight_adjust) / sizeof(float),
               &src->reserved[70], sizeof(dest->HDR_3exp_tuning.AE_hist_bin_weight_adjust) / sizeof(float));

  dest->HDR_3exp_tuning.aec_luma_adjust.num_zones = (int)src->reserved[198];
  for(int i = 0; i < (dest->HDR_3exp_tuning.aec_luma_adjust.num_zones + 1); i++) {
    dest->HDR_3exp_tuning.aec_luma_adjust.triggers[i].start = (int)src->reserved[199 + i * 7];
    dest->HDR_3exp_tuning.aec_luma_adjust.triggers[i].end = (int)src->reserved[200 + i * 7];

    dest->HDR_3exp_tuning.aec_luma_adjust.data[i].dark_percentage_threshold = src->reserved[201 + i * 7];
    dest->HDR_3exp_tuning.aec_luma_adjust.data[i].sat_percentage_threshold = src->reserved[202 + i * 7];
    dest->HDR_3exp_tuning.aec_luma_adjust.data[i].dark_sat_percentage_threshold = src->reserved[203 + i * 7];
    dest->HDR_3exp_tuning.aec_luma_adjust.data[i].dark_bin_weight_adjust = src->reserved[204 + i * 7];
    dest->HDR_3exp_tuning.aec_luma_adjust.data[i].sat_bin_weight_adjust = src->reserved[205 + i * 7];
    AEC_LOW("[%d]\t%d\t%d\t%f\t%f\t%f\t%f\t%f", i,
        dest->HDR_3exp_tuning.aec_luma_adjust.triggers[i].start,
        dest->HDR_3exp_tuning.aec_luma_adjust.triggers[i].end,
        dest->HDR_3exp_tuning.aec_luma_adjust.data[i].dark_percentage_threshold,
        dest->HDR_3exp_tuning.aec_luma_adjust.data[i].sat_percentage_threshold,
        dest->HDR_3exp_tuning.aec_luma_adjust.data[i].dark_sat_percentage_threshold,
        dest->HDR_3exp_tuning.aec_luma_adjust.data[i].dark_bin_weight_adjust,
        dest->HDR_3exp_tuning.aec_luma_adjust.data[i].sat_bin_weight_adjust);
  }
#endif
  dest->HDR_3exp_tuning.face_roi_factor = src->reserved[71];
  dest->HDR_3exp_tuning.target_adjust_zone_num = (int)src->reserved[72];
  dest->HDR_3exp_tuning.gain[0] = src->reserved[73];
  dest->HDR_3exp_tuning.gain[1] = src->reserved[74];
  dest->HDR_3exp_tuning.gain[2] = src->reserved[75];
  dest->HDR_3exp_tuning.gain[3] = src->reserved[76];
  dest->HDR_3exp_tuning.gain[4] = src->reserved[77];

  dest->HDR_3exp_tuning.tolerance[0] = src->reserved[78];
  dest->HDR_3exp_tuning.tolerance[1] = src->reserved[79];
  dest->HDR_3exp_tuning.tolerance[2] = src->reserved[80];
  dest->HDR_3exp_tuning.tolerance[3] = src->reserved[81];
  dest->HDR_3exp_tuning.tolerance[4] = src->reserved[82];
  dest->HDR_3exp_tuning.enable_target_adjust = (boolean)src->reserved[83];
  dest->HDR_3exp_tuning.es_ratio = src->reserved[84];
  dest->HDR_3exp_tuning.enable_HDR3 = (boolean)src->reserved[85];
#endif

#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_exposure_table
  *=============================================================================*/
  {
  aec_exposure_table_adapter* dest = &tuning_adapter->aec_exposure_table;
  aec_exposure_table_type*    src  = &tuning_info->aec_exposure_table;

  dest->valid_entries =
    src->valid_entries;
  dest->aec_enable_digital_gain_for_EV_lowlight =
    src->aec_enable_digital_gain_for_EV_lowlight;
  dest->total_entries_with_digital_gain_for_EV =
    src->total_entries_with_digital_gain_for_EV;
  dest->fix_fps_aec_table_index =
    src->fix_fps_aec_table_index;

  COPY_ARRAY(dest->exposure_entries,
             src->exposure_entries);
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_snapshot_lut
  *=============================================================================*/
  {
  aec_snapshot_exposure_adapter* dest = &tuning_adapter->aec_snapshot_lut;
  aec_snapshot_exposure_type*    src  = &tuning_info->aec_snapshot_lut;

  dest->enable =
    src->enable;
  dest->exposure_stretch_enable =
    src->exposure_stretch_enable;
  dest->valid_entries =
    src->valid_entries;

  COPY_ARRAY(dest->snapshot_ae_table, src->snapshot_ae_table);
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_luma_target
  *=============================================================================*/
  {
  aec_luma_target_adapter* dest = &tuning_adapter->aec_luma_target;
  aec_luma_target_type*    src  = &tuning_info->aec_luma_target;

  dest->num_zones =
    src->num_zones;

  COPY_ARRAY(dest->triggers, src->triggers);
  COPY_ARRAY(dest->triggered_params, src->triggered_params);
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_metering_tables
  *=============================================================================*/
  {
  aec_metering_table_adapter* dest = &tuning_adapter->aec_metering_tables;
  aec_metering_table_type*    src  = &tuning_info->aec_metering_tables;

  COPY_ARRAY(dest->AEC_weight_center_weighted,
             src->AEC_weight_center_weighted);
  COPY_ARRAY(dest->AEC_weight_spot_metering,
             src->AEC_weight_spot_metering);
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_fast_convergence
  *=============================================================================*/
  {
  aec_fast_conv_adapter* dest = &tuning_adapter->aec_fast_convergence;
  aec_fast_conv_type*    src  = &tuning_info->aec_fast_convergence;

  dest->frame_skip =
    src->frame_skip;
  dest->aggressiveness =
    src->aggressiveness;

  /* Mapping reserved to extended paremters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->low_luma_cap_ratio  = src->reserved[0];
  dest->bright_ratio_adjust = src->reserved[1];
  dest->dark_ratio_adjust   = src->reserved[2];
  dest->enable_speed_adjust = (boolean)src->reserved[3];
#else
  /* TODO: verify that these same reserved parameters are used for 310*/
  dest->low_luma_cap_ratio  = src->reserved[0];
  dest->bright_ratio_adjust = src->reserved[1];
  dest->dark_ratio_adjust   = src->reserved[2];
  dest->enable_speed_adjust = (boolean)src->reserved[3];
#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_slow_convergence
  *=============================================================================*/
  {
  aec_slow_conv_adapter* dest = &tuning_adapter->aec_slow_convergence;
  aec_slow_conv_type*    src  = &tuning_info->aec_slow_convergence;

  dest->slow_convergence_enable =
    src->slow_convergence_enable;
  dest->frame_skip =
    src->frame_skip;
  dest->conv_speed =
    src->conv_speed;
  dest->ht_enable =
    src->ht_enable;
  dest->ht_tolerance_adjust_factor =
    src->ht_tolerance_adjust_factor;
  dest->ht_thres =
    src->ht_thres;
  dest->ht_max =
    src->ht_max;
  dest->ht_gyro_enable =
    src->ht_gyro_enable;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->ref_frame_rate             = src->reserved[0];
  dest->step_dark                  = src->reserved[1] * (1 - dest->conv_speed)
                                   + src->reserved[2] * dest->conv_speed;
  dest->step_bright                = src->reserved[3] * (1 - dest->conv_speed)
                                   + src->reserved[4] * dest->conv_speed;
  dest->step_regular               = src->reserved[5] * (1 - dest->conv_speed)
                                   + src->reserved[6] * dest->conv_speed;
  dest->luma_tol_ratio_dark        = src->reserved[7] * (1 - dest->conv_speed)
                                   + src->reserved[8] * dest->conv_speed;
  dest->luma_tol_ratio_bright      = src->reserved[9] * (1 - dest->conv_speed)
                                   + src->reserved[10] * dest->conv_speed;
  dest->raw_step_adjust_cap        = src->reserved[11];
  dest->adjust_skip_luma_tolerance = (int)src->reserved[12];
  dest->bright_region_num_thres    = src->reserved[13];
  dest->dark_region_num_thres      = src->reserved[14];

  /* Mapping holding time control to extended tuning parameters */
  dest->ht_luma_thres_low          = src->reserved[15];
  dest->ht_luma_thres_high         = src->reserved[16];
  dest->ht_luma_val_low            = src->reserved[17];
  dest->ht_luma_val_high           = src->reserved[18];
  dest->ht_gyro_thres_low          = src->reserved[19];
  dest->ht_gyro_thres_high         = src->reserved[20];
  dest->ht_gyro_val_low            = src->reserved[21];
  dest->ht_gyro_val_high           = src->reserved[22];
#else
  /* TODO: verify that these same reserved parameters are used for 310*/
  dest->ref_frame_rate             = src->reserved[0];
  dest->step_dark                  = src->reserved[1] * (1 - dest->conv_speed)
                                   + src->reserved[2] * dest->conv_speed;
  dest->step_bright                = src->reserved[3] * (1 - dest->conv_speed)
                                   + src->reserved[4] * dest->conv_speed;
  dest->step_regular               = src->reserved[5] * (1 - dest->conv_speed)
                                   + src->reserved[6] * dest->conv_speed;
  dest->luma_tol_ratio_dark        = src->reserved[7] * (1 - dest->conv_speed)
                                   + src->reserved[8] * dest->conv_speed;
  dest->luma_tol_ratio_bright      = src->reserved[9] * (1 - dest->conv_speed)
                                   + src->reserved[10] * dest->conv_speed;
  dest->raw_step_adjust_cap        = src->reserved[11];
  dest->adjust_skip_luma_tolerance = (int)src->reserved[12];
  dest->bright_region_num_thres    = src->reserved[13];
  dest->dark_region_num_thres      = src->reserved[14];

  /* Mapping holding time control to extended tuning parameters */
  dest->ht_luma_thres_low          = src->reserved[15];
  dest->ht_luma_thres_high         = src->reserved[16];
  dest->ht_luma_val_low            = src->reserved[17];
  dest->ht_luma_val_high           = src->reserved[18];
  dest->ht_gyro_thres_low          = src->reserved[19];
  dest->ht_gyro_thres_high         = src->reserved[20];
  dest->ht_gyro_val_low            = src->reserved[21];
  dest->ht_gyro_val_high           = src->reserved[22];
#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_led_flash
  *=============================================================================*/
  {
  aec_flash_tuning_adapter* dest = &tuning_adapter->aec_led_flash;
  aec_flash_tuning_type*    src  = &tuning_info->aec_led_flash;
  aec_generic_tuning_type*  src_generic  = &tuning_info->aec_generic;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION >= 0x0310)
  int i = 0, j = 0;
#endif
  dest->wled_trigger.start =
    src->wled_trigger.start;
  dest->wled_trigger.end =
    src->wled_trigger.end;
  dest->force_led_af_in_led_on_mode =
    src->force_led_af_in_led_on_mode;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->led_flux_used = true;
  dest->led_flux.pre_flux =
    src->aec_led_pre_flux;
  dest->led_flux.flux_hi =
    src->aec_led_flux_hi;
  dest->led_flux.flux_med =
    src->aec_led_flux_med;
  dest->led_flux.flux_low =
    src->aec_led_flux_low;
#else
  dest->led_flux_used = false;
  dest->led_table_tuning.dual_led_enable =
    src->led_table_tuning.dual_led_enable;
  dest->led_table_tuning.dual_led_intersect_slope =
    src->led_table_tuning.dual_led_intersect_slope;
  dest->led_table_tuning.led_power_size =
    src->led_table_tuning.led_power_size;
  dest->led_table_tuning.led_mix_size =
    src->led_table_tuning.led_mix_size;

  for (i = 0; i < MAX_LED_POWER_LEVEL; i++) {
    for (j = 0; j < MAX_LED_MIX_LEVEL; j++) {
      dest->led_table_tuning.cct_table[i][j].LED1_setting =
        src->led_table_tuning.cct_table[i][j].LED1_setting;
      dest->led_table_tuning.cct_table[i][j].LED2_setting =
        src->led_table_tuning.cct_table[i][j].LED2_setting;
      dest->led_table_tuning.cct_table[i][j].rg_ratio =
        src->led_table_tuning.cct_table[i][j].rg_ratio;
      dest->led_table_tuning.cct_table[i][j].bg_ratio =
        src->led_table_tuning.cct_table[i][j].bg_ratio;
      dest->led_table_tuning.cct_table[i][j].flux =
        src->led_table_tuning.cct_table[i][j].flux;
    }
  }

  dest->led_table_tuning.cct_table_calibration.calib_num =
    src->led_table_tuning.cct_table_calibration.calib_num;

  COPY_ARRAY(dest->led_table_tuning.cct_table_calibration.calib_entries,
             src->led_table_tuning.cct_table_calibration.calib_entries);

  dest->led_table_tuning.cct_table_calibration.corner_sample_screen_enable =
    src->led_table_tuning.cct_table_calibration.corner_sample_screen_enable;

  COPY_ARRAY(dest->led_table_tuning.cct_table_calibration.rg_ratio_ok_range,
             src->led_table_tuning.cct_table_calibration.rg_ratio_ok_range);

  COPY_ARRAY(dest->led_table_tuning.cct_table_calibration.bg_ratio_ok_range,
             src->led_table_tuning.cct_table_calibration.bg_ratio_ok_range);

  COPY_ARRAY(dest->led_table_tuning.cct_table_calibration.fixed_CCTs,
             src->led_table_tuning.cct_table_calibration.fixed_CCTs);

  COPY_ARRAY(dest->led_table_tuning.cct_table_calibration.fixed_exp_index,
             src->led_table_tuning.cct_table_calibration.fixed_exp_index);

  dest->led_table_tuning.cct_table_calibration.frame_err_allowance =
    src->led_table_tuning.cct_table_calibration.frame_err_allowance;
  dest->led_table_tuning.cct_table_calibration.frame_num =
    src->led_table_tuning.cct_table_calibration.frame_num;
  dest->led_table_tuning.cct_table_calibration.frame_num_max =
    src->led_table_tuning.cct_table_calibration.frame_num_max;
#endif

  dest->smart_flash_est_enable =
    src->smart_flash_est_enable;
  dest->smart_flash_est_strength =
    src->smart_flash_est_strength;
  dest->smart_flash_est_awb_enable =
    src->smart_flash_est_awb_enable;
  dest->smart_flash_est_awb_strength =
    src->smart_flash_est_awb_strength;

  dest->flash_target_enable =
    src->flash_target_enable;
  dest->flash_target.num_zones =
    src->flash_target.num_zones;

  COPY_ARRAY(dest->flash_target.triggers,
             src->flash_target.triggers);

  COPY_ARRAY(dest->flash_target.triggered_params,
             src->flash_target.triggered_params);

  dest->target_interp_k_start =
    src->target_interp_k_start;
  dest->target_interp_k_end =
    src->target_interp_k_end;
  dest->smart_flash_target_enable =
    src->smart_flash_target_enable;
  dest->max_target_offset =
    src->max_target_offset;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->bright_level_diff_thres = (int)(src->reserved[0]);
  dest->bright_level_thres      = (int)(src->reserved[1]);
  dest->bright_sat_diff_thres   = src->reserved[2];
  dest->bright_sat_thres        = src->reserved[3];
  dest->conv_min_luma           = (int)(src->reserved[4]);
  dest->conv_min_adjust         = (int)(src->reserved[5]);
  dest->off_stats_min           = (int)(src->reserved[6]);
  dest->sat_pct_thres           = src->reserved[7];
  dest->k_stats_pct             = src->reserved[8];
  dest->k_stats_min_diff        = src->reserved[9];
  dest->high_stat_target        = (int)(src->reserved[10]);
  dest->led_conv_smooth_rate    = src->reserved[11] == 0 ? 1 : src->reserved[11];
  /* For LED bias Table adj with Saturation prevention */
  dest->enable_led_sat_prevent  = (int)src_generic->reserved[0];
  dest->enable_speckle_handling = (int)src_generic->reserved[1];
  dest->k_floor_pct = src_generic->reserved[2];
  dest->sat_prevent_strength    = src->reserved[12];
  /* For LED bias Table adj with circular speckle handling */
  dest->sat_pct_start           = src->reserved[13];
  dest->sat_pct_end             = src->reserved[14];
  dest->lcd_flash_target        = (float)src_generic->reserved[45];
#else
  /* TODO: verify that these same reserved parameters are used for 310*/
  dest->bright_level_diff_thres = (int)(src->reserved[0]);
  dest->bright_level_thres      = (int)(src->reserved[1]);
  dest->bright_sat_diff_thres   = src->reserved[2];
  dest->bright_sat_thres        = src->reserved[3];
  dest->conv_min_luma           = (int)(src->reserved[4]);
  dest->conv_min_adjust         = (int)(src->reserved[5]);
  dest->off_stats_min           = (int)(src->reserved[6]);
  dest->sat_pct_thres           = src->reserved[7];
  dest->k_stats_pct             = src->reserved[8];
  dest->k_stats_min_diff        = src->reserved[9];
  dest->high_stat_target        = (int)(src->reserved[10]);
  dest->led_conv_smooth_rate    = src->reserved[11] == 0 ? 1 : src->reserved[11];
  /* For LED bias Table adj with Saturation prevention */
  dest->enable_led_sat_prevent = src_generic->reserved[0];
  dest->enable_speckle_handling = src_generic->reserved[1];
  dest->k_floor_pct = src_generic->reserved[2];
  dest->sat_prevent_strength    = src->reserved[12];
  /* For LED bias Table adj with circular speckle handling */
  dest->sat_pct_start           = src->reserved[13];
  dest->sat_pct_end             = src->reserved[14];
  dest->lcd_flash_target        = (float)src_generic->reserved[64];
#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_touch
  *=============================================================================*/
  {
  aec_touch_adapter* dest = &tuning_adapter->aec_touch;
  aec_touch_type*    src  = &tuning_info->aec_touch;

  dest->touch_roi_enable =
    src->touch_roi_enable;
  dest->touch_roi_weight =
    src->touch_roi_weight;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->temporal_filter_weight         = src->reserved[0];
  dest->speed_adjust_ratio             = src->reserved[1];
  dest->center_weight_strength         = src->reserved[2];
  dest->touch_LED_metering_enable      = (boolean)(src->reserved[3]);

  /* Touch EV feature */
  dest->touch_EV_enable     = (boolean)tuning_info->aec_generic.reserved[16];
  dest->luma_delta_thres    = (uint16)tuning_info->aec_generic.reserved[17];
  dest->luma_avg_pct_thres  = tuning_info->aec_generic.reserved[18];
  dest->luma_avg_min        = tuning_info->aec_generic.reserved[19];
#else
  /* TODO: verify that these same reserved parameters are used for 310*/
  dest->temporal_filter_weight         = src->reserved[0];
  dest->speed_adjust_ratio             = src->reserved[1];
  dest->center_weight_strength         = src->reserved[2];
  dest->touch_LED_metering_enable      = (boolean)(src->reserved[3]);

  /* Touch EV feature */
  dest->touch_EV_enable     = (boolean)tuning_info->aec_generic.reserved[16];
  dest->luma_delta_thres    = (uint16)tuning_info->aec_generic.reserved[17];
  dest->luma_avg_pct_thres  = tuning_info->aec_generic.reserved[18];
  dest->luma_avg_min        = tuning_info->aec_generic.reserved[19];
#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_face_detection
  *=============================================================================*/
  {
  aec_face_priority_adapter* dest = &tuning_adapter->aec_face_detection;
  aec_face_priority_type*    src  = &tuning_info->aec_face_detection;

  dest->aec_face_enable =
    src->aec_face_enable;
  dest->aec_face_weight =
    src->aec_face_weight;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->status_update_delay_time_enter     = src->reserved[0];
  dest->status_update_delay_time_exit      = src->reserved[1];
  dest->speed_adjust_delay_time_enter      = src->reserved[2];
  dest->speed_adjust_delay_time_exit       = src->reserved[3];
  dest->speed_adjust_ratio                 = src->reserved[4];
  dest->temporal_filter_weight             = src->reserved[5];
  dest->center_weight_strength             = src->reserved[6];

  /* Backlit compensation feature */
  dest->backlit_comp_enable                = (boolean)src->reserved[7];
  dest->backlit_weight_adjust_hi           = src->reserved[8];
  dest->backlit_weight_adjust_low          = src->reserved[9];
  dest->backlit_luma_ratio_thres_hi        = src->reserved[10];
  dest->backlit_luma_ratio_thres_low       = src->reserved[11];

  /* Face luma target */
  dest->face_luma_target                = tuning_info->aec_generic.reserved[44];
#else
  /* TODO: verify that these same reserved parameters are used for 310*/
  dest->status_update_delay_time_enter     = src->reserved[0];
  dest->status_update_delay_time_exit      = src->reserved[1];
  dest->speed_adjust_delay_time_enter      = src->reserved[2];
  dest->speed_adjust_delay_time_exit       = src->reserved[3];
  dest->speed_adjust_ratio                 = src->reserved[4];
  /* Disable temporal filter */
  dest->temporal_filter_weight             =  0.0f;
  dest->center_weight_strength             = src->reserved[6];

  /* Backlit compensation feature */
  dest->backlit_comp_enable                = (boolean)src->reserved[7];
  dest->backlit_weight_adjust_hi           = src->reserved[8];
  dest->backlit_weight_adjust_low          = src->reserved[9];
  dest->backlit_luma_ratio_thres_hi        = src->reserved[10];
  dest->backlit_luma_ratio_thres_low       = src->reserved[11];

  /* Face luma target */
  dest->face_luma_target                = tuning_info->aec_generic.reserved[63];

  /* Frontlit compensation feature */
  dest->frontlit_comp_enable                 = (boolean)tuning_info->aec_generic.reserved[66];
  dest->frontlit_weight_adjust_hi            = tuning_info->aec_generic.reserved[67];
  dest->frontlit_weight_adjust_low           = tuning_info->aec_generic.reserved[68];
  dest->frontlit_luma_ratio_thres_hi         = tuning_info->aec_generic.reserved[69];
  dest->frontlit_luma_ratio_thres_low        = tuning_info->aec_generic.reserved[70];

#endif

    /* Face luma weight adjustment feature */
    /* NOTE: Max face weight is common.
       NOTE: No space for enabled flag, we use face_sz_max also as enable flag */
    dest->face_sz_max                        = src->reserved[13];
    dest->face_sz_min                        = src->reserved[14];
    if (dest->face_sz_max <= 0.0f) {
      /* This is equivalent to disabling adjustment */
      dest->face_sz_min = dest->face_sz_max;
     }
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_led_fd
  *=============================================================================*/
  {
  aec_led_fd_adapter*       dest    = &tuning_adapter->aec_led_fd;
  aec_hdr_tuning_type*      src_hdr = &tuning_info->aec_hdr;

  dest->enable_led_fd_aec               = (int)src_hdr->reserved[15];
  dest->lux_index_off_req_delay.start   = (int)src_hdr->reserved[16];
  dest->lux_index_off_req_delay.end     = (int)src_hdr->reserved[17];
  dest->led_face_wt_max                 = src_hdr->reserved[18];
  dest->led_face_wt_min                 = src_hdr->reserved[19];
  dest->led_face_sz_max                 = src_hdr->reserved[20];
  dest->led_face_sz_min                 = src_hdr->reserved[21];
  dest->led_fd_tuned_frame_count        = (uint8)src_hdr->reserved[22];
  dest->led_fd_luma_target              = (int)src_hdr->reserved[23];
  dest->led_fd_effect_face_sz_max       = src_hdr->reserved[24];
  dest->led_fd_effect_face_sz_min       = src_hdr->reserved[25];

  if (dest->led_fd_effect_face_sz_min < 0.0f)
    dest->led_fd_effect_face_sz_min = 0.0f;
  else if (dest->led_fd_effect_face_sz_min > 1.0f)
    dest->led_fd_effect_face_sz_min = 1.0f;

  if (dest->led_fd_effect_face_sz_max < 0.0f)
    dest->led_fd_effect_face_sz_max = 0.0f;
  else if (dest->led_fd_effect_face_sz_max > 1.0f)
    dest->led_fd_effect_face_sz_max = 1.0f;

  if (dest->led_fd_effect_face_sz_max < dest->led_fd_effect_face_sz_min)
    dest->led_fd_effect_face_sz_max = dest->led_fd_effect_face_sz_min;
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_bright_region
  *=============================================================================*/
  {
  aec_bright_region_adapter* dest = &tuning_adapter->aec_bright_region;
  aec_bright_region_type*    src  = &tuning_info->aec_bright_region;

  dest->bright_region_enable =
    src->bright_region_enable;
  dest->num_zones =
    src->num_zones;

  COPY_ARRAY(dest->triggers, src->triggers);

  COPY_ARRAY(dest->triggered_params, src->triggered_params);

  dest->min_tweight =
    src->min_tweight;
  dest->max_bright_pct =
    src->max_bright_pct;
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_dark_region
  *=============================================================================*/
  {
  aec_dark_region_adapter* dest = &tuning_adapter->aec_dark_region;
  aec_dark_region_type*    src  = &tuning_info->aec_dark_region;

  dest->dark_region_enable =
    src->dark_region_enable;
  dest->num_zones =
    src->num_zones;

  COPY_ARRAY(dest->triggers, src->triggers);

  COPY_ARRAY(dest->triggered_params, src->triggered_params);

  dest->min_tweight =
    src->min_tweight;
  dest->max_dark_pct =
    src->max_dark_pct;
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_extreme_color
  *=============================================================================*/
  {
  aec_extreme_color_adapter* dest = &tuning_adapter->aec_extreme_color;
  aec_extreme_color_type*    src  = &tuning_info->aec_extreme_color;

  dest->extreme_color_enable =
    src->extreme_color_enable;
  dest->num_zones =
    src->num_zones;

  COPY_ARRAY(dest->triggers, src->triggers);

  COPY_ARRAY(dest->triggered_params, src->triggered_params);

  dest->red_th =
    src->red_th;
  dest->red_interp_pct =
    src->red_interp_pct;
  dest->green_rg_th =
    src->green_rg_th;
  dest->green_rg_interp_pct =
    src->green_rg_interp_pct;
  dest->green_bg_th =
    src->green_bg_th;
  dest->green_bg_interp_pct =
    src->green_bg_interp_pct;
  dest->green_cntr_th =
    src->green_cntr_th;
  dest->green_cntr_interp_pct =
    src->green_cntr_interp_pct;
  dest->blue_th =
    src->blue_th;
  dest->blue_interp_pct =
    src->blue_interp_pct;
  dest->luma_high_th =
    src->luma_high_th;
  dest->luma_low_th =
    src->luma_low_th;
  dest->color_stats_pct_high_th =
    src->color_stats_pct_high_th;
  dest->color_stats_pct_low_th =
    src->color_stats_pct_low_th;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  /* The period of print stats for debug. It is given in percentage */
  dest->dbg_print_period        = src->reserved[0];
#else
  /* TODO: verify that these same reserved parameters are used for 310*/
  /* The period of print stats for debug. It is given in percentage */
  dest->dbg_print_period        = src->reserved[0];
#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_flat_scene
  *=============================================================================*/
  {
  aec_flat_scene_adapter* dest = &tuning_adapter->aec_flat_scene;
  aec_flat_scene_type*    src  = &tuning_info->aec_flat_scene;

  dest->hist_flat_detector_enable =
    src->hist_flat_detector_enable;
  dest->start_level =
    src->start_level;
  dest->end_level =
    src->end_level;
  dest->range =
    src->range;
  dest->delta_th_low =
    src->delta_th_low;
  dest->delta_th_high =
    src->delta_th_high;
  dest->bright_flat_det_th =
    src->bright_flat_det_th;
  dest->dark_flat_det_th =
    src->dark_flat_det_th;
  dest->bright_flat_tail_det =
    src->bright_flat_tail_det;
  dest->dark_flat_tail_det =
    src->dark_flat_tail_det;
  dest->bright_flat_compensation_enable =
    src->bright_flat_compensation_enable;
  dest->dark_flat_compensation_enable =
    src->dark_flat_compensation_enable;
  dest->flat_white_grey_vs_nongrey_th =
    src->flat_white_grey_vs_nongrey_th;
  dest->flat_dark_grey_vs_nongrey_th =
    src->flat_dark_grey_vs_nongrey_th;
  dest->near_grey_tolerance =
    src->near_grey_tolerance;
  dest->bright_flat_compensation_ratio =
    src->bright_flat_compensation_ratio;
  dest->dark_flat_compensation_ratio =
    src->dark_flat_compensation_ratio;
  dest->flat_indoor_disable_start_idx =
    src->flat_indoor_disable_start_idx;
  dest->flat_indoor_disable_end_idx =
    src->flat_indoor_disable_end_idx;
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_hist_metering
  *=============================================================================*/
  {
  aec_hist_metering_adapter* dest = &tuning_adapter->aec_hist_metering;
  aec_hist_metering_type*    src  = &tuning_info->aec_hist_metering;
  aec_hdr_tuning_type*    src_aec_hdr  = &tuning_info->aec_hdr;
  aec_face_priority_adapter* dest_face_AEC = &tuning_adapter->aec_face_detection;

  dest->hist_metering_enable =
    src->hist_metering_enable;
  dest->adrc_enable =
    src->adrc_enable;
  dest->dark_end_control_enable =
    src->dark_end_control_enable;
  dest->num_zones =
    src->num_zones;
   /* Face weight trigger zones copy hist_target in 310*/
  dest_face_AEC->face_wt_zone.num_zones=
    src->num_zones;
  COPY_ARRAY(dest_face_AEC->face_wt_zone.triggers, src->triggers);

  COPY_ARRAY(dest->triggers, src->triggers);

  int i = 0;

  /* Shallow copy the triggered_params field. */
  for (i = 0; i < MAX_AEC_TRIGGER_ZONE; i++) {
    dest->triggered_params[i].max_target_adjust_ratio =
      src->triggered_params[i].max_target_adjust_ratio;
    dest->triggered_params[i].min_target_adjust_ratio =
      src->triggered_params[i].min_target_adjust_ratio;
    dest->triggered_params[i].sat_low_ref =
      src->triggered_params[i].sat_low_ref;
    dest->triggered_params[i].sat_high_ref =
      src->triggered_params[i].sat_high_ref;
    dest->triggered_params[i].dark_low_ref =
      src->triggered_params[i].dark_low_ref;
    dest->triggered_params[i].dark_high_ref =
      src->triggered_params[i].dark_high_ref;
    dest->triggered_params[i].max_drc_gain =
      src->triggered_params[i].max_drc_gain;
    dest->triggered_params[i].adrc_conv_speed =
      src->triggered_params[i].adrc_conv_speed;
    dest->triggered_params[i].gtm_percentage =
      src->triggered_params[i].gtm_percentage;
    dest->triggered_params[i].ltm_percentage =
      src->triggered_params[i].ltm_percentage;
    dest->triggered_params[i].la_percentage =
      src->triggered_params[i].la_percentage;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
    dest->triggered_params[i].gamma_percentage = 0;
#else
    dest->triggered_params[i].gamma_percentage =
      src->triggered_params[i].gamma_percentage;
#endif

  COPY_ARRAY(dest->triggered_params[i].hist_sat_pushup_range,
             src->triggered_params[i].hist_sat_pushup_range);

  COPY_ARRAY(dest->triggered_params[i].hist_sat_pushdown_range,
             src->triggered_params[i].hist_sat_pushdown_range);

  COPY_ARRAY(dest->triggered_params[i].hist_dark_range,
             src->triggered_params[i].hist_dark_range);
  }

  dest->target_filter_factor =
    src->target_filter_factor;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->gtm_enable          = (boolean)src->reserved[0];
  dest->ltm_enable          = (boolean)src->reserved[1];
  dest->la_enable           = (boolean)src->reserved[2];
  dest->gamma_enable        = (boolean)src->reserved[3];
  dest->dark_pushup_enable  = (boolean)src->reserved[4];
  dest->triggered_params[0].gamma_percentage = src->reserved[5];
  dest->triggered_params[1].gamma_percentage = src->reserved[6];
  dest->triggered_params[2].gamma_percentage = src->reserved[7];
  dest->triggered_params[3].gamma_percentage = src->reserved[8];
  dest->triggered_params[4].gamma_percentage = src->reserved[9];
  dest->triggered_params[5].gamma_percentage = src->reserved[10];

  /* Clamp ADRC with antibanding enabled */
  dest->adrc_atb_ctrl.adrc_antibanding_priority_enable = (boolean)src->reserved[11];
  dest->adrc_atb_ctrl.atb_cutoff_idx_50hz              = (int)src->reserved[12];
  dest->adrc_atb_ctrl.atb_cutoff_idx_60hz              = (int)src->reserved[13];
  dest->adrc_atb_ctrl.atb_trigger.start                = (int)src->reserved[14];
  dest->adrc_atb_ctrl.atb_trigger.end                  = (int)src->reserved[15];

  dest->clamp_sat_pos_enable = (int)src->reserved[16];
#else
  /* TODO: verify that these same reserved parameters are used for 310*/
  dest->gtm_enable          = (boolean)src->reserved[0];
  dest->ltm_enable          = (boolean)src->reserved[1];
  dest->la_enable           = (boolean)src->reserved[2];
  dest->gamma_enable        = (boolean)src->reserved[3];
  dest->dark_pushup_enable  = (boolean)src->reserved[4];

  /* Clamp ADRC with antibanding enabled */
  dest->adrc_atb_ctrl.adrc_antibanding_priority_enable = (boolean)src->reserved[11];
  dest->adrc_atb_ctrl.atb_cutoff_idx_50hz              = (int)src->reserved[12];
  dest->adrc_atb_ctrl.atb_cutoff_idx_60hz              = (int)src->reserved[13];
  dest->adrc_atb_ctrl.atb_trigger.start                = (int)src->reserved[14];
  dest->adrc_atb_ctrl.atb_trigger.end                  = (int)src->reserved[15];

  dest->clamp_sat_pos_enable = (int)src->reserved[16];
#endif

  /* Histogram AEC for touch ROI metering
     * In 309/310, borrow parameters from sensor HDR reserved field */

    dest->touch_roi_hist_adjust_enable =
      (boolean)src_aec_hdr->reserved[0];
    dest->face_roi_hist_adjust_enable =
      (boolean)src_aec_hdr->reserved[1];

    dest->dark_roi_max_target_adjust_ratio[0]   =
      dest->dark_roi_max_target_adjust_ratio[1] =
      dest->dark_roi_max_target_adjust_ratio[2] =
      dest->dark_roi_max_target_adjust_ratio[3] =
      dest->dark_roi_max_target_adjust_ratio[4] =
      dest->dark_roi_max_target_adjust_ratio[5] =
      src_aec_hdr->reserved[2];
    dest->dark_roi_min_target_adjust_ratio[0]   =
      dest->dark_roi_min_target_adjust_ratio[1] =
      dest->dark_roi_min_target_adjust_ratio[2] =
      dest->dark_roi_min_target_adjust_ratio[3] =
      dest->dark_roi_min_target_adjust_ratio[4] =
      dest->dark_roi_min_target_adjust_ratio[5] =
      src_aec_hdr->reserved[3];
    dest->dark_roi_dark_low_ref[0]   =
      dest->dark_roi_dark_low_ref[1] =
      dest->dark_roi_dark_low_ref[2] =
      dest->dark_roi_dark_low_ref[3] =
      dest->dark_roi_dark_low_ref[4] =
      dest->dark_roi_dark_low_ref[5] =
      src_aec_hdr->reserved[4];
    dest->dark_roi_sat_high_ref[0]   =
      dest->dark_roi_sat_high_ref[1] =
      dest->dark_roi_sat_high_ref[2] =
      dest->dark_roi_sat_high_ref[3] =
      dest->dark_roi_sat_high_ref[4] =
      dest->dark_roi_sat_high_ref[5] =
      src_aec_hdr->reserved[5];
    dest->bright_roi_max_target_adjust_ratio[0]   =
      dest->bright_roi_max_target_adjust_ratio[1] =
      dest->bright_roi_max_target_adjust_ratio[2] =
      dest->bright_roi_max_target_adjust_ratio[3] =
      dest->bright_roi_max_target_adjust_ratio[4] =
      dest->bright_roi_max_target_adjust_ratio[5] =
      src_aec_hdr->reserved[6];
    dest->bright_roi_min_target_adjust_ratio[0]   =
      dest->bright_roi_min_target_adjust_ratio[1] =
      dest->bright_roi_min_target_adjust_ratio[2] =
      dest->bright_roi_min_target_adjust_ratio[3] =
      dest->bright_roi_min_target_adjust_ratio[4] =
      dest->bright_roi_min_target_adjust_ratio[5] =
      src_aec_hdr->reserved[7];
    dest->bright_roi_dark_low_ref[0]   =
      dest->bright_roi_dark_low_ref[1] =
      dest->bright_roi_dark_low_ref[2] =
      dest->bright_roi_dark_low_ref[3] =
      dest->bright_roi_dark_low_ref[4] =
      dest->bright_roi_dark_low_ref[5] =
      src_aec_hdr->reserved[8];
    dest->bright_roi_sat_high_ref[0]   =
      dest->bright_roi_sat_high_ref[1] =
      dest->bright_roi_sat_high_ref[2] =
      dest->bright_roi_sat_high_ref[3] =
      dest->bright_roi_sat_high_ref[4] =
      dest->bright_roi_sat_high_ref[5] =
      src_aec_hdr->reserved[9];
    dest->dark_roi_luma_ratio_start[0]   =
      dest->dark_roi_luma_ratio_start[1] =
      dest->dark_roi_luma_ratio_start[2] =
      dest->dark_roi_luma_ratio_start[3] =
      dest->dark_roi_luma_ratio_start[4] =
      dest->dark_roi_luma_ratio_start[5] =
      src_aec_hdr->reserved[10];
    dest->dark_roi_luma_ratio_end[0]   =
      dest->dark_roi_luma_ratio_end[1] =
      dest->dark_roi_luma_ratio_end[2] =
      dest->dark_roi_luma_ratio_end[3] =
      dest->dark_roi_luma_ratio_end[4] =
      dest->dark_roi_luma_ratio_end[5] =
      src_aec_hdr->reserved[11];
    dest->bright_roi_luma_ratio_start[0]   =
      dest->bright_roi_luma_ratio_start[1] =
      dest->bright_roi_luma_ratio_start[2] =
      dest->bright_roi_luma_ratio_start[3] =
      dest->bright_roi_luma_ratio_start[4] =
      dest->bright_roi_luma_ratio_start[5] =
      src_aec_hdr->reserved[12];
    dest->bright_roi_luma_ratio_end[0]   =
      dest->bright_roi_luma_ratio_end[1] =
      dest->bright_roi_luma_ratio_end[2] =
      dest->bright_roi_luma_ratio_end[3] =
      dest->bright_roi_luma_ratio_end[4] =
      dest->bright_roi_luma_ratio_end[5] =
      src_aec_hdr->reserved[13];


  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_Y_hist_stretch
  *=============================================================================*/
  {
  aec_Y_hist_stretch_adapter* dest = &tuning_adapter->aec_Y_hist_stretch;
  aec_Y_hist_stretch_type*    src  = &tuning_info->aec_Y_hist_stretch;

  dest->enable =
    src->enable;
  dest->num_zones =
    src->num_zones;

  COPY_ARRAY(dest->triggers, src->triggers);

  COPY_ARRAY(dest->triggered_params, src->triggered_params);
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_hdr
  *=============================================================================*/
  {
  aec_hdr_tuning_adapter* dest = &tuning_adapter->aec_hdr;
  aec_hdr_tuning_type*    src  = &tuning_info->aec_hdr;

  dest->hdr_indoor_trigger.start =
    src->hdr_indoor_trigger.start;
  dest->hdr_indoor_trigger.end =
    src->hdr_indoor_trigger.end;
  /* Max HDR gain is tunable parameter.In case customer does not set any value this parameter
   * will be loaded with MAX_IDEAL_HDR_GAIN(i.e '8')
   */
  dest->maxIdealHDRgain =
    (float)((src->reserved[14] >= 1.0f) ? (src->reserved[14]) : (MAX_IDEAL_HDR_GAIN));
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_preview_motion_iso
  *=============================================================================*/
  {
  aec_motion_iso_adapter* dest = &tuning_adapter->aec_preview_motion_iso;
  aec_motion_iso_type*    src  = &tuning_info->aec_preview_motion_iso;

  dest->motion_iso_enable =
    src->motion_iso_enable;
  dest->motion_iso_aggressiveness =
    src->motion_iso_aggressiveness;
  dest->motion_iso_threshold =
    src->motion_iso_threshold;
  dest->motion_iso_max_gain =
    src->motion_iso_max_gain;
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_snapshot_motion_iso
  *=============================================================================*/
  {
  aec_motion_iso_adapter* dest = &tuning_adapter->aec_snapshot_motion_iso;
  aec_motion_iso_type*    src  = &tuning_info->aec_snapshot_motion_iso;

  dest->motion_iso_enable =
    src->motion_iso_enable;
  dest->motion_iso_aggressiveness =
    src->motion_iso_aggressiveness;
  dest->motion_iso_threshold =
    src->motion_iso_threshold;
  dest->motion_iso_max_gain =
    src->motion_iso_max_gain;
  }
 /*=============================================================================
  * aec_algo_tuning_adapter::aec_fast_aec
  *=============================================================================*/
  {
  aec_fast_aec_adapter* dest = &tuning_adapter->aec_fast_aec;
  aec_fast_aec_type*    src  = &tuning_info->aec_fast_aec;

  dest->start_index =
    src->start_index;
  dest->frame_skip =
    src->frame_skip;
  dest->luma_settled_count =
    src->luma_settled_count;
  }

  return true;
}


/*=============================================================================
 *  AEC Methods
 *=============================================================================*/

static boolean parse_AEC_chromatix_iot(void* iot_tuning_info,
  aec_algo_tuning_adapter* tuning_adapter)
{
  const chromatix_iot_parms_type* tuning_info = (chromatix_iot_parms_type*)iot_tuning_info;

  if (tuning_info == NULL)
  {
    AEC_ERR("error: tuning_info is NULL");
    return false;
  }

  if (tuning_adapter == NULL)
  {
    AEC_ERR("error: tuning_adapter is NULL");
    return false;
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_ir_mode
  *=============================================================================*/
  {
    aec_ir_mode_adapter* dest = &tuning_adapter->aec_ir_mode;
    const chromatix_ir_mode_type* src  = &tuning_info->irmode_data;

    dest->ir_mode_enable =
      src->ir_mode_enable;
    dest->ir_led_adj.ir_led_enable =
      src->ir_led_adj.ir_led_enable;
    dest->ir_led_adj.ir_led_intensity_unit =
      src->ir_led_adj.ir_led_intensity_unit;
    dest->ir_led_adj.ir_led_intensity_idx_max =
      src->ir_led_adj.ir_led_intensity_idx_max;
    dest->ir_led_adj.ir_led_intensity_idx_min =
      src->ir_led_adj.ir_led_intensity_idx_min;
    dest->ir_als_adj.ir_lightsensor_enable =
      src->ir_als_adj.ir_lightsensor_enable;
    dest->ir_als_adj.day2ir_switch_pt =
      src->ir_als_adj.day2ir_switch_pt;
    dest->ir_als_adj.ir2day_switch_pt =
      src->ir_als_adj.ir2day_switch_pt;

    COPY_FIELD(dest->ir_center_weight,
      src->ir_center_weight);
    COPY_FIELD(dest->ir_highlight_suppress_weight,
      src->ir_highlight_suppress_weight);

    dest->ir_mode_conv_speed =
      src->ir_mode_conv_speed;
    dest->day2ir_exp_index =
      src->day2ir_exp_index;
    dest->ir2day_exp_index =
      src->ir2day_exp_index;
    dest->ir_mode_day2ir_check_cnt =
      src->ir_mode_day2ir_check_cnt;
    dest->ir_mode_ir2day_check_cnt =
      src->ir_mode_ir2day_check_cnt;
    dest->ir_mode_skip_check_cnt =
      src->ir_mode_skip_check_cnt;
    dest->ir_mode_aec_skip_cnt =
      src->ir_mode_aec_skip_cnt;
    dest->ir_daylight_dist_adj.normal_light_cnt_thres =
      src->ir_daylight_dist_adj.normal_light_cnt_thres;
    dest->ir_daylight_dist_adj.normal_light_target_thres =
      src->ir_daylight_dist_adj.normal_light_target_thres;
    dest->ir_daylight_dist_adj.normal_light_GB_ratio_max =
      src->ir_daylight_dist_adj.normal_light_GB_ratio_max;
    dest->ir_daylight_dist_adj.normal_light_GB_ratio_min =
      src->ir_daylight_dist_adj.normal_light_GB_ratio_min;
    dest->ir_daylight_dist_adj.normal_light_GR_ratio_max =
      src->ir_daylight_dist_adj.normal_light_GR_ratio_max;
    dest->ir_daylight_dist_adj.normal_light_GR_ratio_min =
      src->ir_daylight_dist_adj.normal_light_GR_ratio_min;
    dest->ir_daylight_dist_adj.normal_light_RB_ratio_max =
      src->ir_daylight_dist_adj.normal_light_RB_ratio_max;
    dest->ir_daylight_dist_adj.normal_light_RB_ratio_min =
      src->ir_daylight_dist_adj.normal_light_RB_ratio_min;
    COPY_FIELD(dest->ir_daylight_dist_adj.ir_distribution_weight,
      src->ir_daylight_dist_adj.ir_distribution_weight);
  }

 /*=============================================================================
   * aec_algo_tuning_adapter::aec_shdr_mode
   *=============================================================================*/
  {
    aec_shdr_mode_adapter *dest = &tuning_adapter->aec_shdr_mode;
    const shdr_aec_tuning_type *src = &tuning_info->shdr_mode_data.shdr_aec_params;
    int i =0;

    dest->shdr_enable = src->shdr_enable;
    dest->num_lut_entries = src->num_lut_entries;
    dest->num_lowlight_lut_entries = src->num_lowlight_lut_entries;
    dest->pre_Bhist_inverse_map = src->pre_Bhist_inverse_map;
    dest->temporal_filter1 = src->temporal_filter1;
    dest->temporal_filter2 = src->temporal_filter2;

    for(i = 0; i < dest->num_lut_entries; i++){
      dest->exp_lut[i].exp_ratio = src->exp_lut[i].exp_ratio;
      dest->exp_lut[i].gtm_gamma = src->exp_lut[i].gtm_gamma;
      dest->exp_lut[i].exp_change_low_th = src->exp_lut[i].exp_change_low_th;
      dest->exp_lut[i].exp_change_up_th = src->exp_lut[i].exp_change_up_th;
      dest->exp_lut[i].non_linear_low_th = src->exp_lut[i].non_linear_low_th;
      dest->exp_lut[i].non_linear_hi_th = src->exp_lut[i].non_linear_hi_th;
    }

    for(i = 0; i < dest->num_lowlight_lut_entries; i++){
      dest->aec_svhdr_lowlight[i].lux_index =
        src->aec_svhdr_lowlight[i].lux_index;
      dest->aec_svhdr_lowlight[i].k =src->aec_svhdr_lowlight[i].k;
      dest->aec_svhdr_lowlight[i].exp_ratio_low_limit =
        src->aec_svhdr_lowlight[i].exp_ratio_low_limit;
      dest->aec_svhdr_lowlight[i].exp_ratio_up_limit =
        src->aec_svhdr_lowlight[i].exp_ratio_up_limit;
    }
  }

  return true;
}

/*=============================================================================
*  AWB Methods
*=============================================================================*/
boolean parse_awb_outdoor_special_adapter(
  awb_outdoor_special_adapter* dest,
  awb_outdoor_special_type* src) {

  int i=0;
  if (NULL == src || NULL == dest) {
    return false;
  }

  for (i = 0; i < DETECT_ZONE_NUM; i++) {
    dest->detection_zone[i].rg_center      = src->detection_zone[i].rg_center;
    dest->detection_zone[i].bg_center      = src->detection_zone[i].bg_center;
    dest->detection_zone[i].rg_radius_grid = src->detection_zone[i].rg_radius_grid;
    dest->detection_zone[i].bg_radius_grid = src->detection_zone[i].bg_radius_grid;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    dest->detection_zone[i].calibration_index = src->detection_zone[i].calibration_index;
#endif
  }

  dest->count_trigger.nstart              = src->count_trigger.nstart;
  dest->count_trigger.nend                = src->count_trigger.nend;
  dest->exp_index_trigger.gain_start      = src->exp_index_trigger.gain_start;
  dest->exp_index_trigger.gain_end        = src->exp_index_trigger.gain_end;
  dest->exp_index_trigger.lux_index_start = src->exp_index_trigger.lux_index_start;
  dest->exp_index_trigger.lux_index_end   = src->exp_index_trigger.lux_index_end;
  Q3A_MEMCPY_S(dest->aux_cond, sizeof(dest->aux_cond),
    src->aux_cond, sizeof(src->aux_cond));
  dest->adjust_ratio.red_gain_adj         = src->adjust_ratio.red_gain_adj;
  dest->adjust_ratio.blue_gain_adj        = src->adjust_ratio.blue_gain_adj;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  dest->exp_index_trigger_shade.gain_start = src->exp_index_trigger_shade.gain_start;
  dest->exp_index_trigger_shade.gain_end = src->exp_index_trigger_shade.gain_end;
  dest->exp_index_trigger_shade.lux_index_start = src->exp_index_trigger_shade.lux_index_start;
  dest->exp_index_trigger_shade.lux_index_end = src->exp_index_trigger_shade.lux_index_end;

  dest->adjust_ratio_shade.red_gain_adj = src->adjust_ratio_shade.red_gain_adj;
  dest->adjust_ratio_shade.blue_gain_adj = src->adjust_ratio_shade.blue_gain_adj;
#endif

  return true;
}


boolean parse_AWB_chromatix(void* awb_tuning_info, awb_algo_tuning_adapter* tuning_adapter) {
  awb_algo_struct_type* tuning_info = (awb_algo_struct_type*)awb_tuning_info;

  if (tuning_info == NULL) {
    AWB_ERR("error: tuning_info is NULL");
    return false;
  }

  if (tuning_adapter == NULL) {
    AWB_ERR("error: tuning_adapter is NULL");
    return false;
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_basic_tuning
  *=============================================================================*/
  {
    awb_basic_tuning_adapter* dest = &tuning_adapter->awb_basic_tuning;
    awb_basic_tuning_type*    src  = &tuning_info->awb_basic_tuning;

    COPY_ARRAY(dest->reference, src->reference);

    dest->indoor_index                = src->indoor_index;
    dest->outdoor_index               = src->outdoor_index;

    dest->awb_min_gains.b_gain        = src->awb_min_gains.b_gain;
    dest->awb_min_gains.g_gain        = src->awb_min_gains.g_gain;
    dest->awb_min_gains.r_gain        = src->awb_min_gains.r_gain;

    dest->awb_max_gains.b_gain        = src->awb_max_gains.b_gain;
    dest->awb_max_gains.g_gain        = src->awb_max_gains.g_gain;
    dest->awb_max_gains.r_gain        = src->awb_max_gains.r_gain;

    dest->awb_self_cal_enable         = src->awb_self_cal_enable;
    dest->awb_self_cal_adj_ratio_high = src->awb_self_cal_adj_ratio_high;
    dest->awb_self_cal_adj_ratio_low  = src->awb_self_cal_adj_ratio_low;

    dest->AWB_lowlight_LUT.enable = src->AWB_lowlight_LUT.enable;
    COPY_ARRAY(dest->AWB_lowlight_LUT.lut_entry, src->AWB_lowlight_LUT.lut_entry);

    dest->awb_adjust_gain_table.awb_interpolate_gain_adj_enable =
      src->awb_adjust_gain_table.awb_interpolate_gain_adj_enable;

    dest->awb_adjust_gain_table.exp_index_bright_light_gain_adjust.gain_start =
      src->awb_adjust_gain_table.exp_index_bright_light_gain_adjust.gain_start;
    dest->awb_adjust_gain_table.exp_index_bright_light_gain_adjust.gain_end =
      src->awb_adjust_gain_table.exp_index_bright_light_gain_adjust.gain_end;
    dest->awb_adjust_gain_table.exp_index_bright_light_gain_adjust.lux_index_start =
      src->awb_adjust_gain_table.exp_index_bright_light_gain_adjust.lux_index_start;
    dest->awb_adjust_gain_table.exp_index_bright_light_gain_adjust.lux_index_end =
      src->awb_adjust_gain_table.exp_index_bright_light_gain_adjust.lux_index_end;

    dest->awb_adjust_gain_table.exp_index_lowlight_gain_adjust.gain_start =
      src->awb_adjust_gain_table.exp_index_lowlight_gain_adjust.gain_start;
    dest->awb_adjust_gain_table.exp_index_lowlight_gain_adjust.gain_end =
      src->awb_adjust_gain_table.exp_index_lowlight_gain_adjust.gain_end;
    dest->awb_adjust_gain_table.exp_index_lowlight_gain_adjust.lux_index_start =
      src->awb_adjust_gain_table.exp_index_lowlight_gain_adjust.lux_index_start;
    dest->awb_adjust_gain_table.exp_index_lowlight_gain_adjust.lux_index_end =
      src->awb_adjust_gain_table.exp_index_lowlight_gain_adjust.lux_index_end;

    dest->awb_adjust_gain_table.control_gain_adj_lowlight =
      src->awb_adjust_gain_table.control_gain_adj_lowlight;

    COPY_ARRAY(dest->awb_adjust_gain_table.gain_adj_brightlight,
      src->awb_adjust_gain_table.gain_adj_brightlight);
    COPY_ARRAY(dest->awb_adjust_gain_table.gain_adj_normallight,
      src->awb_adjust_gain_table.gain_adj_normallight);
    COPY_ARRAY(dest->awb_adjust_gain_table.gain_adj_lowlight,
      src->awb_adjust_gain_table.gain_adj_lowlight);

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    dest->awb_adjust_gain_table.ext_adj_gain_enable = src->awb_adjust_gain_table.ext_adj_gain_enable;

    COPY_ARRAY(dest->awb_adjust_gain_table.exp_index_trigger,
      src->awb_adjust_gain_table.exp_index_trigger);
    COPY_ARRAY(dest->awb_adjust_gain_table.cct_trigger,
      src->awb_adjust_gain_table.cct_trigger);
    COPY_ARRAY(dest->awb_adjust_gain_table.gain_adj_brightlight_shade,
      src->awb_adjust_gain_table.gain_adj_brightlight_shade);
    COPY_ARRAY(dest->awb_adjust_gain_table.gain_adj_normallight_low,
      src->awb_adjust_gain_table.gain_adj_normallight_low);
    COPY_ARRAY(dest->awb_adjust_gain_table.gain_adj_extreme_lowlight,
      src->awb_adjust_gain_table.gain_adj_extreme_lowlight);

    COPY_ARRAY(dest->awb_adjust_gain_table.resv_dev_float1,
      src->awb_adjust_gain_table.resv_dev_float1);
    COPY_ARRAY(dest->awb_adjust_gain_table.resv_dev_float2,
      src->awb_adjust_gain_table.resv_dev_float2);

    dest->awb_adjust_gain_table.wb_adj_gain_compute_type = tuning_info->awb_reserved_data.reserved_int[0] == 2 ?
      tuning_info->awb_reserved_data.reserved_int[0] : dest->awb_adjust_gain_table.ext_adj_gain_enable;
#else
    dest->awb_adjust_gain_table.wb_adj_gain_compute_type = 0;
#endif

    dest->outlier_distances.outlier_distance =
      src->outlier_distances.outlier_distance;
    dest->outlier_distances.outlier_distance_day =
      src->outlier_distances.outlier_distance_day;
    dest->outlier_distances.outlier_distance_f =
      src->outlier_distances.outlier_distance_f;
    dest->outlier_distances.outlier_distance_a =
      src->outlier_distances.outlier_distance_a;
    dest->outlier_distances.outlier_distance_h =
      src->outlier_distances.outlier_distance_h;
    dest->outlier_distances.outlier_distance_customday =
      src->outlier_distances.outlier_distance_customday;
    dest->outlier_distances.outlier_distance_customf =
      src->outlier_distances.outlier_distance_customf;

    dest->outlier_distances.special_cluster_distances.awb_day_cluster_left_outlier_distance =
      src->outlier_distances.special_cluster_distances.awb_day_cluster_left_outlier_distance;
    dest->outlier_distances.special_cluster_distances.awb_day_cluster_top_outlier_distance =
      src->outlier_distances.special_cluster_distances.awb_day_cluster_top_outlier_distance;
    dest->outlier_distances.special_cluster_distances.awb_a_h_cluster_left_outlier_distance =
      src->outlier_distances.special_cluster_distances.awb_a_h_cluster_left_outlier_distance;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    dest->outlier_distances.special_cluster_distances.awb_day_cluster_left_outlier_distance =
      src->outlier_distances.special_cluster_distances.awb_day_cluster_left_outlier_distance;
#endif

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    COPY_FIELD(dest->outlier_distances_ext,
      src->outlier_distances_ext);
    dest->enable_awb_basic_init_cct                  = src->enable_awb_basic_init_cct;
    COPY_ARRAY(dest->initial_cct_est, src->initial_cct_est);
    COPY_ARRAY(dest->initial_weight_for_cct_est, src->initial_weight_for_cct_est);
    dest->awb_simulator_en                           = tuning_info->awb_reserved_data.reserved_int[199];
#else
    dest->outlier_distances_ext.outlier_distance_bright.special_cluster_distances.\
      ignore_day_cluster_left_top_outlier_distance   = tuning_info->awb_reserved_data.reserved_int[2];
    dest->initial_cct_est[0]                         = (int)tuning_info->awb_reserved_data.reserved_float[21];
    dest->initial_cct_est[1]                         = (int)tuning_info->awb_reserved_data.reserved_float[23];
    dest->initial_cct_est[2]                         = (int)tuning_info->awb_reserved_data.reserved_float[25];
    dest->initial_weight_for_cct_est[0]              = (int)tuning_info->awb_reserved_data.reserved_float[22];
    dest->initial_weight_for_cct_est[1]              = (int)tuning_info->awb_reserved_data.reserved_float[24];
    dest->initial_cct_est[0]                         = (int)tuning_info->awb_reserved_data.reserved_float[21];
#endif
    dest->tuning_version                             = tuning_info->awb_reserved_data.reserved_float[38];
    dest->requested_rgn_skip_pattern                 = tuning_info->awb_reserved_data.reserved_int[19];
    dest->sim_region_pix_cnt                         = tuning_info->awb_reserved_data.reserved_int[11];
    dest->fast_aec_forced_cnt                        = tuning_info->awb_reserved_data.reserved_int[49];
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_weight_vector
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_weight_vector, tuning_info->awb_weight_vector);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_green_zone_param
  *=============================================================================*/
  {
    awb_green_struct_adapter* dest = &tuning_adapter->awb_green_zone_param;
    awb_green_struct_type*    src  = &tuning_info->awb_green_zone_param;
    int i=0;

    dest->green_proj = src->green_proj;
    dest->green_offset_bg = src->green_offset_bg;
    dest->green_rg_offset_top_H = src->green_rg_offset_top_H;
    dest->green_rg_offset_bottom_H = src->green_rg_offset_bottom_H;
    dest->slope_factor_m = src->slope_factor_m;
    dest->slope_factor_m_shade = src->slope_factor_m_shade;
    dest->awb_cw_green_reject_threshold = src->awb_cw_green_reject_threshold;
    dest->awb_custom_fl_green_reject_threshold = src->awb_custom_fl_green_reject_threshold;
    dest->bright_green_percentage = src->bright_green_percentage;
    dest->dark_green_percentage = src->dark_green_percentage;
    dest->green_enhance_low_pec = src->green_enhance_low_pec;
    dest->green_enhance_high_pec = src->green_enhance_high_pec;
    dest->green2f_outlier_dist2 = src->green2f_outlier_dist2;

    dest->green_enhance_gain_adjust_high.blue_gain_adj =
      src->green_enhance_gain_adjust_high.blue_gain_adj;
    dest->green_enhance_gain_adjust_high.red_gain_adj =
      src->green_enhance_gain_adjust_high.red_gain_adj;

    dest->outdoor_heu_green_D50_enable =
      tuning_info->awb_reserved_data.reserved_int[28];
    dest->green_zone_A_bottom_enable =
      tuning_info->awb_reserved_data.reserved_int[29];
    dest->green_zone_bottom_grid =
      tuning_info->awb_reserved_data.reserved_int[30];
    dest->green_zone_left_offset_enable =
      tuning_info->awb_reserved_data.reserved_int[31];
    dest->green_zone_left_offset =
      tuning_info->awb_reserved_data.reserved_int[32];

    for (i = 0; i < GREEN_ZONE_ASSIST_NUM; i++) {
      dest->green_zone_assist[i].zone_control =
        src->green_zone_assist[i].zone_control;
      dest->green_zone_assist[i].trigger_type =
        src->green_zone_assist[i].trigger_type;
      dest->green_zone_assist[i].control_direction =
        src->green_zone_assist[i].control_direction;

      dest->green_zone_assist[i].detect_zone.rg_center =
        src->green_zone_assist[i].detect_zone.rg_center;
      dest->green_zone_assist[i].detect_zone.bg_center =
        src->green_zone_assist[i].detect_zone.bg_center;
      dest->green_zone_assist[i].detect_zone.rg_radius_grid =
        src->green_zone_assist[i].detect_zone.rg_radius_grid;
      dest->green_zone_assist[i].detect_zone.bg_radius_grid =
        src->green_zone_assist[i].detect_zone.bg_radius_grid;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
      dest->green_zone_assist[i].detect_zone.calibration_index =
        src->green_zone_assist[i].detect_zone.calibration_index;

#endif

      dest->green_zone_assist[i].exp_index_trigger.gain_start =
        src->green_zone_assist[i].exp_index_trigger.gain_start;
      dest->green_zone_assist[i].exp_index_trigger.gain_end =
        src->green_zone_assist[i].exp_index_trigger.gain_end;
      dest->green_zone_assist[i].exp_index_trigger.lux_index_start =
        src->green_zone_assist[i].exp_index_trigger.lux_index_start;
      dest->green_zone_assist[i].exp_index_trigger.lux_index_end =
        src->green_zone_assist[i].exp_index_trigger.lux_index_end;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
      dest->green_zone_assist[i].enable_mlc_zone =
        src->green_zone_assist[i].enable_mlc_zone;
#endif
    }

    dest->bright_green_exp_index.gain_start =
      src->bright_green_exp_index.gain_start;
    dest->bright_green_exp_index.gain_end =
      src->bright_green_exp_index.gain_end;
    dest->bright_green_exp_index.lux_index_start =
      src->bright_green_exp_index.lux_index_start;
    dest->bright_green_exp_index.lux_index_end =
      src->bright_green_exp_index.lux_index_end;

    dest->shade_green_exp_index.gain_start =
      src->shade_green_exp_index.gain_start;
    dest->shade_green_exp_index.gain_end =
      src->shade_green_exp_index.gain_end;
    dest->shade_green_exp_index.lux_index_start =
      src->shade_green_exp_index.lux_index_start;
    dest->shade_green_exp_index.lux_index_end =
      src->shade_green_exp_index.lux_index_end;

    dest->exp_weight = src->exp_weight;
    dest->green_proj_interp_enable = src->green_proj_interp_enable;
    dest->green_proj_per_low = src->green_proj_per_low;
    dest->green_proj_per_high = src->green_proj_per_high;
    dest->green_proj_per_max = src->green_proj_per_max;
    dest->green_proj_per_max_ratio = src->green_proj_per_max_ratio;
    dest->green_proj_index_start = src->green_proj_index_start;
    dest->green_proj_index_end = src->green_proj_index_end;
  }

  /*=============================================================================
    * awb_algo_tuning_adapter::awb_dark_threshold
    *=============================================================================*/
  {
    awb_dark_threshold_adapter* dest = &tuning_adapter->awb_dark_threshold;
    awb_dark_threshold_type*    src  = &tuning_info->awb_dark_threshold;

    dest->lowlight_measure_delta = src->lowlight_measure_delta;

    dest->dark_r_threshold = src->dark_r_threshold;
    dest->dark_g_threshold = src->dark_g_threshold;
    dest->dark_b_threshold = src->dark_b_threshold;

    dest->dark_r_threshold_lowlight = src->dark_r_threshold_lowlight;
    dest->dark_g_threshold_lowlight = src->dark_g_threshold_lowlight;
    dest->dark_b_threshold_lowlight = src->dark_b_threshold_lowlight;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    dest->dark_threshold_exp_trigger.gain_start =
      src->dark_threshold_exp_trigger.gain_start;
    dest->dark_threshold_exp_trigger.gain_end =
      src->dark_threshold_exp_trigger.gain_end;
    dest->dark_threshold_exp_trigger.lux_index_start =
      src->dark_threshold_exp_trigger.lux_index_start;
    dest->dark_threshold_exp_trigger.lux_index_end =
      src->dark_threshold_exp_trigger.lux_index_end;
#else
    dest->dark_threshold_exp_trigger.lux_index_start =
      tuning_info->awb_reserved_data.reserved_int[0];
    dest->dark_threshold_exp_trigger.lux_index_end =
      tuning_info->awb_reserved_data.reserved_int[1];
#endif
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_temporal_tuning
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_temporal_tuning, tuning_info->awb_temporal_tuning);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_heuristic_control
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_heuristic_control, tuning_info->awb_heuristic_control);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_heuristic_weight
  *=============================================================================*/
  {
    COPY_ARRAY(tuning_adapter->awb_heuristic_weight, tuning_info->awb_heuristic_weight);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_white_world
  *=============================================================================*/
  {
    awb_white_world_struct_adapter* dest = &tuning_adapter->awb_white_world;
    awb_white_world_struct_type*    src  = &tuning_info->awb_white_world;

    dest->white_stat_y_threshold_high      = src->white_stat_y_threshold_high;
    dest->white_outlier_valid_ymax_ratio   = src->white_outlier_valid_ymax_ratio;
    dest->white_stat_cnt_threshold         = src->white_stat_cnt_threshold;
    dest->white_peak_separation_distance   = src->white_peak_separation_distance;
    dest->white_aec_stable_range_threshold = src->white_aec_stable_range_threshold;
    dest->white_history_weight             = src->white_history_weight;
    dest->white_current_weight             = src->white_current_weight;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    dest->white_world_decision_enable = src->white_world_decision_enable;
    dest->white_world_lowlight_exp_offset = src->white_world_lowlight_exp_offset;
#else
    dest->white_world_decision_enable = tuning_info->awb_reserved_data.reserved_int[13];
    dest->white_world_lowlight_exp_offset = AWB_WHITEWORLD_DISABLE_LOWLIGHT;
#endif
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_decision_boundary
  *=============================================================================*/
  {
    awb_decision_boundary_adapter* dest = &tuning_adapter->awb_decision_boundary;
    awb_decision_boundary_type*    src = &tuning_info->awb_decision_boundary;

    dest->a_h_line_boundary = src->a_h_line_boundary;
    dest->day_f_line_boundary = src->day_f_line_boundary;
    dest->d75_d65_line_boundary = src->d75_d65_line_boundary;
    dest->d65_d50_line_boundary = src->d65_d50_line_boundary;
    dest->d65_d50_shifted_line_boundary = src->d65_d50_shifted_line_boundary;
    dest->f_a_line_boundary = src->f_a_line_boundary;
    dest->cw_tl84_line_boundary = src->cw_tl84_line_boundary;
    dest->d50_weighted_sample_boundary = src->d50_weighted_sample_boundary;
    dest->d65_weighted_sample_boundary = src->d65_weighted_sample_boundary;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    dest->h_cluster_cutoff_boundary.RG_ratio =
      src->h_cluster_cutoff_boundary.RG_ratio;
    dest->h_cluster_cutoff_boundary.BG_ratio =
      src->h_cluster_cutoff_boundary.BG_ratio;
#else
    dest->h_cluster_cutoff_boundary.RG_ratio =
      tuning_info->awb_reserved_data.reserved_float[12];
    dest->h_cluster_cutoff_boundary.BG_ratio =
      tuning_info->awb_reserved_data.reserved_float[13];
#endif
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_clusters_tuning
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_clusters_tuning, tuning_info->awb_clusters_tuning);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_weight_lowlight_warm
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_weight_lowlight_warm,
      tuning_info->awb_weight_lowlight_warm);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_CCM_control
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_CCM_control, tuning_info->awb_CCM_control);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_preFlash_param
  *=============================================================================*/
  {
    awb_preflash_adapter* dest = &tuning_adapter->awb_preFlash_param;
    awb_preflash_type*    src  = &tuning_info->awb_preFlash_param;


    dest->awb_pre_flash_enable                   = src->awb_pre_flash_enable;
    dest->awb_flash_stat_sat_threshold           = src->awb_flash_stat_sat_threshold;
    dest->awb_preflash_outlier_distance          = src->awb_preflash_outlier_distance;
    dest->awb_preflash_outlier_distance_no_flash = src->awb_preflash_outlier_distance_no_flash;

    dest->awb_mainflash_typ_ref.RG_ratio         = src->awb_mainflash_typ_ref.RG_ratio;
    dest->awb_mainflash_typ_ref.BG_ratio         = src->awb_mainflash_typ_ref.BG_ratio;

    dest->awb_preFlash_min_ref.RG_ratio          = src->awb_preFlash_min_ref.RG_ratio;
    dest->awb_preFlash_min_ref.BG_ratio          = src->awb_preFlash_min_ref.BG_ratio;

    dest->awb_preFlash_max_ref.RG_ratio          = src->awb_preFlash_max_ref.RG_ratio;
    dest->awb_preFlash_max_ref.BG_ratio          = src->awb_preFlash_max_ref.BG_ratio;

    dest->awb_mainflash_min_ref.RG_ratio         = src->awb_mainflash_min_ref.RG_ratio;
    dest->awb_mainflash_min_ref.BG_ratio         = src->awb_mainflash_min_ref.BG_ratio;

    dest->awb_mainflash_max_ref.RG_ratio         = src->awb_mainflash_max_ref.RG_ratio;
    dest->awb_mainflash_max_ref.BG_ratio         = src->awb_mainflash_max_ref.BG_ratio;

    dest->awb_flash_valid_sample_high_threshold  = src->awb_flash_valid_sample_high_threshold;
    dest->awb_flash_valid_sample_low_threshold   = src->awb_flash_valid_sample_low_threshold;

    dest->awb_flash_typ_decision_rg_offset.gain_start       = src->awb_flash_typ_decision_rg_offset.gain_start;
    dest->awb_flash_typ_decision_rg_offset.gain_end         = src->awb_flash_typ_decision_rg_offset.gain_end;
    dest->awb_flash_typ_decision_rg_offset.lux_index_start  = src->awb_flash_typ_decision_rg_offset.lux_index_start;
    dest->awb_flash_typ_decision_rg_offset.lux_index_end    = src->awb_flash_typ_decision_rg_offset.lux_index_end;

    dest->awb_flash_typ_decision_bg_offset.gain_start       = src->awb_flash_typ_decision_bg_offset.gain_start;
    dest->awb_flash_typ_decision_bg_offset.gain_end         = src->awb_flash_typ_decision_bg_offset.gain_end;
    dest->awb_flash_typ_decision_bg_offset.lux_index_start  = src->awb_flash_typ_decision_bg_offset.lux_index_start;
    dest->awb_flash_typ_decision_bg_offset.lux_index_end    = src->awb_flash_typ_decision_bg_offset.lux_index_end;

    dest->awb_flash_corner_sample_ref_adjust.red_gain_adj  = src->awb_flash_corner_sample_ref_adjust.red_gain_adj;
    dest->awb_flash_corner_sample_ref_adjust.blue_gain_adj = src->awb_flash_corner_sample_ref_adjust.blue_gain_adj;

    COPY_ARRAY(dest->awb_flash_lut, src->awb_flash_lut);

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    dest->awb_preflash_enhance_enable         = src->awb_preflash_enhance_enable;
    dest->awb_preflash_luma_difference        = src->awb_preflash_luma_difference;
    dest->awb_preflash_init_decision.RG_ratio = src->awb_preflash_init_decision.RG_ratio;
    dest->awb_preflash_init_decision.BG_ratio = src->awb_preflash_init_decision.BG_ratio;
    dest->awb_preflash_init_cct               = src->awb_preflash_init_cct;
#else
    dest->awb_preflash_enhance_enable         = 0;
    dest->awb_preflash_luma_difference        = tuning_info->awb_reserved_data.reserved_int[14];
    dest->awb_preflash_init_decision.RG_ratio = tuning_info->awb_reserved_data.reserved_float[0];
    dest->awb_preflash_init_decision.BG_ratio = tuning_info->awb_reserved_data.reserved_float[1];
    dest->awb_preflash_init_cct               = (uint32)tuning_info->awb_reserved_data.reserved_float[2];
#endif
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_led_tuning
  *=============================================================================*/
  {
    awb_led_tuning_adapter* dest = &tuning_adapter->awb_led_tuning;
    awb_led_tuning_type*    src  = &tuning_info->awb_led_tuning;
    int i = 0;

    dest->awb_led_strobe_adjustment_factor = src->awb_led_strobe_adjustment_factor;

    dest->LED_gain_adj.red_gain_adj        = src->LED_gain_adj.red_gain_adj;
    dest->LED_gain_adj.blue_gain_adj       = src->LED_gain_adj.blue_gain_adj;
    dest->strobe_gain_adj.red_gain_adj     = src->strobe_gain_adj.red_gain_adj;
    dest->strobe_gain_adj.blue_gain_adj    = src->strobe_gain_adj.blue_gain_adj;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    COPY_ARRAY(dest->dual_led_gain_adj, src->dual_led_gain_adj);
    /*ToDo_Wrapper - reserved_int[198] is currently not used for dual led*/
    dest->dual_led_enable                               = tuning_info->awb_reserved_data.reserved_int[198];
    dest->mix_LED_table.led1_gain_adj.blue_gain_adj     = tuning_info->awb_reserved_data.reserved_float[266];
    dest->mix_LED_table.led1_gain_adj.red_gain_adj      = tuning_info->awb_reserved_data.reserved_float[267];
    dest->mix_LED_table.led2_gain_adj.blue_gain_adj     = tuning_info->awb_reserved_data.reserved_float[268];
    dest->mix_LED_table.led2_gain_adj.red_gain_adj      = tuning_info->awb_reserved_data.reserved_float[269];

#else
    dest->dual_led_enable                               = src->mix_LED_table.dual_led_enable;
    dest->mix_LED_table.dual_led_enable                 = src->mix_LED_table.dual_led_enable;
    dest->mix_LED_table.preflash_current_ratio          = src->mix_LED_table.preflash_current_ratio;
    dest->mix_LED_table.continuous_burst_current_ratio  = src->mix_LED_table.continuous_burst_current_ratio;
    dest->mix_LED_table.led1_gain_adj.blue_gain_adj     = src->mix_LED_table.led1_gain_adj.blue_gain_adj;
    dest->mix_LED_table.led1_gain_adj.red_gain_adj      = src->mix_LED_table.led1_gain_adj.red_gain_adj;
    dest->mix_LED_table.led2_gain_adj.blue_gain_adj     = src->mix_LED_table.led2_gain_adj.blue_gain_adj;
    dest->mix_LED_table.led2_gain_adj.red_gain_adj      = src->mix_LED_table.led2_gain_adj.red_gain_adj;
    dest->mix_LED_table.table_size                      = src->mix_LED_table.table_size;
    dest->mix_LED_table.dual_led_intersect_slope        = tuning_info->awb_reserved_data.reserved_float[27];

    for (i = 0; i < MAX_LED_MIX_LEVEL; i++) {
      dest->mix_LED_table.CCT_control[i].LED1_setting    = src->mix_LED_table.CCT_control[i].LED1_setting;;
      dest->mix_LED_table.CCT_control[i].LED2_setting    = src->mix_LED_table.CCT_control[i].LED2_setting;;
      dest->mix_LED_table.CCT_control[i].rg_ratio        = src->mix_LED_table.CCT_control[i].rg_ratio;;
      dest->mix_LED_table.CCT_control[i].bg_ratio        = src->mix_LED_table.CCT_control[i].bg_ratio;;
      dest->mix_LED_table.CCT_control[i].flux            = src->mix_LED_table.CCT_control[i].flux;;
      dest->mix_LED_table.CCT_control[i].CCT             = src->mix_LED_table.CCT_control[i].CCT;;
      dest->mix_LED_table.CCT_control[i].preflash_flux   = src->mix_LED_table.CCT_control[i].preflash_flux;
      dest->mix_LED_table.CCT_control[i].cont_burst_flux = src->mix_LED_table.CCT_control[i].cont_burst_flux;
    }

    dest->mix_LED_table.range_low  = tuning_info->awb_reserved_data.reserved_float[45];
    dest->mix_LED_table.range_high = tuning_info->awb_reserved_data.reserved_float[46];
#endif
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_extended_outdoor_heuristic
  *=============================================================================*/
  {
    awb_extended_outdoor_heuristic_adapter* dest = &tuning_adapter->awb_extended_outdoor_heuristic;
    awb_extended_outdoor_heuristic_type*    src = &tuning_info->awb_extended_outdoor_heuristic;

    dest->awb_exposure_outdoor_heuristic = src->awb_exposure_outdoor_heuristic;
    parse_awb_outdoor_special_adapter(&dest->awb_outdoor_green_adjust,
      &src->awb_outdoor_green_adjust);
    parse_awb_outdoor_special_adapter(&dest->awb_outdoor_bright_blue_sky_adjust,
      &src->awb_outdoor_bright_blue_sky_adjust);
    parse_awb_outdoor_special_adapter(&dest->awb_outdoor_not_enough_stat_adjust,
      &src->awb_outdoor_not_enough_stat_adjust);
    parse_awb_outdoor_special_adapter(&dest->awb_outdoor_blue_ground_adjust,
      &src->awb_outdoor_blue_ground_adjust);
    parse_awb_outdoor_special_adapter(&dest->awb_outdoor_cloud_sky_adjust,
      &src->awb_outdoor_cloud_sky_adjust);
    parse_awb_outdoor_special_adapter(&dest->awb_outdoor_blue_sky_adjust,
      &src->awb_outdoor_blue_sky_adjust);
    parse_awb_outdoor_special_adapter(&dest->awb_outdoor_fog_sky_adjust,
      &src->awb_outdoor_fog_sky_adjust);
    parse_awb_outdoor_special_adapter(&dest->awb_outdoor_green_detect_adjust,
      &src->awb_outdoor_green_detect_adjust);

    dest->awb_outdoor_no_day_weighted_sample_adjust.red_gain_adj =
      src->awb_outdoor_no_day_weighted_sample_adjust.red_gain_adj;
    dest->awb_outdoor_no_day_weighted_sample_adjust.blue_gain_adj =
      src->awb_outdoor_no_day_weighted_sample_adjust.blue_gain_adj;
    dest->awb_outdoor_all_out_of_zone_adjust.red_gain_adj =
      src->awb_outdoor_all_out_of_zone_adjust.red_gain_adj;
    dest->awb_outdoor_all_out_of_zone_adjust.blue_gain_adj =
      src->awb_outdoor_all_out_of_zone_adjust.blue_gain_adj;

    dest->awb_extended_outdoor_exp_trigger.directsun_exp_index =
      src->awb_extended_outdoor_exp_trigger.directsun_exp_index;
    dest->awb_extended_outdoor_exp_trigger.outdoor_heuristic_exposure_index_trigger =
      src->awb_extended_outdoor_exp_trigger.outdoor_heuristic_exposure_index_trigger;
    dest->awb_extended_outdoor_exp_trigger.outdoor_exp_bright_shade_enable =
      src->awb_extended_outdoor_exp_trigger.outdoor_exp_bright_shade_enable;
    dest->awb_extended_outdoor_exp_trigger.outdoor_heuristic_exp_index_bright =
      src->awb_extended_outdoor_exp_trigger.outdoor_heuristic_exp_index_bright;
    dest->awb_extended_outdoor_exp_trigger.outdoor_heuristic_exp_index_shade =
      src->awb_extended_outdoor_exp_trigger.outdoor_heuristic_exp_index_shade;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    dest->enable_zone_select = src->enable_zone_select;
    dest->enable_artf_grass_adjust = src->enable_artf_grass_adjust;
    COPY_ARRAY(dest->awb_outdoor_normal_zone_select, src->awb_outdoor_normal_zone_select);
    COPY_ARRAY(dest->awb_outdoor_shade_zone_select, src->awb_outdoor_shade_zone_select);

    parse_awb_outdoor_special_adapter(&dest->awb_outdoor_artificial_grass_adjust,
      &src->awb_outdoor_artificial_grass_adjust);

    COPY_ARRAY(dest->resv_dev_float1, src->resv_dev_float1);
    COPY_ARRAY(dest->resv_dev_float2, src->resv_dev_float2);
#endif
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_single_color_tracking
  *=============================================================================*/
  {
    awb_single_color_tracking_adapter* dest = &tuning_adapter->awb_single_color_tracking;
    awb_single_color_tracking_type*    src  = &tuning_info->awb_single_color_tracking;
    int i=0, j=0;

    dest->awb_single_color_heuristic_enable = src->awb_single_color_heuristic_enable;

    for (i = 0; i < NUM_SPECIAL_COLOR_DETECT; i++) {
      for (j = 0; j < DETECT_ZONE_NUM; j++) {
        dest->awb_single_color_detect[i].detection_zone[j].rg_center =
          src->awb_single_color_detect[i].detection_zone[j].rg_center;
        dest->awb_single_color_detect[i].detection_zone[j].bg_center =
          src->awb_single_color_detect[i].detection_zone[j].bg_center;
        dest->awb_single_color_detect[i].detection_zone[j].rg_radius_grid =
          src->awb_single_color_detect[i].detection_zone[j].rg_radius_grid;
        dest->awb_single_color_detect[i].detection_zone[j].bg_radius_grid =
          src->awb_single_color_detect[i].detection_zone[j].bg_radius_grid;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
        dest->awb_single_color_detect[i].detection_zone[j].calibration_index =
          src->awb_single_color_detect[i].detection_zone[j].calibration_index;
#endif
      }

      dest->awb_single_color_detect[i].exp_index.gain_start =
        src->awb_single_color_detect[i].exp_index.gain_start;
      dest->awb_single_color_detect[i].exp_index.gain_end =
        src->awb_single_color_detect[i].exp_index.gain_end;
      dest->awb_single_color_detect[i].exp_index.lux_index_start =
        src->awb_single_color_detect[i].exp_index.lux_index_start;
      dest->awb_single_color_detect[i].exp_index.lux_index_end =
        src->awb_single_color_detect[i].exp_index.lux_index_end;

      dest->awb_single_color_detect[i].count_offset.nstart =
        src->awb_single_color_detect[i].count_offset.nstart;
      dest->awb_single_color_detect[i].count_offset.nend =
        src->awb_single_color_detect[i].count_offset.nend;

      dest->awb_single_color_detect[i].adjust_ratio.red_gain_adj =
        src->awb_single_color_detect[i].adjust_ratio.red_gain_adj;
      dest->awb_single_color_detect[i].adjust_ratio.blue_gain_adj =
        src->awb_single_color_detect[i].adjust_ratio.blue_gain_adj;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
      COPY_ARRAY(dest->awb_single_color_detect[i].enable_single_color_zone,
        src->awb_single_color_detect[i].enable_single_color_zone);
      dest->awb_single_color_detect[i].all_outlier_selection =
        src->awb_single_color_detect[i].all_outlier_selection;
      dest->awb_single_color_detect[i].trigger_type =
        src->awb_single_color_detect[i].trigger_type;
      dest->awb_single_color_detect[i].decision_for_ccm =
        src->awb_single_color_detect[i].decision_for_ccm;
#endif
    }

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    dest->awb_single_color_valid_zones = src->awb_single_color_valid_zones;
    dest->awb_single_color_transition_frame_count = src->awb_single_color_transition_frame_count;
    COPY_ARRAY(dest->resv_dev_float1, src->resv_dev_float1);
    COPY_ARRAY(dest->resv_dev_float2, src->resv_dev_float2);
#else
    dest->awb_single_color_valid_zones = tuning_info->awb_reserved_data.reserved_int[12];
    dest->bright_light_lux_idx_trigger = (float)tuning_info->awb_reserved_data.reserved_float[28];
    dest->low_light_lux_idx_trigger = (float)tuning_info->awb_reserved_data.reserved_float[29];
    dest->single_color_escape_counter_initial_value =
      (int)tuning_info->awb_reserved_data.reserved_float[30];
    dest->bright_light_escape_counter_decrease_number =
      (int)tuning_info->awb_reserved_data.reserved_float[31];
    dest->normal_light_escape_counter_decrease_number =
      (int)tuning_info->awb_reserved_data.reserved_float[32];
    dest->low_light_escape_counter_decrease_number =
      (int)tuning_info->awb_reserved_data.reserved_float[33];
    dest->fast_single_color_heuristic_converge_onoff =
      (boolean)tuning_info->awb_reserved_data.reserved_float[34];
#endif
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_misleading_color_zone
  *=============================================================================*/
  {
    awb_misleading_zone_process_adapter* dest = &tuning_adapter->awb_misleading_color_zone;
    awb_misleading_zone_process_type*    src  = &tuning_info->awb_misleading_color_zone;
    int i=0;

    dest->awb_day_sum_threshold_count.nstart = src->awb_day_sum_threshold_count.nstart;
    dest->awb_day_sum_threshold_count.nend   = src->awb_day_sum_threshold_count.nend;
    dest->awb_flo_sum_threshold_count.nstart = src->awb_flo_sum_threshold_count.nstart;
    dest->awb_flo_sum_threshold_count.nend   = src->awb_flo_sum_threshold_count.nend;

    for (i = 0; i < MISLEADING_COLOR_ZONE_NUM; i++) {
      dest->awb_misleading_color_zones[i].zone_control =
        src->awb_misleading_color_zones[i].zone_control;
      dest->awb_misleading_color_zones[i].trigger_type =
        src->awb_misleading_color_zones[i].trigger_type;
      dest->awb_misleading_color_zones[i].control_direction =
        src->awb_misleading_color_zones[i].control_direction;

      dest->awb_misleading_color_zones[i].detect_zone.rg_center =
        src->awb_misleading_color_zones[i].detect_zone.rg_center;
      dest->awb_misleading_color_zones[i].detect_zone.bg_center =
        src->awb_misleading_color_zones[i].detect_zone.bg_center;
      dest->awb_misleading_color_zones[i].detect_zone.rg_radius_grid =
        src->awb_misleading_color_zones[i].detect_zone.rg_radius_grid;
      dest->awb_misleading_color_zones[i].detect_zone.bg_radius_grid =
        src->awb_misleading_color_zones[i].detect_zone.bg_radius_grid;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
      dest->awb_misleading_color_zones[i].detect_zone.calibration_index =
        src->awb_misleading_color_zones[i].detect_zone.calibration_index;
#endif

      dest->awb_misleading_color_zones[i].exp_index_trigger.gain_start =
        src->awb_misleading_color_zones[i].exp_index_trigger.gain_start;
      dest->awb_misleading_color_zones[i].exp_index_trigger.gain_end =
        src->awb_misleading_color_zones[i].exp_index_trigger.gain_end;
      dest->awb_misleading_color_zones[i].exp_index_trigger.lux_index_start =
        src->awb_misleading_color_zones[i].exp_index_trigger.lux_index_start;
      dest->awb_misleading_color_zones[i].exp_index_trigger.lux_index_end =
        src->awb_misleading_color_zones[i].exp_index_trigger.lux_index_end;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
      dest->awb_misleading_color_zones[i].enable_mlc_zone =
        src->awb_misleading_color_zones[i].enable_mlc_zone;
#endif
    }

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    dest->awb_misleading_color_valid_zones = src->awb_misleading_color_valid_zones;
    COPY_ARRAY(dest->awb_misleading_sum_control_zones,
      src->awb_misleading_sum_control_zones);
    COPY_ARRAY(dest->resv_dev_float1, src->resv_dev_float1);
    COPY_ARRAY(dest->resv_dev_float2, src->resv_dev_float2);
#else
    dest->awb_misleading_color_valid_zones = tuning_info->awb_reserved_data.reserved_int[6];
#endif
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_golden_module_info
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_golden_module_info, tuning_info->awb_golden_module_info);
  }
  /*=============================================================================
  * awb_algo_tuning_adapter::awb_flow_control
  *=============================================================================*/
  {
    awb_flow_control_adapter* dest = &tuning_adapter->awb_flow_control;
    awb_flow_control_type*    src  = &tuning_info->awb_flow_control;

    dest->skip_frames                = src->skip_frames;
    dest->stat_saturation_threshold  = src->stat_saturation_threshold;
    dest->all_outlier_heuristic_flag = src->all_outlier_heuristic_flag;
    dest->valid_per_th               = src->valid_per_th;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    dest->enable_flow_control_grp1        = src->enable_flow_control_grp1;
    dest->trigger_exp_lowlight_skip_frame = src->trigger_exp_lowlight_skip_frame;
    dest->lowlight_skip_frame             = src->lowlight_skip_frame;
    dest->subsampling_factor              = src->subsampling_factor;
    dest->trigger_exp_to_lux_index        = src->trigger_exp_to_lux_index;
    dest->initial_cct                     = src->initial_cct;
    dest->initial_prev_cct                = src->initial_prev_cct;
    dest->awb_full_camif_enable           = src->awb_full_camif_enable;

    dest->initial_gain.r_gain             = src->initial_gain.r_gain;
    dest->initial_gain.g_gain             = src->initial_gain.g_gain;
    dest->initial_gain.b_gain             = src->initial_gain.b_gain;
#else
    dest->subsampling_factor              = (int)tuning_info->awb_reserved_data.reserved_int[17];
    dest->initial_gain.r_gain             = tuning_info->awb_reserved_data.reserved_float[16];
    dest->initial_gain.g_gain             = tuning_info->awb_reserved_data.reserved_float[17];
    dest->initial_gain.b_gain             = tuning_info->awb_reserved_data.reserved_float[18];
    dest->initial_cct                     = (int)tuning_info->awb_reserved_data.reserved_float[19];
    dest->trigger_exp_to_lux_index        = (int)tuning_info->awb_reserved_data.reserved_float[20];
    dest->initial_prev_cct                = (int)tuning_info->awb_reserved_data.reserved_float[26];
    dest->awb_full_camif_enable           = tuning_info->awb_reserved_data.reserved_int[18];
#endif
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_blue_tuning
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_blue_tuning, tuning_info->awb_blue_tuning);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_aux_sensor
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_aux_sensor, tuning_info->awb_aux_sensor);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_front_camera_control
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_front_camera_control,
      tuning_info->awb_front_camera_control);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_MWB
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_MWB, tuning_info->awb_MWB);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_dynamic_convergence
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_dynamic_convergence,
      tuning_info->awb_dynamic_convergence);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_gyro_assisted_convergence
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_gyro_assisted_convergence,
      tuning_info->awb_gyro_assisted_convergence);
  }


  /*=============================================================================
  * awb_algo_tuning_adapter::awb_special_weights_for_lab
  *=============================================================================*/
  {
    awb_special_weights_adapter* dest = &tuning_adapter->awb_special_weights_for_lab;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    awb_special_weights_type*    src  = &tuning_info->awb_special_weights_for_lab;

    dest->awb_sp_weights_enable       = src->awb_sp_weights_enable;
    dest->weight_day                  = src->weight_day;
    dest->weight_f                    = src->weight_f;
    dest->weight_a                    = src->weight_a;
    dest->weight_h                    = src->weight_h;
#else
    dest->weight_day                  = tuning_info->awb_reserved_data.reserved_int[7];
    dest->weight_f                    = tuning_info->awb_reserved_data.reserved_int[8];
    dest->weight_a                    = tuning_info->awb_reserved_data.reserved_int[9];
    dest->weight_h                    = tuning_info->awb_reserved_data.reserved_int[10];
#endif
  }

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  /*=============================================================================
  * awb_algo_tuning_adapter::awb_extreme_cct_control
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_extreme_cct_control,
      tuning_info->awb_extreme_cct_control);
  }

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_preflash_enhance_control
  *=============================================================================*/
  {
    COPY_FIELD(tuning_adapter->awb_preflash_enhance_control,
      tuning_info->awb_preflash_enhance_control);
  }
#endif
/*=============================================================================
* awb_algo_tuning_adapter::awb_dynamic_frame_skip
*=============================================================================*/
  {
    awb_dynamic_frame_skip_adapter* dest = &tuning_adapter->awb_dynamic_frame_skip;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    awb_dynamic_frame_skip_type*    src  = &tuning_info->awb_dynamic_frame_skip;

    dest->enable                      = src->enable;
    dest->high_skip_value             = src->high_skip_value;
    dest->low_frame_skip              = src->low_frame_skip;
    dest->min_frame_skip              = src->min_frame_skip;
    dest->scene_change_high_threshold = src->scene_change_high_threshold;
    dest->scene_change_low_threshold  = src->scene_change_low_threshold;
#else
    dest->enable                      = (int)tuning_info->awb_reserved_data.reserved_float[39];
    dest->high_skip_value             = (int)tuning_info->awb_reserved_data.reserved_float[40];
    dest->low_frame_skip              = (int)tuning_info->awb_reserved_data.reserved_float[41];
    dest->min_frame_skip              = (int)tuning_info->awb_reserved_data.reserved_float[42];
    dest->scene_change_high_threshold = tuning_info->awb_reserved_data.reserved_float[43];
    dest->scene_change_low_threshold  = tuning_info->awb_reserved_data.reserved_float[44];
#endif
  }

/*=============================================================================
* awb_algo_tuning_adapter::awb_correlation_frame_skip_type
*=============================================================================*/
  {
    awb_correlation_frame_skip_type* dest = &tuning_adapter->awb_correlation_frame_skip;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)

    dest->enable                      = (1.0f == tuning_info->awb_reserved_data.reserved_float[31]) ?
                                        TRUE : FALSE;
    dest->correlation_threshold       = tuning_info->awb_reserved_data.reserved_float[32];
    dest->skip_threshold              = tuning_info->awb_reserved_data.reserved_float[33];
#else
    dest->enable                      = (1.0f == tuning_info->awb_reserved_data.reserved_float[35]) ?
                                        TRUE : FALSE;
    dest->correlation_threshold       = tuning_info->awb_reserved_data.reserved_float[36];
    dest->skip_threshold              = (int)tuning_info->awb_reserved_data.reserved_float[37];
#endif
  }


#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  /*=============================================================================
* awb_algo_tuning_adapter::wb_refpnt_cali_data
*=============================================================================*/
  {
    WBReferencePointCaliData_adapter* dest = &tuning_adapter->wb_refpnt_cali_data;
    WBReferencePointCaliData_type*    src  = &tuning_info->wb_refpnt_cali_data;

    dest->wb_pnt_length = src->wb_pnt_length;;
    dest->wb_triad_length = src->wb_triad_length;;
    dest->wb_pnt_idx_rg = src->wb_pnt_idx_rg;;
    dest->wb_pnt_idx_bg = src->wb_pnt_idx_bg;;
    dest->wb_pnt_idx_wb = src->wb_pnt_idx_wb;;
    dest->wb_pnt_num_wb = src->wb_pnt_num_wb;;
    COPY_ARRAY(dest->wb_ref_pnt, src->wb_ref_pnt);
    dest->wb_ref_pnt_cnt = src->wb_ref_pnt_cnt;;
    dest->wb_ref_pnt_rg_min = src->wb_ref_pnt_rg_min;;
    dest->wb_ref_pnt_rg_max = src->wb_ref_pnt_rg_max;;
    dest->wb_ref_pnt_rg_triidxnum = src->wb_ref_pnt_rg_triidxnum;;
    COPY_ARRAY(dest->wb_ref_pnt_rg_triidx, src->wb_ref_pnt_rg_triidx);
    COPY_ARRAY(dest->wb_ref_pnt_triad, src->wb_ref_pnt_triad);
    dest->wb_ref_pnt_triad_cnt = src->wb_ref_pnt_triad_cnt;

    Q3A_MEMCPY_S(tuning_adapter->wb_refpnt_cali_data.wb_cal_factor,
      sizeof(tuning_adapter->wb_refpnt_cali_data.wb_cal_factor),
      &tuning_info->awb_reserved_data.reserved_float[168],
      2 * Q3A_AWB_AGW_MAX_LIGHT * sizeof(float));
  }
#endif

  /*=============================================================================
  * awb_algo_tuning_adapter::awb_algo_improvement_enable
  *=============================================================================*/
  {
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    tuning_adapter->awb_algo_improvement_enable = true;
#else
    tuning_adapter->awb_algo_improvement_enable = false;
#endif
  }

  /*=============================================================================
    * awb_algo_tuning_adapter::
    *=============================================================================*/
    {
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  int i = 0;
  int dualcam_param_base = 200;
  tuning_adapter->awb_dualcam_sync.enabled = (bool)tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 0];
  if(tuning_adapter->awb_dualcam_sync.enabled) {
    tuning_adapter->awb_dualcam_sync.sync_mode = (int)tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 1];
    tuning_adapter->awb_dualcam_sync.weight_trigger.lux_index_start = tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 2];
    tuning_adapter->awb_dualcam_sync.weight_trigger.lux_index_end = tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 3];
    tuning_adapter->awb_dualcam_sync.weight_trigger.gain_start= tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 4];
    tuning_adapter->awb_dualcam_sync.weight_trigger.gain_end= tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 5];
    tuning_adapter->awb_dualcam_sync.weight_fcm[0] = tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 6];
    tuning_adapter->awb_dualcam_sync.weight_fcm[1] = tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 7];
    tuning_adapter->awb_dualcam_sync.weight_fcm[2] = tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 8];
    tuning_adapter->awb_dualcam_sync.weight_fcm[3] = tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 9];
    tuning_adapter->awb_dualcam_sync.stats_dark_threshold = tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 10];
    tuning_adapter->awb_dualcam_sync.stats_sat_threshold = tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 11];
    tuning_adapter->awb_dualcam_sync.zoom_level_start = tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 12];
    tuning_adapter->awb_dualcam_sync.zoom_level_end = tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 13];
    tuning_adapter->awb_dualcam_sync.custom_reference_points.num_customer_refer_points = (int)tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 14];
    int start_index = dualcam_param_base + 15;
    for(i =0;i<tuning_adapter->awb_dualcam_sync.custom_reference_points.num_customer_refer_points;i++) {
      tuning_adapter->awb_dualcam_sync.custom_reference_points.calibration_index[i] = tuning_info->awb_reserved_data.reserved_float[start_index + i*3];
      tuning_adapter->awb_dualcam_sync.custom_reference_points.rg_table[i] = tuning_info->awb_reserved_data.reserved_float[start_index + i*3 +1];
      tuning_adapter->awb_dualcam_sync.custom_reference_points.bg_table[i] = tuning_info->awb_reserved_data.reserved_float[start_index + i*3 +2];
    }
    tuning_adapter->awb_dualcam_sync.sync_convergence_speed = tuning_info->awb_reserved_data.reserved_float[start_index + AWB_MAX_NUMBER_OF_CUSTOMER_REFERENCE_POINT*3];
    dualcam_param_base = start_index + AWB_MAX_NUMBER_OF_CUSTOMER_REFERENCE_POINT*3;
    tuning_adapter->awb_dualcam_sync.switch_ref_mode = tuning_info->awb_reserved_data.reserved_float[dualcam_param_base + 1];
  }else {
    tuning_adapter->awb_dualcam_sync.sync_mode = 2;
    tuning_adapter->awb_dualcam_sync.weight_trigger.lux_index_start = 240;
    tuning_adapter->awb_dualcam_sync.weight_trigger.lux_index_end = 340;
    tuning_adapter->awb_dualcam_sync.weight_trigger.gain_start= 0.1;
    tuning_adapter->awb_dualcam_sync.weight_trigger.gain_end= 0.2;
    tuning_adapter->awb_dualcam_sync.weight_fcm[0] = 1.0;
    tuning_adapter->awb_dualcam_sync.weight_fcm[1] = 0;
    tuning_adapter->awb_dualcam_sync.weight_fcm[2] = 1.0;
    tuning_adapter->awb_dualcam_sync.weight_fcm[3] = 1.0;
    tuning_adapter->awb_dualcam_sync.stats_dark_threshold = 1;
    tuning_adapter->awb_dualcam_sync.stats_sat_threshold = 230;
    tuning_adapter->awb_dualcam_sync.zoom_level_start = 1.9;
    tuning_adapter->awb_dualcam_sync.zoom_level_end = 4.0;
    tuning_adapter->awb_dualcam_sync.custom_reference_points.num_customer_refer_points = 0;
    tuning_adapter->awb_dualcam_sync.sync_convergence_speed = 0.5;
  }
#else
      tuning_adapter->custom_reference_points.num_customer_refer_points = 0;

#endif
    }


  /*=============================================================================
  * awb_algo_tuning_adapter::awb_face_awb
  *=============================================================================*/
  {
    int i = 0;

    awb_skin_control_adapter* dest = &tuning_adapter->awb_skin_control;
    awb_skin_control_type*    src = &tuning_info->awb_skin_control;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    dest->face_awb_enable = tuning_info->awb_skin_control.skin_ctrl_enable;
    dest->exclude_face_stats_enable = tuning_info->awb_reserved_data.reserved_int[1];
    dest->min_face_stats_count = tuning_info->awb_reserved_data.reserved_int[2];
    dest->min_frame_count = tuning_info->awb_reserved_data.reserved_int[20];
    dest->face_awb_stable_cnt_thd = tuning_info->awb_reserved_data.reserved_int[3];
    dest->face_awb_off_cnt_thd = tuning_info->awb_reserved_data.reserved_int[4];
    dest->face_stats_outlier_distance = tuning_info->awb_reserved_data.reserved_int[5];
    dest->face_awb_ext_stable_cnt_thd = tuning_info->awb_reserved_data.reserved_int[9];
    dest->faceawb_stability_improvement_cnt_thd = tuning_info->awb_reserved_data.reserved_int[21];
    dest->faceawb_stability_improvement_x_dev_thd = tuning_info->awb_reserved_data.reserved_int[22];
    dest->faceawb_stability_improvement_y_dev_thd = tuning_info->awb_reserved_data.reserved_int[23];
    dest->faceawb_stability_improvement_cnt_thd_for_gain = tuning_info->awb_reserved_data.reserved_int[24];
    dest->faceawb_stability_improvement_lock_cnt_thd = tuning_info->awb_reserved_data.reserved_int[25];
    dest->faceawb_roi_stability_improvement = tuning_info->awb_reserved_data.reserved_int[26];
    dest->faceawb_roi_stability_improvement_cnt_thd = tuning_info->awb_reserved_data.reserved_int[27];
    dest->faceawb_gain_stability_improvement = tuning_info->awb_reserved_data.reserved_int[41];

    dest->dark_skin_point_transfer_ratio_r = tuning_info->awb_reserved_data.reserved_float[40];
    dest->dark_skin_point_transfer_ratio_b = tuning_info->awb_reserved_data.reserved_float[41];

    COPY_FIELD(dest->skin_ref_exp_index_trigger,tuning_info->awb_skin_control.skin_ref_exp_index_trigger);
    COPY_ARRAY(dest->skin_reference, tuning_info->awb_skin_control.skin_reference);

    dest->light_wb_target_rg = tuning_info->awb_reserved_data.reserved_float[42];
    dest->light_wb_target_bg = tuning_info->awb_reserved_data.reserved_float[43];
    dest->dark_wb_target_rg = tuning_info->awb_reserved_data.reserved_float[44];
    dest->dark_wb_target_bg = tuning_info->awb_reserved_data.reserved_float[45];


    dest->roi_start_zone_0 = tuning_info->awb_reserved_data.reserved_float[46];
    dest->roi_end_zone_0 = tuning_info->awb_reserved_data.reserved_float[47];
    dest->roi_start_zone_1 = tuning_info->awb_reserved_data.reserved_float[48];
    dest->roi_end_zone_1 = tuning_info->awb_reserved_data.reserved_float[49];
    dest->min_weight_zone_0 = tuning_info->awb_reserved_data.reserved_float[50];
    dest->min_weight_zone_1 = tuning_info->awb_reserved_data.reserved_float[51];
    dest->max_weight = tuning_info->awb_reserved_data.reserved_float[52];

    dest->skin_dark_threshold_r = tuning_info->awb_reserved_data.reserved_float[53];
    dest->skin_dark_threshold_g = tuning_info->awb_reserved_data.reserved_float[54];
    dest->skin_dark_threshold_b = tuning_info->awb_reserved_data.reserved_float[55];
    dest->light_skin_weight_of_dist_confidence_ratio = tuning_info->awb_reserved_data.reserved_float[56];
    dest->light_skin_weight_of_cct_confidence_ratio = tuning_info->awb_reserved_data.reserved_float[57];


    dest->light_skin_cct_high_thd = tuning_info->awb_reserved_data.reserved_float[58];
    dest->light_skin_cct_low_thd = tuning_info->awb_reserved_data.reserved_float[59];
    dest->dark_skin_cct_high_thd = tuning_info->awb_reserved_data.reserved_float[60];
    dest->dark_skin_cct_low_thd = tuning_info->awb_reserved_data.reserved_float[61];

    dest->light_skin_cct_cr_high_cct_high_lux_index = tuning_info->awb_reserved_data.reserved_float[62];
    dest->light_skin_cct_cr_high_cct_low_lux_index = tuning_info->awb_reserved_data.reserved_float[63];
    dest->light_skin_cct_cr_low_cct_high_lux_index = tuning_info->awb_reserved_data.reserved_float[64];
    dest->light_skin_cct_cr_low_cct_low_lux_index = tuning_info->awb_reserved_data.reserved_float[65];

    dest->dark_skin_cct_cr_high_cct_high_lux_index = tuning_info->awb_reserved_data.reserved_float[66];
    dest->dark_skin_cct_cr_high_cct_low_lux_index = tuning_info->awb_reserved_data.reserved_float[67];
    dest->dark_skin_cct_cr_low_cct_high_lux_index = tuning_info->awb_reserved_data.reserved_float[68];
    dest->dark_skin_cct_cr_low_cct_low_lux_index = tuning_info->awb_reserved_data.reserved_float[69];

    dest->faceawb_convergence_speed = tuning_info->awb_reserved_data.reserved_float[70];

    dest->x_roi_divide = tuning_info->awb_reserved_data.reserved_float[71];
    dest->y_roi_divide = tuning_info->awb_reserved_data.reserved_float[72];

    dest->face_awb_exp_index_bright_light_gain_adjust_start = tuning_info->awb_reserved_data.reserved_float[73];
    dest->face_awb_exp_index_bright_light_gain_adjust_end = tuning_info->awb_reserved_data.reserved_float[74];
    dest->face_awb_exp_index_lowlight_gain_adjust_start = tuning_info->awb_reserved_data.reserved_float[75];
    dest->face_awb_exp_index_lowlight_gain_adjust_end = tuning_info->awb_reserved_data.reserved_float[76];


    for (i = 0; i < FACE_AWB_GAIN_ADJUST_MAX_NUMBER; i++) {
      dest->skin_gain_adjust_r_gain_bright_light[i] = tuning_info->awb_reserved_data.reserved_float[77 + i * 2];
      dest->skin_gain_adjust_b_gain_bright_light[i] = tuning_info->awb_reserved_data.reserved_float[78 + i * 2];
    }


    for (i = 0; i < FACE_AWB_GAIN_ADJUST_MAX_NUMBER; i++) {
      dest->skin_gain_adjust_r_gain_normal_light[i] = tuning_info->awb_reserved_data.reserved_float[91 + i * 2];
      dest->skin_gain_adjust_b_gain_normal_light[i] = tuning_info->awb_reserved_data.reserved_float[92 + i * 2];
    }

    for (i = 0; i < FACE_AWB_GAIN_ADJUST_MAX_NUMBER; i++) {
      dest->skin_gain_adjust_r_gain_low_light[i] = tuning_info->awb_reserved_data.reserved_float[105 + i * 2];
      dest->skin_gain_adjust_b_gain_low_light[i] = tuning_info->awb_reserved_data.reserved_float[106 + i * 2];
    }

    dest->faceawb_stability_improvement_r_gain_dev_thd = tuning_info->awb_reserved_data.reserved_float[129];
    dest->faceawb_stability_improvement_b_gain_dev_thd = tuning_info->awb_reserved_data.reserved_float[130];

#else
    dest->face_awb_enable =             tuning_info->awb_skin_control.skin_ctrl_enable;
    dest->exclude_face_stats_enable =   tuning_info->awb_reserved_data.reserved_int[4];
    dest->min_face_stats_count =        tuning_info->awb_reserved_data.reserved_int[5];
    dest->min_frame_count =             (int)(tuning_info->awb_reserved_data.reserved_int[15]/2.0f);
    dest->face_awb_stable_cnt_thd =     tuning_info->awb_reserved_data.reserved_int[15];
    dest->face_awb_off_cnt_thd =        tuning_info->awb_reserved_data.reserved_int[16];
    dest->face_stats_outlier_distance = tuning_info->awb_reserved_data.reserved_int[3];
    dest->face_awb_ext_stable_cnt_thd = 0;
    dest->faceawb_roi_stability_improvement = 0;
    dest->faceawb_gain_stability_improvement = 0;

    dest->dark_skin_point_transfer_ratio_r = tuning_info->awb_reserved_data.reserved_float[3];
    dest->dark_skin_point_transfer_ratio_b = tuning_info->awb_reserved_data.reserved_float[4];

    COPY_FIELD(dest->skin_ref_exp_index_trigger, tuning_info->awb_skin_control.skin_ref_exp_index_trigger);
    COPY_ARRAY(dest->skin_reference, tuning_info->awb_skin_control.skin_reference);

    dest->light_wb_target_rg = tuning_info->awb_reserved_data.reserved_float[47];
    dest->light_wb_target_bg = tuning_info->awb_reserved_data.reserved_float[48];
    dest->dark_wb_target_rg =  tuning_info->awb_reserved_data.reserved_float[14];
    dest->dark_wb_target_bg =  tuning_info->awb_reserved_data.reserved_float[15];

    dest->roi_start_zone_0 = tuning_info->awb_reserved_data.reserved_float[5];
    dest->roi_end_zone_0 = tuning_info->awb_reserved_data.reserved_float[6];
    dest->roi_start_zone_1 = tuning_info->awb_reserved_data.reserved_float[7];
    dest->roi_end_zone_1 = tuning_info->awb_reserved_data.reserved_float[8];
    dest->min_weight_zone_0 =  tuning_info->awb_reserved_data.reserved_float[9];
    dest->min_weight_zone_1 =  tuning_info->awb_reserved_data.reserved_float[10];
    dest->max_weight =  MAX_WEIGHT;
    dest->skin_dark_threshold_r =  tuning_info->awb_reserved_data.reserved_int[19];
    dest->skin_dark_threshold_g =  tuning_info->awb_reserved_data.reserved_int[19];
    dest->skin_dark_threshold_b =  tuning_info->awb_reserved_data.reserved_int[19];
    dest->light_skin_weight_of_dist_confidence_ratio = tuning_info->awb_reserved_data.reserved_float[11];
    dest->light_skin_weight_of_cct_confidence_ratio =  tuning_info->awb_reserved_data.reserved_float[49];

    dest->dark_skin_cct_high_thd =  DARK_SKIN_CCT_HIGH_THD;
    dest->dark_skin_cct_low_thd =   DARK_SKIN_CCT_LOW_THD;
    dest->light_skin_cct_high_thd = LIGHT_SKIN_CCT_HIGH_THD;
    dest->light_skin_cct_low_thd =  LIGHT_SKIN_CCT_LOW_THD;
    dest->light_skin_cct_cr_high_cct_high_lux_index = LIGHT_SKIN_CCT_CONFIDENCE_RATIO_OF_HIGH_CCT_HIGH_LUX_INDEX;
    dest->light_skin_cct_cr_high_cct_low_lux_index =	LIGHT_SKIN_CCT_CONFIDENCE_RATIO_OF_HIGH_CCT_LOW_LUX_INDEX;
    dest->light_skin_cct_cr_low_cct_high_lux_index =	LIGHT_SKIN_CCT_CONFIDENCE_RATIO_OF_LOW_CCT_HIGH_LUX_INDEX;
    dest->light_skin_cct_cr_low_cct_low_lux_index =	  LIGHT_SKIN_CCT_CONFIDENCE_RATIO_OF_LOW_CCT_LOW_LUX_INDEX;

    dest->dark_skin_cct_cr_high_cct_high_lux_index = (1.0f-DARK_SKIN_CCT_CONFIDENCE_RATIO_OF_HIGH_CCT_HIGH_LUX_INDEX);
    dest->dark_skin_cct_cr_high_cct_low_lux_index =  (1.0f-DARK_SKIN_CCT_CONFIDENCE_RATIO_OF_HIGH_CCT_LOW_LUX_INDEX);
    dest->dark_skin_cct_cr_low_cct_high_lux_index =  (1.0f-DARK_SKIN_CCT_CONFIDENCE_RATIO_OF_LOW_CCT_HIGH_LUX_INDEX);
    dest->dark_skin_cct_cr_low_cct_low_lux_index =   (1.0f-DARK_SKIN_CCT_CONFIDENCE_RATIO_OF_LOW_CCT_LOW_LUX_INDEX);

    dest->faceawb_convergence_speed = (float)((float)tuning_info->awb_reserved_data.reserved_int[18]/256.0f);

    dest->x_roi_divide = X_ROI_DIVIDE;
    dest->y_roi_divide = Y_ROI_DIVIDE;

    dest->face_awb_exp_index_bright_light_gain_adjust_start = GAIN_ADJ_BRIGHT_LIGHT_START;
    dest->face_awb_exp_index_bright_light_gain_adjust_end =  GAIN_ADJ_BRIGHT_LIGHT_END;
    dest->face_awb_exp_index_lowlight_gain_adjust_start =  GAIN_ADJ_LOWLIGHT_START;
    dest->face_awb_exp_index_lowlight_gain_adjust_end = GAIN_ADJ_LOWLIGHT_END;

    for (i = 0; i < FACE_AWB_GAIN_ADJUST_MAX_NUMBER; i++) {
      dest->skin_gain_adjust_r_gain_bright_light[i] = 1.0f;
      dest->skin_gain_adjust_b_gain_bright_light[i] = 1.0f;
    }
    for (i = 0; i < FACE_AWB_GAIN_ADJUST_MAX_NUMBER; i++) {
      dest->skin_gain_adjust_r_gain_normal_light[i] = 1.0f;
      dest->skin_gain_adjust_b_gain_normal_light[i] = 1.0f;
    }
    for (i = 0; i < FACE_AWB_GAIN_ADJUST_MAX_NUMBER; i++) {
      dest->skin_gain_adjust_r_gain_low_light[i] = 1.0f;
      dest->skin_gain_adjust_b_gain_low_light[i] = 1.0f;
    }
#endif

  }

  /*=============================================================================
  * awb_algo_tuning_adapter::
  *=============================================================================*/
  {
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
     awb_reserved_data_type* dest = &tuning_adapter->awb_reserved_data;
     awb_reserved_data_type* src = &tuning_info->awb_reserved_data;
     int i = 0;

     for (i = 0; i < AWB_RESERVED_PARAM - AWB_RESERVED_INT_OFFSET; i++) {
        dest->reserved_int[i] = src->reserved_int[i + AWB_RESERVED_INT_OFFSET];
     }
     for (i = 0; i < AWB_RESERVED_PARAM - AWB_RESERVED_FLOAT_OFFSET; i++) {
        dest->reserved_float[i] = src->reserved_float[i + AWB_RESERVED_FLOAT_OFFSET];
     }
#endif
  }

  return true;
}
/*=============================================================================
*  AFD Methods
*=============================================================================*/
boolean parse_AFD_chromatix(void* afd_tuning_info, afd_algo_tuning_adapter* tuning_adapter)
{
  chromatix_auto_flicker_detection_data_type *tuning = (chromatix_auto_flicker_detection_data_type*)afd_tuning_info;

  tuning_adapter->AFD_continuous_enable = tuning->AFD_continuous_enable;
  tuning_adapter->std_threshold = tuning->std_threshold;
  tuning_adapter->percent_threshold = tuning->percent_threshold;
  tuning_adapter->diff_threshold = tuning->diff_threshold;
  tuning_adapter->frame_ct_threshold = tuning->frame_ct_threshold;
  tuning_adapter->num_frames = tuning->num_frames;
  tuning_adapter->frame_skip = tuning->frame_skip;
  tuning_adapter->num_rows = tuning->num_rows;
  tuning_adapter->num_frames_settle = tuning->num_frames_settle;
  tuning_adapter->num_peaks_threshold = tuning->num_peaks_threshold;
  tuning_adapter->INTL_adj_factor = tuning->INTL_adj_factor;
  tuning_adapter->start_antbanding_table = tuning->start_antbanding_table;
  tuning_adapter->max_exp_time_ms = tuning->max_exp_time_ms;
  tuning_adapter->thldLevel2Ratio = tuning->thldLevel2Ratio;
  tuning_adapter->thldLevel3Ratio = tuning->thldLevel3Ratio;
  tuning_adapter->thldEkL = tuning->thldEkL;
  tuning_adapter->thldEkU = tuning->thldEkU;
  tuning_adapter->thldCounterL3 = tuning->thldCounterL3;
  tuning_adapter->thldCounterL2 = tuning->thldCounterL2;
  tuning_adapter->LevelDepth = tuning->LevelDepth;

  return TRUE;
}
/*=============================================================================
*  ASD Methods
*=============================================================================*/
boolean parse_ASD_chromatix(void* asd_tuning_info, asd_algo_tuning_adapter* tuning_adapter)
{
  AAA_ASD_struct_type *tuning = (AAA_ASD_struct_type*)asd_tuning_info;

  COPY_FIELD(tuning_adapter->snow_scene_detect, tuning->snow_scene_detect);
  COPY_FIELD(tuning_adapter->backlit_scene_detect, tuning->backlit_scene_detect);

  COPY_FIELD(tuning_adapter->landscape_scene_detect, tuning->landscape_scene_detect);
  COPY_FIELD(tuning_adapter->portrait_scene_detect, tuning->portrait_scene_detect);

  COPY_FIELD(tuning_adapter->auto_hdr_detect, tuning->auto_hdr_detect);

  COPY_FIELD(tuning_adapter->hazy_scene_detect, tuning->hazy_scene_detect);

  return true;
}

/*=============================================================================
*  IS Methods
*=============================================================================*/
boolean parse_IS_chromatix(void* is_tuning_info, is_algo_tuning_adapter* tuning_adapter)
{
  Chromatix_EIS_algo_type *chromatix_EIS = (Chromatix_EIS_algo_type*)is_tuning_info;

  tuning_adapter->eis_enable = chromatix_EIS->enable;
  tuning_adapter->focal_length = chromatix_EIS->focal_length;
  tuning_adapter->gyro_pixel_scale = chromatix_EIS->gyro_pixel_scale;
  tuning_adapter->virtual_margin = chromatix_EIS->virtual_margin;
  tuning_adapter->gyro_noise_floor = chromatix_EIS->gyro_noise_floor;
  tuning_adapter->gyro_frequency = chromatix_EIS->gyro_frequency;
  tuning_adapter->rs_offset_1 = chromatix_EIS->rs_offset_1;
  tuning_adapter->rs_offset_2 = chromatix_EIS->rs_offset_2;
  tuning_adapter->rs_offset_3 = chromatix_EIS->rs_offset_3;
  tuning_adapter->rs_offset_4 = chromatix_EIS->rs_offset_4;
  tuning_adapter->s3d_offset_1 = chromatix_EIS->s3d_offset_1;
  tuning_adapter->s3d_offset_2 = chromatix_EIS->s3d_offset_2;
  tuning_adapter->s3d_offset_3 = chromatix_EIS->s3d_offset_3;
  tuning_adapter->s3d_offset_4 = chromatix_EIS->s3d_offset_4;
  tuning_adapter->rs_threshold_1 = chromatix_EIS->rs_threshold_1;
  tuning_adapter->rs_threshold_2 = chromatix_EIS->rs_threshold_2;
  tuning_adapter->rs_threshold_3 = chromatix_EIS->rs_threshold_3;
  tuning_adapter->s3d_threshold_1 = chromatix_EIS->s3d_threshold_1;
  tuning_adapter->s3d_threshold_2 = chromatix_EIS->s3d_threshold_2;
  tuning_adapter->s3d_threshold_3 = chromatix_EIS->s3d_threshold_3;
  tuning_adapter->rs_time_interval_1 = chromatix_EIS->rs_time_interval_1;
  tuning_adapter->rs_time_interval_2 = chromatix_EIS->rs_time_interval_2;
  tuning_adapter->rs_time_interval_3 = chromatix_EIS->rs_time_interval_3;
  tuning_adapter->rs_time_interval_4 = chromatix_EIS->rs_time_interval_4;
  /* TODO: Replace the reserve with actual names */
  tuning_adapter->reserve_1 = chromatix_EIS->reserve_1;
  tuning_adapter->reserve_2 = chromatix_EIS->reserve_2;
  tuning_adapter->reserve_3 = chromatix_EIS->reserve_3;
  tuning_adapter->reserve_4 = chromatix_EIS->reserve_4;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  tuning_adapter->rs_interval_offset = chromatix_EIS->reserved_int[0];
  tuning_adapter->s3d_interval_offset = chromatix_EIS->reserved_int[1];
#else
  tuning_adapter->rs_interval_offset = 2500;
  tuning_adapter->s3d_interval_offset = 2500;
#endif

  return true;
}

boolean parse_chromatix(void* chromatix_ptr, chromatix_3a_parms_wrapper* chromatix_wrapper)
{
  chromatix_3a_parms_type* chromatix = (chromatix_3a_parms_type*)chromatix_ptr;
  boolean result = false;

  if (chromatix_ptr == NULL)
  {
    STATS_ERR("error: chromatix_ptr is NULL");
    return result;
  }

  if (chromatix_wrapper == NULL)
  {
    STATS_ERR("error: chromatix_wrapper is NULL");
    return result;
  }

  result = true;

  result &= parse_AF_chromatix((void*)&chromatix->AF_algo_data, &chromatix_wrapper->AF);
  result &= parse_AEC_chromatix((void*)&chromatix->AEC_algo_data, &chromatix_wrapper->AEC);
  result &= parse_AWB_chromatix((void*)&chromatix->AWB_bayer_algo_data, &chromatix_wrapper->AWB);
  result &= parse_AFD_chromatix((void*)&chromatix->auto_flicker_detection_data, &chromatix_wrapper->AFD);
  result &= parse_ASD_chromatix((void*)&chromatix->ASD_3A_algo_data, &chromatix_wrapper->ASD);
  result &= parse_IS_chromatix((void*)&chromatix->chromatix_EIS_data, &chromatix_wrapper->IS);

  /*TODO: temporary copies - remove when the wrapper for the rest of 3A is implemented*/
  chromatix_wrapper->chromatix_version_info = chromatix->chromatix_version_info;
  chromatix_wrapper->aaa_version            = chromatix->aaa_version;
  chromatix_wrapper->aaa_sync_data          = chromatix->aaa_sync_data;

  return result;
}

boolean parse_chromatix_iot(void* chromatix_ptr, chromatix_3a_parms_wrapper* chromatix_wrapper)
{
  chromatix_iot_parms_type* chromatix_iot = (chromatix_iot_parms_type*)chromatix_ptr;
  boolean result = false;

  if (chromatix_ptr == NULL)
  {
    STATS_ERR("error: chromatix_ptr is NULL");
    return result;
  }

  if (chromatix_wrapper == NULL)
  {
    STATS_ERR("error: chromatix_wrapper is NULL");
    return result;
  }

  result &= parse_AEC_chromatix_iot((void*)chromatix_iot, &chromatix_wrapper->AEC);

  return result;
}

boolean parse_chromatix_vfe(void* chromatix_ptr, chromatix_3a_parms_wrapper* chromatix_wrapper)
{
  chromatix_parms_type* chromatix = (chromatix_parms_type*)chromatix_ptr;
  boolean result = false;

  if (chromatix_ptr == NULL)
  {
    STATS_ERR("error: chromatix_ptr is NULL");
    return result;
  }

  if (chromatix_wrapper == NULL)
  {
    STATS_ERR("error: chromatix_wrapper is NULL");
    return result;
  }
  chromatix_wrapper->ASD.gamma_table.bits_depth = 14;
  chromatix_wrapper->ASD.gamma_table.entries_num = GAMMA_TABLE_SIZE;

  /* If more then one module involved make different function for ASD related parameter */
  COPY_ARRAY(chromatix_wrapper->ASD.gamma_table.gamma_G,
               chromatix->chromatix_VFE.chromatix_gamma.default_gamma_table.gamma_G);
  return true;
}
