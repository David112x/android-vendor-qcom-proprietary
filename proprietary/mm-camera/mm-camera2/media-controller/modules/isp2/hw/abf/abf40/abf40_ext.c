/* abf40_ext.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* isp headers */
#include "isp_log.h"
#include "abf40.h"

/** abf40_get_trigger_index:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean abf40_get_trigger_index(void *data1,
  void *data2 )
{
  uint8_t*  trigger_idx;
  abf40_t   *abf = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  abf = (abf40_t *)data1;
  trigger_idx = (uint8_t *)data2;

  if ((abf->aec_update.low_light_capture_update_flag) &&
      (abf->aec_update.nr_flag))
  {
    if (abf->aec_update.nr_flag > 2) {
      ISP_ERR("failed: nr_flag %d",abf->aec_update.nr_flag);
      return FALSE;
    }
    *trigger_idx = MAX_LIGHT_TYPES_FOR_SPATIAL + abf->aec_update.nr_flag - 1;
  }
  return TRUE;
}

static ext_override_func abf_override_func_ext = {
  .get_trigger_index = abf40_get_trigger_index,
};

boolean abf40_fill_func_table_ext(abf40_t *abf)
{
  abf->ext_func_table = &abf_override_func_ext;
  return TRUE;
} /* abf47_fill_func_table */
