/* af_biz.c
*
* Copyright (c) 2014-2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#include "af_biz.h"
#include "stats_chromatix_wrapper.h"
void* af_factory_create_custom();
#if MOVE_LENS_PROFILE
#include <sys/time.h>

/** af_biz_util_current_timestamp:
 *  Get current time in milliseconds for profiling
 *
 *  return: milliseconds
**/
int64_t af_biz_util_current_timestamp(void) {
  struct    timeval te;
  int64_t   milliseconds;
  gettimeofday(&te, NULL);
  /* Calculate milliseconds */
  milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;

  return milliseconds;
} /* af_biz_util_current_timestamp */
#endif

/** af_biz_util_cur_pos_after_lens_move:
 *  Once we receive lens moved status we need to update
 *  current lens position.
 *
 *  @af: internal AF data structure
 *
 **/
static void af_biz_util_cur_pos_after_lens_move(af_biz_internal *af) {
  /* if we are moving towards MACRO position */
  if (af->af_out.move_lens.direction == AF_MOVE_NEAR) {
    af->af_input.cur_lens_pos -= af->af_out.move_lens.num_of_steps;
  } else {
    af->af_input.cur_lens_pos += af->af_out.move_lens.num_of_steps;
  }

} /* af_biz_util_cur_pos_after_lens_move */

/** af_biz_util_move_lens_done: Lens is moved as per the request.
 *  Check if we are done focusing or still need to move.
 *
 *  @af: internal AF data structure
 *
 *  @status: lens move status
 *
 *  Return 0: Success, < 0: Failure
 **/
int af_biz_util_move_lens_done(af_biz_internal *af) {
  int rc = 0;

  if (af->af_out.move_lens.use_dac_value) {
    af->af_input.cur_lens_pos = af->af_out.move_lens.pos[0];
  } else {
    /* Update current lens position */
    af_biz_util_cur_pos_after_lens_move(af);
  }
  AF_LOW(" After move: CurPosition: %d", af->af_input.cur_lens_pos);

  return 0;
} /* af_biz_util_move_lens_done */

/** af_biz_util_update_output_data:
 * Update data that needs to be output in internal structure.
 *
 *  @af: internal AF data structure
 *
 *  @type: type of output data
 *
 *  @data: any extra argument caller of this function likes to pass.
 **/
void af_biz_util_update_output_data(af_biz_internal *af,
  af_output_type type, void *data) {
  AF_LOW(" Update output data of type: %d", type);
  switch (type) {

  case AF_OUTPUT_STOP_AF: // not used
    af->af_out.type = (af_output_type)(AF_OUTPUT_STOP_AF | af->af_out.type);
    af->af_out.stop_af = (boolean) * ((int *)data);
    break;

  case AF_OUTPUT_CHECK_LED: // not used
    af->af_out.type = (af_output_type)(AF_OUTPUT_CHECK_LED | af->af_out.type);
    break;

  case AF_OUTPUT_FOCUS_MODE: // not used
    af->af_out.type = (af_output_type)(AF_OUTPUT_FOCUS_MODE | af->af_out.type);
    Q3A_MEMCPY(&af->af_out.focus_mode_info,
      &af->af_mode, sizeof(af_mode_info_t));
    break;
  case AF_OUTPUT_SPOT_LIGHT_DETECTION:
    af->af_out.type = (af_output_type)(AF_OUTPUT_SPOT_LIGHT_DETECTION | af->af_out.type);
    af->af_out.spot_light_detected = *((uint8_t *)data);
    break;
  case AF_OUTPUT_FOCUS_VALUE:
    af->af_out.type = (af_output_type)(AF_OUTPUT_FOCUS_VALUE | af->af_out.type);
    af->af_out.focus_value = *(float *)data;
    break;
  case AF_OUTPUT_RESET_LENS:
  {
    boolean          rc;
    af_reset_lens_t  *reset_form = (af_reset_lens_t *)data;
    int comp = 0;

    af->af_out.type = (af_output_type)(AF_OUTPUT_RESET_LENS | af->af_out.type);
    Q3A_MEMCPY(&af->af_out.reset_lens, (af_reset_lens_t *)data,
      sizeof(af_reset_lens_t));

#if MOVE_LENS_PROFILE
    int64_t start_time = af_biz_util_current_timestamp();
#endif
    af->af_out.reset_lens.cur_pos = af->af_input.cur_lens_pos;
    // update cur lens pos
    af->af_input.cur_lens_pos = af->af_out.reset_lens.reset_pos;
    AF_LOW(" Reset lens at pos: %d", af->af_input.cur_lens_pos);

#if MOVE_LENS_PROFILE
    int64_t end_time = af_biz_util_current_timestamp();
    AF_LOW(" Reset lens took %llu ms", end_time - start_time);
#endif

  } /* End case AF_OUTPUT_RESET_LENS: */
    break;

  case AF_OUTPUT_MOVE_LENS:
  {
    boolean          rc;

    af->af_out.type = (af_output_type)(AF_OUTPUT_MOVE_LENS | af->af_out.type);
    Q3A_MEMCPY(&af->af_out.move_lens, (af_move_lens_t *)data,
      sizeof(af_move_lens_t));
    af->af_out.move_lens.cur_pos = af->af_input.cur_lens_pos;

#if MOVE_LENS_PROFILE
    int64_t start_time = af_biz_util_current_timestamp();
#endif
    af_biz_util_move_lens_done(af);
#if MOVE_LENS_PROFILE
    int64_t end_time = af_biz_util_current_timestamp();
    AF_LOW(" Move lens took %llu ms", end_time - start_time);
#endif

  } /* End case AF_OUTPUT_MOVE_LENS: */
    break;

  case AF_OUTPUT_STATUS:
    af->af_out.type = (af_output_type)(AF_OUTPUT_STATUS | af->af_out.type);
    Q3A_MEMCPY(&af->af_out.focus_status, (af_status_t *)data,
     sizeof(af_status_t));
    break;

  case AF_OUTPUT_ROI_INFO: // not used
    af->af_out.type = (af_output_type)(AF_OUTPUT_ROI_INFO | af->af_out.type);
    Q3A_MEMCPY(&af->af_out.roi_info, (af_roi_info_t *)data,
      sizeof(af_roi_info_t));
    break;

  case AF_OUTPUT_EZTUNE: // not used
    af->af_out.type = (af_output_type)(AF_OUTPUT_EZTUNE | af->af_out.type);
    break;

  case AF_OUTPUT_STATS_CONFIG:
    af->af_out.type = (af_output_type)(AF_OUTPUT_STATS_CONFIG | af->af_out.type);
    Q3A_MEMCPY(&af->af_out.af_stats_config, (af_config_t *)data,
      sizeof(af_config_t));
    break;

  case AF_OUTPUT_EXP_COMPENSATE:
    af->af_out.type = (af_output_type)(AF_OUTPUT_EXP_COMPENSATE | af->af_out.type);
    af->af_out.af_exp_compensate = *(boolean*)data;
    break;

  case AF_OUTPUT_PDAF_CONFIG:
    af->af_out.type = (af_output_type)(AF_OUTPUT_PDAF_CONFIG | af->af_out.type);
    Q3A_MEMCPY(&af->af_out.pdaf_config, (af_core_pdaf_config_data_t *)data,
      sizeof(af_core_pdaf_config_data_t));
    break;
  case AF_OUTPUT_DCIAF_CONFIG:
    af->af_out.type = (af_output_type)(AF_OUTPUT_DCIAF_CONFIG | af->af_out.type);
    Q3A_MEMCPY(&af->af_out.dciaf_config, (af_core_dciaf_config_data_t *)data,
      sizeof(af_core_dciaf_config_data_t));
    break;
  case AF_OUTPUT_RESET_AEC:
    af->af_out.type = (af_output_type)(AF_OUTPUT_RESET_AEC | af->af_out.type);
    break;

  case AF_OUTPUT_EZ_METADATA:
    af->af_out.type = (af_output_type)(AF_OUTPUT_EZ_METADATA | af->af_out.type);
    Q3A_MEMCPY(&af->af_out.eztune, (af_eztune_t *)data,
      sizeof(af_eztune_t));
    break;

  case AF_OUTPUT_SWAF_CONFIG: {
    int k = 0;
    Q3A_MEMCPY(&af->af_out.swaf_config, (swaf_config_data_t *)data, sizeof(swaf_config_data_t));
    af->af_out.type = (af_output_type)(AF_OUTPUT_SWAF_CONFIG | af->af_out.type);
    break;
  }

  case AF_OUTPUT_DEBUG_DATA: {
    if (AF_DEBUG_DATA_LEVEL_VERBOSE == af->af_input.exif_dbg_level) {
      /* Copy the AF debug data pointer to output struct */
      af_debug_t *core_output = (af_debug_t *)data;
      af_debug_info_t *debug_info = af->af_out.debug_info;
      debug_info->enable = TRUE;
      af->af_out.type = (af_output_type)(AF_OUTPUT_DEBUG_DATA | af->af_out.type);
    } else if (AF_DEBUG_DATA_LEVEL_CONCISE == af->af_input.exif_dbg_level) {
    /* Copy the AF debug data pointer to output struct */
      af_debug_t *core_output = (af_debug_t *)data;
      af_debug_info_t *debug_info = af->af_out.debug_info;
      debug_info->enable = TRUE;
      af->af_out.type = (af_output_type)(AF_OUTPUT_DEBUG_DATA | af->af_out.type);
    }
  }
    break;

  default:
    break;
  }
  af->af_out.result = TRUE;
} /* af_biz_util_update_output_data */

/** af_biz_util_update_roi_output_data:
 * Update roi that needs to be output in internal structure.
 *
 *  @af: internal AF data structure
 *
 *  @core_stats_info: algo config
 *
 **/
static void af_biz_util_update_roi_output_data(af_biz_internal *af,
  af_core_config_stats_engine_type *core_stats_info) {
  af->af_out.roi_info.roi[0].x = core_stats_info->bayer_config.roi_info.u.fw_info.roi.x;
  af->af_out.roi_info.roi[0].y = core_stats_info->bayer_config.roi_info.u.fw_info.roi.y;
  af->af_out.roi_info.roi[0].dx = core_stats_info->bayer_config.roi_info.u.fw_info.roi.width;
  af->af_out.roi_info.roi[0].dy = core_stats_info->bayer_config.roi_info.u.fw_info.roi.height;
}/*af_biz_util_update_roi_output_data*/

/** af_biz_util_check_if_input_is_default_roi:
 *  Check if input ROI is same as center FOV ROI. In that case
 *  we might configure ROI differently.
 *  @af: internal AF data structure
 *  @ctr_x: x coordinate of the ROI center
 *  @ctr_y: y coordinate of the ROI center
 *  Return: if default ROI return TRUE, else FALSE.
 **/
boolean af_biz_util_check_if_input_is_default_roi(
  af_biz_internal *af,
  int ctr_x,
  int ctr_y) {
  int camif_height = af->af_input.img_sensor_info.camif.height;
  int camif_width = af->af_input.img_sensor_info.camif.width;
  int fov_ctr_x = camif_width / 2;
  int fov_ctr_y = camif_height / 2;
  int dist = 0;


  dist = sqrt((double)((fov_ctr_x - ctr_x) * (fov_ctr_x - ctr_x)) +
    ((fov_ctr_y - ctr_y) * (fov_ctr_y - ctr_y)));

  AF_LOW(" CAMIF size: %dx%d FOV ctr: %dx%d ROI ctr: %dx%d, dist:%d",
    camif_width, camif_height, fov_ctr_x, fov_ctr_y, ctr_x, ctr_y, dist);


  /* if distance between two coordinates are within threshold we assume
     it's default roi */
  return ((dist < 10) ? TRUE : FALSE);
} /* af_biz_util_check_if_input_is_default_roi */

/** af_biz_util_get_focus_distance: Interface for outside components
 *  to request access to AF parameters.
 *
 *  @af: internal AF data structure
 *
 *  @f_info: focus distances information (out)
 *
 *  Return: 0 - success  <0 - failure
 **/
int af_biz_util_get_focus_distance(af_biz_internal *af,
  af_focus_distances_t *f_info) {
  float opt_focus_dist, near_focus_dist, far_focus_dist;
  float coc, back_focus, hyperD;
  af_actuator_info_t *actr_info = &(af->sensor_input.actuator_info);
  af_algo_tune_parms_adapter_t *fptr = &af->af_input.tuning_info;
  float f_len = actr_info->focal_length;
  float f_num = actr_info->af_f_num;
  float f_pix = actr_info->af_f_pix;
  int num_steps;
  float f_dist = actr_info->af_total_f_dist * 100; // recvd m, change to cm

  if (NULL == fptr) num_steps = 0.0;
  else num_steps = af->af_mode.far_end;

  AF_LOW("f_length %f f_num=%f f_pix=%f pos_far_end %d total_f_dist %f",
    actr_info->focal_length, actr_info->af_f_num,
    actr_info->af_f_pix, num_steps, actr_info->af_total_f_dist);

  if (!actr_info->focal_length || !actr_info->af_f_num ||
    !actr_info->af_f_pix || !num_steps ||
    !actr_info->af_total_f_dist) {
    /* There is no support to check focus distance is supported for this
     * sensor or not. so returning fixed focus distances to pass the test */
    f_info->focus_distance[FOCUS_DISTANCE_NEAR_INDEX] = 0.10;
    f_info->focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX] = 0.15;
    f_info->focus_distance[FOCUS_DISTANCE_FAR_INDEX] = 0.17;
    AF_LOW(" Return fixed focus distance for this sensor\n");
    return 0;
  }

  coc = 2.0 * f_pix;
  hyperD = f_len * f_len / (f_num * coc) * 1000.0;;
  back_focus = 5 + ((f_dist / num_steps) *
    (num_steps - af->af_input.cur_lens_pos + 1));
  back_focus = (back_focus +  f_len * 1000.0) / 1000.0;
  opt_focus_dist = 1.0 / ((1.0 / f_len) - (1.0 / back_focus));

  near_focus_dist = (hyperD * opt_focus_dist) /
    (hyperD + (opt_focus_dist - f_len));

  far_focus_dist = (hyperD * opt_focus_dist) /
    (hyperD - (opt_focus_dist - f_len));

  f_info->focus_distance[FOCUS_DISTANCE_NEAR_INDEX] =
    near_focus_dist / 1000;  /* in meters */

  f_info->focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX] =
    opt_focus_dist / 1000;  /* in meters */

  f_info->focus_distance[FOCUS_DISTANCE_FAR_INDEX] =
    far_focus_dist / 1000;  /* in meters */

  if (f_info->focus_distance[FOCUS_DISTANCE_FAR_INDEX] < 0
    || f_info->focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX] < 0
    || f_info->focus_distance[FOCUS_DISTANCE_NEAR_INDEX] < 0) {
    f_info->focus_distance[FOCUS_DISTANCE_NEAR_INDEX] = 0.10;
    f_info->focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX] = 0.15;
    f_info->focus_distance[FOCUS_DISTANCE_FAR_INDEX] = 0.17;
  }

  AF_LOW("LP %d, FD %f, NF %f, FF %f", af->af_input.cur_lens_pos,
    opt_focus_dist / 1000, near_focus_dist / 1000,
    far_focus_dist / 1000);

  return 0;
} /* af_biz_util_get_focus_distance */

/** af_biz_util_pack_output:
*
*    Pack output of AF process
*
*    @af: internal AF data
*
*    @af_out: output AF data
*
**/
void af_biz_util_pack_output(af_biz_internal *af,
  af_output_data_t *af_out) {
  // fill any internal data
  Q3A_MEMCPY(&af->af_out.focus_mode_info,
    &af->af_mode, sizeof(af_mode_info_t));
  /* Just empty the bin from internal out bin to external */
  Q3A_MEMCPY(af_out, &af->af_out, sizeof(af_output_data_t));
} /* af_biz_util_pack_output */

void print_step_table(const char *str, af_step_size_adapter_t *light) {
#if !defined(LOG_DEBUG)
  CAM_UNUSED_PARAM(str);
  CAM_UNUSED_PARAM(light);
#endif
  AF_LOW(" %s scan step: %d %d %d %d %d", str,
    light->rgn_0, light->rgn_1, light->rgn_2, light->rgn_3, light->rgn_4);
  return;
}
void print_bv_threshold(const char *str, BV_threshold_adapter_t *BV_thres) {
#if !defined(LOG_DEBUG)
  CAM_UNUSED_PARAM(str);
  CAM_UNUSED_PARAM(BV_thres);
#endif
  AF_LOW(" %s ",str);
  AF_LOW(" thres: %f, %f, %f, %f, %f, %f, %f, %f",
      BV_thres->thres[0], BV_thres->thres[1], BV_thres->thres[2],
      BV_thres->thres[3], BV_thres->thres[4], BV_thres->thres[5],
      BV_thres->thres[6], BV_thres->thres[7]);
  return;
}

static void af_biz_debug_print_header_info(af_biz_internal *af)
{
  af_header_info_adapter_t *info = &af->af_input.tuning_info.af_header_info;

  /* print af header info */
  AF_LOW(" header_version: %d", info->header_version);
  //AF_LOW(" cam_name: %d", info->cam_name);
} /* af_biz_debug_print_header_info */

static void af_biz_debug_print_algo_info_single(af_biz_internal *af)
{
  af_tuning_single_adapter_t *single = &af->af_input.tuning_info.af_algo.af_single;
  uint8_t i;
  af_step_size_table_adapter_t *table;
  af_step_size_adapter_t *light;
  single_threshold_adapter_t *thres;
  BV_threshold_adapter_t *BV_thres;

  /* print single info */
  for (i = 0;i < SINGLE_MAX_IDX; i++)
   AF_LOW(" step_index[%d]: %d", i, single->index[i]);
   AF_LOW(" actuator_type %d, is_hys_comp_needed %d",
    single->actuator_type, single->is_hys_comp_needed);
  AF_LOW(" step_index_per_um %d", single->step_index_per_um);
  AF_LOW(" TAF_step_table:");
  {
    table = &single->TAF_step_table;
    print_step_table("Prescan_normal_light", &table->Prescan_normal_light);
    print_step_table("Prescan_low_light", &table->Prescan_low_light);
    print_step_table("Finescan_normal_light", &table->Finescan_normal_light);
    print_step_table("Finescan_low_light", &table->Finescan_low_light);
  }
  AF_LOW(" CAF_step_table:");
  {
    table = &single->CAF_step_table;
    print_step_table("Prescan_normal_light", &table->Prescan_normal_light);
    print_step_table("Prescan_low_light", &table->Prescan_low_light);
    print_step_table("Finescan_normal_light", &table->Finescan_normal_light);
    print_step_table("Finescan_low_light", &table->Finescan_low_light);
  }
  AF_LOW(" PAAF_enable %d", single->PAAF_enable);
  AF_LOW(" sw threshold:");
  {
    thres = &single->sw;
    AF_LOW(" flat_inc_thres %f, flat_dec_thres %f, macro_thres %f,"
      " drop_thres %f", thres->flat_inc_thres, thres->flat_dec_thres,
      thres->macro_thres, thres->drop_thres);
    AF_LOW(" hist_dec_dec_thres %lu, hist_inc_dec_thres %lu",
      thres->hist_dec_dec_thres, thres->hist_inc_dec_thres);
    print_bv_threshold("dec_dec_3frame", &thres->dec_dec_3frame);
    print_bv_threshold("inc_dec_3frame", &thres->inc_dec_3frame);
    print_bv_threshold("dec_dec", &thres->dec_dec);
    print_bv_threshold("inc_dec", &thres->inc_dec);
    print_bv_threshold("dec_dec_noise", &thres->dec_dec_noise);
    print_bv_threshold("inc_dec_noise", &thres->inc_dec_noise);
  }
  AF_LOW(" hw threshold:");
  {
    thres = &single->hw;
    AF_LOW(" flat_inc_thres %f, flat_dec_thres %f, macro_thres %f,"
      " drop_thres %f", thres->flat_inc_thres, thres->flat_dec_thres,
      thres->macro_thres, thres->drop_thres);
    AF_LOW(" hist_dec_dec_thres %lu, hist_inc_dec_thres %lu",
      thres->hist_dec_dec_thres, thres->hist_inc_dec_thres);
    print_bv_threshold("dec_dec_3frame", &thres->dec_dec_3frame);
    print_bv_threshold("inc_dec_3frame", &thres->inc_dec_3frame);
    print_bv_threshold("dec_dec", &thres->dec_dec);
    print_bv_threshold("inc_dec", &thres->inc_dec);
    print_bv_threshold("dec_dec_noise", &thres->dec_dec_noise);
    print_bv_threshold("inc_dec_noise", &thres->inc_dec_noise);
  }
  for (i = 0; i < 8;i++) {
    AF_LOW(" BV_gain[%d] : %f", i, single->BV_gain[i]);
  }
  AF_LOW(" optics info:");
  {
    AF_LOW(" CAF_far_end %d", single->optics.CAF_far_end);
    AF_LOW(" CAF_near_end %d", single->optics.CAF_near_end);
    AF_LOW(" TAF_far_end %d", single->optics.TAF_far_end);
    AF_LOW(" TAF_near_end %d", single->optics.TAF_near_end);
    AF_LOW(" srch_rgn_1 %d", single->optics.srch_rgn_1);
    AF_LOW(" srch_rgn_2 %d", single->optics.srch_rgn_2);
    AF_LOW(" srch_rgn_3 %d", single->optics.srch_rgn_3);
    AF_LOW(" fine_srch_rgn %d", single->optics.fine_srch_rgn);
    AF_LOW(" far_zone %d", single->optics.far_zone);
    AF_LOW(" near_zone %d", single->optics.near_zone);
    AF_LOW(" mid_zone %d", single->optics.mid_zone);
    AF_LOW(" init_pos %d", single->optics.init_pos);
  }
} /* af_biz_debug_print_algo_info_single */

static void af_biz_debug_print_algo_info(af_biz_internal *af) {
  af_tuning_algo_adapter_t *info = &af->af_input.tuning_info.af_algo;
  af_tuning_continuous_adapter_t *cont = &info->af_cont;
  af_tuning_exhaustive_adapter_t *exh = &info->af_exh;
  af_tuning_fullsweep_adapter_t *full = &info->af_full;
  af_tuning_sp_adapter_t *sp = &info->af_sp;
  af_tuning_single_adapter_t *single = &info->af_single;
  af_shake_resistant_adapter_t *shake = &info->af_shake_resistant;
  af_motion_sensor_adapter_t *motion = &info->af_motion_sensor;
  af_fd_priority_caf_adapter_t *fd = &info->fd_prio;

  af_tuning_sad_adapter_t *sad = &cont->af_sad;
  af_tuning_gyro_adapter_t *gyro = &cont->af_gyro;

  AF_LOW(" E");

  /* Print Common Element */
  AF_LOW(" af_process_type: %d", info->af_process_type);
  AF_LOW(" Position: near:%d far:%d hypf:%d def_mac:%d"
    "bound:%d def_norm:%d mac_rgn:%d ",
    info->position_near_end, info->position_far_end,
    info->position_normal_hyperfocal, info->position_default_in_macro,
    info->position_boundary, info->position_default_in_normal,
    info->position_macro_rgn);
  AF_LOW(" Undershoot protect: %d ajust: %d",
    info->undershoot_protect, info->undershoot_adjust);
  AF_LOW(" threshold: min/max:%f, lowlightgain:%f, base_delay_adj:%f",
    info->min_max_ratio_th, info->low_light_gain_th ,
    info->base_delay_adj_th);
  AF_LOW(" LED_AF: enable: %d, trigger_idx: %ld",
    info->led_af_assist_enable, info->led_af_assist_trigger_idx);

  /* Print Continous search Element */
  AF_LOW(" CAF enabled : %d", cont->enable);
  AF_LOW(" Scene change detection ratio: %d",
    cont->scene_change_detection_ratio);
  AF_LOW(" Panning Stable FV change trigger: %f",
    cont->panning_stable_fv_change_trigger);
  AF_LOW(" Panning Stable FVavg to FV change trigger: %f",
    cont->panning_stable_fvavg_to_fv_change_trigger);
  AF_LOW(" Panning Unstable trigger cnt: %d",
    cont->panning_unstable_trigger_cnt);
  AF_LOW(" Panning Stable trigger cnt: %d",
    cont->panning_stable_trigger_cnt);
  AF_LOW(" CAF Downhill Allowance: %lu",
    cont->downhill_allowance);
  AF_LOW(" CAF uphill allowance: %d",
    cont->uphill_allowance);
  AF_LOW(" Base frame delay: %d", cont->base_frame_delay);
  AF_LOW(" CAF trigger luma change  threshold: %d",
    cont->scene_change_luma_threshold);
  AF_LOW(" CAF trigger luma settle threshold: %d",
    cont->luma_settled_threshold);
  AF_LOW(" Threshold in noise: %f",
    cont->noise_level_th);
  AF_LOW(" Search step size: %d", cont->search_step_size);
  AF_LOW(" Init Search type: %d", cont->init_search_type);
  AF_LOW(" Search type: %d", cont->search_type);
  AF_LOW(" Low light wait: %d", cont->low_light_wait);
  AF_LOW(" Max indecision cnt: %d", cont->max_indecision_cnt);
  AF_LOW(" flat_fv_confidence_level: %f", cont->flat_fv_confidence_level);
  AF_LOW(" SAD enabled : %d", sad->enable);
  AF_LOW(" SAD: gain_min: %f gain_max: %f ref_gain_min: %f ref_gain_max: %f"
     "th_min: %u th_max: %u ref_th_min: %u ref_th_max: %u frames_to_wait: %d",
     sad->gain_min, sad->gain_max, sad->ref_gain_min, sad->ref_gain_max,
     sad->threshold_min, sad->threshold_max, sad->ref_threshold_min,
     sad->ref_threshold_max, sad->frames_to_wait);
  AF_LOW(" GYRO enabled : %d", gyro->enable);
  AF_LOW(" GYRO Threshold: min_move %f, stable_det %f, unstble_cnt_th %u,"
    "stable_cnt_th %u, fastpan %f slowpan %f, fastpancnt %u, sum_to_orig %u"
    "Stable Count delay %u", gyro->min_movement_threshold,
    gyro->stable_detected_threshold, gyro->unstable_count_th,
    gyro->stable_count_th, gyro->fast_pan_threshold, gyro->slow_pan_threshold,
    gyro->fast_pan_count_threshold, gyro->sum_return_to_orig_pos_threshold,
    gyro->stable_count_delay);

  /* Print Exhaustive Search Element */
  AF_LOW(" For Exhaustive Search: ");
  AF_LOW(" Num gross steps: %d",
    exh->num_gross_steps_between_stat_points);
  AF_LOW(" Num fine steps: %d",
    exh->num_fine_steps_between_stat_points);
  AF_LOW(" Num fine search points: %d",
    exh->num_fine_search_points);
  AF_LOW(" Downhill allowance: %d",exh->downhill_allowance);
  AF_LOW(" Uphill allowance: %d",exh->uphill_allowance);
  AF_LOW(" Base frame delay: %d",exh->base_frame_delay);
  AF_LOW(" coarse_frame_delay: %u", exh->coarse_frame_delay);
  AF_LOW(" fine_frame_delay: %u", exh->fine_frame_delay);
  AF_LOW(" coarse_to_fine_frame_delay: %u",
    exh->coarse_to_fine_frame_delay);
  AF_LOW(" noise_level_th: %f",exh->noise_level_th);
  AF_LOW(" flat_fv_confidence_level: %f",
    exh->flat_fv_confidence_level);
  AF_LOW(" climb_ratio_th: %f",exh->climb_ratio_th);
  AF_LOW(" low_light_luma_th: %d",exh->low_light_luma_th);
  AF_LOW(" enable_multiwindow: %d",exh->enable_multiwindow);
  AF_LOW(" gain_thresh: %f",exh->gain_thresh);

  /* Print Full Sweep Element */

  /* Print Slope predictive Element */
  AF_LOW(" For Slope-predictive Search:");
  AF_LOW(" Flat curve threshold: %f",
    sp->fv_curve_flat_threshold);
  AF_LOW(" Slope Threshold_1: %f Threshold_2: %f Threshold_3: %f"
    "Threshold_4: %f", sp->slope_threshold1, sp->slope_threshold2,
    sp->slope_threshold3, sp->slope_threshold4);

  /* Print Single search Element */
  af_biz_debug_print_algo_info_single(af);

  /* Print Shake resistant Element */
  /* Print Motion Element */
  /* Print Face AF Element */

}

static void af_biz_debug_print_vfe_info(af_biz_internal *af)
{
  af_tuning_vfe_adapter_t *info = &af->af_input.tuning_info.af_vfe[0];
  int i = 0;

  AF_LOW(" VFE config parameters: ");
  AF_LOW(" Min FV: %d", info->config.fv_min);
  AF_LOW(" max_h_num: %d max_v_num: %d",
    info->config.max_h_num, info->config.max_v_num);
  AF_LOW(" max_block_size: %d x %d min_block_size: %d x %d",
    info->config.max_block_width,
    info->config.max_block_height,
    info->config.min_block_width,
    info->config.min_block_height);
}

static void af_biz_debug_print_af_tuning_parms(af_biz_internal *af)
{
  AF_LOW(" AF Tunng Parameters:");

  /* print Header info */
  af_biz_debug_print_header_info(af);

  /* print af_Algo info */
  af_biz_debug_print_algo_info(af);

  /* print vfe info */
  af_biz_debug_print_vfe_info(af);
}

/** af_biz_reset_lens: Update current lens position and set
 *  request to reset lens.
 *
 *  @af: internal AF data structure
 *
 *  @def_pos: if true, lens will be reset to default position
 *          specific to each focus mode.
 *
 *  @pos: position to reset the lens to
 *
 * Return 0: Success <0: Failure
 **/
static int af_biz_reset_lens(af_biz_internal *af, boolean def_pos,
  int pos) {
  int rc = 0;

  af_reset_lens_t reset_form;
  /* If default position flag is not set, then only we'll use "pos" */
  if (!def_pos && !pos &&
    ((pos < af->af_mode.near_end) || (pos > af->af_mode.far_end))) {
    AF_INFO(" Invalid Lens Position to reset to!");
    return -1;
  }

  /* If we already are at the reset position, we can just return */
  if ((def_pos && (af->af_input.cur_lens_pos == af->af_mode.default_pos)) ||
    (!def_pos && (af->af_input.cur_lens_pos == pos))) {
    AF_LOW(" Lens already at reset position %d",
      af->af_input.cur_lens_pos);
    return 0;
  }

  /* update reset lens request form */
  reset_form.reset_lens = TRUE;
  reset_form.reset_pos = (def_pos) ? af->af_mode.far_end: pos;

  /* update output data */
  af_biz_util_update_output_data(af, AF_OUTPUT_RESET_LENS, (void *)&reset_form);

  AF_LOW(" Resetting the lens from %d to: %d",
    af->af_input.cur_lens_pos,
    reset_form.reset_pos);

  return rc;
} /* af_biz_reset_lens */


/** af_biz_move_lens: Update current lens position and set
 *  request to move lens.
 *
 *  @af: internal AF data structure
 *
 *  @pos: position to move the lens to
 *
 * Return 0: Success <0: Failure
 **/
static int af_biz_move_lens_to(af_biz_internal *af, int pos) {

  int rc = 0;
  int steps = pos - af->af_input.cur_lens_pos;

  AF_LOW("Requested pos : %d, cur_lens_pos : %d, Step size : %d\n", pos,af->af_input.cur_lens_pos,steps);

  if (steps != 0) {
    af_move_lens_t move_lens;
    Q3A_MEMSET(&move_lens, 0, sizeof(af_move_lens_t));
    /* create a request to move lens */
    move_lens.move_lens = TRUE;
    if (steps > 0) {
      move_lens.direction    = AF_MOVE_FAR;
      move_lens.num_of_steps = steps;
    } else if (steps < 0) {
      move_lens.direction    = AF_MOVE_NEAR;
      move_lens.num_of_steps = -steps;
    }
    move_lens.use_dac_value = FALSE;
    af_biz_util_update_output_data(af, AF_OUTPUT_MOVE_LENS, &move_lens);
  }
  return rc;
} /* af_biz_move_lens_to */

/** af_biz_is_af_call_during_infy_mode: Handle
 *  autofocus/touch-AF call during AF mode Infinity
 *
 *  @af: internal AF data structure
 *
 *
 * Return: TRUE - is AF call during infy mode FALSE:regular mode
 **/
static boolean af_biz_is_af_call_during_infy_mode(af_biz_internal *af)
{
  boolean rc = FALSE;
  af_status_t af_status;
  /* create a request to af status */
  af_status.focus_done = FALSE;
  if (af->af_mode.mode == AF_MODE_INFINITY) {
    AF_LOW("Received AF call during Infinity focus mode!!!");

    /* create a request to af status */
    af_status.focus_done = FALSE;
    af_status.status = AF_STATUS_INVALID;
    af->status = af_status.status;
    af_biz_util_update_output_data(af, AF_OUTPUT_STATUS, &af_status);
    rc = TRUE;
  }
  return rc;
} /* af_biz_is_af_call_during_infy_mode */

/** af_biz_start_focus: Start autofocus.
 *
 *  @af: internal AF data structure
 *
 * Return 0: Success <0: Failure
 **/
int af_biz_start_focus(af_biz_internal * af) {
  int rc = 0;
  af_core_set_param_info_type set_param;
  set_param.param_type = AF_CORE_SET_PARAM_START;
  if (NULL == af->af_algo_ops.set_parameters) {
    AF_ERR("error: set_parameter() not set");
    return -1;
  }

  af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);
  return rc;
} /* af_biz_util_start_focus */

/** af_biz_focus_mode: Set focus mode and update required
 *  parameters.
 *
 *  @af: internal AF data structure
 *
 *  @af_mode: focus mode to set
 *
 * Return 0: Success <0: Failure
 **/
static int af_biz_focus_mode_internal(af_biz_internal *af,
  af_mode_type af_mode ) {
  int rc = 0;
  af_tuning_algo_adapter_t *algo_tuning = NULL;

  algo_tuning = &(af->af_input.tuning_info.af_algo);

  AF_LOW("Setting focus mode before bestmode check to: %d",
    af_mode);

  /* CONFIG AF for BESTHOT mode */
  switch (af->bestshot_d.curr_mode) {
    case AF_BESTSHOT_THEATRE:
    case AF_BESTSHOT_SPORTS:
    case AF_BESTSHOT_ANTISHAKE:
    case AF_BESTSHOT_FIREWORKS:
    case AF_BESTSHOT_ACTION:
    /*For above Bestshot mode Focus mode set by UI is overwritten to AUTO*/
      af_mode = AF_MODE_AUTO;
      break;
    case AF_BESTSHOT_SUNSET:
    case AF_BESTSHOT_LANDSCAPE:
    /*For above Bestshot mode Focus mode set by UI is overwritten to INFINITY*/
      af_mode = AF_MODE_INFINITY;
      break;
    case AF_BESTSHOT_FLOWERS:
      af_mode = AF_MODE_MACRO;
      break;
    case AF_BESTSHOT_SNOW:
    case AF_BESTSHOT_BEACH:
    case AF_BESTSHOT_NIGHT:
    case AF_BESTSHOT_PORTRAIT:
    case AF_BESTSHOT_BACKLIGHT:
    case AF_BESTSHOT_CANDLELIGHT:
    case AF_BESTSHOT_PARTY:
    case AF_BESTSHOT_NIGHT_PORTRAIT:
    case AF_BESTSHOT_AR:
    case AF_BESTSHOT_AUTO:
    case AF_BESTSHOT_OFF:
    case AF_BESTSHOT_FACE_PRIORITY:
    case AF_BESTSHOT_BARCODE:
    case AF_BESTSHOT_HDR:
    /*For other Best mode we do not overwrite the AF mode */
    /*The Focus mode set by UI would be used in this case */
    default:
      break;
    }

  af->af_mode_changed = (af_mode == af->af_mode.mode) ? FALSE : TRUE;
  af->af_mode.prev_mode = af->af_mode.mode;
  af->af_mode.mode = af_mode;
  AF_HIGH("Setting focus mode to: %d", af_mode);

  switch (af_mode) {

  case AF_MODE_AUTO:
    af->af_input.search_range.near_end_position =
      af->search_limit[AF_FOCUS_MODE_DEFAULT].near_end;
    af->af_input.search_range.far_end_position =
      af->search_limit[AF_FOCUS_MODE_DEFAULT].far_end;
    af->af_input.search_range.default_position =
      af->search_limit[AF_FOCUS_MODE_DEFAULT].default_pos;
    af->af_input.search_range.hyperfocal_position =
      algo_tuning->position_normal_hyperfocal;
    break;

  case AF_MODE_CAF:
    af->af_input.search_range.near_end_position =
      af->search_limit[AF_FOCUS_MODE_CAF].near_end;
    af->af_input.search_range.far_end_position =
      af->search_limit[AF_FOCUS_MODE_CAF].far_end;
    af->af_input.search_range.default_position =
      af->search_limit[AF_FOCUS_MODE_CAF].default_pos;
    af->af_input.search_range.hyperfocal_position =
      algo_tuning->position_normal_hyperfocal;
    break;

  case AF_MODE_NORMAL:
  case AF_MODE_CAF_NORMAL:
    af->af_input.search_range.near_end_position =
      af->search_limit[AF_FOCUS_MODE_NORMAL].near_end;
    af->af_input.search_range.far_end_position =
      af->search_limit[AF_FOCUS_MODE_NORMAL].far_end;
    af->af_input.search_range.default_position =
      af->search_limit[AF_FOCUS_MODE_NORMAL].default_pos;
    af->af_input.search_range.hyperfocal_position =
      algo_tuning->position_normal_hyperfocal;
    break;

  case AF_MODE_MACRO:
  case AF_MODE_CAF_MACRO:
    af->af_input.search_range.near_end_position =
      af->search_limit[AF_FOCUS_MODE_MACRO].near_end;
    af->af_input.search_range.far_end_position =
      af->search_limit[AF_FOCUS_MODE_MACRO].far_end;
    af->af_input.search_range.default_position =
      af->search_limit[AF_FOCUS_MODE_MACRO].default_pos;
    af->af_input.search_range.hyperfocal_position =
      algo_tuning->position_normal_hyperfocal;
    break;

  case AF_MODE_INFINITY:
    af->af_input.search_range.near_end_position =
      algo_tuning->position_normal_hyperfocal;
    af->af_input.search_range.far_end_position =
      algo_tuning->position_far_end;
    af->af_input.search_range.default_position =
      algo_tuning->position_normal_hyperfocal;
    af->af_input.search_range.hyperfocal_position =
      algo_tuning->position_normal_hyperfocal;
    break;

  case AF_MODE_MANUAL_HAL1:
  case AF_MODE_MANUAL:
    af->af_input.search_range.near_end_position =
      af->search_limit[AF_FOCUS_MODE_MANUAL].near_end;
    af->af_input.search_range.far_end_position =
      af->search_limit[AF_FOCUS_MODE_MANUAL].far_end;
    af->af_input.search_range.default_position =
      af->search_limit[AF_FOCUS_MODE_MANUAL].default_pos;
    af->af_input.search_range.hyperfocal_position =
      algo_tuning->position_normal_hyperfocal;
    break;
  default:
    AF_ERR(" Failed: Unknown AF mode\n");
    rc = -1;
    break;
  }
  af->af_mode.far_end = af->af_input.search_range.far_end_position;
  af->af_mode.hyp_pos = af->af_input.search_range.hyperfocal_position;
  af->af_mode.default_pos = af->af_input.search_range.default_position;
  af->af_mode.near_end = af->af_input.search_range.near_end_position;

  if (!rc) {
    /* update output data */
    af_biz_util_update_output_data(af, AF_OUTPUT_FOCUS_MODE, NULL);

    /* reset the lens if AF mode has changed */
    if (af->af_mode_changed == TRUE) {
      /* But not if we are chaning modes from single to CAF.
         for example, when we are returning from touch-AF to CAF */
      AF_LOW(" Focus mode changed!");
      if (!IS_CAF_ENABLED(af_mode)) {
        if (af_mode == AF_MODE_INFINITY) {
          // If the SOF Id is 0, it is very likely that the reset_lens which was
          // sent at AF_SET_PARM_INIT is not yet complete resulting in a
          // very blur preview. So, defer the lens movement here and
          // do it when we get the first SOF
          if(af->sof_id != 0) {
            AF_HIGH(" Infinity mode, move lens to def pos");
            af_biz_move_lens_to(af, af->af_mode.default_pos);
            af->move_lens_to_def_pos = TRUE;
          } else {
            AF_HIGH(" First SOF not received yet, move lens in first SOF");
          }
        }
      }
      /* Reset lens if mode set to manual but first sof hasnt done reset yet */
      if ((!af->lens_reset_on_init) &&
        (af_mode == AF_MODE_MANUAL || af_mode == AF_MODE_MANUAL_HAL1)) {
        AF_HIGH("Manual Mode before SOF! Reset the lens!");
        af_biz_reset_lens(af, TRUE, 0);
        af->lens_reset_on_init = TRUE;
      }
      /* reset lens while coming back from manual mode since biz layer is
         not aware of lens move happened in manual mode since it handled in port
         but dont need to reset when going into manual mode since port keeps
         track of cur position using af_focus_pos except for above case */
      if(af->af_mode.prev_mode == AF_MODE_MANUAL ||
        af->af_mode.prev_mode == AF_MODE_MANUAL_HAL1){
        AF_HIGH(" Lens Reset because coming from manual mode");
        af_biz_reset_lens(af, TRUE, 0);
      }
    }
  }
  // Send set param to core
  af_core_set_param_info_type set_param;
  set_param.param_type = AF_CORE_SET_PARAM_FOCUS_MODE;
  set_param.u.set_focus.af_mode_changed = af->af_mode_changed;
  Q3A_MEMCPY(&set_param.u.set_focus.focus_mode, &af->af_mode,
    sizeof(af_mode_info_t));

  if (NULL == af->af_algo_ops.set_parameters) {
    AF_ERR("error: set_parameter() not set");
    return -1;
  }
  af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);

  return rc;
} /* af_biz_focus_mode_internal */


/** af_biz_focus_mode: Set focus mode and update required
 *  parameters.
 *
 *  @af: internal AF data structure
 *
 *  @af_mode: focus mode to set
 *
 * Return 0: Success <0: Failure
 **/
static int af_biz_focus_mode(af_biz_internal *af,
  af_mode_type af_mode) {
  int rc = 0;

  /* Check if AF mode is not supported */
  if ((af_mode == AF_MODE_NOT_SUPPORTED) || (af_mode >= AF_MODE_MAX)) {
    /* We'll default to AUTO */
    af_mode = AF_MODE_AUTO;
  }

  af->af_UI_mode = af_mode;
  if (af_biz_focus_mode_internal(af, af->af_UI_mode) < 0) rc = FALSE;
  return rc;
} /* af_biz_focus_mode */

/** af_biz_cancel_focus: Cancel autofocus. If single AF, it'll
 *  cancel current focusing and stops AF stats generation. If
 *  CAF, it stops current search and go to monitor mode.
 *
 *  @af: internal AF data structure
 *
 * Return 0: Success <0: Failure
 **/
static int af_biz_cancel_focus(af_biz_internal *af) {
  af_core_set_param_info_type set_param;
  set_param.param_type = AF_CORE_SET_PARAM_CANCEL;
  if (NULL == af->af_algo_ops.set_parameters) {
    AF_ERR("error: set_parameter() not set");
    return -1;
  }
  af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);

  return 0;
} /* af_biz_cancel_focus */

/** af_biz_stop_focus: Stops autofocus. Stops AF stats
 *  generation too.
 *
 *  @af: internal AF data structure
 *
 * Return 0: Success <0: Failure
 **/
int af_biz_stop_focus(af_biz_internal *af) {
  /* fill request form to stop autofocus */
  af_biz_cancel_focus(af);
  int stop_focus = FALSE;
  stop_focus = TRUE;
  af_biz_util_update_output_data(af, AF_OUTPUT_STOP_AF, &stop_focus);
  return 0;
} /* af_biz_stop_focus */

/** af_biz_bestshot_mode: update AF parameters to support
 *  bestshot mode
 *
 *  @af: internal AF data structure
 *
 *  @mode: bestshot mode to be set
 *
 *  Return: 0 - Success <1 - Failure
 **/
static int af_biz_bestshot_mode(af_biz_internal *af,
  af_bestshot_mode_type_t mode) {
  int rc = 0;
  af_mode_type af_mode = AF_MODE_AUTO;
  AF_LOW("mode =%d",mode);
  if (mode  >= AF_BESTSHOT_MAX) {
    AF_INFO(" Invalid bestshot mode, setting it to default");
    mode = AF_BESTSHOT_OFF;
  }

  if (af->bestshot_d.curr_mode == mode) {
    AF_LOW(" Request to set same mode again (prev: %d new: %d). Return!",
      af->bestshot_d.curr_mode, mode);
    return 0;
  }

  af->bestshot_d.curr_mode = mode;
  af->bestshot_d.enable = FALSE;
  af_biz_focus_mode_internal(af, af->af_UI_mode);
  af->bestshot_d.enable = TRUE;

  return rc;
} /* af_biz_bestshot_mode */

/** af_biz_map_roi: Save new RoI and ask to update VFE
 *  configuration..
 *
 *  @af: internal AF data structure
 *
 **/
static void af_biz_map_roi(af_biz_internal *af,
  af_roi_info_t *roi_info) {
  int i = 0;

  int ctr_x = 0, ctr_y = 0;
  int camif_height = af->af_input.img_sensor_info.camif.height;
  int camif_width = af->af_input.img_sensor_info.camif.width;

  Q3A_MEMSET(&af->af_input.roi_info, 0, sizeof(af_core_config_roi_type));
  if (roi_info->type == AF_ROI_TYPE_FACE) {
    af->af_input.roi_info.roi_type = AF_CORE_ROI_TYPE_FACE;
  } else if (roi_info->type == AF_ROI_TYPE_TOUCH) {
    af->af_input.roi_info.roi_type = AF_CORE_ROI_TYPE_TOUCH;
  } else if (roi_info->type == AF_ROI_TYPE_GENERAL) {
    af->af_input.roi_info.roi_type = AF_CORE_ROI_TYPE_FIXED;
  } else {
    af->af_input.roi_info.roi_type = AF_CORE_ROI_TYPE_FIXED;
  }
  af->af_input.roi_info.roi_cordinates.num_of_roi = roi_info->num_roi;
  af->af_input.roi_info.roi_updated = TRUE;

  AF_LOW(" type: %d num_roi: %d",
    roi_info->type, roi_info->num_roi);
  for (i = 0;
    i < (int)roi_info->num_roi && i < AF_MAX_ROI_COUNT; i++) {
    af->af_input.roi_info.roi_cordinates.roi[i].x = roi_info->roi[i].x;
    af->af_input.roi_info.roi_cordinates.roi[i].y = roi_info->roi[i].y;
    af->af_input.roi_info.roi_cordinates.roi[i].width = roi_info->roi[i].dx;
    af->af_input.roi_info.roi_cordinates.roi[i].height = roi_info->roi[i].dy;
    AF_LOW(" roi[%d]- x: %d y: %d dx: %d dy: %d", i,
      roi_info->roi[i].x, roi_info->roi[i].y,
      roi_info->roi[i].dx, roi_info->roi[i].dy);
  }

  /* calculate center ROI coordinate */
  if ((roi_info->num_roi < 1) ||
    (((roi_info->roi->x + roi_info->roi->dx) >= camif_width) ||
    ((roi_info->roi->y + roi_info->roi->dy) >= camif_height)) ||
    (!(roi_info->roi->dx) && !(roi_info->roi->dy))) {
    AF_LOW(" ROI limit check failed. Using Center ROI!");
    ctr_x = camif_width / 2;
    ctr_y = camif_height / 2;
  } else {
    ctr_x = roi_info->roi->x + (roi_info->roi->dx / 2);
    ctr_y = roi_info->roi->y + (roi_info->roi->dy / 2);
  }

  // Check for Touch ROI via range
  boolean is_roi_default =
    af_biz_util_check_if_input_is_default_roi(af, ctr_x, ctr_y);
  if (roi_info->type == AF_ROI_TYPE_TOUCH &&
    (is_roi_default)) {
    AF_LOW(" NON Touch ROI ");
    af->af_input.roi_info.roi_type = AF_CORE_ROI_TYPE_FIXED;
  }
  else if (roi_info->type == AF_ROI_TYPE_GENERAL &&
    (!is_roi_default)) {
    AF_LOW(" Touch ROI ");
    af->af_input.roi_info.roi_type = AF_CORE_ROI_TYPE_TOUCH;
  }

  /* Update ROI info to outer world in case it's required */
  af_biz_util_update_output_data(af, AF_OUTPUT_ROI_INFO,
    roi_info);
  af->roi_changed = TRUE;
}

/** af_biz_map_fw_region_type: convert biz logic fw region type
 * to af core regino type
**/
static af_core_fw_region_type_t af_biz_map_fw_region_type (bf_fw_region_type_t input) {
  af_core_fw_region_type_t output = AF_CORE_BF_FW_MAX_REGION;
  switch (input) {
    case BF_FW_PRIMARY_REGION:
      output = AF_CORE_BF_FW_PRIMARY_REGION;
      break;
    case BF_FW_SECONDARY_REGION:
      output = AF_CORE_BF_FW_SECONDARY_REGION;
      break;
    default:
      break;
  }
   return output;
}

/** af_biz_map_stats_input: update stats info in input sturcture
 *
 *  @af: internal AF data structure
 *
 *  @stats_info: stats input send by vfe
 *
 *  Return: 0 - Success <1 - Failure
 **/
static int af_biz_map_stats_input(af_biz_internal *af,
  stats_af_t *stats_info) {
  int rc = 0;
  af->frame_id = stats_info->frame_id;
  af_core_stats_bayer_type *bayer_stats =
    &af->af_input.stats_info.bayer_stats;
  q3a_bf_stats_t *primary_kernel = stats_info->u.p_q3a_bf_stats[0];
  q3a_bf_stats_t *secondary_kernel = stats_info->u.p_q3a_bf_stats[1];

  if (stats_info->stats_type_mask == STATS_AF) {
    AF_ERR(" YUV STATS NOT SUPPORTED");
    return -1;
  }

  af->af_input.stats_info.mask = AF_CORE_STATS_MASK_BAYER;
  af->af_input.stats_info.bayer_stats.frame_id = stats_info->frame_id;

  if (stats_info->stats_type_mask & STATS_BF_SCL) {
    af->af_input.stats_info.bayer_stats.u.grid_stats.kernel_count = 2;
  } else {
    af->af_input.stats_info.bayer_stats.u.grid_stats.kernel_count = 1;
  }

  if(stats_info->isFake) {
    af->af_input.stats_info.bayer_stats.isFake = TRUE;
    return 0;
  } else {
    af->af_input.stats_info.bayer_stats.isFake = FALSE;
  }

  if (primary_kernel->mask == MCT_EVENT_STATS_BF_FW) {
    af->af_input.stats_info.bayer_stats.bf_type = AF_CORE_BF_FW;
  } else if (primary_kernel->mask == MCT_EVENT_STATS_BF_SINGLE) {
    af->af_input.stats_info.bayer_stats.bf_type = AF_CORE_BF_SINGLE;
  } else if (primary_kernel->mask == MCT_EVENT_STATS_BF_SCALE) {
    af->af_input.stats_info.bayer_stats.bf_type = AF_CORE_BF_SCALE;
  }

  if (primary_kernel->mask == MCT_EVENT_STATS_BF_SINGLE ||
      primary_kernel->mask == MCT_EVENT_STATS_BF_SCALE) {
    /* If BF single or BF scale, use grid stats */
    bayer_stats->u.grid_stats.array_size = MAX_BF_STATS_NUM;
    bayer_stats->u.grid_stats.horizontal_regions = primary_kernel->u.grid_stats.bf_region_h_num;
    bayer_stats->u.grid_stats.vertical_regions = primary_kernel->u.grid_stats.bf_region_v_num;
    bayer_stats->u.grid_stats.use_max_fv[0] = primary_kernel->u.grid_stats.use_max_fv;
    bayer_stats->u.grid_stats.red_sum[0] = primary_kernel->u.grid_stats.bf_r_sum;
    bayer_stats->u.grid_stats.blue_sum[0] = primary_kernel->u.grid_stats.bf_b_sum;
    bayer_stats->u.grid_stats.gr_sum[0] = primary_kernel->u.grid_stats.bf_gr_sum;
    bayer_stats->u.grid_stats.gb_sum[0] = primary_kernel->u.grid_stats.bf_gb_sum;
    bayer_stats->u.grid_stats.red_sharpness[0] = primary_kernel->u.grid_stats.bf_r_sharp;
    bayer_stats->u.grid_stats.blue_sharpness[0] = primary_kernel->u.grid_stats.bf_b_sharp;
    bayer_stats->u.grid_stats.gr_sharpness[0] = primary_kernel->u.grid_stats.bf_gr_sharp;
    bayer_stats->u.grid_stats.gb_sharpness[0] = primary_kernel->u.grid_stats.bf_gb_sharp;
    bayer_stats->u.grid_stats.red_count[0] = primary_kernel->u.grid_stats.bf_r_num;
    bayer_stats->u.grid_stats.blue_count[0] = primary_kernel->u.grid_stats.bf_b_num;
    bayer_stats->u.grid_stats.gr_count[0] = primary_kernel->u.grid_stats.bf_gr_num;
    bayer_stats->u.grid_stats.gb_count[0] = primary_kernel->u.grid_stats.bf_gb_num;
    bayer_stats->u.grid_stats.red_max_fv[0] = primary_kernel->u.grid_stats.bf_r_max_fv;
    bayer_stats->u.grid_stats.blue_max_fv[0] = primary_kernel->u.grid_stats.bf_b_max_fv;
    bayer_stats->u.grid_stats.gr_max_fv[0] = primary_kernel->u.grid_stats.bf_gr_max_fv;
    bayer_stats->u.grid_stats.gb_max_fv[0] = primary_kernel->u.grid_stats.bf_gb_max_fv;
  } else if (primary_kernel->mask == MCT_EVENT_STATS_BF_FW) {
    bayer_stats->u.fw_stats.num_bf_fw_roi_dim =
      primary_kernel->u.fw_stats.bf_fw_roi_cfg.num_bf_fw_roi_dim;
    uint32 i;
    for (i = 0; i < bayer_stats->u.fw_stats.num_bf_fw_roi_dim; i++) {
      bayer_stats->u.fw_stats.bf_fw_roi_dim[i].x =
        primary_kernel->u.fw_stats.bf_fw_roi_cfg.bf_fw_roi_dim[i].x;
      bayer_stats->u.fw_stats.bf_fw_roi_dim[i].y =
        primary_kernel->u.fw_stats.bf_fw_roi_cfg.bf_fw_roi_dim[i].y;
      bayer_stats->u.fw_stats.bf_fw_roi_dim[i].w =
        primary_kernel->u.fw_stats.bf_fw_roi_cfg.bf_fw_roi_dim[i].w;
      bayer_stats->u.fw_stats.bf_fw_roi_dim[i].h =
        primary_kernel->u.fw_stats.bf_fw_roi_cfg.bf_fw_roi_dim[i].h;
      bayer_stats->u.fw_stats.bf_fw_roi_dim[i].is_valid =
        primary_kernel->u.fw_stats.bf_fw_roi_cfg.bf_fw_roi_dim[i].is_valid;
      bayer_stats->u.fw_stats.bf_fw_roi_dim[i].region_type =
        af_biz_map_fw_region_type (primary_kernel->u.fw_stats.bf_fw_roi_cfg.bf_fw_roi_dim[i].region);
    }
    bayer_stats->u.fw_stats.bf_h1_num = primary_kernel->u.fw_stats.bf_h1_num;
    bayer_stats->u.fw_stats.bf_h2_num = primary_kernel->u.fw_stats.bf_h2_num;
    bayer_stats->u.fw_stats.bf_v_num = primary_kernel->u.fw_stats.bf_v_num;
    bayer_stats->u.fw_stats.bf_h1_sum = primary_kernel->u.fw_stats.bf_h1_sum;
    bayer_stats->u.fw_stats.bf_h2_sum = primary_kernel->u.fw_stats.bf_h2_sum;
    bayer_stats->u.fw_stats.bf_v_sum = primary_kernel->u.fw_stats.bf_v_sum;
    bayer_stats->u.fw_stats.bf_h1_sharp = primary_kernel->u.fw_stats.bf_h1_sharp;
    bayer_stats->u.fw_stats.bf_h2_sharp = primary_kernel->u.fw_stats.bf_h2_sharp;
    bayer_stats->u.fw_stats.bf_v_sharp = primary_kernel->u.fw_stats.bf_v_sharp;
  }

  // secondary kernel if required
  if (stats_info->stats_type_mask & STATS_BF_SCL) {
    bayer_stats->u.grid_stats.horizontal_regions = secondary_kernel->u.grid_stats.bf_region_h_num;
    bayer_stats->u.grid_stats.vertical_regions = secondary_kernel->u.grid_stats.bf_region_v_num;
    bayer_stats->u.grid_stats.use_max_fv[1] = secondary_kernel->u.grid_stats.use_max_fv;
    bayer_stats->u.grid_stats.red_sum[1] = secondary_kernel->u.grid_stats.bf_r_sum;
    bayer_stats->u.grid_stats.blue_sum[1] = secondary_kernel->u.grid_stats.bf_b_sum;
    bayer_stats->u.grid_stats.gr_sum[1] = secondary_kernel->u.grid_stats.bf_gr_sum;
    bayer_stats->u.grid_stats.gb_sum[1] = secondary_kernel->u.grid_stats.bf_gb_sum;
    bayer_stats->u.grid_stats.red_sharpness[1] = secondary_kernel->u.grid_stats.bf_r_sharp;
    bayer_stats->u.grid_stats.blue_sharpness[1] = secondary_kernel->u.grid_stats.bf_b_sharp;
    bayer_stats->u.grid_stats.gr_sharpness[1] = secondary_kernel->u.grid_stats.bf_gr_sharp;
    bayer_stats->u.grid_stats.gb_sharpness[1] = secondary_kernel->u.grid_stats.bf_gb_sharp;
    bayer_stats->u.grid_stats.red_count[1] = secondary_kernel->u.grid_stats.bf_r_num;
    bayer_stats->u.grid_stats.blue_count[1] = secondary_kernel->u.grid_stats.bf_b_num;
    bayer_stats->u.grid_stats.gr_count[1] = secondary_kernel->u.grid_stats.bf_gr_num;
    bayer_stats->u.grid_stats.gb_count[1] = secondary_kernel->u.grid_stats.bf_gb_num;
    bayer_stats->u.grid_stats.red_max_fv[1] = secondary_kernel->u.grid_stats.bf_r_max_fv;
    bayer_stats->u.grid_stats.blue_max_fv[1] = secondary_kernel->u.grid_stats.bf_b_max_fv;
    bayer_stats->u.grid_stats.gr_max_fv[1] = secondary_kernel->u.grid_stats.bf_gr_max_fv;
    bayer_stats->u.grid_stats.gb_max_fv[1] = secondary_kernel->u.grid_stats.bf_gb_max_fv;

  }
  return rc;
}

/** af_biz_map_aec_info: update aec info in input sturcture
 *
 *  @af: internal AF data structure
 *
 *  @aec_info: aec input send by aec
 *
 *  Return: 0 - Success <1 - Failure
 **/
static int af_biz_map_aec_info(af_biz_internal *af,
  af_input_from_aec_t *aec_info) {
  int rc = 0;
  uint32_t i;
  af->af_input.aec_info.apex_values.av = aec_info->Av_af;
  af->af_input.aec_info.apex_values.bv = aec_info->Bv_af;
  af->af_input.aec_info.apex_values.sv = aec_info->Sv_af;
  af->af_input.aec_info.apex_values.tv = aec_info->Tv_af;
  af->af_input.aec_info.apex_values.ev = aec_info->Ev_af;
  af->af_input.aec_info.comp_luma = aec_info->comp_luma;
  af->af_input.aec_info.converge_percent =
    (aec_info->aec_settled == 1? 100.0f : 0.0f); // for now 100% is settled
  af->af_input.aec_info.cur_exp_time = aec_info->exp_time;
  af->af_input.aec_info.preview_fps = aec_info->preview_fps/AF_Q8;
  af->af_input.aec_info.cur_gain = aec_info->cur_real_gain;
  af->af_input.aec_info.cur_luma = aec_info->cur_luma;
  af->af_input.aec_info.exp_index = aec_info->exp_index;
  af->af_input.aec_info.frame_period =
    af->af_input.aec_info.preview_fps == 0 ?
    0 : (SEC_TO_MS/af->af_input.aec_info.preview_fps);
  af->af_input.aec_info.lux_index = aec_info->lux_idx;
  af->af_input.aec_info.max_exp_table_entry = aec_info->exp_tbl_val;
  af->af_input.aec_info.target_luma = aec_info->target_luma;
  af->af_input.aec_info.pixels_per_region =
    (uint32)aec_info->pixels_per_region;
  for (i = 0; i < MAX_YUV_STATS_NUM; i++)
  {
      // Copy luma values.
      af->af_input.aec_info.luma_values[i] = (uint8)aec_info->SY_data.SY[i];
  }
 // we are not using lots of values from aec input
  return rc;
}

/** af_biz_map_sensor_info: update aec info in input sturcture
 *
 *  @af: internal AF data structure
 *
 *  @aec_info: aec input send by aec
 *
 *  Return: 0 - Success <1 - Failure
 **/
static int af_biz_map_sensor_info(af_biz_internal *af,
  af_input_from_sensor_t *sensor_info) {
  Q3A_MEMCPY(&af->sensor_input, sensor_info,
    sizeof(af_input_from_sensor_t));
  int rc = 0;
  af->af_input.img_sensor_info.camif.height = sensor_info->sensor_res_height;
  af->af_input.img_sensor_info.camif.width = sensor_info->sensor_res_width;
  af->af_input.img_sensor_info.focal_length =
    sensor_info->actuator_info.focal_length;
  af->af_input.img_sensor_info.focus_distance =
    sensor_info->actuator_info.af_total_f_dist;
  af->af_input.img_sensor_info.f_number = sensor_info->actuator_info.af_f_num;
  af->af_input.img_sensor_info.max_preview_fps = sensor_info->max_preview_fps;
  af->af_input.img_sensor_info.pixel_size =
    sensor_info->actuator_info.af_f_pix;
  af->af_input.img_sensor_info.actuator_sensitivity =
    sensor_info->actuator_sensitivity;
  //  some param not passed in core
  return rc;
}

/** af_biz_map_gyro_info: update aec info in input sturcture
 *
 *  @af: internal AF data structure
 *
 *  @aec_info: aec input send by aec
 *
 *  Return: 0 - Success <1 - Failure
 **/
static int af_biz_map_gyro_info(af_biz_internal *af,
  af_input_from_gyro_t *gyro_info) {
  int rc = 0;
  af->af_input.motion_sensor_info.gyro_info.enabled = gyro_info->float_ready;
  AF_HIGH(" sample_index %f %f %f %d", gyro_info->flt[0], gyro_info->flt[1], gyro_info->flt[2], af->af_input.motion_sensor_info.gyro_info.sample_index);
  af->af_input.motion_sensor_info.gyro_info.angular_velocity_x[af->af_input.motion_sensor_info.gyro_info.sample_index] = gyro_info->flt[0];
  af->af_input.motion_sensor_info.gyro_info.angular_velocity_y[af->af_input.motion_sensor_info.gyro_info.sample_index] = gyro_info->flt[1];
  af->af_input.motion_sensor_info.gyro_info.angular_velocity_z[af->af_input.motion_sensor_info.gyro_info.sample_index] = gyro_info->flt[2];
  af->af_input.motion_sensor_info.gyro_info.sample_index += 1;
  af->af_input.motion_sensor_info.gyro_info.sample_index %= AF_GYRO_MAX_SAMPLE_SIZE;
  return rc;
}

/** af_biz_map_gravity_vector_info: update aec info in input sturcture
 *
 *  @af: internal AF data structure
 *
 *  @aec_info: aec input send by aec
 *
 *  Return: 0 - Success <1 - Failure
 **/
static int af_biz_map_gravity_vector_info(af_biz_internal *af,
  af_input_from_gravity_vector_t *gravity_info) {
  int rc = 0;
  af->af_input.motion_sensor_info.gravity_info.enabled =
    gravity_info->is_ready;
  af->af_input.motion_sensor_info.gravity_info.gravity_vector_x =
    gravity_info->g_vector[0];
  af->af_input.motion_sensor_info.gravity_info.gravity_vector_y =
    gravity_info->g_vector[1];
  af->af_input.motion_sensor_info.gravity_info.gravity_vector_z =
    gravity_info->g_vector[2];
  af->af_input.motion_sensor_info.accel_info.enabled = gravity_info->is_ready;
  af->af_input.motion_sensor_info.accel_info.accel_x = gravity_info->lin_accel[0];
  af->af_input.motion_sensor_info.accel_info.accel_y = gravity_info->lin_accel[1];
  af->af_input.motion_sensor_info.accel_info.accel_z = gravity_info->lin_accel[2];
  //  some param not passed in core
  return rc;
}

/** af_biz_map_pdaf_info: update pdaf info in input sturcture
 *
 *  @af: internal AF data structure
 *
 *  @depth_service: depth input
 *
 *  Return: 0 - Success <1 - Failure
 **/
static int af_biz_map_pdaf_info(af_biz_internal *af,
  depth_service_output_t *depth_service) {
  int i,rc = 0;
  int size =  depth_service->input.info.x_win_num * depth_service->input.info.y_win_num;
  if (size > MAX_DEPTH_WINDOW_SIZE) {
    AF_ERR(" size %d > %d", size, MAX_DEPTH_WINDOW_SIZE);
    rc = -1;
    return rc;
  }
  af->af_input.stats_info.mask = AF_CORE_STATS_MASK_DEPTH_ASSISTED;
  af->af_input.stats_info.depth_stats.mask |= AF_DAAF_PDAF;
  af->af_input.stats_info.depth_stats.pdaf_info.daaf_info.frame_id = depth_service->input.info.frame_id;
  af->af_input.stats_info.depth_stats.pdaf_info.daaf_info.h_num = depth_service->input.info.x_win_num;
  af->af_input.stats_info.depth_stats.pdaf_info.daaf_info.v_num = depth_service->input.info.y_win_num;
  af->af_input.stats_info.depth_stats.pdaf_info.daaf_info.status = depth_service->is_ready;
  af->af_input.stats_info.depth_stats.pdaf_info.daaf_info.af_focus_pos_dac = depth_service->input.info.af.af_focus_pos_dac;
  for (i = 0; i < size; i++) {
    af->af_input.stats_info.depth_stats.pdaf_info.pdaf_update[i].defocus =
      depth_service->input.u.pdaf_info.defocus[i].defocus;
    af->af_input.stats_info.depth_stats.pdaf_info.pdaf_update[i].df_conf_level =
      depth_service->input.u.pdaf_info.defocus[i].df_conf_level;
    af->af_input.stats_info.depth_stats.pdaf_info.pdaf_update[i].phase_diff =
      depth_service->input.u.pdaf_info.defocus[i].phase_diff;
  }
  return rc;
}
/** af_biz_map_tof_info: update TOF info in input sturcture
 *
 *  @af: internal AF data structure
 *
 *  @depth_service: depth input
 *
 *  Return: 0 - Success <1 - Failure
 **/
static int af_biz_map_tof_info(af_biz_internal *af,
  depth_service_output_t *depth_service) {
  int i, rc = 0;
  int size =  depth_service->input.info.x_win_num * depth_service->input.info.y_win_num;
  if (size > MAX_DEPTH_WINDOW_SIZE) {
    AF_ERR(" size %d > %d", size, MAX_DEPTH_WINDOW_SIZE);
    rc = -1;
    return rc;
  }
  af->af_input.stats_info.mask = AF_CORE_STATS_MASK_DEPTH_ASSISTED;
  af->af_input.stats_info.depth_stats.mask |= AF_DAAF_TOF;
  af->af_input.stats_info.depth_stats.tof_info.daaf_info.frame_id = depth_service->input.info.frame_id;
  af->af_input.stats_info.depth_stats.tof_info.daaf_info.h_num = depth_service->input.info.x_win_num;
  af->af_input.stats_info.depth_stats.tof_info.daaf_info.v_num = depth_service->input.info.y_win_num;
  af->af_input.stats_info.depth_stats.tof_info.daaf_info.status = depth_service->is_ready;
  af->af_input.stats_info.depth_stats.tof_info.daaf_info.af_focus_pos_dac =
    depth_service->input.info.af.af_focus_pos_dac;

  af->af_input.stats_info.depth_stats.tof_info.frame_id =
    depth_service->input.u.tof_info.frame_id;
  af->af_input.stats_info.depth_stats.tof_info.distance =
    depth_service->input.u.tof_info.distance;
  af->af_input.stats_info.depth_stats.tof_info.confidence =
    depth_service->input.u.tof_info.confidence;
  af->af_input.stats_info.depth_stats.tof_info.near_limit =
    depth_service->input.u.tof_info.near_limit;
  af->af_input.stats_info.depth_stats.tof_info.far_limit =
    depth_service->input.u.tof_info.far_limit;
  af->af_input.stats_info.depth_stats.tof_info.timestamp =
    depth_service->input.u.tof_info.timestamp;
  af->af_input.stats_info.depth_stats.tof_info.max_distance =
    depth_service->input.u.tof_info.max_distance;

   Q3A_MEMCPY(&(af->af_input.stats_info.depth_stats.tof_info.daaf_info.cal_data),
      &(depth_service->input.cal_data), sizeof(depth_cal_data_t));
  return rc;
}

/** af_biz_map_dciaf_info: update DCRF info in input sturcture
 *
 *  @af: internal AF data structure
 *
 *  @depth_service: depth input
 *
 *  Return: 0 - Success <1 - Failure
 **/
static int af_biz_map_dciaf_info(af_biz_internal *af,
  depth_service_output_t *depth_service) {
  int i, rc = 0;
  int size =  depth_service->input.info.x_win_num * depth_service->input.info.y_win_num;
  if (size > MAX_DEPTH_WINDOW_SIZE) {
    AF_ERR(" size %d > %d", size, MAX_DEPTH_WINDOW_SIZE);
    rc = -1;
    return rc;
  }
  af->af_input.stats_info.mask = AF_CORE_STATS_MASK_DEPTH_ASSISTED;
  af->af_input.stats_info.depth_stats.mask |= AF_DAAF_DCIAF;

  af->af_input.stats_info.depth_stats.dciaf_info.daaf_info.frame_id = depth_service->input.info.frame_id;
  af->af_input.stats_info.depth_stats.dciaf_info.daaf_info.h_num = depth_service->input.info.x_win_num;
  af->af_input.stats_info.depth_stats.dciaf_info.daaf_info.v_num = depth_service->input.info.y_win_num;
  af->af_input.stats_info.depth_stats.dciaf_info.daaf_info.status = depth_service->is_ready;
  af->af_input.stats_info.depth_stats.dciaf_info.daaf_info.af_focus_pos_dac =
    depth_service->input.info.af.af_focus_pos_dac;

  af->af_input.stats_info.depth_stats.dciaf_info.timestamp =
    depth_service->input.u.dciaf_info.timestamp;
  af->af_input.stats_info.depth_stats.dciaf_info.confidence =
    depth_service->input.u.dciaf_info.confidence;
  af->af_input.stats_info.depth_stats.dciaf_info.distance_in_mm =
    depth_service->input.u.dciaf_info.distance_in_mm;
  af->af_input.stats_info.depth_stats.dciaf_info.focused_roi.x =
    depth_service->input.u.dciaf_info.focused_roi.left;
  af->af_input.stats_info.depth_stats.dciaf_info.focused_roi.y =
    depth_service->input.u.dciaf_info.focused_roi.top;
  af->af_input.stats_info.depth_stats.dciaf_info.focused_roi.height =
    depth_service->input.u.dciaf_info.focused_roi.height;
  af->af_input.stats_info.depth_stats.dciaf_info.focused_roi.width =
    depth_service->input.u.dciaf_info.focused_roi.width;

  af->af_input.stats_info.depth_stats.dciaf_info.focused_x =
    depth_service->input.u.dciaf_info.focused_x;
  af->af_input.stats_info.depth_stats.dciaf_info.focused_y =
    depth_service->input.u.dciaf_info.focused_y;
  Q3A_MEMCPY(&(af->af_input.stats_info.depth_stats.dciaf_info.daaf_info.cal_data),
       &(depth_service->input.cal_data), sizeof(depth_cal_data_t));
  return rc;
}
/** af_biz_map_stream_crop_info: update stream crop information and trigger
 *  reconfiguration of the RoI.
 *
 *  @af: internal AF data structure
 *
 *  @stream_crop: crop information sent by VFE
 *
 *  Return: 0 - Success <1 - Failure
 **/
static int af_biz_map_stream_crop_info(af_biz_internal *af,
  af_stream_crop_t *stream_crop) {
  int rc = 0;
  int camif_height = af->af_input.img_sensor_info.camif.height;
  int camif_width = af->af_input.img_sensor_info.camif.width;

  if(memcmp(&af->af_stream_crop, stream_crop, sizeof(af_stream_crop_t))) {
    AF_LOW("CROP data: ppX:%d, ppY:%d, ppDX:%d, ppDY:%d \t,"
        "vfeX:%d, vfeY%d, vfDX:%d, vfeDY:%d",
      stream_crop->pp_x,
      stream_crop->pp_y,
      stream_crop->pp_crop_out_x,
      stream_crop->pp_crop_out_y,
      stream_crop->vfe_map_x,
      stream_crop->vfe_map_y,
      stream_crop->vfe_map_width,
      stream_crop->vfe_map_height
      );

    Q3A_MEMCPY(&af->af_stream_crop, stream_crop, sizeof(af_stream_crop_t));
    af->af_input.roi_info.roi_updated = TRUE;
  }
  // calculate zoom
  af->af_input.zoom_ratio = 1.0f;

  if (camif_width)
    af->af_input.zoom_ratio =
      (float)(af->af_stream_crop.vfe_map_width)/(float)camif_width;

  if (af->preview_size.width)
    af->af_input.zoom_ratio = af->af_input.zoom_ratio *
     (float)(af->af_stream_crop.pp_crop_out_x)/(float)af->preview_size.width;
  af->af_input.zoom_ratio = (1.0 / af->af_input.zoom_ratio);
  AF_LOW(" zoom_ratio =%f", af->af_input.zoom_ratio);
  return rc;
}

/** af_biz_map_isp_capability:  map isp caps into core structure.
 *
 *  @af: internal AF data structure
 *
 *  @stats_caps_info:  isp stats caps info
 *
 *  Return: 0 - Success <1 - Failure
 **/
static int af_biz_map_isp_capability(af_biz_internal *af,
  mct_stats_info_t *stats_caps_info) {

  uint8  stats_caps = AF_STATS_CAPS_DEFAULT;
  uint8 i;
  int rc = 0;
  /* Map caps info */
  if (stats_caps_info->stats_mask & (1 << MSM_ISP_STATS_BF) &&
      stats_caps_info->bf_caps.bf_mask & MCT_EVENT_STATS_BF_SINGLE) {
    stats_caps |= AF_STATS_CAPS_DEFAULT;
  } else {
    stats_caps &= ~AF_STATS_CAPS_DEFAULT;
  }

  if (stats_caps_info->stats_mask & (1 << MSM_ISP_STATS_BF) &&
      stats_caps_info->bf_caps.bf_mask & MCT_EVENT_STATS_BF_FW) {
    stats_caps |= AF_STATS_CAPS_FW;
  } else {
    stats_caps &= ~AF_STATS_CAPS_FW;
  }

  /* Check if bf scale is supported.*/
  if (stats_caps_info->stats_mask & (1 << MSM_ISP_STATS_BF_SCALE)) {
    stats_caps |= AF_STATS_CAPS_SCALE;
  } else {
    stats_caps &= ~AF_STATS_CAPS_SCALE;
  }
  af->af_input.hw_cap_info.capability_mask = stats_caps;

  if (stats_caps & AF_STATS_CAPS_FW) {
    af->af_input.hw_cap_info.roi_info.roi_type = AF_CORE_ROI_SYSTEM_FLOATING;
    af->af_input.hw_cap_info.roi_info.u.fw_info.num_of_zones = 1;
    af->af_input.hw_cap_info.roi_info.u.fw_info.zone_info[0].max_windows =
      stats_caps_info->bf_caps.bf_roi_caps.max_total_grids;
    af->af_input.hw_cap_info.roi_info.u.fw_info.zone_info[0].max_win_width =
      stats_caps_info->bf_caps.bf_roi_caps.max_grid_width;
    af->af_input.hw_cap_info.roi_info.u.fw_info.zone_info[0].max_win_height =
      stats_caps_info->bf_caps.bf_roi_caps.max_grid_height;
    af->af_input.hw_cap_info.roi_info.u.fw_info.zone_info[0].min_win_width =
      stats_caps_info->bf_caps.bf_roi_caps.min_grid_width;
    af->af_input.hw_cap_info.roi_info.u.fw_info.zone_info[0].min_win_height =
      stats_caps_info->bf_caps.bf_roi_caps.min_grid_height;
    af->af_input.hw_cap_info.roi_info.u.fw_info.zone_info[0].min_hor_offset =
      stats_caps_info->bf_caps.bf_roi_caps.min_hor_offset;
    af->af_input.hw_cap_info.roi_info.u.fw_info.zone_info[0].max_hor_offset =
      stats_caps_info->bf_caps.bf_roi_caps.max_hor_offset;
    af->af_input.hw_cap_info.roi_info.u.fw_info.zone_info[0].min_ver_offset =
      stats_caps_info->bf_caps.bf_roi_caps.min_ver_offset;
    af->af_input.hw_cap_info.roi_info.u.fw_info.zone_info[0].max_ver_offset =
      stats_caps_info->bf_caps.bf_roi_caps.max_ver_offset;

    af->af_input.hw_cap_info.filter_info.num_of_kernels =
      stats_caps_info->bf_caps.bf_fw_caps.num_filter_caps;
    uint8 num_of_kernel = stats_caps_info->bf_caps.bf_fw_caps.num_filter_caps;
    for ( i = 0; i < num_of_kernel && i < AF_MAX_NUM_OF_KERNELS; i++ ) {
      /* filter coeff size */
      af->af_input.hw_cap_info.filter_info.kernel_info[i].num_fir_a_kernel =
        (uint8)stats_caps_info->bf_caps.bf_fw_caps.filter_caps[i].num_fir_a_kernel;
      af->af_input.hw_cap_info.filter_info.kernel_info[i].num_iir_a_kernel =
        (uint8)stats_caps_info->bf_caps.bf_fw_caps.filter_caps[i].num_iir_a_kernel;
      af->af_input.hw_cap_info.filter_info.kernel_info[i].num_iir_b_kernel =
        (uint8)stats_caps_info->bf_caps.bf_fw_caps.filter_caps[i].num_iir_b_kernel;

      af->af_input.hw_cap_info.filter_info.kernel_info[i].downscale_supported =
        stats_caps_info->bf_caps.bf_fw_caps.filter_caps[i].downscale_supported;
      af->af_input.hw_cap_info.filter_info.kernel_info[i].is_coring_variable =
        stats_caps_info->bf_caps.bf_fw_caps.filter_caps[i].is_coring_variable;

      /* set mask for filter type baaf support both in each */
      af->af_input.hw_cap_info.filter_info.kernel_info[i].mask =
        (AF_CORE_FILTER_FIR|AF_CORE_FILTER_IIR);
    }
  } else { /* hw caps for grid kernel */
    af->af_input.hw_cap_info.roi_info.roi_type = AF_CORE_ROI_SYSTEM_GRID;
    if (stats_caps_info->kernel_size == MCT_EVENT_STATS_HPF_2X11) {
      af->af_input.hw_cap_info.filter_info.kernel_info[0].num_fir_a_kernel = MAX_HPF_2x11_BUFF_SIZE;
    } else if (stats_caps_info->kernel_size == MCT_EVENT_STATS_HPF_2X5) {
      af->af_input.hw_cap_info.filter_info.kernel_info[0].num_fir_a_kernel = MAX_HPF_2x5_BUFF_SIZE;
    }
    if (stats_caps & AF_STATS_CAPS_SCALE) {
      af->af_input.hw_cap_info.filter_info.num_of_kernels = 2;
    } else {
      af->af_input.hw_cap_info.filter_info.num_of_kernels = 1;
    }
  }
  return rc;
}

/** af_biz_initialize_internal_data:
*
*    Pack output of AF process
*
*    @af: internal AF data
*
*
**/
void af_biz_initialize_internal_data(af_biz_internal *af) {
  af->af_inited = FALSE;
  af->af_ctrl_mode = 1;
  af->hfr_mode = 0;
  Q3A_MEMSET(&af->af_mode, 0, sizeof(af_mode_info_t));
  af->af_mode.mode = AF_MODE_NOT_SUPPORTED;
  af->af_mode.prev_mode = AF_MODE_NOT_SUPPORTED;
  af->af_UI_mode = AF_MODE_AUTO;
  af->bestshot_d.curr_mode = AF_BESTSHOT_OFF;
  af->af_input.stream_type = AF_RUN_MODE_INIT;
  af->sof_id = 0;
  af->frame_id = 0;
  af->wait_for_aec_est = FALSE;
  af->sw_filter_type = AFS_OFF;
  af->isp_reconfig_needed = FALSE;
  af->roi_changed = FALSE;
  af->status = AF_STATUS_INVALID;
  // Input Struct Init
  af->af_input.zoom_ratio = 1.9f;
  af->af_input.hw_cap_info.filter_info.kernel_info[0].num_fir_a_kernel = MAX_HPF_2x11_BUFF_SIZE;
  af->af_input.force_roi_reconfig = FALSE;
  af->af_input.enable_caf_lock = FALSE;
  af->af_input.debug_data = af->af_out.debug_info->debug_data;
  af->af_input.debug_data_max_size = AF_DEBUG_DATA_SIZE;
  af->af_mode_changed = FALSE;
  af->af_input.force_paaf_enable = TRUE;
  af->lens_reset_on_init = FALSE;
  af->move_lens_to_def_pos = FALSE;
} /* af_biz_initialize_internal_data */

/** af_biz_handle_move_lens:
*
*    Pack output of AF process
*
*    @af: internal AF data
*
*    @af_out: output AF data
*
**/
void af_biz_handle_move_lens(af_biz_internal *af,
  af_core_lens_move_type *core_lens_info) {
  af_move_lens_t move_lens;
  Q3A_MEMSET(&move_lens, 0, sizeof(af_move_lens_t));
  /* create a request to move lens */
  move_lens.move_lens = TRUE;
  if (core_lens_info->use_dac_value) {
    // NOTE dac mode is not used as of now
    move_lens.use_dac_value = TRUE;
    move_lens.num_of_interval = 1;
    move_lens.pos[0] = core_lens_info->u.dac.dac;
    move_lens.delay[0] = 0;
  } else {
    move_lens.use_dac_value = FALSE;
    move_lens.num_of_steps = core_lens_info->u.steps.steps_to_move;
    if (core_lens_info->u.steps.direction == AF_CORE_MOVE_LENS_NEAR) {
      move_lens.direction = AF_MOVE_NEAR;
    } else if (core_lens_info->u.steps.direction == AF_CORE_MOVE_LENS_FAR) {
      move_lens.direction    = AF_MOVE_FAR;
    }
  }
  move_lens.fovc_enabled = core_lens_info->fovc_enabled;
  move_lens.mag_factor = core_lens_info->mag_factor;
  af_biz_util_update_output_data(af, AF_OUTPUT_MOVE_LENS, &move_lens);
} /* af_biz_handle_move_lens */

/** af_biz_handle_stats_config_paaf:
*
*    Pack output of AF process
*
*    @af: internal AF data
*
*    @af_out: output AF data
*
**/
void af_biz_handle_stats_config_paaf(af_biz_internal *af,
  af_core_config_stats_engine_paaf *core_stats_info,
  swaf_config_data_t *sw_stats_config) {
  sw_stats_config->frame_id = af->af_out.frame_id;
  uint32 i;
  if (core_stats_info->filter_info.kernel_info[0].fir.enabled == FALSE &&
    core_stats_info->filter_info.kernel_info[0].iir.enabled == FALSE) {
    sw_stats_config->sw_filter_type = AFS_OFF;
    sw_stats_config->enable = FALSE;
  } else if (core_stats_info->filter_info.kernel_info[0].fir.enabled == TRUE) {
    sw_stats_config->sw_filter_type = AFS_ON_FIR;
    sw_stats_config->enable = TRUE;
  } else if (core_stats_info->filter_info.kernel_info[0].iir.enabled == TRUE) {
    sw_stats_config->sw_filter_type = AFS_ON_IIR;
    sw_stats_config->enable = TRUE;
  }

  // store filter type
  af->sw_filter_type = sw_stats_config->sw_filter_type;
  // ROI config
  sw_stats_config->roi.x = (uint16_t)core_stats_info->roi.x;
  sw_stats_config->roi.y = (uint16_t)core_stats_info->roi.y;
  sw_stats_config->roi.dx = (uint16_t)core_stats_info->roi.width;
  sw_stats_config->roi.dy = (uint16_t)core_stats_info->roi.height;

  double fv_min, fv_min_hi, fv_min_lo;
  af_tuning_sw_stats_adapter_t *tuning = &af->af_input.tuning_info.af_swaf_config;
  // TODO, i have fixed it to get from default
  fv_min_hi = tuning->sw_hpf_default.iir.fv_min_hi;
  fv_min_lo = tuning->sw_hpf_default.iir.fv_min_lo;

  if (sw_stats_config->sw_filter_type == AFS_ON_FIR) {
    sw_stats_config->coeff_len =
      core_stats_info->filter_info.kernel_info[0].fir.coeff_size;
    for (i = 0; i < sw_stats_config->coeff_len && i < MAX_SWAF_COEFFFIR_NUM; i++) {
      sw_stats_config->coeff_fir[i] =
        core_stats_info->filter_info.kernel_info[0].fir.a[i];
    }
    // TODO, i have fixed it to get from default
    fv_min_hi = tuning->sw_hpf_default.fir.fv_min_hi;
    fv_min_lo = tuning->sw_hpf_default.fir.fv_min_lo;
  } else if (sw_stats_config->sw_filter_type == AFS_ON_IIR) {
    sw_stats_config->coeff_len =
      core_stats_info->filter_info.kernel_info[0].iir.coeffa_size;
    for (i = 0; i < sw_stats_config->coeff_len && i < MAX_SWAF_COEFFA_NUM; i++) {
      sw_stats_config->coeffa[i] =
        (double)core_stats_info->filter_info.kernel_info[0].iir.a[i];
      sw_stats_config->coeffb[i] =
        (double)core_stats_info->filter_info.kernel_info[0].iir.b[i];
    }
    // TODO, i have fixed it to get from default
    fv_min_hi = tuning->sw_hpf_default.iir.fv_min_hi;
    fv_min_lo = tuning->sw_hpf_default.iir.fv_min_lo;

  }
  // calculate fv_min using tuning and aec info
  /* Calculate FV_min wrt. to Lux Index */
  float lux_idx = af->af_input.aec_info.lux_index;
  float lux_trig_hi = tuning->sw_fv_min_lux_trig_hi;
  float lux_trig_lo = tuning->sw_fv_min_lux_trig_lo;
  /* Interpolate FV_min wrt to Lux index trigger point */
  if (lux_idx >= tuning->sw_fv_min_lux_trig_lo ) {
    fv_min = fv_min_lo;
  } else if (lux_idx <= tuning->sw_fv_min_lux_trig_hi) {
    fv_min = fv_min_hi;
  } else {
    fv_min = fv_min_hi + (fv_min_lo - fv_min_hi) *
      ((lux_idx - lux_trig_hi) / (lux_trig_lo - lux_trig_hi));
  }
  sw_stats_config->fv_min = fv_min;

}

/** af_biz_roi_boundary_check:
*
*    boundary check for ROI
*
*    @core_stats_info:  algo config
*
*    @fw_roi_caps:  fw roi hw caps
*
**/
void af_biz_roi_boundary_check(
  af_core_config_stats_engine_bayer_type* core_stats_info,
  af_core_hw_cap_roi_fw_type *fw_roi_caps) {

  AF_LOW(" ROI before boundary check x:%f, y=%f, dx=%f, dy=%f",
    core_stats_info->roi_info.u.fw_info.roi.x,
    core_stats_info->roi_info.u.fw_info.roi.y,
    core_stats_info->roi_info.u.fw_info.roi.width,
    core_stats_info->roi_info.u.fw_info.roi.height);

  /* Width height should be as per isp min/max rules. */
  if (core_stats_info->roi_info.u.fw_info.roi.width <
    fw_roi_caps->zone_info[0].min_win_width) {
    core_stats_info->roi_info.u.fw_info.roi.width =
      fw_roi_caps->zone_info[0].min_win_width;
  }
  if (core_stats_info->roi_info.u.fw_info.roi.width >
    fw_roi_caps->zone_info[0].max_win_width) {
    core_stats_info->roi_info.u.fw_info.roi.width =
      fw_roi_caps->zone_info[0].max_win_width;
  }
  if (core_stats_info->roi_info.u.fw_info.roi.height <
    fw_roi_caps->zone_info[0].min_win_height) {
    core_stats_info->roi_info.u.fw_info.roi.height =
      fw_roi_caps->zone_info[0].min_win_height;
  }
  if (core_stats_info->roi_info.u.fw_info.roi.height >
    fw_roi_caps->zone_info[0].max_win_height) {
    core_stats_info->roi_info.u.fw_info.roi.height =
      fw_roi_caps->zone_info[0].max_win_height;
  }
  /* Start should be more than min offset, and use 2 to for
        even/odd check which isp perform. */
  if (core_stats_info->roi_info.u.fw_info.roi.x <
    fw_roi_caps->zone_info[0].min_hor_offset) {
    core_stats_info->roi_info.u.fw_info.roi.x =
      fw_roi_caps->zone_info[0].min_hor_offset + 2;
  }
  /*End should be inside max offset plus min width,
       2 for safety for even/odd check */
  if (core_stats_info->roi_info.u.fw_info.roi.x +
    core_stats_info->roi_info.u.fw_info.roi.width >
    fw_roi_caps->zone_info[0].max_hor_offset +
    fw_roi_caps->zone_info[0].min_win_width) {
    core_stats_info->roi_info.u.fw_info.roi.x =
      fw_roi_caps->zone_info[0].max_hor_offset +
      fw_roi_caps->zone_info[0].min_win_width -
      core_stats_info->roi_info.u.fw_info.roi.width - 2;
  }
  if (core_stats_info->roi_info.u.fw_info.roi.y <
    fw_roi_caps->zone_info[0].min_ver_offset) {
    core_stats_info->roi_info.u.fw_info.roi.y =
      fw_roi_caps->zone_info[0].min_ver_offset + 2;
  }
  if (core_stats_info->roi_info.u.fw_info.roi.y +
    core_stats_info->roi_info.u.fw_info.roi.height >
    fw_roi_caps->zone_info[0].max_ver_offset +
    fw_roi_caps->zone_info[0].min_win_height) {
    core_stats_info->roi_info.u.fw_info.roi.y =
      fw_roi_caps->zone_info[0].max_ver_offset +
      fw_roi_caps->zone_info[0].min_win_height -
      core_stats_info->roi_info.u.fw_info.roi.height - 2;
  }
  AF_LOW(" ROI after boundary check x:%f, y=%f, dx=%f, dy=%f",
    core_stats_info->roi_info.u.fw_info.roi.x,
    core_stats_info->roi_info.u.fw_info.roi.y,
    core_stats_info->roi_info.u.fw_info.roi.width,
    core_stats_info->roi_info.u.fw_info.roi.height);
}

/** af_biz_convert_fw_roi_calc_config:
 *
 *  Calculate floating window roi config
 *
 *  @af: internal AF data
 *
 *  @fw_roi: Roi from algo
 *
 *  @output: [OUT] roi config to generate fw
 **/
static boolean af_biz_convert_fw_roi_calc_config(af_biz_internal *af,
  af_core_config_roi_fw_type *fw_roi, af_biz_fw_int_param_t *output)
{
  uint32_t                roi_w = 0,
                          roi_h = 0,
                          total_row = 0,
                          total_col = 0;
  int32_t                 gap_w = 0,
                          gap_h = 0;
  af_core_hw_cap_roi_fw_type *fw_caps =
    &af->af_input.hw_cap_info.roi_info.u.fw_info;
  float grid_size_h = fw_roi->grid_size_h;
  float grid_size_v = fw_roi->grid_size_v;
  float grid_gap_h  = fw_roi->grid_gap_h;
  float grid_gap_v  = fw_roi->grid_gap_v;

  /* Calculate Row, Col using grid size. */
  total_col = (uint32_t)(1 / grid_size_h);
  total_row = (uint32_t)(1 / grid_size_v);

  /* Adjust grid gap to make sure it is less than
     equal grid size in case of negative gap. */
  if (grid_gap_h < 0 && fabs(grid_size_h) < fabs(grid_gap_h)) {
    grid_gap_h = (-1.0) * grid_size_h;
  }
  if (grid_gap_v < 0 && fabs(grid_size_v) < fabs(grid_gap_v)) {
    grid_gap_v = (-1.0) * grid_size_v;
  }

  /* Lets adjust size to make room for gap,
    Constraint should be all size and grid will add to 1
    i.e (n-1)(S`+G) + S` = 1, S` = S + D, D= G(S-1) */
  float delta_h = grid_gap_h * (grid_size_h - 1);
  float delta_v = grid_gap_v * (grid_size_v - 1);
  grid_size_h = grid_size_h + delta_h;
  grid_size_v = grid_size_v + delta_v;


  roi_w = (uint32_t)(fw_roi->roi.width * grid_size_h);
  roi_h = (uint32_t)(fw_roi->roi.height * grid_size_v);
  gap_w = (int32_t)(grid_gap_h * fw_roi->roi.width);
  gap_h = (int32_t)(grid_gap_v * fw_roi->roi.height);

  AF_LOW("size_h=%f, size_v=%f, gap_h=%f, gap_v=%f, delta_h=%f, delta_v=%f",
    grid_size_h, grid_size_v, grid_gap_h, grid_gap_v, delta_h, delta_v);

  AF_LOW("roi_w=%d, roi_h=%d, gap_h=%d, gap_v=%d, t_row=%d, t_col=%d",
    roi_w, roi_h, gap_w, gap_h, total_row, total_col);

  if (grid_size_h <= 0 || grid_size_v <= 0) {
    AF_LOW("Grid Size become ngative after gap adjust");
    return FALSE;
  }

  /* Min window size check */
  if (roi_w < fw_caps->zone_info[0].min_win_width) {
    AF_LOW("Width is below isp limit");
    return FALSE;
  }
  if (roi_h < fw_caps->zone_info[0].min_win_height) {
    AF_LOW("Height is below isp limit");
    return FALSE;
  }

  /* Total window check. */
  if (total_row * total_col > fw_caps->zone_info[0].max_windows) {
    AF_LOW("total number of windows more than isp caps");
    return FALSE;
  }
  /* Fill output info */
  output->gap_h = gap_h;
  output->gap_w = gap_w;
  output->roi_h = roi_h;
  output->roi_w = roi_w;
  output->total_col = total_col;
  output->total_row = total_row;

  return TRUE;
}

/** af_biz_convert_diamond_roi:
 *
 *  Convert ROI into floating window for diamond type
 *
 *  @af: internal AF data
 *
 *  @fw_roi: Roi from algo
 *
 *  @af_fw_roi_cfg: floating window config for isp
 **/
static boolean af_biz_convert_diamond_roi(af_biz_internal *af,
  af_core_config_roi_fw_type *fw_roi,
  bf_fw_roi_cfg_t *af_fw_roi_cfg)
{
  int32_t rej_row, rej_col;
  int32_t  row = 0, col = 0, mid_col = 0, mid_row = 0;
  bf_fw_roi_dim_t  *af_fw_roi_dim = NULL;
  uint32_t          region_num = 0;
  uint32_t          abs_height = 0; /* Absolute height:(y+h), temp placeholder */
  af_biz_fw_int_param_t config;

  /* Get Floating window count and size info. */
  if (!af_biz_convert_fw_roi_calc_config(af,fw_roi, &config)) {
    AF_ERR("Invalid config, falling to default single window config");
    return FALSE;
  }

  /* Gnerate Floating windows. */
  af_fw_roi_cfg->is_valid = TRUE;
  af_fw_roi_cfg->num_bf_fw_roi_dim = 0;
  af_fw_roi_cfg->max_primary_abs_height = 0;
  af_fw_roi_cfg->max_secondary_abs_height = 0;
  mid_col = config.total_col / 2;
  mid_row = config.total_row / 2;
  AF_LOW("mid_row=%d, mid_col=%d", mid_row, mid_col);

  for (row = 0; row < (int32_t)config.total_row &&
    af_fw_roi_cfg->num_bf_fw_roi_dim < BF_FW_MAX_ROI_REGIONS; row++) {
    for (col = 0; col < (int32_t)config.total_col &&
      af_fw_roi_cfg->num_bf_fw_roi_dim < BF_FW_MAX_ROI_REGIONS; col++) {
      rej_row = abs(col - (int32_t)config.total_row/2);
      rej_col = abs(row - (int32_t)config.total_col/2);
      if (mid_row - rej_row < abs(mid_row - row)) {
        if (mid_col - rej_col < abs(mid_col - col)) {
          AF_LOW("Ignore row =%d, col=%d", row, col);
          continue;
        }
      }
      af_fw_roi_dim = &af_fw_roi_cfg->bf_fw_roi_dim[
        af_fw_roi_cfg->num_bf_fw_roi_dim++];
      af_fw_roi_dim->region = BF_FW_PRIMARY_REGION;
      af_fw_roi_dim->x =
        (uint32_t)fw_roi->roi.x + ((int32_t)config.roi_w + config.gap_w) * col;
      af_fw_roi_dim->y =
        (uint32_t)fw_roi->roi.y + ((int32_t)config.roi_h + config.gap_h) * row;
      af_fw_roi_dim->w = config.roi_w - 1;
      af_fw_roi_dim->h = config.roi_h - 1;
      af_fw_roi_dim->region_num = region_num++;
      af_fw_roi_dim->is_valid = TRUE;
      abs_height = af_fw_roi_dim->y + af_fw_roi_dim->h;
      if (abs_height > af_fw_roi_cfg->max_primary_abs_height) {
        af_fw_roi_cfg->max_primary_abs_height = abs_height;
      }
      AF_LOW("ROI[%d][%d, %d]: x=%d, y=%d, dx=%d, dy=%d, max_abs_y=%d",
        af_fw_roi_dim->region_num, row, col, af_fw_roi_dim->x,
        af_fw_roi_dim->y, af_fw_roi_dim->w, af_fw_roi_dim->h,
        af_fw_roi_cfg->max_primary_abs_height);
    }
  }

  return TRUE;
}

/** af_biz_convert_cross_roi:
 *
 *  Convert ROI into floating window for cross type
 *
 *  @af: internal AF data
 *
 *  @fw_roi: Roi from algo
 *
 *  @af_fw_roi_cfg: floating window config for isp
 **/
static boolean af_biz_convert_cross_roi(af_biz_internal *af,
  af_core_config_roi_fw_type *fw_roi,
  bf_fw_roi_cfg_t *af_fw_roi_cfg)
{
  uint32_t  row = 0, col = 0, mid_col = 0, mid_row = 0;
  bf_fw_roi_dim_t  *af_fw_roi_dim = NULL;
  uint32_t          region_num = 0;
  uint32_t          abs_height = 0; /* Absolute height:(y+h), temp placeholder */
  af_biz_fw_int_param_t config;

  /* Get Floating window count and size info. */
  if (!af_biz_convert_fw_roi_calc_config(af,fw_roi, &config)) {
    AF_ERR("Invalid config, falling to default single window config");
    return FALSE;
  }

  /* Gnerate Floating windows. */
  af_fw_roi_cfg->is_valid = TRUE;
  af_fw_roi_cfg->num_bf_fw_roi_dim = 0;
  af_fw_roi_cfg->max_primary_abs_height = 0;
  af_fw_roi_cfg->max_secondary_abs_height = 0;
  mid_col = config.total_col / 2;
  mid_row = config.total_row / 2;
  AF_LOW("mid_row=%d, mid_col=%d", mid_row, mid_col);
  for (row = 0; row < config.total_row &&
    af_fw_roi_cfg->num_bf_fw_roi_dim < BF_FW_MAX_ROI_REGIONS; row++) {
    for (col = 0; col < config.total_col &&
      af_fw_roi_cfg->num_bf_fw_roi_dim < BF_FW_MAX_ROI_REGIONS; col++) {
      if (row != mid_row && col != mid_col) {
        AF_LOW("Ignore row =%d, col=%d", row, col);
      } else {
        af_fw_roi_dim = &af_fw_roi_cfg->bf_fw_roi_dim[
          af_fw_roi_cfg->num_bf_fw_roi_dim++];
        af_fw_roi_dim->region = BF_FW_PRIMARY_REGION;
        af_fw_roi_dim->x =
          (uint32_t)fw_roi->roi.x + ((int32_t)config.roi_w + config.gap_w) * col;
        af_fw_roi_dim->y =
          (uint32_t)fw_roi->roi.y + ((int32_t)config.roi_h + config.gap_h) * row;
        af_fw_roi_dim->w = config.roi_w - 1;
        af_fw_roi_dim->h = config.roi_h - 1;
        af_fw_roi_dim->region_num = region_num++;
        af_fw_roi_dim->is_valid = TRUE;
        abs_height = af_fw_roi_dim->y + af_fw_roi_dim->h;
        if (abs_height > af_fw_roi_cfg->max_primary_abs_height) {
          af_fw_roi_cfg->max_primary_abs_height = abs_height;
        }
        AF_LOW("ROI[%d][%d, %d]: x=%d, y=%d, dx=%d, dy=%d, max_abs_y=%d",
          af_fw_roi_dim->region_num, row, col, af_fw_roi_dim->x,
          af_fw_roi_dim->y, af_fw_roi_dim->w, af_fw_roi_dim->h,
          af_fw_roi_cfg->max_primary_abs_height);
      }
    }
  }
  return TRUE;
}

/** af_biz_convert_rectangle_roi:
 *
 *  Convert ROI into floating window for rectangle type
 *
 *  @af: internal AF data
 *
 *  @fw_roi: Roi from algo
 *
 *  @af_fw_roi_cfg: floating window config for isp
 **/
static boolean af_biz_convert_rectangle_roi(af_biz_internal *af,
  af_core_config_roi_fw_type *fw_roi,
  bf_fw_roi_cfg_t *af_fw_roi_cfg)
{
  uint32_t  row = 0, col = 0;
  bf_fw_roi_dim_t  *af_fw_roi_dim = NULL;
  uint32_t          region_num = 0;
  uint32_t          abs_height = 0; /* Absolute height:(y+h), temp placeholder */
  af_biz_fw_int_param_t config;

  /* Get Floating window count and size info. */
  if (!af_biz_convert_fw_roi_calc_config(af,fw_roi, &config)) {
    AF_ERR("Invalid config, falling to default single window config");
    return FALSE;
  }

  /* Gnerate Floating windows. */
  af_fw_roi_cfg->is_valid = TRUE;
  af_fw_roi_cfg->num_bf_fw_roi_dim = 0;
  af_fw_roi_cfg->max_primary_abs_height = 0;
  af_fw_roi_cfg->max_secondary_abs_height = 0;
  for (row = 0; row < config.total_row &&
    af_fw_roi_cfg->num_bf_fw_roi_dim < BF_FW_MAX_ROI_REGIONS; row++) {
    for (col = 0; col < config.total_col &&
      af_fw_roi_cfg->num_bf_fw_roi_dim < BF_FW_MAX_ROI_REGIONS; col++) {
      af_fw_roi_dim = &af_fw_roi_cfg->bf_fw_roi_dim[
        af_fw_roi_cfg->num_bf_fw_roi_dim++];
      af_fw_roi_dim->region = BF_FW_PRIMARY_REGION;
      af_fw_roi_dim->x =
        (uint32_t)fw_roi->roi.x + ((int32_t)config.roi_w + config.gap_w) * col;
      af_fw_roi_dim->y =
        (uint32_t)fw_roi->roi.y + ((int32_t)config.roi_h + config.gap_h) * row;
      af_fw_roi_dim->w = config.roi_w - 1;
      af_fw_roi_dim->h = config.roi_h - 1;
      af_fw_roi_dim->region_num = region_num++;
      af_fw_roi_dim->is_valid = TRUE;
      abs_height = af_fw_roi_dim->y + af_fw_roi_dim->h;
      if (abs_height > af_fw_roi_cfg->max_primary_abs_height) {
        af_fw_roi_cfg->max_primary_abs_height = abs_height;
      }
      AF_LOW("ROI[%d][%d, %d]: x=%d, y=%d, dx=%d, dy=%d, max_abs_y:%d",
        af_fw_roi_dim->region_num, row, col, af_fw_roi_dim->x,
        af_fw_roi_dim->y, af_fw_roi_dim->w, af_fw_roi_dim->h,
        af_fw_roi_cfg->max_primary_abs_height);
    }
  }
  return TRUE;
}


/** af_biz_convert_fw_roi:
*
*    convert roi into floating windows
*
*    @af: internal AF data
*
*    @core_stats_info:  algo config
*
*    @bf_fw_roi_cfg:  fw roi config
*
**/
void af_biz_convert_fw_roi(af_biz_internal *af,
  af_core_config_stats_engine_bayer_type* core_stats_info,
  bf_fw_roi_cfg_t *bf_fw_roi_cfg) {

  /* Convert bounding box into floating windows using
        selected pattern and chromatix data.  */
  boolean is_success = FALSE;
  float grid_gap_h = core_stats_info->roi_info.u.fw_info.grid_gap_h;
  float grid_gap_v = core_stats_info->roi_info.u.fw_info.grid_gap_v;
  float grid_size_h = core_stats_info->roi_info.u.fw_info.grid_size_h;
  float grid_size_v = core_stats_info->roi_info.u.fw_info.grid_size_v;
  AF_LOW(" pattern=%d, size_h=%f, size_v=%f, gap_h=%f, gap_v=%f",
    core_stats_info->roi_info.u.fw_info.pattern,
    grid_size_h, grid_size_v, grid_gap_h, grid_gap_v);

  /* Input validation for grid size, fallback to single window if fail. */
  if (BAF_ROI_PATTERN_CUSTOM != core_stats_info->roi_info.u.fw_info.pattern &&
    (grid_size_v > 1.0 || grid_size_v <= 0.0 || grid_size_h > 1.0 ||
    grid_size_h <= 0.0 || grid_gap_h >= 1.0 || grid_gap_h <= -1.0 ||
    grid_gap_v >= 1.0 || grid_gap_v <= -1.0)) {
    AF_LOW(" Grid Size Input validation failed, fallback to Single ROI");
    core_stats_info->roi_info.u.fw_info.pattern = BAF_ROI_PATTERN_MAX;
  }
  switch (core_stats_info->roi_info.u.fw_info.pattern) {
    case BAF_ROI_PATTERN_RECT:
      is_success = af_biz_convert_rectangle_roi(af,
        &core_stats_info->roi_info.u.fw_info, bf_fw_roi_cfg);
      break;
    case BAF_ROI_PATTERN_CROSS:
      is_success = af_biz_convert_cross_roi(af,
        &core_stats_info->roi_info.u.fw_info, bf_fw_roi_cfg);
      break;
    case BAF_ROI_PATTERN_DIAMOND:
      is_success = af_biz_convert_diamond_roi(af,
        &core_stats_info->roi_info.u.fw_info, bf_fw_roi_cfg);
      break;
    case BAF_ROI_PATTERN_CUSTOM:
    default:
      break;
  }

  if (!is_success) {
    AF_LOW("Default Single window config.");
    /* Configure single window */
    bf_fw_roi_cfg->num_bf_fw_roi_dim = 1;
    bf_fw_roi_cfg->bf_fw_roi_dim[0].x =
      (uint32_t)core_stats_info->roi_info.u.fw_info.roi.x;
    bf_fw_roi_cfg->bf_fw_roi_dim[0].y =
      (uint32_t)core_stats_info->roi_info.u.fw_info.roi.y;
    bf_fw_roi_cfg->bf_fw_roi_dim[0].w =
      (uint32_t)core_stats_info->roi_info.u.fw_info.roi.width;
    bf_fw_roi_cfg->bf_fw_roi_dim[0].h =
      (uint32_t)core_stats_info->roi_info.u.fw_info.roi.height;

    bf_fw_roi_cfg->bf_fw_roi_dim[0].region = BF_FW_PRIMARY_REGION;
    bf_fw_roi_cfg->bf_fw_roi_dim[0].is_valid = TRUE;
    bf_fw_roi_cfg->bf_fw_roi_dim[0].region_num = 0;
    bf_fw_roi_cfg->max_primary_abs_height =
      bf_fw_roi_cfg->bf_fw_roi_dim[0].y + bf_fw_roi_cfg->bf_fw_roi_dim[0].h;
  }

}

/** af_biz_handle_stats_config_vfe:
*
*    Pack output of AF process
*
*    @af: internal AF data
*
*    @af_out: output AF data
*
**/
void af_biz_handle_stats_config_vfe_fw(af_biz_internal *af,
  af_core_config_stats_engine_bayer_type *core_stats_info,
  af_config_t *stats_config) {
  uint32 i = 0, j = 0;

  bf_fw_config_t *fw_cfg = &stats_config->bf_fw;
  stats_config->config_id = af->sof_id;
  stats_config->mask = MCT_EVENT_STATS_BF_FW;

  /* input cfg */
  fw_cfg->bf_input_cfg.is_valid = core_stats_info->bf_input_cfg.is_valid;
  fw_cfg->bf_input_cfg.bf_input_g_sel =
    (bf_input_g_sel_t)core_stats_info->bf_input_cfg.bf_g_sel;
  fw_cfg->bf_input_cfg.bf_channel_sel =
    (bf_channel_sel_t)core_stats_info->bf_input_cfg.bf_input_sel;
  for (i = 0; i < MAX_Y_CONFIG; i++) {
    fw_cfg->bf_input_cfg.y_a_cfg[i] = core_stats_info->bf_input_cfg.y_channel_weight[i];
  }

  /*gamma config */
  fw_cfg->bf_gamma_lut_cfg.is_valid = core_stats_info->bf_gamma_lut_cfg.is_valid;
  fw_cfg->bf_gamma_lut_cfg.num_gamm_lut = core_stats_info->bf_gamma_lut_cfg.num_gamm_lut;
  for (i = 0; i < MAX_BF_GAMMA_ENTRIES; i++) {
    fw_cfg->bf_gamma_lut_cfg.gamma_lut[i] = core_stats_info->bf_gamma_lut_cfg.gamma_lut[i];
  }

  /* bf scale cfg */
  fw_cfg->bf_scale_cfg.is_valid = core_stats_info->bf_scale_cfg.is_valid;

  fw_cfg->bf_scale_cfg.bf_scale_en =  core_stats_info->bf_scale_cfg.bf_scale_en;

  fw_cfg->bf_scale_cfg.scale_m = core_stats_info->bf_scale_cfg.scale_m;

  fw_cfg->bf_scale_cfg.scale_n = core_stats_info->bf_scale_cfg.scale_n;


  uint32 no_kernel = core_stats_info->filter_info.num_of_kernel;
  /* bf_filter_cfg */
  for (j = 0; j < no_kernel; j++) {
    /* H/V Filters */
    fw_cfg->bf_filter_cfg[j].is_valid =
      core_stats_info->filter_info.kernel_info[j].is_valid;

    fw_cfg->bf_filter_cfg[j].h_scale_en =
      core_stats_info->filter_info.kernel_info[j].scalar_enabled;

    fw_cfg->bf_filter_cfg[j].bf_fir_filter_cfg.enable =
      core_stats_info->filter_info.kernel_info[j].fir.enabled;

    fw_cfg->bf_filter_cfg[j].bf_fir_filter_cfg.num_a =
      core_stats_info->filter_info.kernel_info[j].fir.coeff_size;

    for (i = 0; i < MAX_BF_FIR_FILTER_SIZE; i++) {
      fw_cfg->bf_filter_cfg[j].bf_fir_filter_cfg.a[i] =
        (int32)core_stats_info->filter_info.kernel_info[j].fir.a[i];
    }
    fw_cfg->bf_filter_cfg[j].bf_iir_filter_cfg.enable =
      core_stats_info->filter_info.kernel_info[j].iir.enabled;

    fw_cfg->bf_filter_cfg[j].bf_iir_filter_cfg.a11 =
      core_stats_info->filter_info.kernel_info[j].iir.a[0];

    fw_cfg->bf_filter_cfg[j].bf_iir_filter_cfg.a12 =
      core_stats_info->filter_info.kernel_info[j].iir.a[1];

    fw_cfg->bf_filter_cfg[j].bf_iir_filter_cfg.a21 =
      core_stats_info->filter_info.kernel_info[j].iir.a[2];
    fw_cfg->bf_filter_cfg[j].bf_iir_filter_cfg.a22 =
      core_stats_info->filter_info.kernel_info[j].iir.a[3];

    fw_cfg->bf_filter_cfg[j].bf_iir_filter_cfg.b10 =
      core_stats_info->filter_info.kernel_info[j].iir.b[0];

    fw_cfg->bf_filter_cfg[j].bf_iir_filter_cfg.b11 =
      core_stats_info->filter_info.kernel_info[j].iir.b[1];

    fw_cfg->bf_filter_cfg[j].bf_iir_filter_cfg.b12 =
      core_stats_info->filter_info.kernel_info[j].iir.b[2];

    fw_cfg->bf_filter_cfg[j].bf_iir_filter_cfg.b20 =
      core_stats_info->filter_info.kernel_info[j].iir.b[3];

    fw_cfg->bf_filter_cfg[j].bf_iir_filter_cfg.b21 =
      core_stats_info->filter_info.kernel_info[j].iir.b[4];

    fw_cfg->bf_filter_cfg[j].bf_iir_filter_cfg.b22 =
      core_stats_info->filter_info.kernel_info[j].iir.b[5];

    fw_cfg->bf_filter_cfg[j].shift_bits =
      core_stats_info->filter_info.kernel_info[j].shift_bits;

    fw_cfg->bf_filter_cfg[j].bf_filter_coring_cfg.threshold =
      core_stats_info->filter_info.kernel_info[j].coring_info.bayer_coring_info.u.fw_coring_cfg.threshold;

    fw_cfg->bf_filter_cfg[j].bf_filter_coring_cfg.gain =
      core_stats_info->filter_info.kernel_info[j].coring_info.bayer_coring_info.u.fw_coring_cfg.gain;

    for (i = 0; i < MAX_BF_FILTER_CORING_INDEX; i++) {
      fw_cfg->bf_filter_cfg[j].bf_filter_coring_cfg.ind[i] =
        core_stats_info->filter_info.kernel_info[j].coring_info.bayer_coring_info.u.fw_coring_cfg.index[i];
    }
  }

  /* ROI config section */
  fw_cfg->bf_fw_roi_cfg.is_valid = TRUE;
  af_biz_roi_boundary_check(core_stats_info,&af->af_input.hw_cap_info.roi_info.u.fw_info);
  af_biz_convert_fw_roi(af, core_stats_info, &fw_cfg->bf_fw_roi_cfg);

}

/** af_biz_handle_stats_config_vfe_grid:
*
*    Pack output of AF process
*
*    @af: internal AF data
*
*    @af_out: output AF data
*
**/
void af_biz_handle_stats_config_vfe_grid(af_biz_internal *af,
  af_core_config_stats_engine_bayer_type *core_stats_info,
  af_config_t *stats_config) {
  uint32 i;
  // config_id is not filled in algo and same for roi_type

  // stats_config should have eperate h/v_num for both kernel
  stats_config->config_id = af->sof_id;
  stats_config->common.grid_info.h_num =
    core_stats_info->roi_info.u.grid_info.h_num;

  stats_config->common.grid_info.v_num =
    core_stats_info->roi_info.u.grid_info.v_num;

  stats_config->mask = MCT_EVENT_STATS_BF_SINGLE;
  if (core_stats_info->filter_info.num_of_kernel > 1) {
    stats_config->mask |= MCT_EVENT_STATS_BF_SCALE;
  }
  // primary kernel config
  stats_config->bf.roi.left =
    (uint16_t)core_stats_info->roi_info.u.grid_info.roi.x;
  stats_config->bf.roi.top =
    (uint16_t)core_stats_info->roi_info.u.grid_info.roi.y;
  stats_config->bf.roi.width =
    (uint16_t)core_stats_info->roi_info.u.grid_info.roi.width;
  stats_config->bf.roi.height =
    (uint16_t)core_stats_info->roi_info.u.grid_info.roi.height;
  // check type for fir and iir later
  uint32_t size = core_stats_info->filter_info.kernel_info[0].fir.coeff_size;
  for (i = 0; i < size; i++) {
    stats_config->bf.hpf[i] =
      core_stats_info->filter_info.kernel_info[0].fir.a[i];
  }

  // scale config
  stats_config->bf_scale.roi.left =
    (uint16_t)core_stats_info->roi_info.u.grid_info.roi.x;
  stats_config->bf_scale.roi.top =
    (uint16_t)core_stats_info->roi_info.u.grid_info.roi.y;
  stats_config->bf_scale.roi.width =
    (uint16_t)core_stats_info->roi_info.u.grid_info.roi.width;
  stats_config->bf_scale.roi.height =
    (uint16_t)core_stats_info->roi_info.u.grid_info.roi.height;
  if (stats_config->mask & AF_CONFIG_KERNEL_SCALE) {
    size = core_stats_info->filter_info.kernel_info[1].fir.coeff_size;
    for (i = 0; i < size; i++) {
      stats_config->bf_scale.hpf[i] =
        core_stats_info->filter_info.kernel_info[1].fir.a[i];
    }
  }

  // param filled from tuning
  stats_config->bf.r_min =
    af->af_input.tuning_info.af_vfe[Q3A_AF_KERNEL_PRIMARY].config.fv_min;
  stats_config->bf.b_min =
    af->af_input.tuning_info.af_vfe[Q3A_AF_KERNEL_PRIMARY].config.fv_min;
  stats_config->bf.gr_min =
    af->af_input.tuning_info.af_vfe[Q3A_AF_KERNEL_PRIMARY].config.fv_min;
  stats_config->bf.gb_min =
    af->af_input.tuning_info.af_vfe[Q3A_AF_KERNEL_PRIMARY].config.fv_min;
  stats_config->bf_scale.r_min =
    af->af_input.tuning_info.af_vfe[Q3A_AF_KERNEL_SCALE].config.fv_min;
  stats_config->bf_scale.b_min =
    af->af_input.tuning_info.af_vfe[Q3A_AF_KERNEL_SCALE].config.fv_min;
  stats_config->bf_scale.gr_min =
    af->af_input.tuning_info.af_vfe[Q3A_AF_KERNEL_SCALE].config.fv_min;
  stats_config->bf_scale.gb_min =
    af->af_input.tuning_info.af_vfe[Q3A_AF_KERNEL_SCALE].config.fv_min;
  stats_config->bf_scale.scale =
    af->af_input.tuning_info.af_vfe[Q3A_AF_KERNEL_SCALE].config.bf_scale_factor;

}

/** af_biz_handle_stats_config:
*
*    Pack output of AF process
*
*    @af: internal AF data
*
*    @af_out: output AF data
*
**/
void af_biz_handle_stats_config(af_biz_internal *af,
  af_core_config_stats_engine_type *core_stats_info) {
  swaf_config_data_t sw_stats_config;
  uint8_t size, i;
  AF_LOW("mask=%d ", core_stats_info->mask);

  if (core_stats_info->mask & AF_CORE_STATS_MASK_BAYER) {
    // Reset Roi updated flag
    af->af_input.roi_info.roi_updated = FALSE;
    Q3A_MEMSET(&af->stats_config, 0 , sizeof(af_config_t));
    AF_LOW("Grid or fw config %x",core_stats_info->bayer_config.bf_cfg_type);
    if (core_stats_info->bayer_config.bf_cfg_type == AF_CORE_STATS_GRID_CFG) {
      af_biz_handle_stats_config_vfe_grid(af,
        &core_stats_info->bayer_config, &af->stats_config);
    } else {
      af_biz_handle_stats_config_vfe_fw(af,
        &core_stats_info->bayer_config, &af->stats_config);
    }
    af_biz_util_update_roi_output_data(af, core_stats_info);
    af_biz_util_update_output_data(af,
      AF_OUTPUT_STATS_CONFIG, &af->stats_config);
  }
  if (core_stats_info->mask & AF_CORE_STATS_MASK_PREVIEW_ASSISTED) {
    Q3A_MEMSET(&sw_stats_config, 0 , sizeof(swaf_config_data_t));
    af_biz_handle_stats_config_paaf(af,
      &core_stats_info->paaf_config, &sw_stats_config);
    af_biz_util_update_output_data(af,
      AF_OUTPUT_SWAF_CONFIG, &sw_stats_config);
  }
  if (core_stats_info->mask & AF_CORE_STATS_MASK_DEPTH_ASSISTED) {
    if (AF_DAAF_PDAF == core_stats_info->daaf_config.daaf_type) {
      af_biz_util_update_output_data(af,
        AF_OUTPUT_PDAF_CONFIG, &(core_stats_info->daaf_config.u.pdaf_config));
    }
    if (AF_DAAF_DCIAF == core_stats_info->daaf_config.daaf_type) {
      af_biz_util_update_output_data(af,
        AF_OUTPUT_DCIAF_CONFIG, &(core_stats_info->daaf_config.u.dciaf_config));
    }
  }
} /* af_biz_handle_stats_config */


/** af_biz_handle_algo_data:
*
*    Pack output of AF process
*
*    @af: internal AF data
*
*    @af_out: output AF data
*
**/
void af_biz_handle_algo_data(af_biz_internal *af,
  af_core_opaque_data_type *core_algo_info) {

  /* create a request to algo info */
  af_biz_util_update_output_data(af, AF_OUTPUT_UPDATE_EVENT, core_algo_info);
} /* af_biz_handle_algo_data */


/** af_biz_handle_focus_status_update:
*
*    Pack output of AF process
*
*    @af: internal AF data
*
*    @af_out: output AF data
*
**/
void af_biz_handle_focus_status_update(af_biz_internal *af,
  af_core_status_type *core_status_info) {
  af_status_t af_status;
  /* create a request to af status */
  af_status.focus_done = FALSE;

  if (*core_status_info == AF_CORE_STATUS_FOCUSED) {
    af_status.status = AF_STATUS_FOCUSED;
    af_status.focus_done = TRUE;
  } else if (*core_status_info == AF_CORE_STATUS_NOT_FOCUSED) {
    af_status.status = AF_STATUS_UNKNOWN;
    af_status.focus_done = TRUE;
  } else if (*core_status_info == AF_CORE_STATUS_SCANNING) {
    af_status.status = AF_STATUS_FOCUSING;
  /* considered SCENE CHANGE and STATUS INACTIVE */
  } else {
    af_status.status = AF_STATUS_INVALID;
  }
  AF_LOW(" status=%d", af_status.status);
  af->status = af_status.status;
   // af_status.focus_done not used in port
  af_biz_util_update_output_data(af, AF_OUTPUT_STATUS, &af_status);
} /* af_biz_handle_focus_status_update */

/** af_biz_handle_core_output:
*
*    Pack output of AF process
*
*    @af: internal AF data
*
*    @af_out: output AF data
*
**/
void af_biz_handle_core_output(af_biz_internal *af) {
  af_core_output_params_type *core_out = &af->af_core_out;
  af_output_data_t *output = &af->af_out;
  AF_LOW(" type=%d", core_out->mask);
  af->af_out.frame_id = af->frame_id;
  af->af_out.sof_id = af->sof_id;
  if (core_out->mask & AF_CORE_OUTPUT_MOVE_LENS) {
    af_biz_handle_move_lens(af, &core_out->lens_move_info);
  }
  if (core_out->mask & AF_CORE_OUTPUT_CONFIG_STATS) {
    af_biz_handle_stats_config(af, &core_out->config_info);
  }
  if (core_out->mask & AF_CORE_OUTPUT_OPAQUE_DATA) {
    af_biz_handle_algo_data(af, &core_out->data);
  }
  if (core_out->mask & AF_CORE_OUTPUT_EXP_COMPENSATE) {
    af_biz_util_update_output_data(af, AF_OUTPUT_EXP_COMPENSATE, &core_out->af_exp_compensate);
  }
  if (core_out->mask & AF_CORE_OUTPUT_FOCUS_STATUS) {
    af_biz_handle_focus_status_update(af, &core_out->status);
    af->af_out.grav_applied = core_out->gravity_compensation;
    af->af_out.af_depth_based_focus = core_out->af_depth_based_focus;
    af->af_out.af_peer_focus_info = core_out->af_peer_focus_info;
  }
  if (core_out->mask & AF_CORE_OUTPUT_SPOT_LIGHT_DETECTION) {
    af_biz_util_update_output_data(af, AF_OUTPUT_SPOT_LIGHT_DETECTION,
      &core_out->af_spot_light_detected);
  }
  if (core_out->mask & AF_CORE_OUTPUT_FOCUS_VALUE) {
    af_biz_util_update_output_data(af, AF_OUTPUT_FOCUS_VALUE,
      &core_out->focus_value);
  }
  if (core_out->mask & AF_CORE_OUTPUT_EZ_METADATA) {
    af_biz_util_update_output_data(af, AF_OUTPUT_EZ_METADATA,
      &core_out->eztune_output_data);
  }
  if (core_out->mask & AF_CORE_OUTPUT_RESET_AEC) {
    af_biz_util_update_output_data(af, AF_OUTPUT_RESET_AEC, NULL);
  }
  if (core_out->mask & AF_CORE_OUTPUT_DEBUG_DATA) {
    af_biz_util_update_output_data(af, AF_OUTPUT_DEBUG_DATA,
      &core_out->debug_output_data);
  }
  Q3A_MEMSET(&af->af_core_out, 0, sizeof(af_core_output_params_type));
} /* af_biz_handle_core_output */

/**
 * aec_biz_clear_iface_ops
 *
 * @af_ops: structure with function pointers to be assign
 *
 * Clear interface by setting all pointers to NULL
 *
 * Return: void
 **/
void af_biz_clear_iface_ops(af_ops_t *af_ops)
{
  af_ops->set_parameters = NULL;
  af_ops->get_parameters = NULL;
  af_ops->process = NULL;
  af_ops->init = NULL;
  af_ops->deinit = NULL;
  return;
}

/**
 * aec_biz_clear_algo_ops
 *
 * @af_ops: structure with function pointers to be assign
 *
 * Clear interface by setting all pointers to NULL
 *
 * Return: void
 **/
void af_biz_clear_algo_ops(af_biz_algo_ops_t *af_algo_ops)
{
  af_algo_ops->set_parameters = NULL;
  af_algo_ops->get_parameters = NULL;
  af_algo_ops->process = NULL;
  af_algo_ops->init = NULL;
  af_algo_ops->deinit = NULL;
  af_algo_ops->set_log = NULL;
  return;
}


/** af_biz_dlsym
 *
 *    @lib_handler: Handler to library
 *    @fn_ptr: Function to initialize
 *    @fn_name: Name of symbol to find in library
 *
 * Return: TRUE on success
 **/
static boolean af_biz_dlsym(void *lib_handler, void *fn_ptr,
  const char *fn_name)
{
  char *error = NULL;

  if (NULL == lib_handler || NULL == fn_ptr) {
    AF_ERR("Error Loading %s", fn_name);
    return FALSE;
  }

  *(void **)(fn_ptr) = dlsym(lib_handler, fn_name);
  if (!fn_ptr) {
    error = (char *)dlerror();
    AF_ERR("Error: %s", error);
    return FALSE;
  }

  AF_LOW("Loaded %s %p", fn_name, fn_ptr);
  return TRUE;
}

/**
 *
 **/
void *af_biz_init(void *libptr)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_FAILED;
  af_biz_internal *af;

  AF_HIGH(" Init");
  do {
    af = (void *)calloc(1, sizeof(af_biz_internal));
    if (NULL == af) {
      AF_ERR("malloc failed");
      break;
    }
    af->af_out.debug_info = (void*)calloc(1, sizeof(af_debug_info_t));
    if (NULL == af->af_out.debug_info) {
      AF_ERR("malloc failed for debug info");
      break;
    }
    AF_HIGH("af_biz: %p Init", af);

    /* Init biz internal member */
    af_biz_initialize_internal_data(af);

    dlerror(); /* Clear previous errors */
    if (!af_biz_dlsym(libptr, &af->af_algo_ops.set_parameters,
      "af_core_set_param")) {
      break;
    }
    if (!af_biz_dlsym(libptr, &af->af_algo_ops.get_parameters,
      "af_core_get_param")) {
      break;
    }
    if (!af_biz_dlsym(libptr, &af->af_algo_ops.process,
      "af_core_process")) {
      break;
    }
    if (!af_biz_dlsym(libptr, &af->af_algo_ops.init,
      "af_core_init")) {
      break;
    }
    if (!af_biz_dlsym(libptr, &af->af_algo_ops.deinit,
      "af_core_deinit")) {
      break;
    }
    if (!af_biz_dlsym(libptr, &af->af_algo_ops.set_log,
      "q3a_core_set_log_level")) {
      break;
    }
    /* set logging for core */
    af->af_algo_ops.set_log();

    rc = af->af_algo_ops.init(&af->af_core);
    if (Q3A_CORE_RESULT_SUCCESS != rc) {
      AF_ERR("Error Initializing AF Core!");
      break;
    }

  /* Set HAF custom library factory info in core */
  af_core_set_param_info_type set_param;
  set_param.param_type = AF_CORE_SET_PARAM_HAF_CUSTOM_FACTORY_PTR;
  set_param.u.haf_custom_factory_create = af_factory_create_custom;
  af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);

  /*Enable tuning debug if set*/
  STATS_ENABLE_TUNING_DEBUG(af->enableTuningDebug);

  } while (0);

    if (Q3A_CORE_RESULT_SUCCESS != rc) {
      /* Handling error */
      if (af) {
        af_biz_clear_algo_ops(&af->af_algo_ops);
        if (af->af_out.debug_info) {
          free(af->af_out.debug_info);
        }
        free(af);
        af = NULL;
      }
  }
  return af;
}

/** af_biz_get_search_limit:
 *
 *  Read reserved data from chromatix header and update
 *  search ranges. Not required after new chromatix
 *  revision has valid entries.
 *
 *  @af: internal AF data structure
 *
 * Return: None
 **/
static void af_biz_get_search_limit(af_biz_internal *af)
{
  af_tuning_algo_adapter_t *tuning_ptr = &af->af_input.tuning_info.af_algo;
  int i = 0;

  for (i = 0; i < AF_FOCUS_MODE_MAX; i++) {
    af->search_limit[i].near_end =
      tuning_ptr->algo_extended.search_limit[i].near_end;
    af->search_limit[i].far_end =
      tuning_ptr->algo_extended.search_limit[i].far_end;
    af->search_limit[i].default_pos =
      tuning_ptr->algo_extended.search_limit[i].default_pos;

    AF_LOW("index: %d near_end: %u far_end: %u default: %u",
      i,
      af->search_limit[i].near_end,
      af->search_limit[i].far_end,
      af->search_limit[i].default_pos);
  }
}

/** af_biz_set_parameters: Set AF parameters based on parameter
 *  type.
 *
 *  @param: information about parameter to be set
 *
 *  @af: internal AF data structure
 *
 *
 * Return: Success- TRUE  Failure- FALSE
 **/
boolean af_biz_set_parameters(af_set_parameter_t *param,
  af_output_data_t *output, uint8_t num_of_outputs, void *af_internal) {
  boolean rc            = TRUE;
  q3a_core_result_type result;
  boolean need_callback = FALSE;
  af_biz_internal *af = (af_biz_internal *)af_internal;
  int i = 0;
  af_core_set_param_info_type set_param;
  af_core_set_param_info_type set_param_1;

  if (!param || !af || num_of_outputs > MAX_AF_ALGO_OUTPUTS) {
    AF_ERR(" Invalid parameters! num_of_ouputs: %u", num_of_outputs);
    rc = FALSE;
    goto done;
  }

  AF_LOW("af_biz:%p, camera_id:%u, param=%p, type =%d",
    af, param->camera_id, param, param->type);
  /* reset all output data  and reserve debug
  data pointer to avoid reset of debug data address.*/
  af_debug_info_t *debug_info = af->af_out.debug_info;
  Q3A_MEMSET(&af->af_out, 0, sizeof(af_output_data_t));
  Q3A_MEMSET(output, 0, sizeof(af_output_data_t));
  af->af_out.debug_info = debug_info;

  if (NULL == af->af_algo_ops.set_parameters) {
    AF_ERR("error: set_parameter() not set");
    rc = FALSE;
    goto done;
  }

  switch (param->type) {
  case AF_SET_PARAM_INIT_CHROMATIX_PTR: {
    chromatix_3a_parms_wrapper *chromatix = (chromatix_3a_parms_wrapper *)param->u.af_init_param.tuning_info;
    pthread_rwlock_rdlock(&chromatix->lock);
    Q3A_MEMCPY(&af->af_input.tuning_info, &chromatix->AF, sizeof(af->af_input.tuning_info));
    pthread_rwlock_unlock(&chromatix->lock);
    af->preview_size = param->u.af_init_param.preview_size;
    Q3A_MEMCPY(&af->af_input.extended_tuning_data,
      &param->u.af_init_param.core_tuning_params,
      sizeof(af_core_tuning_params_t));

    if(af->enableTuningDebug) {
      af_biz_debug_print_af_tuning_parms(af);
    }

    /*Logic to read reserved data from chromatix pointer and update
      local search range.
      Note: Not required after next chromatix update */
    af_biz_get_search_limit(af);

    /* Update with default search range */
    af->af_mode.near_end =
      af->search_limit[AF_FOCUS_MODE_DEFAULT].near_end;
    af->af_mode.far_end =
      af->search_limit[AF_FOCUS_MODE_DEFAULT].far_end;
    af->af_mode.default_pos =
      af->search_limit[AF_FOCUS_MODE_DEFAULT].default_pos;
    af->af_mode.hyp_pos =
      af->af_input.tuning_info.af_algo.position_normal_hyperfocal;

    // Send tuning info to core
    set_param.param_type = AF_CORE_SET_PARAM_TUNING_INFO;
    set_param.u.tuning_load.algo_tuning_info = &af->af_input.tuning_info;
    set_param.u.tuning_load.extended_tuning_info.af_caf_trigger_after_taf
      = af->af_input.extended_tuning_data.af_caf_trigger_after_taf;
    Q3A_MEMCPY(&set_param.u.tuning_load.extended_tuning_info,
      &param->u.af_init_param.core_tuning_params,
      sizeof(af_core_tuning_params_t));
    af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);
    need_callback = TRUE;
  }
    break;
  case AF_SET_PARAM_RUN_MODE:
    af->af_input.stream_type = param->u.af_run_mode;
    AF_LOW("Stream type=%d", af->af_input.stream_type);
    break;
  case AF_SET_PARAM_INIT:
    AF_LOW("AF_SET_PARAM_INIT! Reset the lens!");
    if(!af->lens_reset_on_init) {
      af_biz_reset_lens(af, TRUE, 0);
      af->lens_reset_on_init = TRUE;
      need_callback =  TRUE;
    }
    af->af_inited = TRUE;
    break;
  case AF_SET_PARAM_RESET_LENS:
    /* Request to reset the lens from outside - we'll always reset
       to default position */
    if (af_biz_reset_lens(af, TRUE, 0) < 0) rc = FALSE;
    need_callback = TRUE;
    break;
  case AF_SET_PARAM_METERING_MODE:
    af->metering_mode = param->u.af_metering_mode;
    break;
  case AF_SET_PARAM_START:
    /* Proceed further only if it's not AF call during Infy mode. */
    if (af_biz_is_af_call_during_infy_mode(af) == FALSE) {
      if (af_biz_start_focus(af) < 0) {
        rc = FALSE;
      }
    }
    need_callback = TRUE;
    break;
  case AF_SET_PARAM_WAIT_FOR_AEC_EST:
    af->wait_for_aec_est = param->u.af_wait_for_aec_est;
    AF_LOW("wait_for_aec=%d", af->wait_for_aec_est);
    break;
  case AF_SET_PARAM_MOVE_LENS: // here param is pos to move
    if (af_biz_move_lens_to(af, param->u.af_steps_to_move) < 0) rc = FALSE;
    need_callback = TRUE;
    break;
  case AF_SET_PARAM_FOCUS_MODE:
    if (af_biz_focus_mode(af, param->u.af_mode) < 0) rc = FALSE;
    need_callback = TRUE;
    break;
  case AF_SET_PARAM_CANCEL_FOCUS:
    af->wait_for_aec_est = FALSE;
    if (af_biz_cancel_focus(af) < 0) rc = FALSE;
    need_callback = TRUE;
    break;
  case AF_SET_PARAM_STOP_FOCUS:
    af->wait_for_aec_est = FALSE;
    if (af_biz_stop_focus(af) < 0) rc = FALSE;
    need_callback = TRUE;
    break;
  case AF_SET_PARAM_BESTSHOT:
    if (af_biz_bestshot_mode(af, param->u.af_bestshot_mode) < 0) rc = FALSE;
    need_callback = TRUE;
    break;
  case AF_SET_PARAM_ROI:
      af_biz_map_roi(af, &param->u.af_roi_info);
      need_callback = TRUE;
    break;
  case AF_SET_PARAM_LOCK_CAF:
    af->af_input.enable_caf_lock = param->u.af_lock_caf;
    break;
  case AF_SET_PARAM_UPDATE_AEC_INFO:
    af_biz_map_aec_info(af, &param->u.aec_info);
    break;
  case AF_SET_PARAM_UPDATE_SENSOR_INFO:
    af_biz_map_sensor_info(af, &param->u.sensor_info);
    break;
  case AF_SET_PARAM_UPDATE_GYRO_INFO:
    af_biz_map_gyro_info(af, &param->u.gyro_info);
    break;

  case AF_SET_PARAM_UPDATE_FACE_INFO:
    af->af_input.face_detected = param->u.face_detected;
    break;

  case AF_SET_PARAM_UPDATE_ISP_INFO:
    // not used
    break;
  case AF_SET_PARAM_EZ_ENABLE:
    af->af_input.enable_eztune = param->u.af_ez_enable;
    break;
  case AF_SET_PARAM_STREAM_CROP_INFO:
    if (af_biz_map_stream_crop_info(af, &param->u.stream_crop) < 0) rc = FALSE;
    break;
  case AF_SET_PARAM_MOVE_LENS_CB:
    // not used anymore
    break;
  case AF_SET_PARAM_SOF:
    if (!af->move_lens_to_def_pos) {
      AF_HIGH("First SOF evt! Move lens to def pos!");
      //af_biz_move_lens_to(af, af->af_mode.far_end);
      af_biz_move_lens_to(af, af->af_mode.default_pos);
      need_callback =  TRUE;
      af->move_lens_to_def_pos = TRUE;
    }
    af->sof_id = param->u.af_set_sof_id;
    break;
  case AF_SET_PARAM_META_MODE:
    af->af_ctrl_mode = param->u.af_set_meta_mode;
    AF_LOW(" AF Meta mode: %d", af->af_ctrl_mode);
    break;
  case AF_SET_PARAM_CROP_REGION:
    // not used anymore
    break;
  case AF_SET_PARAM_PAAF:
    if (param->u.paaf_mode) {
      af->af_input.force_paaf_enable = TRUE;
    } else {
      af->af_input.force_paaf_enable = FALSE;
    }
    break;
  case AF_SET_PARAM_EZ_TUNE_RUNNING:
    af->af_input.eztune_running = param->u.ez_running;
    break;
  case AF_SET_PARAM_CONFIG_AE_SCAN_TEST:
    set_param.param_type = AF_CORE_SET_PARAM_CONFIG_AE_SCAN_TEST;
    set_param.u.ae_scan_test_config = param->u.ae_scan_test_config;
    af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);
    break;
  case AF_SET_PARAM_RESET_CAF:
    /* When we reset the CAF, that means we want full sweep. So set the
     * prev_runtype to AF_RUN_MODE_CAMERA to avoid entering directly in
     * monitor mode. */
    AF_LOW(" AF_SET_PARAM_RESET_CAF");
    set_param.param_type = AF_CORE_SET_PARAM_RESET_CAF;
    af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);
    need_callback = TRUE;
    break;
  case AF_SET_PARAM_IMGLIB_OUTPUT:
    AF_LOW(" SW stats received! FrameID:0x%x FV:%f",
      param->u.sw_stats.frame_id, param->u.sw_stats.fV);
    set_param.u.preview_stats.frame_id = param->u.sw_stats.frame_id;
    set_param.u.preview_stats.fv = (uint32_t)param->u.sw_stats.fV;
    set_param.u.preview_stats.grid_count = 1; // fix as of now

    set_param.param_type = AF_CORE_SET_PARAM_PREVIEW_STATS;
    af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);
    need_callback = TRUE;
    break;
  case AF_SET_PARAM_HFR_MODE: {
    af->hfr_mode = param->u.hfr_mode;
    AF_LOW("HFR mode=%d", af->hfr_mode);
  }
    break;
  case AF_SET_PARAM_SW_STATS_POINTER:
//    AF_LOW("set pointer backup");
//    af->sw_stats_bak = param->u.p_sw_stats;
// Dont need it any more.
    break;

  case AF_SET_PARAM_UPDATE_STATS_CAPS: {
    af_biz_map_isp_capability(af, &param->u.af_stats_caps_info);
  }
    break;

  case AF_SET_PARAM_UPDATE_GRAVITY_VECTOR:
    af_biz_map_gravity_vector_info(af, &param->u.gravity_info);
    break;

  case AF_SET_PARAM_LENS_POS_FOCUS: {
    AF_LOW("cur_pos_comp =%d", param->u.cur_pos_comp);
    af->af_input.cur_lens_pos = param->u.cur_pos_comp;
  }
    break;

  case AF_SET_PARAM_RECONFIG_ISP: {
    AF_LOW(" Need to reconfigure the ROI and HPF");
    af->af_input.roi_info.roi_updated = TRUE;
    af->isp_reconfig_needed = TRUE;
  }
    break;
  case AF_SET_PARAM_DEPTH_SERVICE: {
    if(param->u.depth_service.input.info.type == DEPTH_SERVICE_PDAF)
      af_biz_map_pdaf_info(af, &param->u.depth_service);
    else if (param->u.depth_service.input.info.type == DEPTH_SERVICE_DUAL_CAM_AF)
      af_biz_map_dciaf_info(af, &param->u.depth_service);
    else if (param->u.depth_service.input.info.type == DEPTH_SERVICE_TOF)
      af_biz_map_tof_info(af, &param->u.depth_service);
  }
  need_callback = TRUE;
    break;
  case AF_SET_PARAM_HAF_ENABLE: {
    set_param.param_type = AF_CORE_SET_PARAM_HAF_ENABLE;
    set_param.u.haf = param->u.haf;
    af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);
  }
  need_callback = TRUE;
    break;
  case AF_SET_PARAM_SET_PEER_FOCUS_INFO: {
      set_param.param_type = AF_CORE_SET_PEER_FOCUS_INFO;
      set_param.u.af_peer_focus_info = param->u.af_peer_focus_info;
      af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);
      need_callback = TRUE;
    }
    break;
  case AF_SET_PARAM_SET_ROLE_SWITCH: {
      set_param.param_type = AF_CORE_SET_ROLE_SWITCH;
      set_param.u.role_switch = param->u.role_switch;
      af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);
      need_callback = TRUE;
    }
    break;
  case AF_SET_PARAM_SET_STEREO_CAL_DATA: {
    set_param.param_type = AF_CORE_SET_STEREO_CAL_DATA;
    set_param.u.p_af_syscalib_data = param->u.p_af_syscalib_data;
    af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);
    need_callback = FALSE;
  }
    break;
  case AF_SET_PARAM_PDAF_DATA: {
    af_pdaf_data_t *pdaf = &param->u.pdaf_data;
    if (pdaf->pdaf_proc_cb) {
      boolean rc = pdaf->pdaf_proc_cb((void*)pdaf, (void*)&af->depth_output);
      if (rc == TRUE) {
        af_biz_map_pdaf_info(af, &af->depth_output);
      }
    }
  }
    break;
  case AF_SET_PARAM_HAF_CUSTOM_PARAM: {
    set_param.param_type = AF_CORE_SET_PARAM_HAF_CUSTOM_PARAM;
    /*Cutomer Can pass in their Custom Param here*/
    set_param.u.customData = NULL;
    af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);
  }
    break;
  case AF_SET_PARAM_FOV_COMP_ENABLE: {
    AF_LOW("af biz AF_SET_PARAM_FOV_COMP_ENABLE");
    set_param.param_type = AF_CORE_SET_PARAM_FOV_COMP_ENABLE;
    set_param.u.fov_comp_enable = param->u.fov_comp_enable;
    af->af_algo_ops.set_parameters(af->af_core, &set_param, &af->af_core_out);
  }
    break;
  default:
    break;
  }

done:
  if(!af) return FALSE;

  if (TRUE == need_callback) {
    af_biz_handle_core_output(af);
    af_biz_util_pack_output(af, output);
  }
  return TRUE;
} /* af_biz_set_parameters */



/** af_biz_get_parameters:
 *  Interface for outside components to request access to AF
 *  parameters.
 *
 *  @param: information of speicific parameter to access
 *
 *  @af_internal: internal af data structure
 *
 *  Return: TRUE - success  FALSE - failure
 **/
boolean af_biz_get_parameters(af_get_parameter_t *param, void *af_internal) {
  boolean rc = TRUE;
  af_biz_internal *af = (af_biz_internal *)af_internal;

  if (!param || !af_internal) {
    rc = FALSE;
    goto done;
  }

  /* If AF tuning header has not been initialized yet or sensor told us
    AF is not supported, we return */
  if (af->sensor_input.af_not_supported) {
    AF_LOW(" AF: %p, is not supported. Returning!", af);
    rc = FALSE;
    goto done;
  }

  AF_LOW("af_biz:%p, camera_id: %u, param type: %d",
    af, param->camera_id, param->type);
  switch (param->type) {

  case AF_GET_PARAM_FOCUS_DISTANCES:
    if (af_biz_util_get_focus_distance(af, &(param->u.af_focus_distance)) < 0) {
      rc = FALSE;
    }
    break;

  case AF_GET_PARAM_CUR_LENS_POSITION:
    param->u.af_cur_lens_pos = af->af_input.cur_lens_pos;
    AF_LOW(" Get current lens position: %d",
      param->u.af_cur_lens_pos);
    break;

  case AF_GET_PARAM_DEFAULT_LENS_POSITION:
    param->u.af_def_lens_pos = af->af_mode.default_pos;
    AF_LOW(" Get default lens position: %d",
      param->u.af_def_lens_pos);
    break;

  case AF_GET_PARAM_STATUS:
    param->u.af_status = af->status;
    AF_LOW(" Get AF status: %d",
      param->u.af_status);
    break;

  case AF_GET_PARAM_STATS_CONFIG_INFO:
    Q3A_MEMCPY(&param->u.af_stats_config, &af->stats_config,
      sizeof(af_config_t));
    break;

  case AF_GET_PARAM_FOCUS_MODE:
    param->u.af_mode = af->af_mode.mode;
    AF_LOW(" Get AF mode: %d",
      param->u.af_mode);
    break;

  case AF_GET_PARAM_MOBICAT_INFO:
    break;
  case AF_GET_PARAM_SW_STATS_FILTER_TYPE:
    AF_LOW(" Get filter type!");
    param->u.af_sw_stats_filter_type = af->sw_filter_type;
    break;
  default:
    rc = FALSE;
    break;
  }

done:
  return rc;
}

/** af_biz_process: Main entry point to process the AF stats.
 *
 *  @stats: AF stats
 *
 *  @output: Data exposed externally and updated once AF state
 *         is processed.
 *
 *  @af_obj: pointer to internal AF data.
 **/
 void af_biz_process(stats_af_t *stats, af_output_data_t *output,
  uint8_t num_of_outputs, void *af_obj)
{
  af_debug_data_level_type debug_level = AF_DEBUG_DATA_LEVEL_VERBOSE;
  af_biz_internal *af = (af_biz_internal *)af_obj;
  q3a_core_result_type result;
  AF_LOW("af_biz: %p, camera_id: %u, Process AF stats!", af, stats->camera_id);

  if (num_of_outputs != MAX_AF_ALGO_OUTPUTS) {
    AF_ERR("Does not support %d outputs", num_of_outputs);
    return;
  }

  af->af_input.full_sweep_mode = AF_FULLSWEEP_SRCH_MODE_OFF;
  af->af_input.enable_mobicat = FALSE;
#ifdef _ANDROID_
  int full_sweep_flag = 0;
  char value[92] = { 0 };
  property_get("persist.vendor.camera.mobicat", value, "0");
  af->af_input.enable_mobicat = atoi(value) > 0 ? TRUE : FALSE;


  // Get Setproc for fullsweep algo.
  /* Enable full-sweep property:
     * 0 - disable
     * 1 - far-to-near
     * 2 - reverse search (near-to-far)
     * 3 - both (far->near->far)*/
  property_get("vendor.debug.camera.af_fullsweep", value, "0");
  full_sweep_flag = atoi(value);
  AF_LOW(" Full-Sweep mode: %d", full_sweep_flag);
  if (full_sweep_flag == 1) {
    af->af_input.full_sweep_mode = AF_FULLSWEEP_SRCH_MODE_INFY_TO_NEAR;
  } else if (full_sweep_flag == 2) {
    af->af_input.full_sweep_mode = AF_FULLSWEEP_SRCH_MODE_NEAR_TO_INFY;
  } else if (full_sweep_flag == 3) {
    af->af_input.full_sweep_mode = AF_FULLSWEEP_SRCH_MODE_BOTH;
  } else {
    af->af_input.full_sweep_mode = AF_FULLSWEEP_SRCH_MODE_OFF;
  }
#endif

  debug_level =
    ((stats_debug_data_log_level & Q3A_DEBUG_DATA_LEVEL_CONCISE) > 0) ?
      AF_DEBUG_DATA_LEVEL_CONCISE : AF_DEBUG_DATA_LEVEL_VERBOSE;
  af->af_input.enable_exif =
    stats_exif_debug_mask & EXIF_DEBUG_MASK_AF ? TRUE : FALSE;
  af->af_input.exif_dbg_level = (FALSE == af->af_input.enable_exif) ?
    (AF_DEBUG_DATA_LEVEL_NONE) : debug_level;

  /* reset all output data  and reserve debug
  data pointer to avoid reset of debug data address.*/
  af_debug_info_t *debug_info = af->af_out.debug_info;
  Q3A_MEMSET(&af->af_out, 0, sizeof(af_output_data_t));
  Q3A_MEMSET(output, 0, sizeof(af_output_data_t));
  af->af_out.debug_info = debug_info;

  Q3A_MEMCPY(&af->af_out.focus_mode_info,
      &af->af_mode, sizeof(af_mode_info_t));

  if (af->af_inited ==  FALSE) {
    AF_LOW(" AF is not Initialized. Returning!");
    return;
  }

  /* If AF tuning header has not been initialized yet or sensor told us
    AF is not supported, we return */
  if (af->sensor_input.af_not_supported) {
    AF_LOW(" AF is not supported. Returning!");
    return;
  }

  /* If we are waiting for AEC to finish estimation with LED ON */
  /* This should be before checking the CAF trigger condition in order
   * to handle the LED assisted AF in CAF mode properly. */
  if (af->wait_for_aec_est == TRUE) {
    AF_LOW(" Waiting for AEC to finish estimation with LED ON!");
    return;
  }

  if (af->af_mode.mode == AF_MODE_MANUAL ||
    af->af_mode.mode == AF_MODE_MANUAL_HAL1 || !af->af_ctrl_mode) {
    AF_LOW(" AF control mode is OFF (Manual)! Stop processing stats!");
    return;
  }
  // Set ISP config flag
  if (af->isp_reconfig_needed) {
    af->af_input.force_roi_reconfig = TRUE;
    af->isp_reconfig_needed = FALSE;
  } else {
    af->af_input.force_roi_reconfig = FALSE;
  }

  if (af_biz_map_stats_input(af, stats) < 0) {
    AF_LOW(" InValid Stats! Stop processing stats!");
    return;
  }

  // Set roi changed flag
  if (af->roi_changed) {
    af->af_input.roi_changed = TRUE;
    af->roi_changed = FALSE;
  } else {
    af->af_input.roi_changed = FALSE;
  }

  if (af->af_algo_ops.process) {
    result = af->af_algo_ops.process(af->af_core, &af->af_input, &af->af_core_out);
  } else {
    result = Q3A_CORE_RESULT_FAILED;
    AF_ERR("Error: process() not set");
  }
  if (result == Q3A_CORE_RESULT_SUCCESS) {
    af_biz_handle_core_output(af);
    af_biz_util_pack_output(af, output);
  }

  // Resetting the depth information
  af->af_input.stats_info.depth_stats.mask = AF_DAAF_NONE;
} /* af_biz_process */

/** af_biz_destroy
 *
 **/
void af_biz_destroy(void *af_obj)
{
  af_biz_internal *af =
    (af_biz_internal *)af_obj;

  AF_LOW("af_biz: %p", af);
  if (af == NULL) {
    return;
  }

  // deinit internal and free any malloc
  if (af->af_algo_ops.deinit) {
    af->af_algo_ops.deinit(af->af_core);
  }

  af_biz_clear_algo_ops(&af->af_algo_ops);
  if (af) {
    if (af->af_out.debug_info) {
      free(af->af_out.debug_info);
    }
    free(af);
  }
  AF_LOW("X");
}

/**
 * af_biz_load_function
 *
 * @af_ops: structure with function pointers to be assign
 *
 * Return: Handler to AF interface library
 **/
void * af_biz_load_function(af_ops_t *af_ops)
{
  void *q3a_handler = NULL;

  if (!af_ops) {
    return NULL;
  }

  q3a_handler = dlopen("libmmcamera2_q3a_core.so", RTLD_NOW);
  if (!q3a_handler) {
    AF_ERR("dlerror: %s", dlerror());
    return NULL;
  }

  af_ops->init = af_biz_init;
  af_ops->deinit = af_biz_destroy;
  af_ops->process = af_biz_process;
  af_ops->set_parameters = af_biz_set_parameters;
  af_ops->get_parameters = af_biz_get_parameters;

  return q3a_handler;
}

/**
 * af_biz_unload_function
 *
 * @af_ops: structure with function pointers to be assign
 * @lib_handler: Handler to the algo library
 *
 * Return: void
 **/
void af_biz_unload_function(af_ops_t *af_ops, void *lib_handler)
{
  if (lib_handler) {
    dlclose(lib_handler);
  }
  af_biz_clear_iface_ops(af_ops);

  return;
}
