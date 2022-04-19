/* ltm44.c
 *
 * Copyright (c) 2014-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>
#include <cutils/properties.h>
#include "cam_types.h"
/* mctl headers */
#include "modules.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_LTM, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_LTM, fmt, ##args)

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "ltm44.h"
#include "chromatix.h"
#include "isp_pipeline_reg.h"
#include "ltm_curves.h"

/* define PRINT_REG_VAL_SET */
#define LTM_CURVE_DEBUG 0

#define LTM_IP_RGN_CNT 19
#define LTM_DC_RGN_CNT 20
const uint32_t ltm_dc_region[LTM_DC_RGN_CNT + 1] = {0, 13, 40, 67, 94, 121, 148,
      175, 202, 229, 256, 283, 310, 337, 364, 391, 418, 445, 472, 499, 509};

static void ltm44_dump_lut(char* tablename, uint32_t *lut, int32_t size);
boolean ltm44_update_8k_params(isp_sub_module_t *isp_sub_module,
  ltm44_t *mod);
boolean ltm44_adjust_curve_strength(ltm44_t *mod,
  isp_sub_module_t *isp_sub_module);

/** ltm44_debug:
 *    @pcmd: configuration command
 *
 *  This function dumps ltm configuration
 *
 *  Return: None
 **/
static void ltm44_debug(isp_ltm_reg_t* pcmd)
{
  ISP_DBG("hMnEnable %d", pcmd->ltm_cfg.hMnEnable);
  ISP_DBG("vMnEnable %d", pcmd->ltm_cfg.vMnEnable);
  ISP_DBG("ipEnable %d", pcmd->ltm_cfg.ipEnable);
  ISP_DBG("dcEnable %d", pcmd->ltm_cfg.dcEnable);
  ISP_DBG("autoSwitchOverride %d", pcmd->ltm_cfg.autoSwitchOverride);
  ISP_DBG("dc3dAvgPongSel %d", pcmd->ltm_cfg.dc3dAvgPongSel);
  ISP_DBG("dc3dSumClear %d", pcmd->ltm_cfg.dc3dSumClear);
  ISP_DBG("ip3dAvgPongSel %d", pcmd->ltm_cfg.ip3dAvgPongSel);
  ISP_DBG("lutBankSelect %d", pcmd->ltm_cfg.lutBankSelect);
  ISP_DBG("debugOutSelect %d", pcmd->ltm_cfg.debugOutSelect);
  ISP_DBG("dc3dAvgPong %d", pcmd->ltm_status.dc3dAvgPong);

  ISP_DBG("h_image_size.hIn %d ", pcmd->ltm_h_scale_cfg.h_image_size.hIn);
  ISP_DBG("h_image_size.hOut %d ", pcmd->ltm_h_scale_cfg.h_image_size.hOut);
  ISP_DBG("h_phase.hPhaseMult %d ", pcmd->ltm_h_scale_cfg.h_phase.hPhaseMult);
  ISP_DBG("h_phase.hInterpReso %d ", pcmd->ltm_h_scale_cfg.h_phase.hInterpReso);
  ISP_DBG("h_stripe.hMnInit %d ", pcmd->ltm_h_scale_cfg.h_stripe.hMnInit);
  ISP_DBG("h_stripe.hPhaseInit %d ", pcmd->ltm_h_scale_cfg.h_stripe.hPhaseInit);
  ISP_DBG("v_image_size.vIn %d ", pcmd->ltm_v_scale_cfg.v_image_size.vIn);
  ISP_DBG("v_image_size.vOut %d ", pcmd->ltm_v_scale_cfg.v_image_size.vOut);
  ISP_DBG("v_phase.vPhaseMult %d ", pcmd->ltm_v_scale_cfg.v_phase.vPhaseMult);
  ISP_DBG("v_phase.vInterpReso %d ", pcmd->ltm_v_scale_cfg.v_phase.vInterpReso);
  ISP_DBG("v_stripe.vMnInit %d ", pcmd->ltm_v_scale_cfg.v_stripe.vMnInit);
  ISP_DBG("v_stripe.vPhaseInit %d ", pcmd->ltm_v_scale_cfg.v_stripe.vPhaseInit);

  ISP_DBG("dc_cfg.initDX %d ", pcmd->ltm_dcip_cfg.dc_cfg.initDX);
  ISP_DBG("dc_cfg.initCellNumX %d ", DC_INIT_CELLNUM(pcmd));
  ISP_DBG("dc_cfg.binInitCnt %d ", pcmd->ltm_dcip_cfg.dc_cfg.binInitCnt);
  ISP_DBG("ip_init.initDX %d ", pcmd->ltm_dcip_cfg.ip_init.initDX);
  ISP_DBG("ip_init.initPX %d ", pcmd->ltm_dcip_cfg.ip_init.initPX);
  ISP_DBG("ip_cfg.initCellNumX %d ", IP_INIT_CELLNUM(pcmd));
  ISP_DBG("ip_inv_cellwidth.invCellwidthL %d ",
    pcmd->ltm_dcip_cfg.ip_inv_cellwidth.invCellwidthL);
  ISP_DBG("ip_inv_cellwidth.invCellwidthR %d ",
    pcmd->ltm_dcip_cfg.ip_inv_cellwidth.invCellwidthR);
  ISP_DBG("ip_inv_cellheight.invCellheightL %d ",
    pcmd->ltm_dcip_cfg.ip_inv_cellheight.invCellheightL);
  ISP_DBG("ip_inv_cellheight.invCellheightR %d ",
    pcmd->ltm_dcip_cfg.ip_inv_cellheight.invCellheightR);
  ISP_DBG("ip_cellwidth.cellwidthL %d ",
    pcmd->ltm_dcip_cfg.ip_cellwidth.cellwidthL);
  ISP_DBG("ip_cellwidth.cellwidthR %d ",
    pcmd->ltm_dcip_cfg.ip_cellwidth.cellwidthR);
  ISP_DBG("ip_cellheight.cellheightL %d ",
    pcmd->ltm_dcip_cfg.ip_cellheight.cellheightL);
  ISP_DBG("ip_cellheight.cellheightR %d ",
    pcmd->ltm_dcip_cfg.ip_cellheight.cellheightR);

  ISP_DBG("c1 %d ", pcmd->ltm_rgb2y_cfg.rgb2y_cfg0.c1);
  ISP_DBG("c2 %d ", pcmd->ltm_rgb2y_cfg.rgb2y_cfg0.c2);
  ISP_DBG("c3 %d ", pcmd->ltm_rgb2y_cfg.rgb2y_cfg0.c3);
  ISP_DBG("thr %d ", pcmd->ltm_rgb2y_cfg.rgb2y_cfg1.thr);
  ISP_DBG("k %d ", pcmd->ltm_rgb2y_cfg.rgb2y_cfg1.k);
  ISP_DBG("c4 %d ", pcmd->ltm_rgb2y_cfg.rgb2y_cfg1.c4);

  ISP_DBG("scaleYInWidth %d ", pcmd->ltm_scale_h_pad_cfg.scaleYInWidth);
  ISP_DBG("hSkipCnt %d ", pcmd->ltm_scale_h_pad_cfg.hSkipCnt);
  ISP_DBG("scaleYInHeight %d ", pcmd->ltm_scale_v_pad_cfg.scaleYInHeight);
  ISP_DBG("vSkipCnt %d ", pcmd->ltm_scale_v_pad_cfg.vSkipCnt);

} /* ltm44_debug */

/** ltm44_Print_tables:
 *    @ltm: configuration command
 *
 *  This function dumps ltm ltm tables
 *
 *  Return: None
 **/

static void ltm44_print_tables(ltm44_t *ltm)
{
  ISP_DBG("hw_master_curve");
  int32_t i=0;
  for(i=0; i < LTM_HW_LUT_SIZE; i = i+8) {
   ISP_DBG("i=%d,%x %x %x %x %x %x %x %x\n",
     i,ltm->hw_master_curve[i],ltm->hw_master_curve[i+1],
     ltm->hw_master_curve[i+2],ltm->hw_master_curve[i+3],
     ltm->hw_master_curve[i+4],ltm->hw_master_curve[i+5],
     ltm->hw_master_curve[i+6],ltm->hw_master_curve[i+7]);
  }
  ISP_DBG("hw_master_scale");
  for(i=0; i < LTM_HW_LUT_SIZE; i = i+8) {
    ISP_DBG("i=%d,%x %x %x %x %x %x %x %x\n",
      i,ltm->hw_master_scale[i],ltm->hw_master_scale[i+1],
      ltm->hw_master_scale[i+2],ltm->hw_master_scale[i+3],
      ltm->hw_master_scale[i+4],ltm->hw_master_scale[i+5],
      ltm->hw_master_scale[i+6],ltm->hw_master_scale[i+7]);
  }
  ISP_DBG("hw_shift_scale");
  for(i=0; i < LTM_HW_LUT_SIZE; i = i+8) {
    ISP_DBG("i=%d,%x %x %x %x %x %x %x %x\n",
      i,ltm->hw_shift_scale[i],ltm->hw_shift_scale[i+1],
      ltm->hw_shift_scale[i+2],ltm->hw_shift_scale[i+3],
      ltm->hw_shift_scale[i+4],ltm->hw_shift_scale[i+5],
      ltm->hw_shift_scale[i+6],ltm->hw_shift_scale[i+7]);
  }
  ISP_DBG("hw_shift_curve");
  for(i=0; i < LTM_HW_LUT_SIZE; i = i+8) {
    ISP_DBG("i=%d,%x %x %x %x %x %x %x %x\n",
      i,ltm->hw_shift_curve[i],ltm->hw_shift_curve[i+1],
      ltm->hw_shift_curve[i+2],ltm->hw_shift_curve[i+3],
      ltm->hw_shift_curve[i+4],ltm->hw_shift_curve[i+5],
      ltm->hw_shift_curve[i+6],ltm->hw_shift_curve[i+7]);
  }
}

/** ltm44_check_enable:
 *
 *  @isp_sub_module: isp_sub_module handle
 *  @mod: ltm module data
 *
 *  Check region based enable. If ltm is enabled in the region
 *  selected and enable or else disable ltm
 *
 * Return TRUE on LTM enable and FALSE on LTM disable
 **/
static boolean ltm44_check_enable(isp_sub_module_t *isp_sub_module, ltm44_t *mod)
{
  boolean  module_enable;
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(mod);

  if(((!isp_sub_module->chromatix_module_enable ||
       (mod->cur_flash_mode != CAM_FLASH_MODE_OFF)||
     isp_sub_module->manual_ctrls.tonemap_mode ==
     CAM_TONEMAP_MODE_CONTRAST_CURVE) &&
     mod->effects.spl_effect == CAM_EFFECT_MODE_OFF) ||
     (mod->isp_out_info.is_split == TRUE &&
      mod->exceed_split_limits == TRUE)) {
      module_enable = FALSE;
  } else if (isp_sub_module->chromatix_module_enable &&
    (!mod->normal_idx.ltm_region_enable &&
     !mod->backlit_idx.ltm_region_enable) &&
     mod->effects.spl_effect == CAM_EFFECT_MODE_OFF) {
      module_enable = FALSE;
  } else {
    module_enable = TRUE;
  }


  return module_enable;
}

/** adjust_signbit:
*
*  @val: input 32 bit signed value
*  @nm_bits: number of bits to adjust the sign bit
*
*  The function converts from 32 bit signed value to
*  a num_bits signed value and returns it
*
**/
static int32_t adjust_signbit(int32_t val, uint8_t num_bits) {
 if (val < 0) {
    val = -(val);
    val = (1 << num_bits) -val;
 }
 return val;
}

/** pack_tbl_to_lut_i32:
*
*  @src: input array to be packed
*  @dest: packed output array in delta|base format
*
* Packs the output in LUT_BASE_BITS|LUT_DELTA_BITS format
*
* Returns nothing
**/
static void pack_tbl_to_lut_i32(int *src, uint32_t *dest, float ratio)
{
  int i = 0;
  int32_t delta = 0;
  int32_t  curr_base = 0, next_base = 0;

  ISP_DBG("ratio %f", ratio);
  /* Convert to LUT (LUT_DELTA_BITS signed delta | LUT_BASE_BITS signed base) */
  for (i=0; i < LTM_HW_LUT_SIZE; i++) {
    curr_base = CLAMP_LTM(roundf(src[i]* ratio), -(1 << (LUT_BASE_BITS - 1)),
      ((1 << (LUT_BASE_BITS - 1)) - 1));
    next_base = CLAMP_LTM(roundf(src[i+1]* ratio), -(1 << (LUT_BASE_BITS - 1)),
      ((1 << (LUT_BASE_BITS - 1)) - 1));
    delta = (next_base - curr_base);
    delta = CLAMP_LTM(delta, -(1 << (LUT_DELTA_BITS - 1)),
      ((1 << (LUT_DELTA_BITS - 1)) - 1));
    curr_base =  adjust_signbit(curr_base, LUT_BASE_BITS);
    delta = adjust_signbit(delta, LUT_DELTA_BITS);
    dest[i] = (delta << LUT_BASE_BITS) | curr_base;
  }
}

/** pack_tbl_to_lut_i32_shift:
 *
 *  @src: input array to be packed
 *  @dest: packed output array in delta|base format
 *
 * Packs the output in LUT_BASE_BITS|LUT_DELTA_BITS format
 *
 * Returns nothing
 **/
void pack_tbl_to_lut_i32_shift(float *src, uint32_t *dest, float ratio)
{
  int i = 0;
  int32_t delta = 0;
  int32_t  curr_base = 0, next_base = 0;

  ISP_DBG("ratio %f", ratio);
/* Convert to LUT (LUT_DELTA_BITS signed delta | LUT_BASE_BITS signed base) */
  for (i=0; i < LTM_HW_LUT_SIZE; i++) {
    curr_base = CLAMP_LTM(roundf(src[i]* ratio), -(1 << (LUT_BASE_BITS - 1)),
      ((1 << (LUT_BASE_BITS - 1)) - 1));
    next_base = CLAMP_LTM(roundf(src[i+1]* ratio), -(1 << (LUT_BASE_BITS - 1)),
      ((1 << (LUT_BASE_BITS - 1)) - 1));
    delta = (next_base - curr_base);
    delta = CLAMP_LTM(delta, -(1 << (LUT_DELTA_BITS - 1)),
      ((1 << (LUT_DELTA_BITS - 1)) - 1));
    curr_base =  adjust_signbit(curr_base, LUT_BASE_BITS);
    delta = adjust_signbit(delta, LUT_DELTA_BITS);
    dest[i] = (delta << LUT_BASE_BITS) | curr_base;
  }
}


/** ltm44_upsample_table
 *    @temp_LAtable: input LA table
 *    @upsample_LAtable : Output LA table
 *    @ Src_size
 *    @ Dest_size
 *  This function upsample Src table to Dest table.
 *
 *  Return: None
 **/
void ltm44_upsample_table(float *temp_LAtable, float *upsample_LAtable,
   int32_t src_size, int32_t des_size)
{
  float init = 0.0f;
  int32_t i=0, j=0;
  int32_t interval = des_size/src_size;
  for (i =0,j=0 ; i < src_size; i++) {
    if (interval == 2) {
      if ( i== 0) {
        upsample_LAtable[j++] = temp_LAtable[i];
        upsample_LAtable[j++] = temp_LAtable[i];
        } else {
         upsample_LAtable[j++] = (temp_LAtable[i-1] + temp_LAtable[i])/2;
         upsample_LAtable[j++] = temp_LAtable[i];
        }
     } else if (interval == 1) {
      upsample_LAtable[i] = temp_LAtable[i];
     }
  }
}

/** ltm44_set_spl_effect:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function configures ltm module according special effect set.
 *
 *  Return: TRUE - Success
 *           -FALSE -- failure
 **/
boolean ltm44_prepare_effect_curves(isp_sub_module_t *isp_sub_module,
  ltm44_t *ltm, uint32_t type)
{
  uint32_t                   i = 0;
  chromatix_VFE_common_type *chrComPtr = NULL;
  chromatix_rolloff_type    *chromatix_rolloff = NULL;
  float                     *effect_table = NULL;
  float                     temp_LAtable[64];
  float                     upsample_LAtable[128];
  int32_t                   interval = 1;
  int32_t                   mast_y0, mast_y1;
  int16_t                   mast_base, mast_diff;
  //float                     ratio;

  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(ltm);
  chrComPtr = isp_sub_module->chromatix_ptrs.chromatixComPtr;
  RETURN_IF_NULL(chrComPtr);

  switch (type) {
  case CAM_EFFECT_MODE_SOLARIZE: {
    effect_table = &chrComPtr->chromatix_LA_special_effects.LA_LUT_solarize[0];
   }
   break;

  case CAM_EFFECT_MODE_POSTERIZE: {
    effect_table = &chrComPtr->chromatix_LA_special_effects.LA_LUT_posterize[0];
   }
   break;

  default:
    return TRUE;
  }

  interval = (uint32_t) isp_sub_module_util_power(2.0, LUT_BASE_BITS)/64;
  ISP_DBG("interval = %d", interval);

  for (i = 0; i < 64; i++) {
    temp_LAtable[i] = effect_table[i]*i*interval;
  }

  /*For 8994 only*/
  /*upsample temp_LAtable and interval *= 2 */
  interval = (uint32_t) isp_sub_module_util_power(2.0, LUT_BASE_BITS)/
    LTM_HW_LUT_SIZE;

  ISP_DBG("interval = %d", interval);

  ltm44_upsample_table(temp_LAtable, upsample_LAtable,
    LA_TBL_SIZE, LTM_HW_LUT_SIZE);

  for (i = 0; i < LTM_HW_LUT_SIZE; i ++) {
    ltm->hw_master_scale[i] = FLOAT_TO_Q(10,2.0f);// 2 << 10;
    ltm->hw_shift_scale[i] = 0;
    ltm->hw_shift_curve[i] = 0;
    ltm->hw_mask_curve[i] = (i*interval) - (1 << LUT_BASE_BITS - 1);//(1 << 13)
    upsample_LAtable[i] = (upsample_LAtable[i] - (i*interval))/2;
  }

  for (i = 0; i < LTM_HW_LUT_SIZE-1; i++) {
    // scale "base" based on lum variation
    /* weaken master curve */
    mast_y0 = (float)(upsample_LAtable[i] ) ;
    mast_y1 = (float)(upsample_LAtable[i+1] ) ;

    mast_y0 = CLAMP_LTM(mast_y0, -(1 << (LUT_BASE_BITS - 1)),
      ((1 << (LUT_BASE_BITS - 1)) - 1));
    mast_y1 = CLAMP_LTM(mast_y1, -(1 << (LUT_BASE_BITS - 1)),
      ((1 << (LUT_BASE_BITS - 1)) - 1));

    mast_diff = (mast_y1 - mast_y0);
    mast_base = mast_y0;

    mast_diff = CLAMP_LTM(mast_diff, -(1 << (LUT_DELTA_BITS - 1)),
      ((1 << (LUT_DELTA_BITS - 1)) - 1));

    mast_base = (int16_t)((uint16_t)mast_base << (16 - LUT_BASE_BITS))
      >> (16 - LUT_BASE_BITS);
    mast_diff = (int16_t)((uint16_t)mast_diff << (16 - LUT_DELTA_BITS))
      >> (16 - LUT_DELTA_BITS);

    if (mast_base >> 15) {
      mast_base |= (1 << (LUT_BASE_BITS-1));
    }
    if (mast_diff >> 15) {
      mast_diff |= (1 << (LUT_DELTA_BITS-1));
    }

    ltm->hw_master_curve[i] =
      ((mast_diff & DELTA_MASK) << LUT_BASE_BITS) | (mast_base & BASE_MASK);
  }
  ltm->hw_master_curve[LTM_HW_LUT_SIZE-1] = ltm->hw_master_curve[LTM_HW_LUT_SIZE-2];
  return TRUE;
}


/** is_rollover_point
 *
 *  @M
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean is_rollover_point(uint32_t M, uint32_t N, uint32_t offset,
  uint32_t *mn_init)
{
  uint32_t mn_init_1 = offset * M % N;
  uint32_t mn_init_2 = (offset + 1) * M % N;

  if (mn_init_1 == 0) {
    *mn_init = 0;
    return TRUE;
  }
  if (mn_init_2 < M && mn_init_2 != 0) {
    *mn_init = mn_init_2;
    return TRUE;
  }
  return FALSE;
}

/** ltm44_calculate_phase
 *
 *  @M
 *
 *  Calculate phase
 *
 *  Return TRUE on success and FALSE on failure
 **/
static void ltm44_calculate_phase(uint32_t  M, uint32_t  N, uint32_t  offset,
  uint32_t interp_reso, uint32_t *mn_init, uint32_t *phase_init,
  uint32_t *h_skip_cnt)
{

  *h_skip_cnt = 0;
  while (!is_rollover_point(M, N, offset, mn_init)) {
    offset++;
    (*h_skip_cnt)++;
  }
  *phase_init = (*mn_init << (SCALE_PHASE_Q + interp_reso)) / M;
}

/** check_if_exceeds_dual_vfe_limits
 *
 *  @input_width: total camif input to vfe in dual vfe configuration
 *  @cell_width: ltm cell width value
 *
 *  Each VFE is limited by 12 cells as per hw design. Each cell will be of
 * width size equal to cell_width passed in. To ensure num of pixels received
 * by left VFE (which is always bigger or euqal to right) do not exceed total
 * area covered by LTM hardawre. This functions returns TRUE if dual vfe limits
 * reached. FALSE if not reached.
 *
 *  Return TRUE if LTM can not cover all pixels and FALSE if LTM can
 **/
boolean check_if_exceeds_dual_vfe_limits(uint32_t input_width,
  uint32_t cell_width)
{
  uint32_t max_ltm_coverage_per_vfe = cell_width * (LTM_H_CELLS - 1);

  /* Check if rightmost split point falls within left VFE's LTM limit */
  if (input_width > 0 &&
    input_width > max_ltm_coverage_per_vfe) {
    ISP_ERR("right most split point %d, is bryond area covered by LTM %d",
      input_width, max_ltm_coverage_per_vfe);
    return TRUE;
  }
  ISP_DBG("right most split point %d,withn area pix covered by LTM %d",
    input_width, max_ltm_coverage_per_vfe);
  return FALSE;
}

/** ltm44_calculate_left_right_overlap
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  compute left and right overlap
 *
 *  Return TRUE on success and FALSE on failure
 **/
void ltm44_calculate_left_right_overlap(uint32_t ltm_sensor_width,
  uint32_t ltm_stripe_point,
  uint32_t* ltm_min_overlap_left,
  uint32_t* ltm_min_overlap_right)
{
  uint32_t stripe_calc_left = 0, stripe_calc_right = 0;
  uint32_t min_ip_index = 0, max_ip_index = 0;
  uint32_t ip_cell_width = 0;

  ip_cell_width = ceil(ltm_sensor_width/LTM_IP_RGN_CNT);

  if(!ip_cell_width) {
    ISP_ERR("ip_cell_width %d ltm_sensor_width %d ", ip_cell_width, ltm_sensor_width);
    ip_cell_width = 1;
  }

  /* Get the IP region that MIN and Max stripe falls into
   * Since there are 12 regions, and index starts from 0, min ip index will
   * be 1 less than calculated
  */
  min_ip_index = ltm_stripe_point / ip_cell_width;

  /* Unless the split point falls on exact boundary, we need to include the
   * entire region in DC domain to get the right value
   */
  max_ip_index = min_ip_index + 1;

  if ((ltm_stripe_point % ip_cell_width) > 0)
    max_ip_index = max_ip_index + 1;

  /* Use the region index to get the IP region point as scaled by DC region*/

  stripe_calc_left = (ltm_dc_region[min_ip_index] * ltm_sensor_width) /
         ltm_dc_region[LTM_DC_RGN_CNT];

  /* Just to be conservative, include two pixels tot the left */
  *ltm_min_overlap_left = stripe_calc_left - 2;

  stripe_calc_right = (ltm_dc_region[max_ip_index] * ltm_sensor_width) /
         ltm_dc_region[LTM_DC_RGN_CNT];

  *ltm_min_overlap_right = stripe_calc_right + 2;

  ISP_DBG("left stripe = %d, overlap = %d",
    stripe_calc_right, *ltm_min_overlap_right);

} /*ltm44_calculate_left_right_overlap*/

/** ltm44_compute_overlap
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  compute overall overlap
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean ltm44_compute_overlap(uint32_t ltm_sensor_width,
  uint32_t ltm_min_stripe,
  uint32_t ltm_max_stripe,
  uint32_t* ltm_min_overlap_left,
  uint32_t* ltm_min_overlap_right)
{
  uint32_t left_overlap_min_stripe = 0, right_overlap_min_stripe = 0;
  uint32_t left_overlap_max_stripe = 0, right_overlap_max_stripe = 0;
  uint32_t ip_cell_width = 0;

  /* Calculate the overlap for min stripe */
  ltm44_calculate_left_right_overlap(ltm_sensor_width, ltm_min_stripe,
      &left_overlap_min_stripe, &right_overlap_min_stripe);

  /* Calculate the overlap for max stripe */
  ltm44_calculate_left_right_overlap(ltm_sensor_width, ltm_max_stripe,
      &left_overlap_max_stripe, &right_overlap_max_stripe);

  /* Left overlap is to left of min stripe
   * similarly right overlap is to the right of max stripe
   */
  *ltm_min_overlap_left = ltm_min_stripe - left_overlap_min_stripe;
  *ltm_min_overlap_right = right_overlap_max_stripe - ltm_max_stripe;

  ip_cell_width = ceil(ltm_sensor_width/LTM_IP_RGN_CNT) + 1;

  if (*ltm_min_overlap_left < ip_cell_width)
    *ltm_min_overlap_left = ip_cell_width;
  else
    *ltm_min_overlap_right = ip_cell_width;

  ISP_DBG("ltm_min_overlap_left %d ltm_min_overlap_right %d",
    *ltm_min_overlap_left, *ltm_min_overlap_right);

  return TRUE;
} /* ltm44_compute_overlap */

/** ltm44_calculate_min_overlap
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  calulate left min overlap and right min overlap
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean ltm44_calculate_min_overlap(
  isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                        ret = TRUE;
  ltm44_t                       *ltm = NULL;
  isp_private_event_t           *private_event = NULL;
  int                            rc = 0;
  isp_stripe_limit_info_t       *stripe_limit = NULL;
  uint32_t                       sensor_output_width = 0;
  uint32_t                       sub_grid_width = 0;
  uint32_t                       mid_point  = 0;
  uint32_t                       ltm_min_overlap_left = 0;
  uint32_t                       ltm_min_overlap_right = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  if (!isp_sub_module->submod_enable) {
    ISP_DBG("skip, ltm enabled %d", isp_sub_module->submod_enable);
    return TRUE;
  }

  ltm = (ltm44_t *)isp_sub_module->private_data;
  if (!ltm) {
    ISP_ERR("failed: ltm %p", ltm);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  stripe_limit = (isp_stripe_limit_info_t *)private_event->data;
  if (!stripe_limit) {
    ISP_ERR("failed: stripe_limit %p", stripe_limit);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  sensor_output_width = ltm->sensor_out_info.request_crop.last_pixel
    - ltm->sensor_out_info.request_crop.first_pixel + 1;

  if (!sensor_output_width) {
    ISP_ERR("failed: last_pixel %d first_pixel %d sensor_output_width %d",
      sensor_output_width, ltm->sensor_out_info.request_crop.last_pixel,
      ltm->sensor_out_info.request_crop.first_pixel);
    ret = FALSE;
    goto ERROR;
  }

  ISP_DBG("min_overlap_left before %d min_overlap_right before %d",
    stripe_limit->min_overlap_left, stripe_limit->min_overlap_right);

  /* value expected is  half of actual */
  char value[PROPERTY_VALUE_MAX];
  int32_t get_val = 0;
  property_get("persist.vendor.camera.ltm.overlap", value, "0");
  get_val = atoi(value);
  if (get_val > 0 && get_val < 50) {
    ltm->cell_overlap_factor = get_val / 10.0f;
    ltm_min_overlap_right = ltm_min_overlap_left =
    (sensor_output_width / (LTM_H_CELLS -1)) * ltm->cell_overlap_factor;
  }
  else {
    ltm->cell_overlap_factor = 1.0;
    ltm44_compute_overlap(sensor_output_width,
      stripe_limit->smallest_split_point,
      stripe_limit->min_left_split,
      &ltm_min_overlap_left,
      &ltm_min_overlap_right);
  }

  if (ltm_min_overlap_left != 0 && ( stripe_limit->min_overlap_left <
    ltm_min_overlap_left))
    stripe_limit->min_overlap_left = ltm_min_overlap_left;
  if (ltm_min_overlap_right != 0 && ( stripe_limit->min_overlap_right <
      ltm_min_overlap_right))
    stripe_limit->min_overlap_right = ltm_min_overlap_right;

  ISP_DBG("min_overlap_left after %d min_overlap_left after %d"
          "overlap_factor %f",stripe_limit->min_overlap_left,
          stripe_limit->min_overlap_right, ltm->cell_overlap_factor);

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* ltm44_calculate_min_overlap */

/** ltm44_set_stripe_info
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean ltm44_set_stripe_info(
  isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                        ret = TRUE;
  ltm44_t                       *ltm = NULL;
  isp_private_event_t           *private_event = NULL;
  int                            rc = 0;
  ispif_out_info_t               *ispif_out_info = NULL;
  uint32_t                       sub_grid_width = 0;
  uint32_t                       sensor_output_width = 0,
                                 left_input_width =0,
                                 cell_width = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  ltm = (ltm44_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(ltm);

  private_event = (isp_private_event_t *)data;
  RETURN_IF_NULL(private_event);

  ispif_out_info = (ispif_out_info_t *)private_event->data;
  RETURN_IF_NULL(ispif_out_info);

  ltm->ispif_out_info = *ispif_out_info;
  ISP_INFO("right stripe offset %d overlap %d ",
    ltm->ispif_out_info.right_stripe_offset,
    ltm->ispif_out_info.overlap);

  sensor_output_width = ltm->sensor_out_info.request_crop.last_pixel
    - ltm->sensor_out_info.request_crop.first_pixel + 1;

  left_input_width = ltm->ispif_out_info.right_stripe_offset+
    ltm->ispif_out_info.overlap;
  cell_width = ((sensor_output_width + 18)/19);

  if (check_if_exceeds_dual_vfe_limits(left_input_width, cell_width)) {
    /* Keep LTM disabled for such exceeding use case*/
    ltm->exceed_split_limits = TRUE;
  }

ERROR:
  return ret;
} /* ltm44_update_min_stripe_overlap */

/** ltm44_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean ltm44_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  ltm44_t            *ltm = NULL;
  isp_out_info_t              *isp_split_out_info = NULL;
  isp_private_event_t         *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  ltm = (ltm44_t *)isp_sub_module->private_data;
  if (!ltm) {
    ISP_ERR("failed: ltm %p", ltm);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  ltm->isp_out_info = *isp_split_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** ltm44_find_stream_type:
 *
 *  @data1: pointer to mct_stream_info_t
 *  @data2: stream mode to compare to
 *
 *  Find if a particular stream type exist
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean ltm44_find_stream_type(void *data1, void *data2)
{
  boolean            rc = FALSE;
  mct_stream_info_t  *stream_data = (mct_stream_info_t *)data1;
  cam_stream_type_t  *stream_type = (cam_streaming_mode_t *)data2;

  if(!stream_data || !stream_type) {
    ISP_ERR("failed: %p %p", stream_data, stream_type);
    goto error;
  }

  if(stream_data->stream_type == *stream_type)
    rc = TRUE;

error:
  return rc;
}

/** ltm44_set_longshot:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to frame skip
 *
 *  Enable/Disable longshot mode
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean ltm44_set_longshot(isp_sub_module_t *isp_sub_module,
  void *data)
{
  ltm44_t            *ltm = NULL;
  int8_t              longshot_enabled;
  isp_private_event_t         *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  private_event = (isp_private_event_t *)data;
  longshot_enabled =
    *(uint32_t *)private_event->data;

  if (!isp_sub_module->submod_enable) {
    ISP_DBG("skip, ltm enabled %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ltm = (ltm44_t *)isp_sub_module->private_data;
  if (!ltm) {
    ISP_ERR("failed: ltm %p", ltm);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  ltm->longshot_enabled = longshot_enabled;
  ISP_DBG("Longshot enabled %d",
    ltm->longshot_enabled );
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** ltm44_set_frame_skip:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to frame skip
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean ltm44_set_frame_skip(isp_sub_module_t *isp_sub_module,
  void *data)
{
  ltm44_t            *ltm = NULL;
  uint32_t            ltm_skip = 0;
  enum msm_vfe_frame_skip_pattern *frame_skip = NULL;
  isp_private_event_t         *private_event = NULL;
  cam_stream_type_t        stream_type;
  mct_list_t               *find_stream_type = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  frame_skip =
    (enum msm_vfe_frame_skip_pattern *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  if (!isp_sub_module->submod_enable) {
    ISP_DBG("skip, ltm enabled %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ltm = (ltm44_t *)isp_sub_module->private_data;
  if (!ltm) {
    ISP_ERR("failed: ltm %p", ltm);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  stream_type = CAM_STREAM_TYPE_POSTVIEW;
  find_stream_type = mct_list_find_custom(isp_sub_module->l_stream_info,
    &stream_type, ltm44_find_stream_type);

  if(find_stream_type == NULL &&
    ltm->longshot_enabled == FALSE) {
    ltm_skip = 0;
    ltm->skip_ip = TRUE;
  }
  else {
    ltm->skip_ip = FALSE;
    ltm_skip = 1;
  }

  if (*frame_skip >= ltm_skip ) {
    ISP_DBG("Frame skip %x is covers min required by LTM", *frame_skip);
  } else {
    *frame_skip = ltm_skip;
  }

  ISP_DBG("frame_skip %x", *frame_skip);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/* ltm44_disable_ltm_ip
 *
 *  @mod : instance of ltm module
 *  @isp_sub_module : handle to isp_sub_module_t
 *  @disable: enable/disable flag if 1 disable IP
 *
 * This fuction is used to enable/disable Image processing block of LTM runtime.
 */
boolean ltm44_disable_ltm_ip(ltm44_t *mod,
  isp_sub_module_t *isp_sub_module, boolean disable)
{
  boolean                        ret = TRUE;
  isp_ltm_reg_t                 *reg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list   *hw_update = NULL;
  struct msm_vfe_cfg_cmd2       *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd    *reg_cfg_cmd = NULL;

  RETURN_IF_NULL(mod);

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  RETURN_IF_NULL(hw_update);
  memset(hw_update, 0, sizeof(*hw_update));

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_MALLOC;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  reg_cmd = (isp_ltm_reg_t *)CAM_MALLOC(sizeof(*reg_cmd));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));

  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd->cmd_type = VFE_CFG_MASK;
  reg_cfg_cmd->u.mask_info.reg_offset = ISP_LTM_CFG_OFF;
  reg_cfg_cmd->u.mask_info.mask = ISP_LTM_IP_EN;
  if (!disable)
    reg_cfg_cmd->u.mask_info.val = 0x4; /* Enable Image processing*/
  else
    reg_cfg_cmd->u.mask_info.val = 0;

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update %d", ret);
    goto ERROR_APPEND;
  }
  return ret;

ERROR_APPEND:
  free(reg_cmd);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_MALLOC:
  free(hw_update);

  return ret;

}/* ltm44_disable_ltm_ip */

/* ltm44_update_3d_gaussian_kernel
 *
 * generate weight table/kernel given sigma values
 */
boolean ltm44_update_3d_gaussian_kernel(isp_sub_module_t *isp_sub_module,
  ltm44_t *mod)
{
  int                    i = 0,
                         j = 0,
                         k = 0;
  double                 SigmaXY = 0.0f,
                         SigmaZ = 0.0f;
  double                 kernelX =  0.0f,
                         kernelY = 0.0f,
                         kernelZ = 0.0f,
                         kernel = 0.0f;
  int                    l = 0;
  chromatix_parms_type  *chromatix = NULL;
  chromatix_LTM_type    *ltm_type = NULL;

  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(mod);

  chromatix =
   (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  RETURN_IF_NULL(chromatix);

  ltm_type = mod->pChromatix_LTM;
  RETURN_IF_NULL(ltm_type);
  chromatix_LTM_reserved *ltm_reserve = &ltm_type->reservedData;
  RETURN_IF_NULL(ltm_reserve);
  SigmaXY = ltm_reserve->fSigmaXY;
  SigmaZ = ltm_reserve->fSigmaZ;

  for (k = 0; k < 6; k++) {
    for (j = 0; j < 3; j++) {
      for (i = 0; i < 3; i++) {
        l = i + j * 3 + k * 9;
        kernelX = (double)i / SigmaXY;
        kernelY = (double)j / SigmaXY;
        kernelZ = (double)k / SigmaZ;
        kernel = exp(-0.5 *
          (kernelX * kernelX + kernelY * kernelY + kernelZ * kernelZ));
        /* weight index goes from 0 to 53 (54 entries) */
        mod->weight_curve[l] = (uint16_t)(kernel * 255.0 + 0.5);
      }
    }
  }
  return TRUE;
}

/** pack_tbl_to_lut
 *
 * converts entries in source table to destination in base|delta format
 */
static void pack_tbl_to_lut_d(double *src, uint32_t *dest, int src_size){
  int i = 0;
  // Convert to LUT64/128 (11s/14s delta | 12s/14s base) format
  for (i=0; i < LTM_HW_LUT_SIZE; i++) {
    int j = src_size * i / LTM_HW_LUT_SIZE;
    int k = src_size * (i + 1) / LTM_HW_LUT_SIZE;
    int32_t base = (int32_t)src[j] & ((1<<LUT_BASE_BITS)-1);  // 12s/14s
    int32_t delta = 0;

    if (k < src_size)
      delta = (int32_t)(src[k]-src[j]);  // 11s/14s
    else delta = (int32_t)(src[k-1]-src[j]);  // 11s/14s
     dest[i] = (delta << LUT_BASE_BITS) | base;
  }
}

/** pack_w_tbl_to_lut
 *
 * converts entries in source table to destination in base|delta format
 * 11/26 - verified bit exact
 */
static void pack_w_tbl_to_lut(uint16_t *src, uint32_t *dest){
  int i = 0;
  /* construct blocks with layout
     ________________________
     |   9   |   6   |   3   |
     ________________________
     |   8   |   5   |   2   |
     ________________________
     |   7   |   4   |   1   |
  */
 for (i=0; i<6; i++) {
    int block_start = 3 * 3 * i;
    int j = 2;
    for( ; j >= 0; j--) {
      int32_t packed = 0;
      packed = (((uint32_t)src[block_start + j + 6] << 16) & 0xFF0000)|
               (((uint32_t)src[block_start + j + 3] << 8)& 0xFF00) |
               (((uint32_t)src[block_start + j] & 0xFF));
      dest[i*3+(2-j)] = packed;
    }
  }
}

/** pack_tbl_to_lut
 *
 * converts entries in source table to destination in base|delta format
 */
static void pack_tbl_to_lut_u32(uint32_t *src, uint32_t *dest, int src_size){
  int i = 0;
  // Convert to LUT64/128 (11s/14s delta | 12s/14s base) format
  for (i=0; i < LTM_HW_LUT_SIZE; i++) {
    int j = src_size * i / LTM_HW_LUT_SIZE;
    int k = src_size * (i + 1) / LTM_HW_LUT_SIZE;
    int32_t base = ((int32_t)src[j] - (1 << (LUT_BASE_BITS - 1))) &
      ((1<<LUT_BASE_BITS)-1);  // 12s/14s
    int32_t delta = 0;

    if (k < src_size)
      delta = (int32_t)(src[k]-src[j]);  // 11s/14s
    else delta = (int32_t)(src[k-1]-src[j]);  // 11s/14s
    dest[i] = (delta << LUT_BASE_BITS) | base;
  }
}

/** ltm44_update_luts:
 *
 *  @mod: module data
 *  @isp_sub_module: isp base module
 *
 *  requests session for tables and pack into module LUTs
 *
 **/
boolean ltm44_pack_hw_luts(ltm44_t *mod)
{
  int32_t i = 0;

  /* pack to LUT for local buffers */
  pack_tbl_to_lut_u32(mod->algo_mask_curve, mod->hw_mask_curve, 64);
  pack_tbl_to_lut_d(mod->algo_master_curve, mod->hw_master_curve, 256);
  pack_tbl_to_lut_d(mod->algo_master_scale, mod->hw_master_scale, 256);
  pack_tbl_to_lut_d(mod->algo_shift_curve, mod->hw_shift_curve, 256);
  pack_tbl_to_lut_d(mod->algo_shift_scale, mod->hw_shift_scale, 256);
  pack_w_tbl_to_lut(mod->weight_curve, mod->hw_weight);
  if (LTM_CURVE_DEBUG && !(mod->frame_number % 20)) {
    ISP_DBG("%s: master_curve: \n", __func__);
    for (i = 0; i < LTM_HW_LUT_SIZE; i++) {
      ISP_DBG("src: %lf dest: %d",
        mod->algo_master_curve[i], mod->hw_master_curve[i]);
    }

    ltm44_dump_lut("mask_curve", mod->hw_mask_curve, 64);
    ltm44_dump_lut("master_curve", mod->hw_master_curve, 64);
  }
  return TRUE;
}

/** ltm44_algo_curve_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean ltm44_algo_curve_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                          ret = TRUE;
  isp_saved_ltm_params_t          *ltm_algo_parm;
  isp_sub_module_priv_t           *isp_sub_module_priv = NULL;
  ltm44_t                         *mod = NULL;
  isp_saved_ltm_params_t          *output = NULL;
  int32_t                          i = 0;

  ISP_DBG("");
  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  RETURN_IF_NULL(isp_sub_module_priv);

  mod = (ltm44_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  ltm_algo_parm =
    (isp_saved_ltm_params_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(ltm_algo_parm);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->hdr_mode ==  CAM_SENSOR_HDR_STAGGERED){
    for (i=0; i < LTM_HW_LUT_SIZE+1; i++) {
      mod->unpacked_master_curve[i] = (int)ltm_algo_parm->master_curve[i * 2] / 4;
      mod->unpacked_master_scale[i] = (int)ltm_algo_parm->master_scale[i * 2] / 4;
      mod->unpacked_shift_curve[i] = (int)ltm_algo_parm->shift_curve[i * 2] / 4;
      mod->unpacked_shift_scale[i] = (int)ltm_algo_parm->shift_scale[i * 2] / 4;
      mod->unpacked_hw_mask[i] = (int)ltm_algo_parm->mask_curve[i * 2] / 4;
    }
  /* write tables to dmi */
    pack_tbl_to_lut_i32(mod->unpacked_hw_mask,
      mod->hw_mask_curve, 1.0f);
    pack_tbl_to_lut_i32(mod->unpacked_master_curve,
      mod->hw_master_curve, 1.0f);
    pack_tbl_to_lut_i32(mod->unpacked_master_scale,
      mod->hw_master_scale, 1.0f);
    pack_tbl_to_lut_i32(mod->unpacked_shift_curve,
      mod->hw_shift_curve, 1.0f);
    pack_tbl_to_lut_i32(mod->unpacked_shift_scale,
      mod->hw_shift_scale, 1.0f);

    isp_sub_module->trigger_update_pending = TRUE;
  } else {
  /* copy data to the destination pointers supplied in request_data */
  memcpy(mod->algo_master_curve, ltm_algo_parm->master_curve,
    LTM_MC_TBL_SIZE * sizeof(double));
  memcpy(mod->algo_master_scale, ltm_algo_parm->master_scale,
    LTM_MS_TBL_SIZE * sizeof(double));
  memcpy(mod->algo_shift_curve, ltm_algo_parm->shift_curve,
    LTM_SC_TBL_SIZE * sizeof(double));
  memcpy(mod->algo_shift_scale, ltm_algo_parm->shift_scale,
    LTM_SS_TBL_SIZE * sizeof(double));
  memcpy(mod->algo_mask_curve, ltm_algo_parm->mask_curve,
    LTM_MASK_TBL_SIZE * sizeof(uint32_t));
  if (LTM_CURVE_DEBUG  && !(mod->frame_number % 20)) {
    ISP_DBG("master_curve: ");
    for (i=0; i<LTM_MC_TBL_SIZE; i++) {
      ISP_HIGH("src: %lf dest: %lf",
        ltm_algo_parm->master_curve[i], mod->algo_master_curve[i]);
    }

    ltm44_dump_lut("mask_curve", (uint32_t *)mod->algo_mask_curve, 64);
    ltm44_dump_lut("master_curve", (uint32_t *)mod->algo_master_curve, 256);
  }
  /* Currently not using algorithm curve may use later */
  /*  ltm44_pack_hw_luts(isp_sub_module, mod); */
  /*  isp_sub_module->trigger_update_pending = TRUE; */
 }
END:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}


/** ltm44_enable_disable_region_based:
 *
 *  @isp_sub_module: isp_sub_module_t
 *  @data: ltm module data
 *
 *   Check LTM region and based on the region
 *   Enable/Disable LTM
 *   Return TRUE on success and FALSE on failure
 **/
boolean ltm44_enable_disable_region_based(isp_sub_module_t *isp_sub_module,
  void *data)
{
  chromatix_parms_type    *chromatix             = NULL;
  ASD_VFE_struct_type     *ASD_algo_data         = NULL;
  chromatix_LTM_type      *chromatix_ltm_backlit = NULL;
  chromatix_LTM_type      *chromatix_ltm_normal  = NULL;
  ltm44_t                 *mod                   = NULL;
  boolean ret = TRUE;
  boolean mod_enable = FALSE;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: isp_sub_module %p, data %p", isp_sub_module, data);
    return FALSE;
  }

  mod = (ltm44_t *)data;
  chromatix = (chromatix_parms_type *)
   isp_sub_module->chromatix_ptrs.chromatixPtr;
  ASD_algo_data = &(chromatix->ASD_algo_data);
  chromatix_ltm_normal =
    &(chromatix->chromatix_post_processing.chromatix_LTM_data);


  if (mod->ext_func_table && mod->ext_func_table->find_rgn) {
    ret = mod->ext_func_table->find_rgn(chromatix_ltm_normal, &mod->aec_update,
      &mod->normal_idx);
    if (ret == FALSE) {
      ISP_ERR("failed: find_rgn");
      return FALSE;
    }
  }  else {
    ISP_ERR("failed: %p", mod->ext_func_table);
    ret =  FALSE;
  }

  if (ASD_algo_data->backlit_scene_detect.backlit_LTM.enable) {
    chromatix_ltm_backlit = &(ASD_algo_data->backlit_scene_detect.backlit_LTM);
    RETURN_IF_NULL(chromatix_ltm_backlit);
    if (mod->ext_func_table && mod->ext_func_table->find_rgn) {
      ret = mod->ext_func_table->find_rgn(chromatix_ltm_backlit, &mod->aec_update,
        &mod->backlit_idx);
      if (ret == FALSE) {
        ISP_ERR("failed: find_rgn");
        return FALSE;
      }
    }  else {
      ISP_ERR("failed: %p", mod->ext_func_table);
      ret =  FALSE;
    }
  } else {
    mod->backlit_idx.ltm_region_enable = FALSE;
  }

  /*enable/disable LTM based on region specific enable/disable*/
  mod_enable = ltm44_check_enable(isp_sub_module, mod);
  if (mod_enable){
    mod->disable_ip = FALSE;
  } else {
    mod->disable_ip = TRUE;
  }


  ISP_DBG("Backlit Enable %d Normal enable %d",
    ASD_algo_data->backlit_scene_detect.backlit_LTM.enable,
    chromatix_ltm_normal->enable);

  return ret;
}

/** ltm44_interpolate_ltm_region:
 *
 *  @isp_sub_module: isp_sub_module_t
 *  @data: ltm module data
 *
 *  Interpolate LTM region. If backlit is enabled then
 *  interpolate also with back Chromatix LTM
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean ltm44_interpolate_ltm_region(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                ret = TRUE;
  chromatix_parms_type  *chromatix = NULL;
  chromatix_LTM_core    *chromatix_ltm_core = NULL;
  AAA_ASD_struct_type   *ASD_3a_data = NULL;
  chromatix_3a_parms_type *chromatix_3a_ptr = NULL;
  ASD_VFE_struct_type     *ASD_algo_data = NULL;
  chromatix_LTM_type    *chromatix_ltm_backlit = NULL;
  chromatix_LTM_type    *chromatix_ltm_normal = NULL;
  float                  ratio = 0;
  float                  max_percent_threshold;
  ltm44_t               *mod = NULL;
  int32_t                nGlobalToneStrength_backlit;
  int32_t                nGlobalToneContrast_backlit;
  int32_t                nGlobalToneWeight_backlit;
  int32_t                nLocalToneStrength_backlit;
  int32_t                nLocalToneContrast_backlit;
  float                  fixed_content_low_backlit;
  float                  fixed_content_high_backlit;
  int                    i = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: isp_sub_module %p, data %p", isp_sub_module, data);
    return FALSE;
  }

  mod = (ltm44_t *)data;
  chromatix = (chromatix_parms_type *)
   isp_sub_module->chromatix_ptrs.chromatixPtr;
  ASD_algo_data = &(chromatix->ASD_algo_data);
  chromatix_ltm_normal =
    &(chromatix->chromatix_post_processing.chromatix_LTM_data);

  chromatix_3a_ptr = (chromatix_3a_parms_type *)
    isp_sub_module->chromatix_ptrs.chromatix3APtr;
  RETURN_IF_NULL(chromatix_3a_ptr);
  ASD_3a_data = &(chromatix_3a_ptr->ASD_3A_algo_data);
  max_percent_threshold = ASD_3a_data->backlit_scene_detect.max_percent_threshold;

  mod->nGlobalToneStrength = (int)Round((float)LINEAR_INTERPOLATION_LTM(
    (float)LTM_CORE_TYPE(chromatix_ltm_normal,
    nGlobalToneStrength, mod->normal_idx.tr_index),
    (float)LTM_CORE_TYPE(chromatix_ltm_normal,
    nGlobalToneStrength, mod->normal_idx.tr_index + 1),
    mod->normal_idx.ratio));
  mod->nGlobalToneContrast = (int)Round((float)LINEAR_INTERPOLATION_LTM(
    (float)LTM_CORE_TYPE(chromatix_ltm_normal,
    nGlobalToneContrast, mod->normal_idx.tr_index),
    (float)LTM_CORE_TYPE(chromatix_ltm_normal,
    nGlobalToneContrast, mod->normal_idx.tr_index + 1),
    mod->normal_idx.ratio));
  mod->nLocalToneContrast = (int)Round((float)LINEAR_INTERPOLATION_LTM(
    (float)LTM_CORE_TYPE(chromatix_ltm_normal,
    nLocalToneContrast, mod->normal_idx.tr_index),
    (float)LTM_CORE_TYPE(chromatix_ltm_normal,
    nLocalToneContrast, mod->normal_idx.tr_index + 1),
    mod->normal_idx.ratio));
  mod->nLocalToneStrength = (int)Round((float)LINEAR_INTERPOLATION_LTM(
    (float)LTM_CORE_TYPE(chromatix_ltm_normal,
    nLocalToneStrength, mod->normal_idx.tr_index),
    (float)LTM_CORE_TYPE(chromatix_ltm_normal,
    nLocalToneStrength, mod->normal_idx.tr_index + 1),
    mod->normal_idx.ratio));
  mod->fixed_content_low = ((float)LINEAR_INTERPOLATION_LTM(
    (float)LTM_CORE_TYPE(chromatix_ltm_normal,
    fixed_content_low, mod->normal_idx.tr_index),
    (float)LTM_CORE_TYPE(chromatix_ltm_normal,
    fixed_content_low, mod->normal_idx.tr_index + 1),
    mod->normal_idx.ratio));
  mod->fixed_content_high = ((float)LINEAR_INTERPOLATION_LTM(
    (float)LTM_CORE_TYPE(chromatix_ltm_normal,
    fixed_content_high, mod->normal_idx.tr_index),
    (float)LTM_CORE_TYPE(chromatix_ltm_normal,
    fixed_content_high, mod->normal_idx.tr_index + 1),
    mod->normal_idx.ratio));

  if (ASD_algo_data->backlit_scene_detect.backlit_LTM.enable) {
    chromatix_ltm_backlit = &(ASD_algo_data->backlit_scene_detect.backlit_LTM);
    RETURN_IF_NULL(chromatix_ltm_backlit);

    nGlobalToneStrength_backlit = (int)Round((float)LINEAR_INTERPOLATION_LTM(
      (float)LTM_CORE_TYPE(chromatix_ltm_backlit,
      nGlobalToneStrength, mod->backlit_idx.tr_index),
      (float)LTM_CORE_TYPE(chromatix_ltm_backlit,
      nGlobalToneStrength, mod->backlit_idx.tr_index + 1),
      mod->backlit_idx.ratio));

    nGlobalToneContrast_backlit = (int)Round((float)LINEAR_INTERPOLATION_LTM(
      (float)LTM_CORE_TYPE(chromatix_ltm_backlit,
      nGlobalToneContrast, mod->backlit_idx.tr_index),
      (float)LTM_CORE_TYPE(chromatix_ltm_backlit,
      nGlobalToneContrast, mod->backlit_idx.tr_index + 1),
      mod->backlit_idx.ratio));


    nLocalToneContrast_backlit = (int)Round((float)LINEAR_INTERPOLATION_LTM(
      (float)LTM_CORE_TYPE(chromatix_ltm_backlit,
      nLocalToneContrast, mod->backlit_idx.tr_index),
      (float)LTM_CORE_TYPE(chromatix_ltm_backlit,
      nLocalToneContrast, mod->backlit_idx.tr_index + 1),
      mod->backlit_idx.ratio));
    nLocalToneStrength_backlit = (int)Round((float)LINEAR_INTERPOLATION_LTM(
      (float)LTM_CORE_TYPE(chromatix_ltm_backlit,
      nLocalToneStrength, mod->backlit_idx.tr_index),
      (float)LTM_CORE_TYPE(chromatix_ltm_backlit,
      nLocalToneStrength, mod->backlit_idx.tr_index + 1),
      mod->backlit_idx.ratio));
   fixed_content_high_backlit = ((float)LINEAR_INTERPOLATION_LTM(
      (float)LTM_CORE_TYPE(chromatix_ltm_backlit,
      fixed_content_high, mod->backlit_idx.tr_index),
      (float)LTM_CORE_TYPE(chromatix_ltm_backlit,
      fixed_content_high, mod->backlit_idx.tr_index + 1),
      mod->backlit_idx.ratio));
   fixed_content_low_backlit = ((float)LINEAR_INTERPOLATION_LTM(
      (float)LTM_CORE_TYPE(chromatix_ltm_backlit,
      fixed_content_low, mod->backlit_idx.tr_index),
      (float)LTM_CORE_TYPE(chromatix_ltm_backlit,
      fixed_content_low, mod->backlit_idx.tr_index + 1),
      mod->backlit_idx.ratio));

   if (chromatix_ltm_backlit->enable &&
      chromatix_ltm_normal->enable) {
      ratio = 1.0f - max_percent_threshold;
      mod->nGlobalToneStrength = (int)Round((float)LINEAR_INTERPOLATION_LTM(
       (float)mod->nGlobalToneStrength, (float)nGlobalToneStrength_backlit, ratio));
      mod->nGlobalToneContrast = (int)Round((float)LINEAR_INTERPOLATION_LTM(
        (float)mod->nGlobalToneContrast, (float)nGlobalToneContrast_backlit,ratio));
      mod->nLocalToneContrast = (int)Round((float)LINEAR_INTERPOLATION_LTM(
       (float)mod->nLocalToneContrast, (float)nLocalToneContrast_backlit, ratio));
      mod->nLocalToneStrength = (int)Round((float)LINEAR_INTERPOLATION_LTM(
       (float)mod->nLocalToneStrength, (float)nLocalToneStrength_backlit, ratio));
      mod->fixed_content_high = ((float)LINEAR_INTERPOLATION_LTM(
       (float)mod->fixed_content_high, (float)fixed_content_high_backlit, ratio));
      mod->fixed_content_low = ((float)LINEAR_INTERPOLATION_LTM(
       (float)mod->fixed_content_low, (float)fixed_content_low_backlit, ratio));
    } else if (chromatix_ltm_backlit->enable) {
      mod->nGlobalToneStrength = nGlobalToneStrength_backlit;
      mod->nGlobalToneContrast = nGlobalToneContrast_backlit;
      mod->nLocalToneContrast =  nLocalToneContrast_backlit;
      mod->nLocalToneStrength = nLocalToneStrength_backlit;
      mod->fixed_content_high =  fixed_content_high_backlit;
      mod->fixed_content_low = fixed_content_low_backlit;
    }
  }

  /* Add GTC range check for backward compatability */
  if ( mod->nGlobalToneContrast < 25 || mod->nGlobalToneContrast > 460)
    mod->nGlobalToneContrast = mod->nGlobalToneStrength;

  mod->fixed_content_high = CLAMP_LTM(mod->fixed_content_high, 0.0f, 4.0f);
  mod->fixed_content_low = CLAMP_LTM(mod->fixed_content_low, 0.0f, 4.0f);

  return ret;
}

/** ltm44_stats_aec_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean ltm44_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret = TRUE;
  stats_update_t          *stats_update = NULL;
  aec_update_t            *cur_aec_update = NULL;
  aec_update_t            *new_aec_update = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_LA_type       *chromatix_LA = NULL;
  ltm44_t                 *mod = NULL;
  trigger_ratio_t          trigger_ratio;
  trigger_point_type      *trigger_point = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  mod= (ltm44_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(stats_update);

  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return TRUE;
  }
  chromatix_LA = &chromatix_ptr->chromatix_VFE.chromatix_LA;
  trigger_point = &(chromatix_LA->la_brightlight_trigger);
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  cur_aec_update = &mod->aec_update;
  new_aec_update = &stats_update->aec_update;

  if(isp_sub_module->submod_trigger_enable == FALSE) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

#ifdef LTM_ALGO_BASED_CURVES
  /* Decide the trigger ratio for current lighting condition,
     fill in trigger ratio*/
  trigger_ratio.ratio = isp_sub_module_util_get_aec_ratio_bright(
    chromatix_LA->control_la, (void *)trigger_point, new_aec_update);

  /*protect the ratio if out of bound*/
  if (trigger_ratio.ratio > 1.0)
    trigger_ratio.ratio = 1.0;
  else if (trigger_ratio.ratio < 0.0)
    trigger_ratio.ratio = 0.0;

    /*if aec  change, mark trigger update as TRUE*/
  if (!F_EQUAL(trigger_ratio.ratio, mod->trigger_ratio.ratio)) {
    ISP_HIGH("aec trigger upadate, old ratio = %f"
      " new ratio = %f", mod->trigger_ratio.ratio,
      trigger_ratio.ratio);
  } else {
    isp_sub_module->trigger_update_pending = TRUE;
    mod->trigger_ratio = trigger_ratio;
    ltm44_update_8k_params(isp_sub_module, mod);
  }
#endif

  mod->enable_adrc =
    isp_sub_module_util_is_adrc_mod_enable(new_aec_update->ltm_ratio,
      new_aec_update->total_drc_gain);

  if(new_aec_update->lux_idx != cur_aec_update->lux_idx ||
     !F_EQUAL(new_aec_update->real_gain, cur_aec_update->real_gain) ||
     new_aec_update->exp_index != cur_aec_update->exp_index) {
      isp_sub_module->trigger_update_pending = TRUE;
    } else if (mod->enable_adrc == TRUE) {
      if (!F_EQUAL(new_aec_update->total_drc_gain,
         cur_aec_update->total_drc_gain) ||
        !F_EQUAL(new_aec_update->ltm_ratio, cur_aec_update->ltm_ratio)) {
        isp_sub_module->trigger_update_pending = TRUE;
      }
    }

   if (isp_sub_module->trigger_update_pending) {
     *cur_aec_update = *new_aec_update;
      ltm44_enable_disable_region_based(isp_sub_module, mod);
   }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** ltm44_stats_asd_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 * Handle ASD update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean ltm44_stats_asd_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  stats_update_t          *stats_update = NULL;
  asd_update_t            *cur_asd = NULL;
  asd_update_t            *new_asd = NULL;
  ltm44_t                 *mod = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  mod = (ltm44_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);


  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(stats_update);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  cur_asd = &mod->asd_update;
  new_asd = &stats_update->asd_update;
  if(isp_sub_module->submod_trigger_enable == FALSE) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  if (mod->enable_backlight_compensation &&
      new_asd->backlight_detected &&
      (cur_asd->backlight_scene_severity !=
        new_asd->backlight_scene_severity)) {
    mod->backlight_comp_update = TRUE;
    mod->enable_backlight_compensation = FALSE;
    isp_sub_module->trigger_update_pending = TRUE;
    ltm44_update_8k_params(isp_sub_module, mod);
  }
  /* Store ASD update in module private */
  *cur_asd = *new_asd;
   cur_asd->backlight_scene_severity =
     MIN(255, new_asd->backlight_scene_severity);
   mod->backlight_severity =
     cur_asd->backlight_scene_severity;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** calculate_ltm_scaler_factor
 *    @scale_factor_horiz: horizontal scale factor
 *    @scale_factor_vert:  vertical scale factor
 *    @hFactor: output
 *    @vFactor: output
 *
 * calculate ltm downscaler factor
 *
 * Return: TRUE
 **/
static boolean calculate_ltm_scaler_factor(int scale_factor_horiz,
  int scale_factor_vert, uint32_t *h_inter_resol, uint32_t *v_inter_resol)
{

  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    ISP_DBG(" Output1 scale larger than camsensor FOV, set scale_factor=1");
    scale_factor_horiz = 1;
    scale_factor_vert = 1;
  }

  if (scale_factor_horiz >= 1 && scale_factor_horiz < 4) {
    *h_inter_resol = 3;
  } else if (scale_factor_horiz >= 4 && scale_factor_horiz < 8) {
    *h_inter_resol = 2;
  } else if (scale_factor_horiz >= 8 && scale_factor_horiz < 16) {
    *h_inter_resol = 1;
  } else if (scale_factor_horiz >= 16 && scale_factor_horiz
                                          < MAX_SCALE_FACTOR_VAL) {
    *h_inter_resol = 0;
  } else {
    ISP_DBG("scale_factor_horiz is greater than %d, which is more than "
      "the supported maximum scale factor.", MAX_SCALE_FACTOR_VAL);
    return FALSE;
  }

  if (scale_factor_vert >= 1 && scale_factor_vert < 4) {
    *v_inter_resol = 3;
  } else if (scale_factor_vert >= 4 && scale_factor_vert < 8) {
    *v_inter_resol = 2;
  } else if (scale_factor_vert >= 8 && scale_factor_vert < 16) {
    *v_inter_resol = 1;
  } else if (scale_factor_vert >= 16 && scale_factor_vert
                                         < MAX_SCALE_FACTOR_VAL) {
    *v_inter_resol = 0;
  } else {
    ISP_DBG("scale_factor_vert is greater than %d, which is more than "
      "the supported maximum scale factor.", MAX_SCALE_FACTOR_VAL);
    return FALSE;
  }

  return TRUE;
}/* calculate_scaler44_factor */

/*ltm scaler configuration */
/** calculate_interp_reso
 *
 *  calculate ratio
 */
static int calculate_interp_reso(int input, int output) {
  int compare = 16;
  float MN_ratio = input/output;
  int i = 0;
  for (i = 0; i < 3; i++) {
    if ((MN_ratio) >= compare) {
      return i;
    }
    compare = compare/2;
  }
  return 3;
}

/* ltm44_scale_reg_update_platform *
 * @reg - isp_ltm_reg_t *
 *
 * changing values for some register before
 * writing to the hardware *
 */
static void ltm44_scale_reg_update_platform(isp_ltm_reg_t *reg)
{
   if (LTM_VERSION == 46) {
      /* In VFE 46 , for this registers writing n means n+1 */
      reg->ltm_h_scale_cfg.h_image_size.hIn   -= 1;
      reg->ltm_h_scale_cfg.h_image_size.hOut  -= 1;
      reg->ltm_scale_h_pad_cfg.scaleYInWidth  -= 1;
      reg->ltm_v_scale_cfg.v_image_size.vIn   -= 1;
      reg->ltm_v_scale_cfg.v_image_size.vOut  -= 1;
      reg->ltm_scale_v_pad_cfg.scaleYInHeight -= 1;
  }
}

/** ltm44_splt_config
 *  @isp_sub_module: isp sub module object
 *  @mod: ltm44 module object
 *
 *  configures all of the register variables to be programmed
*/
static boolean ltm44_split_config(isp_sub_module_t *isp_sub_module,
  ltm44_t *mod)
{
  int                     tmp_scaled_right_offset = 0;
  int                     cnt_init_dc_cell= 0;
  int                     cellwidth = 0;
  int32_t                 left_input_width = 0, right_input_width = 0,
                          sensor_input_width = 0, sensor_input_height = 0,
                          input_width = 0, input_height = 0;
  float                   ratio = 1.0f;
  isp_ltm_reg_t           *reg = NULL;
  uint32_t                ltm_ds_width = LTM_DS_WIDTH;

  RETURN_IF_NULL(mod);
  RETURN_IF_NULL(isp_sub_module);

  reg = &(mod->reg);
  sensor_input_width  = mod->sensor_out_info.request_crop.last_pixel -
    mod->sensor_out_info.request_crop.first_pixel + 1;
  sensor_input_height = mod->sensor_out_info.request_crop.last_line -
    mod->sensor_out_info.request_crop.first_line + 1;

  if (mod->isp_out_info.is_split == TRUE) {
    left_input_width = mod->ispif_out_info.right_stripe_offset +
      mod->ispif_out_info.overlap;
    right_input_width = sensor_input_width -
      mod->ispif_out_info.right_stripe_offset;
    input_width = reg->ltm_dcip_cfg.ip_cellwidth.cellwidthL * (LTM_H_CELLS_SPLIT - 1);
    ltm_ds_width = LTM_DS_SPLIT_WIDTH;
  }

  if (input_width == 0) {
    ISP_ERR("input_width is zero, init has issues");
    return FALSE;
  }

  tmp_scaled_right_offset = ((mod->ispif_out_info.right_stripe_offset)
    * ltm_ds_width) / input_width;
  /* This scaler is for right VFE. calculate actual scaler output
   * coming out of right VFE. this info will be used by fovcrop to
   * configure its register */
  if (mod->isp_out_info.stripe_id == ISP_STRIPE_RIGHT) {
    /* DC config, first DC grid is half grid*/
    tmp_scaled_right_offset = tmp_scaled_right_offset - (LTM_DC_CELLWIDTH >> 1);
    cnt_init_dc_cell = (tmp_scaled_right_offset / LTM_DC_CELLWIDTH);

    /* IP Config, start from FULL grid */
    IP_INIT_CELLNUM(reg) = (mod->isp_out_info.right_stripe_offset) /
      reg->ltm_dcip_cfg.ip_cellwidth.cellwidthL;

    DC_INIT_CELLNUM(reg) = 1;
    reg->ltm_dcip_cfg.dc_cfg.initDX = (tmp_scaled_right_offset %
      (LTM_DC_CELLWIDTH* cnt_init_dc_cell));

    if (reg->ltm_dcip_cfg.dc_cfg.initDX >= 13)
        IP_INIT_CELLNUM(reg) = 1;
    else
        IP_INIT_CELLNUM(reg) = 0;

    reg->ltm_dcip_cfg.ip_init.initDX =
      mod->isp_out_info.right_stripe_offset %
    reg->ltm_dcip_cfg.ip_cellwidth.cellwidthL;

    /* Initial cell width size is 13 and is not exact half of 27,
     * adjust the first grid offset.
     */
    if (!reg->ltm_dcip_cfg.dc_cfg.initDX)
        reg->ltm_dcip_cfg.ip_init.initDX -=
          ((float)reg->ltm_dcip_cfg.ip_cellwidth.cellwidthL * 0.02f);

    reg->ltm_dcip_cfg.ip_init.initPX = reg->ltm_dcip_cfg.ip_init.initDX *
      reg->ltm_dcip_cfg.ip_inv_cellwidth.invCellwidthL;

    ISP_DBG("ltm_dbg === STRIPE ID : %d (0-LEFT/ 1-RIGHT)====",
      mod->isp_out_info.stripe_id);
    ISP_DBG("ltm_dbg  right stripe offset %d, overlap %d",
      mod->ispif_out_info.right_stripe_offset, mod->ispif_out_info.overlap);
    ISP_DBG("ltm_dbg DC_cfg: cell num %d, initDX %d, initDx position %f",
      DC_INIT_CELLNUM(reg), reg->ltm_dcip_cfg.dc_cfg.initDX,
      (float)reg->ltm_dcip_cfg.dc_cfg.initDX/LTM_DC_CELLWIDTH);
    ISP_DBG("ltm_dbg IP cfg: cell num %d, initDX %d, initDx position %f CW %d",
      IP_INIT_CELLNUM(reg), reg->ltm_dcip_cfg.ip_init.initDX,
      (float) reg->ltm_dcip_cfg.ip_init.initDX /
      reg->ltm_dcip_cfg.ip_cellwidth.cellwidthL,
      reg->ltm_dcip_cfg.ip_cellwidth.cellwidthL);
  }

  mod->enable_backlight_compensation = TRUE;
  isp_sub_module->submod_trigger_enable = TRUE;

  return TRUE;
}
/** ltm44_general_config
 *  @isp_sub_module: isp sub module object
 *  @mod: ltm44 module object
 *
 *  configures all of the register variables to be programmed
*/
static boolean ltm44_general_config(ltm44_t *mod,
  isp_sub_module_t *isp_sub_module)
{
   isp_ltm_reg_t *reg = NULL;
   int32_t  input_width = 0,input_height = 0;
   uint32_t debug_out_select =0;
   char value[PROPERTY_VALUE_MAX];

   RETURN_IF_NULL(mod);
   RETURN_IF_NULL(isp_sub_module);

  reg = &(mod->reg);
  /* general config */
  /* needs to be switched off first frame */
  if (isp_sub_module->submod_enable == FALSE) {
    mod->reg.ltm_cfg.ipEnable = 0;
    mod->reg.ltm_cfg.dcEnable = 0;
  } else {
    reg->ltm_cfg.ipEnable = 1;
    reg->ltm_cfg.dcEnable = 1;
  }
  /* 0 = hw, 1 = sw */
  reg->ltm_cfg.autoSwitchOverride = 1;
  /* use only when SW override */
  reg->ltm_cfg.dc3dAvgPongSel = 0;
  reg->ltm_cfg.dc3dSumClear = 1;
  /* use only when SW override */
  reg->ltm_cfg.ip3dAvgPongSel = 0;
  reg->ltm_cfg.lutBankSelect = 0;
  /*0x0: LTM RGB Out, 0x1 mask 0x2 Yout 0x3 Yp */
  property_get("persist.vendor.camera.ltm.out", value, "0");
  debug_out_select = atoi(value);
  ISP_HIGH("debug out value %s %d", value, debug_out_select);
  reg->ltm_cfg.debugOutSelect = debug_out_select;

  return TRUE;

}

/** ltm44_rgb2y_config
 *  @isp_sub_module: isp sub module object
 *  @mod: ltm44 module object
 *
 *  configures all of the register variables to be programmed
*/

static boolean ltm44_rgb2y_config(ltm44_t *mod,
  isp_sub_module_t *isp_sub_module)
{
  chromatix_LTM_type    *ltm_pp = NULL;
  isp_ltm_reg_t         *reg = &(mod->reg);
  chromatix_parms_type  *chromatix = NULL;
  chromatix_LTM_type    *ltm = NULL;
  float                  asd_max_percent_threshold = 0.0;
  int                    backlit_detect_enable = 0;
  float                  ratio = 1.0f;
  int32_t                idx = 2;
  LTM_light_type         default_idx = LTM_NORMAL_LIGHT;

  RETURN_IF_NULL(isp_sub_module);
  chromatix = (chromatix_parms_type *)
    isp_sub_module->chromatix_ptrs.chromatixPtr;
  RETURN_IF_NULL(chromatix);

  ASD_VFE_struct_type     *ASD_algo_data = NULL;
  AAA_ASD_struct_type     *ASD_3a_data = NULL;
  chromatix_3a_parms_type *chromatix_3a_ptr = NULL;
  chromatix_3a_ptr = (chromatix_3a_parms_type *)
        isp_sub_module->chromatix_ptrs.chromatix3APtr;

  RETURN_IF_NULL(chromatix_3a_ptr);
  ASD_algo_data = &(chromatix->ASD_algo_data);
  ASD_3a_data = &(chromatix_3a_ptr->ASD_3A_algo_data);
  asd_max_percent_threshold = ASD_3a_data->backlit_scene_detect.max_percent_threshold;
  backlit_detect_enable = ASD_3a_data->backlit_scene_detect.backlight_detection_enable;

  ltm_pp = &(chromatix->chromatix_post_processing.chromatix_LTM_data);
  if(!backlit_detect_enable){
    ltm = ltm_pp ;
  } else {
    ltm = &(ASD_algo_data->backlit_scene_detect.backlit_LTM);
  }

  if(!backlit_detect_enable){
      mod->nGlobalToneContrast = LTM_CORE_TYPE(ltm_pp, nGlobalToneContrast, default_idx);
      mod->nGlobalToneStrength = LTM_CORE_TYPE(ltm_pp, nGlobalToneStrength, default_idx);
      mod->nGlobalToneWeight = LTM_CORE_TYPE(ltm_pp, nGlobalToneWeight, default_idx);
      mod->nLocalToneContrast = LTM_CORE_TYPE(ltm_pp, nLocalToneContrast, default_idx);
      mod->nLocalToneStrength = LTM_CORE_TYPE(ltm_pp, nLocalToneStrength, default_idx);
  } else {
    ratio = 1.0f - asd_max_percent_threshold;

    mod->nGlobalToneContrast = (int)Round((float)LINEAR_INTERPOLATION_LTM(
      (float)ltm_pp->chromatix_ltm_core_data[default_idx].nGlobalToneContrast,
      (float)ltm->chromatix_ltm_core_data[default_idx].nGlobalToneContrast, ratio));
    mod->nGlobalToneStrength = (int)Round((float)LINEAR_INTERPOLATION_LTM(
      (float)ltm_pp->chromatix_ltm_core_data[default_idx].nGlobalToneStrength,
      (float)ltm->chromatix_ltm_core_data[default_idx].nGlobalToneStrength, ratio));
    mod->nGlobalToneWeight = (int)Round((float)LINEAR_INTERPOLATION_LTM(
      (float)ltm_pp->chromatix_ltm_core_data[default_idx].nGlobalToneWeight,
      (float)ltm->chromatix_ltm_core_data[default_idx].nGlobalToneWeight, ratio));
    mod->nLocalToneContrast = (int)Round((float)LINEAR_INTERPOLATION_LTM(
      (float)ltm_pp->chromatix_ltm_core_data[default_idx].nLocalToneContrast,
      (float)ltm->chromatix_ltm_core_data[default_idx].nLocalToneContrast, ratio));
    mod->nLocalToneStrength = (int)Round((float)LINEAR_INTERPOLATION_LTM(
      (float)ltm_pp->chromatix_ltm_core_data[default_idx].nLocalToneStrength,
      (float)ltm->chromatix_ltm_core_data[default_idx].nLocalToneStrength, ratio));
  }

 /* Add GTC range check for backward compatability */
 if (mod->nGlobalToneContrast < 25 || mod->nGlobalToneContrast > 460)
    mod->nGlobalToneContrast = mod->nGlobalToneStrength;

 mod->pChromatix_LTM = ltm;

 /* yp coefficents */
 reg->ltm_dcip_cfg.dc_cfg.binInitCnt = LTM_TYPE(mod->pChromatix_LTM,
     reservedData).nInitCntFactor;
 reg->ltm_rgb2y_cfg.rgb2y_cfg0.c1 =
   (uint16_t)(LTM_TYPE(mod->pChromatix_LTM, reservedData).fCV_R * 1024 + 0.5);
 reg->ltm_rgb2y_cfg.rgb2y_cfg0.c2 =
   (uint16_t)(LTM_TYPE(mod->pChromatix_LTM, reservedData).fCV_G * 1024 + 0.5);
 reg->ltm_rgb2y_cfg.rgb2y_cfg0.c3 =
   (uint16_t)(LTM_TYPE(mod->pChromatix_LTM, reservedData).fCV_B * 1024 + 0.5);
 reg->ltm_rgb2y_cfg.rgb2y_cfg1.thr =
   (uint16_t)(LTM_TYPE(mod->pChromatix_LTM, reservedData).fCV_Cthr * 4 + 0.5);
 reg->ltm_rgb2y_cfg.rgb2y_cfg1.k =
   (uint16_t)(LTM_TYPE(mod->pChromatix_LTM, reservedData).fCV_Kc * 512 + 0.5);
 reg->ltm_rgb2y_cfg.rgb2y_cfg1.c4 =
   (uint16_t)(LTM_TYPE(mod->pChromatix_LTM, reservedData).fCV_Kmax * 1024 + 0.5);

 return TRUE;

}

/** ltm44_scaler_config
 *  @isp_sub_module: isp sub module object
 *  @mod: ltm44 module object
 *
 *  configures all of the register variables to be programmed
*/

static boolean ltm44_scaler_config(ltm44_t *mod,
  isp_sub_module_t *isp_sub_module)
{
 unsigned int scale_factor_horiz, scale_factor_vert;
 uint32_t   h_interp, h_phase_increment, v_interp, v_phase_increment;
 uint32_t   interp_reso = 0,
            mn_init = 0,
            phase_init = 0,
            h_skip_cnt = 0;
 int32_t    input_width = 0,
            input_height = 0;
 isp_ltm_reg_t *reg = NULL;
 uint32_t  ltm_ds_width = LTM_DS_WIDTH;

 RETURN_IF_NULL(mod);
 RETURN_IF_NULL(isp_sub_module);

 reg = &(mod->reg);

 input_width = mod->sensor_out_info.request_crop.last_pixel -
   mod->sensor_out_info.request_crop.first_pixel + 1;
 input_height = mod->sensor_out_info.request_crop.last_line -
   mod->sensor_out_info.request_crop.first_line + 1;

 if (mod->isp_out_info.is_split == TRUE) {
   ltm_ds_width = LTM_DS_SPLIT_WIDTH;
 }

 /* phase register temporary */
 scale_factor_horiz = input_width / ltm_ds_width;
 scale_factor_vert = input_height / LTM_DS_HEIGHT;

 calculate_ltm_scaler_factor(scale_factor_horiz,
    scale_factor_vert, &h_interp, &v_interp);
  h_phase_increment = 1 << (SCALE_PHASE_Q + h_interp);
  v_phase_increment = 1 << (SCALE_PHASE_Q + v_interp);

 /* mn downscaler enable */
 reg->ltm_cfg.hMnEnable = 1;
 reg->ltm_cfg.vMnEnable = 1;

 /* downscaler configurations */
 /* input ratio (N)*/
 reg->ltm_h_scale_cfg.h_image_size.hIn = input_width;
 /* output ratio (M), fixed to 320 in spec */
 reg->ltm_h_scale_cfg.h_image_size.hOut = ltm_ds_width;

 reg->ltm_h_scale_cfg.h_phase.hPhaseMult =
   (float)((float)input_width /(float)ltm_ds_width)* h_phase_increment;
 reg->ltm_h_scale_cfg.h_phase.hInterpReso = h_interp;
 reg->ltm_v_scale_cfg.v_image_size.vIn = input_height;
 /* fixed to 240 per spec */
 reg->ltm_v_scale_cfg.v_image_size.vOut = LTM_DS_HEIGHT;
 reg->ltm_v_scale_cfg.v_phase.vPhaseMult =
   (float)((float)input_height/(float)LTM_DS_HEIGHT)* v_phase_increment;
 reg->ltm_v_scale_cfg.v_phase.vInterpReso = v_interp;
 /* TODO defined as 0 <= mninit < M, temporarily set as 0 (same as ref)*/
 reg->ltm_v_scale_cfg.v_stripe.vMnInit = 0;
 reg->ltm_v_scale_cfg.v_stripe.vPhaseInit = 0;

 /* scale pad */
 /* TODO verify this */
 reg->ltm_scale_h_pad_cfg.scaleYInWidth = input_width;
 reg->ltm_scale_h_pad_cfg.hSkipCnt = 0;
 reg->ltm_scale_v_pad_cfg.scaleYInHeight = input_height;
 reg->ltm_scale_v_pad_cfg.vSkipCnt = 0;

 ltm44_scale_reg_update_platform(reg);

 return TRUE;
}

/** ltm44_dcip_config
 *  @isp_sub_module: isp sub module object
 *  @mod: ltm44 module object
 *
 *  configures all of the register variables to be programmed
*/

static boolean ltm44_dcip_config(ltm44_t *mod,
  isp_sub_module_t *isp_sub_module)
{
 isp_ltm_reg_t *reg = NULL;
 int32_t       input_width = 0,
               input_height = 0;

 RETURN_IF_NULL(mod);
 RETURN_IF_NULL(isp_sub_module);

 reg = &(mod->reg);
 input_width = mod->sensor_out_info.request_crop.last_pixel -
   mod->sensor_out_info.request_crop.first_pixel + 1;
 input_height = mod->sensor_out_info.request_crop.last_line -
   mod->sensor_out_info.request_crop.first_line + 1;

  /* IP configurations */
  DC_INIT_CELLNUM(reg) = 0;
  reg->ltm_dcip_cfg.dc_cfg.initDX = 0;
  reg->ltm_dcip_cfg.dc_cfg.binInitCnt = 0;

  IP_INIT_CELLNUM(reg) = 0;
  reg->ltm_dcip_cfg.ip_init.initDX = 0;
  reg->ltm_dcip_cfg.ip_init.initPX = 0;
  reg->ltm_dcip_cfg.ip_cellwidth.cellwidthL =
        (input_width + (LTM_H_CELLS - 2)) /
        (LTM_H_CELLS - 1);

  if (mod->isp_out_info.is_split == TRUE) {
    reg->ltm_dcip_cfg.ip_cellwidth.cellwidthL =
      ((input_width + (LTM_H_CELLS_SPLIT - 2)) /
        (LTM_H_CELLS_SPLIT - 1));
  }
  reg->ltm_dcip_cfg.ip_cellwidth.cellwidthR = 0;
  reg->ltm_dcip_cfg.ip_cellheight.cellheightL =
     (reg->ltm_v_scale_cfg.v_image_size.vIn + (LTM_V_CELLS -2)) /
     (LTM_V_CELLS -1);
  reg->ltm_dcip_cfg.ip_cellheight.cellheightR =
     reg->ltm_dcip_cfg.ip_cellheight.cellheightL;

  /* inverse registers */
  reg->ltm_dcip_cfg.ip_inv_cellwidth.invCellwidthL =
   (uint16_t)((1 << IP_INV_Q) / reg->ltm_dcip_cfg.ip_cellwidth.cellwidthL);
  if (reg->ltm_dcip_cfg.ip_cellwidth.cellwidthR) {
    reg->ltm_dcip_cfg.ip_inv_cellwidth.invCellwidthR =
       (uint16_t)((1 << IP_INV_Q) / reg->ltm_dcip_cfg.ip_cellwidth.cellwidthR);
  } else {
    reg->ltm_dcip_cfg.ip_inv_cellwidth.invCellwidthR =
      reg->ltm_dcip_cfg.ip_inv_cellwidth.invCellwidthL;
  }

  reg->ltm_dcip_cfg.ip_inv_cellheight.invCellheightL =
    (uint16_t)((1 << IP_INV_Q) / reg->ltm_dcip_cfg.ip_cellheight.cellheightL);

  if (reg->ltm_dcip_cfg.ip_cellheight.cellheightR) {
    reg->ltm_dcip_cfg.ip_inv_cellheight.invCellheightR =
      (uint16_t)((1 << IP_INV_Q) / reg->ltm_dcip_cfg.ip_cellheight.cellheightR);
  } else {
    reg->ltm_dcip_cfg.ip_inv_cellheight.invCellheightR =
      reg->ltm_dcip_cfg.ip_inv_cellheight.invCellheightL;
  }
 return TRUE;
}

/** ltm44_default_config
 *  @isp_sub_module: isp sub module object
 *  @mod: ltm44 module object
 *
 *  configures all of the register variables to be programmed
*/
static boolean ltm44_default_config(isp_sub_module_t *isp_sub_module,
  ltm44_t *mod)
{
  boolean ret = TRUE;

  RETURN_IF_NULL(mod);
  RETURN_IF_NULL(isp_sub_module);

  ret = ltm44_general_config(mod, isp_sub_module);
  RETURN_IF_FALSE(ret);

  ret = ltm44_rgb2y_config(mod, isp_sub_module);
  RETURN_IF_FALSE(ret);

  ret = ltm44_scaler_config(mod, isp_sub_module);
  RETURN_IF_FALSE(ret);

  ret = ltm44_dcip_config(mod, isp_sub_module);
  RETURN_IF_FALSE(ret);

  mod->enable_backlight_compensation = TRUE;
  isp_sub_module->submod_trigger_enable = TRUE;

  return TRUE;
}

/** ltm44_handle_set_chromatix_ptr
 *
 *  @mod: mod ctrl handle
 *  @event: set chromatix ptr event to be handled
 *
 *  Store chromatix ptr to be used later for interpolation
 *
 *  Return TRUE on success and FALS on failure
 **/
boolean ltm44_handle_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
 ltm44_t              *mod = NULL;
 modulesChromatix_t   *chromatix_ptrs = NULL;

 ISP_DBG("");
 RETURN_IF_NULL(isp_sub_module);
 RETURN_IF_NULL(event);

  mod = (ltm44_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  chromatix_ptrs = (modulesChromatix_t *)
    event->u.module_event.module_event_data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

 ISP_DBG("chroma ptr %x", (unsigned int)chromatix_ptrs->chromatixPtr);
 PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
 isp_sub_module->chromatix_ptrs = *chromatix_ptrs;

 if (!mod->effect_applied)
  isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
/* Check is there is a request to disable the module */
 if (isp_sub_module->disable_module) {
   isp_sub_module->submod_enable = FALSE;
 }

 isp_sub_module->trigger_update_pending = TRUE;

 PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
 return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;

}

#define START_LUMA       40// (luma-target) based, disable for now
#define END_LUMA         80
#define START_GAIN       1.8f   // total_gain based
#define END_GAIN         16.0f
#define START_ATTENUATE  5.0f  // delta(exp_idx) based
#define END_ATTENUATE    20.0f




/** ltm44_update_dmi_lut:
 *
 *  @isp_sub_module: isp base module
 *  @dmi_bank_offset: dmi RAM bank select
 *  @lut_ptr: LUT data pointer
 *  @lut_size: siz eof data to be written to DMI bank
 *
 *  write dmi data stored at lut_ptr to dmi channel specified
 *
 **/
boolean ltm44_update_dmi_lut(isp_sub_module_t *isp_sub_module,
  uint32_t dmi_bank_offset, void *lut_ptr, uint32_t lut_size)
{
  boolean ret = TRUE;
  ISP_DBG("dmi_bank_offset %d", dmi_bank_offset);
  ret = isp_sub_module_util_write_dmi(
    (void*)lut_ptr, lut_size,
    dmi_bank_offset,
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  GOTO_ERROR_IF_FALSE(ret);

  return ret;
error:
  return FALSE;
}

/** write_lut_to_dmi:
 *
 *  @isp_sub_module: isp base module
 *  @mod: module data
 *
 *  prepares LUT for curves and write to DMI
 *    1. mask curve
 *    2. master curve
 *    3. master scale
 *    4. shift curve
 *    5. shift scale
 *
 **/

boolean write_lut_to_dmi(isp_sub_module_t *isp_sub_module, ltm44_t *mod){

  boolean   ret = FALSE;
  uint32_t  mask_bank_off = LTM_MASK_LUT_BANK0;
  uint32_t  mc_bank_off = LTM_MC_LUT_BANK0;
  uint32_t  ms_bank_off = LTM_MS_LUT_BANK0;
  uint32_t  sc_bank_off = LTM_SC_LUT_BANK0;
  uint32_t  ss_bank_off = LTM_SS_LUT_BANK0;
  uint32_t  weight_bank_off = LTM_WEIGHT_LUT_BANK0;
  uint32_t  lut_size = sizeof(uint32_t) * LTM_HW_LUT_SIZE;

  if (mod->bankSel == 1) {
    mask_bank_off   = LTM_MASK_LUT_BANK1;
    mc_bank_off     = LTM_MC_LUT_BANK1;
    ms_bank_off     = LTM_MS_LUT_BANK1;
    sc_bank_off     = LTM_SC_LUT_BANK1;
    ss_bank_off     = LTM_SS_LUT_BANK1;
    weight_bank_off = LTM_WEIGHT_LUT_BANK1;
  }
  ISP_DBG("DMI bank %d frame num %llu", mod->bankSel, mod->frame_number);

  /* Using fixed curve no need to update every time */
    /* Write mask rect curve */
  ret = ltm44_update_dmi_lut(isp_sub_module, mask_bank_off,
    (void *)&mod->hw_mask_curve[0], lut_size);
  GOTO_ERROR_IF_FALSE(ret);

  /* Write master scale */
  ret = ltm44_update_dmi_lut(isp_sub_module, ms_bank_off,
    (void *)&mod->hw_master_scale[0], lut_size);
  GOTO_ERROR_IF_FALSE(ret);

  /* Write shift curve */
  ret = ltm44_update_dmi_lut(isp_sub_module, sc_bank_off,
    (void *)&mod->hw_shift_curve[0], lut_size);
  GOTO_ERROR_IF_FALSE(ret);

    /* Write  weight*/
    ret = ltm44_update_dmi_lut(isp_sub_module, weight_bank_off,
      (void *)&mod->hw_weight[0], sizeof(uint32_t) * LTM_HW_W_LUT_SIZE);
    GOTO_ERROR_IF_FALSE(ret);

  /* Write master curve */
  ret = ltm44_update_dmi_lut(isp_sub_module, mc_bank_off,
    (void *)&mod->hw_master_curve[0], lut_size);
  GOTO_ERROR_IF_FALSE(ret);

  /* Write shift scale */
  ret = ltm44_update_dmi_lut(isp_sub_module, ss_bank_off,
    (void *)&mod->hw_shift_scale[0], lut_size);
  GOTO_ERROR_IF_FALSE(ret);


error:
  if (ret == FALSE) {
    ISP_ERR(" ERROR: cannot program DMI");
  }
  return FALSE;
}

/** ltm44_store_hw_update:
 *    @ltm: ltm module instance
 *    @isp_sub_module: isp base module
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function checks and sends configuration update to kernel
 *
 *  Return:   0 - Success
 *           -1 - configuration error
 **/
static int ltm44_store_hw_update(isp_sub_module_t *isp_sub_module,
  ltm44_t *mod)
{
  boolean                        ret = TRUE;
  int                            i, rc = 0;
  isp_ltm_reg_t                 *reg_cmd = NULL;
  char                           value[PROPERTY_VALUE_MAX];
  struct msm_vfe_cfg_cmd_list   *hw_update = NULL;
  struct msm_vfe_cfg_cmd2       *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd    *reg_cfg_cmd = NULL;
  uint32_t get_val;

  RETURN_IF_NULL(mod);

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  RETURN_IF_NULL(hw_update);
  memset(hw_update, 0, sizeof(*hw_update));

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd)*7);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_MALLOC;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd)*7);

  reg_cmd = (isp_ltm_reg_t *)CAM_MALLOC(sizeof(*reg_cmd));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));

  property_get("persist.vendor.camera.ltm.noip", value, "0");
  get_val = atoi(value);
  ISP_DBG("no image processing flag %s", value);
  /* First frame does not need image processing. We never get first frame
   * for online ISP because of frame skip, but we will get it in offline ISP */

  if (get_val > 0 || (((!mod->frame_number && mod->skip_ip) ||
    mod->disable_ip) && !mod->is_offline)) {
    ISP_DBG(" disabling IP");
    mod->reg.ltm_cfg.ipEnable = 0;
  } else {
    mod->reg.ltm_cfg.ipEnable = 1;
  }
  /* switching mode */
  property_get("persist.vendor.camera.ltm.sw", value, "0");
  ISP_DBG("switching mode value %s", value);
  get_val = atoi(value);
  if (get_val > 0) {
    mod->reg.ltm_cfg.autoSwitchOverride = 1;
    mod->reg.ltm_cfg.dc3dAvgPongSel = mod->frame_number % 2;
    mod->reg.ltm_cfg.ip3dAvgPongSel =
      (mod->reg.ltm_cfg.dc3dAvgPongSel == 0) ? 1 : 0;
    ISP_HIGH("software override : frame #%lld | dc = %d | ip = %d",
      mod->frame_number, mod->reg.ltm_cfg.dc3dAvgPongSel,
      mod->reg.ltm_cfg.ip3dAvgPongSel);
  } else {
    mod->reg.ltm_cfg.autoSwitchOverride = 0;
  }

  if (LTM_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      LTM_CGC_OVERRIDE_REGISTER, LTM_CGC_OVERRIDE_BIT, TRUE);
    if (ret == FALSE) {
      ISP_ERR("failed: enable cgc");
    }
  }

  ltm44_print_tables(mod);
  write_lut_to_dmi(isp_sub_module, mod);
  mod->reg.ltm_cfg.lutBankSelect = mod->bankSel;
  *reg_cmd = mod->reg;

  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 7;

  /* write LTMCfg register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[0], 0,
    sizeof(reg_cmd->ltm_cfg), VFE_WRITE, ISP_LTM_CFG_OFF);

  /* write LTM_SCALE registers */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[1],
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset + reg_cfg_cmd[0].u.rw_info.len +
    sizeof(mod->reg.ltm_status),
    sizeof(reg_cmd->ltm_h_scale_cfg), VFE_WRITE, ISP_LTM_H_SCALE_OFF);
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[2],
    reg_cfg_cmd[1].u.rw_info.cmd_data_offset + reg_cfg_cmd[1].u.rw_info.len,
    sizeof(reg_cmd->ltm_v_scale_cfg), VFE_WRITE, ISP_LTM_V_SCALE_OFF);

  /* write LTM_DCIP register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[3],
    reg_cfg_cmd[2].u.rw_info.cmd_data_offset + reg_cfg_cmd[2].u.rw_info.len,
    sizeof(reg_cmd->ltm_dcip_cfg), VFE_WRITE, ISP_LTM_DCIP_OFF);

  /* write LTM_RGB2Y registers */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[4],
    reg_cfg_cmd[3].u.rw_info.cmd_data_offset + reg_cfg_cmd[3].u.rw_info.len,
    sizeof(reg_cmd->ltm_rgb2y_cfg), VFE_WRITE, ISP_LTM_RGB2Y_OFF);

  /* write LTM_PAD registers */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[5],
    reg_cfg_cmd[4].u.rw_info.cmd_data_offset + reg_cfg_cmd[4].u.rw_info.len,
    sizeof(reg_cmd->ltm_scale_h_pad_cfg), VFE_WRITE, ISP_LTM_H_PAD_OFF);
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[6],
    reg_cfg_cmd[5].u.rw_info.cmd_data_offset + reg_cfg_cmd[5].u.rw_info.len,
    sizeof(reg_cmd->ltm_scale_v_pad_cfg), VFE_WRITE, ISP_LTM_V_PAD_OFF);

  ltm44_debug(reg_cmd);

#ifdef PRINT_REG_VAL_SET
  isp_sub_module_util_hw_reg_set(cfg_cmd->cfg_data, cfg_cmd->cmd_len);
#endif

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update %d", ret);
    goto ERROR_APPEND;
  }
    /* rotate banks */
  mod->bankSel = (mod->bankSel == 0) ? 1 : 0;
  ISP_DBG("%s: frame id: %lld bank sel: %d",
    mod->isp_out_info.stripe_id? "RIGHT": "LEFT",
    mod->frame_number, mod->bankSel);
  if (mod->frame_number > 0)
    isp_sub_module->trigger_update_pending = FALSE;

  if (LTM_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      LTM_CGC_OVERRIDE_REGISTER, LTM_CGC_OVERRIDE_BIT, FALSE);
    if (ret == FALSE) {
      ISP_ERR("failed: disable cgc");
    }
  }

  return ret;

ERROR_APPEND:
  free(reg_cmd);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_MALLOC:
  free(hw_update);

  return ret;
} /* ltm44_store_hw_update */

/**ltm44_cfg_set_from_chromatix:
 *    @cfg: local la 8k configuration
 *    @chromatix: chromatix header configuration
 *
 * Convert chromatix header tuning for local luma adaptation usage.
 **/
static void ltm44_cfg_set_from_chromatix(isp_la_8k_type *cfg,
  LA_args_type *chromatix)
{
  if (!cfg || !chromatix) {
    ISP_ERR("failed: %p %p", cfg, chromatix);
    return;
  }

  cfg->offset =
    LA_SCALE(chromatix->LA_reduction_fine_tune, 0, 16, 0, 100);
  cfg->low_beam =
    LA_SCALE(chromatix->shadow_boost_fine_tune, 0, 4, 0, 100);
  cfg->high_beam =
    LA_SCALE(chromatix->highlight_suppress_fine_tune, 0, 4, 0, 100);

  /* CDF_50_thr maps in inverse to shadow_boost allowance. It should be 100 when
   * when allowance is min(0) and 70, when allowance is max(70) */
  cfg->CDF_50_thr =
    LA_SCALE(chromatix->shadow_boost_allowance, 100, 70, 0, 100);
  cfg->cap_high =
    LA_SCALE(chromatix->shadow_boost_allowance, 0.05, 0.25, 0, 100);
  cfg->histogram_cap =
    LA_SCALE(chromatix->shadow_boost_allowance, 3, 12, 0, 100);
  cfg->cap_low =
    LA_SCALE(chromatix->shadow_boost_allowance, 1.5, 1.5, 0, 100);

  if (chromatix->shadow_range > 1)
   cfg->cap_adjust = 256.0f / chromatix->shadow_range;
  else
   cfg->cap_adjust = 256.0f;
}

/** ltm44_trigger_update_aec:
 *
 * @mod: LA module
 * @chromatix_ptr: chromatix pointer
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean ltm44_update_8k_params(isp_sub_module_t *isp_sub_module,
  ltm44_t *mod)
{
  boolean                 ret = TRUE;
  isp_la_8k_type          la_8k_config_indoor,
                          la_8k_config_outdoor;
  isp_la_8k_type          la_config_compensated,
                          la_config_backlight;
  chromatix_LA_type      *chromatix_LA = NULL;
  chromatix_parms_type   *chromatix_ptr = NULL;
  ASD_VFE_struct_type     *ASD_algo_data = NULL;

  RETURN_IF_NULL(mod);
  RETURN_IF_NULL(isp_sub_module);

  chromatix_ptr = (chromatix_parms_type *)
    isp_sub_module->chromatix_ptrs.chromatixPtr;

  chromatix_LA = &chromatix_ptr->chromatix_VFE.chromatix_LA;
  ASD_algo_data = &chromatix_ptr->ASD_algo_data;

  ltm44_cfg_set_from_chromatix(&la_8k_config_indoor,
    &chromatix_LA->LA_config);
  ltm44_cfg_set_from_chromatix(&la_8k_config_outdoor,
    &chromatix_LA->LA_config_outdoor);

  /*interpolate algo parm by aec ratio*/
  ISP_DBG("aec ratio = %f\n", mod->trigger_ratio.ratio);
  mod->la_8k_algo_parm.offset = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.offset, la_8k_config_outdoor.offset,
    mod->trigger_ratio.ratio));
  mod->la_8k_algo_parm.low_beam = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.low_beam, la_8k_config_outdoor.low_beam,
    mod->trigger_ratio.ratio));
  mod->la_8k_algo_parm.high_beam = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.high_beam, la_8k_config_outdoor.high_beam,
    mod->trigger_ratio.ratio));
  mod->la_8k_algo_parm.histogram_cap = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.histogram_cap, la_8k_config_outdoor.histogram_cap,
    mod->trigger_ratio.ratio));
  mod->la_8k_algo_parm.cap_high = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.cap_high, la_8k_config_outdoor.cap_high,
    mod->trigger_ratio.ratio));
  mod->la_8k_algo_parm.cap_low = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.cap_low, la_8k_config_outdoor.cap_low,
    mod->trigger_ratio.ratio));

  mod->la_8k_algo_parm.cap_adjust = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.cap_adjust, la_8k_config_outdoor.cap_adjust,
    mod->trigger_ratio.ratio));

  mod->la_8k_algo_parm.CDF_50_thr = (float)(LINEAR_INTERPOLATION(
    la_8k_config_indoor.CDF_50_thr, la_8k_config_outdoor.CDF_50_thr,
    mod->trigger_ratio.ratio));

  ISP_DBG("backlight_scene_severity :%d", mod->backlight_severity);
  if (mod->backlight_severity != 0) {
    la_config_compensated = mod->la_8k_algo_parm;
    ltm44_cfg_set_from_chromatix(&la_config_backlight,
      &ASD_algo_data->backlit_scene_detect.backlight_la_8k_config);

    mod->la_8k_algo_parm.offset = (la_config_compensated.offset * (255 -
      mod->backlight_severity) + la_config_backlight.offset *
      mod->backlight_severity)/255.0;

    mod->la_8k_algo_parm.low_beam = (la_config_compensated.low_beam * (255 -
      mod->backlight_severity) + la_config_backlight.low_beam *
      mod->backlight_severity)/255.0;

    mod->la_8k_algo_parm.high_beam = (la_config_compensated.high_beam * (255
      - mod->backlight_severity) + la_config_backlight.high_beam *
      mod->backlight_severity)/255.0;

    mod->la_8k_algo_parm.histogram_cap =(la_config_compensated.histogram_cap
      * (255 - mod->backlight_severity) + la_config_backlight.histogram_cap
      * mod->backlight_severity)/255.0;

    mod->la_8k_algo_parm.cap_high = (la_config_compensated.cap_high * (255 -
      mod->backlight_severity) + la_config_backlight.cap_high *
      mod->backlight_severity)/255;

    mod->la_8k_algo_parm.cap_low = (la_config_compensated.cap_low * (255 -
      mod->backlight_severity) + la_config_backlight.cap_low *
      mod->backlight_severity)/255.0;

    mod->la_8k_algo_parm.cap_adjust = (la_config_compensated.cap_adjust *
      (255 - mod->backlight_severity) + la_config_backlight.cap_adjust *
      mod->backlight_severity)/255.0;

    mod->la_8k_algo_parm.CDF_50_thr = (la_config_compensated.CDF_50_thr *
      (255 - mod->backlight_severity) +
      la_config_backlight.CDF_50_thr * mod->backlight_severity)
        / 255.0;
  }
  return ret;
}

/** ltm44_disable_module:
 *
 *  @ltm : ltm module handle
 *  @isp_sub_module: isp sub module handle
 *  @disable: enable/disable flag
 *
 *  This function enable/disabled ltm module in special cases on the fly.
 *
 *  Return:  TRUE - Success
 *           - FALSE- Failure
 **/
boolean ltm44_disable_module(ltm44_t *ltm,
  isp_sub_module_t *isp_sub_module, boolean disable)
{
  boolean                   ret = TRUE;
  chromatix_parms_type     *chromatix = NULL;
  chromatix_LTM_type       *chromatix_LTM = NULL;
  uint32_t                  ltm_enable = 0;

  ISP_DBG(" disable %d", disable);
  if (!ltm || !isp_sub_module) {
    ISP_ERR("failed: %p %p", ltm, isp_sub_module);
    return FALSE;
  }

  chromatix =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix) {
    ISP_ERR("failed: chromatix %p", chromatix);
    return FALSE;
  }
  chromatix_LTM =
    &(chromatix->chromatix_post_processing.chromatix_LTM_data);

  ISP_HIGH("LTM chromatix %d Disable event received %d ",
    chromatix_LTM->enable, disable);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  ltm_enable = chromatix_LTM->enable;

  /* TODO: To use idx instead of 0 */
  if (ltm_enable) {
    if (disable) {
      /* Disable LTM only if its already enabled thr chromatix */
      isp_sub_module->submod_enable = FALSE;
      isp_sub_module->update_module_bit = TRUE;
      isp_sub_module->trigger_update_pending = TRUE;
      isp_sub_module->disable_module = TRUE;
    } else {
      /* enable */
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->update_module_bit = TRUE;
      isp_sub_module->trigger_update_pending = TRUE;
      isp_sub_module->disable_module = FALSE;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}


/** ltm44_set_ltm_enable:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function handles event from HAl about tone mapping.
 *
 *  Return:   TRUE - Success
 *           -FALSE- Failure
 **/
boolean ltm44_set_ltm_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                   ret = TRUE;
  ltm44_t                  *ltm = NULL;
  mct_event_control_t      *ctrl_event = NULL;
  mct_event_control_parm_t *param = NULL;
  uint32_t *enable_param_data;
  bool disable = FALSE;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  ltm = (ltm44_t *)isp_sub_module->private_data;
  ctrl_event = &event->u.ctrl_event;

  if (!ltm || !ctrl_event) {
    ISP_ERR("failed: ltm %p ctrl_event %p", ltm, ctrl_event);
    return FALSE;
  }

  param = ctrl_event->control_event_data;
  RETURN_IF_NULL(param);
  enable_param_data = ((uint32_t *)param->parm_data);
  RETURN_IF_NULL(enable_param_data);
  disable = (*enable_param_data)? FALSE : TRUE;

  ISP_DBG("ltm disable %d ", disable);
  ret = ltm44_disable_module(ltm, isp_sub_module, disable);
  return ret;
}

/** ltm44_setup_dark_bright_region_index:
 *
 *  @master_scale: master scale curve
 *  @mod: ltm module data
 *
 *  Find dark bright region index
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean ltm44_setup_dark_bright_region_index(int32_t *master_scale,
  ltm44_t *mod)
{
  LTM_dark_high_reg_state_t state = LTM_REGION_INDEX_INIT;
  int i = 0;
  RETURN_IF_NULL(master_scale);
  RETURN_IF_NULL(mod);

  if (master_scale[0] >= 0) {
    mod->dark_boost_thr_index = LTM_HW_LUT_SIZE;
    mod->highlight_sup_thr_index = LTM_HW_LUT_SIZE;
  }
  else {
    mod->dark_boost_thr_index = 0;
    mod->highlight_sup_thr_index = 0;
  }

  for (i = 0; i < LTM_HW_LUT_SIZE; i++) {
    if(state == LTM_BRIGHT_REGION_FOUND)
      break;
    switch (state){
      case LTM_REGION_INDEX_INIT:
        if(master_scale[i] == 0) {
          mod->dark_boost_thr_index = i;
          state = LTM_DARK_BOOST_INDEX_FOUND;
        }
        break;
      case LTM_DARK_BOOST_INDEX_FOUND:
        if(master_scale[i] != 0) {
          mod->highlight_sup_thr_index = i;
          state = LTM_BRIGHT_REGION_FOUND;
        }
        break;
      default:
        break;
    }
  }

  ISP_DBG("dark_boost_thr=%d, highlight_sup_thr=%d,", mod->dark_boost_thr_index,
    mod->highlight_sup_thr_index);
  return TRUE;

}

/** load_default_tables
 *
 *  load default tables for curves
 */
boolean load_default_tables(isp_sub_module_t *isp_sub_module,
  void *data)
{
  ltm44_t *mod = NULL;
  chromatix_LTM_type   *ltm = NULL;
  chromatix_LTM_type   *ltm_pp = NULL;
  chromatix_LTM_type      *chromatix_LTM_backlit = NULL;
  isp_ltm_reg_t        *reg = NULL;
  int i ,j , idx = LTM_NORMAL_LIGHT;
  float ratio = 0.0f ;
  float asd_max_percent_threshold = 0.0;
  int backlit_detect_enable = 0;
  uint16_t                 wt[54];
  float                    fSigmaXY, fSigmaXY_2;

  RETURN_IF_NULL(data);
  RETURN_IF_NULL(isp_sub_module);
  mod = (ltm44_t *)data;
  reg = &(mod->reg);

  chromatix_parms_type *chromatix = (chromatix_parms_type *)
     isp_sub_module->chromatix_ptrs.chromatixPtr;
  ltm_pp = &(chromatix->chromatix_post_processing.chromatix_LTM_data);

  ASD_VFE_struct_type     *ASD_algo_data = NULL;
  AAA_ASD_struct_type     *ASD_3a_data = NULL;
  chromatix_3a_parms_type *chromatix_3a_ptr = NULL;
  chromatix_3a_ptr = (chromatix_3a_parms_type *)
    isp_sub_module->chromatix_ptrs.chromatix3APtr;
  RETURN_IF_NULL(chromatix_3a_ptr);
  ASD_algo_data = &(chromatix->ASD_algo_data);
  ASD_3a_data = &(chromatix_3a_ptr->ASD_3A_algo_data);
  asd_max_percent_threshold = ASD_3a_data->backlit_scene_detect.max_percent_threshold;
  backlit_detect_enable = ASD_3a_data->backlit_scene_detect.backlight_detection_enable;
  if (ASD_algo_data->backlit_scene_detect.backlit_LTM.enable) {
    chromatix_LTM_backlit = &(ASD_algo_data->backlit_scene_detect.backlit_LTM);
  }
  if (!chromatix_LTM_backlit) {
    ltm = &(chromatix->chromatix_post_processing.chromatix_LTM_data);
  } else {
    ltm = &(ASD_algo_data->backlit_scene_detect.backlit_LTM);
  }

  ISP_DBG ("ltm backlit = %d", backlit_detect_enable);
  if(!backlit_detect_enable){
    for (i = 0, j = 0; i < CHROMATIX_LUT_ENTRY ; i += LUT_SUB_SAMPLE, j++) {
      mod->unpacked_master_scale[j] = Round(
                       (float)ltm_pp->reservedData.master_scale[i]/2.0f);
      mod->unpacked_master_scale_attn[j] = Round(
                       (float)ltm_pp->reservedData.master_scale[i]/2.0f);
      mod->unpacked_shift_scale[j] = ltm_pp->reservedData.shift_scale[i];
      mod->unpacked_shift_scale_attn[j] = ltm_pp->reservedData.shift_scale[i];

      mod->unpacked_hw_mask[j] = Round((float)ltm_pp->reservedData.mask_rect[i]/4.0f);
      mod->unpacked_master_curve[j] = Round((float)ltm_pp->reservedData.master_curve[i]/4.0f);
      mod->unpacked_shift_curve[j] = Round((float)ltm_pp->reservedData.shift_curve[i]/4.0f);
    }
  } else {
     ratio = 1.0f - asd_max_percent_threshold;

     for(i = 0, j = 0; i < CHROMATIX_LUT_ENTRY; i += LUT_SUB_SAMPLE, j++) /* 129 Entries */
     {
       mod->unpacked_master_scale[j] = Round((float)(LINEAR_INTERPOLATION_LTM(
                            (float)ltm_pp->reservedData.master_scale[i],
                            (float)ltm->reservedData.master_scale[i],ratio)/2.0f));
       mod->unpacked_master_scale_attn[j] = Round((float)(LINEAR_INTERPOLATION_LTM(
                           (float)ltm_pp->reservedData.master_scale[i],
                           (float)ltm->reservedData.master_scale[i],ratio)/2.0f));
       mod->unpacked_shift_scale[j] = Round((float) LINEAR_INTERPOLATION_LTM(
                            (float)ltm_pp->reservedData.shift_scale[i],
                            (float)ltm->reservedData.shift_scale[i],ratio));
       mod->unpacked_shift_scale_attn[j] = Round((float) LINEAR_INTERPOLATION_LTM(
                            (float)ltm_pp->reservedData.shift_scale[i],
                            (float)ltm->reservedData.shift_scale[i],ratio));
       mod->unpacked_hw_mask[j] = Round((float)ltm->reservedData.mask_rect[i]/4.0f);
       mod->unpacked_master_curve[j] = Round((float)ltm->reservedData.master_curve[i]/4.0f);
       mod->unpacked_shift_curve[j] = Round((float)ltm->reservedData.shift_curve[i]/4.0f);
    }
  }

  /* write tables to dmi */
  pack_tbl_to_lut_i32(mod->unpacked_hw_mask,
    mod->hw_mask_curve, 1.0f);
  pack_tbl_to_lut_i32(mod->unpacked_master_curve,
    mod->hw_master_curve, 1.0f);
  pack_tbl_to_lut_i32(mod->unpacked_master_scale,
    mod->hw_master_scale, 1.0f);
  pack_tbl_to_lut_i32(mod->unpacked_shift_curve,
    mod->hw_shift_curve, 1.0f);
  pack_tbl_to_lut_i32(mod->unpacked_shift_scale,
    mod->hw_shift_scale, 1.0f);

  if(ltm->reservedData.wt_3x3 == WT_1x1_FROMHEADER ||
     ltm->reservedData.wt_3x3 == WT_3x3_FROMHEADER) {
    memset(mod->weight_curve, 0, sizeof(mod->weight_curve));
    mod->weight_curve[ 0] = ltm->reservedData.nLowTrig0;
    mod->weight_curve[ 9] = ltm->reservedData.nLowTrig1;
    mod->weight_curve[18] = ltm->reservedData.nLowTrig2;
    mod->weight_curve[27] = ltm->reservedData.nLowTrig3;
    mod->weight_curve[36] = ltm->reservedData.nHighTrig0;
    mod->weight_curve[45] = ltm->reservedData.nHighTrig1;

    fSigmaXY = ltm->reservedData.fSigmaXY;
    fSigmaXY_2 = fSigmaXY * 0.85;

    /* 3x3 from cfg */
    if (ltm->reservedData.wt_3x3 == WT_3x3_FROMHEADER) {
        for (i = 0; i < 6; i++) {
            mod->weight_curve[(i * 9) +1] =
              mod->weight_curve[(i * 9) +3] =
              mod->weight_curve[i * 9] * fSigmaXY;

            mod->weight_curve[(i *9) + 4] =
              mod->weight_curve[i * 9] * fSigmaXY_2;
        }
    }
    pack_w_tbl_to_lut(mod->weight_curve, mod->hw_weight);

    /* updated 3x3 */
  } else if ( ltm->reservedData.wt_3x3 == WT_3x3_NEWDEFAULT ) {
    memcpy(mod->weight_curve, weight_new_3x3kernel, 54 * sizeof(uint16_t));
    pack_w_tbl_to_lut(mod->weight_curve, mod->hw_weight);
    /*  updated 1x1 */
  } else if ( ltm->reservedData.wt_3x3 == WT_1x1_NEWDEFAULT ) {
    memcpy(mod->weight_curve, weight_new_1x1kernel, 54 * sizeof(uint16_t));
    pack_w_tbl_to_lut(mod->weight_curve, mod->hw_weight);
  } else if (mod->isp_out_info.is_split) {
    memcpy(mod->hw_weight, weight_default_1x1kernel, (LTM_HW_W_LUT_SIZE * 4));
  } else {
    memcpy(mod->hw_weight, weight_default_3x3kernel, (LTM_HW_W_LUT_SIZE * 4));
  }

  ltm44_setup_dark_bright_region_index(mod->unpacked_master_scale, mod);
  return TRUE;
}

/** ltm44_weaken_curves:
 *
 *  @isp_sub_module: isp_sub_module handle
 *  @data: ltm module data
 *  @ratio: combined ratio(luma + exp based + gain based)
 *  @exp_based_ratio: exp based ratio
 *
 *  Weaken the master and shift curves
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean ltm44_weaken_curves(isp_sub_module_t *isp_sub_module, void *data,
 float ratio)
{
  int32_t    mast_y0, mast_y1, i, j;
  int32_t    shift_y0, shift_y1;
  int16_t    mast_base, mast_diff;
  int16_t    shift_base, shift_diff;
  ltm44_t   *mod = NULL;
  int        temp_master_scale[LTM_HW_LUT_SIZE + 1];
  int        temp_shift_scale[LTM_HW_LUT_SIZE + 1];
  float      scale_factor, ltm_strength, drc_gain_nonlinear;
  int32_t    filter_off      = 0;
  char       value[PROPERTY_VALUE_MAX];

  int        mapped_idx = 0;
  float      dark_str, highlight_str;

  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(data);
  mod = (ltm44_t *)data;

#ifdef __ANDROID__
  property_get("persist.vendor.camera.ltm.filteroff", value, "0");
  filter_off = atoi(value);
#else
  filter_off = 0;
#endif
  /* Filter the exp_based_ratio to avoid flicker */
  if (!filter_off) {
    if (!mod->init_aec_settled) {
      ratio =(float)(TEMPORAL_WEIGHT * ratio +
        (1.0f - TEMPORAL_WEIGHT) * mod->ratio);
      mod->init_aec_settled = mod->aec_update.settled;
      ISP_HIGH("exp_based_ratio %f mod->init_aec_settled  %d", ratio, mod->init_aec_settled );
    }
  }

  ISP_DBG("exp_based_ratio %f mod->ratio %f", ratio, mod->ratio);
  if (!F_EQUAL(ratio, mod->ratio)) {
    ISP_DBG("change in exp_based_ratio %f ", ratio);
  }

  mod->ratio = ratio;

  ltm_strength = ((float)mod->nGlobalToneStrength * ratio) / 256.0f;
  scale_factor = ((float)mod->nGlobalToneContrast * ratio) / 256.0f;
  CLAMP_LTM(scale_factor, 0.0f, 8.0f);

  for (i = 0; i < LTM_HW_LUT_SIZE + 1; i++) {
    temp_shift_scale[i] = mod->unpacked_shift_scale[i] * scale_factor;
    temp_master_scale[i] = mod->unpacked_master_scale[i] * ltm_strength;
  }

  if (mod->enable_adrc) {
    /* mix default master_scale and adrc master_scale between
       ADRC gain 1.0 - 2.0 */

    drc_gain_nonlinear = 1.0 + mod->ltm_adrc_algo.adrc_master_scale[0] / 1024.0f;

    for (i = 0, j = 0; j < LTM_HW_LUT_SIZE + 1; j++, i+=ADRC_LUT_SUB_SAMPLE) {
      mod->ltm_adrc_algo.adrc_unpacked_master_curve[j] =
        Round(LINEAR_INTERPOLATION_LTM((mod->ltm_adrc_algo.adrc_master_curve[i]/4.0f),
         (float)mod->unpacked_master_curve[j],
         mod->ltm_adrc_algo.adrc_ltm_mix_rate));

      temp_master_scale[j] =
        MAX(mod->ltm_adrc_algo.adrc_master_scale[i]/2.0f,
         temp_master_scale[j] * mod->ltm_adrc_algo.adrc_ltm_mix_rate);

      mapped_idx = CLAMP_LTM(j * drc_gain_nonlinear, 0 , LTM_HW_LUT_SIZE);
      mod->ltm_adrc_algo.adrc_unpacked_shift_scale[j] =
        (temp_shift_scale[mapped_idx]);
    }
  }


   /* Update the dark_boost_thr_index and highlight_sup_thr_index based on
     adrc based master scale curve */
    ltm44_setup_dark_bright_region_index(
      temp_master_scale, mod);
    mod->fixed_content_high = CLAMP_LTM(mod->fixed_content_high, 0.0f, 2.0f);
    mod->fixed_content_low = CLAMP_LTM(mod->fixed_content_low, 0.0f, 2.0f);
    dark_str = mod->fixed_content_low;
    highlight_str = mod->fixed_content_high;
    for (i = 0; i < LTM_HW_LUT_SIZE + 1; i++) {
      /* weaken master scale */
      if(i < mod->dark_boost_thr_index) {
        temp_master_scale[i] *= dark_str;
      }
      else if(i >= mod->highlight_sup_thr_index) {
        temp_master_scale[i] *= highlight_str;
      }
    }
    pack_tbl_to_lut_i32(temp_master_scale, mod->hw_master_scale, 1.0f);
  if (mod->enable_adrc) {
    pack_tbl_to_lut_i32(mod->ltm_adrc_algo.adrc_unpacked_shift_scale,
      mod->hw_shift_scale, 1.0f);
    pack_tbl_to_lut_i32(mod->ltm_adrc_algo.adrc_unpacked_master_curve,
      mod->hw_master_curve, 1.0f);
  } else {
     pack_tbl_to_lut_i32(temp_shift_scale, mod->hw_shift_scale, 1.0f);
  }

  return TRUE;
}

/** ltm44_adjust_curve_strength
 *
 *  attenuate strength of tone curves
 */
boolean ltm44_adjust_curve_strength(ltm44_t *mod,
  isp_sub_module_t *isp_sub_module)
{
  boolean    ret = TRUE;
  int        delta_idx;
  float      exp_based_ratio;
  chromatix_parms_type   *chromatix_ptr = NULL;
  chromatix_LTM_type     *chromatix_LTM = NULL;
  float      end_attenuate = 0.0f;
  float      start_attenuate = 0.0f;

  RETURN_IF_NULL(mod);
  RETURN_IF_NULL(isp_sub_module);

  chromatix_ptr = (chromatix_parms_type *)
    isp_sub_module->chromatix_ptrs.chromatixPtr;

  chromatix_LTM  =
    &(chromatix_ptr->chromatix_post_processing.chromatix_LTM_data);

  if (isp_sub_module->submod_trigger_enable == FALSE) {
    ISP_DBG("Special effects enabled skip curve attenuation");
    return FALSE;
  }
  mod->prev_exp_idx = mod->exp_idx;
  mod->exp_idx = mod->aec_update.exp_index;

  if ((mod->prev_exp_idx != 0) && (mod->exp_idx != 0)) {
    /* Exp adjustment based attenuation */
    start_attenuate = mod->nLocalToneStrength;
    end_attenuate = mod->nLocalToneContrast;
    delta_idx = abs(mod->exp_idx - mod->prev_exp_idx);
    delta_idx = MIN(delta_idx, end_attenuate);
    /* output range: 0 to(end-start) */
    delta_idx = MAX(delta_idx, start_attenuate);
    if (end_attenuate == start_attenuate) {
      ISP_ERR("nLocalToneStrength and nLocalToneContrast cannot be same");
      exp_based_ratio = 1.0f;
    } else {
      exp_based_ratio = (float)(end_attenuate - delta_idx) /
        (end_attenuate - start_attenuate);
    }
  } else {
    exp_based_ratio = 1.0f;
  }

  ISP_DBG("ltm debugging: start_attenuate %f - end_attenuate %f",
    start_attenuate, end_attenuate);
  ISP_DBG("nLocalTOneStrength %d nlocaltonecontrast %d", mod->nLocalToneStrength,
    mod->nLocalToneContrast);
  ISP_DBG("ltm debugging: exp_index %d, luma %d, target %d",
    mod->aec_update.exp_index, mod->aec_update.cur_luma,
    mod->aec_update.target_luma);

  if (exp_based_ratio > 1.0f) {
    exp_based_ratio = 1.0f;
  }

  ret = ltm44_weaken_curves(isp_sub_module, mod,
    exp_based_ratio);
  return ret;
}

/** ltm44_set_spl_effect:
 *
 *  @ltm: ltm module private data
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function configures ltm module according special effect set.
 *
 *  Return:   0 - Success
 *           -1 - Parameter size mismatch
 **/
boolean ltm44_set_spl_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                   ret = TRUE;
  boolean                   mod_enable = FALSE;
  uint32_t                  type = 0;
  ltm44_t                  *ltm = NULL;
  mct_event_control_t      *ctrl_event = NULL;
  mct_event_control_parm_t *param = NULL;
  chromatix_parms_type     *chromatix = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  ISP_DBG("LTM Special effects event received");
  chromatix =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix) {
    ISP_ERR("failed: chromatix %p", chromatix);
    return FALSE;
  }

  ltm = (ltm44_t *)isp_sub_module->private_data;
  ctrl_event = &event->u.ctrl_event;

  if (!ltm || !ctrl_event) {
    ISP_ERR("failed: ltm %p ctrl_event %p", ltm, ctrl_event);
    return FALSE;
  }

  param = ctrl_event->control_event_data;
  if (ltm->effects.spl_effect == *(uint32_t *)param->parm_data) {
    ISP_HIGH(": same effect is received");
    return TRUE;
  }
  ltm->effects.spl_effect = *(uint32_t *)param->parm_data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  type = ltm->effects.spl_effect;
  ISP_DBG(": contrast %d effect type %d",  ltm->effects.contrast, type);

  isp_sub_module->submod_trigger_enable = FALSE;
  isp_sub_module->trigger_update_pending = TRUE;
  ISP_DBG(": type %d",  type);
  switch (type) {
  case CAM_EFFECT_MODE_SOLARIZE:
  case CAM_EFFECT_MODE_POSTERIZE:
    ret = ltm44_prepare_effect_curves(isp_sub_module, ltm, type);
    ltm->effect_applied = TRUE;
    break;

  default: {
    ltm->effect_applied = FALSE;
    ltm->effects.spl_effect = CAM_EFFECT_MODE_OFF;
    load_default_tables(isp_sub_module, ltm);
    isp_sub_module->submod_trigger_enable = TRUE;
    }
    break;
  }

  /* Enable or disable module based on special effects */
  mod_enable = ltm44_check_enable(isp_sub_module, ltm);
   if (isp_sub_module->submod_enable != mod_enable) {
    isp_sub_module->submod_enable = mod_enable;
    isp_sub_module->update_module_bit = TRUE;
  }

  ISP_DBG("effect_dbg applied effect type %d",type);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* ltm44_set_spl_effect */



/** ltm44_update_adrc_based_curve:
 *    @ltm_mod: ltm module instance
 *    @trigger_params: module trigger update params
 *    @in_param_size: enable parameter size
 *
 *   If adrc is enabled, update the master scale and master
 *   curve based on adrc algo
 *
 *   Update master_scale, master curve and shift_scale
 *   based on DRC gain
 *
 * Return:   0 - Success
 *           -1 - Parameters size mismatch
 **/
boolean ltm44_update_adrc_based_curve(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  ltm44_t                 *mod = NULL;
  int                     i = 0;
  boolean                 ret = TRUE;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  mod = (ltm44_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  /* Run adrc algo based on the adrc total gain and LTM ratio */
  ret = ltm_update_adrc_ltm_curve(&mod->ltm_adrc_algo, &mod->aec_update,
                                  module, event);
  if (ret == FALSE) {
    ISP_ERR("failed: ltm_update_adrc_ltm_curve");
    /* take default value for master,mask, shift,shift scale curve */
    for (i = 0; i < CURVE_ENTRY_NUM ; i++) {
      mod->ltm_adrc_algo.adrc_master_curve[i] =
        (double)adrc_master_curve_default[i];
      mod->ltm_adrc_algo.adrc_master_scale[i] =
        (double)adrc_master_scale_curve_default[i];
    }
  }

  return TRUE;
}

/** ltm44_trigger_update:
 *    @ltm_mod: ltm module instance
 *    @trigger_params: module trigger update params
 *    @in_param_size: enable parameter size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function checks and initiates triger update of module
 *
 *  Return:   0 - Success
 *           -1 - Parameters size mismatch
 **/
boolean ltm44_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                          ret = TRUE;
  isp_private_event_t             *private_event = NULL;
  isp_sub_module_priv_t           *isp_sub_module_priv = NULL;
  ltm44_t                         *mod = NULL;
  isp_ltm_reg_t                   *p_cmd = NULL;
  isp_sub_module_output_t         *sub_module_output = NULL;
  isp_meta_entry_t                *ltm_dmi_info = NULL;
  uint32_t                        *ltm_dmi_tbl = NULL;
  uint8_t                        module_enable;
  uint32_t                        i = 0, j = 0;

  chromatix_parms_type            *pchromatix;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  pchromatix =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  RETURN_IF_NULL(pchromatix);

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  RETURN_IF_NULL(isp_sub_module_priv);

  mod = (ltm44_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  p_cmd = &mod->reg;

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(private_event);

  sub_module_output = (isp_sub_module_output_t *)private_event->data;
  RETURN_IF_NULL(sub_module_output);
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->manual_ctrls.manual_update &&
    !isp_sub_module->config_pending) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    ret = ltm44_check_enable(isp_sub_module, mod);
    if (ret == FALSE) {
      mod->disable_ip = TRUE;
    } else {
      mod->disable_ip = FALSE;
    }

  }

  if(isp_sub_module->config_pending)
    isp_sub_module->config_pending = FALSE;

  if ((mod->isp_out_info.is_split == TRUE &&
       mod->exceed_split_limits == TRUE)) {
    if (isp_sub_module->submod_enable != FALSE) {
      /* Disable LTM if input width is beyond LTM grids */
      isp_sub_module->submod_enable = FALSE;
      isp_sub_module->update_module_bit = TRUE;
    }
  } else if ((isp_sub_module->trigger_update_pending == TRUE) &&
    (isp_sub_module->submod_trigger_enable == FALSE)) {
    if (isp_sub_module->submod_enable == TRUE) {
      ISP_DBG("No trigger update for ltm: enable = %d",
        isp_sub_module->submod_enable);
      ret = ltm44_store_hw_update(isp_sub_module, mod);
      if (ret == FALSE) {
        ISP_ERR("failed: ltm44_store_hw_update");
      }
    }
  } else if (isp_sub_module->trigger_update_pending == TRUE &&
    (isp_sub_module->submod_trigger_enable == TRUE)) {

    ISP_DBG("submodule_trigger_enable %d, trigger_update_pending %d",
      isp_sub_module->submod_trigger_enable,
      isp_sub_module->trigger_update_pending);

      /*update algo parm*/
#ifndef LTM_ALGO_BASED_CURVES  /*disable algo curve*/
      sub_module_output->algo_params->is_ltm_algo_parm_valid = FALSE;
      sub_module_output->algo_params->is_la_algo_parm_valid = FALSE;
#else
      sub_module_output->algo_params->is_ltm_algo_parm_valid = TRUE;
      sub_module_output->algo_params->is_la_algo_parm_valid = TRUE;
      sub_module_output->algo_params->backlit_ltm_data =
          pchromatix->ASD_algo_data.backlit_scene_detect.backlit_LTM;
      sub_module_output->algo_params->normal_ltm_data =
          pchromatix->chromatix_post_processing.chromatix_LTM_data;
      sub_module_output->algo_params->backlight_scene_severity =
          mod->asd_update.backlight_scene_severity;
      sub_module_output->algo_params->la_8k_algo_parm = mod->la_8k_algo_parm;
      sub_module_output->algo_params->ltm_lut_size = LTM_HW_LUT_SIZE;

      sub_module_output->algo_params->normal_ltm_data =
          pchromatix->chromatix_post_processing.chromatix_LTM_data;
      sub_module_output->algo_params->backlight_scene_severity =
          mod->asd_update.backlight_scene_severity;
      sub_module_output->algo_params->la_8k_algo_parm = mod->la_8k_algo_parm;
      sub_module_output->algo_params->ltm_lut_size = LTM_HW_LUT_SIZE;
#endif
    if(isp_sub_module->hdr_mode != CAM_SENSOR_HDR_STAGGERED) {
        ltm44_interpolate_ltm_region(isp_sub_module, mod);

        /* If adrc is enabled, update the master scale and curve based on
           adrc algo */
        if (mod->enable_adrc) {
          ltm44_update_adrc_based_curve(module,isp_sub_module, event);
        }

        ltm44_adjust_curve_strength(mod, isp_sub_module);
    }

  if (isp_sub_module->chromatix_ptrs.chromatixIotPtr) {
     chromatix_iot_parms_type* p_chromatix_iot =
       (chromatix_iot_parms_type*)isp_sub_module->chromatix_ptrs.chromatixIotPtr;
     sub_module_output->algo_params->autoltm_params =
        p_chromatix_iot->shdr_mode_data.shdr_ltm_params;
    }

    if (isp_sub_module->submod_enable == TRUE) {
      ISP_DBG("No trigger update for ltm: enable = %d",
        isp_sub_module->submod_enable);
      ret = ltm44_store_hw_update(isp_sub_module, mod);
      if (ret == FALSE) {
        ISP_ERR("failed: ltm44_store_hw_update");
      }
    }
  }
  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
  }

FILL_METADATA:
  if (sub_module_output->frame_meta) {
    if (mod->effect_applied == TRUE)
      sub_module_output->frame_meta->special_effect = mod->effects.spl_effect;

    if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_STAGGERED)
      sub_module_output->frame_meta->sensor_hdr = CAM_SENSOR_HDR_STAGGERED;

  }
  if (sub_module_output->metadata_dump_enable == 1) {
    /*fill in DMI info*/
    ltm_dmi_info = &sub_module_output->
      meta_dump_params->meta_entry[ISP_META_LTM_TBL];
    /*Currently filling on master scale curve*/
    ltm_dmi_info->len =
       (5 * sizeof(uint32_t) * LTM_HW_LUT_SIZE) +
       (sizeof(uint32_t) * LTM_HW_W_LUT_SIZE);
    /*dmi type*/
    ltm_dmi_info->dump_type  = ISP_META_LTM_TBL;
    ltm_dmi_info->start_addr = 0;
    sub_module_output->meta_dump_params->frame_meta.num_entry++;

#ifdef DYNAMIC_DMI_ALLOC
    ltm_dmi_tbl =
      (uint32_t *)CAM_MALLOC(ltm_dmi_info->len);
    if (!ltm_dmi_tbl) {
      ISP_ERR("failed: %p", ltm_dmi_tbl);
      ret = FALSE;
      goto end;
    }
#endif
    memcpy(ltm_dmi_info->isp_meta_dump,
      &mod->hw_master_curve[0], sizeof(uint32_t) * LTM_HW_LUT_SIZE);
    memcpy(ltm_dmi_info->isp_meta_dump + (1*sizeof(uint32_t) * LTM_HW_LUT_SIZE),
      &mod->hw_master_scale[0], sizeof(uint32_t) * LTM_HW_LUT_SIZE);
    memcpy(ltm_dmi_info->isp_meta_dump + (2*sizeof(uint32_t) * LTM_HW_LUT_SIZE),
      &mod->hw_shift_scale[0], sizeof(uint32_t) * LTM_HW_LUT_SIZE);
    memcpy(ltm_dmi_info->isp_meta_dump + (3*sizeof(uint32_t) * LTM_HW_LUT_SIZE),
      &mod->hw_shift_curve[0], sizeof(uint32_t) * LTM_HW_LUT_SIZE);
    memcpy(ltm_dmi_info->isp_meta_dump + (4*sizeof(uint32_t) * LTM_HW_LUT_SIZE),
      &mod->hw_mask_curve[0], sizeof(uint32_t) * LTM_HW_LUT_SIZE);
    memcpy(ltm_dmi_info->isp_meta_dump + (5*sizeof(uint32_t) * LTM_HW_LUT_SIZE),
      &mod->hw_weight[0], sizeof(uint32_t) * LTM_HW_W_LUT_SIZE);
    //ltm_dmi_info->hw_dmi_tbl = ltm_dmi_tbl;
    /* current exp idx */
    sub_module_output->meta_dump_params->frame_meta.addlinfo.reserved[0] =
      mod->exp_idx;
    /* previous exp idx */
    sub_module_output->meta_dump_params->frame_meta.addlinfo.reserved[1] =
      mod->prev_exp_idx;

    sub_module_output->meta_dump_params->frame_meta.adrc_info.ltm_ratio =
     mod->aec_update.ltm_ratio;
    sub_module_output->meta_dump_params->frame_meta.adrc_info.exposure_ratio =
     mod->aec_update.hdr_sensitivity_ratio;
    sub_module_output->meta_dump_params->frame_meta.adrc_info.
      exposure_time_ratio = mod->aec_update.hdr_exp_time_ratio;
    sub_module_output->meta_dump_params->frame_meta.adrc_info.reserved_data[0] =
      mod->aec_update.total_drc_gain;
  }

end:
  if (sub_module_output)
    mod->frame_number = sub_module_output->frame_id;
  else
    mod->frame_number++;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}/*ltm44_trigger_update*/

/** ltm44_config:
 *    @ltm: ltm module instance
 *    @in_params: configuration parameters
 *    @size: configuration parameters size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function makes initial configuration of ltm module
 *
 *  Return:   0 - Success
 *           -1 - Parameters size mismatch
 **/
static boolean ltm44_config(isp_sub_module_t *isp_sub_module,
  ltm44_t *ltm, chromatix_parms_type *chromatix_ptr)
{
  boolean                         ret = TRUE;
  isp_ltm_reg_t                  *p_cmd  = &ltm->reg;

  if (!isp_sub_module || !ltm || !chromatix_ptr) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, ltm, chromatix_ptr);
    return FALSE;
  }
  isp_sub_module->trigger_update_pending = TRUE;
  return ret;
} /* ltm44_config */

/** ltm44_set_stream_config:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Store sensor fmt in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean ltm44_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  sensor_out_info_t *sensor_out_info = NULL;
  ltm44_t           *ltm = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  sensor_out_info =
    (sensor_out_info_t *)event->u.module_event.module_event_data;
  if (!sensor_out_info) {
    ISP_ERR("failed: sensor_out_info %p", sensor_out_info);
    return FALSE;
  }

  ltm = (ltm44_t *)isp_sub_module->private_data;
  if (!ltm) {
    ISP_ERR("failed: ltm %p", ltm);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  ltm->sensor_out_info = *sensor_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* ltm44_set_stream_config */


/** ltm44_set_flash_mode:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to cam_flash_mode_t
 *
 *  Handle set flash mode event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean ltm44_set_flash_mode(isp_sub_module_t *isp_sub_module,
  void *data)
{
  ltm44_t         *ltm = NULL;
  cam_flash_mode_t          *flash_mode = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  ltm = (ltm44_t *)isp_sub_module->private_data;
  if (!ltm) {
    ISP_ERR("failed: ltm %p", ltm);
    return FALSE;
  }

  if(ltm->effect_applied != TRUE) {
    flash_mode = (cam_flash_mode_t *)data;
    if (!flash_mode) {
      ISP_ERR("failed: flash_mode %p", flash_mode);
      return FALSE;
    }

    if (*flash_mode > CAM_FLASH_MODE_OFF &&
    *flash_mode < CAM_FLASH_MODE_MAX)
      ltm->disable_ip = TRUE;
    else
      ltm->disable_ip = FALSE;

    isp_sub_module->trigger_update_pending = TRUE;
    ltm->cur_flash_mode = *flash_mode;
  }
  return TRUE;
}

/** ltm44_reset:
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function resets ltm module
 *
 *  Return: None
 **/
boolean ltm44_reset(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  ltm44_t     *mod = NULL;
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mod = (ltm44_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  memset(&mod->reg, 0, sizeof(mod->reg));
#ifdef LTM_ALGO_BASED_CURVES
  memset(&mod->trigger_ratio, 0, sizeof(mod->trigger_ratio));
#endif
  mod->enable_backlight_compensation = FALSE;
  mod->backlight_severity            = 0;

  /* ltm specific */
  mod->bankSel = 0;
  mod->frame_number = 0;
  mod->skip_ip = TRUE;
  return TRUE;
} /* ltm_reset */

/** ltm44_set_stream_config_overwrite:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Store sensor fmt in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean ltm44_set_stream_config_overwrite(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  sensor_out_info_t *sensor_out_info = NULL;
  ltm44_t           *ltm = NULL;
  boolean            ret = TRUE;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  ltm = (ltm44_t *)isp_sub_module->private_data;
  if (!ltm) {
    ISP_ERR("failed: ltm %p", ltm);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  ltm->frame_number = 0;
  ltm->skip_ip = TRUE;

  ltm44_default_config(isp_sub_module, ltm);

  if (ltm->isp_out_info.is_split == TRUE)
    ltm44_split_config(isp_sub_module, ltm);

#ifdef LTM_ALGO_BASED_CURVES
  ltm44_update_3d_gaussian_kernel(isp_sub_module, ltm);
#endif

  ret = load_default_tables(isp_sub_module, ltm);
  ltm44_adjust_curve_strength(ltm, isp_sub_module);

  if (ltm->effect_applied == TRUE) {
    ret = ltm44_prepare_effect_curves(isp_sub_module,
      ltm, ltm->effects.spl_effect);
    if (ret == FALSE) {
      ISP_ERR("failed: ltm47_prepare_effect_curves");
    } else {
      isp_sub_module->submod_trigger_enable = FALSE;
    }
  }


  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* ltm44_set_stream_config_overwrite */

/** ltm44_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration during first
 *  stream ON
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean ltm44_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  ltm44_t                *mod = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;
  mct_stream_info_t     *streaminfo;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  mod = (ltm44_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  streaminfo = (mct_event_control_parm_t *)
    event->u.ctrl_event.control_event_data;

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ISP_DBG("ltm enable = %d", isp_sub_module->submod_enable);

  ret = ltm44_default_config(isp_sub_module, mod);
  if(ret == FALSE) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  if (mod->isp_out_info.is_split == TRUE)
    ltm44_split_config(isp_sub_module, mod);
#ifdef LTM_ALGO_BASED_CURVES
  ltm44_update_3d_gaussian_kernel(isp_sub_module, mod);
#endif
  load_default_tables(isp_sub_module, mod);
  mod->ratio = 0.1f;
  ltm44_adjust_curve_strength(mod, isp_sub_module);


  if (mod->effect_applied == TRUE) {
    ret = ltm44_prepare_effect_curves(isp_sub_module,
      mod, mod->effects.spl_effect);
    if (ret == FALSE) {
      ISP_ERR("failed: ltm44_prepare_effect_curves");
    } else {
      mod->is_offline =
        (streaminfo->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC);
      isp_sub_module->submod_trigger_enable = FALSE;
    }
  }

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* ltm44_streamon */

/** ltm44_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean ltm44_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  ltm44_t *mod = NULL;
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (ltm44_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ltm44_reset(module, isp_sub_module, event);
  isp_sub_module->trigger_update_pending = FALSE;
  mod->effects.spl_effect = CAM_EFFECT_MODE_OFF;
  isp_sub_module->manual_ctrls.tonemap_mode = CAM_TONEMAP_MODE_HIGH_QUALITY;


  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* ltm44_streamoff */

/** ltm_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the ltm module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean ltm44_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  ltm44_t *mod = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }
  ISP_HIGH("");
  mod = (ltm44_t *)CAM_MALLOC(sizeof(ltm44_t));
  if (!mod) {
    ISP_ERR("failed: ltm %p", mod);
    return FALSE;
  }

  memset(mod, 0, sizeof(*mod));
  memset(&mod->reg, 0, sizeof(mod->reg));
#ifdef LTM_ALGO_BASED_CURVES
  memset(&mod->trigger_ratio, 0, sizeof(mod->trigger_ratio));
#endif
  /* ltm specific */
  mod->bankSel = 0;
  mod->frame_number = 0;
  mod->enable_backlight_compensation = FALSE;
  mod->backlight_severity            = 0;
  mod->ratio = 0.1f;
  mod->exp_idx = 210;
  mod->init_aec_settled = FALSE;
  mod->effects.spl_effect = CAM_EFFECT_MODE_OFF;
  mod->ltm_adrc_algo.gamma_table_g = NULL;
  isp_sub_module->manual_ctrls.tonemap_mode = CAM_TONEMAP_MODE_HIGH_QUALITY;
  isp_sub_module->private_data = (void *)mod;
  isp_sub_module->trigger_update_pending = FALSE;
  mod->skip_ip = TRUE;
  FILL_LTM_FUNC_TABLE(mod);
  return TRUE;
}/* ltm_init */

/** ltm44_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void ltm44_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  ltm44_t                 *mod = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

    /* get chroma enhance mod private*/
  mod = (ltm44_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return;
  }

  /* Free Gamma interpolated table */
  if (mod->ltm_adrc_algo.gamma_table_g) {
    free(mod->ltm_adrc_algo.gamma_table_g);
  }

  free(isp_sub_module->private_data);
  return;
} /* ltm44_destroy */

/** ltm44_dump_registers:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void ltm44_dump_registers(ltm44_t *mod) {
  uint32_t *register_data;
  register_data = (uint32_t *)(&mod->reg.ltm_cfg);
  ISP_HIGH(" (%p) LTM_CFG             [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_status);
  ISP_HIGH(" (%p) LTM_STATUS          [%x]", mod, *register_data);

  /* scale registers */
  register_data = (uint32_t *)(&mod->reg.ltm_h_scale_cfg.h_image_size);
  ISP_HIGH(" (%p) LTM_H_IMG_SIZE      [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_h_scale_cfg.h_phase);
  ISP_HIGH(" (%p) LTM_H_PHASE         [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_h_scale_cfg.h_stripe);
  ISP_HIGH(" (%p) LTM_H_STRIPE        [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_v_scale_cfg.v_image_size);
  ISP_HIGH(" (%p) LTM_V_IMG_SIZE      [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_v_scale_cfg.v_phase);
  ISP_HIGH(" (%p) LTM_V_PHASE         [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_v_scale_cfg.v_stripe);
  ISP_HIGH(" (%p) LTM_V_STRIPE        [%x]", mod, *register_data);

  /* dcip registers */
  register_data = (uint32_t *)(&mod->reg.ltm_dcip_cfg.dc_cfg);
  ISP_HIGH(" (%p) LTM_DC_CFG          [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_dcip_cfg.ip_init);
  ISP_HIGH(" (%p) LTM_IP_INIT         [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_dcip_cfg.ip_inv_cellwidth);
  ISP_HIGH(" (%p) LTM_INV_CELLWIDTH   [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_dcip_cfg.ip_inv_cellheight);
  ISP_HIGH(" (%p) LTM_INV_CELLHEIGHT  [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_dcip_cfg.ip_cellwidth);
  ISP_HIGH(" (%p) LTM_CELLWIDTH       [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_dcip_cfg.ip_cellheight);
  ISP_HIGH(" (%p) LTM_CELLHEIGHT      [%x]", mod, *register_data);

  /* rgb2y reigsters */
  register_data = (uint32_t *)(&mod->reg.ltm_rgb2y_cfg.rgb2y_cfg0);
  ISP_HIGH(" (%p) LTM_RGB2Y_CFG0      [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_rgb2y_cfg.rgb2y_cfg1);
  ISP_HIGH(" (%p) LTM_RGB2Y_CFG1      [%x]", mod, *register_data);

  /* scale pad registers */
  register_data = (uint32_t *)(&mod->reg.ltm_scale_h_pad_cfg);
  ISP_HIGH(" (%p) LTM_H_PAD           [%x]", mod, *register_data);
  register_data = (uint32_t *)(&mod->reg.ltm_scale_v_pad_cfg);
  ISP_HIGH(" (%p) LTM_V_PAD           [%x]", mod, *register_data);
}

/** ltm44_dump_lut
 *    @tablename: lut table name
 *    @lut: values
 *  dump LUT to log
 */
static void ltm44_dump_lut(char* tablename, uint32_t *lut, int32_t size){
  int32_t i = 0;
  int32_t mask = (1<<LUT_BASE_BITS)-1;
  for (i = 0; i < size; i+=8) {
    ISP_DBG("%s: [%d] %x %x %x %x %x %x %x %x", tablename, i,
               lut[i] & mask, lut[i+1] & mask,
               lut[i+2] & mask, lut[i+3] & mask,
               lut[i+4] & mask, lut[i+5] & mask,
               lut[i+6] & mask, lut[i+7] & mask);
  }
}

#if !OVERRIDE_FUNC

/** ltm44_find_ltm_region:
*
*  @chromatix_ltm: chromatix_LTM_type
*  @aec_update: aec_update_t
*  @event: mct event
*
*  Find which LTM region it falls into based on the aec gain/
*  lux index
*
* Return TRUE on success and FALSE on failure
**/
static boolean ltm44_find_ltm_region(chromatix_LTM_type *chromatix_ltm,
  aec_update_t *aec_update, LTM_trigger_index_t *ltm_tr_idx)
{
  int       i = 0;
  uint8_t   trigger_index = 0;
  float     aec_reference;
  float     ratio = 0;
  float     start = 0;
  float     end = 0;
  /* 3 region only 2 set trigger points*/
 trigger_point_type trigger_points_ptr[LTM_MAX_LIGHT - 1];

  RETURN_IF_NULL(chromatix_ltm);
  RETURN_IF_NULL(aec_update);
  RETURN_IF_NULL(ltm_tr_idx);

 if (chromatix_ltm->control_LTM == 0) {
    /* lux index based */
    aec_reference = aec_update->lux_idx;
  } else {
    /* Gain based */
    aec_reference = aec_update->real_gain;
  }
  /*|-Bright-|-Bright_Normal-|-Normal-|-Normal_Low-|-Low-|*/
  trigger_points_ptr[LTM_BRIGHT_LIGHT] = chromatix_ltm->LTM_bright_light_trigger;
  trigger_points_ptr[LTM_NORMAL_LIGHT] = chromatix_ltm->LTM_low_light_trigger;

  for (i = 0; i < LTM_MAX_LIGHT - 1; i++) {
    trigger_point_type *ltm_trigger = &trigger_points_ptr[i];
    trigger_index = i;

    if (chromatix_ltm->control_LTM == 0) {
      start = ltm_trigger->lux_index_start;
      end   = ltm_trigger->lux_index_end;
    } else {
      start = ltm_trigger->gain_start;
      end   = ltm_trigger->gain_end;
      ISP_DBG("gain_start :%f", start);
      ISP_DBG("gain_end :%f", end);
    }

    /* index is within interpolation range, find ratio */
     if (aec_reference >= start && aec_reference < end) {
       ratio = (aec_reference - start) / (end - start);
       ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
         ratio);
       break;
     }

     /* already scanned past the lux index */
     if (aec_reference < end) {
           break;
     }
  }
  if (i == LTM_MAX_LIGHT - 1) {
    /* falls within region 3 LTM_LOW_LIGHT but we do not use trigger points in the region */
    ratio = 0;
  }

  /* LTM trigger region index */
  ltm_tr_idx->tr_index = i;
  ltm_tr_idx->ratio = ratio;
  ltm_tr_idx->ltm_region_enable = chromatix_ltm->enable;

  ISP_DBG(" start gain: %f end gain %f enable %u", start, end,
    ltm_tr_idx->ltm_region_enable);

  return TRUE;
}

static ltm_ext_override_func ltm_override_func = {
  .find_rgn = ltm44_find_ltm_region
};

/** ltm44_fill_func_table:
 *
 *  @ltm: ltm module data
 *
 *  Initialize with the override functions
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean ltm44_fill_func_table(ltm44_t *ltm)
{
  ltm->ext_func_table = &ltm_override_func;
  return TRUE;
} /* bpc44_fill_func_table */

#endif
