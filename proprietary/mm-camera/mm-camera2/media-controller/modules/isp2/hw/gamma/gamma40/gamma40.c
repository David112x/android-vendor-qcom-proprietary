/* gamma40.c
 *
 * Copyright (c) 2013-2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_GAMMA, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_GAMMA, fmt, ##args)

/* isp headers */
#include "module_gamma.h"
#include "isp_sub_module_util.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"

#define GAMMA_CHROMATIX_TBL_SIZE 14
#define ROUND(a)((a >= 0) ? (long)(a + 0.5) : (long)(a - 0.5))

/*#define GAMMA_DEBUG */
#ifdef GAMMA_DEBUG
#define PRINT_REG_VAL_SET
#endif


#define TBL_INTERPOLATE_GAMMA(in1, in2, out, ratio, size, i) ({\
    for (i=0; i<size; i++) \
      out[i] = roundf((float)LINEAR_INTERPOLATION(in1[i], in2[i], ratio)); })
/* Macro defines*/
#define PRINT_TABLE(table) ({ \
  int i; \
  ISP_DBG("gamma table from %s", #table); \
  for (i=0; i<ISP_GAMMA_NUM_ENTRIES; i++) \
    ALOGE("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", \
      table[i*16], table[i*16+1], table[i*16+2], table[i*16+3], table[i*16+4], \
      table[i*16+5], table[i*16+6], table[i*16+7], table[i*16+8], \
      table[i*16+9], table[i*16+10], table[i*16+11], table[i*16+12], \
      table[i*16+13], table[i*16+14], table[i*16+15]); })

/* Forward declaration */
boolean gamma40_init_config(isp_sub_module_t *isp_sub_module,
gamma40_t *gamma);
static boolean gamma40_store_hw_update(isp_sub_module_t *isp_sub_module,
	gamma40_t *gamma);


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
boolean gamma40_get_interpolated_table(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event) {
  boolean                       ret = TRUE;
  gamma40_t                    *gamma = NULL;
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

  gamma = (gamma40_t *)isp_sub_module->private_data;
  if (!gamma) {
    ISP_ERR("failed: gamma %p", gamma);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  /* Copy interpolated gamma table*/
  memcpy(gamma_table_g, &gamma->gamma_table_rgb.gamma_g,
         ISP_GAMMA_NUM_ENTRIES * sizeof(uint8_t));


  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** gamma40_get_vfe_diag_info_user:
*
*  @module: mct module handle
*  @isp_sub_module: isp sub module handle
*	@event: mct event handle
*
*	This function populates vfe diag data
*
*  Return: TRUE on success and FALSE on failure
*/
boolean gamma40_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
  {
    ISP_ERR("TODO: not implemented yet");
    return TRUE;
  }

/** gamma40_set_chromatix_ptr
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration using default values from
 *  chromatix ptr
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gamma40_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean    ret = TRUE;
  gamma40_t  *gamma = NULL;
  modulesChromatix_t  *chromatix_ptrs = NULL;

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

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }

  gamma = (gamma40_t *)isp_sub_module->private_data;
  if (!gamma) {
    ISP_ERR("failed: mod %p", gamma);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  /* Update streaming mode mask in module private */
  /* mct_list_traverse(isp_sub_module->l_stream_info,
  isp_sub_module_util_update_streaming_mode, &gamma->streaming_mode_mask);*/

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("gamma enable = %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  ret = gamma40_init_config(isp_sub_module, gamma);

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}
/** gamma40_stats_aec_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *	@event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean gamma40_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret = TRUE;
  stats_update_t          *stats_update = NULL;
  aec_update_t            *cur_aec_update = NULL;
  aec_update_t            *new_aec_update = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_gamma_type    *chromatix_gamma = NULL;
  trigger_point_type      *lowlight = NULL;
  trigger_point_type      *outdoor = NULL;
  gamma40_t               *gamma = NULL;
  trigger_ratio_t          trigger_ratio;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  gamma = (gamma40_t *)isp_sub_module->private_data;
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
  cur_aec_update = &gamma->aec_update;
  new_aec_update = &stats_update->aec_update;

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
  ISP_DBG("ratio %f lighting %d",
    trigger_ratio.ratio, trigger_ratio.lighting);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** gamma40_stats_asd_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 * Handle ASD update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean gamma40_stats_asd_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  stats_update_t   *stats_update = NULL;
  asd_update_t     *cur_asd = NULL;
  asd_update_t     *new_asd = NULL;
  gamma40_t        *gamma = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  gamma = (gamma40_t *)isp_sub_module->private_data;
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

/** gamma40_apply_contrast_sigmoid:
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
static int16_t gamma40_apply_contrast_sigmoid(double sigmoid, double multiplier,
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

/** gamma40_compute_contrast_table:
 *
 *    @sigmoid: sigmoid is curvature strength to be applied. This values depends
 *                  on contrast value from user.
 *    @lut_size: look up table size
 *    @input_table: input rgb table
 *    @output_table: output rgb table
 *
 *  This function updates gamma table depending on contrast
 *
 *  Return: TRUE  - Success
 *          FALSE - Input/Output table is NULL or input values are invalid
 **/
static boolean gamma40_compute_contrast_table(double sigmoid, int lut_size,
  gamma_rgb_ds_t  *input_table_rgb, gamma_rgb_ds_t  *output_table_rgb)
{
  int             i = 0, x = 0;
  uint8_t        *gamma_table = NULL;
  int             entries = (lut_size >= 0) ? lut_size : ISP_GAMMA_NUM_ENTRIES;
  int             mid_pt = 0;
  if (input_table_rgb == NULL || output_table_rgb == NULL) {
    ISP_ERR(": input improper");
    return FALSE;
  }

  /* Store AEC update in module private */
  mid_pt = isp_sub_module_util_power(2.0, GAMMA_CHROMATIX_PACK_BIT) / 2;
  const double multiplier =
    isp_sub_module_util_power((double)mid_pt, (1.0 - sigmoid));

  ISP_DBG("sigmoid %f mid_pt %d multiplier %f entries %d",
    sigmoid, mid_pt, multiplier, entries);
  for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
    output_table_rgb->gamma_r[i] = gamma40_apply_contrast_sigmoid(sigmoid,
      multiplier, input_table_rgb->gamma_r[i], mid_pt);
    output_table_rgb->gamma_g[i] = gamma40_apply_contrast_sigmoid(sigmoid,
      multiplier, input_table_rgb->gamma_g[i], mid_pt);
    output_table_rgb->gamma_b[i] = gamma40_apply_contrast_sigmoid(sigmoid,
      multiplier, input_table_rgb->gamma_b[i], mid_pt);
	  /* 8994 has 512 entries with output 14bits.
     * 8084 has 256 gamma entries with output 12bits, SW needs
     * to downsample gamma by 2 and right shift 2 bits of the table values
     */
    ISP_DBG("%d: input %d output %d", i, input_table_rgb->gamma_r[i],
      output_table_rgb->gamma_r[i]);
  }
  return TRUE;
}

/** gamma40_update_tab_on_contrast:
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
static uint16_t gamma40_update_tab_on_contrast(int contrast,
  gamma_rgb_ds_t  *input_table_rgb, gamma_rgb_ds_t  *output_table_rgb)
{
  int             i = 0;
  const uint8_t  *gamma_table = NULL;
  //int             entries = (lut_size >= 0) ? lut_size : ISP_GAMMA_NUM_ENTRIES;
  double          sigmoid = 0.0;

  /* Apply Sigmoid Gamma Table */
  if (input_table_rgb == NULL || output_table_rgb == NULL) {
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
  gamma40_compute_contrast_table(sigmoid, ISP_GAMMA_NUM_ENTRIES,
    input_table_rgb, output_table_rgb);
#endif

  return TRUE;
} /* gamma40_update_tab_on_contrast */

/** get_gamma_down_sampled_table
 *
 *  @table: gamma table
 *  @skipRatio: number of entries to be skipped
 *  @i: index in gamma table
 *
 * The 8 significant bits from chromatix values will be used
 * also Down sample the 512 entries to 64 entries by
 * skipping the 8 entries
 */
void gamma40_get_downsampled_table(uint8_t *ispTable,
  uint16_t *table,
  int skipRatio)
{
  int i;

     //Down sample the 512 entries to 64 entries by skipping the 8 entries
     for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
       ispTable[i] = (uint8_t)(table[skipRatio * i] >> 6);
     }
}

/** gamma40_get_hi_lo_gamma_bits
 *
 *  @table: gamma table
 *  @skipRatio: number of entries to be skipped
 *  @i: index in gamma table
 *
 *    The higher 12 bits in the configuration contains the delta between the
 *    current GammaTable value and the next value, while the lower 12 bits
 *    contains the current GammaTable value
 */
static int16_t gamma40_get_hi_lo_gamma_bits(uint8_t *table,
  int i)
{
  int16_t       hw_lut_entry = 0;
  uint8_t      delta_lut = 0;

  delta_lut = (table[(i+1)] - table[i]);

  hw_lut_entry = (int16_t)((delta_lut << GAMMA_HW_PACK_BIT) + (table[i]));
  return hw_lut_entry;
}

/** gamma40_get_last_gamma_value
 *
 *  @table: gamma table
 *  @skipRatio: number of entries to be skipped
 *
 */
static int16_t gamma40_get_last_gamma_value(uint8_t *table)
{
  int16_t       hw_lut_entry = 0;
  uint8_t      delta_lut = 0;
  /* consider the next entry of the last entry 2^12- 8084.
                                               2^14- 8994.
        value suggested by system team */
  uint16_t next_entry_final = (uint16_t) isp_sub_module_util_power(2.0, GAMMA_HW_PACK_BIT);

  /* this is effectively  table[255] - table[254];
        this part is the delta */
  /* use 256 as next entry of last entry */
  delta_lut = next_entry_final -
    (table[(ISP_GAMMA_NUM_ENTRIES - 1)]);
  /* scale the delta */

  /* form the value:  upper byte is delta,
        lower byte is the entry itself. */
  hw_lut_entry = (int16_t)(delta_lut <<  GAMMA_HW_PACK_BIT) +
    (table[(ISP_GAMMA_NUM_ENTRIES - 1)]);
  return hw_lut_entry;
}

/** gamma40_update_vfe_table:
 *    @gamma_config_cmd: configuration command
 *    @table_rgb: pointer to table
 *    @size: size of the table
 *
 *  This function updates gamma table depending on contrast
 *
 *  Return: TRUE  - Success
 *             FALSE - Input/Output table is NULL or contrast value is invalid
 **/

static void gamma40_update_vfe_table(volatile ISP_GammaConfigCmdType *
  gamma_config_cmd, gamma_rgb_ds_t *table_rgb)
{
  int             i = 0;
  int             skipRatio = 0;

  if (!table_rgb) {
    ISP_ERR("failed: table_rgb %p", table_rgb);
    return;
  }
  uint8_t *table_r = &table_rgb->gamma_r[0];
  uint8_t *table_g = &table_rgb->gamma_g[0];
  uint8_t *table_b = &table_rgb->gamma_b[0];
  //Finally compute the LUT entries
  for (i = 0; i < ISP_GAMMA_NUM_ENTRIES - 1; i++) {
    gamma_config_cmd->Gamatbl.hw_table_r[i] =
      gamma40_get_hi_lo_gamma_bits(table_r, i);
    gamma_config_cmd->Gamatbl.hw_table_g[i] =
      gamma40_get_hi_lo_gamma_bits(table_g, i);
    gamma_config_cmd->Gamatbl.hw_table_b[i] =
      gamma40_get_hi_lo_gamma_bits(table_b, i);
  }
  gamma_config_cmd->Gamatbl.hw_table_r[ISP_GAMMA_NUM_ENTRIES - 1] =
    gamma40_get_last_gamma_value(table_r);
  gamma_config_cmd->Gamatbl.hw_table_g[ISP_GAMMA_NUM_ENTRIES - 1] =
    gamma40_get_last_gamma_value(table_g);
  gamma_config_cmd->Gamatbl.hw_table_b[ISP_GAMMA_NUM_ENTRIES - 1] =
    gamma40_get_last_gamma_value(table_b);
} /* gamma40_update_vfe_table */

/** set_gamma40_table
 *
 *  @p_gamma_rgb: contain pointers to each color of gamma
 *  @gamma: single gamma table
 *
 *  This function initializes rgb gamma pointers
 *
 *  Return: None
 */
static void gamma40_set_gamma_table(gamma_rgb_t  *gamma_rgb,
  uint16_t *gamma)
{
  if (!gamma || !gamma_rgb) {
    ISP_ERR("failed: %p %p", gamma, gamma_rgb);
    return;
  }
  memcpy(gamma_rgb->gamma_r, gamma, ISP_GAMMA_NUM_ENTRIES * sizeof(uint16_t));
  memcpy(gamma_rgb->gamma_g, gamma, ISP_GAMMA_NUM_ENTRIES * sizeof(uint16_t));
  memcpy(gamma_rgb->gamma_b, gamma, ISP_GAMMA_NUM_ENTRIES * sizeof(uint16_t));
}


/** copy_gamma
 *
 *  @dst: destination
 *  @src: src
 *  @size: size
 *
 *  This function copies rgb gamma table
 *
 *  Return: None
 */
void gamma40_copy_gamma(gamma_rgb_ds_t *dst,
  gamma_rgb_ds_t *src, size_t size)
{
  if (!dst || !src) {
    ISP_ERR("failed: %p %p", dst, src);
    return;
  }
  memcpy(dst->gamma_r, src->gamma_r, size * sizeof(uint8_t));
  memcpy(dst->gamma_g, src->gamma_g, size * sizeof(uint8_t));
  memcpy(dst->gamma_b, src->gamma_b, size * sizeof(uint8_t));
}

/**   chromatix_to_isp_gamma
 *
 *  @p_gamma_rgb: isp gamma rgb table
 *  @chromatix_gamma40_table: chromoatix gamma table
 *
 *  This function initializes gamma table from chromatix
 *
 *  Return: None
 */
void gamma40_chromatix_to_isp_gamma(gamma_rgb_t *gamma_rgb,
  chromatix_gamma_table_type *chromatix_gamma_table)
{
  int i = 0;

  if (!gamma_rgb || !chromatix_gamma_table) {
    ISP_ERR("failed: %p %p", gamma_rgb, chromatix_gamma_table);
    return;
  }

  memcpy(gamma_rgb->gamma_r, chromatix_gamma_table->gamma_R,
    GAMMA_CHROMATIX_LUT_SIZE * sizeof(uint16_t));
  memcpy(gamma_rgb->gamma_g, chromatix_gamma_table->gamma_G,
    GAMMA_CHROMATIX_LUT_SIZE * sizeof(uint16_t));
  memcpy(gamma_rgb->gamma_b, chromatix_gamma_table->gamma_B,
    GAMMA_CHROMATIX_LUT_SIZE * sizeof(uint16_t));
}

/** gamma40_interpolate
 *
 *    @tbl1: input table 1
 *    @tbl2: input table 2
 *    @out: out table
 *    @ratio:
 *
 *  Interpolate for each of tree colors RGB
 *  and summurise result in one table
 *
 *  Return: None
 */
void gamma40_interpolate(gamma_rgb_ds_t  *tbl1,
    gamma_rgb_ds_t  *tbl2, gamma_rgb_ds_t  *gamma_rgb,
  float ratio)
{
  int   i = 0;
  if (!tbl1 || !tbl2 || !gamma_rgb) {
    ISP_ERR("failed: %p %p %p", tbl1, tbl2, gamma_rgb);
    return;
  }

  TBL_INTERPOLATE_GAMMA(tbl1->gamma_r, tbl2->gamma_r, gamma_rgb->gamma_r,
    ratio, ISP_GAMMA_NUM_ENTRIES, i);
  TBL_INTERPOLATE_GAMMA(tbl1->gamma_g, tbl2->gamma_g, gamma_rgb->gamma_g,
    ratio, ISP_GAMMA_NUM_ENTRIES, i);
  TBL_INTERPOLATE_GAMMA(tbl1->gamma_b, tbl2->gamma_b, gamma_rgb->gamma_b,
    ratio, ISP_GAMMA_NUM_ENTRIES, i);
}

/** gamma40_set_table:
 *    @gamma: gamma module instance
 *
 *  This function sets gamma table depending on special effect/ bestshot.
 *
 *  Return:   0 - Success
 **/
boolean gamma40_set_table(isp_sub_module_t *isp_sub_module,
  gamma40_t* gamma, gamma_table_type gamma_table_type)
{
  int rc =0;
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
    gamma40_copy_gamma(&(gamma->gamma_table_rgb), &gamma->outdoor_gamma_tbl,
                                          ISP_GAMMA_NUM_ENTRIES);

      break;

  case GAMMA_TABLE_LOWLIGHT:
    gamma40_copy_gamma(&(gamma->gamma_table_rgb), &gamma->lowlight_gamma_tbl,
                                          ISP_GAMMA_NUM_ENTRIES);

      break;

    case GAMMA_TABLE_WHITE_BOARD:
      gamma->gamma_lut_size = 1024;
       gamma40_get_downsampled_table(&gamma->gamma_table_rgb.gamma_r[0],
        (uint16_t *)VFE_Whiteboard_GammaTable,
          gamma->gamma_lut_size/ISP_GAMMA_NUM_ENTRIES);
      gamma40_get_downsampled_table(&gamma->gamma_table_rgb.gamma_g[0],
        (uint16_t *)VFE_Whiteboard_GammaTable,
           gamma->gamma_lut_size/ISP_GAMMA_NUM_ENTRIES);
      gamma40_get_downsampled_table(&gamma->gamma_table_rgb.gamma_b[0],
        (uint16_t *)VFE_Whiteboard_GammaTable,
           gamma->gamma_lut_size/ISP_GAMMA_NUM_ENTRIES);
      break;

    case GAMMA_TABLE_BLACK_BOARD:
      gamma->gamma_lut_size = 1024;
      gamma40_get_downsampled_table(&gamma->gamma_table_rgb.gamma_r[0],
        (uint16_t *)VFE_Blackboard_GammaTable,
           gamma->gamma_lut_size/ISP_GAMMA_NUM_ENTRIES);
      gamma40_get_downsampled_table(&gamma->gamma_table_rgb.gamma_g[0],
        (uint16_t *)VFE_Blackboard_GammaTable,
           gamma->gamma_lut_size/ISP_GAMMA_NUM_ENTRIES);
      gamma40_get_downsampled_table(&gamma->gamma_table_rgb.gamma_b[0],
        (uint16_t *)VFE_Blackboard_GammaTable,
           gamma->gamma_lut_size/ISP_GAMMA_NUM_ENTRIES);
      break;

    case GAMMA_TABLE_DEFAULT:
    default: {
      /* Check if ext table type is supported */
      if (gamma->ext_func_table && gamma->ext_func_table->ext_set_table)
        rc = gamma->ext_func_table->ext_set_table(gamma, isp_sub_module, &gamma_table_type);
      else
        rc = FALSE;

      if (rc == FALSE) {
        /* No supported table / ext table type found, hence use default*/
        gamma40_copy_gamma(&(gamma->gamma_table_rgb),
          &gamma->default_gamma_tbl,ISP_GAMMA_NUM_ENTRIES);
      }
    }
      break;
  }
  gamma->gamma_table_type = gamma_table_type;
  isp_sub_module->trigger_update_pending = TRUE;

  return TRUE;
} /* vfe_gamma_set_table */

/** gamma40_set_bestshot:
 *    @gamma: gamma module instance
 *    @isp_sub_module: base module instance
 *  This function configures gamma module according bestshot set.
 *
 *  Return:   0 - Success
 *           -1 - Parameter size mismatch
 **/
boolean gamma40_set_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                   ret = TRUE;
  int32_t                  *bestshot = NULL;
  gamma40_t                *gamma = NULL;
  mct_event_control_t      *ctrl_event = NULL;
  mct_event_control_parm_t *param = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  gamma = (gamma40_t *)isp_sub_module->private_data;
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
    ret = gamma40_set_table(isp_sub_module, gamma, GAMMA_TABLE_OUTDOOR);
    break;

  case CAM_SCENE_MODE_NIGHT:
  case CAM_SCENE_MODE_FIREWORKS:
  case CAM_SCENE_MODE_NIGHT_PORTRAIT:
    ret = gamma40_set_table(isp_sub_module, gamma, GAMMA_TABLE_LOWLIGHT);
    break;

  case CAM_SCENE_MODE_BACKLIGHT:
    ret = gamma40_set_table(isp_sub_module, gamma, GAMMA_TABLE_BACKLIGHT);
    break;

  default:
    gamma->bestshot_mode = CAM_SCENE_MODE_OFF;
    isp_sub_module->submod_trigger_enable = TRUE;
    ret = TRUE;
    break;
  }

  if (ret == TRUE && isp_sub_module->submod_trigger_enable == FALSE){
    ISP_DBG(": enable update through BSM for mode : %d",
      gamma->bestshot_mode);
    gamma40_update_vfe_table(&gamma->ISP_GammaCfgCmd,
          &gamma->gamma_table_rgb);
    isp_sub_module->trigger_update_pending = TRUE;
  } else {
    ISP_DBG("failed: set table for bestshot %d, reset to bestshot off",
      gamma->bestshot_mode);
    gamma->bestshot_mode = CAM_SCENE_MODE_OFF;
    isp_sub_module->submod_trigger_enable  = TRUE;
    isp_sub_module->trigger_update_pending = TRUE;
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* vfe_gamma_set_bestshot */

/** gamma40_set_spl_effect:
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
boolean gamma40_set_spl_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                   ret = TRUE;
  int                       type = 0;
  gamma40_t                *gamma = NULL;
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

  gamma = (gamma40_t *)isp_sub_module->private_data;
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
  gamma->effect_applied = FALSE;
  if (gamma->bestshot_mode != CAM_SCENE_MODE_OFF) {
    ISP_HIGH(": Best shot enabled, skip set effect");
    return TRUE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  type = gamma->effects.spl_effect;
  ISP_DBG(": contrast %d effect type %d",  gamma->effects.contrast, type);

  /* Special effect and contrast are mutually exclusive. Special effect has
     higher priority. Thus when special effect is applied, contrast is not
     applied and set to default */
  if(gamma->effects.contrast != DEFAULT_CONTRAST)
    switch(type) {
    case CAM_EFFECT_MODE_POSTERIZE:
    case CAM_EFFECT_MODE_SOLARIZE: {
      /* UI should prevent this */
      ISP_ERR(": Warning Overriding contrast value to default");
      gamma->effects.contrast = DEFAULT_CONTRAST;
    }
      break;
    default:
      break;
    }

  isp_sub_module->submod_trigger_enable = FALSE;
  ISP_DBG(": type %d",  type);
  switch (type) {
  case CAM_EFFECT_MODE_BLACKBOARD: {
    gamma->effect_applied = TRUE;
    ret = gamma40_set_table(isp_sub_module, gamma, GAMMA_TABLE_BLACK_BOARD);
    break;
  }

  case CAM_EFFECT_MODE_WHITEBOARD: {
    gamma->effect_applied = TRUE;
    ret = gamma40_set_table(isp_sub_module, gamma, GAMMA_TABLE_WHITE_BOARD);
    break;
  }

  default:
    /* Check if ext spl effect is supported */
    gamma->effects.spl_effect = CAM_EFFECT_MODE_OFF;
    if (gamma->ext_func_table && gamma->ext_func_table->ext_set_spl_effect)
      ret = gamma->ext_func_table->ext_set_spl_effect(gamma, isp_sub_module, &type);
    else
      ret = gamma40_set_table(isp_sub_module, gamma, GAMMA_TABLE_DEFAULT);

    ret = TRUE;
    isp_sub_module->submod_trigger_enable = TRUE;
    isp_sub_module->trigger_update_pending = TRUE;
    break;
  }

  if (ret == TRUE && isp_sub_module->submod_trigger_enable == FALSE) {
    ISP_ERR(": enable update through special effect type %d", type);
    gamma40_update_vfe_table(&gamma->ISP_GammaCfgCmd, &gamma->gamma_table_rgb);
    isp_sub_module->trigger_update_pending = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* gamma40_set_spl_effect */

/** gamma40_set_solarize_table:
 *
 *    @gamma: gamma module instance
 *    @chromatix_ptr: pointer to chromatix
 *
 *  This function sets solarize table
 *
 *  Return:   None
 **/
static void gamma40_set_solarize_table(gamma40_t *gamma,
  chromatix_parms_type *chromatix_ptr)
{
  /* FIXME: There is no solorize reflection point in chromatix 301 */
  int                   i = 0;
  uint16_t              solarize_reflection_point = 32;
  uint16_t              gamma_max_value = 0x3FFF;
  gamma_rgb_ds_t           Gamma_Lut;
  chromatix_gamma_type *pchromatix_gamma = NULL;

  if (!gamma || !chromatix_ptr) {
    ISP_ERR("failed: gamma %p ctrl_event %p", gamma, chromatix_ptr);
    return;
  }
  pchromatix_gamma =
    &(chromatix_ptr->chromatix_VFE.chromatix_gamma);

  /* Chromatix reflection point is based on 256 (8 bit) gamma entries,
   * whereas VFE gamma has 1024 (10 bit) entries. So we multiple the
   * reflection point by 4 here */
  gamma40_copy_gamma(&Gamma_Lut, &gamma->default_gamma_tbl,
    ISP_GAMMA_NUM_ENTRIES);

  for (i = solarize_reflection_point - 1; i < ISP_GAMMA_NUM_ENTRIES; ++i) {
    gamma->solarize_gamma_table.gamma_r[i] =
      ((gamma_max_value - Gamma_Lut.gamma_r[i]) *
        Gamma_Lut.gamma_r[solarize_reflection_point - 1] /
        (gamma_max_value - Gamma_Lut.gamma_r[solarize_reflection_point - 1]));

    gamma->solarize_gamma_table.gamma_g[i] =
      ((gamma_max_value - Gamma_Lut.gamma_g[i]) *
        Gamma_Lut.gamma_g[solarize_reflection_point - 1] /
        (gamma_max_value - Gamma_Lut.gamma_g[solarize_reflection_point - 1]));

    gamma->solarize_gamma_table.gamma_b[i] =
      ((gamma_max_value - Gamma_Lut.gamma_b[i]) *
        Gamma_Lut.gamma_b[solarize_reflection_point - 1] /
        (gamma_max_value - Gamma_Lut.gamma_b[solarize_reflection_point - 1]));
  }
} /* vfe_gamma_get_table */

/** gamma40_set_contrast:
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
boolean gamma40_set_contrast(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                   ret = TRUE;
  int32_t                  *contrast = NULL;
  gamma40_t                *gamma = NULL;
  mct_event_control_t      *ctrl_event = NULL;
  mct_event_control_parm_t *param = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  gamma = (gamma40_t *)isp_sub_module->private_data;
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
} /* gamma40_set_effect */

/** gamma40_reset:
 *
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets gamma module to a known state
 *
 *  Return:   None
 **/
boolean  gamma40_reset(isp_sub_module_t *isp_submodule,
  gamma40_t *gamma)
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

  isp_submodule->trigger_update_pending    = FALSE;
  isp_submodule->submod_trigger_enable     = TRUE;
  isp_submodule->submod_enable             = TRUE;

  /*config gamma HW table and configuration*/
  gamma->ISP_GammaCfgCmd.LutSel.ch0BankSelect = 0;
  gamma->ISP_GammaCfgCmd.LutSel.ch1BankSelect = 0;
  gamma->ISP_GammaCfgCmd.LutSel.ch2BankSelect = 0;

  isp_submodule->manual_ctrls.tonemap_mode = CAM_TONEMAP_MODE_FAST;
  gamma->gamma_algo.gamma_tbl = (gamma_rgb_ds_t *)&gamma->gamma_table_rgb;
  gamma->gamma_algo.stats_update = &gamma->aec_update;
  return TRUE;
}/* gamma40_reset */

boolean gamma40_query_cap(mct_module_t *module,
  void *query_buf)
{
  mct_pipeline_cap_t *cap_buf;
  mct_pipeline_isp_cap_t *mod_cap = NULL;
  cap_buf = (mct_pipeline_cap_t *)query_buf;
  mod_cap = &cap_buf->isp_cap;
  mod_cap->max_gamma_curve_points = ISP_GAMMA_NUM_ENTRIES;
  return TRUE;
}
/** gamma40_init:
 *
 *    @module: mct module instance
 *    @isp_sub_module: base module
 *
 *  This function initializes gamma module
 *
 *  Return:   None
 **/
boolean gamma40_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  gamma40_t *gamma = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  gamma = (gamma40_t *)CAM_MALLOC(sizeof(gamma40_t));
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
  gamma40_reset(isp_sub_module, gamma);
  FILL_FUNC_TABLE(gamma);
  return TRUE;
}/* gamma40_init */

/** gamma40_destroy:
 *
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  This function destroys gamma module
 *
 *  Return: void
 **/
void gamma40_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  gamma40_t *gamma = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }
  gamma = (gamma40_t *)isp_sub_module->private_data;

  memset(gamma, 0, sizeof(gamma40_t));
  free(gamma);

  return;
} /* gamma40_destroy */

/** gamma40_init_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration of gamma module
 *
 *  Return:   TRUE - Success
 **/
boolean gamma40_init_config(isp_sub_module_t *isp_sub_module,
  gamma40_t *gamma)
{
  boolean ret = TRUE;
  chromatix_parms_type          *chromatix_ptr =
    isp_sub_module->chromatix_ptrs.chromatixPtr;
  chromatix_gamma_type          *pchromatix_gamma =
      &(chromatix_ptr->chromatix_VFE.chromatix_gamma);
  gamma_rgb_ds_t                    gammaTable;
  gamma_rgb_t                       gammaChromatixTable;
  int skipRatio = GAMMA_CHROMATIX_LUT_SIZE / ISP_GAMMA_NUM_ENTRIES;

  /*init gamma parameters*/
  gamma->gamma_table_type = GAMMA_TABLE_DEFAULT;
  gamma->enable_backlight_compensation = TRUE;

  /*load chromatix to gamma*/

  /* Get the downsampled low light gamma tables*/
  gamma40_chromatix_to_isp_gamma(&gammaChromatixTable,
    &(GAMMA_LOWLIGHT_TABLE(pchromatix_gamma)));

  gamma40_get_downsampled_table((&gamma->lowlight_gamma_tbl.gamma_r[0]),
    &(gammaChromatixTable.gamma_r[0]), skipRatio);
  gamma40_get_downsampled_table((&gamma->lowlight_gamma_tbl.gamma_g[0]),
    &(gammaChromatixTable.gamma_g[0]), skipRatio);
  gamma40_get_downsampled_table((&gamma->lowlight_gamma_tbl.gamma_b[0]),
    &(gammaChromatixTable.gamma_b[0]), skipRatio);

  /* Get the downsampled outdoor light gamma tables*/
  gamma40_chromatix_to_isp_gamma(&gammaChromatixTable,
    &(GAMMA_OUTDOOR_TABLE(pchromatix_gamma)));

  gamma40_get_downsampled_table((&gamma->outdoor_gamma_tbl.gamma_r[0]),
    &(gammaChromatixTable.gamma_r[0]), skipRatio);
  gamma40_get_downsampled_table((&gamma->outdoor_gamma_tbl.gamma_g[0]),
    &(gammaChromatixTable.gamma_g[0]), skipRatio);
  gamma40_get_downsampled_table((&gamma->outdoor_gamma_tbl.gamma_b[0]),
    &(gammaChromatixTable.gamma_b[0]), skipRatio);

  /* Get the downsampled default gamma tables*/
  gamma40_chromatix_to_isp_gamma(&gammaChromatixTable,
    &(GAMMA_DEFAULT_TABLE(pchromatix_gamma)));

  gamma40_get_downsampled_table(&(gamma->default_gamma_tbl.gamma_r[0]),
    &(gammaChromatixTable.gamma_r[0]), skipRatio);
  gamma40_get_downsampled_table(&(gamma->default_gamma_tbl.gamma_g[0]),
    &(gammaChromatixTable.gamma_g[0]), skipRatio);
  gamma40_get_downsampled_table(&(gamma->default_gamma_tbl.gamma_b[0]),
    &(gammaChromatixTable.gamma_b[0]), skipRatio);
  gamma->gamma_lut_size = GAMMA_CHROMATIX_LUT_SIZE;

  /*load chromatix to gamma*/
  gamma40_copy_gamma(&(gamma->gamma_table_rgb), &gamma->default_gamma_tbl,
    ISP_GAMMA_NUM_ENTRIES);

  gamma40_set_solarize_table(gamma, chromatix_ptr);

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
        gamma40_update_vfe_table(&gamma->ISP_GammaCfgCmd,
          &(gamma->gamma_table_rgb));
        break;
      default:
        /* apply contrast, update the original table*/
        if (gamma->effects.contrast != DEFAULT_CONTRAST) {
          gamma40_update_tab_on_contrast(gamma->effects.contrast,
            &(gamma->gamma_table_rgb),&gammaTable);
          gamma40_update_vfe_table(&gamma->ISP_GammaCfgCmd,
            &(gammaTable));
        } else {
          /* contrast 5, default table, use original table*/
          gamma40_update_vfe_table(&gamma->ISP_GammaCfgCmd,
            &(gamma->gamma_table_rgb));
        }
        break;
    }
  }else {
    switch(gamma->bestshot_mode) {
    case CAM_SCENE_MODE_LANDSCAPE:
    case CAM_SCENE_MODE_SNOW:
    case CAM_SCENE_MODE_BEACH:
    case CAM_SCENE_MODE_SPORTS:
    case CAM_SCENE_MODE_ACTION:
      ret = gamma40_set_table(isp_sub_module, gamma, GAMMA_TABLE_OUTDOOR);
      break;
    case CAM_SCENE_MODE_NIGHT:
    case CAM_SCENE_MODE_FIREWORKS:
    case CAM_SCENE_MODE_NIGHT_PORTRAIT:
      ret = gamma40_set_table(isp_sub_module, gamma, GAMMA_TABLE_LOWLIGHT);
      break;
    case CAM_SCENE_MODE_BACKLIGHT:
      ret = gamma40_set_table(isp_sub_module, gamma, GAMMA_TABLE_BACKLIGHT);
      break;
    default:
      ret = gamma40_set_table(isp_sub_module, gamma, GAMMA_TABLE_DEFAULT);
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
} /* gamma40_init_config */

/** gamma40_store_single_HW_write:
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
boolean gamma40_store_single_HW_write(void* cmd_offset,
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

/** gamma40_reset_dmi_cfg:
 * @mod_ctrl: gamma module instance
 * @gamma_channel: parameter id
 * @isp_sub_module: Base module object
 *
 *  Reset DMI table for gamma
 *
 *  Return:   0 - Success
 **/
static boolean gamma40_reset_dmi_cfg(gamma40_t *gamma,
  uint32_t gamma_channel, isp_sub_module_t *isp_sub_module)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  uint32_t                    *dmi_cfg = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  uint32_t                    *data = NULL;
  uint32_t                     len = 0;

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

} /* gamma40_reset_dmi_cfg */

/** gamma40_single_HW_write:
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
static boolean gamma40_store_dmi_info(void* cmd_offset,
  uint32_t cmd_len, uint32_t hw_reg_offset, uint32_t reg_num,
  uint32_t cmd_type, isp_sub_module_t *isp_sub_module)
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
} /* gamma40_single_HW_write */


/** gamma40_dmi_hw_update:
 *    @gamma: gamma module
 *    @bank_sel: mem bank select
 *    @isp_sub_module: base module
 *
 *  This function updates DMI tables for gamma
 *
 *  Return:   TRUE - Success FALSE- failure
 **/
static boolean gamma40_dmi_hw_update(gamma40_t *gamma,
  ISP_GammaLutSelect bank_sel, isp_sub_module_t *isp_sub_module)
{
  uint16_t tbl_len = sizeof(int16_t) * ISP_GAMMA_NUM_ENTRIES;
  boolean  ret;

  if (!gamma || !isp_sub_module) {
    ISP_ERR("failed: %p %p", gamma, isp_sub_module);
    return FALSE;
  }

  ret = isp_sub_module_util_write_dmi(
    (void*)gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_g, tbl_len,
    RGBLUT_RAM_CH0_BANK0 + bank_sel.ch0BankSelect,
    VFE_WRITE_DMI_16BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    return FALSE;
  }

  ret = isp_sub_module_util_write_dmi(
    (void*)gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_b, tbl_len,
    RGBLUT_RAM_CH1_BANK0 + bank_sel.ch1BankSelect,
    VFE_WRITE_DMI_16BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_b");
    return FALSE;
  }

  ret = isp_sub_module_util_write_dmi(
    (void*)gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_r, tbl_len,
    RGBLUT_RAM_CH2_BANK0 + bank_sel.ch2BankSelect,
    VFE_WRITE_DMI_16BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_r");
    return FALSE;
  }

  return TRUE;
} /* gamma40_dmi_hw_update */

/** gamma40_do_hw_update:
 *    @ isp_sub_module:base module object
 *    @gamma: gamma module instance
 *
 *  This function stores hw update reg cmd in base module private
 *
 *  Return:   TREU - Success
 *            FALSE - configuration error
 **/
static boolean gamma40_store_hw_update(isp_sub_module_t *isp_sub_module,
  gamma40_t *gamma)
{
  int     i = 0;
  boolean ret = TRUE;

  if (!gamma || !isp_sub_module) {
    ISP_ERR("failed: %p %p", gamma, isp_sub_module);
    return FALSE;
  }

  ret = gamma40_dmi_hw_update(gamma,
    gamma->ISP_GammaCfgCmd.LutSel, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: gamma_dmi_hw_update");
    goto ERROR;
  }

  ret = gamma40_store_single_HW_write(&gamma->ISP_GammaCfgCmd,
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
  isp_sub_module->trigger_update_pending = FALSE;
ERROR:
  return ret;
}/* gamma40_do_hw_update */

/** gamma40_interp_gamma_table:
 *    @ dest: destination gamma curve
 *    @src_count: manual gamma table length
 *    @src_curve: manual gamam curve
 *
 *  Do a linear interpolation of the input gamma curve
 *  into isp native curve format
 *
 *  Return:   None
 **/
static void gamma40_interp_gamma_table(uint8_t *dest,
  int src_count, const cam_tonemap_curve_t *src_curve)
{
  int i,index1,index2;
  float x;
  uint8_t result;

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
      "point[index2] = %f, dest[%d] = %d", index1, index2, x,
      src_curve->tonemap_points[index1][1],
      src_curve->tonemap_points[index2][1], i, dest[i]);
  }
  return;
}

/** gamma40_manual_update:
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
boolean gamma40_manual_update(mct_module_t *module,
   isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                      ret = TRUE;
  int32_t                      curve;
  gamma_rgb_ds_t               app_gamma_tbl;
  gamma40_t                    *gamma = NULL;
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

  gamma = (gamma40_t *)isp_sub_module->private_data;
  if (!gamma) {
    ISP_ERR("failed: gamma %p", gamma);
    return FALSE;
  }

  /* App curve 0 - G. Curve 1 - B, Curve 2 - R */
  gamma40_interp_gamma_table(&(app_gamma_tbl.gamma_g[0]),
    isp_sub_module->manual_ctrls.gamma_curve.tonemap_points_cnt,
    &(isp_sub_module->manual_ctrls.gamma_curve.curves[0]));

  gamma40_interp_gamma_table(&(app_gamma_tbl.gamma_b[0]),
    isp_sub_module->manual_ctrls.gamma_curve.tonemap_points_cnt,
    &(isp_sub_module->manual_ctrls.gamma_curve.curves[1]));

  gamma40_interp_gamma_table(&(app_gamma_tbl.gamma_r[0]),
    isp_sub_module->manual_ctrls.gamma_curve.tonemap_points_cnt,
    &(isp_sub_module->manual_ctrls.gamma_curve.curves[2]));

  gamma40_update_vfe_table(&gamma->ISP_GammaCfgCmd,
    &app_gamma_tbl);

  /*Populate and store hw update params inside isp_sub_mod.
      then using utility function append this isp_sub_mod's hw update list
      into global hw update list*/
  ret = gamma40_store_hw_update(isp_sub_module, gamma);
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

/** gamma40_trigger_update:
 *    @module: mct module instance
 *    @isp_sub_module: base module
 *    @event: mct event
 *
 *  This function checks and initiates triger update of module
 *
 *  Return:   TRUE - Success
 *             FALSE - get aec ratio returned error
 **/
boolean gamma40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  uint8_t                       is_burst = FALSE;
  boolean                       ret = TRUE;
  int                           i,backlight_comp_update = FALSE;
  int                           update_gamma = FALSE;
  gamma40_t                    *gamma = NULL;
  gamma_rgb_ds_t                contrast_gamma_table_rgb;
  isp_sub_module_priv_t        *isp_sub_module_priv = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  isp_meta_entry_t             *gamma_dmi_info = NULL;
  isp_hw_read_info_t           *applied_dmi_tbl = NULL;
  gamma_table_t                *gamma_hw_table = NULL;
  uint16_t                     *gamma_dmi_tbl_r = NULL;
  uint32_t                      gamma_entry_size;
  uint32_t                      gamma_table_size;

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

  gamma = (gamma40_t *)isp_sub_module->private_data;
  if (!gamma) {
    ISP_ERR("failed: gamma %p", gamma);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("enable = %d, trigger_enable = %d",
       isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
   }

  if (isp_sub_module->manual_ctrls.manual_update &&
      isp_sub_module->chromatix_module_enable) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    if (isp_sub_module->manual_ctrls.tonemap_mode ==
       CAM_TONEMAP_MODE_CONTRAST_CURVE) {
      isp_sub_module->submod_trigger_enable = FALSE;
      if (isp_sub_module->manual_ctrls.update_gamma) {
        /* Apply the manual gamma curve from the app */
        ret = gamma40_manual_update(module,isp_sub_module, event);
        isp_sub_module->manual_ctrls.update_gamma = FALSE;
        goto FILL_METADATA;
      }
    } else {
        /* For special effect we need to keep trigger updates off */
        if (gamma->effects.spl_effect == CAM_EFFECT_MODE_OFF) {
          isp_sub_module->submod_trigger_enable = TRUE;
        }
        if (isp_sub_module->manual_ctrls.update_gamma) {
        ISP_ERR("Manual gamma curve not applied");
        goto FILL_METADATA;
      }
    }
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
      float bl_ratio =
        (float)gamma->asd_update.backlight_scene_severity/255.0;
      ISP_DBG(": bl_ratio %f",  bl_ratio);

      bl_ratio = MIN(0, MAX(1.0, bl_ratio));
  #if 0
      /* FIXME: there is no backlight gamma in chromatix 301 */
      TBL_INTERPOLATE_INT(pchromatix->backlight_gamma_table.gamma,
        gamma->p_gamma_table, gamma->p_gamma_table,
        bl_ratio, GAMMA_TABLE_SIZE, i);
  #endif
      gamma->backlight_severity =
        gamma->asd_update.backlight_scene_severity;
    }

    /* update based on contrast */
    if (gamma->bestshot_mode == CAM_SCENE_MODE_OFF) {
      ISP_DBG(": Apply contrast %d", gamma->effects.contrast);
      if (gamma->effects.contrast != DEFAULT_CONTRAST) {
        gamma40_update_tab_on_contrast(gamma->effects.contrast,
          &(gamma->gamma_table_rgb),&contrast_gamma_table_rgb);
        gamma40_update_vfe_table(&gamma->ISP_GammaCfgCmd,
          &contrast_gamma_table_rgb);
      } else {
        gamma40_update_vfe_table(&gamma->ISP_GammaCfgCmd,
          &gamma->gamma_table_rgb);
      }
    }

    /*Populate and store hw update params inside isp_sub_mod.
        then using utility function append this isp_sub_mod's hw update list
        into global hw update list*/
    ret = gamma40_store_hw_update(isp_sub_module, gamma);
    if (ret == FALSE) {
      ISP_ERR("failed: gamma_do_hw_update");
      goto end;
    }
  } else if (isp_sub_module->trigger_update_pending == TRUE &&
     isp_sub_module->submod_trigger_enable == FALSE) {
       isp_sub_module->trigger_update_pending == FALSE;
       ret = gamma40_store_hw_update(isp_sub_module, gamma);
       if (ret == FALSE) {
         ISP_HIGH("failed: gamma_do_hw_update");
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
    gamma_dmi_info->len = sizeof(uint16_t) * ISP_GAMMA_NUM_ENTRIES * 3;
    /*dmi type */
    gamma_dmi_info->dump_type  = ISP_META_GAMMA_TBL;
    gamma_dmi_info->start_addr = 0;
    output->meta_dump_params->frame_meta.num_entry++;

#ifdef DYNAMIC_DMI_ALLOC
    gamma_dmi_tbl_r = (uint16_t *)CAM_MALLOC(gamma_dmi_info->len);
    if (!gamma_dmi_tbl_r) {
      ISP_ERR("failed: %p", gamma_dmi_tbl_r);
      ret = FALSE;
      goto end;
    }
#endif
    if (gamma_dmi_info->len <= sizeof(gamma_dmi_info->isp_meta_dump)) {
      memcpy(&gamma_dmi_info->isp_meta_dump[0],
        &gamma->applied_table.hw_table_r[0],
        sizeof(uint16_t) * ISP_GAMMA_NUM_ENTRIES);
      memcpy(&gamma_dmi_info->isp_meta_dump[
        ISP_GAMMA_NUM_ENTRIES * sizeof(uint16_t)],
        &gamma->applied_table.hw_table_g[0],
        sizeof(uint16_t) * ISP_GAMMA_NUM_ENTRIES);
      memcpy(&gamma_dmi_info->isp_meta_dump[
        ISP_GAMMA_NUM_ENTRIES * 2 * sizeof(uint16_t)],
        &gamma->applied_table.hw_table_b[0],
        sizeof(uint16_t) * ISP_GAMMA_NUM_ENTRIES);
    } else {
      ISP_ERR("failed memcpy: hw_dmi_tbl size %d is less than gamma hw table %d",
        gamma_dmi_info->len, sizeof(gamma_dmi_info->isp_meta_dump));
      ret = FALSE;
      goto end;
    }

    /* TODO: fill proper value when available*/
   output->meta_dump_params->frame_meta.se_lux_idx =
      gamma->aec_update.lux_idx;

   output->meta_dump_params->frame_meta.adrc_info.gamma_ratio =
     gamma->aec_update.gamma_ratio;
   output->meta_dump_params->frame_meta.adrc_info.exposure_ratio =
     gamma->aec_update.hdr_sensitivity_ratio;
   output->meta_dump_params->frame_meta.adrc_info.exposure_time_ratio =
     gamma->aec_update.hdr_exp_time_ratio;
   output->meta_dump_params->frame_meta.adrc_info.reserved_data[0] =
     gamma->aec_update.total_drc_gain;
  }

  /* reuse gamma_dmi_info for other meta dmi tbl posting */
  /* This meta entry preparation is for HDR library and not meta dump*/
  applied_dmi_tbl = &output->dmi_tbl->dmi_info[ISP_METADUMP_GAMMA];
  applied_dmi_tbl->read_length = sizeof(gamma_table_t);
  /* number of bytes each gamma entry is*/
  applied_dmi_tbl->entry_size = sizeof(uint16_t);
  applied_dmi_tbl->num_entries = ISP_GAMMA_NUM_ENTRIES;
  applied_dmi_tbl->pack_bits= GAMMA_HW_PACK_BIT;
  gamma_hw_table = (gamma_table_t *)applied_dmi_tbl->hw_dmi_tbl;
  memcpy(gamma_hw_table, &(gamma->ISP_GammaCfgCmd.Gamatbl),
    sizeof(gamma_table_t));
  ISP_DBG(" applied_dmi_tbl->read_lengh %d table %p",
    applied_dmi_tbl->read_length, applied_dmi_tbl->hw_dmi_tbl);

  if (output->frame_meta) {
    output->frame_meta->bestshot_mode = gamma->bestshot_mode;
    if (gamma->effect_applied == TRUE)
      output->frame_meta->special_effect = gamma->effects.spl_effect;
    output->frame_meta->contrast = gamma->effects.contrast;

    output->frame_meta->tonemap_mode =
      isp_sub_module->manual_ctrls.tonemap_mode;

    //Populate CAM_INTF_META_TONEMAP_CURVES
    output->frame_meta->tone_map.tonemap_points_cnt = ISP_GAMMA_NUM_ENTRIES;
    /* ch0 = G, ch 1 = B, ch 2 = R*/
    for (i=0; i<ISP_GAMMA_NUM_ENTRIES; i++) {
       output->frame_meta->tone_map.curves[0].tonemap_points[i][0] =
         (float)i / (ISP_GAMMA_NUM_ENTRIES - 1);

       output->frame_meta->tone_map.curves[0].tonemap_points[i][1] =
         (float)(gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_g[i] &
            GAMMA_HW_UNPACK_MASK) / MAX_GAMMA_VALUE;

       output->frame_meta->tone_map.curves[1].tonemap_points[i][0] =
         (float)i / (ISP_GAMMA_NUM_ENTRIES - 1);

       output->frame_meta->tone_map.curves[1].tonemap_points[i][1] =
         (float)(gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_b[i] &
             GAMMA_HW_UNPACK_MASK) / MAX_GAMMA_VALUE;

       output->frame_meta->tone_map.curves[2].tonemap_points[i][0] =
         (float)i / (ISP_GAMMA_NUM_ENTRIES - 1);

       output->frame_meta->tone_map.curves[2].tonemap_points[i][1] =
         (float)(gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_r[i] &
            GAMMA_HW_UNPACK_MASK) / MAX_GAMMA_VALUE;

       /* profile_tone_curve */
       output->frame_meta->tone_curve.curve.tonemap_points[i][0] =
         (float)i / (ISP_GAMMA_NUM_ENTRIES - 1);
       output->frame_meta->tone_curve.curve.tonemap_points[i][1] =
         (float)(gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_g[i] &
            GAMMA_HW_UNPACK_MASK) / MAX_GAMMA_VALUE;
    }
  }
end:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* gamma40_trigger_update */

#if !OVERRIDE_FUNC
/** gamma40_ext_interpolate:
 *
 *  @gamma: gamma module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean gamma40_ext_interpolate(void *data1,
  void *data2, void *data3)
{
  gamma40_t *gamma = NULL;

  if (!data1) {
    ISP_ERR("failed: %p", data1);
    return FALSE;
  }
  gamma = (gamma40_t *)data1;
  if (gamma->effects.spl_effect == CAM_EFFECT_MODE_OFF) {
    if (gamma->gamma_ratio.lighting == TRIGGER_OUTDOOR)
      gamma40_interpolate(&gamma->default_gamma_tbl, &gamma->outdoor_gamma_tbl,
        &(gamma->gamma_table_rgb), gamma->gamma_ratio.ratio);
    else if (gamma->gamma_ratio.lighting == TRIGGER_LOWLIGHT)
      gamma40_interpolate(&gamma->default_gamma_tbl, &gamma->lowlight_gamma_tbl,
        &(gamma->gamma_table_rgb), gamma->gamma_ratio.ratio);
    else
      gamma40_copy_gamma(&(gamma->gamma_table_rgb), &gamma->default_gamma_tbl,
        ISP_GAMMA_NUM_ENTRIES);
  }
  return TRUE;
}

/** gamma40_get_trigger_ratio:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean gamma40_get_trigger_ratio(void *data1,
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
  gamma40_t               *gamma = NULL;
  isp_sub_module_t        *isp_sub_module = NULL;

  if (!data1|| !data2 || !data3) {
    ISP_ERR("failed: %p %p %p", data1, data2, data3);
    return FALSE;
  }
  gamma = (gamma40_t *)data1;
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

  lowlight =  &(GAMMA_LOWLIGHT_TRIGGER(chromatix_gamma));
  outdoor = &(GAMMA_OUTDOOR_TRIGGER(chromatix_gamma));

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
  .ext_interpolate       = gamma40_ext_interpolate,
  .get_trigger_ratio     = gamma40_get_trigger_ratio,
  .ext_set_table         = NULL,
  .ext_set_spl_effect    = NULL,
};

boolean gamma40_fill_func_table(gamma40_t *gamma)
{
  gamma->ext_func_table = &gamma_override_func;
  return TRUE;
}
#endif
