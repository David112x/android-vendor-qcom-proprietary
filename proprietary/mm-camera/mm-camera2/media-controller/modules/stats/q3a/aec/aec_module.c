/* aec_module.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_pipeline.h"
#include "aec_module.h"
#include "aec_port.h"

/* Every AEC sink port ONLY corresponds to ONE session */

/** aec_module_remove_port:
 *    @data:      mct_port_t object
 *    @user_data: mct_module_t module
 *
 * TODO description
 *
 * TODO Return
 **/
static boolean aec_module_remove_port(void *data, void *user_data)
{
  mct_port_t   *port   = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;

  if (!port || !module || strcmp(MCT_OBJECT_NAME(port), "aec_sink") ||
    strcmp(MCT_OBJECT_NAME(module), "aec")) {
    return FALSE;
  }

  /* 1. remove port from the module
   * 2. port_deinit;
   * 3. mct_port_destroy */
  mct_module_remove_port(module, port);
  aec_port_deinit(port);
  mct_port_destroy(port);

  return TRUE;
}

/** aec_module_find_port:
 *    @data1: mct_port_t object
 *    @data2: identity object to be checked
 *
 *  Check if this port has already been linked by the same session.
 *
 *  Return TRUE if the port has the same session id.
 **/
static boolean aec_module_find_port(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  unsigned int *id = (unsigned int *)data2;

  return aec_port_find_identity(port, *id);
}

/** aec_module_set_mod:
 *    @module:      TODO
 *    @module_type: TODO
 *    @identity:    TODO
 *
 * This function shouldnt be allowed by any
 * modules to overwrite module type
 *
 * TODO Return
 **/
void aec_module_set_mod(mct_module_t *module, unsigned int module_type,
  unsigned int identity)
{
  (void)  module;
  (void) module_type;
  (void) identity;
  /* TODO */
  return;
}

/** aec_module_find_stored_parm:
 *    @data1: Stored AEC params
 *    @data2: Session ID
 *
 * Supporting function to initialize the session ID.
 *
 * Returns session ID if everything fine, else FALSE
 **/
static boolean aec_module_find_stored_parm(void *data1, void *data2)
{
  aec_stored_params_t *stored_params = (aec_stored_params_t *)data1;
  unsigned int *id = (unsigned int *)data2;

  if (!stored_params) {
    return FALSE;
  }

  return stored_params->session_id == *id;
}

/** aec_module_start_session:
 *    @module:    aec module
 *    @sessionid: session identity
 *
 * TODO description
 *
 * TODO Return
 **/
static boolean aec_module_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  boolean    rc = FALSE;
  mct_port_t *port = NULL;
  mct_list_t *list = NULL;

  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "aec")) {
    return FALSE;
  }

  AEC_LOW("sessionid=%d", sessionid);
  MCT_OBJECT_LOCK(module);
  if (MCT_MODULE_NUM_SINKPORTS(module) != 0) {
    list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
      &sessionid, aec_module_find_port);
    if (list != NULL) {
      rc = TRUE;
      goto start_done;
    }
  }

  /* Now need to create a new sink port */
  port = mct_port_create("aec_sink");
  if (port == NULL) {
    goto start_done;
  }

  if (aec_port_init(port, &sessionid) == FALSE) {
    goto port_init_error;
  }

  if (mct_module_add_port(module, port) == FALSE) {
    goto port_add_error;
  }

  aec_stored_params_t *stored_params = NULL;
  list = mct_list_find_custom(module->module_private, &sessionid,
    aec_module_find_stored_parm);
  if (list == NULL) {
    stored_params = (aec_stored_params_t *)malloc(sizeof(aec_stored_params_t));
    if (NULL == stored_params) {
      AEC_ERR("Memory allocation failed");
      goto private_init_error;
    }

    stored_params->session_id = sessionid;
    stored_params->first_init = TRUE;
    /* This will be enabled from tuning */
    stored_params->enable = FALSE;
    module->module_private =
      mct_list_append(module->module_private, stored_params, NULL, NULL);
  } else {
    stored_params = (aec_stored_params_t *)list->data;
  }
  aec_port_set_stored_parm(port, stored_params);

  rc = TRUE;
  goto start_done;

private_init_error:
  mct_module_remove_port(module, port);
port_add_error:
  aec_port_deinit(port);
port_init_error:
  mct_port_destroy(port);
start_done:
  MCT_OBJECT_UNLOCK(module);
  return rc;
}

/** aec_module_stop_session:
 *    @module:    aec module
 *    @sessionid: session identity
 *
 * Deinitialize aec component
 *
 * Return  boolean
 **/
static boolean aec_module_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  mct_list_t *list;
  mct_port_t *port = NULL;

  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "aec")) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(module);
  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &sessionid,
    aec_module_find_port);
  if (list == NULL) {
    MCT_OBJECT_UNLOCK(module);
    return FALSE;
  }

  port = MCT_PORT_CAST(list->data);
  mct_module_remove_port(module, port);

  aec_port_deinit(port);
  mct_port_destroy(port);
  MCT_OBJECT_UNLOCK(module);

  return TRUE;
}

/** aec_module_set_session_data
 *    @module: aec module
 *    @set_buf: session data
 *    @sessionid: session identity
 *
 *  Use session data for initialization
 *  Return: true if success
 **/
static boolean aec_module_set_session_data(mct_module_t *module,
  void *set_buf,
  unsigned int sessionid)
{
  mct_list_t *list = NULL;
  mct_port_t *port = NULL;
  boolean rc = FALSE;
  mct_pipeline_session_data_t *session_data =
    (mct_pipeline_session_data_t *)set_buf;

  AEC_LOW("module %p, sessionid = 0x%x", module, sessionid);
  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "aec")) {
    AEC_ERR("error: invalid module name");
    return FALSE;
  }
  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
    &sessionid, aec_module_find_port);
  if (list == NULL) {
    AEC_ERR("error: no sink ports");
    return FALSE;
  }
  if (!(Q3A_LIB_UPDATE_MASK & session_data->set_session_mask)) {
    return TRUE; /* Not an error but msg not for this module */
  }

  port = MCT_PORT_CAST(list->data);
  rc = aec_port_set_session_data(port, (void *)&session_data->q3a_lib_info,
    session_data, &sessionid);
  return rc;
}


/**aec_module_query_mod:
 *    @module:    aec module
 *    @query_buf: TODO
 *    @sessionid: idenity
 *
 * TODO description
 *
 * TODO Return
 **/
boolean aec_module_query_mod(mct_module_t *module,
  void *buf, unsigned int sessionid)
{
  boolean rc = FALSE;
  mct_list_t *list = NULL;
  mct_port_t *port = NULL;
  mct_pipeline_cap_t *query_buf = (mct_pipeline_cap_t *)buf;

  AEC_LOW("module %p, sessionid = 0x%x", module, sessionid);
  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "aec")) {
    AEC_ERR("error: invalid module name");
    return rc;
  }

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
    &sessionid, aec_module_find_port);
  if (list == NULL) {
    AEC_ERR("error: no sink ports");
    return rc;
  }
  port = MCT_PORT_CAST(list->data);

  rc = aec_port_query_capabilities(port, &query_buf->stats_cap);
  if (FALSE == rc) {
    AEC_ERR("error: fail to query AEC capabilities");
    return rc;
  }

  /* Set generic AEC module capabilities */
  query_buf->stats_cap.auto_exposure_lock_supported = TRUE;
#ifdef _OEM_CHROMATIX_0308_
  query_buf->stats_cap.exposure_compensation_max = 20;
  query_buf->stats_cap.exposure_compensation_min = -20;
  query_buf->stats_cap.exposure_compensation_default = 0;
  query_buf->stats_cap.exposure_compensation_step = (float)(1.0/10.0);
  query_buf->stats_cap.exp_compensation_step.numerator = 1;
  query_buf->stats_cap.exp_compensation_step.denominator = 10;
#else /* default */
  query_buf->stats_cap.exposure_compensation_max = 12;
  query_buf->stats_cap.exposure_compensation_min = -12;
  query_buf->stats_cap.exposure_compensation_default = 0;
  query_buf->stats_cap.exposure_compensation_step = (float)(1.0/6.0);
  query_buf->stats_cap.exp_compensation_step.numerator = 1;
  query_buf->stats_cap.exp_compensation_step.denominator = 6;
#endif
  query_buf->stats_cap.max_num_metering_areas = 81;
  query_buf->stats_cap.supported_aec_modes[0] = CAM_AEC_MODE_FRAME_AVERAGE;
  query_buf->stats_cap.supported_aec_modes[1] = CAM_AEC_MODE_CENTER_WEIGHTED;
  query_buf->stats_cap.supported_aec_modes[2] = CAM_AEC_MODE_SPOT_METERING;
  //query_buf->stats_cap.supported_aec_modes[3] = CAM_AEC_MODE_SMART_METERING;
  query_buf->stats_cap.supported_aec_modes[3] = CAM_AEC_MODE_CENTER_WEIGHTED;
  query_buf->stats_cap.supported_aec_modes[4] = CAM_AEC_MODE_SPOT_METERING_ADV;
  query_buf->stats_cap.supported_aec_modes[5] =
    CAM_AEC_MODE_CENTER_WEIGHTED_ADV;
  query_buf->stats_cap.supported_aec_modes_cnt = CAM_AEC_MODE_MAX - 1; //to remove support for smart_metering

  query_buf->stats_cap.supported_ae_modes[0] = CAM_AE_MODE_OFF;
  query_buf->stats_cap.supported_ae_modes[1] = CAM_AE_MODE_ON;
  query_buf->stats_cap.supported_ae_modes_cnt = 2; //== CAM_AE_MODE_MAX

  query_buf->stats_cap.supported_flash_modes[0] = CAM_FLASH_MODE_OFF;
  query_buf->stats_cap.supported_flash_modes[1] = CAM_FLASH_MODE_AUTO;
  query_buf->stats_cap.supported_flash_modes[2] = CAM_FLASH_MODE_ON;
  query_buf->stats_cap.supported_flash_modes[3] = CAM_FLASH_MODE_TORCH;
  query_buf->stats_cap.supported_flash_modes_cnt = 4;

  query_buf->stats_cap.supported_iso_modes[0] = CAM_ISO_MODE_AUTO;
  query_buf->stats_cap.supported_iso_modes[1] = CAM_ISO_MODE_DEBLUR;
  query_buf->stats_cap.supported_iso_modes[2] = CAM_ISO_MODE_100;
  query_buf->stats_cap.supported_iso_modes[3] = CAM_ISO_MODE_200;
  query_buf->stats_cap.supported_iso_modes[4] = CAM_ISO_MODE_400;
  query_buf->stats_cap.supported_iso_modes[5] = CAM_ISO_MODE_800;
  query_buf->stats_cap.supported_iso_modes[6] = CAM_ISO_MODE_1600;
  query_buf->stats_cap.supported_iso_modes[7] = CAM_ISO_MODE_3200;
  query_buf->stats_cap.supported_iso_modes_cnt = CAM_ISO_MODE_MAX;

  query_buf->stats_cap.supported_instant_aec_modes[0] = CAM_AEC_NORMAL_CONVERGENCE;
  query_buf->stats_cap.supported_instant_aec_modes[1] = CAM_AEC_AGGRESSIVE_CONVERGENCE;
  query_buf->stats_cap.supported_instant_aec_modes[2] = CAM_AEC_FAST_CONVERGENCE;
  query_buf->stats_cap.supported_instant_aec_modes_cnt = CAM_AEC_CONVERGENCE_MAX;

  query_buf->stats_cap.max_frame_applying_delay = 0;
  query_buf->stats_cap.max_meta_reporting_delay = 1;
  return rc;
}

/** aec_module_deinit:
 *    @module: aec module
 *
 * This function deinits aec module. WIll be called by Stats
 * module and triggered by MCT.
 *
 * Return nothing
 **/
void aec_module_deinit(mct_module_t *module)
{
  if (!module || strcmp(MCT_OBJECT_NAME(module), "aec")) {
    return;
  }

  /* Remove the private module data. */
  mct_list_free_list(module->module_private);

  /* Remove all ports of this module */
  mct_list_traverse(MCT_OBJECT_CHILDREN(module), aec_module_remove_port,
    module);

  mct_list_free_list(MCT_OBJECT_CHILDREN(module));

  mct_module_destroy(module);
}

/** aec_module_init:
 *    @name: name of this stats interface module("aec").
 *
 * TODO description
 *
 * Return this stats interface module if succes, otherwise return NULL.
 **/
mct_module_t *aec_module_init(const char *name) {
  int          i;
  mct_module_t *aec;

  if (strcmp(name, "aec")) {
    return NULL;
  }

  aec = mct_module_create("aec");
  if (!aec) {
    return NULL;
  }

  mct_module_set_set_mod_func(aec, aec_module_set_mod);
  mct_module_set_query_mod_func(aec, aec_module_query_mod);
  mct_module_set_start_session_func(aec, aec_module_start_session);
  mct_module_set_stop_session_func(aec, aec_module_stop_session);
  mct_module_set_session_data_func(aec, aec_module_set_session_data);

  return aec;
}


/** aec_module_get_port:
 *    @module:   TODO
 *    sessionid: TODO
 *
 * TODO description
 *
 * TODO Return
 **/
mct_port_t *aec_module_get_port(mct_module_t *module, unsigned int sessionid)
{
  mct_list_t *list = NULL;

  if (!module || strcmp(MCT_OBJECT_NAME(module), "aec")) {
    return FALSE;
  }

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &sessionid,
    aec_module_find_port);
  if (list == NULL) {
    return NULL;
  }

  return ((mct_port_t *)list->data);
}
