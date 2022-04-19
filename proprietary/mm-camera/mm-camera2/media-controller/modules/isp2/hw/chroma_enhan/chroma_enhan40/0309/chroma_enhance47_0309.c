/* chroma_enhance47_0309.c
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CHROMA_ENHANCE, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CHROMA_ENHANCE, fmt, ##args)

/* isp headers */
#include "module_chroma_enhan40.h"
#include "chroma_enhan40.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"
#include "chroma_enhan_algo.h"


/** chroma_enhan_set_chromatix_ptr
 *
 *  @module: mct module handle
 *  @isp_sub_mdoule: isp sub module handle
 *  @event: event handle
 *
 *  Update chromatix ptr
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhan_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean               ret = TRUE;
  modulesChromatix_t   *chromatix_ptrs = NULL;
  chroma_enhan40_t     *mod = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
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

  isp_sub_module->trigger_update_pending = TRUE;

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  if (mod->set_effect == TRUE) {
    ret = chroma_enhan_apply_spl_effect(module, isp_sub_module,
      mod->effect_mode);
    if (ret == FALSE) {
      ISP_ERR("failed: la40_apply_effect");
    }
  }

  if (mod->set_bestshot == TRUE) {
    ret = chroma_enhan_apply_bestshot(module, isp_sub_module,
      mod->bestshot_mode);
    if (ret == FALSE) {
      ISP_ERR("failed: la40_apply_bestshot");
    }
  }

  return TRUE;
}

/** chroma_enhan40_trigger_update_flash:
 *
 * @mod: chroma enhance module
 * @data: handle to stats_update_t
 *
 * Handle Flash event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhan40_trigger_update_flash(chroma_enhan40_t *mod  __unused,
  chromatix_parms_type *chromatix_ptr __unused)
{
  return TRUE;
}
