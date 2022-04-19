/* chroma_enhance47_0310.c
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
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_CV_type    *chromatix_CV_ptr = NULL;
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
  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  chromatix_CV_ptr =
      &chromatix_ptr->chromatix_VFE.chromatix_CV;
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

  if (chromatix_CV_ptr->default_conv_enable) {
    chroma_enhance_apply_default_settings(isp_sub_module);
    if (ret == FALSE) {
      ISP_ERR("failed: apply_default_settings");
    }
  }
  return TRUE;
}

/** chroma_enhance_apply_default_settings:
 *
 * @isp_sub_module: isp sub module handle
 *
 * Apply default settings
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhance_apply_default_settings(isp_sub_module_t *isp_sub_module)
{
  chroma_enhan40_t         *mod = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: %p", isp_sub_module);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  /* reset hue/saturation */
  SET_UNITY_MATRIX(mod->effects_matrix, 2);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  /* disable trigger when default settings enabled */
  isp_sub_module->submod_trigger_enable = FALSE;

  /* Enable trigger_update_pending */
  isp_sub_module->trigger_update_pending = TRUE;

  mod->cv_data.chroma.a_m  = 0.500000f;
  mod->cv_data.chroma.a_p  = 0.500000f;
  mod->cv_data.chroma.b_m  = -0.338000f;
  mod->cv_data.chroma.b_p  = -0.338000f;
  mod->cv_data.chroma.c_m  = 0.500000f;
  mod->cv_data.chroma.c_p  = 0.500000f;
  mod->cv_data.chroma.d_m  = -0.162000f;
  mod->cv_data.chroma.d_p  = -0.162000f;
  mod->cv_data.chroma.k_cb = 128.0f;
  mod->cv_data.chroma.k_cr = 128.0f;

  mod->cv_data.luma.v0 = 0.299000f;
  mod->cv_data.luma.v1 = 0.587000f;
  mod->cv_data.luma.v2 = 0.114000f;
  mod->cv_data.luma.k  = 0.0f;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
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
boolean chroma_enhan40_trigger_update_flash(chroma_enhan40_t *mod,
  chromatix_parms_type *chromatix_ptr)
{
  chromatix_CV_type *chromatix_CV_ptr     = NULL;
  float              led1_value_mainflash = 0.0;
  float              led2_value_mainflash = 0.0;
  float              led_mainflash_ratio  = 0.0;
  float              ratio = 0.0, flash_start = 0.0, flash_end = 0.0;
  flash_sensitivity_t *flash_sensitivity = NULL;
  color_conversion_type_t cv_led1, cv_led2, cv_flash;

  chromatix_CV_ptr = &chromatix_ptr->chromatix_VFE.chromatix_CV;
  memset(&cv_flash, 0, sizeof(chromatix_color_conversion_type));

  if (!mod ||  !chromatix_ptr) {
    ISP_ERR("%s:%d failed: %p %p\n", __func__, __LINE__, mod, chromatix_ptr);
    return FALSE;
  }

  led1_value_mainflash =
    mod->dual_led_setting.led1_high_setting;
  led2_value_mainflash =
    mod->dual_led_setting.led2_high_setting;
  ISP_DBG("led1_value_mainflash %f led2_value_mainflash %f ",
    led1_value_mainflash, led2_value_mainflash);
  if (led1_value_mainflash > 0 && led2_value_mainflash > 0 ) {
    /* Dual LED case */
    led_mainflash_ratio =
      (float)led1_value_mainflash/(led1_value_mainflash +
        led2_value_mainflash);
    chroma_enhan40_copy_cv_data(&cv_led1,
      &chromatix_CV_ptr->LED1_color_conversion);
    chroma_enhan40_copy_cv_data(&cv_led2,
      &chromatix_CV_ptr->LED2_color_conversion);
    chroma_enhan_interpolate(&cv_led1, &cv_led2, &cv_flash, led_mainflash_ratio);
  } else {
    if (led2_value_mainflash == 0.0)
       chroma_enhan40_copy_cv_data(&cv_flash,
         &chromatix_CV_ptr->LED1_color_conversion);
    if (led1_value_mainflash == 0.0)
      chroma_enhan40_copy_cv_data(&cv_flash,
         &chromatix_CV_ptr->LED2_color_conversion);
  }
  flash_start = chromatix_CV_ptr->CV_LED_start;
  flash_end = chromatix_CV_ptr->CV_LED_end;

  flash_sensitivity = &mod->stats_update.aec_update.flash_sensitivity;
  if ((mod->cur_flash_mode == CAM_FLASH_MODE_ON) ||
      (mod->cur_flash_mode == CAM_FLASH_MODE_SINGLE)) {
    if (flash_sensitivity->high != 0) {
      ratio = flash_sensitivity->off / flash_sensitivity->high;
    } else {
      ISP_ERR("invalid sensitivity high %f", flash_sensitivity->high);
      ratio = flash_end;
    }
  } else {
    /* assume flash off. To be changed when AUTO mode is added */
    ratio = flash_start;
  }

  ISP_DBG("<flash_dbg> flash_ratio %f, flash_start %5.2f flash_end %5.2f ",
    ratio, flash_start, flash_end);
  ISP_DBG("off %5.2f high %5.2f ", flash_sensitivity->off,
    flash_sensitivity->high);

  if (ratio >= flash_end) {
    mod->cv_data = cv_flash;
  } else if (ratio < flash_start) {
    /* Nothing to do. use the same information i.e. mod->cv_data
       remains unchanged */
  } else {
    /* Interpolation range */
    if (flash_end - flash_start > 0) {
      ratio = (flash_end - ratio) / (flash_end - flash_start);
      chroma_enhan_interpolate( &mod->cv_data, &cv_flash, &mod->cv_data,
          ratio);
    } else {
      ISP_ERR("Incorrect Flash params: start:%f end:%f", flash_start,
        flash_end);
    }
  }

  return TRUE;
}
