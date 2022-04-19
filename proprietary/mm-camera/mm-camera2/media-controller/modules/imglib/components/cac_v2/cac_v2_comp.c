/**********************************************************************
*  Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <cutils/trace.h>
#include "cac_v2_comp.h"
#include "chromatix_swpostproc.h"
#include "img_buffer.h"
#include <cutils/properties.h>

#define USE_CHROMATIX
#define USE_RNR_HYSTERISIS

#define MAX_CDS_BUF_SIZE (5000 * 4000 * .5)

/**
 * CONSTANTS and MACROS
 **/


#define ION_HEAP_ID 0
typedef struct {
  void *ptr;
  int (*cac2_process)(cac2_args_t  *args);
  int (*cac2_init)(int ion_heap_id);
  int (*cac2_deinit)();
  int (*cac2_pre_allocate_buffers)(int ion_heap_id);
  uint32_t (*cac2_destroy_buffers)();
  img_mem_handle_t workbuf;
  img_ops_core_type cds_proc_type;
} cac_lib_info_t;

static cac_lib_info_t g_cac_lib;

int cac2_comp_abort(void *handle, void *p_data);

/**
 * Function: cac2_comp_init
 *
 * Description: Initializes the Qualcomm CAC component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_userdata - the handle which is passed by the client
 *   p_data - The pointer to the parameter which is required during the
 *            init phase
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int cac2_comp_init(void *handle, void* p_userdata, void *p_data)
{
  cac2_comp_t *p_comp = (cac2_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  status = p_comp->b.ops.init(&p_comp->b, p_userdata, p_data);
  if (status < 0)
  {
    IDBG_ERROR("%s:%d] p_comp->b.ops.init returned %d",
      __func__, __LINE__,status);
    return status;
  }
 return status;
}

/**
 * Function: cac2_comp_lib_debug
 *
 * Description: Debug params for cac library
 *
 * Input parameters:
 *   p_caclib - library instance pointer
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void cac2_comp_lib_debug(cac2_args_t *p_caclib)
{
  int i =0;
  IDBG_MED("%s:%d] CACLIB pInY %p", __func__, __LINE__,
    p_caclib->p_y);
  IDBG_MED("%s:%d] CACLIB pInC %p", __func__, __LINE__,
    p_caclib->p_crcb);
  IDBG_MED("%s:%d] CACLIB fdInY %d", __func__, __LINE__,
    p_caclib->fd);
  IDBG_MED("%s:%d] CACLIB Ion Heap ID %d", __func__, __LINE__,
    p_caclib->ion_heap_id);
  IDBG_MED("%s:%d] CACLIB width %d", __func__, __LINE__,
    p_caclib->image_width);
  IDBG_MED("%s:%d] CACLIB height %d", __func__, __LINE__,
    p_caclib->image_height);
  IDBG_MED("%s:%d] CACLIB Y_stride %d", __func__, __LINE__,
    p_caclib->y_stride);
  IDBG_MED("%s:%d] CACLIB C_stride %d", __func__, __LINE__,
    p_caclib->cbcr_stride);
  IDBG_MED("%s:%d] CACLIB chromaorder %d", __func__, __LINE__,
    p_caclib->image_format);
  IDBG_MED("%s:%d] CACLIB Bright_Spot_HighTH %d", __func__, __LINE__,
    p_caclib->detection_th1);
  IDBG_MED("%s:%d] CACLIB Bright_Spot_LowTH %d", __func__, __LINE__,
    p_caclib->detection_th2);
  IDBG_MED("%s:%d] CACLIB Saturation_TH %d", __func__, __LINE__,
    p_caclib->detection_th3);
  IDBG_MED("%s:%d] CACLIB Color_Cb_TH %d", __func__, __LINE__,
    p_caclib->verification_th1);
  IDBG_MED("%s:%d] CACLIB CorrRatio_TH %d", __func__, __LINE__,
    p_caclib->correction_strength);
  IDBG_MED("%s:%d] CACLIB sampling_factor %d", __func__, __LINE__,
    p_caclib->sampling_factor);
   IDBG_MED("%s:%d] CACLIB lut_size %d", __func__, __LINE__,
    p_caclib->lut_size);
  IDBG_MED("%s:%d] CACLIB sigma_lut %p", __func__, __LINE__,
    p_caclib->sigma_lut);
  for(i =0; i < p_caclib->lut_size; i++){
    IDBG_MED("%s:%d] CACLIB sigma_lut[%d] = %f",  __func__, __LINE__,
      i, p_caclib->sigma_lut[i]);
  }
  IDBG_MED("%s:%d] CACLIB center_noise_sigma %f", __func__, __LINE__,
    p_caclib->center_noise_sigma);
  IDBG_MED("%s:%d] CACLIB center_noise_weight %f", __func__, __LINE__,
    p_caclib->center_noise_weight);
  IDBG_MED("%s:%d] CACLIB weight_order %f", __func__, __LINE__,
    p_caclib->weight_order);
  IDBG_MED("%s:%d] CACLIB scale_factor %f", __func__, __LINE__,
    p_caclib->scale_factor);
  IDBG_MED("%s:%d] CACLIB cac2_enabled %d", __func__, __LINE__,
    p_caclib->cac2_enable_flag);
  IDBG_MED("%s:%d] CACLIB rnr_enabled %d", __func__, __LINE__,
    p_caclib->rnr_enable_flag);
}

/**
 * Function: cac2_comp_get_rnr_scaling_factor
 *
 * Description: This function calculates the scaling factor for
 * the current resolution wrt the max resolution (camif o/p)
 *
 * Arguments:
 *   @p_comp: cac component
 *   @p_frame: Input image frame
 *
 * Return values:
 *     new scaling factor
 *
 * Notes: none
 **/
static float cac2_comp_get_rnr_scaling_factor(cac2_comp_t *p_comp,
  img_frame_t *p_frame)
{
  float center_x, center_y, max_radius, current_radius;
  float default_scale_factor, new_scale_factor;
  uint32_t max_width, max_height, current_width, current_height;

  // CAMIF output window size - 4208x3120
  max_width = 4208; max_height = 3120;
  default_scale_factor = 16.0f;
  center_x = (float)(max_width - 1) * 0.5f;
  center_y = (float)(max_height - 1) * 0.5f;
  max_radius = (float)p_comp->rnr_chromatix_info.lut_size - 1;

  // Current snapshot resolution
  current_width  = p_frame[0].info.width;
  current_height = p_frame[0].info.height;
  center_x = (float)(current_width - 1) * 0.5f;
  center_y = (float)(current_height - 1) * 0.5f;
  current_radius = (float)(sqrt(center_x * center_x + center_y * center_y)) /
    default_scale_factor;
  new_scale_factor = default_scale_factor * current_radius / max_radius;

  return new_scale_factor;
}

/* Function: cac2_comp_rnr_hysterisis
 *
 * Description: Helper function for rnr hysterisis
 *
 * Arguments:
 *   @trigger_pt_values: array of 4 tigger pt values
 *   @rnr_hysterisis_info hysterisis info for RNR
 *   @trigger: can be gain or lux value
 *   @trend: hysterisis trend
 *
 * Return values:
 *     Sampling factor
 *
 * Notes: none
 **/
static int cac2_comp_rnr_hysterisis(float *trigger_pt_values,
  hysterisis_info_t *rnr_hysterisis_info, float trigger, hysterisis_trend trend)
{

  int sampling_factor = 2;

  if (!trigger_pt_values) {
    IDBG_ERROR("%s %d: Null parameter for trigger_pt_values",
      __func__, __LINE__);
    return 0;
  }
  float trigger_ptA = trigger_pt_values[0];
  float trigger_ptB = trigger_pt_values[1];
  float trigger_ptC = trigger_pt_values[2];
  float trigger_ptD = trigger_pt_values[3];

  IDBG_MED("%s %d: trigger %f TpointA %f, TpointB %f, Tpointc %f, TpointD %f",
    __func__, __LINE__, trigger, trigger_ptA, trigger_ptB, trigger_ptC,
    trigger_ptD);

  if (trigger < trigger_ptA) {
    sampling_factor = 2;
    rnr_hysterisis_info->norml_hyst_enabled = FALSE;
    rnr_hysterisis_info->lowl_hyst_enabled = FALSE;
    IDBG_MED("%s %d: trigger < trigger_ptA, sampling Factor = %d",
      __func__,__LINE__, sampling_factor);
  } else if (trigger > trigger_ptD) {
    sampling_factor = 8;
    rnr_hysterisis_info->norml_hyst_enabled = FALSE;
    rnr_hysterisis_info->lowl_hyst_enabled = FALSE;
    IDBG_MED("%s %d: trigger > trigger_ptD, sampling Factor = %d",
      __func__, __LINE__, sampling_factor);
  } else if ((trigger > trigger_ptB) && (trigger < trigger_ptC)) {
    sampling_factor = 4;
    rnr_hysterisis_info->norml_hyst_enabled = FALSE;
    rnr_hysterisis_info->lowl_hyst_enabled = FALSE;
    IDBG_MED("%s %d: trigger > trigger_ptB && trigger < trigger_ptC,"
      "sampling Factor = %d", __func__, __LINE__, sampling_factor);
  } else {
    if (trigger >= trigger_ptA && trigger <= trigger_ptB) {
      if (rnr_hysterisis_info->norml_hyst_enabled) {
        if ((trend == HYSTERISIS_TREND_DOWNWARD) ||
          (trend == HYSTERISIS_TREND_UPWARD)) {
          sampling_factor = rnr_hysterisis_info->prev_sampling_factor;
          IDBG_MED("%s %d: Normal light Hysterisis, trend %d"
            "sampling Factor = %d", __func__, __LINE__, trend, sampling_factor);
        }
      } else {
        switch(trend) {
        case HYSTERISIS_TREND_UPWARD:
          sampling_factor = 2;
          break;
        case HYSTERISIS_TREND_DOWNWARD:
          sampling_factor = 4;
          break;
        case HYSTERISIS_TREND_NONE:
        default:
          IDBG_WARN("%s %d: trend NONE Invalid case,default sampling factor 2",
           __func__, __LINE__);
        break;
       }
       rnr_hysterisis_info->norml_hyst_enabled = TRUE;
       rnr_hysterisis_info->lowl_hyst_enabled = FALSE;
       IDBG_MED("%s %d: trend %d Normal Light Hysterisis enabled,"
         "sampling Factor = %d", __func__, __LINE__, trend, sampling_factor);
      }
    } else if (trigger >= trigger_ptC && trigger <= trigger_ptD) {
      if (rnr_hysterisis_info->lowl_hyst_enabled) {
        if ((trend == HYSTERISIS_TREND_DOWNWARD) ||
          (trend == HYSTERISIS_TREND_UPWARD)) {
          sampling_factor = rnr_hysterisis_info->prev_sampling_factor;
          IDBG_MED("%s %d: Low light Hysterisis, trend %d sampling Factor = %d",
            __func__, __LINE__, trend, sampling_factor);
        }
      } else {
        switch(trend) {
        case HYSTERISIS_TREND_UPWARD:
          sampling_factor = 4;
          break;
        case HYSTERISIS_TREND_DOWNWARD:
          sampling_factor = 8;
          break;
        case HYSTERISIS_TREND_NONE:
        default:
          IDBG_WARN("%s %d: trend None Invalid case, default sampling factor 2",
           __func__, __LINE__);
        break;
        }
        rnr_hysterisis_info->lowl_hyst_enabled = TRUE;
        rnr_hysterisis_info->norml_hyst_enabled = FALSE;

        IDBG_MED("%s %d: trend %d lowlHysterisis enabled,sampling Factor = %d",
          __func__, __LINE__, trend, sampling_factor);
      }
    }
  }
  return sampling_factor;
}

/**
 * Function: cac2_comp_get_sampling_factor
 *
 * Description: This function calculates the RNR sampling factor
 * using hysterisis
 *
 * Arguments:
 *   @p_client: CAC Component
 *   @chromatix_rnr: RNR structure from chromatix
 *   @trigger: current trigger -can be gain or lux value
 *
 * Return values:
 * None
 *
 * Notes: none
 **/
#ifdef USE_RNR_HYSTERISIS
static void cac2_comp_get_sampling_factor(cac2_comp_t *p_comp,
  chromatix_RNR1_type *chromatix_rnr, float trigger)
{
  float hyst_trigger_pt_values[4];

  if (chromatix_rnr->control_rnr == 0) { //lux based
    //If LUX did not change, use prev sampling factor
    if (trigger == p_comp->rnr_hysterisis_info.prev_lux_value) {
      p_comp->rnr_chromatix_info.sampling_factor =
        p_comp->rnr_hysterisis_info.prev_sampling_factor;
    } else {
      // normal light is chromatix_rnr->hysteresis_point[0]
      // low  light is chromatix_rnr->hysteresis_point[1]
      hyst_trigger_pt_values[0] =
        chromatix_rnr->hysteresis_point[0].lux_index_start;
      hyst_trigger_pt_values[1] =
        chromatix_rnr->hysteresis_point[0].lux_index_end;
      hyst_trigger_pt_values[2] =
        chromatix_rnr->hysteresis_point[1].lux_index_start;
      hyst_trigger_pt_values[3] =
        chromatix_rnr->hysteresis_point[1].lux_index_end;

      if (trigger > p_comp->rnr_hysterisis_info.prev_lux_value) {
        p_comp->rnr_hysterisis_info.lux_trend =
          HYSTERISIS_TREND_UPWARD;
      } else {
        p_comp->rnr_hysterisis_info.lux_trend =
          HYSTERISIS_TREND_DOWNWARD;
      }
      p_comp->rnr_chromatix_info.sampling_factor =
        cac2_comp_rnr_hysterisis(hyst_trigger_pt_values,
          &p_comp->rnr_hysterisis_info, trigger,
          p_comp->rnr_hysterisis_info.lux_trend);
    }
  } else {//gain based
    //If gain did not change, use prev sampling factor
    if (trigger == p_comp->rnr_hysterisis_info.prev_gain_value) {
      p_comp->rnr_chromatix_info.sampling_factor =
        p_comp->rnr_hysterisis_info.prev_sampling_factor;
    } else {
      // normal light is chromatix_rnr->hysteresis_point[0]
      // low  light is chromatix_rnr->hysteresis_point[1]
      hyst_trigger_pt_values[0] =
        chromatix_rnr->hysteresis_point[0].gain_start;
      hyst_trigger_pt_values[1] =
        chromatix_rnr->hysteresis_point[0].gain_end;
      hyst_trigger_pt_values[2] =
        chromatix_rnr->hysteresis_point[1].gain_start;
      hyst_trigger_pt_values[3] =
        chromatix_rnr->hysteresis_point[1].gain_end;

      if (trigger > p_comp->rnr_hysterisis_info.prev_gain_value) {
        p_comp->rnr_hysterisis_info.gain_trend =
          HYSTERISIS_TREND_UPWARD;
      } else {
        p_comp->rnr_hysterisis_info.gain_trend =
          HYSTERISIS_TREND_DOWNWARD;
      }
      p_comp->rnr_chromatix_info.sampling_factor =
        cac2_comp_rnr_hysterisis(hyst_trigger_pt_values,
          &p_comp->rnr_hysterisis_info, trigger,
          p_comp->rnr_hysterisis_info.gain_trend);
    }
  }
  return;
}
#endif

/**
 * Function: cac2_comp_get_rnr_lut
 *
 * Description: Get RNR LUTs from chromatix
 *
 * Arguments:
 *   @p_client: cac client
 *   @trigger: lux or gain
 *   @chromatix: chromatix pointer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
#ifdef USE_CHROMATIX
static int cac2_comp_get_rnr_lut(cac2_comp_t *p_comp,
  float trigger, chromatix_RNR1_type *chromatix_rnr)
{
  float sigma_trigger_start[MAX_SIGMA_TABLES - 1];
  float sigma_trigger_end[MAX_SIGMA_TABLES - 1];
  int sigmaRegionStart = 0, sigmaRegionEnd = 0;
  float sigma_interp_ratio = 0;
  uint32_t i = 0;

  if (!p_comp || !chromatix_rnr) {
    IDBG_ERROR("%s %d: Invalid input, p_comp=%p, chromatix_rnr=%p",
      __func__, __LINE__, p_comp, chromatix_rnr);
    return IMG_ERR_INVALID_INPUT;
  }

  for (i = 0; i < IMGLIB_ARRAY_SIZE(chromatix_rnr->sigma_lut_trigger); i++) {
    if (chromatix_rnr->control_rnr  == 0 ) { //lux index
      sigma_trigger_start[i] =
        chromatix_rnr->sigma_lut_trigger[i].lux_index_start;
      sigma_trigger_end[i] = chromatix_rnr->sigma_lut_trigger[i].lux_index_end;
    } else {
      sigma_trigger_start[i] =
        chromatix_rnr->rnr_data[i].rnr_trigger.gain_start;
      sigma_trigger_end[i] = chromatix_rnr->rnr_data[i].rnr_trigger.gain_end;
    }
  }

  sigmaRegionStart = IMGLIB_ARRAY_SIZE(sigma_trigger_start);
  sigmaRegionEnd = IMGLIB_ARRAY_SIZE(sigma_trigger_start);

  for (i = 0; i < IMGLIB_ARRAY_SIZE(sigma_trigger_start); i++) {
    if ((sigma_trigger_start[i] >= sigma_trigger_end[i])) {
      IDBG_ERROR("%s %d: sigma trigger start %f is <= to sigma trigger end %f",
        __func__, __LINE__, sigma_trigger_start[i], sigma_trigger_end[i]);
      return IMG_ERR_INVALID_INPUT;
    }

    if (trigger <= sigma_trigger_start[i]) {
      sigmaRegionStart = i;
      sigmaRegionEnd = i + 1;
      sigma_interp_ratio = 0.0;
      break;
    } else if (trigger < sigma_trigger_end[i] ) {
      sigmaRegionStart = i;
      sigmaRegionEnd = i + 1;
      sigma_interp_ratio = (float)(trigger - sigma_trigger_start[i]) /
        (float)(sigma_trigger_end[i] - sigma_trigger_start[i]);
      break;
    }
  }
  if (sigma_interp_ratio < 1.0) {
    IDBG_MED("%s %d: Sigma Interpolation ratio %f < 1.0", __func__, __LINE__,
      sigma_interp_ratio);
  }
  IDBG_MED("%s %d: sigmaRegionStart %d, sigmaRegionEnd %d", __func__, __LINE__,
    sigmaRegionStart, sigmaRegionEnd);

  if (sigmaRegionStart != sigmaRegionEnd) {
    for (i = 0; i < (uint32_t)chromatix_rnr->lut_size; i++) {
      p_comp->rnr_chromatix_info.sigma_lut[i] =
        BILINEAR_INTERPOLATION (
          chromatix_rnr->sigma_lut[sigmaRegionStart][i],
          chromatix_rnr->sigma_lut[sigmaRegionEnd][i],
          sigma_interp_ratio);
    }
  } else {
    for (i = 0; i < (uint32_t)chromatix_rnr->lut_size; i++) {
      p_comp->rnr_chromatix_info.sigma_lut[i] =
        chromatix_rnr->sigma_lut[sigmaRegionStart][i];
    }
  }

#ifdef DEBUG_SIGMA_TABLES
  for (i = 0; i < (uint32_t)chromatix_rnr->lut_size; i++) {
    IDBG_MED("%s:sigma_lut[%d]: %f", __func__, i,
      p_comp->rnr_chromatix_info.sigma_lut[i]);
  }
#endif

  return IMG_SUCCESS;
}
#endif

/**
 * Function: cac2_comp_get_rnr_params
 *
 * Description: This function is to update the RNR parameters
 * for RNR from chromatix header
 *
 * Arguments:
 *   @p_comp: cac component
 *   @p_frame: Input image frame
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int cac2_comp_get_rnr_params(cac2_comp_t *p_comp, img_frame_t *p_frame)
{
  uint32_t i = 0;

#ifdef USE_CHROMATIX
  chromatix_sw_postproc_type *chromatix = NULL;
  chromatix_RNR1_type *chromatix_rnr = NULL;
  int regionStart = 0, regionEnd = 0;
  float  trigger, interp_ratio = 0;
  float trigger_start[MAX_LIGHT_TYPES_FOR_SPATIAL];
  float trigger_end[MAX_LIGHT_TYPES_FOR_SPATIAL];

  chromatix = p_comp->chromatix_ptr;
  if (NULL == chromatix) {
    IDBG_ERROR("%s %d: Chromatix is null", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  //Set RnR params
  chromatix_rnr = &(chromatix->chromatix_radial_noise1_reduction);

  //Check if RNR enabled or disabled
  p_comp->rnr_enable_flag = chromatix_rnr->rnr_enable;
  trigger = (chromatix_rnr->control_rnr == 0) ?
    (float)p_comp->info_3a.lux_idx : (float)p_comp->info_3a.gain;

  IDBG_HIGH("%s: trigger = %f, rnr_enable_flag = %d\n", __func__, trigger,
    p_comp->rnr_enable_flag);

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    if (chromatix_rnr->control_rnr  == 0 ) { //lux index
      trigger_start[i] = chromatix_rnr->rnr_data[i].rnr_trigger.lux_index_start;
      trigger_end[i] = chromatix_rnr->rnr_data[i].rnr_trigger.lux_index_end;
    } else {
      trigger_start[i] = chromatix_rnr->rnr_data[i].rnr_trigger.gain_start;
      trigger_end[i] = chromatix_rnr->rnr_data[i].rnr_trigger.gain_end;
    }
  }

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    if ((trigger_start[i] >= trigger_end[i])) {
      IDBG_ERROR("%s %d: trigger start %f is <= to trigger end %f", __func__,
        __LINE__, trigger_start[i], trigger_end[i]);
      return IMG_ERR_INVALID_INPUT;
    }
    if (i == (MAX_LIGHT_TYPES_FOR_SPATIAL - 1)) {
      regionStart = MAX_LIGHT_TYPES_FOR_SPATIAL - 1;
      regionEnd = MAX_LIGHT_TYPES_FOR_SPATIAL - 1;
      break;
    }
    if (trigger <= trigger_start[i]) {
      regionStart = i;
      regionEnd = i;
      interp_ratio = 0.0;
      break;
    } else if (trigger < trigger_end[i] ) {
      regionStart = i;
      regionEnd = i + 1;
      interp_ratio = (float)(trigger - trigger_start[i]) /
        (float)(trigger_end[i] - trigger_start[i]);
      break;
    }
  }
  if (interp_ratio < 1.0) {
    IDBG_MED("%s %d: Interpolation ratio %f < 1.0", __func__, __LINE__,
      interp_ratio);
  }
  IDBG_MED("%s %d: regionStart %d, regionEnd %d", __func__, __LINE__,
    regionStart, regionEnd);

  p_comp->rnr_chromatix_info.lut_size = chromatix_rnr->lut_size;

  //Interpolate the LUT based on 3 trigger regions
  cac2_comp_get_rnr_lut(p_comp, trigger, chromatix_rnr);

  p_comp->rnr_chromatix_info.scale_factor =
    cac2_comp_get_rnr_scaling_factor(p_comp, p_frame);
  IDBG_MED("%s :scaling factor %f", __func__,
    p_comp->rnr_chromatix_info.scale_factor);

  if (chromatix_rnr->rnr_data[regionStart].center_noise_weight != 0) {
    p_comp->rnr_chromatix_info.center_noise_weight =
      chromatix_rnr->rnr_data[regionStart].center_noise_weight;
  } else {
    p_comp->rnr_chromatix_info.center_noise_weight = 1.0f;
  }
  if (regionStart != regionEnd) {
    p_comp->rnr_chromatix_info.center_noise_sigma =
      BILINEAR_INTERPOLATION (
        chromatix_rnr->rnr_data[regionStart].center_noise_sigma,
        chromatix_rnr->rnr_data[regionEnd].center_noise_sigma, interp_ratio);
  } else {
    p_comp->rnr_chromatix_info.center_noise_sigma =
      chromatix_rnr->rnr_data[regionStart].center_noise_sigma;
  }
  IDBG_MED("%s: center_noise_sigma = %f\n", __func__,
    p_comp->rnr_chromatix_info.center_noise_sigma);

#ifdef USE_RNR_HYSTERISIS
   //Hysterisis only in burst mode
  if((p_comp->rnr_hysterisis_info.prev_lux_value != 0) ||
      (p_comp->rnr_hysterisis_info.prev_gain_value != 0)) {
    cac2_comp_get_sampling_factor(p_comp, chromatix_rnr, trigger);
    //If hysterisis fails, get sampling factor from chromatix.
    if(p_comp->rnr_chromatix_info.sampling_factor == 0) {
      p_comp->rnr_chromatix_info.sampling_factor =
        chromatix_rnr->rnr_data[regionStart].sampling_factor;
    }
  } else {
    p_comp->rnr_chromatix_info.sampling_factor =
      chromatix_rnr->rnr_data[regionStart].sampling_factor;
  }
  //Save current gain, lux and sampling factor  values as prev values
  p_comp->rnr_hysterisis_info.prev_lux_value = p_comp->info_3a.lux_idx;
  p_comp->rnr_hysterisis_info.prev_gain_value = p_comp->info_3a.gain;
  p_comp->rnr_hysterisis_info.prev_sampling_factor =
    p_comp->rnr_chromatix_info.sampling_factor;
#else
  IDBG_MED("%s:%d: Hysterisis not enabled", __func__, __LINE__);
  p_comp->rnr_chromatix_info.sampling_factor =
    chromatix_rnr->rnr_data[regionStart].sampling_factor;
#endif
  p_comp->rnr_chromatix_info.weight_order =
    chromatix_rnr->rnr_data[regionStart].weight_order;
#else

  IMG_UNUSED(p_frame);

  p_comp->rnr_enable_flag = TRUE;
  p_comp->rnr_chromatix_info.sampling_factor = 2;
  p_comp->rnr_chromatix_info.scale_factor = 16.0f;

  for(i = 0; i < RNR_LUT_SIZE; i++) {
    p_comp->rnr_chromatix_info.sigma_lut[i] = sigma_lut_in[i];
  }
  p_comp->rnr_chromatix_info.lut_size = RNR_LUT_SIZE;
  p_comp->rnr_chromatix_info.center_noise_sigma = 2.0f;
  p_comp->rnr_chromatix_info.center_noise_weight = 1.0f;
  p_comp->rnr_chromatix_info.weight_order = 2.0f;
#endif
  return IMG_SUCCESS;
}

/**
 * Function: module_cac2_config_get_cac_params
 *
 * Description: This function is to update the CAC parameters
 * for cac v2 from chromatix header
 *
 * Arguments:
 *   @p_comp: cac component
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int cac2_comp_get_cac_params(cac2_comp_t *p_comp)
{
#ifdef USE_CHROMATIX
  chromatix_sw_postproc_type *chromatix = NULL;
  chromatix_CAC2_type *chromatix_cac2 = NULL;
  int regionStart = 0, regionEnd = 0, i = 0;
  float  trigger, interp_ratio = 0;
  float trigger_start[MAX_LIGHT_TYPES_FOR_SPATIAL];
  float trigger_end[MAX_LIGHT_TYPES_FOR_SPATIAL];

  chromatix = p_comp->chromatix_ptr;
  if (NULL == chromatix) {
    IDBG_ERROR("%s %d: Chromatix is null", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  chromatix_cac2 = &(chromatix->chromatix_CAC2_data);
  trigger = (chromatix_cac2->control_cac2 == 0) ?
    (float)p_comp->info_3a.lux_idx :(float)p_comp->info_3a.gain;
  IDBG_MED("%s:lux_idx %f, gain %f chromatix_cac2->control_cac2 %d, trigger %f",
    __func__, p_comp->info_3a.lux_idx, p_comp->info_3a.gain,
    chromatix_cac2->control_cac2, trigger);

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    if (chromatix_cac2->control_cac2 == 0) { //lux index
      trigger_start[i] =
        chromatix_cac2->cac2_data[i].cac2_trigger.lux_index_start;
      trigger_end[i] = chromatix_cac2->cac2_data[i].cac2_trigger.lux_index_end;
    } else {
      trigger_start[i] = chromatix_cac2->cac2_data[i].cac2_trigger.gain_start;
      trigger_end[i] = chromatix_cac2->cac2_data[i].cac2_trigger.gain_end;
    }
  }

  for(i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++ ) {
    IDBG_MED("%s:triger_start[%d] = %f, trigger_end[%d] = %f", __func__,
      i, trigger_start[i], i, trigger_end[i]);
  }
  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++ ) {
    if ((trigger_start[i] >= trigger_end[i])) {
      IDBG_ERROR("%s %d: strigger start %f is <= to trigger end %f", __func__,
        __LINE__, trigger_start[i], trigger_end[i]);
      return IMG_ERR_INVALID_INPUT;
    }
    if (i == (MAX_LIGHT_TYPES_FOR_SPATIAL - 1)) {
      regionStart = MAX_LIGHT_TYPES_FOR_SPATIAL - 1;
      regionEnd = MAX_LIGHT_TYPES_FOR_SPATIAL - 1;
      break;
    }
    if (trigger <= trigger_start[i]) {
      regionStart = i;
      regionEnd = i;
      interp_ratio = 0.0;
      break;
    } else if (trigger < trigger_end[i] ) {
      regionStart = i;
      regionEnd = i + 1;
      interp_ratio = (float) (trigger - trigger_start[i]) /
        (float)(trigger_end[i] - trigger_start[i]);
      break;
    }
  }
  p_comp->cac2_enable_flag = chromatix_cac2->cac2_data[regionStart].cac2_enable;

  if (interp_ratio < 1.0) {
    IDBG_MED("%s %d: Interpolation ratio %f < 1.0", __func__, __LINE__,
      interp_ratio);
  }
  IDBG_HIGH("%s:%d] region start %d end %d", __func__, __LINE__,
    regionStart, regionEnd);
  if (regionStart != regionEnd) {
    p_comp->cac_chromatix_info.detection_th1 =
      Round(BILINEAR_INTERPOLATION (
        chromatix_cac2->cac2_data[regionStart].y_spot_thr_low,
        chromatix_cac2->cac2_data[regionEnd].y_spot_thr_low, interp_ratio));
    p_comp->cac_chromatix_info.detection_th2 =
      Round(BILINEAR_INTERPOLATION (
        chromatix_cac2->cac2_data[regionStart].y_spot_thr_up,
        chromatix_cac2->cac2_data[regionEnd].y_spot_thr_up, interp_ratio));
    p_comp->cac_chromatix_info.detection_th3 =
      Round(BILINEAR_INTERPOLATION (
        chromatix_cac2->cac2_data[regionStart].y_saturation_thr,
        chromatix_cac2->cac2_data[regionEnd].y_saturation_thr, interp_ratio));
    p_comp->cac_chromatix_info.verification_th1 =
      Round(BILINEAR_INTERPOLATION(
        chromatix_cac2->cac2_data[regionStart].c_saturation_thr,
        chromatix_cac2->cac2_data[regionEnd].c_saturation_thr, interp_ratio));
    p_comp->cac_chromatix_info.correction_strength =
      Round(BILINEAR_INTERPOLATION(
        chromatix_cac2->cac2_data[regionStart].y_spot_thr_weight,
        chromatix_cac2->cac2_data[regionEnd].y_spot_thr_weight,
        interp_ratio));
  } else {
    p_comp->cac_chromatix_info.detection_th1 =
      chromatix_cac2->cac2_data[regionStart].y_spot_thr_low;
    p_comp->cac_chromatix_info.detection_th2 =
      chromatix_cac2->cac2_data[regionStart].y_spot_thr_up;
    p_comp->cac_chromatix_info.detection_th3 =
      chromatix_cac2->cac2_data[regionStart].y_saturation_thr;
    p_comp->cac_chromatix_info.verification_th1 =
      chromatix_cac2->cac2_data[regionStart].c_saturation_thr;
    p_comp->cac_chromatix_info.correction_strength =
      chromatix_cac2->cac2_data[regionStart].y_spot_thr_weight;
  }
#else
  p_comp->cac_chromatix_info.detection_th1 = 15;
  p_comp->cac_chromatix_info.detection_th2 = 30;
  p_comp->cac_chromatix_info.detection_th3 = 240;
  p_comp->cac_chromatix_info.verification_th1 = 10;
  p_comp->cac_chromatix_info.correction_strength = 32;

#endif

  return IMG_SUCCESS;
}

/**
 * Function: cac2_comp_get_config
 *
 * Description: Get the CAC and RNR config
 *
 * Input parameters:
 *   p_comp - The pointer to the component handle.
 *   p_frame - Frame which needs to be processed
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/

int cac2_comp_get_config(cac2_comp_t *p_comp, img_frame_t *p_frame)
{
  int rc = IMG_SUCCESS;

  rc = cac2_comp_get_cac_params(p_comp);
  if(rc != IMG_SUCCESS) {
    IDBG_ERROR("%s %d: Error configureing CAC params, disabling CAC",
      __func__, __LINE__);
    p_comp->cac2_enable_flag = FALSE;
  }

  /*If CDS is enabled, it is low light. Disable CAC*/
  if (p_comp->cac2_enable_flag && p_comp->cds_enabled) {
    p_comp->cac2_enable_flag = FALSE;
  }

  rc = cac2_comp_get_rnr_params(p_comp, p_frame);
  if(rc != IMG_SUCCESS) {
    IDBG_ERROR("%s %d: Error configureing RNR params, disabling RNR",
      __func__, __LINE__);
    p_comp->rnr_enable_flag = FALSE;
  }

  return rc;
}

/**
 * Function: cac2_comp_apply_cds
 *
 * Description: Run the CDS algorithm on the given frame
 *
 * Input parameters:
 *   p_comp - The pointer to the component handle.
 *   p_frame - Frame which needs to be processed
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int cac2_comp_apply_cds(cac2_comp_t *p_comp, img_frame_t *p_frame)
{
  uint8_t *p_wb_addr = g_cac_lib.workbuf.vaddr;
  img_plane_t *p_plane = &p_frame->frame[0].plane[1];
  bool use_workbuf = (p_wb_addr != NULL);
  uint32_t size = p_plane->stride * (IMG_HEIGHT(p_frame) >> 1);
  IDBG_HIGH("%s:%d] Apply CDS %dx%d pad %dx%d cds_proc_type %d"
    "use_workbuf %d",
    __func__, __LINE__,
    p_plane->width, p_plane->height,
    p_plane->stride, p_plane->scanline,
    p_comp->cds_proc_type,
    use_workbuf);
#ifdef DEBUG_CDS
  img_dump_frame(p_frame, "swcds_in.yuv", 1, NULL);
#endif
  img_sw_cds(p_plane->addr,
    IMG_WIDTH(p_frame),
    IMG_HEIGHT(p_frame) >> 1,
    p_plane->stride,
    use_workbuf ? p_wb_addr : p_plane->addr,
    p_frame->frame[0].plane[1].stride,
    p_comp->cds_proc_type);
  /* copy databack */
  if (use_workbuf) {
    memcpy(p_plane->addr, p_wb_addr, size);
  }
#ifdef DEBUG_CDS
  img_dump_frame(p_frame, "swcds_out.yuv", 1, NULL);
#endif
  return IMG_SUCCESS;
}

/**
 * Function: cac2_comp_process_frame
 *
 * Description: Run the denoise algorithm on the given frame
 *
 * Input parameters:
 *   p_comp - The pointer to the component handle.
 *   p_frame - Frame which needs to be processed
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int cac2_comp_process_frame(cac2_comp_t *p_comp, img_frame_t *p_frame)
{
  int rc = IMG_SUCCESS;
  img_component_t *p_base = &p_comp->b;
  cac2_args_t cac_rnr_params;

  memset(&cac_rnr_params, 0x0, sizeof(cac2_args_t));

  pthread_mutex_lock(&p_base->mutex);

  cac2_comp_get_config(p_comp, p_frame);

  /* apply CDS if needed */
  IDBG_HIGH("%s:%d] CDS enabled %d Process CDS %d",
    __func__, __LINE__,
    p_comp->cds_enabled,
    p_comp->process_cds);
  if (p_comp->cds_enabled && p_comp->process_cds) {
    cac2_comp_apply_cds(p_comp, p_frame);
  }
  //Fill in the Frame info
  cac_rnr_params.p_y = p_frame->frame[0].plane[IY].addr;
  cac_rnr_params.fd = p_frame->frame[0].plane[IY].fd;
  cac_rnr_params.p_crcb = p_frame->frame[0].plane[IC].addr;
  cac_rnr_params.image_width = p_frame->info.width;
  cac_rnr_params.image_height = p_frame->info.height;
  cac_rnr_params.y_stride = p_frame->frame[0].plane[IY].stride;
  cac_rnr_params.cbcr_stride = p_frame->frame[0].plane[IC].stride;
  cac_rnr_params.image_format = CAC_CHROMA_ORDER_CRCB;
  cac_rnr_params.ion_heap_id = ION_HEAP_ID;

  //Fill in the CAC parameters

  if (p_comp->cac2_enable_flag ) {
    cac_rnr_params.detection_th1 =
      p_comp->cac_chromatix_info.detection_th1;
    cac_rnr_params.detection_th2 =
      p_comp->cac_chromatix_info.detection_th2;
    cac_rnr_params.detection_th3 = p_comp->cac_chromatix_info.detection_th3;
    cac_rnr_params.verification_th1 =
      p_comp->cac_chromatix_info.verification_th1;
    cac_rnr_params.correction_strength =
      p_comp->cac_chromatix_info.correction_strength;
  }
  cac_rnr_params.cac2_enable_flag = p_comp->cac2_enable_flag;

  //Fill in the RNR parameters
  if (p_comp->rnr_enable_flag) {
    cac_rnr_params.sampling_factor = p_comp->rnr_chromatix_info.sampling_factor;
    cac_rnr_params.sigma_lut = p_comp->rnr_chromatix_info.sigma_lut;
    cac_rnr_params.lut_size = p_comp->rnr_chromatix_info.lut_size;
    cac_rnr_params.scale_factor = p_comp->rnr_chromatix_info.scale_factor;
    cac_rnr_params.center_noise_sigma =
      p_comp->rnr_chromatix_info.center_noise_sigma;
    cac_rnr_params.center_noise_weight =
      p_comp->rnr_chromatix_info.center_noise_weight;
    cac_rnr_params.weight_order = p_comp->rnr_chromatix_info.weight_order;
  }
  cac_rnr_params.rnr_enable_flag = p_comp->rnr_enable_flag;;
  pthread_mutex_unlock(&p_base->mutex);

  //Print Params - To debug
  cac2_comp_lib_debug(&cac_rnr_params);
  IDBG_INFO("%s:%d] Start CAC ", __func__, __LINE__);

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_CAC);
  rc = g_cac_lib.cac2_process(&cac_rnr_params);
  ATRACE_CAMSCOPE_END(CAMSCOPE_CAC);

  pthread_mutex_lock(&p_base->mutex);
  p_comp->b.state = IMG_STATE_IDLE;
  pthread_mutex_unlock(&p_base->mutex);

  if (rc) {
    IDBG_ERROR("%s:%d] CAC failed", __func__, __LINE__);
  } else {
    IDBG_INFO("%s:%d] CAC Successfull", __func__, __LINE__);
  }

  goto Error;

Error:
  return rc;
}


/**
 * Function: cac2_thread_loop
 *
 * Description: Main algorithm thread loop
 *
 * Input parameters:
 *   data - The pointer to the component object
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
void *cac2_thread_loop(void *handle)
{
  cac2_comp_t *p_comp = (cac2_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame = NULL;
  int i = 0, count;
  IDBG_MED("%s:%d] ", __func__, __LINE__);

  count = img_q_count(&p_base->inputQ);
  IDBG_MED("%s:%d] num buffers %d", __func__, __LINE__, count);

  for (i = 0; i < count; i++) {
    p_frame = img_q_dequeue(&p_base->inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
      goto error;
    }
    /*process the frame*/
    status = cac2_comp_process_frame(p_comp, p_frame);
    if (status < 0) {
      IDBG_ERROR("%s:%d] process error %d", __func__, __LINE__, status);
      goto error;
    }

    /*enque the frame to the output queue*/
    status = img_q_enqueue(&p_base->outputQ, p_frame);
    if (status < 0) {
      IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
      goto error;
    }
    IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
  }

  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_STOPPED;
  pthread_mutex_unlock(&p_base->mutex);
  IMG_SEND_EVENT(p_base, QIMG_EVT_DONE);
  return IMG_SUCCESS;

error:
    /* flush rest of the buffers */
  count = img_q_count(&p_base->inputQ);
  IDBG_MED("%s:%d] Error buf count %d", __func__, __LINE__, count);

  for (i = 0; i < count; i++) {
    p_frame = img_q_dequeue(&p_base->inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
      continue;
    }
    status = img_q_enqueue(&p_base->outputQ, p_frame);
    if (status < 0) {
      IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
      continue;
    }
    IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
  }
  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_STOPPED;
  pthread_mutex_unlock(&p_base->mutex);
  IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_ERROR, status, status);
  return NULL;

}


/**
 * Function: cac2_comp_abort
 *
 * Description: Aborts the execution of CAC
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type is defined in cac.h
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int cac2_comp_abort(void *handle, void *p_data)
{
  cac2_comp_t *p_comp = (cac2_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status;

  if (p_base->mode == IMG_ASYNC_MODE) {
    status = p_comp->b.ops.abort(&p_comp->b, p_data);
    if (status < 0)
      return status;
  }
  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_INIT;
  pthread_mutex_unlock(&p_base->mutex);

  return 0;
}

/**
 * Function: cac2_comp_process
 *
 * Description: This function is used to send any specific commands for the
 *              CAC component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   cmd - The command type which needs to be processed
 *   p_data - The pointer to the command payload
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int cac2_comp_process(void *handle, img_cmd_type cmd, void *p_data)
{
  cac2_comp_t *p_comp = (cac2_comp_t *)handle;
  int status;

  status = p_comp->b.ops.process(&p_comp->b, cmd, p_data);
  if (status < 0)
    return status;

  return 0;
}

/**
 * Function: cac2_comp_start
 *
 * Description: Start the execution of CAC
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type will be defined in cac.h
 *   sync_mode: Indicates if the componnt is executed in
 *   syncronous mode.In this mode it will be executed in the
 *   calling threads context and there will be no callback.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int cac2_comp_start(void *handle, void *p_data)
{
  cac2_comp_t *p_comp = (cac2_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame;

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT) ||
    (NULL == p_base->thread_loop)) {
    IDBG_ERROR("%s:%d] Error state %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_NOT_SUPPORTED;
  }

  p_base->state = IMG_STATE_STARTED;
  pthread_mutex_unlock(&p_base->mutex);

  if (p_base->mode == IMG_SYNC_MODE) {
    p_frame = img_q_dequeue(&p_base->inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
      status = IMG_ERR_INVALID_INPUT;
      goto error;
    }
    status = cac2_comp_process_frame(p_comp, p_frame);
  } else {
    status = p_comp->b.ops.start(&p_comp->b, p_data);
  }

error:
  if (status != IMG_SUCCESS) {
    pthread_mutex_lock(&p_base->mutex);
    p_base->state = IMG_STATE_INIT;
    pthread_mutex_unlock(&p_base->mutex);
  }

  return status;
}


/**
 * Function: cac2_comp_get_param
 *
 * Description: Gets CAC parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int cac2_comp_get_param(void *handle, img_param_type param, void *p_data)
{
  cac2_comp_t *p_comp = (cac2_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);
  if (status < 0)
    return status;

  switch (param) {
  default:
    IDBG_ERROR("%s:%d] Error param=%d", __func__, __LINE__, param);
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: cac2_comp_set_param
 *
 * Description: Set CAC parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in cac.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int cac2_comp_set_param(void *handle, img_param_type param, void *p_data)
{
  cac2_comp_t *p_comp = (cac2_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);
  if (status < 0)
    return status;

  IDBG_MED("%s:%d] param 0x%x", __func__, __LINE__, param);
  switch (param) {
  case QCAC_CHROMATIX_INFO : {
    void *l_chromatix = (void *)p_data;
    if (NULL == l_chromatix) {
      IDBG_ERROR("%s:%d] invalid chromatix info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->chromatix_ptr = l_chromatix;
  }
  break;

  case QCDS_PROCESS: {
    uint8_t *l_cds_process = (uint8_t *)p_data;
    if (NULL == l_cds_process) {
      IDBG_ERROR("%s:%d] invalid cac enabled info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->process_cds = *l_cds_process;
  }
  break;

  case QCDS_ENABLED: {
    uint8_t *l_cds_enabled = (uint8_t *)p_data;
    if (NULL == l_cds_enabled) {
      IDBG_ERROR("%s:%d] invalid cac enabled info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->cds_enabled = *l_cds_enabled;
  }
  break;

  case QCAC_3A_INFO: {
    cac_3a_info_t *l_3aInfo = (cac_3a_info_t *)p_data;
    if (NULL == l_3aInfo) {
      IDBG_ERROR("%s:%d] invalid 3A Info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->info_3a = *l_3aInfo;
  }

  case QIMG_PARAM_MODE:
  case QCAC_CHROMA_ORDER:
  case QLDS_ENABLED:
  case QFD_RESULT:
  case QYUYV422_TO_420_ENABLED:
    break;

  default:
    break;
  }
  return status;
}

/**
 * Function: cac2_comp_deinit
 *
 * Description: Un-initializes the CAC component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int cac2_comp_deinit(void *handle)
{
  cac2_comp_t *p_comp = (cac2_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] \n", __func__, __LINE__);
  status = cac2_comp_abort(handle, NULL);
  if (status < 0)
    return status;

  status = p_comp->b.ops.deinit(&p_comp->b);
  if (status < 0)
    return status;

  free(p_comp);
  return IMG_SUCCESS;
}

/**
 * Function: cac2_comp_create
 *
 * Description: This function is used to create Qualcomm CAC
 *              denoise component
 *
 * Input parameters:
 *   @handle: library handle
 *   @p_ops - The pointer to img_component_t object. This object
 *            contains the handle and the function pointers for
 *            communicating with the imaging component.
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int cac2_comp_create(void* handle, img_component_ops_t *p_ops)
{
  IMG_UNUSED(handle);

  cac2_comp_t *p_comp = NULL;
  int status;

  if (NULL == g_cac_lib.ptr) {
    IDBG_ERROR("%s:%d] library not loaded", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (NULL == p_ops) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_comp = (cac2_comp_t *)malloc(sizeof(cac2_comp_t));
  if (NULL == p_comp) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  memset(p_comp, 0x0, sizeof(cac2_comp_t));
  status = img_comp_create(&p_comp->b);
  if (status < 0)
  {
    free(p_comp);
    return status;
  }

  /*set the main thread*/
  p_comp->b.thread_loop = cac2_thread_loop;
  p_comp->b.p_core = p_comp;
  p_comp->cds_proc_type = g_cac_lib.cds_proc_type;

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init            = cac2_comp_init;
  p_ops->deinit          = cac2_comp_deinit;
  p_ops->set_parm        = cac2_comp_set_param;
  p_ops->get_parm        = cac2_comp_get_param;
  p_ops->start           = cac2_comp_start;
  p_ops->abort           = cac2_comp_abort;
  p_ops->process         = cac2_comp_process;


  p_ops->handle = (void *)p_comp;
  return IMG_SUCCESS;
}

/**
 * Function: cac2_comp_load
 *
 * Description: This function is used to load Qualcomm CAC
 * library
 *
 * Input parameters:
 *   @name: library name
 *   @handle: library handle
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NOT_FOUND
 *
 * Notes: none
 **/
int cac2_comp_load(const char* name, void** handle)
{
  IMG_UNUSED(name);
  IMG_UNUSED(handle);

  int rc = IMG_SUCCESS;
  if (g_cac_lib.ptr) {
    IDBG_ERROR("%s:%d] library already loaded", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }

  g_cac_lib.ptr = dlopen("libmmcamera_cac2_lib.so", RTLD_NOW);
  if (!g_cac_lib.ptr) {
    IDBG_ERROR("%s:%d] Error opening CAC library", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_cac_lib.cac2_process) =
    dlsym(g_cac_lib.ptr, "cac2_process");
  if (!g_cac_lib.cac2_process) {
    IDBG_ERROR("%s:%d] Error linking camera CAC module start",
      __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_cac_lib.cac2_init) =
    dlsym(g_cac_lib.ptr, "cac2_init");
  if (!g_cac_lib.cac2_init) {
    IDBG_ERROR("%s:%d] Error linking cac_module_init",
    __func__, __LINE__);
  dlclose(g_cac_lib.ptr);
  g_cac_lib.ptr = NULL;
  return IMG_ERR_NOT_FOUND;
  }
  *(void **)&(g_cac_lib.cac2_deinit) =
    dlsym(g_cac_lib.ptr, "cac2_deinit");
  if (!g_cac_lib.cac2_deinit) {
    IDBG_ERROR("%s:%d] Error linking cac_module_deinit",
    __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_cac_lib.cac2_pre_allocate_buffers) =
    dlsym(g_cac_lib.ptr, "cac2_pre_allocate_buffers");
  if (!g_cac_lib.cac2_pre_allocate_buffers) {
    IDBG_ERROR("%s:%d] Error linking pre_allocate_buffers",
    __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }
  *(void **)&(g_cac_lib.cac2_destroy_buffers) =
    dlsym(g_cac_lib.ptr, "cac2_destroy_buffers");
  if (!g_cac_lib.cac2_destroy_buffers) {
    IDBG_ERROR("%s:%d] Error linking pre_allocate_buffers",
    __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }
  rc = g_cac_lib.cac2_init(ION_HEAP_ID);
  if (rc) {
    IDBG_ERROR("%s:%d] CAC lib module init failed", __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  /* set CDS proc type for debugging. default should be 1*/
#ifdef _ANDROID_
  char prop[PROPERTY_VALUE_MAX];
  property_get("persist.vendor.camera.imglib.cds", prop, "1");
  g_cac_lib.cds_proc_type = (uint32_t)atoi(prop);
  IDBG_HIGH("%s:%d] cds_proc_type %d", __func__, __LINE__,
    g_cac_lib.cds_proc_type);
#endif

  IDBG_HIGH("%s:%d] CAC library loaded successfully", __func__, __LINE__);

  return rc;
}

 /**
* Function: cac2_comp_alloc
*
* Description: This function is used to Allocate buffers
* library
*
* Input parameters:
 *   @handle: pointer to the core ops structure
 *   @p_params: pointer to the alloc params
*
* Return values:
*     Standard Imglb Error values
*
* Notes: none
**/
int cac2_comp_alloc(void *handle, void *p_params)
{
  int rc = IMG_SUCCESS;
  IMG_UNUSED(handle);
  IMG_UNUSED(p_params);
  IDBG_MED("%s:%d] ptr %p", __func__, __LINE__, g_cac_lib.ptr);

  if (g_cac_lib.ptr) {
    rc = g_cac_lib.cac2_pre_allocate_buffers(ION_HEAP_ID);
    if (rc) {
      IDBG_ERROR("%s:%d] CAC lib allocate buffers failed", __func__, __LINE__);
      dlclose(g_cac_lib.ptr);
      g_cac_lib.ptr = NULL;
      return IMG_ERR_NOT_FOUND;
    }

    g_cac_lib.workbuf.vaddr = NULL;
    switch (g_cac_lib.cds_proc_type) {
    case IMG_OPS_C:
    case IMG_OPS_NEON:
    case IMG_OPS_FCV:
      rc = img_buffer_get(IMG_BUFFER_HEAP,
        -1, 0, MAX_CDS_BUF_SIZE, &g_cac_lib.workbuf);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("%s:%d] Can not allocate heap memory", __func__, __LINE__);
      }
      break;
    default:;
    }
  }

  return rc;
}
/**
 * Function: cac2_comp_dealloc
 *
 * Description: This function is used to DeAllocate buffers
 * library
 *
 * Input parameters:
 *   @handle: pointer to the handle
 *
 * Return values:
 *    Standard Imglb Error values
 *
 * Notes: none
 **/
int cac2_comp_dealloc(void *handle)
{
  int rc = IMG_SUCCESS;
  IMG_UNUSED(handle);
  IDBG_MED("%s:%d] ptr %p", __func__, __LINE__, g_cac_lib.ptr);

  if (g_cac_lib.ptr) {
    if (g_cac_lib.workbuf.vaddr) {
      img_buffer_release(&g_cac_lib.workbuf);
    }
    rc = (int)g_cac_lib.cac2_destroy_buffers();
    if (rc) {
      IDBG_ERROR("%s:%d] CAC lib deallocate buffers failed", __func__, __LINE__);
      dlclose(g_cac_lib.ptr);
      g_cac_lib.ptr = NULL;
      return IMG_ERR_NOT_FOUND;
   }
  }
  return rc;
}

/**
 * Function: cac_comp_unload
 *
 * Description: This function is used to unload Qualcomm CAC
 * library
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void cac2_comp_unload(void* handle)
{
  IMG_UNUSED(handle);

  int rc = 0;
  IDBG_HIGH("%s:%d] ptr %p", __func__, __LINE__, g_cac_lib.ptr);

  if (g_cac_lib.ptr) {
    rc = g_cac_lib.cac2_deinit();
    if (rc) {
      IDBG_ERROR("%s:%d] CAC lib module deinit failed", __func__, __LINE__);
    }
    rc = dlclose(g_cac_lib.ptr);
    if (rc < 0)
      IDBG_HIGH("%s:%d] error %s", __func__, __LINE__, dlerror());
      g_cac_lib.ptr = NULL;
  }
}
