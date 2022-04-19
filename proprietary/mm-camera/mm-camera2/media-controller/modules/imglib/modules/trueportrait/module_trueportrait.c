/***************************************************************************
* Copyright (c) 2013-2018 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
****************************************************************************/

#include "module_imgbase.h"

/** TP_FACE_TILT_CUTOFF:
 *
 *  Tp face tilt cut off
 **/
#define TP_FACE_TILT_CUTOFF (45)

/** TP_HEADER_SIZE:
 *
 *  Tp max header size
 **/
#define TP_HEADER_SIZE (25)

/** TP_MAX_WIDTH:
 *
 *  Tp max width size
 **/
#define TP_MAX_WIDTH (800)

/** TP_MAX_HEIGHT:
 *
 *  Tp max height size
 **/
#define TP_MAX_HEIGHT (800)

/** TP_FULL_META_SIZE:
 *
 *  Tp max meta/body mask mask size
 **/
#define TP_FULL_META_SIZE (sizeof(cam_misc_buf_t) +\
  TP_HEADER_SIZE + TP_MAX_WIDTH * TP_MAX_HEIGHT)

/** TP_BODYMASK_MASK:
 *
 *  Tp enable bodymask
 **/
#define TP_BODYMASK_MASK (1 << 0)

/** TP_EFFECTS_MASK:
 *
 *  Tp enable effects
 **/
#define TP_EFFECTS_MASK (1 << 1)

/**
 *  Static functions
 **/
static boolean module_trueportrait_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* /*p_mod*/,unsigned int sessionid);
static boolean module_trueportrait_init_params(imgbase_client_t *p_client,
  img_init_params_t *p_params);
static int32_t module_trueportrait_update_meta(imgbase_client_t *,
  img_meta_t *);

/** g_caps:
 *
 *  Set the capabilities for trueportrait module
 **/
static img_caps_t g_caps = {
  .num_input = 1,
  .num_output = 0,
  .num_meta = 1,
  .inplace_algo = 1,
  .face_detect_tilt_cut_off = TP_FACE_TILT_CUTOFF,
  .num_release_buf = 0,
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_query_mod = module_trueportrait_query_mod,
  .imgbase_client_init_params = module_trueportrait_init_params,
  .imgbase_client_update_meta = module_trueportrait_update_meta,
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/**
 * Function: module_trueportrait_init_params
 *
 * Description: This function is used to init parameters
 *
 * Arguments:
 *   p_client - Imgbase client
 *  p_params - trueportrait init params
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_trueportrait_init_params(imgbase_client_t *p_client,
  img_init_params_t *p_params)
{
  boolean ret = FALSE;
  if (p_params) {
    ret = TRUE;
  }
  return ret;
}

/**
 * Function: module_trueportrait_deinit
 *
 * Description: This function is used to deinit trueportrait
 *               module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_trueportrait_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/**
 * Function: module_trueportrait_query_mod
 *
 * Description: This function is used to query trueportrait
 *               caps
 *
 * Arguments:
 *   @p_mct_cap - capababilities
 *   @p_mod - pointer to the module
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_trueportrait_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* p_mod,unsigned int sessionid)
{
  mct_pipeline_imaging_cap_t *buf;

  IMG_UNUSED(p_mod);
  IMG_UNUSED(sessionid);
  if (!p_mct_cap) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return FALSE;
  }
  buf = &p_mct_cap->imaging_cap;

  buf->true_portrait_settings.meta_max_size = TP_FULL_META_SIZE;

  return TRUE;
}

/**
 * Function: module_trueportrait_update_meta
 *
 * Description: This function is called when the base
 *   module updates the metadata
 *
 * Arguments:
 *   @p_client: pointer to imgbase client
 *   @p_meta: pointer to the image meta
 *
 * Return values:
 *     error values
 *
 * Notes: none
 **/
int32_t module_trueportrait_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta)
{
  int rc;
  uint32_t mask;
  img_tp_config_t tp_config;
  memset(&tp_config, 0x0, sizeof(img_tp_config_t));

  if (!p_client || !p_meta) {
    IDBG_ERROR("%s:%d] invalid input %p %p", __func__, __LINE__,
      p_client, p_meta);
    rc = IMG_ERR_INVALID_INPUT;
    goto error;
  }

  char prop[PROPERTY_VALUE_MAX];
  property_get("persist.vendor.camera.imglib.tp.mask", prop, "1");
  mask = (uint32_t)atoi(prop);
  if (mask & TP_BODYMASK_MASK) {
    tp_config.enable_bodymask = TRUE;
  }
  if (mask & TP_EFFECTS_MASK) {
    tp_config.enable_effects = TRUE;
  }

  property_get("persist.vendor.camera.imglib.tp.effect", prop, "0");
  tp_config.effect = (img_tp_effect_t)atoi(prop);

  property_get("persist.vendor.camera.imglib.tp.power", prop, "4");
  tp_config.intensity = (uint32_t)atoi(prop);

  rc = img_set_meta(p_meta, IMG_META_TP_CONFIG, &tp_config);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
    goto error;
  }

  return IMG_SUCCESS;

error:
  return rc;
}

/** module_trueportrait_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the trueportrait
 * module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_trueportrait_init(const char *name)
{
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    NULL,
    &g_caps,
    "libmmcamera_trueportrait_lib.so",
    CAM_QCOM_FEATURE_TRUEPORTRAIT,
    &g_params);
}
