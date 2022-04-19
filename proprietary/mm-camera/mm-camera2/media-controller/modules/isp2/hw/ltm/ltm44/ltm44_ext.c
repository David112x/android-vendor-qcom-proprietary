/* ltm44_ext.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* isp headers */
#include "isp_log.h"
#include "ltm44.h"

 /** ltm44_find_ltm_region:
 *  *
 *   *  @chromatix_ltm: chromatix_LTM_type
 *    *  @aec_update: aec_update_t
 *     *  @event: mct event
 *      *
 *       *  Find which LTM region it falls into based on the aec gain/
 *        *  lux index
 *         *
 *          * Return TRUE on success and FALSE on failure
 *           **/
static boolean ltm44_ext_find_ltm_region(chromatix_LTM_type *chromatix_ltm,
                aec_update_t *aec_update, LTM_trigger_index_t *ltm_tr_idx)
{
  int              i = 0;
  uint8_t          trigger_index = 0;
  float            aec_reference;
  float            ratio = 0;
  float start = 0;
  float end = 0;

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
  for (i = 0; i < MAX_GAMMA_REGIONS; i++) {

    trigger_point_type *ltm_trigger =
      &(chromatix_ltm->chromatix_ltm_core_data[i].LTM_trigger);
    trigger_index = i;

    if (i == MAX_GAMMA_REGIONS - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      ratio = 0;
      break;
    }

    if (chromatix_ltm->control_LTM == 0) {
      start = ltm_trigger->lux_index_start;
      end   = ltm_trigger->lux_index_end;
    } else {
      start = ltm_trigger->gain_start;
      end   = ltm_trigger->gain_end;
      ISP_HIGH("gain_start :%f", start);
      ISP_HIGH("gain_end :%f", end);
    }

    /* index is within interpolation range, find ratio */
     if (aec_reference >= start && aec_reference < end) {
       ratio = (aec_reference - start)/(end - start);
       ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
         ratio);
       break;
     }

     /* already scanned past the lux index */
     if (aec_reference < end) {
           break;
     }
  }
  ltm_tr_idx->ratio = ratio;
  ltm_tr_idx->tr_index = i;
  ltm_tr_idx->ltm_region_enable = chromatix_ltm->chromatix_ltm_core_data[i].rgn_enable;

  ISP_DBG(" start gain: %f end gain %f enable %u", start, end,
    ltm_tr_idx->ltm_region_enable);

  return TRUE;
}

static ltm_ext_override_func ltm_override_func_ext = {
  .find_rgn  = ltm44_ext_find_ltm_region,
};

/** ltm44_fill_func_table_ext:
 *  *
 *   * Initialize ext_func_table with OEM specific function
 *    * implementation pointers
 *     *
 *      **/
boolean ltm44_fill_func_table_ext(ltm44_t *ltm)
{
  ltm->ext_func_table = &ltm_override_func_ext;
  return TRUE;
} /* bpc44_fill_func_table */

