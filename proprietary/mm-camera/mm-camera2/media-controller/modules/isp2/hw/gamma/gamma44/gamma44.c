/* gamma44.c
 *
 * Copyright (c) 2012-2017,2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

/* isp headers */
#include "module_gamma.h"
#include "isp_sub_module_util.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"
#include "gamma44.h"

#if OVERRIDE_FUNC
#include "gamma44_ext.h"
#endif
#define GAMMA_CHROMATIX_TBL_SIZE 14
#define ROUND(a)((a >= 0) ? (long)(a + 0.5) : (long)(a - 0.5))

/* Forward declaration */
boolean gamma44_init_config(isp_sub_module_t *isp_sub_module,
  gamma44_t *gamma);
boolean gamma44_store_hw_update(isp_sub_module_t *isp_sub_module,
  gamma44_t *gamma);

/** gamma40_get_interpolated_table:
 *    @module: mct module instance
 *    @isp_sub_module: base module
 *    @event: mct event
 *
 *  This function return interpolated gamma table
 *
 *  Return:   TRUE - Success
 *            FALSE - error
 **/
boolean gamma44_get_interpolated_table(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event) {
  boolean                       ret = TRUE;
  gamma44_t                    *gamma = NULL;
  uint8_t                      *gamma_table_g = NULL;
  isp_private_event_t          *private_event = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  gamma_table_g = (uint8_t *)private_event->data;
  if (!gamma_table_g) {
    ISP_ERR("failed: gamma_table_rgb ");
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  gamma = (gamma44_t *)isp_sub_module->private_data;
  if (!gamma) {
    ISP_ERR("failed: gamma %p", gamma);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  /* Copy interpolated gamma table*/
  memcpy(gamma_table_g, &gamma->gamma_table_rgb.gamma_g,
         ISP_GAMMA_NUM_ENTRIES * sizeof(float));


  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** gamma44_get_vfe_diag_info_user:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function populates vfe diag data
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean gamma44_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
    if (!module || !isp_sub_module || !event) {
        ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
        return FALSE;
    }
    ISP_ERR("TODO: not implemented yet");
  return TRUE;
}

/**   gamma44_chromatix_to_isp_gamma
 *
 *  @p_gamma_rgb: isp gamma rgb table
 *  @chromatix_gamma_table: chromoatix gamma table
 *
 *  This function initializes gamma table from chromatix
 *  Only for 1 channel - G if same channel flag is set
 *
 *  Return: None
 */
void gamma44_chromatix_to_isp_gamma(gamma44_t *gamma, gamma_rgb_t *gamma_rgb,
  chromatix_gamma_table_type *chromatix_gamma_table)
{
  int           i = 0;

  if (!gamma_rgb || !chromatix_gamma_table || !gamma) {
    ISP_ERR("failed: %p %p %p", gamma_rgb, chromatix_gamma_table, gamma);
    return;
  }

  if (!gamma->enable_same_data_channel) {
    for (i = 0; i < GAMMA_CHROMATIX_LUT_SIZE; i++) {
      gamma_rgb->gamma_r[i] = (float)(chromatix_gamma_table->gamma_R[i]);
      gamma_rgb->gamma_g[i] = (float)(chromatix_gamma_table->gamma_G[i]);
      gamma_rgb->gamma_b[i] = (float)(chromatix_gamma_table->gamma_B[i]);
    }
  }
  else {
    for (i = 0; i < GAMMA_CHROMATIX_LUT_SIZE; i++) {
      gamma_rgb->gamma_g[i] = (float)(chromatix_gamma_table->gamma_G[i]);
    }
  }
}

/** gamma44_interpolate
 *
 *    @tbl1: input table 1
 *    @tbl2: input table 2
 *    @out: out table
 *    @ratio:
 *
 *  Interpolate for each of tree colors RGB
 *  and summurise result in one table
 *  Interpolate for G color only if same channel flag is set
 *
 *  Return: None
 */
void gamma44_interpolate(gamma44_t *gamma, gamma_rgb_ds_t  *tbl1,
    gamma_rgb_ds_t  *tbl2, gamma_rgb_ds_t  *gamma_rgb,
  float ratio)
{
  int   i = 0;
  if (!tbl1 || !tbl2 || !gamma_rgb || !gamma) {
    ISP_ERR("failed: %p %p %p", tbl1, tbl2, gamma_rgb);
    return;
  }

  if (!gamma->enable_same_data_channel) {
    TBL_INTERPOLATE_GAMMA(tbl1->gamma_r, tbl2->gamma_r, gamma_rgb->gamma_r,
      ratio, ISP_GAMMA_NUM_ENTRIES, i);
    TBL_INTERPOLATE_GAMMA(tbl1->gamma_g, tbl2->gamma_g, gamma_rgb->gamma_g,
      ratio, ISP_GAMMA_NUM_ENTRIES, i);
    TBL_INTERPOLATE_GAMMA(tbl1->gamma_b, tbl2->gamma_b, gamma_rgb->gamma_b,
      ratio, ISP_GAMMA_NUM_ENTRIES, i);
  }
  else {
    TBL_INTERPOLATE_GAMMA(tbl1->gamma_g, tbl2->gamma_g, gamma_rgb->gamma_g,
      ratio, ISP_GAMMA_NUM_ENTRIES, i);
  }
}

/** gamma44_copy_gamma
 *
 *  @dst: destination
 *  @src: src
 *  @size: size
 *
 *  This function copies rgb gamma table
 *  Only for 1 channel - G if same channel flag is set
 *
 *  Return: None
 */
void gamma44_copy_gamma(gamma44_t *gamma, gamma_rgb_ds_t *dst,
  gamma_rgb_ds_t *src, size_t size)
{
  boolean enable_same_data_channel = FALSE;
  if (!dst || !src || !gamma) {
    ISP_ERR("failed: %p %p", dst, src);
    return;
  }
  enable_same_data_channel = gamma->enable_same_data_channel;

  if (!enable_same_data_channel) {
    memcpy(dst->gamma_r, src->gamma_r, size * sizeof(float));
    memcpy(dst->gamma_g, src->gamma_g, size * sizeof(float));
    memcpy(dst->gamma_b, src->gamma_b, size * sizeof(float));
  }
  else {
    memcpy(dst->gamma_g, src->gamma_g, size * sizeof(float));
  }
}

/** gamma44_compute_contrast_table:
 *
 *    @sigmoid: sigmoid is curvature strength to be applied. This values depends
 *                  on contrast value from user.
 *    @lut_size: look up table size
 *    @input_table: input rgb table
 *    @output_table: output rgb table
 *
 *  This function updates gamma table depending on contrast
 *  Only for 1 channel - G if same channel flag is set
 *
 *  Return: TRUE  - Success
 *          FALSE - Input/Output table is NULL or input values are invalid
 **/
boolean gamma44_compute_contrast_table(double sigmoid, int lut_size,
  gamma_rgb_ds_t  *input_table_rgb, gamma_rgb_ds_t  *output_table_rgb,
  boolean enable_same_data_channel)
{
  int             i = 0, x = 0;
  uint16_t       *gamma_table = NULL;
  int             entries = (lut_size >= 0) ? lut_size : ISP_GAMMA_NUM_ENTRIES;
  int             mid_pt = 0;
  if (input_table_rgb == NULL || output_table_rgb == NULL) {
    ISP_ERR(": input improper");
    return FALSE;
  }

  /* Generate Sigmoid Gamma Table entries*/
  mid_pt = isp_sub_module_util_power(2.0, GAMMA_HW_PACK_BIT) / 2;
  const double multiplier =
    isp_sub_module_util_power((double)mid_pt, (1.0 - sigmoid));

  ISP_DBG("sigmoid %f mid_pt %d multiplier %f entries %d",
    sigmoid, mid_pt, multiplier, entries);
  if (!enable_same_data_channel) {
    for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
      output_table_rgb->gamma_r[i] = gamma44_apply_contrast_sigmoid(sigmoid,
        multiplier, input_table_rgb->gamma_r[i], mid_pt);
      output_table_rgb->gamma_g[i] = gamma44_apply_contrast_sigmoid(sigmoid,
        multiplier, input_table_rgb->gamma_g[i], mid_pt);
      output_table_rgb->gamma_b[i] = gamma44_apply_contrast_sigmoid(sigmoid,
        multiplier, input_table_rgb->gamma_b[i], mid_pt);
      /* 8994 has 512 entries with output 14bits.
       * 8084 has 256 gamma entries with output 12bits, SW needs
       * to downsample gamma by 2 and right shift 2 bits of the table values
       */
      ISP_DBG("%d: input %d output %d", i, input_table_rgb->gamma_r[i],
        output_table_rgb->gamma_r[i]);
    }
  }
  else {
    for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
      output_table_rgb->gamma_g[i] = gamma44_apply_contrast_sigmoid(sigmoid,
        multiplier, input_table_rgb->gamma_g[i], mid_pt);
      /* 8994 has 512 entries with output 14bits.
       * 8084 has 256 gamma entries with output 12bits, SW needs
       * to downsample gamma by 2 and right shift 2 bits of the table values
       */
      ISP_DBG("%d: input %d output %d", i, input_table_rgb->gamma_g[i],
        output_table_rgb->gamma_g[i]);
    }
  }

  return TRUE;
}

/** gamma44_update_vfe_table:
 *    @gamma_config_cmd: configuration command
 *    @table_rgb: pointer to table
 *    @size: size of the table
 *
 *  This function updates gamma table depending on contrast
 *  Only for 1 channel - G if same channel flag is set
 *
 *  Return: TRUE  - Success
 *             FALSE - Input/Output table is NULL or contrast value is invalid
 **/

void gamma44_update_vfe_table(gamma44_t *gamma,
  volatile ISP_GammaConfigCmdType *gamma_config_cmd,
  gamma_rgb_ds_t *table_rgb)
{
  int             i = 0;
  int             skipRatio = 0;
  boolean         enable_same_data_channel = FALSE;

  if (!table_rgb || !gamma) {
    ISP_ERR("failed: table_rgb %p gamma %p", table_rgb, gamma);
    return;
  }
  float *table_g = &table_rgb->gamma_g[0];
  float *table_r = &table_rgb->gamma_r[0];
  float *table_b = &table_rgb->gamma_b[0];

  enable_same_data_channel = gamma->enable_same_data_channel;

  //Finally compute the LUT entries
  if (!enable_same_data_channel) {
    for (i = 0; i < ISP_GAMMA_NUM_ENTRIES - 1; i++) {
      gamma_config_cmd->Gamatbl.hw_table_r[i] =
        gamma44_get_hi_lo_gamma_bits(table_r, i);
      gamma_config_cmd->Gamatbl.hw_table_g[i] =
        gamma44_get_hi_lo_gamma_bits(table_g, i);
      gamma_config_cmd->Gamatbl.hw_table_b[i] =
        gamma44_get_hi_lo_gamma_bits(table_b, i);
    }

    gamma_config_cmd->Gamatbl.hw_table_r[ISP_GAMMA_NUM_ENTRIES - 1] =
      gamma44_get_last_gamma_value(table_r);
    gamma_config_cmd->Gamatbl.hw_table_g[ISP_GAMMA_NUM_ENTRIES - 1] =
      gamma44_get_last_gamma_value(table_g);
    gamma_config_cmd->Gamatbl.hw_table_b[ISP_GAMMA_NUM_ENTRIES - 1] =
      gamma44_get_last_gamma_value(table_b);
  }
  else {
    for (i = 0; i < ISP_GAMMA_NUM_ENTRIES - 1; i++) {
      gamma_config_cmd->Gamatbl.hw_table_g[i] =
        gamma44_get_hi_lo_gamma_bits(table_g, i);
    }

    gamma_config_cmd->Gamatbl.hw_table_g[ISP_GAMMA_NUM_ENTRIES - 1] =
      gamma44_get_last_gamma_value(table_g);
  }
} /* gamma44_update_vfe_table */

/** gamma44_dmi_hw_update:
 *    @gamma: gamma module
 *    @bank_sel: mem bank select
 *    @isp_sub_module: base module
 *
 *  This function updates DMI tables for gamma R, G and B
 *  If same channel flag is, only G channel data is used
 *
 *  Return:   TRUE - Success FALSE- failure
 **/
boolean gamma44_dmi_hw_update(void *data1,
  ISP_GammaLutSelect bank_sel, void *data2)
{
  gamma44_t *gamma = (gamma44_t *)data1;
  isp_sub_module_t *isp_sub_module = (isp_sub_module_t *)data2;
  uint32_t tbl_len = sizeof(int32_t) * ISP_GAMMA_NUM_ENTRIES;
  boolean  ret, enable_same_data_channel;

  if (!gamma || !isp_sub_module) {
    ISP_ERR("failed: %p %p", gamma, isp_sub_module);
    return FALSE;
  }

  enable_same_data_channel = gamma->enable_same_data_channel;
  ret = isp_sub_module_util_write_dmi(
    (void*)gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_g, tbl_len,
    RGBLUT_RAM_CH0_BANK0 + bank_sel.ch0BankSelect,
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    return FALSE;
  }

  if (!enable_same_data_channel) {
    ret = isp_sub_module_util_write_dmi(
      (void*)gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_b, tbl_len,
      RGBLUT_RAM_CH1_BANK0 + bank_sel.ch1BankSelect,
      VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
    if (ret == FALSE) {
      ISP_ERR("failed writing table_b");
      return FALSE;
    }

    ret = isp_sub_module_util_write_dmi(
      (void*)gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_r, tbl_len,
      RGBLUT_RAM_CH2_BANK0 + bank_sel.ch2BankSelect,
      VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
    if (ret == FALSE) {
      ISP_ERR("failed writing table_r");
      return FALSE;
    }
  }
  else {
    ret = isp_sub_module_util_write_dmi(
      (void*)gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_g, tbl_len,
      RGBLUT_RAM_CH1_BANK0 + bank_sel.ch1BankSelect,
      VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
    if (ret == FALSE) {
      ISP_ERR("failed writing table_b");
      return FALSE;
    }

    ret = isp_sub_module_util_write_dmi(
      (void*)gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_g, tbl_len,
      RGBLUT_RAM_CH2_BANK0 + bank_sel.ch2BankSelect,
      VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
    if (ret == FALSE) {
      ISP_ERR("failed writing table_r");
      return FALSE;
    }
  }

  return TRUE;
} /* gamma44_dmi_hw_update */

/** gamma44_get_downsampled_tables:
 *
 *  @tbl_in: table to be downsampled. If NULL
 *           use the rgb_in table
 *  @rgb_in: table with all R,G,B entries. Use
 *           only the G table here.If NULL use
 *           the tbl_in
 *  @skip_ratio: number of entries to be skipped
 *
 *  This function populates gamma G values from
 *  the chromatix table based on skip ratio.
 **/
void gamma44_get_downsampled_tables(gamma44_t *gamma, gamma_rgb_ds_t *rgb_tbl,
  uint16_t *tbl_in, gamma_rgb_t *rgb_in, int skip_ratio)
{
  if ((!tbl_in && !rgb_in) || !gamma) {
    ISP_ERR("Something wrong. Both pointers are NULL gamma %p", gamma);
    return;
  }

  if (tbl_in) {
    if (!gamma->enable_same_data_channel) {
      gamma44_get_downsampled_table_uint16(&rgb_tbl->gamma_r[0],
        tbl_in, skip_ratio);
      gamma44_get_downsampled_table_uint16(&rgb_tbl->gamma_g[0],
        tbl_in, skip_ratio);
      gamma44_get_downsampled_table_uint16(&rgb_tbl->gamma_b[0],
        tbl_in, skip_ratio);
    }
    else {
      gamma44_get_downsampled_table_uint16(&rgb_tbl->gamma_g[0],
        tbl_in, skip_ratio);
    }
  } else {
    if (!gamma->enable_same_data_channel) {
      gamma44_get_downsampled_table_float(&rgb_tbl->gamma_r[0],
        &rgb_in->gamma_r[0], skip_ratio);
      gamma44_get_downsampled_table_float(&rgb_tbl->gamma_g[0],
        &rgb_in->gamma_g[0], skip_ratio);
      gamma44_get_downsampled_table_float(&rgb_tbl->gamma_b[0],
        &rgb_in->gamma_b[0], skip_ratio);
    }
    else {
      gamma44_get_downsampled_table_float(&rgb_tbl->gamma_g[0],
        &rgb_in->gamma_g[0], skip_ratio);
    }
  }
}

/** gamma44_update_tone_map_curves:
 *
 *  @data1: meta data
 *  @data2: gamma info
 *  @val: tone map mode
 *
 *  This function populates gamma curves information in the meta
 *  data.
 **/
void gamma44_update_tone_map_curves(void * data1, void * data2, int val)
{
  isp_per_frame_meta_t *out_frame_meta = (isp_per_frame_meta_t *)data1;
  gamma44_t *gamma = (gamma44_t *)data2;
  cam_tonemap_mode_t tonemap_mode = (cam_tonemap_mode_t)val;
  int i = 0;
  boolean enable_same_data_channel = FALSE;

  out_frame_meta->bestshot_mode = gamma->bestshot_mode;
  if (gamma->effect_applied == TRUE)
    out_frame_meta->special_effect = gamma->effects.spl_effect;
  out_frame_meta->contrast = gamma->effects.contrast;

  out_frame_meta->tonemap_mode = tonemap_mode;

  enable_same_data_channel = gamma->enable_same_data_channel;
  //Populate CAM_INTF_META_TONEMAP_CURVES
  out_frame_meta->tone_map.tonemap_points_cnt = ISP_GAMMA_NUM_ENTRIES;
  /* ch0 = G, ch 1 = B, ch 2 = R*/
  for (i=0; i<ISP_GAMMA_NUM_ENTRIES; i++) {
    out_frame_meta->tone_map.curves[0].tonemap_points[i][0] =
      (float)i / (ISP_GAMMA_NUM_ENTRIES - 1);
    out_frame_meta->tone_map.curves[0].tonemap_points[i][1] =
      (float)(gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_g[i] &
              GAMMA_HW_UNPACK_MASK) / MAX_GAMMA_VALUE;

    if (!enable_same_data_channel) {
      out_frame_meta->tone_map.curves[1].tonemap_points[i][0] =
        (float)i / (ISP_GAMMA_NUM_ENTRIES - 1);
      out_frame_meta->tone_map.curves[1].tonemap_points[i][1] =
        (float)(gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_b[i] &
                GAMMA_HW_UNPACK_MASK) / MAX_GAMMA_VALUE;

      out_frame_meta->tone_map.curves[2].tonemap_points[i][0] =
        (float)i / (ISP_GAMMA_NUM_ENTRIES - 1);
      out_frame_meta->tone_map.curves[2].tonemap_points[i][1] =
        (float)(gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_r[i] &
                GAMMA_HW_UNPACK_MASK) / MAX_GAMMA_VALUE;
    }
    /* profile_tone_curve */
    out_frame_meta->tone_curve.curve.tonemap_points[i][0] =
      (float)i / (ISP_GAMMA_NUM_ENTRIES - 1);
    out_frame_meta->tone_curve.curve.tonemap_points[i][1] =
      (float)(gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_g[i] &
              GAMMA_HW_UNPACK_MASK) / MAX_GAMMA_VALUE;
  }

  if (enable_same_data_channel) {
    /* Copy G values into R and B */
    memcpy(&out_frame_meta->tone_map.curves[1], &out_frame_meta->tone_map.curves[0],
            sizeof(cam_tonemap_curve_t));
    memcpy(&out_frame_meta->tone_map.curves[2], &out_frame_meta->tone_map.curves[0],
            sizeof(cam_tonemap_curve_t));
  }
}

/** gamma44_set_solarize_table:
 *
 *    @gamma: gamma module instance
 *    @chromatix_ptr: pointer to chromatix
 *
 *  This function sets solarize table
 *
 *  Return:   None
 **/
void gamma44_set_solarize_table(gamma44_t *gamma,
  chromatix_parms_type *chromatix_ptr)
{
  /* FIXME: There is no solorize reflection point in chromatix 301 */
  int                   i = 0;
  uint16_t              solarize_reflection_point = 32;
  uint16_t              gamma_max_value = 0x3FFF;
  gamma_rgb_ds_t           Gamma_Lut;
  uint16_t reflect_gam_value_R = 1, reflect_gam_value_G = 1, reflect_gam_value_B = 1;

  if (!gamma || !chromatix_ptr) {
    ISP_ERR("failed: gamma %p ctrl_event %p", gamma, chromatix_ptr);
    return;
  }

  /* Chromatix reflection point is based on 256 (8 bit) gamma entries,
   * whereas VFE gamma has 1024 (10 bit) entries. So we multiple the
   * reflection point by 4 here */
  gamma44_copy_gamma(gamma, &Gamma_Lut, &gamma->default_gamma_tbl,
    ISP_GAMMA_NUM_ENTRIES);

  reflect_gam_value_G = (gamma_max_value - Gamma_Lut.gamma_g[solarize_reflection_point - 1]);
  if(!reflect_gam_value_G) reflect_gam_value_G = 1;

  /* Use G channel for R and B if the flag is enabled */
  if (!gamma->enable_same_data_channel) {
    reflect_gam_value_R = (gamma_max_value - Gamma_Lut.gamma_r[solarize_reflection_point - 1]);
    reflect_gam_value_B = (gamma_max_value - Gamma_Lut.gamma_b[solarize_reflection_point - 1]);
    if(!reflect_gam_value_R) reflect_gam_value_R = 1;
    if(!reflect_gam_value_B) reflect_gam_value_B = 1;
  }

  for (i = solarize_reflection_point - 1; i < ISP_GAMMA_NUM_ENTRIES; ++i) {
    gamma->solarize_gamma_table.gamma_g[i] =
      ((gamma_max_value - Gamma_Lut.gamma_g[i]) *
        Gamma_Lut.gamma_g[solarize_reflection_point - 1] /
        reflect_gam_value_G);

    /* Use G channel for R and B if the flag is enabled */
    if (!gamma->enable_same_data_channel) {
      gamma->solarize_gamma_table.gamma_r[i] =
        ((gamma_max_value - Gamma_Lut.gamma_r[i]) *
          Gamma_Lut.gamma_r[solarize_reflection_point - 1] /
          reflect_gam_value_R);

      gamma->solarize_gamma_table.gamma_b[i] =
        ((gamma_max_value - Gamma_Lut.gamma_b[i]) *
          Gamma_Lut.gamma_b[solarize_reflection_point - 1] /
          reflect_gam_value_B);
    }
  }
} /* gamma44_set_solarize_table */

void gamma44_send_meta_data(
  isp_meta_entry_t *gamma_dmi_info,
  gamma44_t *gamma)
{
  if (gamma->enable_same_data_channel){
    memcpy(&gamma_dmi_info->isp_meta_dump[0],
      &gamma->applied_table.hw_table_g[0],
      sizeof(uint32_t) * ISP_GAMMA_NUM_ENTRIES);
    memcpy(&gamma_dmi_info->isp_meta_dump[
      ISP_GAMMA_NUM_ENTRIES * sizeof(uint32_t)],
      &gamma->applied_table.hw_table_g[0],
      sizeof(uint32_t) * ISP_GAMMA_NUM_ENTRIES);
    memcpy(&gamma_dmi_info->isp_meta_dump[
      ISP_GAMMA_NUM_ENTRIES * 2 * sizeof(uint32_t)],
      &gamma->applied_table.hw_table_g[0],
      sizeof(uint32_t) * ISP_GAMMA_NUM_ENTRIES);
  } else {
    memcpy(&gamma_dmi_info->isp_meta_dump[0],
      &gamma->applied_table.hw_table_r[0],
      sizeof(uint32_t) * ISP_GAMMA_NUM_ENTRIES);
    memcpy(&gamma_dmi_info->isp_meta_dump[
      ISP_GAMMA_NUM_ENTRIES * sizeof(uint32_t)],
      &gamma->applied_table.hw_table_g[0],
      sizeof(uint32_t) * ISP_GAMMA_NUM_ENTRIES);
    memcpy(&gamma_dmi_info->isp_meta_dump[
      ISP_GAMMA_NUM_ENTRIES * 2 * sizeof(uint32_t)],
      &gamma->applied_table.hw_table_b[0],
      sizeof(uint32_t) * ISP_GAMMA_NUM_ENTRIES);
  }
}

/** gamma44_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration using default values from
 *  chromatix ptr
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean gamma44_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  gamma44_t             *gamma = NULL;
  modulesChromatix_t    *chromatix_ptrs = NULL;
  chromatix_parms_type  *chromatix_ptr = NULL;

  ISP_DBG("");
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  chromatix_ptrs =
    (modulesChromatix_t *)event->u.module_event.module_event_data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;
  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }

  gamma = (gamma44_t *)isp_sub_module->private_data;
  if (!gamma) {
    ISP_ERR("failed: mod %p", gamma);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  /* Update streaming mode mask in module private */
/*  mct_list_traverse(isp_sub_module->l_stream_info,
  isp_sub_module_util_update_streaming_mode, &gamma->streaming_mode_mask);*/

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("gamma enable = %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ret = gamma44_set_same_channel_data_flag(isp_sub_module);

  if (ret == FALSE) {
    ISP_ERR("failed: enable_same_channel_data for gamma");
  }

  ret = gamma44_init_config(isp_sub_module, gamma);

  if (ret == FALSE) {
    ISP_ERR("failed: init_config for module %d",
      isp_sub_module->hw_module_id);
  }


  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}
/** gamma44_stats_aec_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean gamma44_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret = TRUE;
  stats_update_t          *stats_update = NULL;
  aec_manual_update_t     *manual_stats_update  = NULL;
  stats_update_t           new_aec_stats;
  aec_update_t            *cur_aec_update = NULL;
  aec_update_t            *new_aec_update = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_gamma_type    *chromatix_gamma = NULL;
  trigger_point_type      *lowlight = NULL;
  trigger_point_type      *outdoor = NULL;
  gamma44_t               *gamma = NULL;
  trigger_ratio_t          trigger_ratio;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  gamma = (gamma44_t *)isp_sub_module->private_data;
  if (!gamma) {
    ISP_ERR("failed: mod %p", gamma);
    return FALSE;
  }

  if (event->u.module_event.type ==
    MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE) {
    manual_stats_update = (aec_manual_update_t *)event->u.module_event.module_event_data;
    if (!manual_stats_update){
      ISP_ERR("failed: stats_update %p", stats_update);
      return FALSE;
    }
    memset(&new_aec_stats, 0, sizeof(stats_update_t));
    new_aec_stats.aec_update.exif_iso = manual_stats_update->exif_iso;
    new_aec_stats.aec_update.linecount = manual_stats_update->linecount;
    new_aec_stats.aec_update.lux_idx   = manual_stats_update->lux_idx;
    new_aec_stats.aec_update.sensor_gain = manual_stats_update->sensor_gain;
    new_aec_stats.aec_update.real_gain = manual_stats_update->sensor_gain;
    new_aec_stats.aec_update.total_drc_gain = 1.0;
    new_aec_stats.aec_update.gtm_ratio = -1;
    new_aec_stats.aec_update.ltm_ratio = -1;
    new_aec_stats.aec_update.la_ratio = -1;
    new_aec_stats.aec_update.gamma_ratio = -1;
    new_aec_stats.aec_update.color_drc_gain = 1.0;
  } else if (event->u.module_event.type ==
    MCT_EVENT_MODULE_STATS_AEC_UPDATE) {
    stats_update = (stats_update_t *)event->u.module_event.module_event_data;
    if (!stats_update){
      ISP_ERR("failed: stats_update %p", stats_update);
      return FALSE;
    }
    new_aec_stats = *stats_update;
  }


  ISP_DBG("STATS G %f LC %d EXP_TIME %f SGAIN %f TOTDRC-GAIN %f GTM %f"
          " LTM %f LA %f GA %f COLOR_DRC %f)",
    new_aec_stats.aec_update.real_gain,
    new_aec_stats.aec_update.linecount,
    new_aec_stats.aec_update.exp_time,
    new_aec_stats.aec_update.sensor_gain,
    new_aec_stats.aec_update.total_drc_gain,
    new_aec_stats.aec_update.gtm_ratio,
    new_aec_stats.aec_update.ltm_ratio,
    new_aec_stats.aec_update.la_ratio,
    new_aec_stats.aec_update.gamma_ratio,
    new_aec_stats.aec_update.color_drc_gain);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  cur_aec_update = &gamma->aec_update;
  new_aec_update = &new_aec_stats.aec_update;

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    isp_sub_module->trigger_update_pending = TRUE;
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  chromatix_gamma = &chromatix_ptr->chromatix_VFE.chromatix_gamma;
  ISP_DBG("control_gamma %d", chromatix_gamma->control_gamma);

  if (gamma->ext_func_table && gamma->ext_func_table->get_trigger_ratio) {
    gamma->ext_func_table->get_trigger_ratio(gamma, isp_sub_module, new_aec_update);
  } else {
    ISP_ERR("failed: %p", gamma->ext_func_table);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  gamma->enable_adrc =
    isp_sub_module_util_is_adrc_mod_enable(new_aec_update->gamma_ratio,
      new_aec_update->total_drc_gain);

  if ((gamma->enable_adrc &&
      (!F_EQUAL(new_aec_update->total_drc_gain, cur_aec_update->total_drc_gain) ||
       !F_EQUAL(new_aec_update->gamma_ratio, cur_aec_update->gamma_ratio)))) {
    isp_sub_module->trigger_update_pending = TRUE;
    *cur_aec_update = *new_aec_update;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** gamma44_stats_asd_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 * Handle ASD update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean gamma44_stats_asd_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  stats_update_t   *stats_update = NULL;
  asd_update_t     *cur_asd = NULL;
  asd_update_t     *new_asd = NULL;
  gamma44_t        *gamma = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  gamma = (gamma44_t *)isp_sub_module->private_data;
  if (!gamma) {
    ISP_ERR("failed: mod %p", gamma);
    return FALSE;
  }

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  cur_asd = &gamma->asd_update;
  new_asd = &stats_update->asd_update;

  if (gamma->enable_backlight_compensation &&
      new_asd->backlight_detected &&
      (gamma->backlight_severity !=
        stats_update->asd_update.backlight_scene_severity)) {
    gamma->backlight_comp_update = TRUE;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  /* Store ASD update in module private */
  *cur_asd = *new_asd;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** gamma44_apply_contrast_sigmoid:
 *
 *    @sigmoid: sigmoid is curvature strength to be applied
 *    @multiplier: multiplier in gamma curve equation.
 *    @x: input value of pixel.
 *    @mid_pt: mid point of curve from where curve equation changes.
 *
 *  This function updates gamma table depending on contrast
 *
 *  Return: output value truncaed from double to integer
 **/
int16_t gamma44_apply_contrast_sigmoid(double sigmoid, double multiplier,
  double x, double mid_pt)
{
   double out = 0.0;
  if (x < mid_pt) {
    /* Curve quation -
     * lut = pow(255, 1-alpha)/pow(2, 1-alpha)*pow(x,alpha)
     *     = pow(255/2, 1-alpha)*pow(x,alpha)
     */
    out = (int16_t)
      (multiplier * isp_sub_module_util_power((double)x, sigmoid));
  } else {
    /* Curve quation -
     * lut = 255-pow(255,1-alpha)/pow(2,1-alpha)*pow(255-x,alpha)
     *     = 255-pow(255/2, 1-alpha)*pow(255-x,alpha)
     */
    out = (int16_t)(2*mid_pt -
      (multiplier * isp_sub_module_util_power((2*mid_pt - x), sigmoid)));
  }
  return (uint16_t)out;
}

/** gamma44_update_tab_on_contrast:
 *
 *    @contrast: contrast to be applied
 *    @ibits: table entry size in bits
 *    @input_table: input table
 *    @output_table: output table
 *
 *  This function updates gamma table depending on contrast
 *
 *  Return: TRUE  - Success
 *          FALSE - Input/Output table is NULL or contrast value is invalid
 **/
static uint16_t gamma44_update_tab_on_contrast(gamma44_t *gamma, int contrast,
  gamma_rgb_ds_t  *input_table_rgb, gamma_rgb_ds_t  *output_table_rgb)
{
  int             i = 0;
  const uint16_t *gamma_table = NULL;
  //int             entries = (lut_size >= 0) ? lut_size : ISP_GAMMA_NUM_ENTRIES;
  double          sigmoid = 0.0;

  /* Apply Sigmoid Gamma Table */
  if (input_table_rgb == NULL || output_table_rgb == NULL || !gamma ) {
    ISP_ERR(": input improper");
    return FALSE;
  }

  ISP_DBG(": E, contrast = %d",  contrast);
  switch (contrast) {
  case 10:
    sigmoid = 2.4;
    break;

  case 9:
    sigmoid = 2.1;
    break;

  case 8:
    sigmoid = 1.8;
    break;

  case 7:
    sigmoid = 1.5;
    break;

  case 6:
    sigmoid = 1.2;
    break;

  case 5:
    /* the input and output table are the same */
    *output_table_rgb = *input_table_rgb;
    return TRUE;

  case 4:
    sigmoid = 0.9;
    break;

  case 3:
    sigmoid = 0.8;
    break;

  case 2:
    sigmoid = 0.7;
    break;

  case 1:
    sigmoid = 0.6;
    break;

  case 0:
    sigmoid = 0.5;
    break;

  default:
    ISP_ERR("invalid contrast %d", contrast);
    return FALSE;
  }

#ifdef CONTRAST_LOOK_UP_METHOD
  /* do look up */
  for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
    output_table_rgb->gamma_r[i] = gamma_table[input_table_rgb->gamma_r[i]];
    output_table_rgb->gamma_g[i] = gamma_table[input_table_rgb->gamma_g[i]];
    output_table_rgb->gamma_b[i] = gamma_table[input_table_rgb->gamma_b[i]];
  }
#else
  /* Pixel data getting bigger (12-14 bits) Look up table method may not be
   * feasible adding contrast table compute method*/
  gamma44_compute_contrast_table(sigmoid, ISP_GAMMA_NUM_ENTRIES,
    input_table_rgb, output_table_rgb, gamma->enable_same_data_channel);
#endif

  return TRUE;
} /* gamma44_update_tab_on_contrast */

/** get_gamma_down_sampled_table
 *
 *  @table: gamma table
 *  @skipRatio: number of entries to be skipped
 *  @i: index in gamma table
 *
 *    The higher 12 bits in the configuration contains the delta between the
 *    current GammaTable value and the next value, while the lower 12 bits
 *    contains the current GammaTable value
 */
void gamma44_get_downsampled_table_uint16(float *ispTable,
  uint16_t *table,
  int skipRatio)
{
  int i;
  uint16_t tmp[ISP_GAMMA_NUM_ENTRIES];
  uint32_t val;

  if (skipRatio == 1){
         for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
           ispTable[i] = (float)(table[i]);
         }
  }
  else {
     //Down sample the 512 entries to 256 entries by skipping the 2nd entry
     for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
       tmp[i] = (1<<2) * table[skipRatio * i];
     }
     //Perform the smoothing by doing the averaging
    for (i = 1; i < ISP_GAMMA_NUM_ENTRIES - 1; i++) {
      val = (tmp[i - 1] + tmp[i + 1]);
      tmp[i] = (uint16_t)(val/2);
    }
     //Perform the smoothing by doing the averaging
    for (i = 1; i < ISP_GAMMA_NUM_ENTRIES - 1; i++) {
      val = (tmp[i - 1] + tmp[i + 1]);
      tmp[i] = (uint16_t)(val/2);
    }
    for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
      ispTable[i] = (float)(tmp[i] >> (2 + GAMMA_CHROMATIX_TBL_SIZE - ISP_PIPELINE_WIDTH ));
    }
  }
}

/* Same functionality as above function.
 * Except it downsamples table with floating value
 */

void gamma44_get_downsampled_table_float(float *ispTable,
  float *table,
  int skipRatio)
{
  int i;
  float tmp[ISP_GAMMA_NUM_ENTRIES];
  float val;

  if (skipRatio == 1){
         for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
           ispTable[i] = table[i];
         }
  }
  else {
     //Down sample the 512 entries to 256 entries by skipping the 2nd entry
     for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
       tmp[i] = (1<<2) * table[skipRatio * i];
     }
     //Perform the smoothing by doing the averaging
    for (i = 1; i < ISP_GAMMA_NUM_ENTRIES - 1; i++) {
      val = (tmp[i - 1] + tmp[i + 1]);
      tmp[i] = (val/2);
    }
     //Perform the smoothing by doing the averaging
    for (i = 1; i < ISP_GAMMA_NUM_ENTRIES - 1; i++) {
      val = (tmp[i - 1] + tmp[i + 1]);
      tmp[i] = (val/2);
    }
    for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
      ispTable[i] = tmp[i] / (2^(2 + GAMMA_CHROMATIX_TBL_SIZE - ISP_PIPELINE_WIDTH ));
    }
  }
}

/** gamma44_get_hi_lo_gamma_bits
 *
 *  @table: gamma table
 *  @skipRatio: number of entries to be skipped
 *  @i: index in gamma table
 *
 *    The higher 12 bits in the configuration contains the delta between the
 *    current GammaTable value and the next value, while the lower 12 bits
 *    contains the current GammaTable value
 */
int32_t gamma44_get_hi_lo_gamma_bits(float *table, int i)
{
  int32_t       hw_lut_entry = 0;
  int16_t       delta_lut = 0;
  uint16_t      entry_clamp = 0;
  uint16_t      entry_next_clamp = 0;
  uint16_t      gamma_tmp1 = 0;
  uint16_t      gamma_tmp2 = 0;

  gamma_tmp1 = (uint16_t)(Round(table[i]));
  gamma_tmp2 = (uint16_t)(Round(table[i+1]));

  entry_clamp = MIN(gamma_tmp1, MAX_GAMMA_VALUE);
  entry_next_clamp = MIN(gamma_tmp2, MAX_GAMMA_VALUE);
  delta_lut = (entry_next_clamp - entry_clamp);
  delta_lut = CLAMP_LIMIT(delta_lut, GAMMA_DELTA_MIN, GAMMA_DELTA_MAX);
  hw_lut_entry = (int32_t)((delta_lut << GAMMA_HW_PACK_BIT) + entry_clamp);

  return hw_lut_entry;
}

/** gamma44_get_last_gamma_value
 *
 *  @table: gamma table
 *  @skipRatio: number of entries to be skipped
 *
 */
int32_t gamma44_get_last_gamma_value(float *table)
{
  int32_t       hw_lut_entry = 0;
  uint16_t      delta_lut = 0;
  uint16_t      last_entry_clamp = 0;
  uint16_t      gamma_tmp = 0;

  /* consider the next entry of the last entry 2^12- 8084.
                                               2^14- 8994.
        value suggested by system team */
  uint32_t last_entry_next =
    (uint32_t) isp_sub_module_util_power(2.0, GAMMA_HW_PACK_BIT);

  /* this is effectively  table[255] - table[254];
        this part is the delta */
  /* use 256 as next entry of last entry */
  gamma_tmp = (uint16_t)(Round(table[ISP_GAMMA_NUM_ENTRIES - 1]));
  last_entry_clamp = MIN(gamma_tmp,
    MAX_GAMMA_VALUE);
  delta_lut = last_entry_next - last_entry_clamp;
  /* scale the delta */

  /* form the value:  upper byte is delta,
        lower byte is the entry itself. */
  hw_lut_entry = (int32_t)(delta_lut <<  GAMMA_HW_PACK_BIT) +
    last_entry_clamp;

  return hw_lut_entry;
}

/** gamma44_set_gamma_table
 *
 *  @p_gamma_rgb: contain pointers to each color of gamma
 *  @gamma: single gamma table
 *
 *  This function initializes rgb gamma pointers
 *
 *  Return: None
 */
static void gamma44_set_gamma_table(gamma_rgb_ds_t  *gamma_rgb,
  float *gamma)
{
  if (!gamma || !gamma_rgb) {
    ISP_ERR("failed: %p %p", gamma, gamma_rgb);
    return;
  }
  memcpy(gamma_rgb->gamma_r, gamma, ISP_GAMMA_NUM_ENTRIES * sizeof(float));
  memcpy(gamma_rgb->gamma_g, gamma, ISP_GAMMA_NUM_ENTRIES * sizeof(float));
  memcpy(gamma_rgb->gamma_b, gamma, ISP_GAMMA_NUM_ENTRIES * sizeof(float));
}

/** gamma44_set_table:
 *    @gamma: gamma module instance
 *
 *  This function sets gamma table depending on special effect/ bestshot.
 *
 *  Return:   0 - Success
 **/
boolean gamma44_set_table(isp_sub_module_t *isp_sub_module,
  gamma44_t* gamma, gamma_table_type gamma_table_type)
{
  int rc =0;
  boolean ret = FALSE;

  if (!isp_sub_module || !gamma) {
    ISP_ERR("failed: %p %p", isp_sub_module, gamma);
    return FALSE;
  }
  chromatix_parms_type *chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  chromatix_gamma_type *pchromatix_gamma =
    &(chromatix_ptr->chromatix_VFE.chromatix_gamma);

  if (!chromatix_ptr || !pchromatix_gamma) {
    ISP_ERR("failed: %p %p", chromatix_ptr, pchromatix_gamma);
    return FALSE;
  }

  gamma->gamma_lut_size = GAMMA_CHROMATIX_LUT_SIZE;
  switch (gamma_table_type) {
  case GAMMA_TABLE_OUTDOOR:
    gamma44_copy_gamma(gamma, &(gamma->gamma_table_rgb),
      &gamma->outdoor_gamma_tbl, ISP_GAMMA_NUM_ENTRIES);
      break;

  case GAMMA_TABLE_LOWLIGHT:
    gamma44_copy_gamma(gamma, &(gamma->gamma_table_rgb),
      &gamma->lowlight_gamma_tbl, ISP_GAMMA_NUM_ENTRIES);

      break;

    case GAMMA_TABLE_WHITE_BOARD:
      gamma->gamma_lut_size = sizeof(WHITEBOARD_GAMMA_TABLE)/sizeof(uint16_t);
      gamma44_get_downsampled_tables(gamma, &gamma->gamma_table_rgb,
        (uint16_t *)WHITEBOARD_GAMMA_TABLE, NULL,
        gamma->gamma_lut_size/ISP_GAMMA_NUM_ENTRIES);
      break;

    case GAMMA_TABLE_BLACK_BOARD:
      gamma->gamma_lut_size = sizeof(BLACKBOARD_GAMMA_TABLE)/sizeof(uint16_t);
      gamma44_get_downsampled_tables(gamma, &gamma->gamma_table_rgb,
        (uint16_t *)BLACKBOARD_GAMMA_TABLE, NULL,
           gamma->gamma_lut_size/ISP_GAMMA_NUM_ENTRIES);
      break;

    case GAMMA_TABLE_DEFAULT:
    default: {
      /* Check if ext table type is supported */
      if (gamma->ext_func_table && gamma->ext_func_table->ext_set_table)
        ret = gamma->ext_func_table->ext_set_table(gamma, isp_sub_module, &gamma_table_type);
      else
        ret = FALSE;

      if (ret == FALSE) {
        /* No supported table / ext table type found, hence use default*/
        gamma44_copy_gamma(gamma, &(gamma->gamma_table_rgb),
           &gamma->default_gamma_tbl,ISP_GAMMA_NUM_ENTRIES);
      }
    }
      break;
  }
  gamma->gamma_table_type = gamma_table_type;
  isp_sub_module->trigger_update_pending = TRUE;

  return TRUE;
} /* vfe_gamma_set_table */

/** gamma44_set_bestshot:
 *    @gamma: gamma module instance
 *    @isp_sub_module: base module instance
 *  This function configures gamma module according bestshot set.
 *
 *  Return:   0 - Success
 *           -1 - Parameter size mismatch
 **/
boolean gamma44_set_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                   ret = TRUE;
  int32_t                  *bestshot = NULL;
  gamma44_t                *gamma = NULL;
  mct_event_control_t      *ctrl_event = NULL;
  mct_event_control_parm_t *param = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  gamma = (gamma44_t *)isp_sub_module->private_data;
  ctrl_event = &event->u.ctrl_event;

  if (!gamma || !ctrl_event) {
    ISP_ERR("failed: gamma %p ctrl_event %p", gamma, ctrl_event);
    return FALSE;
  }

  param = ctrl_event->control_event_data;
  bestshot = (int32_t *)param->parm_data;
  if (!bestshot) {
    ISP_ERR("failed: stats_update %p", bestshot);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  gamma->bestshot_mode = *bestshot;
  ISP_HIGH("bestshot mode %d",  gamma->bestshot_mode);
  isp_sub_module->submod_trigger_enable = FALSE;
  switch(gamma->bestshot_mode) {
  case CAM_SCENE_MODE_LANDSCAPE:
  case CAM_SCENE_MODE_SNOW:
  case CAM_SCENE_MODE_BEACH:
  case CAM_SCENE_MODE_SPORTS:
  case CAM_SCENE_MODE_ACTION:
    ret = gamma44_set_table(isp_sub_module, gamma, GAMMA_TABLE_OUTDOOR);
    break;

  case CAM_SCENE_MODE_NIGHT:
  case CAM_SCENE_MODE_FIREWORKS:
  case CAM_SCENE_MODE_NIGHT_PORTRAIT:
    ret = gamma44_set_table(isp_sub_module, gamma, GAMMA_TABLE_LOWLIGHT);
    break;

  case CAM_SCENE_MODE_BACKLIGHT:
    ret = gamma44_set_table(isp_sub_module, gamma, GAMMA_TABLE_BACKLIGHT);
    break;

  default:
    gamma->bestshot_mode = CAM_SCENE_MODE_OFF;
    if (gamma->effects.spl_effect == CAM_EFFECT_MODE_OFF) {
      isp_sub_module->submod_trigger_enable = TRUE;
    }
    ret = TRUE;
    break;
  }

  if (ret == TRUE && isp_sub_module->submod_trigger_enable == FALSE){
    ISP_DBG(": enable update through BSM for mode : %d",
      gamma->bestshot_mode);
    gamma44_update_vfe_table(gamma, &gamma->ISP_GammaCfgCmd,
          &gamma->gamma_table_rgb);
    isp_sub_module->trigger_update_pending = TRUE;
  } else {
    ISP_DBG("failed: set table for bestshot %d, reset to bestshot off",
      gamma->bestshot_mode);
    gamma->bestshot_mode = CAM_SCENE_MODE_OFF;
    isp_sub_module->submod_trigger_enable = TRUE;
    isp_sub_module->trigger_update_pending = TRUE;
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* vfe_gamma_set_bestshot */

/** gamma44_set_spl_effect:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function configures gamma module according special effect set.
 *
 *  Return:   0 - Success
 *           -1 - Parameter size mismatch
 **/
boolean gamma44_set_spl_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                   ret = TRUE;
  int                       type = 0;
  gamma44_t                *gamma = NULL;
  mct_event_control_t      *ctrl_event = NULL;
  mct_event_control_parm_t *param = NULL;
  chromatix_parms_type     *chromatix = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  chromatix =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix) {
    ISP_ERR("failed: chromatix %p", chromatix);
    return FALSE;
  }

  gamma = (gamma44_t *)isp_sub_module->private_data;
  ctrl_event = &event->u.ctrl_event;

  if (!gamma || !ctrl_event) {
    ISP_ERR("failed: gamma %p ctrl_event %p", gamma, ctrl_event);
    return FALSE;
  }

  param = ctrl_event->control_event_data;
  if (gamma->effects.spl_effect == *(uint32_t *)param->parm_data) {
    ISP_HIGH(": same effect is received");
    return TRUE;
  }
  gamma->effects.spl_effect = *(uint32_t *)param->parm_data;

  if (gamma->bestshot_mode != CAM_SCENE_MODE_OFF) {
    ISP_HIGH(": Best shot enabled, skip set effect");
    return TRUE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  type = gamma->effects.spl_effect;
  ISP_DBG(": contrast %d effect type %d",  gamma->effects.contrast, type);

  isp_sub_module->submod_trigger_enable = FALSE;
  gamma->effect_applied = FALSE;
  ISP_DBG(": type %d",  type);
  switch (type) {
  case CAM_EFFECT_MODE_POSTERIZE:
  case CAM_EFFECT_MODE_SOLARIZE: {
    /* UI should prevent this */
    ISP_ERR(": Warning Overriding contrast value to default");
    gamma->effects.contrast = DEFAULT_CONTRAST;
    ret = TRUE;
    isp_sub_module->submod_trigger_enable = TRUE;
    isp_sub_module->trigger_update_pending = TRUE;
  }
    break;

  case CAM_EFFECT_MODE_BLACKBOARD: {
    gamma->effect_applied = TRUE;
    ret = gamma44_set_table(isp_sub_module, gamma, GAMMA_TABLE_BLACK_BOARD);
    gamma->effects.contrast = DEFAULT_CONTRAST;
    break;
  }

  case CAM_EFFECT_MODE_WHITEBOARD: {
    gamma->effect_applied = TRUE;
    ret = gamma44_set_table(isp_sub_module, gamma, GAMMA_TABLE_WHITE_BOARD);
    gamma->effects.contrast = DEFAULT_CONTRAST;
    break;
  }

  default:
    /* Check if ext spl effect is supported */
    if (gamma->ext_func_table && gamma->ext_func_table->ext_set_spl_effect) {
     ret = gamma->ext_func_table->ext_set_spl_effect(gamma, isp_sub_module, &type);
    } else {
      gamma->effects.spl_effect = CAM_EFFECT_MODE_OFF;
      ret = TRUE;
      isp_sub_module->submod_trigger_enable = TRUE;
      isp_sub_module->trigger_update_pending = TRUE;
    }
    break;
  }

  if (ret == TRUE && isp_sub_module->submod_trigger_enable == FALSE) {
    ISP_DBG(": enable update through special effect type %d", type);
    gamma44_update_vfe_table(gamma, &gamma->ISP_GammaCfgCmd,
      &gamma->gamma_table_rgb);
    isp_sub_module->trigger_update_pending = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* gamma44_set_spl_effect */

/** gamma44_set_contrast:
 *
 *    @mod: gamma module instance
 *    @pix_settings: PIX parameters
 *    @in_param_size: parameters size
 *
 *  This function sets gamma table depending on contrast
 *
 *  Return:   0: Success
 *           -1: Parameter size does not match
 **/
boolean gamma44_set_contrast(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                   ret = TRUE;
  int32_t                  *contrast = NULL;
  gamma44_t                *gamma = NULL;
  mct_event_control_t      *ctrl_event = NULL;
  mct_event_control_parm_t *param = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  gamma = (gamma44_t *)isp_sub_module->private_data;
  ctrl_event = &event->u.ctrl_event;

  if (!gamma || !ctrl_event) {
    ISP_ERR("failed: gamma %p ctrl_event %p", gamma, ctrl_event);
    return FALSE;
  }

  param = ctrl_event->control_event_data;
  contrast = (int32_t *)param->parm_data;
  if (!contrast) {
    ISP_ERR("failed: stats_update %p", contrast);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (gamma->bestshot_mode != CAM_SCENE_MODE_OFF) {
    ISP_HIGH(": Warning Best shot enabled, ignore contast");
    goto end;
  }

  switch(gamma->gamma_table_type) {
  case GAMMA_TABLE_SOLARIZE:
  case GAMMA_TABLE_POSTERIZE:
  case GAMMA_TABLE_BACKLIGHT: {
    ISP_HIGH(": Warning Effect enabled, ignore contast");
    goto end;
  }
    break;

  default:
    break;
  }
  ISP_DBG(": current contrast %d new contrast = %d",
    gamma->effects.contrast, *contrast);
  if (gamma->effects.contrast != *contrast) {
    gamma->effects.contrast = *contrast;
    isp_sub_module->trigger_update_pending = TRUE;
  }
  ISP_DBG(": X, trigger_update_pending = %s",
    (isp_sub_module->trigger_update_pending ? "TRUE":"FALSE"));

end:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* vfe_gamma_set_contrast */

/** gamma44_reset:
 *
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets gamma module private data to default values
 *
 *  Return:   None
 **/
boolean  gamma44_reset(isp_sub_module_t *isp_submodule,
  gamma44_t *gamma)
{
  memset(&gamma->ISP_GammaCfgCmd, 0, sizeof(gamma->ISP_GammaCfgCmd));
  memset(&gamma->gamma_table_rgb, 0, sizeof(gamma->gamma_table_rgb));
  memset(&gamma->default_gamma_tbl, 0, sizeof(gamma->default_gamma_tbl));
  memset(&gamma->lowlight_gamma_tbl, 0, sizeof(gamma->lowlight_gamma_tbl));
  memset(&gamma->outdoor_gamma_tbl, 0, sizeof(gamma->outdoor_gamma_tbl));
  memset(&gamma->solarize_gamma_table, 0, sizeof(gamma->solarize_gamma_table));
  memset(&gamma->gamma_ratio, 0, sizeof(gamma->gamma_ratio));

  gamma->enable_backlight_compensation = 0;
  gamma->gamma_lut_size                = GAMMA_CHROMATIX_LUT_SIZE;
  gamma->effects.contrast              = DEFAULT_CONTRAST;
  gamma->gamma_table_type              = GAMMA_TABLE_DEFAULT;
  gamma->bestshot_mode                 = CAM_SCENE_MODE_OFF;
  gamma->backlight_severity            = 0;
  gamma->reload_params                 = 0;
  gamma->vfe_reconfig                  = 0;
  gamma->preview_trigger               = 0;

  isp_submodule->trigger_update_pending  = FALSE;
  isp_submodule->submod_trigger_enable   = TRUE;
  isp_submodule->submod_enable           = TRUE;
  isp_submodule->manual_ctrls.tonemap_mode = CAM_TONEMAP_MODE_FAST;
  isp_submodule->manual_ctrls.update_gamma = FALSE;
  gamma->gamma_algo.gamma_tbl = (gamma_rgb_ds_t *)&gamma->gamma_table_rgb;
  gamma->gamma_algo.stats_update = &gamma->aec_update;
  return TRUE;
}/* gamma44_reset */


/** gamma44_query_cap:
 *
 *    @module: mct module instance
 *    @query_buf: query caps buffer handle
 *
 *  Returns TRUE
 *
 **/
boolean gamma44_query_cap(
    mct_module_t *module,
    void         *query_buf)
{
  mct_pipeline_cap_t     *cap_buf = NULL;
  mct_pipeline_isp_cap_t *mod_cap = NULL;

  if (!module) {
    ISP_ERR("failed: %p", module);
    return FALSE;
  }

  cap_buf = (mct_pipeline_cap_t *)query_buf;
  mod_cap = &cap_buf->isp_cap;
  mod_cap->max_gamma_curve_points = ISP_GAMMA_NUM_ENTRIES;
  return TRUE;
}

/** gamma44_init:
 *
 *    @module: mct module instance
 *    @isp_sub_module: base module
 *
 *  This function initializes gamma module
 *
 *  Return:   None
 **/
boolean gamma44_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  gamma44_t *gamma = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  gamma = (gamma44_t *)CAM_MALLOC(sizeof(gamma44_t));
  if (!gamma) {
    ISP_ERR("failed: gamma %p", gamma);
    return FALSE;
  }

  memset(gamma, 0, sizeof(*gamma));

  isp_sub_module->private_data = (void *)gamma;

  if (!gamma) {
    ISP_ERR("failed: mod %p", gamma);
    return FALSE;
  }
  gamma44_reset(isp_sub_module, gamma);
  FILL_FUNC_TABLE(gamma);
  return TRUE;
}/* gamma44_init */

/** gamma44_destroy:
 *
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  This function destroys gamma module
 *
 *  Return: void
 **/
void gamma44_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  gamma44_t *gamma = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }
  gamma = (gamma44_t *)isp_sub_module->private_data;

  memset(gamma, 0, sizeof(gamma44_t));
  free(gamma);

  return;
} /* gamma44_destroy */

/** gamma44_init_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration of gamma module
 *
 *  Return:   TRUE - Success
 **/
boolean gamma44_init_config(isp_sub_module_t *isp_sub_module, gamma44_t *gamma)
{
  boolean ret = TRUE;
  chromatix_parms_type          *chromatix_ptr = NULL;
  chromatix_gamma_type          *pchromatix_gamma = NULL;
  gamma_rgb_ds_t                 gammaTable;
  gamma_rgb_t                    gammaChromatixTable;
  int skipRatio = GAMMA_CHROMATIX_LUT_SIZE / ISP_GAMMA_NUM_ENTRIES;

  chromatix_ptr =
    isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }
  pchromatix_gamma =
    &(chromatix_ptr->chromatix_VFE.chromatix_gamma);


  /*init gamma parameters*/
  gamma->enable_backlight_compensation = TRUE;

  /*load chromatix to gamma*/

  /* Get the downsampled low light gamma tables*/
  gamma44_chromatix_to_isp_gamma(gamma, &gammaChromatixTable,
    &(GAMMA_LOWLIGHT_TABLE(pchromatix_gamma)));
  gamma44_get_downsampled_tables(gamma, &gamma->lowlight_gamma_tbl,
    NULL, &gammaChromatixTable, skipRatio);

  /* Get the downsampled outdoor light gamma tables*/
  gamma44_chromatix_to_isp_gamma(gamma, &gammaChromatixTable,
    &(GAMMA_OUTDOOR_TABLE(pchromatix_gamma)));
  gamma44_get_downsampled_tables(gamma, &gamma->outdoor_gamma_tbl,
    NULL, &gammaChromatixTable, skipRatio);

  /* Get the downsampled default gamma tables*/
  gamma44_chromatix_to_isp_gamma(gamma, &gammaChromatixTable,
    &(GAMMA_DEFAULT_TABLE(pchromatix_gamma)));
  gamma44_get_downsampled_tables(gamma, &gamma->default_gamma_tbl,
    NULL, &gammaChromatixTable, skipRatio);

  gamma->gamma_lut_size = GAMMA_CHROMATIX_LUT_SIZE;

  /*load chromatix to gamma*/
  gamma44_copy_gamma(gamma, &(gamma->gamma_table_rgb), &gamma->default_gamma_tbl,
    ISP_GAMMA_NUM_ENTRIES);

  gamma44_set_solarize_table(gamma, chromatix_ptr);

  ISP_DBG("bestshot_mode %d, gamma_table_type %d",
    gamma->bestshot_mode, gamma->gamma_table_type);
  if (gamma->bestshot_mode == CAM_SCENE_MODE_OFF) {
    /* compensate for contrast */
    switch(gamma->gamma_table_type) {
      case GAMMA_TABLE_SOLARIZE:
      case GAMMA_TABLE_POSTERIZE:
      case GAMMA_TABLE_BACKLIGHT:
        /* special effect, no contrast apply.
                 config gamma HW table and configuration*/
        gamma44_update_vfe_table(gamma, &gamma->ISP_GammaCfgCmd,
          &(gamma->gamma_table_rgb));
        break;
      default:
        if (gamma->ext_func_table && gamma->ext_func_table->ext_set_spl_effect) {
         ret = gamma->ext_func_table->ext_set_spl_effect(gamma, isp_sub_module,
            &gamma->effects.spl_effect);
         if (ret == FALSE) {
           ISP_ERR("failed: ext_set_spl_effect\n");
         }
        } else {
          gamma->effects.spl_effect = CAM_EFFECT_MODE_OFF;
        }
        /* apply contrast, update the original table*/
        if (gamma->effects.contrast != DEFAULT_CONTRAST &&
            gamma->effects.spl_effect == CAM_EFFECT_MODE_OFF) {
          gamma44_update_tab_on_contrast(gamma, gamma->effects.contrast,
            &(gamma->gamma_table_rgb),&gammaTable);
          gamma44_update_vfe_table(gamma, &gamma->ISP_GammaCfgCmd,
            &(gammaTable));
        } else {
          /* contrast 5, default table, use original table*/
          gamma44_update_vfe_table(gamma, &gamma->ISP_GammaCfgCmd,
            &(gamma->gamma_table_rgb));
        }
        break;
    }
  } else {
    switch(gamma->bestshot_mode) {
    case CAM_SCENE_MODE_LANDSCAPE:
    case CAM_SCENE_MODE_SNOW:
    case CAM_SCENE_MODE_BEACH:
    case CAM_SCENE_MODE_SPORTS:
    case CAM_SCENE_MODE_ACTION:
      ret = gamma44_set_table(isp_sub_module, gamma, GAMMA_TABLE_OUTDOOR);
      break;
    case CAM_SCENE_MODE_NIGHT:
    case CAM_SCENE_MODE_FIREWORKS:
    case CAM_SCENE_MODE_NIGHT_PORTRAIT:
      ret = gamma44_set_table(isp_sub_module, gamma, GAMMA_TABLE_LOWLIGHT);
      break;
    case CAM_SCENE_MODE_BACKLIGHT:
      ret = gamma44_set_table(isp_sub_module, gamma, GAMMA_TABLE_BACKLIGHT);
      break;
    default:
      ret = gamma44_set_table(isp_sub_module, gamma, GAMMA_TABLE_DEFAULT);
      break;
    }

    if (ret == TRUE){
      ISP_DBG(": enable update through BSM for mode : %d",
       gamma->bestshot_mode);
      isp_sub_module->trigger_update_pending = TRUE;
    } else {
      ISP_ERR("failed: set table for bestshot %d, reset to bestshot off",
        gamma->bestshot_mode);
      gamma->bestshot_mode = CAM_SCENE_MODE_OFF;
    }
  }

  isp_sub_module->trigger_update_pending = TRUE;

  return TRUE;
} /* gamma44_init_config */

/** gamma44_store_single_HW_write:
 *
 *  @cmd_offset: reg cmd offset
 *  @cmd_len: direction of port to be retrieved
 *  @cmd_data_offset: cmd data offset
 *  @hw_reg_offset: identity to match
 *  @reg_num : number of registers
 *  @cmd_type : type of cmd
 *  @ isp_sub_module: base module object
 *
 *  Preapre hw update list of reg settings for module.
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gamma44_store_single_HW_write(void* cmd_offset,
  uint32_t cmd_len, uint32_t cmd_data_offset, uint32_t hw_reg_offset,
  uint32_t reg_num, uint32_t cmd_type,
  isp_sub_module_t *isp_sub_module)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_GammaConfigCmdType      *reg_cmd = NULL;
  uint32_t                    *data = NULL;
  uint32_t                     len = 0;

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    goto ERROR;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD_MALLOC;
  }
  memset(reg_cfg_cmd, 0, (sizeof(*reg_cfg_cmd)));

  reg_cmd = (ISP_GammaConfigCmdType *)CAM_MALLOC(sizeof(*reg_cmd));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cmd %p", reg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, (sizeof(*reg_cmd)));
  *reg_cmd = *(ISP_GammaConfigCmdType *)cmd_offset;

  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_data = reg_cmd;
  cfg_cmd->cmd_len = cmd_len;
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = cmd_data_offset;
  reg_cfg_cmd[0].cmd_type = cmd_type;
  reg_cfg_cmd[0].u.rw_info.reg_offset = hw_reg_offset;
  reg_cfg_cmd[0].u.rw_info.len = reg_num * sizeof(uint32_t);

#ifdef PRINT_REG_VAL_SET
  ISP_DBG("hw_reg_offset %x, len %d", reg_cfg_cmd[0].u.rw_info.reg_offset,
    reg_cfg_cmd[0].u.rw_info.len);
  data = (uint32_t *)cmd_offset;
  for (len = 0; len < (cmd_len / 4); len++) {
    ISP_DBG("data[%d] %x", len, data[len]);
  }
#endif

  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }
  isp_sub_module->trigger_update_pending = FALSE;
  return ret;

ERROR_APPEND:
  free(reg_cmd);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD_MALLOC:
  free(hw_update);
ERROR:
  return FALSE;

}

/** gamma44_reset_dmi_cfg:
 * @mod_ctrl: gamma module instance
 * @gamma_channel: parameter id
 * @isp_sub_module: Base module object
 *
 *  Reset DMI table for gamma
 *
 *  Return:   0 - Success
 **/
boolean gamma44_reset_dmi_cfg(
    gamma44_t        *gamma,
    uint32_t         gamma_channel,
    isp_sub_module_t *isp_sub_module)
{
  boolean  ret = TRUE;
  uint32_t len = 0;

  uint32_t *dmi_cfg = NULL;
  uint32_t *data    = NULL;

  struct msm_vfe_cfg_cmd2     *cfg_cmd     = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update   = NULL;

  if (!gamma || !isp_sub_module) {
      ISP_ERR("failed: %p %p", gamma, isp_sub_module);
      return FALSE;
  }

  dmi_cfg = (uint32_t *)CAM_MALLOC(sizeof(uint32_t) * 2);
  if (!dmi_cfg) {
    ISP_ERR("failed: dmi_cfg %p", dmi_cfg);
    return FALSE;
  }
  memset(dmi_cfg, 0, sizeof(*dmi_cfg));

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    goto ERROR_FREE_DMI_CFG;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd) * 2);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_FREE_HW_UPDATE;
  }
  memset(reg_cfg_cmd, 0, (sizeof(*reg_cfg_cmd)));

  /* reset dmi cfg: config dmi channel and set auto increment*/
  dmi_cfg[0] = ISP_DMI_CFG_DEFAULT;
  dmi_cfg[0] += gamma_channel;

  /* reset dmi_addr_cfg: dmi address always start form 0 */
  dmi_cfg[1] = 0;

  cfg_cmd = &hw_update->cfg_cmd;
  /* PACK the 2 cfg cmd for 1 ioctl*/
  cfg_cmd->cfg_data = dmi_cfg;
  cfg_cmd->cmd_len = sizeof(*dmi_cfg);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 2;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_DMI_CFG_OFF;
  reg_cfg_cmd[0].u.rw_info.len = 1 * sizeof(uint32_t);

  reg_cfg_cmd[1].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset +
      reg_cfg_cmd[0].u.rw_info.len;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_DMI_ADDR;
  reg_cfg_cmd[1].u.rw_info.len = 1 * sizeof(uint32_t);

#ifdef PRINT_REG_VAL_SET
  ISP_DBG("hw_reg_offset %x, len %d", reg_cfg_cmd[0].u.rw_info.reg_offset,
    reg_cfg_cmd[0].u.rw_info.len);
  data = (uint32_t *)cfg_cmd->cfg_data;
  for (len = 0; len < (cfg_cmd->cmd_len / 4); len++) {
    ISP_DBG("data[%d] %x", len, data[len]);
  }
#endif

  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_FREE_REG_CFG_CMD;
  }

  return ret;

ERROR_FREE_REG_CFG_CMD:
  free(reg_cfg_cmd);
ERROR_FREE_HW_UPDATE:
  free(hw_update);
ERROR_FREE_DMI_CFG:
  free(dmi_cfg);
  return FALSE;

} /* gamma44_reset_dmi_cfg */

/** gamma44_single_HW_write:
 *    @cmd_offset:command
 *    @cmd_len: command length
 *    @hw_reg_offset: HW register offset
 *    @reg_num: number of registers
 *    @cmd_type: type of command
 *    @ isp_sub_module: base module object
 *
 *  Preapre hw update list of reg settings for module.
 *  This function populates dmi table  and appends hw update list
 *
 *  Return:   0 - Success
 **/
boolean gamma44_store_dmi_info(
    void*            cmd_offset,
    uint32_t         cmd_len,
    uint32_t         hw_reg_offset __unused,
    uint32_t         reg_num __unused,
    uint32_t         cmd_type,
    isp_sub_module_t *isp_sub_module)
{

  boolean ret = TRUE;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2 *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd = NULL;
  uint32_t *data = NULL;
  uint32_t len = 0;

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_HW_UPDATE;
  }
  memset(reg_cfg_cmd, 0, (sizeof(*reg_cfg_cmd)));

  cfg_cmd = &hw_update->cfg_cmd;

  cfg_cmd->cfg_data = cmd_offset;
  cfg_cmd->cmd_len = cmd_len;
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].cmd_type = cmd_type;
  reg_cfg_cmd[0].u.dmi_info.hi_tbl_offset = 0;
  reg_cfg_cmd[0].u.dmi_info.lo_tbl_offset = 0;
  reg_cfg_cmd[0].u.dmi_info.len = cmd_len;

#ifdef PRINT_REG_VAL_SET
  ISP_ERR("hw_reg_offset %x, len %d", reg_cfg_cmd[0].u.rw_info.reg_offset,
    reg_cfg_cmd[0].u.rw_info.len);
  data = (uint32_t *)cmd_offset;
  for (len = 0; len < (cmd_len / 4); len++) {
    ISP_ERR("data[%d] %x", len, data[len]);
  }
#endif

  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR;

  }
  return ret;
ERROR:
  free(reg_cfg_cmd);
ERROR_HW_UPDATE:
  free(hw_update);

 return FALSE;
} /* gamma44_single_HW_write */

/** gamma44_do_hw_update:
 *    @ isp_sub_module:base module object
 *    @gamma: gamma module instance
 *
 *  This function stores hw update reg cmd in base module private
 *
 *  Return:   TREU - Success
 *            FALSE - configuration error
 **/
boolean gamma44_store_hw_update(isp_sub_module_t *isp_sub_module,
  gamma44_t *gamma)
{
  int     i = 0;
  boolean ret = TRUE;

  if (!gamma || !isp_sub_module) {
    ISP_ERR("failed: %p %p", gamma, isp_sub_module);
    return FALSE;
  }

  if (GAMMA_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      GAMMA_CGC_OVERRIDE_REGISTER, GAMMA_CGC_OVERRIDE_BIT, TRUE);
    if (ret == FALSE) {
      ISP_ERR("failed: enable cgc");
    }
  }

  ret = gamma44_dmi_hw_update(gamma,
    gamma->ISP_GammaCfgCmd.LutSel, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: gamma_dmi_hw_update");
    goto ERROR;
  }

  ret = gamma44_store_single_HW_write(&gamma->ISP_GammaCfgCmd,
    sizeof(gamma->ISP_GammaCfgCmd.LutSel), 0,
    ISP_RGB_LUT_OFF, ISP_RGB_LUT_LEN, VFE_WRITE, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: gamma_store_single_HW_write");
    goto ERROR;
  }

  gamma->ISP_GammaCfgCmd.LutSel.ch0BankSelect ^= 1;
  gamma->ISP_GammaCfgCmd.LutSel.ch1BankSelect ^= 1;
  gamma->ISP_GammaCfgCmd.LutSel.ch2BankSelect ^= 1;

  gamma->applied_table = gamma->ISP_GammaCfgCmd.Gamatbl;

  if (GAMMA_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      GAMMA_CGC_OVERRIDE_REGISTER, GAMMA_CGC_OVERRIDE_BIT, FALSE);
    if (ret == FALSE) {
      ISP_ERR("failed: disable cgc");
    }
  }

  isp_sub_module->trigger_update_pending = FALSE;

ERROR:
  return ret;
}/* gamma44_do_hw_update */

/** gamma44_interp_gamma_table:
 *    @ dest: destination gamma curve
 *    @src_count: manual gamma table length
 *    @src_curve: manual gamam curve
 *
 *  Do a linear interpolation of the input gamma curve
 *  into isp native curve format
 *
 *  Return:   None
 **/
void gamma44_interp_gamma_table(float *dest,
  int src_count, const cam_tonemap_curve_t *src_curve)
{
  int i,index1,index2;
  float x;
  float result;

  if (!dest || !src_curve) {
    ISP_ERR("failed: %p %p", dest, src_curve);
    return;
  }
  /* Application sends the gamma values from the range of 0.0 to 1.0 ,
     and also the number of curve points can be <= to the max
     gamma cure points.
     We need to interpret it into the ISP native format and count */

  /* We are assuming x coordinate of input curve is evenly sampled. */
  for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
    x = 1.0 * i * (src_count - 1) / (ISP_GAMMA_NUM_ENTRIES - 1);

    index1 = x;
    index2 = index1 + 1;

    if (index1 < src_count - 1)
      result = ROUND(MAX_GAMMA_VALUE * ((x - index1) *
         src_curve->tonemap_points[index2][1] + (index2 - x) *
         src_curve->tonemap_points[index1][1]));
    else
      result = MAX_GAMMA_VALUE * src_curve->tonemap_points[index1][1];

    if (result > MAX_GAMMA_VALUE)
      dest[i] = MAX_GAMMA_VALUE;
    else
      dest[i] = result;

    ISP_DBG("index1 = %d, index2 = %d, x = %f, point[index1] = %f,"
      "point[index2] = %f, dest[%d] = %f", index1, index2, x,
      src_curve->tonemap_points[index1][1],
      src_curve->tonemap_points[index2][1], i, dest[i]);
  }
  return;
}

/** gamma44_manual_update:
 *    @module: mct module instance
 *    @isp_sub_module: base module
 *    @event: mct event
 *
 *   Apply the manual gamma curve when the manual tone map mode
 *   is set
 *
 *  Return:   TRUE - Success
 *             FALSE - get aec ratio returned error
 **/
boolean gamma44_manual_update(mct_module_t *module,
   isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                      ret = TRUE;
  int32_t                      curve;
  gamma_rgb_ds_t               app_gamma_tbl;
  gamma44_t                    *gamma = NULL;
  isp_sub_module_output_t      *output = NULL;
  isp_private_event_t          *private_event = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    return FALSE;
  }

  gamma = (gamma44_t *)isp_sub_module->private_data;
  if (!gamma) {
    ISP_ERR("failed: gamma %p", gamma);
    return FALSE;
  }

  /* For manual mode , same channel flag is mo more valid */
  gamma->enable_same_data_channel = FALSE;

  /* App curve 0 - G. Curve 1 - B, Curve 2 - R */
  gamma44_interp_gamma_table(&(app_gamma_tbl.gamma_g[0]),
    isp_sub_module->manual_ctrls.gamma_curve.tonemap_points_cnt,
    &(isp_sub_module->manual_ctrls.gamma_curve.curves[0]));

  gamma44_interp_gamma_table(&(app_gamma_tbl.gamma_b[0]),
    isp_sub_module->manual_ctrls.gamma_curve.tonemap_points_cnt,
    &(isp_sub_module->manual_ctrls.gamma_curve.curves[1]));

  gamma44_interp_gamma_table(&(app_gamma_tbl.gamma_r[0]),
    isp_sub_module->manual_ctrls.gamma_curve.tonemap_points_cnt,
    &(isp_sub_module->manual_ctrls.gamma_curve.curves[2]));

  gamma44_update_vfe_table(gamma, &gamma->ISP_GammaCfgCmd,
    &app_gamma_tbl);

  /*Populate and store hw update params inside isp_sub_mod.
      then using utility function append this isp_sub_mod's hw update list
      into global hw update list*/
  ret = gamma44_store_hw_update(isp_sub_module, gamma);
  if (ret == FALSE) {
    ISP_ERR("failed: gamma_do_hw_update");
    return ret;
  }
  /* append hw update list of gamma module to global list*/
  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list mod %s",
    MCT_MODULE_NAME(module));
    return ret;
  }

  return ret;
}

/** gamma44_trigger_update:
 *    @module: mct module instance
 *    @isp_sub_module: base module
 *    @event: mct event
 *
 *  This function checks and initiates triger update of module
 *
 *  Return:   TRUE - Success
 *             FALSE - get aec ratio returned error
 **/
boolean gamma44_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  uint8_t                       is_burst = FALSE;
  boolean                       ret = TRUE;
  int                           i,backlight_comp_update = FALSE;
  int                           update_gamma = FALSE;
  gamma44_t                    *gamma = NULL;
  gamma_rgb_ds_t                contrast_gamma_table_rgb;
  isp_sub_module_priv_t        *isp_sub_module_priv = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  isp_meta_entry_t             *gamma_dmi_info = NULL;
  isp_hw_read_info_t           *applied_dmi_tbl = NULL;
  gamma_table_t                *gamma_hw_table = NULL;
  uint32_t                     *gamma_dmi_tbl_r = NULL;
  uint32_t                      gamma_entry_size;
  uint32_t                      gamma_table_size;
  chromatix_parms_type       *chromatix_ptr = NULL;
  chromatix_videoHDR_type    *chromatix_VHDR = NULL;
  chromatix_videoHDR_reserve *chromatix_HDR_reserve = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  gamma = (gamma44_t *)isp_sub_module->private_data;
  if (!gamma) {
    ISP_ERR("failed: gamma %p", gamma);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  gamma44_set_same_channel_data_flag(isp_sub_module);

  if (isp_sub_module->manual_ctrls.manual_update &&
      isp_sub_module->chromatix_module_enable &&
    (isp_sub_module->config_pending == FALSE)) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    if (isp_sub_module->manual_ctrls.tonemap_mode ==
       CAM_TONEMAP_MODE_CONTRAST_CURVE) {
      isp_sub_module->submod_trigger_enable = FALSE;
      if (isp_sub_module->manual_ctrls.update_gamma) {
        /* Apply the manual gamma curve from the app */
        ret = gamma44_manual_update(module,isp_sub_module, event);
        isp_sub_module->manual_ctrls.update_gamma = FALSE;
        goto FILL_METADATA;
      }
    } else {
      /* For special effect we need to keep trigger updates off */
      if (gamma->effects.spl_effect == CAM_EFFECT_MODE_OFF) {
        isp_sub_module->submod_trigger_enable = TRUE;
      }
      isp_sub_module->trigger_update_pending = TRUE;
      if (isp_sub_module->manual_ctrls.update_gamma) {
        ISP_ERR("Manual gamma curve not applied");
      }
    }
  }

  /* Reset config_pending */
  isp_sub_module->config_pending = FALSE;

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("sub mod enable = %d", isp_sub_module->submod_enable);
    goto FILL_METADATA;
  }

  if(isp_sub_module->trigger_update_pending == TRUE &&
     isp_sub_module->submod_trigger_enable == TRUE) {

    ISP_DBG("ratio %f lighting %d",
       gamma->gamma_ratio.ratio, gamma->gamma_ratio.lighting);

    if (gamma->ext_func_table && gamma->ext_func_table->ext_interpolate) {
      ret = gamma->ext_func_table->ext_interpolate(gamma, isp_sub_module, NULL);
    } else {
      ISP_ERR("failed: %p", gamma->ext_func_table);
      ret = FALSE;
    }
    if (FALSE == ret) {
      ISP_ERR("failed");
      goto end;
    }

    if (gamma->enable_adrc) {
      ret = gamma_update_adrc_gamma_curve(&gamma->gamma_algo);
      if (ret == FALSE) {
        ISP_ERR("failed: gamma_update_adrc_gamma_curve");
      }
    }

    ISP_DBG(": bklight %d severity %d",  backlight_comp_update,
      gamma->asd_update.backlight_scene_severity);

    if (gamma->backlight_comp_update) {
      /* TODO rescale this to divide by new max value */
      gamma->backlight_severity =
        gamma->asd_update.backlight_scene_severity;
    }

    /* update based on contrast */
    if (gamma->bestshot_mode == CAM_SCENE_MODE_OFF) {
      ISP_DBG(": Apply contrast %d", gamma->effects.contrast);
      if (gamma->effects.contrast != DEFAULT_CONTRAST) {
        gamma44_update_tab_on_contrast(gamma, gamma->effects.contrast,
          &(gamma->gamma_table_rgb),&contrast_gamma_table_rgb);
        gamma44_update_vfe_table(gamma, &gamma->ISP_GammaCfgCmd,
          &contrast_gamma_table_rgb);
      } else {
        gamma44_update_vfe_table(gamma, &gamma->ISP_GammaCfgCmd,
          &gamma->gamma_table_rgb);
      }
    }

    /*Populate and store hw update params inside isp_sub_mod.
        then using utility function append this isp_sub_mod's hw update list
        into global hw update list*/
    ret = gamma44_store_hw_update(isp_sub_module, gamma);
    if (ret == FALSE) {
      ISP_ERR("failed: gamma_do_hw_update");
      goto end;
    }
  } else if (isp_sub_module->trigger_update_pending == TRUE &&
     isp_sub_module->submod_trigger_enable == FALSE) {
      isp_sub_module->trigger_update_pending == FALSE;
      ret = gamma44_store_hw_update(isp_sub_module, gamma);
       if (ret == FALSE) {
         ISP_ERR("failed: gamma_do_hw_update");
         goto end;
       }
  }
  /* append hw update list of gamma module to global list*/
  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list mod %s",
      MCT_MODULE_NAME(module));
    goto end;
  }

FILL_METADATA:
  if (output->meta_dump_params &&
    output->metadata_dump_enable == 1) {
    /*fill in DMI info*/
    gamma_dmi_info = &output->meta_dump_params->meta_entry[ISP_META_GAMMA_TBL];
    /*dmi table length*/
    gamma_dmi_info->len = sizeof(uint32_t) * ISP_GAMMA_NUM_ENTRIES * 3;
    /*dmi type */
    gamma_dmi_info->dump_type  = ISP_META_GAMMA_TBL;
    gamma_dmi_info->start_addr = 0;
    output->meta_dump_params->frame_meta.num_entry++;

#ifdef DYNAMIC_DMI_ALLOC
    gamma_dmi_tbl_r = (uint32_t *)CAM_MALLOC(gamma_dmi_info->len);
    if (!gamma_dmi_tbl_r) {
      ISP_ERR("failed: %p", gamma_dmi_tbl_r);
      ret = FALSE;
      goto end;
    }
#endif
    if (gamma_dmi_info->len <= sizeof(gamma_dmi_info->isp_meta_dump)) {
      gamma44_send_meta_data(gamma_dmi_info, gamma);
    } else {
      ISP_ERR("failed memcpy: hw_dmi_tbl size %d is less than gamma hw table %d",
        gamma_dmi_info->len, sizeof(gamma_dmi_info->isp_meta_dump));
      ret = FALSE;
      goto end;
    }
    //gamma_dmi_info->hw_dmi_tbl = gamma_dmi_tbl_r;

    /* TODO: fill proper value when available*/
   output->meta_dump_params->frame_meta.se_lux_idx =
      gamma->aec_update.lux_idx;

   output->meta_dump_params->frame_meta.adrc_info.gamma_ratio =
     gamma->aec_update.gamma_ratio;
   output->meta_dump_params->frame_meta.adrc_info.reserved_data[0] =
     gamma->aec_update.total_drc_gain;
  }

  /* reuse gamma_dmi_info for other meta dmi tbl posting */
  /* This meta entry preparation is for HDR library and not meta dump*/
  applied_dmi_tbl = &output->dmi_tbl->dmi_info[ISP_METADUMP_GAMMA];
  applied_dmi_tbl->read_length = sizeof(gamma_table_t);
  /* number of bytes each gamma entry is*/
  applied_dmi_tbl->entry_size = sizeof(uint32_t);
  applied_dmi_tbl->num_entries = ISP_GAMMA_NUM_ENTRIES;
  applied_dmi_tbl->pack_bits= GAMMA_HW_PACK_BIT;
  gamma_hw_table = (gamma_table_t *)applied_dmi_tbl->hw_dmi_tbl;
  memcpy(gamma_hw_table, &(gamma->ISP_GammaCfgCmd.Gamatbl),
    sizeof(gamma_table_t));
  ISP_DBG(" applied_dmi_tbl->read_lengh %d table %p",
    applied_dmi_tbl->read_length, applied_dmi_tbl->hw_dmi_tbl);

  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG && output->hdr_data) {
    chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
    chromatix_VHDR = &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;
    chromatix_HDR_reserve = &chromatix_VHDR->videoHDR_reserve_data;

    output->hdr_data->msb_mode = chromatix_HDR_reserve->hdr_MSB_align;
    output->hdr_data->gamma_table.is_enable = TRUE;
    output->hdr_data->gamma_table.size = ISP_GAMMA_NUM_ENTRIES;
    memcpy(&output->hdr_data->gamma_table.hw_table_g, &gamma->gamma_table_rgb.gamma_g,
    sizeof(gamma->gamma_table_rgb.gamma_g));
  }

  if (output->frame_meta) {
    gamma44_update_tone_map_curves(output->frame_meta,
      gamma, isp_sub_module->manual_ctrls.tonemap_mode);
  }
end:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* gamma44_trigger_update */

/** gamma44_streamon:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: streamon payload
 *
 *  Initialize LUT bank
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gamma44_streamon(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                ret = TRUE;
  gamma44_t             *gamma = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  gamma = (gamma44_t *)isp_sub_module->private_data;
  if (!gamma) {
    ISP_ERR("failed: %p", gamma);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  /*config gamma HW table and configuration*/
  gamma->ISP_GammaCfgCmd.LutSel.ch0BankSelect = 0;
  gamma->ISP_GammaCfgCmd.LutSel.ch1BankSelect = 0;
  gamma->ISP_GammaCfgCmd.LutSel.ch2BankSelect = 0;

  isp_sub_module->manual_ctrls.tonemap_mode = CAM_TONEMAP_MODE_FAST;
  isp_sub_module->manual_ctrls.update_gamma = FALSE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

}

#if !OVERRIDE_FUNC
/** gamma44_ext_interpolate:
 *
 *  @gamma: gamma module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean gamma44_ext_interpolate(
    void *data1,
    void *data2 __unused,
    void *data3 __unused)
{
  gamma44_t *gamma = NULL;

  if (!data1) {
    ISP_ERR("failed: %p", data1);
    return FALSE;
  }
  gamma = (gamma44_t *)data1;

  if (gamma->gamma_ratio.lighting == TRIGGER_OUTDOOR)
    gamma44_interpolate(gamma, &gamma->default_gamma_tbl,
      &gamma->outdoor_gamma_tbl, &(gamma->gamma_table_rgb),
      gamma->gamma_ratio.ratio);
  else if (gamma->gamma_ratio.lighting == TRIGGER_LOWLIGHT)
    gamma44_interpolate(gamma, &gamma->default_gamma_tbl,
      &gamma->lowlight_gamma_tbl, &(gamma->gamma_table_rgb),
      gamma->gamma_ratio.ratio);
  else
    gamma44_copy_gamma(gamma, &(gamma->gamma_table_rgb),
      &gamma->default_gamma_tbl, ISP_GAMMA_NUM_ENTRIES);
  return TRUE;
}

/** gamma44_get_trigger_ratio:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean gamma44_get_trigger_ratio(void *data1,
  void *data2, void *data3)
{
  boolean ret = TRUE;
  trigger_ratio_t          trigger_ratio;
  trigger_point_type      *lowlight = NULL;
  trigger_point_type      *outdoor = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_gamma_type    *chromatix_gamma = NULL;
  aec_update_t            *new_aec_update = NULL;
  aec_update_t            *cur_aec_update = NULL;
  gamma44_t               *gamma = NULL;
  isp_sub_module_t        *isp_sub_module = NULL;

  if (!data1|| !data2 || !data3) {
    ISP_ERR("failed: %p %p %p", data1, data2, data3);
    return FALSE;
  }
  gamma = (gamma44_t *)data1;
  isp_sub_module = (isp_sub_module_t *) data2;

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    isp_sub_module->trigger_update_pending = TRUE;
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  chromatix_gamma = &chromatix_ptr->chromatix_VFE.chromatix_gamma;
  ISP_DBG("control_gamma %d", chromatix_gamma->control_gamma);
  new_aec_update = (aec_update_t *)data3;
  cur_aec_update = &gamma->aec_update;

  lowlight =  &(chromatix_gamma->gamma_lowlight_trigger);
  outdoor = &(chromatix_gamma->gamma_outdoor_trigger);

  ret = isp_sub_module_util_get_aec_ratio_bright_low(
   (unsigned char)chromatix_gamma->control_gamma, outdoor,
   lowlight, new_aec_update, &trigger_ratio, 0);

  if (!F_EQUAL(trigger_ratio.ratio, gamma->gamma_ratio.ratio) ||
      gamma->backlight_comp_update ||
      (trigger_ratio.lighting != gamma->gamma_ratio.lighting)) {
    isp_sub_module->trigger_update_pending = TRUE;
    *cur_aec_update = *new_aec_update;
  }
  ISP_DBG("ratio %f lighting %d",
    trigger_ratio.ratio, trigger_ratio.lighting);
  gamma->gamma_ratio = trigger_ratio;
  return ret;
}

static ext_override_func gamma_override_func = {
  .ext_interpolate       = gamma44_ext_interpolate,
  .get_trigger_ratio     = gamma44_get_trigger_ratio,
  .ext_set_table         = NULL,
  .ext_set_spl_effect    = NULL,
};

boolean gamma44_fill_func_table(gamma44_t *gamma)
{
  gamma->ext_func_table = &gamma_override_func;
  return TRUE;
}
#endif
