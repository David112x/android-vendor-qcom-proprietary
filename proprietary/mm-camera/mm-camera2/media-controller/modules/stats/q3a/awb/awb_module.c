/* awb_module.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "awb_module.h"
#include "awb_port.h"
#include "mct_controller.h"

/* Every AWB sink port ONLY corresponds to ONE session */

/** awb_module_remove_port
 *    @data: mct_port_t object
 *    @user_data: mct_module_t module
 **/
static boolean awb_module_remove_port(void *data, void *user_data)
{
  mct_port_t *port     = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;

  if (!port || !module || strcmp(MCT_OBJECT_NAME(port), "awb_sink") ||
      strcmp(MCT_OBJECT_NAME(module), "awb"))
    return FALSE;

  /* 1. remove port from the module
   * 2. port_deinit;
   * 3. mct_port_destroy */
  mct_module_remove_port(module, port);
  awb_port_deinit(port);
  mct_port_destroy(port);

  return TRUE;
}

/** awb_module_find_port
 *    @data1: mct_port_t object
 *    @data2: identity object to be checked
 *
 *  Check if this port has already been linked by the same session.
 *
 *  Return TRUE if the port has the same session id.
 **/
static boolean awb_module_find_port(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  unsigned int *id = (unsigned int *)data2;

  return awb_port_find_identity(port, *id);
}

/** awb_module_set_mod
 *    @module:
 *    @module_type
 *    @identity
 *
 * This function shouldnt be allowed by any
 *  modules to overwrite module type
 **/
void awb_module_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  /* TODO */
  (void) module;
  (void) module_type;
  (void) identity;
  return;
}

/** awb_module_find_stored_parm:
 *    @data1: Stored AWB params
 *    @data2: Session ID
 *
 * Supporting function to initialize the session ID.
 *
 * Returns session ID if everything fine, else FALSE
 **/
static boolean awb_module_find_stored_parm(void *data1, void *data2)
{
  awb_stored_params_type *stored_params = (awb_stored_params_type *)data1;
  unsigned int *id = (unsigned int *)data2;

  if (!stored_params) {
    return FALSE;
  }

  return stored_params->session_id == *id;
}

/** awb_module_set_session_data
 *    @module: awb module
 *    @set_buf: session data
 *    @sessionid: session identity
 *
 *  Use session data for initialization
 *  Return: true if success
 **/
static boolean awb_module_set_session_data(mct_module_t *module,
  void *set_buf,
  unsigned int sessionid)
{
  mct_list_t *list = NULL;
  mct_port_t *port = NULL;
  boolean rc = FALSE;
  mct_pipeline_session_data_t *session_data =
    (mct_pipeline_session_data_t *)set_buf;

  AWB_HIGH("module %p, sessionid = 0x%x", module, sessionid);
  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "awb")) {
    AWB_ERR("error: invalid module name");
    return FALSE;
  }
  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
          &sessionid, awb_module_find_port);
  if (list == NULL) {
    AWB_ERR("error: no sink ports");
    return FALSE;
  }
  if (!(Q3A_LIB_UPDATE_MASK & session_data->set_session_mask)) {
    return TRUE; /* Not an error but msg not for this module */
  }

  port = MCT_PORT_CAST(list->data);
  rc = awb_port_set_session_data(port, (void *)&session_data->q3a_lib_info,
    session_data, &sessionid);
  return rc;
}

/** awb_module_start_session
 *    @module: awb module
 *    @identity: session identity
 *
 **/
static boolean awb_module_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  boolean rc = FALSE;
  mct_port_t *port = NULL;
  mct_list_t *list = NULL;

  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "awb"))
    return FALSE;

  MCT_OBJECT_LOCK(module);

  if (MCT_MODULE_NUM_SINKPORTS(module) != 0) {
    list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
             &sessionid, awb_module_find_port);
    if (list != NULL) {
      rc = TRUE;
      goto start_done;
    }
  }

  /* Now need to create a new sink port */
  port = mct_port_create("awb_sink");
  if (port == NULL)
    goto start_done;

  if (awb_port_init(port, &sessionid) == FALSE)
    goto port_init_error;

  if (mct_module_add_port(module, port) == FALSE)
    goto port_add_error;

  awb_stored_params_type *stored_params = NULL;
  list = mct_list_find_custom(module->module_private, &sessionid,
    awb_module_find_stored_parm);
  if (list == NULL) {
    stored_params = (awb_stored_params_type *)malloc(sizeof(awb_stored_params_type));
    if (NULL == stored_params) {
      AWB_ERR("Memory allocation failed");
      goto private_init_error;
    }

    stored_params->session_id = sessionid;
    stored_params->first_init = TRUE;
    stored_params->enable = AWB_START_RESTORE;
    module->module_private =
      mct_list_append(module->module_private, stored_params, NULL, NULL);
  } else {
    stored_params = (awb_stored_params_type *)list->data;
  }
  awb_port_set_stored_parm(port, stored_params);

  rc = TRUE;
  goto start_done;

private_init_error:
  mct_module_remove_port(module, port);
port_add_error:
  awb_port_deinit(port);
port_init_error:
  mct_port_destroy(port);
start_done:
  MCT_OBJECT_UNLOCK(module);
  return rc;
}

/** awb_module_stop_session
 *    @module: awb module
 *    @identity: stream|session identity
 *
 *  Deinitialize awb component
 *  Return: boolean
 **/
static boolean awb_module_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  /* Sanity check */

  mct_list_t *list = NULL;
  mct_port_t *port = NULL;
  if (!module || strcmp(MCT_OBJECT_NAME(module), "awb"))
    return FALSE;

  MCT_OBJECT_LOCK(module);

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
           &sessionid, awb_module_find_port);
  if (list == NULL) {
    MCT_OBJECT_UNLOCK(module);
    return FALSE;
  }

  port = MCT_PORT_CAST(list->data);
  mct_module_remove_port(module, port);

  awb_port_deinit(port);

  mct_port_destroy(port);

  MCT_OBJECT_UNLOCK(module);
  return TRUE;
}

/**awb_module_query_mod
 * @module:  awb module
 * @query_buf:
 * @sessionid : idenity
 **/
boolean awb_module_query_mod(mct_module_t *module, void *buf,
  unsigned int sessionid)
{
  (void) module;
  (void) sessionid;
  mct_pipeline_cap_t *query_buf = (mct_pipeline_cap_t *) buf;

  query_buf->stats_cap.auto_wb_lock_supported = TRUE;
  query_buf->stats_cap.supported_white_balances_cnt = CAM_WB_MODE_MAX;
  query_buf->stats_cap.supported_white_balances[0] = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.supported_white_balances[1] = CAM_WB_MODE_CUSTOM;
  query_buf->stats_cap.supported_white_balances[2] = CAM_WB_MODE_INCANDESCENT;
  query_buf->stats_cap.supported_white_balances[3] = CAM_WB_MODE_FLUORESCENT;
  query_buf->stats_cap.supported_white_balances[4] = CAM_WB_MODE_WARM_FLUORESCENT;
  query_buf->stats_cap.supported_white_balances[5] = CAM_WB_MODE_DAYLIGHT;
  query_buf->stats_cap.supported_white_balances[6] = CAM_WB_MODE_CLOUDY_DAYLIGHT;
  query_buf->stats_cap.supported_white_balances[7] = CAM_WB_MODE_TWILIGHT;
  query_buf->stats_cap.supported_white_balances[8] = CAM_WB_MODE_SHADE;
  query_buf->stats_cap.supported_white_balances[9] = CAM_WB_MODE_OFF;
  query_buf->stats_cap.supported_white_balances[10] = CAM_WB_MODE_MANUAL;

  query_buf->stats_cap.supported_scene_modes_cnt = 19;
  query_buf->stats_cap.supported_scene_modes[0] = CAM_SCENE_MODE_OFF;
  query_buf->stats_cap.supported_scene_modes[1]= CAM_SCENE_MODE_AUTO;
  query_buf->stats_cap.supported_scene_modes[2] =
      CAM_SCENE_MODE_LANDSCAPE;
  query_buf->stats_cap.supported_scene_modes[3] = CAM_SCENE_MODE_SNOW;
  query_buf->stats_cap.supported_scene_modes[4] = CAM_SCENE_MODE_BEACH;
  query_buf->stats_cap.supported_scene_modes[5] = CAM_SCENE_MODE_SUNSET;
  query_buf->stats_cap.supported_scene_modes[6] = CAM_SCENE_MODE_NIGHT;
  query_buf->stats_cap.supported_scene_modes[7] =
      CAM_SCENE_MODE_PORTRAIT;
  query_buf->stats_cap.supported_scene_modes[8] =
      CAM_SCENE_MODE_BACKLIGHT;
  query_buf->stats_cap.supported_scene_modes[9] = CAM_SCENE_MODE_SPORTS;
  query_buf->stats_cap.supported_scene_modes[10] =
      CAM_SCENE_MODE_ANTISHAKE;
  query_buf->stats_cap.supported_scene_modes[11] = CAM_SCENE_MODE_FLOWERS;
  query_buf->stats_cap.supported_scene_modes[12] =
      CAM_SCENE_MODE_CANDLELIGHT;
  query_buf->stats_cap.supported_scene_modes[13] =
      CAM_SCENE_MODE_FIREWORKS;
  query_buf->stats_cap.supported_scene_modes[14] = CAM_SCENE_MODE_PARTY;
  query_buf->stats_cap.supported_scene_modes[15] =
      CAM_SCENE_MODE_NIGHT_PORTRAIT;
  query_buf->stats_cap.supported_scene_modes[16] = CAM_SCENE_MODE_THEATRE;
  query_buf->stats_cap.supported_scene_modes[17] = CAM_SCENE_MODE_ACTION;
  query_buf->stats_cap.supported_scene_modes[18] = CAM_SCENE_MODE_AR;

  query_buf->stats_cap.scene_mode_overrides[0].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[1].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[2].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[3].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[4].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[5].awb_mode = CAM_WB_MODE_INCANDESCENT;
  query_buf->stats_cap.scene_mode_overrides[6].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[7].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[8].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[9].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[9].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[10].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[11].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[12].awb_mode = CAM_WB_MODE_INCANDESCENT;
  query_buf->stats_cap.scene_mode_overrides[13].awb_mode = CAM_WB_MODE_CLOUDY_DAYLIGHT;
  query_buf->stats_cap.scene_mode_overrides[14].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[15].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[16].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[17].awb_mode = CAM_WB_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[18].awb_mode = CAM_WB_MODE_AUTO;

  AWB_LOW("X");
  return TRUE;
}

/** awb_module_deinit:
 * @mod: awb module
 *
 * This function deinits awb module. WIll be called by Stats
 * module and triggered by MCT.
 **/
void awb_module_deinit(mct_module_t *module)
{
  if (!module || strcmp(MCT_OBJECT_NAME(module), "awb"))
    return;

  /* Remove the private module data. */
  mct_list_free_list(module->module_private);

  /* Remove all ports of this module */
  mct_list_traverse(MCT_OBJECT_CHILDREN(module), awb_module_remove_port,
    module);

  mct_list_free_list(MCT_OBJECT_CHILDREN(module));

  mct_module_destroy(module);
}

/** awb_module_init:
 *    @name: name of this stats interface module("awb").
 *
 *  Return this stats interface module if succes, otherwise
 *  return NULL.
 **/
mct_module_t* awb_module_init(const char *name)
{
  int i;
  mct_module_t *awb = NULL;

  if (strcmp(name, "awb"))
    return NULL;

  awb = mct_module_create("awb");
  if (!awb)
    return NULL;

  mct_module_set_set_mod_func(awb, awb_module_set_mod);
  mct_module_set_query_mod_func(awb, awb_module_query_mod);
  mct_module_set_start_session_func(awb, awb_module_start_session);
  mct_module_set_stop_session_func(awb, awb_module_stop_session);
  mct_module_set_session_data_func(awb, awb_module_set_session_data);

  return awb;
}

/** awb_module_get_port:
 *    @name:
 *
 *
 *
 **/
mct_port_t *awb_module_get_port(mct_module_t *module, unsigned int sessionid)
{
  mct_list_t *list = NULL;
  if (!module || strcmp(MCT_OBJECT_NAME(module), "awb"))
    return FALSE;

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
           &sessionid, awb_module_find_port);
  if (list == NULL) {
    return NULL;
  }
  return ((mct_port_t *)list->data);
}
