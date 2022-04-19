/* mce40.c
 *
 * Copyright (c) 2012-2014, 2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

/* isp headers */
#include "mce40.h"
#include "isp_sub_module_util.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"

/* #define MCE_DEBUG */
#ifdef MCE_DEBUG
#undef ISP_DBG
#define ISP_DBG ISP_ERR
#undef ISP_HIGH
#define ISP_HIGH ISP_ERR
#endif

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_MCE, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_MCE, fmt, ##args)

/* Forward declarations*/
boolean mce40_init_config(isp_sub_module_t *isp_sub_module,
  mce40_t *mce);

/** mce40_config_debug
 * @mce: mce object
 *
 * This fucntion prints register settings sent to hardware
 *
 *Returns void
 **/
static void mce40_config_debug(mce40_t *mce)
{
  ISP_DBG("MCE Configurations");
  ISP_DBG("qk %d",mce->mce_mix_cmd_2.qk);
  ISP_DBG("===== RED config=======");
  ISP_DBG("red:y1 %d, y2 %d, y3 %d, y4 %d",
    mce->mce_cmd.redCfg.y1, mce->mce_cmd.redCfg.y2,
    mce->mce_cmd.redCfg.y3, mce->mce_cmd.redCfg.y4);
  ISP_DBG("red:yM1 %d, yM3 %d, yS1 %d, yS3 %d",
    mce->mce_cmd.redCfg.yM1, mce->mce_cmd.redCfg.yM3,
    mce->mce_cmd.redCfg.yS1, mce->mce_cmd.redCfg.yS3);
  ISP_DBG("red: width %d, trunc %d, CR %d, CB %d, slope %d, K %d",
    mce->mce_cmd.redCfg.transWidth, mce->mce_cmd.redCfg.transTrunc,
    mce->mce_cmd.redCfg.CRZone, mce->mce_cmd.redCfg.CBZone,
    mce->mce_cmd.redCfg.transSlope, mce->mce_cmd.redCfg.K);

  ISP_DBG("===== GREEN config=======");
  ISP_DBG("green:y1 %d, y2 %d, y3 %d, y4 %d",
    mce->mce_cmd.greenCfg.y1, mce->mce_cmd.greenCfg.y2,
    mce->mce_cmd.greenCfg.y3, mce->mce_cmd.greenCfg.y4);
  ISP_DBG("green: yM1 %d, yM3 %d, yS1 %d, yS3 %d",
    mce->mce_cmd.greenCfg.yM1, mce->mce_cmd.greenCfg.yM3,
    mce->mce_cmd.greenCfg.yS1, mce->mce_cmd.greenCfg.yS3);
  ISP_DBG("green: width %d, trunc %d, CR %d, CB %d, slope %d, K %d",
    mce->mce_cmd.greenCfg.transWidth, mce->mce_cmd.greenCfg.transTrunc,
    mce->mce_cmd.greenCfg.CRZone, mce->mce_cmd.greenCfg.CBZone,
    mce->mce_cmd.greenCfg.transSlope, mce->mce_cmd.greenCfg.K);

  ISP_DBG("===== BLUE config=======");
  ISP_DBG("blue:y1 %d, y2 %d, y3 %d, y4 %d",
    mce->mce_cmd.blueCfg.y1, mce->mce_cmd.blueCfg.y2,
    mce->mce_cmd.blueCfg.y3, mce->mce_cmd.blueCfg.y4);
  ISP_DBG("blue:yM1 %d, yM3 %d, yS1 %d, yS3 %d,",
    mce->mce_cmd.blueCfg.yM1, mce->mce_cmd.blueCfg.yM3,
    mce->mce_cmd.blueCfg.yS1, mce->mce_cmd.blueCfg.yS3);
  ISP_DBG("blue: width %d, trunc %d, CR %d, CB %d, slope %d, K %d",
    mce->mce_cmd.blueCfg.transWidth, mce->mce_cmd.blueCfg.transTrunc,
    mce->mce_cmd.blueCfg.CRZone, mce->mce_cmd.blueCfg.CBZone,
    mce->mce_cmd.blueCfg.transSlope, mce->mce_cmd.blueCfg.K);
}

/** mce40_ez_vfe_update:
 *  @memcolorenhan: vfe data struct to fill in
 *  @mceCfg: mce config cmd
 *
 *  This function populates vfe diag data
 *
 *  Return: void
 **/
static void mce40_ez_vfe_update(memorycolorenhancement_t *memcolorenhan,
  ISP_MCE_ConfigCmdType *mceCfg)
{
  memcolorenhan->red.y1           = mceCfg->redCfg.y1;
  memcolorenhan->red.y2           = mceCfg->redCfg.y2;
  memcolorenhan->red.y3           = mceCfg->redCfg.y3;
  memcolorenhan->red.y4           = mceCfg->redCfg.y4;
  memcolorenhan->red.yM1          = mceCfg->redCfg.yM1;
  memcolorenhan->red.yM3          = mceCfg->redCfg.yM3;
  memcolorenhan->red.yS1          = mceCfg->redCfg.yS1;
  memcolorenhan->red.yS3          = mceCfg->redCfg.yS3;
  memcolorenhan->red.transWidth   = mceCfg->redCfg.transWidth;
  memcolorenhan->red.transTrunc   = mceCfg->redCfg.transTrunc;
  memcolorenhan->red.crZone       = mceCfg->redCfg.CRZone;
  memcolorenhan->red.cbZone       = mceCfg->redCfg.CBZone;
  memcolorenhan->red.translope    = mceCfg->redCfg.transSlope;
  memcolorenhan->red.k            = mceCfg->redCfg.K;

  memcolorenhan->green.y1          = mceCfg->greenCfg.y1;
  memcolorenhan->green.y2          = mceCfg->greenCfg.y2;
  memcolorenhan->green.y3          = mceCfg->greenCfg.y3;
  memcolorenhan->green.y4          = mceCfg->greenCfg.y4;
  memcolorenhan->green.yM1         = mceCfg->greenCfg.yM1;
  memcolorenhan->green.yM3         = mceCfg->greenCfg.yM3;
  memcolorenhan->green.yS1         = mceCfg->greenCfg.yS1;
  memcolorenhan->green.yS3         = mceCfg->greenCfg.yS3;
  memcolorenhan->green.transWidth  = mceCfg->greenCfg.transWidth;
  memcolorenhan->green.transTrunc  = mceCfg->greenCfg.transTrunc;
  memcolorenhan->green.crZone      = mceCfg->greenCfg.CRZone;
  memcolorenhan->green.cbZone      = mceCfg->greenCfg.CBZone;
  memcolorenhan->green.translope   = mceCfg->greenCfg.transSlope;
  memcolorenhan->green.k           = mceCfg->greenCfg.K;

  memcolorenhan->blue.y1           = mceCfg->blueCfg.y1;
  memcolorenhan->blue.y2           = mceCfg->blueCfg.y2;
  memcolorenhan->blue.y3           = mceCfg->blueCfg.y3;
  memcolorenhan->blue.y4           = mceCfg->blueCfg.y4;
  memcolorenhan->blue.yM1          = mceCfg->blueCfg.yM1;
  memcolorenhan->blue.yM3          = mceCfg->blueCfg.yM3;
  memcolorenhan->blue.yS1          = mceCfg->blueCfg.yS1;
  memcolorenhan->blue.yS3          = mceCfg->blueCfg.yS3;
  memcolorenhan->blue.transWidth   = mceCfg->blueCfg.transWidth;
  memcolorenhan->blue.transTrunc   = mceCfg->blueCfg.transTrunc;
  memcolorenhan->blue.crZone       = mceCfg->blueCfg.CRZone;
  memcolorenhan->blue.cbZone       = mceCfg->blueCfg.CBZone;
  memcolorenhan->blue.translope    = mceCfg->blueCfg.transSlope;
  memcolorenhan->blue.k            = mceCfg->blueCfg.K;
}

/** mce40_fill_vfe_diag_data:
 *
 *  @mod: mce  module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean mce40_fill_vfe_diag_data(mce40_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean                   ret = TRUE;
  memorycolorenhancement_t *memcolorenhan = NULL;
  vfe_diagnostics_t        *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    memcolorenhan = &vfe_diag->prev_memcolorenhan;

    /*Populate vfe_diag data for example*/
    ISP_DBG("%s: Populating vfe_diag data", __func__);
    memcolorenhan->qk = mod->mce_mix_cmd_2.qk;
    mce40_ez_vfe_update(memcolorenhan, &mod->applied_mce_cmd);
  }
  return ret;
}

/** mce40_get_vfe_diag_info_user:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function populates vfe diag data
 *
 *  Return: TRUE on success and FALSE on failure
 **/
 boolean mce40_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  isp_private_event_t *private_event = NULL;
  mce40_t             *mce = NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;
  mce40_t             *mceDiag = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p data %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  mce = (mce40_t *)isp_sub_module->private_data;
  if (!mce) {
    ISP_ERR("failed: mce %p", mce);
    return FALSE;
  }

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  vfe_diag = (vfe_diagnostics_t *)private_event->data;
  if (!vfe_diag) {
    ISP_ERR("failed: vfe_diag %p", vfe_diag);
    return FALSE;
  }

  if (sizeof(vfe_diagnostics_t) != private_event->data_size) {
    ISP_ERR("failed: out_param_size mismatch, param_id = %d",
      private_event->type);
    return FALSE;
  }
  memorycolorenhancement_t *memcolorenhan = &vfe_diag->prev_memcolorenhan;

  /*Populate vfe_diag data for example*/
  ISP_DBG("%s: Populating vfe_diag data", __func__);
  memcolorenhan->qk = mce->mce_mix_cmd_2.qk;
  mce40_ez_vfe_update(memcolorenhan, &mce->applied_mce_cmd);
  return TRUE;
}

/** mce40_set_chromatix_ptr:
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
boolean mce40_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  mce40_t             *mce = NULL;
  modulesChromatix_t    *chromatix_ptrs = NULL;

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

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }

  mce = (mce40_t *)isp_sub_module->private_data;
  if (!mce) {
    ISP_ERR("failed: mod %p",mce);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("mce enable = %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  ret = mce40_init_config(isp_sub_module, mce);

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** mce40_stats_aec_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean mce40_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret = TRUE;
  stats_update_t          *stats_update = NULL;
  aec_update_t            *cur_aec_update = NULL;
  aec_update_t            *new_aec_update = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_CS_MCE_type   *chromatix_mce = NULL;
  trigger_point_type      *lowlight = NULL;
  trigger_point_type      *outdoor = NULL;
  mce40_t                 *mce = NULL;
  trigger_ratio_t          trigger_ratio;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mce = (mce40_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mce);

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  cur_aec_update = &mce->aec_update;
  new_aec_update = &stats_update->aec_update;

  if (!F_EQUAL(cur_aec_update->lux_idx, new_aec_update->lux_idx)) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  *cur_aec_update = *new_aec_update;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** mce40_stats_asd_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 * Handle ASD update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean mce40_stats_asd_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  stats_update_t   *stats_update = NULL;
  asd_update_t     *cur_asd = NULL;
  asd_update_t     *new_asd = NULL;
  mce40_t        *mce = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mce = (mce40_t *)isp_sub_module->private_data;
  if (!mce) {
    ISP_ERR("failed: mod %p", mce);
    return FALSE;
  }

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  cur_asd = &mce->asd_update;
  new_asd = &stats_update->asd_update;

  if (new_asd->landscape_severity !=
      cur_asd->landscape_severity) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  /* Store ASD update in module private */
  *cur_asd = *new_asd;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** mce40_init_config
 * @mce: mce object
 *
 * This fucntion does initial default mce module configuration
 *
  *Returns TRUE on success FALSE on failure
 **/
boolean mce40_init_config(isp_sub_module_t *isp_sub_module,
  mce40_t *mce)
{
  uint32_t                       Q_s1, Q_s3;
  ISP_MCE_ConfigCmdType         *mce_cmd = &(mce->mce_cmd);
  chromatix_parms_type          *pchromatix =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  chromatix_CS_MCE_type         *pchromatix_CS_MCE =
    &pchromatix->chromatix_VFE.chromatix_CS_MCE;

  /* Assuming aec ratio = 0, landscape severity = 0
       All boost factors = 0, and qk = 15 */

  /* Overall Q_K and gains */
  mce->mce_mix_cmd_2.qk = 15;
  mce_cmd->redCfg.K = 0;
  mce_cmd->greenCfg.K = 0;
  mce_cmd->blueCfg.K = 0;

  /* Green */
  mce_cmd->greenCfg.y1 = pchromatix_CS_MCE->mce_config.green_y[0];
  mce_cmd->greenCfg.y2 = pchromatix_CS_MCE->mce_config.green_y[1];
  mce_cmd->greenCfg.y3 = pchromatix_CS_MCE->mce_config.green_y[2];
  mce_cmd->greenCfg.y4 = pchromatix_CS_MCE->mce_config.green_y[3];
  /* Compute Y slopes */
  Q_s1 = 20;
  mce_cmd->greenCfg.yM1 = 0;
  mce_cmd->greenCfg.yS1 = Q_s1 - mce->mce_mix_cmd_2.qk;
  Q_s3 = 20;
  mce_cmd->greenCfg.yM3 = 0;
  mce_cmd->greenCfg.yS3 = Q_s3 - mce->mce_mix_cmd_2.qk;

  /* Blue */
  mce_cmd->blueCfg.y1 = pchromatix_CS_MCE->mce_config.blue_y[0];
  mce_cmd->blueCfg.y2 = pchromatix_CS_MCE->mce_config.blue_y[1];
  mce_cmd->blueCfg.y3 = pchromatix_CS_MCE->mce_config.blue_y[2];
  mce_cmd->blueCfg.y4 = pchromatix_CS_MCE->mce_config.blue_y[3];
  /* Compute Y slopes */
  Q_s1 = 20;
  mce_cmd->blueCfg.yM1 = 0;
  mce_cmd->blueCfg.yS1 = Q_s1 - mce->mce_mix_cmd_2.qk;
  Q_s3 = 20;
  mce_cmd->blueCfg.yM3 = 0;
  mce_cmd->blueCfg.yS3 = Q_s3 - mce->mce_mix_cmd_2.qk;

  /* Red */
  mce_cmd->redCfg.y1 = pchromatix_CS_MCE->mce_config.red_y[0];
  mce_cmd->redCfg.y2 = pchromatix_CS_MCE->mce_config.red_y[1];
  mce_cmd->redCfg.y3 = pchromatix_CS_MCE->mce_config.red_y[2];
  mce_cmd->redCfg.y4 = pchromatix_CS_MCE->mce_config.red_y[3];
  /* Compute Y slopes */
  Q_s1 = 20;
  mce_cmd->redCfg.yM1 = 0;
  mce_cmd->redCfg.yS1 = Q_s1 - mce->mce_mix_cmd_2.qk;
  Q_s3 = 20;
  mce_cmd->redCfg.yM3 = 0;
  mce_cmd->redCfg.yS3 = Q_s3 - mce->mce_mix_cmd_2.qk;

  /* Compute C slopes */
  /* Green */
  mce_cmd->greenCfg.CBZone = pchromatix_CS_MCE->mce_config.green_cb_boundary;
  mce_cmd->greenCfg.CRZone = pchromatix_CS_MCE->mce_config.green_cr_boundary;
  mce_cmd->greenCfg.transWidth =
   (pchromatix_CS_MCE->mce_config.green_cb_transition_width +
    pchromatix_CS_MCE->mce_config.green_cr_transition_width) / 2;
  mce_cmd->greenCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_cmd->greenCfg.transWidth)) / log(2.0f) ) + 4;
  mce_cmd->greenCfg.transTrunc =
  Clamp(mce_cmd->greenCfg.transTrunc, 6, 9);
  if (mce_cmd->greenCfg.transWidth) {
    mce_cmd->greenCfg.transSlope =
    (1 << mce_cmd->greenCfg.transTrunc) /
    mce_cmd->greenCfg.transWidth;
  }

  /* Blue */
  mce_cmd->blueCfg.CBZone = pchromatix_CS_MCE->mce_config.blue_cb_boundary;
  mce_cmd->blueCfg.CRZone = pchromatix_CS_MCE->mce_config.blue_cr_boundary;
  mce_cmd->blueCfg.transWidth =
  (pchromatix_CS_MCE->mce_config.blue_cb_transition_width +
   pchromatix_CS_MCE->mce_config.blue_cr_transition_width) / 2;
  mce_cmd->blueCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_cmd->blueCfg.transWidth)) / log(2.0f)) + 4;
  mce_cmd->blueCfg.transTrunc =
  Clamp(mce_cmd->blueCfg.transTrunc, 6, 9);
  if (mce_cmd->blueCfg.transWidth) {
    mce_cmd->blueCfg.transSlope =
    (1 << mce_cmd->blueCfg.transTrunc) /
    mce_cmd->blueCfg.transWidth;
  }

  /* Red */
  mce_cmd->redCfg.CBZone =
    pchromatix_CS_MCE->mce_config.red_cb_boundary;
  mce_cmd->redCfg.CRZone =
    pchromatix_CS_MCE->mce_config.red_cr_boundary;
  mce_cmd->redCfg.transWidth =
    (pchromatix_CS_MCE->mce_config.red_cb_transition_width +
    pchromatix_CS_MCE->mce_config.red_cr_transition_width) / 2;
  mce_cmd->redCfg.transTrunc = (int32_t)ceil(log((float)(
    mce_cmd->redCfg.transWidth)) / log(2.0f)) + 4;
  mce_cmd->redCfg.transTrunc =
  Clamp(mce_cmd->redCfg.transTrunc, 6, 9);
  if (mce_cmd->redCfg.transWidth) {
    mce_cmd->redCfg.transSlope =
      (1 << mce_cmd->redCfg.transTrunc) / mce_cmd->redCfg.transWidth;
  }

  isp_sub_module->trigger_update_pending = TRUE;
  return TRUE;
} /* mce40_init_config */

/** mce40_store_hw_update:
 * @isp_sub_module: base module instance
 * @mce : mce module
 *
 *  This function checks and sends configuration update to kernel
 *
 *  Return:   TRUE - Success, FALSE- configuration error
 **/
static boolean mce40_store_hw_update(isp_sub_module_t *isp_sub_module,
  mce40_t *mce)
{

  boolean ret = TRUE;
  struct msm_vfe_cfg_cmd_list    *hw_update = NULL;
  struct msm_vfe_cfg_cmd2        *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd     *reg_cfg_cmd = NULL;
  ISP_MCE_ConfigCmdType          *reg_cmd = NULL;
  uint32_t *data = NULL;
  uint32_t len = 0;

  if (!isp_sub_module || !mce) {
    ISP_ERR("failed: %p %p", isp_sub_module, mce);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  /*regular hw write for Demosaic*/
  mce40_config_debug(mce);

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd) * 2);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_MALLOC;
  }
  memset(reg_cfg_cmd, 0, (sizeof(struct msm_vfe_reg_cfg_cmd) * 2));

  reg_cmd = (ISP_MCE_ConfigCmdType *)CAM_MALLOC(sizeof(*reg_cmd));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));
  *reg_cmd = mce->mce_cmd;

  cfg_cmd->cfg_data = (void *) reg_cmd;
  cfg_cmd->cmd_len = sizeof(mce->mce_cmd);
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = 2;

  /*write value into mixed register 2*/
  reg_cfg_cmd[0].u.mask_info.mask = mce->mce_mix_cmd_2.mask;
  reg_cfg_cmd[0].u.mask_info.val = mce->mce_mix_cmd_2.cfg;
  reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_MCE40_CHORMA_SUPP_MIX_OFF2;

  /* write value into regular register*/
  reg_cfg_cmd[1].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_MCE40_OFF;
  reg_cfg_cmd[1].u.rw_info.len = ISP_MCE40_LEN * sizeof(uint32_t);

#ifdef PRINT_REG_VAL_SET
    ISP_HIGH("hw_reg_offset %x, len %d", reg_cfg_cmd[0].u.rw_info.reg_offset,
      reg_cfg_cmd[0].u.rw_info.len);
    data = (uint32_t *)cmd_offset;
    for (len = 0; len < (cmd_len / 4); len++) {
      ISP_HIGH("data[%d] %x", len, data[len]);
    }
#endif

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;

  }

  mce->applied_mce_cmd = mce->mce_cmd;
  isp_sub_module->trigger_update_pending = FALSE;

  return ret;

ERROR_APPEND:
  free(reg_cmd);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_MALLOC:
  free(hw_update);
  return FALSE;
}

/** mce40_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 * Handle trigger update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean  mce40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  float                          ratio, fKg, fKb, fKr, max_boost;
  uint32_t                       dS1, dS3, QKg, QKb, QKr, Q_s1, Q_s3;
  ISP_MCE_ConfigCmdType         *mce_config = NULL;
  chromatix_parms_type          *chromatix_ptr = NULL;
  chromatix_CS_MCE_type         *pchromatix_CS_MCE = NULL;
  uint32_t                       landscape_severity = 0;
  float                          lux_idx;
  float                          landscape_green_boost_factor;
  float                          landscape_blue_boost_factor;
  float                          landscape_red_boost_factor;
  isp_private_event_t           *private_event = NULL;
  boolean                        ret = TRUE;
  mce40_t                       *mce = NULL;
  isp_sub_module_output_t       *output = NULL;
  int8_t                        module_enable;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(private_event);

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_3a_parms_type *chromatix_3a_ptr = NULL;
  AAA_ASD_struct_type *ASD_algo_data_ptr = NULL;
  chromatix_3a_ptr  = isp_sub_module->chromatix_ptrs.chromatix3APtr;
  if (!chromatix_3a_ptr) {
    ISP_ERR("failed: chromatix_3a_ptr %p", chromatix_3a_ptr);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  mce = (mce40_t *)isp_sub_module->private_data;
  if (mce == NULL) {
    ISP_ERR("failed: NULL pointer detected ");
    goto ERROR;
  }

  if (isp_sub_module->manual_ctrls.manual_update &&
        isp_sub_module->chromatix_module_enable) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    module_enable = (isp_sub_module->manual_ctrls.tonemap_mode ==
                     CAM_TONEMAP_MODE_CONTRAST_CURVE) ? FALSE : TRUE;

    if (isp_sub_module->submod_enable != module_enable) {
      isp_sub_module->submod_enable = module_enable;
      output->stats_params->
        module_enable_info.reconfig_needed = TRUE;
      output->stats_params->module_enable_info.
        submod_enable[isp_sub_module->hw_module_id] = module_enable;
      output->stats_params->module_enable_info.
        submod_mask[isp_sub_module->hw_module_id] = 1;
    }
  }

  if (isp_sub_module->submod_enable == FALSE ||
      isp_sub_module->submod_trigger_enable == FALSE) {
    ISP_DBG("enable = %d, trigger_enable = %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    goto FILL_METADATA;
  }

  if(isp_sub_module->trigger_update_pending == TRUE) {
    mce_config = &(mce->mce_cmd);
    lux_idx = mce->aec_update.lux_idx;

    pchromatix_CS_MCE = &chromatix_ptr->chromatix_VFE.chromatix_CS_MCE;
    ASD_algo_data_ptr = &chromatix_3a_ptr->ASD_3A_algo_data;

    landscape_green_boost_factor =
        ASD_algo_data_ptr->landscape_scene_detect.landscape_green_boost_factor;
    landscape_blue_boost_factor =
        ASD_algo_data_ptr->landscape_scene_detect.landscape_blue_boost_factor;
    landscape_red_boost_factor =
        ASD_algo_data_ptr->landscape_scene_detect.landscape_red_boost_factor;

    if (mce->bestshot_mode == CAM_SCENE_MODE_LANDSCAPE)
      landscape_severity = 255;
    else
      landscape_severity =
        MIN(255, mce->asd_update.landscape_severity);

    /* Compute MCE gains and Q_K first */
    /* Green */
    ratio = 1.0f - isp_sub_module_util_calc_interpolation_weight(lux_idx,
      pchromatix_CS_MCE->mce_config.green_bright_index,
      pchromatix_CS_MCE->mce_config.green_dark_index);
    ISP_DBG("lux_idx %f, ratio = %f, \n", lux_idx, ratio);

    fKg = ratio * (pchromatix_CS_MCE->mce_config.green_boost_factor - 1.0f);
    /*add ratio for landscape severity  */
    ISP_DBG("pre fKg =%f", fKg);
    fKg = ((fKg + 1) * (((float)landscape_severity / 255.0 *
      (landscape_green_boost_factor - 1)) + 1));

    max_boost = MAX(pchromatix_CS_MCE->mce_config.green_boost_factor,
      landscape_green_boost_factor);
    if (fKg > max_boost)
      fKg = max_boost;

    fKg = fKg-1;
    ISP_DBG("post fKg =%f", fKg);
    if (fKg > 0) {
      QKg = (uint8_t)ceil(log(4.0f / fKg) / log(2.0f)) + 6;
      while ((int32_t)(fKg * (1 << QKg)) > 383)
        QKg--;
    } else {
      fKg = 0;
      QKg = 15;
    }
    QKg = Clamp(QKg, 7, 15);

    /* Blue */
    ratio = 1.0f - isp_sub_module_util_calc_interpolation_weight(lux_idx,
      pchromatix_CS_MCE->mce_config.blue_bright_index,
      pchromatix_CS_MCE->mce_config.blue_dark_index);
    fKb = ratio * (pchromatix_CS_MCE->mce_config.blue_boost_factor - 1.0f);
    /*add ratio for landscape severity */
    ISP_DBG("pre fKb =%f", fKb);
    fKb = ((fKb + 1) * (((float)landscape_severity / 255.0 *
      (landscape_blue_boost_factor - 1)) + 1));

    max_boost = MAX(pchromatix_CS_MCE->mce_config.blue_boost_factor,
      landscape_blue_boost_factor);
    if (fKb > max_boost)
      fKb = max_boost;

    fKb = fKb-1;
    ISP_DBG("post fKb =%f", fKb);
    if (fKb > 0) {
      QKb = (uint8_t)ceil(log(4.0f / fKb) / log(2.0f)) + 6;
      while ((int32_t)(fKb * (1 << QKb)) > 383)
        QKb--;
    } else {
      fKb = 0;
      QKb = 15;
    }
    QKb = Clamp(QKb, 7, 15);

    /* Red */
    ratio = 1.0f - isp_sub_module_util_calc_interpolation_weight(lux_idx,
      pchromatix_CS_MCE->mce_config.red_bright_index,
      pchromatix_CS_MCE->mce_config.red_dark_index);
    fKr = ratio * (pchromatix_CS_MCE->mce_config.red_boost_factor - 1.0f);
    ISP_DBG("pre fKr =%f", fKr);
    /* add ratio for landscape severity */
    fKr = ((fKr + 1) * (((float)landscape_severity / 255.0 *
      (landscape_red_boost_factor - 1)) + 1));
    max_boost = MAX(pchromatix_CS_MCE->mce_config.red_boost_factor,
      landscape_red_boost_factor);
    if (fKr > max_boost)
      fKr = max_boost;

    fKr = fKr-1;
    ISP_DBG("post fKr =%f", fKr);
    if (fKr > 0) {
      QKr = (uint8_t)ceil(log(4.0f / fKr) / log(2.0f)) + 6;
      while ((int32_t)(fKr * (1 << QKr)) > 383)
        QKr--;
    } else {
      fKr = 0;
      QKr = 15;
    }
    QKr = Clamp(QKr, 7, 15);

    /* Overall Q_K and gains */

    /* fKr/fKg/fKb are limited to positive numbers */
    /* must use unsigned for K to prevent sign extension */
    mce->mce_mix_cmd_2.qk = MIN(MIN(QKg, QKb), QKr);
    mce_config->redCfg.K =
    (uint32_t)(fKr * (1 << mce->mce_mix_cmd_2.qk));
    mce_config->greenCfg.K =
    (uint32_t)(fKg * (1 << mce->mce_mix_cmd_2.qk));
    mce_config->blueCfg.K =
    (uint32_t)(fKb * (1 << mce->mce_mix_cmd_2.qk));

    /* Compute Y slopes */
    /* Green */

    dS1 = mce_config->greenCfg.y2 - mce_config->greenCfg.y1;
    dS3 = mce_config->greenCfg.y4 - mce_config->greenCfg.y3;

    if ((fKg > 0) && (dS1 > 0)) {
      Q_s1 = (int32_t)ceil(log(dS1 / fKg) / log(2.0f)) + 6;
      Q_s1 = Clamp(Q_s1, 7, 20);
      mce_config->greenCfg.yM1 = mce_config->greenCfg.K *
        (1 << (Q_s1 - mce->mce_mix_cmd_2.qk)) / dS1;
    } else {
      Q_s1 = 20;
      mce_config->greenCfg.yM1 = 0;
    }

    mce_config->greenCfg.yS1 = Q_s1 - mce->mce_mix_cmd_2.qk;
    if ((fKg > 0) && (dS3 > 0)) {
      Q_s3 = (int32_t)ceil(log(dS3/fKg)/log(2.0f)) + 6;
      Q_s3 = Clamp(Q_s3, 7, 20);
      mce_config->greenCfg.yM3 = mce_config->greenCfg.K *
        (1 << (Q_s3 - mce->mce_mix_cmd_2.qk)) / dS3;
    } else {
      Q_s3 = 20;
      mce_config->greenCfg.yM3 = 0;
    }

    mce_config->greenCfg.yS3 = Q_s3 - mce->mce_mix_cmd_2.qk;
    /* Blue */
    dS1 = mce_config->blueCfg.y2 - mce_config->blueCfg.y1;
    dS3 = mce_config->blueCfg.y4 - mce_config->blueCfg.y3;

    if ((fKb > 0) && (dS1 > 0)) {
      Q_s1 = (int32_t)ceil(log(dS1 / fKb) / log(2.0f)) + 6;
      Q_s1 = Clamp(Q_s1, 7, 20);
      mce_config->blueCfg.yM1 = mce_config->blueCfg.K *
        (1 << (Q_s1 - mce->mce_mix_cmd_2.qk)) / dS1;
    } else {
      Q_s1 = 20;
      mce_config->blueCfg.yM1 = 0;
    }
    mce_config->blueCfg.yS1 = Q_s1 - mce->mce_mix_cmd_2.qk;

    if ((fKb > 0) && (dS3 > 0)) {
      Q_s3 = (int32_t)ceil(log(dS3 / fKb) / log(2.0f)) + 6;
      Q_s3 = Clamp(Q_s3, 7, 20);
      mce_config->blueCfg.yM3 = mce_config->blueCfg.K *
        (1 << (Q_s3 - mce->mce_mix_cmd_2.qk)) / dS3;
    } else {
      Q_s3 = 20;
      mce_config->blueCfg.yM3 = 0;
    }

    mce_config->blueCfg.yS3 = Q_s3 - mce->mce_mix_cmd_2.qk;
    /* Red */
    dS1 = mce_config->redCfg.y2 - mce_config->redCfg.y1;
    dS3 = mce_config->redCfg.y4 - mce_config->redCfg.y3;

    if ((fKr > 0) && (dS1 > 0)) {
      Q_s1 = (int32_t)ceil(log(dS1 / fKr) / log(2.0f)) + 6;
      Q_s1 = Clamp(Q_s1, 7, 20);
      mce_config->redCfg.yM1 = mce_config->redCfg.K *
        (1 << (Q_s1 - mce->mce_mix_cmd_2.qk)) / dS1;
    } else {
      Q_s1 = 20;
      mce_config->redCfg.yM1 = 0;
    }
    mce_config->redCfg.yS1 = Q_s1 - mce->mce_mix_cmd_2.qk;

    if ((fKr > 0) && (dS3 > 0)) {
      Q_s3 = (int32_t)ceil(log(dS3 / fKr) / log(2.0f)) + 6;
      Q_s3 = Clamp(Q_s3, 7, 20);
      mce_config->redCfg.yM3 = mce_config->redCfg.K *
        (1 << (Q_s3 - mce->mce_mix_cmd_2.qk)) / dS3;
    } else {
      Q_s3 = 20;
      mce_config->redCfg.yM3 = 0;
    }
    mce_config->redCfg.yS3 = Q_s3 - mce->mce_mix_cmd_2.qk;
    /* Compute C slopes */
    /* Green */
    mce_config->greenCfg.CBZone =
      pchromatix_CS_MCE->mce_config.green_cb_boundary;
    mce_config->greenCfg.CRZone =
      pchromatix_CS_MCE->mce_config.green_cr_boundary;
    mce_config->greenCfg.transWidth =
     (pchromatix_CS_MCE->mce_config.green_cb_transition_width +
     pchromatix_CS_MCE->mce_config.green_cr_transition_width) / 2;
    mce_config->greenCfg.transTrunc = (int32_t)ceil(log((float)(
      mce_config->greenCfg.transWidth)) / log(2.0f) ) + 4;
    mce_config->greenCfg.transTrunc =
    Clamp(mce_config->greenCfg.transTrunc, 6, 9);

    if (mce_config->greenCfg.transWidth == 0) {
      ISP_ERR("failed, devide by 0, Green transWidth %d cb %d cr %d",
        mce_config->greenCfg.transWidth,
        pchromatix_CS_MCE->mce_config.green_cb_transition_width,
        pchromatix_CS_MCE->mce_config.green_cr_transition_width);
     goto ERROR;
    }

    mce_config->greenCfg.transSlope =
    (1 << mce_config->greenCfg.transTrunc) /
    mce_config->greenCfg.transWidth;

    /* Blue */
    mce_config->blueCfg.CBZone = pchromatix_CS_MCE->mce_config.blue_cb_boundary;
    mce_config->blueCfg.CRZone = pchromatix_CS_MCE->mce_config.blue_cr_boundary;
    mce_config->blueCfg.transWidth =
      (pchromatix_CS_MCE->mce_config.blue_cb_transition_width +
      pchromatix_CS_MCE->mce_config.blue_cr_transition_width) / 2;
    mce_config->blueCfg.transTrunc = (int32_t)ceil(log((float)(
      mce_config->blueCfg.transWidth)) / log(2.0f)) + 4;
    mce_config->blueCfg.transTrunc =
    Clamp(mce_config->blueCfg.transTrunc, 6, 9);

    if (mce_config->blueCfg.transWidth == 0) {
      ISP_ERR("failed, devide by 0, BLUE transWidth %d cb %d cr %d",
        mce_config->greenCfg.transWidth,
        pchromatix_CS_MCE->mce_config.blue_cb_transition_width,
        pchromatix_CS_MCE->mce_config.blue_cr_transition_width);
     goto ERROR;
    }

    mce_config->blueCfg.transSlope =
    (1 << mce_config->blueCfg.transTrunc) /
    mce_config->blueCfg.transWidth;
    /* Red */
    mce_config->redCfg.CBZone =
      pchromatix_CS_MCE->mce_config.red_cb_boundary;
    mce_config->redCfg.CRZone =
      pchromatix_CS_MCE->mce_config.red_cr_boundary;
    mce_config->redCfg.transWidth =
      (pchromatix_CS_MCE->mce_config.red_cb_transition_width +
       pchromatix_CS_MCE->mce_config.red_cr_transition_width) / 2;
    mce_config->redCfg.transTrunc = (int32_t)ceil(log((float)(
      mce_config->redCfg.transWidth)) / log(2.0f)) + 4;
    mce_config->redCfg.transTrunc =
    Clamp(mce_config->redCfg.transTrunc, 6, 9);

    if (mce_config->blueCfg.transWidth == 0) {
      ISP_ERR("failed, devide by 0, RED transWidth %d cb %d cr %d",
        mce_config->redCfg.transWidth,
        pchromatix_CS_MCE->mce_config.red_cb_transition_width,
        pchromatix_CS_MCE->mce_config.red_cr_transition_width);
     goto ERROR;
    }

    mce_config->redCfg.transSlope =
      (1 << mce_config->redCfg.transTrunc) / mce_config->redCfg.transWidth;

    ret = mce40_store_hw_update(isp_sub_module, mce);
    if (ret == FALSE) {
      ISP_ERR("failed: mce40_store_hw_update");
    }
  }
  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list mod %s",
      MCT_MODULE_NAME(module));
  }
FILL_METADATA:
  if (output && isp_sub_module->vfe_diag_enable) {
    ret = mce40_fill_vfe_diag_data(mce, isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: mce40_fill_vfe_diag_data");
    }
  }
ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** mce40_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function destroys mce module
 *
 *  Return: void
 **/
void mce40_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  mce40_t *mce = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }
  mce = (mce40_t *)isp_sub_module->private_data;

  memset(mce, 0, sizeof(mce40_t));
  free(mce);
  return;
}

/** mce40_reset:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mce: mce module
 *
 *  This function resets mce module private data
 *
 *  Return: void
 **/
void mce40_reset(isp_sub_module_t *isp_submodule,
  mce40_t *mce)
{
  mce->cnt = 0;
  memset(&mce->mce_mix_cmd_2, 0, sizeof(mce->mce_mix_cmd_2));
  memset(&mce->mce_cmd, 0, sizeof(mce->mce_cmd));

  mce->mce_mix_cmd_2.hw_wr_mask = 0xF;
  mce->bestshot_mode            = CAM_SCENE_MODE_OFF;

  isp_submodule->trigger_update_pending  = FALSE;
  isp_submodule->submod_trigger_enable   = TRUE;
  isp_submodule->manual_ctrls.tonemap_mode = CAM_TONEMAP_MODE_FAST;
  return;
}
/** mce40_init:
 *
 *  @isp_sub_module: isp sub module handle
 *  @module: mct module
 *
 *  This function resets mce module private data
 *
 *  Return: void
 **/
boolean mce40_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  mce40_t *mce = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  mce = (mce40_t *)CAM_MALLOC(sizeof(mce40_t));
  RETURN_IF_NULL(mce);
  memset(mce, 0, sizeof(*mce));
  isp_sub_module->private_data = (void *)mce;
  mce40_reset(isp_sub_module, mce);
  return TRUE;
}
