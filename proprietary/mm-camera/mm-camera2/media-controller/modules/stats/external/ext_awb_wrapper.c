/* ext_awb_wrapper.c
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "awb.h"
#include "ext_awb_wrapper.h"
#include "q3a_platform.h"
#include <dlfcn.h>

/** ext_awb_algo_get_param
 *
 *    @param: get parameter
 *    @obj: Handler to library
 *
 * Return: boolean
 **/
boolean ext_awb_algo_get_param(awb_get_parameter_t *param, void *obj)
{
  boolean ret = TRUE;
  ext_awb_algo_control_t *awb = (ext_awb_algo_control_t *)obj;
  int i;

  if (!param || !awb) {
    return FALSE;
  }

  switch (param->type){
  case AWB_GAINS:
    {
      /* algo */
      ext_awb_algo_get_args_t args;
      if (awb->ops.get_gains(awb->opsptr, &args)) {
        awb->normal.gains.r_gain = args.wbgain_r;
        awb->normal.gains.g_gain = args.wbgain_g;
        awb->normal.gains.b_gain = args.wbgain_b;
        awb->normal.colortemp = (uint32_t)args.ctemp;
        awb->normal.ccm_enable = TRUE;
        awb->normal.ccm.override_ccm = TRUE;
        STATS_MEMCPY(&awb->normal.ccm.ccm, sizeof(awb->normal.ccm.ccm),
          &args.ccm, sizeof(args.ccm));
        AWB_LOW("gain: r:g:b[%f:%f:%f] ctemp[%d] "
          "ccm[%f:%f:%f:%f:%f:%f:%f:%f:%f]",
          args.wbgain_r, args.wbgain_g, args.wbgain_b,
          args.ctemp,
          args.ccm[0],
          args.ccm[1],
          args.ccm[2],
          args.ccm[3],
          args.ccm[4],
          args.ccm[5],
          args.ccm[6],
          args.ccm[7],
          args.ccm[8]);
      }
    }
    if ((awb->op_mode == Q3A_OPERATION_MODE_SNAPSHOT) && (awb->use_led_aec == TRUE)) {
      param->u.awb_gains.curr_gains.r_gain = awb->led.gains.r_gain;
      param->u.awb_gains.curr_gains.g_gain = awb->led.gains.g_gain;
      param->u.awb_gains.curr_gains.b_gain = awb->led.gains.b_gain;
      param->u.awb_gains.color_temp = awb->led.colortemp;
    } else {
      param->u.awb_gains.curr_gains.r_gain = awb->normal.gains.r_gain;
      param->u.awb_gains.curr_gains.g_gain = awb->normal.gains.g_gain;
      param->u.awb_gains.curr_gains.b_gain = awb->normal.gains.b_gain;
      param->u.awb_gains.color_temp = awb->normal.colortemp;
    }
    param->u.awb_gains.awb_ccm_enable = awb->normal.ccm_enable;
    param->u.awb_gains.ccm.override_ccm = awb->normal.ccm.override_ccm;
    STATS_MEMCPY(&param->u.awb_gains.ccm.ccm, sizeof(param->u.awb_gains.ccm.ccm),
      &awb->normal.ccm.ccm, sizeof(awb->normal.ccm.ccm));
    AWB_LOW("gain: r:g:b[%f:%f:%f] ctemp[%d] "
      "ccm[%f:%f:%f:%f:%f:%f:%f:%f:%f]",
      param->u.awb_gains.curr_gains.r_gain,
      param->u.awb_gains.curr_gains.g_gain,
      param->u.awb_gains.curr_gains.b_gain,
      param->u.awb_gains.color_temp,
      param->u.awb_gains.ccm.ccm[0][0],
      param->u.awb_gains.ccm.ccm[0][1],
      param->u.awb_gains.ccm.ccm[0][2],
      param->u.awb_gains.ccm.ccm[1][0],
      param->u.awb_gains.ccm.ccm[1][1],
      param->u.awb_gains.ccm.ccm[1][2],
      param->u.awb_gains.ccm.ccm[2][0],
      param->u.awb_gains.ccm.ccm[2][1],
      param->u.awb_gains.ccm.ccm[2][2]);
    break;
  case AWB_PARMS:
    {
      /* algo */
      ext_awb_algo_get_args_t args;
      if (awb->ops.get_gains(awb->opsptr, &args)) {
        awb->normal.gains.r_gain = args.wbgain_r;
        awb->normal.gains.g_gain = args.wbgain_g;
        awb->normal.gains.b_gain = args.wbgain_b;
        awb->normal.colortemp = (uint32_t)args.ctemp;
      }
    }
    param->u.awb_gains.curr_gains.r_gain = awb->normal.gains.r_gain;
    param->u.awb_gains.curr_gains.g_gain = awb->normal.gains.g_gain;
    param->u.awb_gains.curr_gains.b_gain = awb->normal.gains.b_gain;
    param->u.awb_gains.color_temp = awb->normal.colortemp;
    AWB_HIGH("gain: r:g:b[%f:%f:%f] ctemp[%d]",
      param->u.awb_gains.curr_gains.r_gain,
      param->u.awb_gains.curr_gains.g_gain,
      param->u.awb_gains.curr_gains.b_gain,
      param->u.awb_gains.color_temp);
    break;
  case AWB_UNIFIED_FLASH:
    for (i = 0; i < param->u.frame_info.num_batch; i++) {
      if ((param->u.frame_info.frame_batch[i].flash_mode == TRUE) &&
        (awb->use_led_aec == TRUE)) {
        param->u.frame_info.frame_batch[i].r_gain = awb->led.gains.r_gain;
        param->u.frame_info.frame_batch[i].b_gain = awb->led.gains.b_gain;
        param->u.frame_info.frame_batch[i].g_gain = awb->led.gains.g_gain;
        param->u.frame_info.frame_batch[i].color_temp = awb->led.colortemp;
        param->u.frame_info.frame_batch[i].ccm = awb->led.ccm;
      } else {
        param->u.frame_info.frame_batch[i].r_gain = awb->normal.gains.r_gain;
        param->u.frame_info.frame_batch[i].b_gain = awb->normal.gains.b_gain;
        param->u.frame_info.frame_batch[i].g_gain = awb->normal.gains.g_gain;
        param->u.frame_info.frame_batch[i].color_temp = awb->normal.colortemp;
        param->u.frame_info.frame_batch[i].ccm = awb->normal.ccm;
      }
      AWB_HIGH("idx[%d] unified[%f:%f:%f] ctemp[%d] "
        "ccm[%f:%f:%f:%f:%f:%f:%f:%f:%f]",
        i,
        param->u.frame_info.frame_batch[i].r_gain,
        param->u.frame_info.frame_batch[i].g_gain,
        param->u.frame_info.frame_batch[i].b_gain,
        param->u.frame_info.frame_batch[i].color_temp,
        param->u.frame_info.frame_batch[i].ccm.ccm[0][0],
        param->u.frame_info.frame_batch[i].ccm.ccm[0][1],
        param->u.frame_info.frame_batch[i].ccm.ccm[0][2],
        param->u.frame_info.frame_batch[i].ccm.ccm[1][0],
        param->u.frame_info.frame_batch[i].ccm.ccm[1][1],
        param->u.frame_info.frame_batch[i].ccm.ccm[1][2],
        param->u.frame_info.frame_batch[i].ccm.ccm[2][0],
        param->u.frame_info.frame_batch[i].ccm.ccm[2][1],
        param->u.frame_info.frame_batch[i].ccm.ccm[2][2]);
    }
    break;
  case AWB_REQUIRED_STATS:
    if (param->u.request_stats.supported_stats_mask & (1 << MSM_ISP_STATS_BG)) {
      param->u.request_stats.enable_stats_mask = (1 << MSM_ISP_STATS_BG);
    }
    awb->stats_enable.awb_enable_stats_mask =
      param->u.request_stats.enable_stats_mask;
    if (param->u.request_stats.supported_stats_mask &
        (1 << MSM_ISP_STATS_AEC_BG)) {
      awb->stats_enable.awb_config_stats_mask =
        awb->stats_enable.awb_enable_stats_mask & (1 << MSM_ISP_STATS_BG);
    }
    break;
  default:
    AWB_LOW("No type, param_type %d", param->type);
    ret = FALSE;
    break;
  }

  AWB_LOW("Done, param_type %d, obj %p, ret %d", param->type, obj, ret);
  return ret;
}

static boolean ext_awb_algo_init_chromatix(ext_awb_algo_control_t *awb,
    awb_set_parameter_init_t* init_param)
{
  chromatix_3a_parms_wrapper *chromatix_wrapper;
  awb_algo_tuning_adapter* bayer_awb_param;

  if (!awb || !init_param) {
    return FALSE;
  }

  chromatix_wrapper = (chromatix_3a_parms_wrapper*) init_param->chromatix;
  if (!chromatix_wrapper) {
    return FALSE;
  }

  bayer_awb_param = &chromatix_wrapper->AWB;

  {
    /* algo */
    ext_awb_algo_set_init_args_t args;
    args.color_ratio[AGW_AWB_D65].rg =
      bayer_awb_param->awb_basic_tuning.reference[AGW_AWB_D65].RG_ratio;
    args.color_ratio[AGW_AWB_D65].bg =
      bayer_awb_param->awb_basic_tuning.reference[AGW_AWB_D65].BG_ratio;
    args.valid_num = 1;
    if (awb->ops.init_chromatix(awb->opsptr, &args)) {
      awb->chromatix.color_ratio[AGW_AWB_D65].rg =
        bayer_awb_param->awb_basic_tuning.reference[AGW_AWB_D65].RG_ratio;
      awb->chromatix.color_ratio[AGW_AWB_D65].bg =
        bayer_awb_param->awb_basic_tuning.reference[AGW_AWB_D65].BG_ratio;
      awb->chromatix.valid_num = 1;
    }
  }

  return TRUE;
}

static boolean ext_awb_algo_pack_output(ext_awb_algo_control_t *awb,
    awb_output_data_t *output)
{
  STATS_MEMSET(output, 0, sizeof(awb_output_data_t));

  output->type = AWB_UPDATE;
  output->awb_update = TRUE;
  output->sof_id = awb->sof_id;
  output->frame_id = awb->frame_id;
  output->wb_mode = awb->wb;
  output->awb_lock = awb->awb_locked;

  output->awb_ccm_enable = awb->normal.ccm_enable;
  output->ccm.override_ccm = awb->normal.ccm.override_ccm;

  output->r_gain = awb->normal.gains.r_gain;
  output->g_gain = awb->normal.gains.g_gain;
  output->b_gain = awb->normal.gains.b_gain;
  output->unadjusted_r_gain = awb->normal.gains.r_gain;
  output->unadjusted_g_gain = awb->normal.gains.g_gain;
  output->unadjusted_b_gain = awb->normal.gains.b_gain;
  output->color_temp = (int)awb->normal.colortemp;
  output->ccm = awb->normal.ccm;
  output->dual_led_settings = awb->dual_led_info;

  if (awb->exif && awb->exif_size) {
    STATS_MEMCPY(output->awb_debug_data_array, sizeof(output->awb_debug_data_array),
      awb->exif, awb->exif_size);
    output->awb_debug_data_size = awb->exif_size;
  }

  AWB_LOW("sof_id %d, frame_id %d gain[%f:%f:%f] ctemp[%d]"
    "[%d:%d:%d:%d] pre-awb:af[%d:%d] flash[%d] exif_size[%d]",
    output->sof_id, output->frame_id,
    output->r_gain, output->g_gain, output->b_gain,
    output->color_temp,
    awb->op_mode,
    awb->est_state,
    awb->use_led_aec,
    awb->use_led_algo,
    awb->is_preflash_awb,
    awb->is_preflash_af,
    awb->is_flash,
    awb->exif_size);

  return TRUE;
}

static void ext_awb_algo_translate_parm(ext_awb_algo_control_t *awb,
    awb_set_aec_parms *aec_parms, ext_awb_algo_set_aec_t *args)
{
  /* led_state: 3 states is available(AEC_EST_START/AEC_EST_DONE/AEC_EST_OFF)
   * is_ledgain: Needs AEC_EST_DONE and use_led_algo = TRUE.
   * is_flash: if TRUE, Whole flash period.
   * is_preflash_awb: if TRUE, inside Pre-flash of AEC/AWB period.
   * is_preflash_af: if TRUE, inside Pre-flash of AF period.
   * is_preflash: if TRUE, inside Pre-flash.
   * is_restoregain: if TRUE, restore Normal Gain in Flash period. */

  /* Translate "est_state" and "use_led_estimation" */
  if (aec_parms->est_state == AEC_EST_START) {
    args->led_state = AEC_EST_START;
    args->use_led_algo = FALSE;
  } else if (aec_parms->est_state == AEC_EST_DONE_FOR_AF) {
    args->led_state = AEC_EST_DONE;
    args->use_led_algo = TRUE;
  } else if (aec_parms->est_state == AEC_EST_DONE) {
    args->led_state = AEC_EST_DONE;
    args->use_led_algo = TRUE;
  } else {
    args->led_state = AEC_EST_OFF;
    args->use_led_algo = FALSE;
  }

  /* Set "is_preflash". */
  if (aec_parms->est_state == AEC_EST_START) {
    awb->is_preflash_awb = TRUE;
  }
  if (aec_parms->est_state == AEC_EST_DONE_FOR_AF) {
    awb->is_preflash_af = TRUE;
  }
  if ((aec_parms->est_state == AEC_EST_DONE) ||
    (aec_parms->est_state == AEC_EST_DONE_SKIP)) {
    awb->is_preflash_awb = FALSE;
    awb->is_preflash_af = FALSE;
  }
  awb->is_preflash = awb->is_preflash_awb || awb->is_preflash_af;

  /* Set "is_ledgain" */
  if (args->led_state == AEC_EST_DONE) {
    awb->is_ledgain = TRUE;
    AWB_LOW("Prepared all flag for LEDgain. Normal state.");
  }

  /* Clear "is_ledgain_valid". */
  if (awb->op_mode == Q3A_OPERATION_MODE_SNAPSHOT) {
    awb->is_ledgain_valid = FALSE;
  }

  /* Set "is_restoregain" with HAL */
  if (!awb->is_preflash && awb->awb_locked) {
    awb->is_restoregain = TRUE;
    args->use_led_algo = FALSE;
  } else {
    awb->is_restoregain = FALSE;
  }

  /* Set "is_flash". */
  awb->is_flash = awb->is_preflash || args->use_led_algo ||
    awb->use_led_aec || awb->awb_locked;

  if (awb->is_preflash_af) {
    args->led_state = AEC_EST_DONE;
    args->use_led_algo = TRUE;
    awb->is_ledgain = TRUE;
  }
}

static boolean ext_awb_algo_aec_parm(ext_awb_algo_control_t *awb,
    awb_set_aec_parms  *aec_parms)
{
  if (!awb || !aec_parms) {
    return FALSE;
  }

  {
    /* algo */
    ext_awb_algo_set_aec_t args;
    args.Av_wb = aec_parms->Av;
    args.Bv_wb = aec_parms->Bv;
    args.Tv_wb = aec_parms->Tv;
    args.Sv_wb = aec_parms->Sv;
    args.flash_sensitivity_off = aec_parms->flash_sensitivity.off;
    args.flash_sensitivity_low = aec_parms->flash_sensitivity.low;
    args.flash_sensitivity_high = aec_parms->flash_sensitivity.high;
    args.led_mode = aec_parms->led_mode;
    args.use_led_algo = aec_parms->use_led_estimation;

    ext_awb_algo_translate_parm(awb, aec_parms, &args);

    if (awb->ops.set_aec(awb->opsptr, &args)) {
      awb->Av = args.Av_wb;
      awb->Bv = args.Bv_wb;
      awb->Tv = args.Tv_wb;
      awb->Sv = args.Sv_wb;
      awb->est_state = args.led_state;
      awb->use_led_aec = aec_parms->use_led_estimation;
      awb->use_led_algo = args.use_led_algo;
      awb->flash_si.off = args.flash_sensitivity_off;
      awb->flash_si.low = args.flash_sensitivity_low;
      awb->flash_si.high = args.flash_sensitivity_high;
    }
  }

  AWB_LOW("Bv:Tv:Sv:Av[%f:%f:%f:%f], led-est:use_aec:use_algo[%d:%d:%d] "
    "mode-op:led[%d:%d] flash-total:pre[%d:%d] restore[%d]",
    aec_parms->Bv, aec_parms->Tv, aec_parms->Sv, aec_parms->Av,
    awb->est_state, awb->use_led_aec, awb->use_led_algo,
    awb->op_mode, aec_parms->led_mode,
    awb->is_flash, awb->is_preflash,
    awb->is_restoregain);

  return TRUE;
}

static boolean ext_awb_algo_bestshot(ext_awb_algo_control_t *awb,
    awb_bestshot_mode_type_t type)
{
  boolean ret = TRUE;

  if (!awb) {
    return FALSE;
  }

  if (type >= AWB_BESTSHOT_MAX) {
    type = AWB_BESTSHOT_OFF;
    AWB_ERR("Invalid bestshot mode, setting it to default");
  }

  if (awb->bestshot.curr_mode == type) {
    return FALSE;
  }

  {
    /* algo */
    int args = (int)type;
    if (awb->ops.set_bestshot(awb->opsptr, (void *)&args))
      awb->bestshot.curr_mode = type;
  }

  AWB_HIGH("bestshot %d", type);

  return ret;
}

static boolean ext_awb_algo_wb(ext_awb_algo_control_t *awb, uint32_t type)
{
  boolean ret = TRUE;

  if (!awb) {
    return FALSE;
  }

  if (awb->wb == (awb_config3a_wb_t)type) {
    return FALSE;
  }

  {
    /* algo */
    int args = (int)type;
    if (awb->ops.set_whitebalance(awb->opsptr, &args))
      awb->wb = type;
  }

  AWB_HIGH("wb %d", type);

  return ret;
}

static boolean ext_awb_algo_opmode(ext_awb_algo_control_t *awb, int32_t opmode)
{
  boolean ret = TRUE;

  if (!awb) {
    return FALSE;
  }

  {
    /* algo */
    int32_t args = (int32_t)opmode;
    if (awb->ops.set_opmode(awb->opsptr, &args))
      awb->op_mode = opmode;
  }

  AWB_HIGH("opmode %d", awb->op_mode);

  return ret;
}

static boolean ext_awb_algo_lock(ext_awb_algo_control_t *awb, boolean lock)
{
  boolean ret = TRUE;

  if (!awb) {
    return FALSE;
  }

  {
    /* algo */
    int args = (int)lock;
    if (awb->ops.set_lock(awb->opsptr, &args))
      awb->awb_locked = args ? TRUE : FALSE;
  }

  AWB_HIGH("lock %d", lock);

  return ret;
}

/** ext_awb_algo_set_param
 *
 *    @param: get parameter
 *    @output: output
 *    @obj: Handler to library
 *
 * Return: boolean
 **/
boolean ext_awb_algo_set_param(awb_set_parameter_t *param,
  awb_output_data_t *output, uint8_t num_of_out, void *obj)
{
  boolean ret = TRUE;
  ext_awb_algo_control_t *awb = (ext_awb_algo_control_t *)obj;
  (void)num_of_out;

  if (!param || !obj) {
    return FALSE;
  }

  switch (param->type) {
  case AWB_SET_PARAM_INIT_CHROMATIX_SENSOR:
    ret = ext_awb_algo_init_chromatix(awb, &param->u.init_param);
    break;
  case AWB_SET_PARAM_PACK_OUTPUT:
    awb->sof_id = param->u.current_sof_id;
    ret = ext_awb_algo_pack_output(awb, output);
    break;
  case AWB_SET_PARAM_AEC_PARM:
    ret = ext_awb_algo_aec_parm(awb, &param->u.aec_parms);
    break;
  case AWB_SET_PARAM_WHITE_BALANCE:
    if (awb->bestshot.curr_mode == AWB_BESTSHOT_OFF) {
      ret = ext_awb_algo_wb(awb, param->u.awb_current_wb);
    }
    else
      awb->bestshot.stored_wb = param->u.awb_current_wb;
    break;
  case AWB_SET_PARAM_BESTSHOT:
    ret = ext_awb_algo_bestshot(awb,param->u.awb_best_shot);
    break;
  case AWB_SET_PARAM_OP_MODE:
    ret = ext_awb_algo_opmode(awb, param->u.init_param.op_mode);
    break;
  case AWB_SET_PARAM_LOCK:
    ret = ext_awb_algo_lock(awb, param->u.awb_lock);
    break;
  case AWB_SET_PARAM_CROP_INFO:
  case AWB_SET_PARAM_UI_FRAME_DIM:
  case AWB_SET_PARAM_INIT_SENSOR_INFO:
  case AWB_SET_PARAM_VIDEO_HDR:
  case AWB_SET_PARAM_SNAPSHOT_HDR:
  case AWB_SET_PARAM_STATS_DEBUG_MASK:
  case AWB_SET_PARAM_ENABLE:
  case AWB_SET_PARAM_EZ_TUNE_RUNNING:
  case AWB_SET_MANUAL_AUTO_SKIP:
  case AWB_SET_PARAM_CAPTURE_MODE:
  case AWB_SET_PARAM_EZ_DUAL_LED_FORCE_IDX:
  case AWB_SET_PARAM_STATS_DEPTH:
  case AWB_SET_PARM_FAST_AEC_DATA:
  default:
    AWB_LOW("No type, param_type %d", param->type);
    ret = FALSE;
    break;
 }

  AWB_LOW("Done, param_type %d, obj %p, ret %d", param->type, obj, ret);
  return ret;
}

void awb_convert_stats_awb_data_gx(uint32_t *gxave,
    uint32_t *grsum, uint32_t *gbsum,
    uint32_t bcnth, uint32_t bcntv)
{
  uint16_t v, h;

  for (v = 0; v < bcntv; v++)
    for (h = 0; h < bcnth; h++)
      (*gxave++) = (uint32_t)((*grsum++) + (*gbsum++)) >> 1;
}

void awb_convert_stats(ext_awb_algo_process_inargs_t *args, stats_t *stats)
{
  uint32_t auiBg_BlkH = stats->bayer_stats.p_q3a_bg_stats->bg_region_h_num;
  uint32_t auiBg_BlkV = stats->bayer_stats.p_q3a_bg_stats->bg_region_v_num;
  uint32_t pixelH = stats->bayer_stats.p_q3a_bg_stats->bg_region_width;
  uint32_t pixelV = stats->bayer_stats.p_q3a_bg_stats->bg_region_height;

  /* R */
  STATS_MEMCPY(args->r, sizeof(args->r), stats->bayer_stats.p_q3a_bg_stats->bg_r_sum,
    auiBg_BlkH * auiBg_BlkV * sizeof(stats->bayer_stats.p_q3a_bg_stats->bg_r_sum[0]));

  /* Ave val for Gr/Gb data */
  awb_convert_stats_awb_data_gx((uint32_t *)&args->g[0],
      &stats->bayer_stats.p_q3a_bg_stats->bg_gr_sum[0],
      &stats->bayer_stats.p_q3a_bg_stats->bg_gb_sum[0],
      auiBg_BlkH, auiBg_BlkV);

  /* B data */
  STATS_MEMCPY(args->b, sizeof(args->b), stats->bayer_stats.p_q3a_bg_stats->bg_b_sum,
      auiBg_BlkH * auiBg_BlkV * sizeof(stats->bayer_stats.p_q3a_bg_stats->bg_b_sum[0]));

  /* Count data */
  STATS_MEMCPY(args->c, sizeof(args->c[0]), stats->bayer_stats.p_q3a_bg_stats->bg_gr_num,
      auiBg_BlkH * auiBg_BlkV * sizeof(stats->bayer_stats.p_q3a_bg_stats->bg_gr_num[0]));

  args->blockH = auiBg_BlkH;
  args->blockV = auiBg_BlkV;
  args->blockPixH = pixelH;
  args->blockPixV = pixelV;

  {
    int i;
    uint32_t auiR = 0;
    uint32_t auiGr = 0;
    uint32_t auiGb = 0;
    uint32_t auiB = 0;
    for (i = 0; i < ROI_CNT; i++) {
      auiR += stats->bayer_stats.p_q3a_bg_stats->bg_r_sum[i];
      auiGr += stats->bayer_stats.p_q3a_bg_stats->bg_gr_sum[i];
      auiGb += stats->bayer_stats.p_q3a_bg_stats->bg_gb_sum[i];
      auiB += stats->bayer_stats.p_q3a_bg_stats->bg_b_sum[i];
    }
  }
}

static void ext_awb_algo_result(ext_awb_algo_control_t *awb,
    ext_awb_algo_process_outargs_t *args)
{
  awb->normal.gains.r_gain = args->result.wbgain_r;
  awb->normal.gains.g_gain = args->result.wbgain_g;
  awb->normal.gains.b_gain = args->result.wbgain_b;
  awb->normal.colortemp = (uint32_t)args->result.ctemp;
  STATS_MEMCPY(awb->normal.ccm.ccm, sizeof(awb->normal.ccm.ccm),
    args->result.ccm, sizeof(args->result.ccm));
  awb->normal.ccm_enable = TRUE;  /* Always being returned CCM from algo */
  awb->normal.ccm.override_ccm = TRUE;
  AWB_LOW("result: gain: id[%d] r:g:b[%f:%f:%f] ctemp[%d]"
    "ccm[%f:%f:%f:%f:%f:%f:%f:%f:%f]",
    awb->frame_id,
    args->result.wbgain_r, args->result.wbgain_g, args->result.wbgain_b,
    args->result.ctemp,
    args->result.ccm[0],
    args->result.ccm[1],
    args->result.ccm[2],
    args->result.ccm[3],
    args->result.ccm[4],
    args->result.ccm[5],
    args->result.ccm[6],
    args->result.ccm[7],
    args->result.ccm[8]);
}

static void ext_awb_algo_update_dual_led(ext_awb_algo_control_t *awb,
    ext_awb_algo_get_args_t *args)
{
  ext_awb_algo_dual_led_info_t *info;

  info = (ext_awb_algo_dual_led_info_t *)&args->dual_led_info;

  if (awb->is_ledgain) {
    /* For main-flash */
    awb->dual_led_info.led1_high_setting = info->led_cool;
    awb->dual_led_info.led2_high_setting = info->led_warm;
  } else {
    /* For pre-flash */
    awb->dual_led_info.led1_low_setting = info->led_cool;
    awb->dual_led_info.led2_low_setting = info->led_warm;
  }
}

static void ext_awb_algo_exif(ext_awb_algo_control_t *awb,
    ext_awb_algo_process_outargs_t *out)
{
  if (out->debug_information && out->debug_information_size) {
    awb->exif = out->debug_information;
    awb->exif_size = out->debug_information_size;
  } else {
    AWB_HIGH("Warning. AWB exif[%p] size[%d]", awb->exif, awb->exif_size);
  }
}

static boolean is_ledgain_available(ext_awb_algo_control_t *awb)
{
  if ((awb->led.gains.r_gain == 0) ||
    (awb->led.gains.g_gain == 0) ||
    (awb->led.gains.b_gain == 0)) {

    /* Just preventing black capture image. */
    AWB_ERR("Warning: got wrong LEDgain. Prepare Normal preview gain.");
    awb->led.gains.r_gain = awb->normal.gains.r_gain;
    awb->led.gains.g_gain = awb->normal.gains.g_gain;
    awb->led.gains.b_gain = awb->normal.gains.b_gain;
    STATS_MEMCPY(awb->led.ccm.ccm, sizeof(awb->led.ccm.ccm),
      awb->normal.ccm.ccm, sizeof(awb->normal.ccm.ccm));

    return FALSE;
  } else {
    AWB_LOW("got LEDgain, valid.");
    return TRUE;
  }
}

/** ext_awb_algo_process
 *
 *    @stats: stats structures
 *    @obj: Handler to library
 *    @output: output
 *
 * Return: void
 **/
void ext_awb_algo_process(stats_t *stats, void *obj, awb_output_data_t *output,
  uint8_t num_of_output)
{
  (void)output;
  (void)num_of_output;
  ext_awb_algo_control_t *awb = (ext_awb_algo_control_t *)obj;
  ext_awb_algo_process_inargs_t   args_in;
  ext_awb_algo_process_outargs_t  args_out;
  ext_awb_algo_get_args_t args;

  awb->stats = *stats;

  switch (awb->op_mode) {
  case Q3A_OPERATION_MODE_PREVIEW:
  case Q3A_OPERATION_MODE_CAMCORDER:

    awb->frame_id = stats->frame_id;

    awb_convert_stats(&args_in, stats);

    if (awb->ops.process(awb->opsptr, &args_in, &args_out)) {
      if (!awb->is_preflash_af)
        ext_awb_algo_result(awb, &args_out);
      ext_awb_algo_exif(awb, &args_out);
      ext_awb_algo_update_dual_led(awb, &args_out.result);
    }

    if (awb->is_ledgain) {
      if (awb->ops.get_pre_led_gains(awb->opsptr, &args)) {
        awb->led.gains.r_gain = args.wbgain_r;
        awb->led.gains.g_gain = args.wbgain_g;
        awb->led.gains.b_gain = args.wbgain_b;
        awb->led.colortemp = (uint32_t)args.ctemp;
        STATS_MEMCPY(awb->led.ccm.ccm, sizeof(awb->led.ccm.ccm),
          args.ccm, sizeof(args.ccm));
        ext_awb_algo_update_dual_led(awb, &args);
      }
      awb->is_ledgain = FALSE;
      awb->is_ledgain_valid = is_ledgain_available(awb);
    }

    if (!awb->is_flash) {
      awb->keepgain.gains.r_gain = awb->normal.gains.r_gain;
      awb->keepgain.gains.g_gain = awb->normal.gains.g_gain;
      awb->keepgain.gains.b_gain = awb->normal.gains.b_gain;
      awb->keepgain.colortemp = awb->normal.colortemp;
      awb->keepgain.ccm_enable = TRUE;
    }
    if (awb->is_restoregain) {
      awb->normal.gains.r_gain = awb->keepgain.gains.r_gain;
      awb->normal.gains.g_gain = awb->keepgain.gains.g_gain;
      awb->normal.gains.b_gain = awb->keepgain.gains.b_gain;
      awb->normal.colortemp = awb->keepgain.colortemp;
      awb->normal.ccm_enable = TRUE;
    }
    break;

  case Q3A_OPERATION_MODE_SNAPSHOT:
    awb->normal.gains.r_gain = awb->keepgain.gains.r_gain;
    awb->normal.gains.g_gain = awb->keepgain.gains.g_gain;
    awb->normal.gains.b_gain = awb->keepgain.gains.b_gain;
    awb->normal.colortemp = awb->keepgain.colortemp;
    awb->normal.ccm_enable = TRUE;
    break;

  default:
    AWB_ERR("invalid awb op_mode: %d", awb->op_mode);
    break;
  }

  if ((awb->op_mode == Q3A_OPERATION_MODE_PREVIEW) ||
    (awb->op_mode == Q3A_OPERATION_MODE_CAMCORDER) ||
    (awb->op_mode == Q3A_OPERATION_MODE_SNAPSHOT)) {
    AWB_LOW("[%d:%d:%d] normal[%f:%f:%f] ctemp[%d], led[%f:%f:%f] ctemp[%d] "
        "res[%d]",
      awb->est_state, awb->use_led_aec, awb->use_led_algo,
      awb->normal.gains.r_gain,
      awb->normal.gains.g_gain,
      awb->normal.gains.b_gain,
      awb->normal.colortemp,
      awb->led.gains.r_gain,
      awb->led.gains.g_gain,
      awb->led.gains.b_gain,
      awb->led.colortemp,
      awb->is_restoregain);
  }
}

static boolean ext_awb_algo_dlsym(void *libptr, void *fptr, const char *fname)
{
  char *error = NULL;

  if (libptr == NULL || fptr == NULL) {
    AWB_ERR("Error, Loading %s", fname);
    return FALSE;
  }

  *(void **)(fptr) = dlsym(libptr, fname);
  if (!*(void **)fptr) {
    error = (char *)dlerror();
    AWB_ERR("Error, dlerror: %s, fname %s, fptr %p", error, fname, fptr);
    return FALSE;
  }

  AWB_HIGH("Loaded %s %p", fname, fptr);
  return TRUE;
}

boolean ext_awb_algo_estimate_cct(void *obj,
  float rgain, float ggain, float bgain, float *colortemp, uint8_t camera_id)
{
  ext_awb_algo_control_t *awb = (ext_awb_algo_control_t *)obj;
  (void)camera_id;

  if (!awb) {
    return FALSE;
  }

  /* algo */
  if (awb->ops.estimate_cct(awb->opsptr, rgain, ggain, bgain, colortemp)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

boolean ext_awb_algo_estimate_ccm(void *obj, float cct, awb_ccm_type *ccm,
  uint8_t camera_id)
{
  ext_awb_algo_control_t *awb = (ext_awb_algo_control_t *)obj;
  (void)camera_id;

  if (!awb) {
    return FALSE;
  }

  /* algo */
  if (awb->ops.estimate_ccm(awb->opsptr, cct, (float *)&ccm->ccm)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/** ext_awb_algo_init
 *
 *    @obj: Handler to library
 *    @position: camera position
 *
 * Return: void * (library handler)
 **/
static void *ext_awb_algo_init(void *obj, cam_position_t position)
{
  ext_awb_algo_control_t *awb = NULL;
  int ret;

  AWB_LOW("obj %p, camposition %d", obj, position);

  if (NULL == obj) {
    return NULL;
  }

  awb = (ext_awb_algo_control_t *)AWB_MALLOC(sizeof(ext_awb_algo_control_t));
  if (NULL == awb) {
    return NULL;
  }

  STATS_MEMSET(awb, 0, sizeof(ext_awb_algo_control_t));

  if (obj) {
    awb->libptr = obj;
  } else {
    awb->libptr = dlopen("libalawb_algo.so", RTLD_NOW);
    if (!awb->libptr) {
      AWB_ERR("Error, dlerror %s", dlerror());
      return NULL;
    }
  }

  dlerror();

  ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.init_front,
      "alawb_algo_init_front");
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.init_rear,
        "alawb_algo_init_rear");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.deinit,
        "alawb_algo_deinit");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.process,
        "alawb_algo_process");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.init_chromatix,
        "alawb_algo_set_parm_init_chromatix");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.set_aec,
        "alawb_algo_set_parm_aec_parm");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.set_whitebalance,
        "alawb_algo_set_parm_whitebalance");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.set_bestshot,
        "alawb_algo_set_parm_bestshot");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.set_opmode,
        "alawb_algo_set_parm_opmode");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.set_lock,
        "alawb_algo_set_parm_lock");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.get_gains,
        "alawb_algo_get_parm_awb_gains");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.get_awb_parms,
        "alawb_algo_get_parm_awb_parms");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.get_led_gains,
        "alawb_algo_get_parm_led_gains");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.get_pre_led_gains,
        "alawb_algo_get_parm_pre_led_gains");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.estimate_cct,
        "alawb_algo_get_estimate_cct");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.estimate_ccm,
        "alawb_algo_get_estimate_ccm");
  }
  if (ret) {
    ret = ext_awb_algo_dlsym(awb->libptr, &awb->ops.get_version,
        "alawb_algo_get_version");
  }
  if (!ret) {
    AWB_ERR("Error, no symbol");
    dlclose(awb->libptr);
    awb->libptr = NULL;
    AWB_FREE(awb);
    awb = NULL;
    return NULL;
  }

  awb->position = position;

  if (awb->position == CAM_POSITION_BACK) {
    ret = awb->ops.init_rear(&awb->opsptr);
  } else if (awb->position == CAM_POSITION_FRONT) {
    ret = awb->ops.init_front(&awb->opsptr);
  } else {
    AWB_ERR("Warning, go via position value[%d]", awb->position);
    ret = 0;
  }
  if (!ret) {
    AWB_ERR("Error, init(), ret %d", ret);
    dlclose(awb->libptr);
    awb->libptr = NULL;
    awb->opsptr = NULL;
    AWB_FREE(awb);
    awb = NULL;
    return NULL;
  }

  ret = awb->ops.get_version((void *)&awb->libver);
  if (ret) {
    AWB_HIGH("model[%s] version[%s] description[%s]",
      awb->libver.model, awb->libver.version, awb->libver.description);
  }

  AWB_HIGH("Done, obj:%p libptr:%p opsptr:%p ret:%d",
    awb, awb->libptr, awb->opsptr, ret);

  return awb;
}

/** ext_awb_algo_deinit
 *
 *    @obj: Handler to library
 *
 * Return: void
 **/
void ext_awb_algo_deinit(void *obj)
{
  ext_awb_algo_control_t *awb = (ext_awb_algo_control_t *)obj;

  AWB_LOW("obj %p, cam position %d", obj, awb->position);

  if (awb) {
    if (awb->libptr && awb->opsptr) {
      awb->ops.deinit(awb->opsptr);
    }
    AWB_FREE(awb);
    awb = NULL;
  }

  AWB_HIGH("Done");
}

/** Init Callbacks according to the each camera
 *
 * Return: TRUE on success
 **/
void *ext_awb_algo_front_init(void *obj)
{
  AWB_LOW("obj %p", obj);
  return ext_awb_algo_init(obj, CAM_POSITION_FRONT);
}

void *ext_awb_algo_rear_init(void *obj)
{
  AWB_LOW("obj %p", obj);
  return ext_awb_algo_init(obj, CAM_POSITION_BACK);
}
