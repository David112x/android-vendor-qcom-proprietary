/* af_module.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_pipeline.h"
#include "af_module.h"
#include "af_port.h"

/* Every AF sink port ONLY corresponds to ONE session */

/** af_module_remove_port
 *    @data:      this is the AF port that needs to be removed from the module
 *    @user_data: this is the AF module, from which the port will be removed
 *
 * Remove, deinit and destroy one port from af module
 *
 * Return TRUE on success, FALSE on failure
 **/
static boolean af_module_remove_port(void *data, void *user_data)
{
  mct_port_t   *port   = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;

  if (!port || !module || strcmp(MCT_OBJECT_NAME(port), "af_sink") ||
    strcmp(MCT_OBJECT_NAME(module), "af")) {
    return FALSE;
  }

  mct_module_remove_port(module, port);
  af_port_deinit(port);
  mct_port_destroy(port);

  return TRUE;
}

/** af_module_find_port:
 *    @data1: pointer to the AF port
 *    @data2: identity object to be checked
 *
 *  Check if this port has already been linked by the same session.
 *
 *  Return TRUE if the port has the same session id, otherwise return FALSE.
 **/
static boolean af_module_find_port(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  unsigned int *id = (unsigned int *)data2;

  return af_port_find_identity(port, *id);
}

/** af_module_set_mod:
 *    @module:      The AF module
 *    @module_type: The type of the module
 *    @identity:    The identity
 *
 * This function should not be allowed by any
 * modules to overwrite module type
 *
 * Return nothing
 **/
void af_module_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  (void) module;
  (void) module_type;
  (void) identity;
  return;
}

/** af_module_set_session_data:
 *    @module:   af module
 *    @set_buf: session data
 *    @sessionid: session id
 *
 *  Use session data for initialization
 *  Return: true if success
 **/
static boolean af_module_set_session_data(mct_module_t *module,
  void *set_buf,
  unsigned int sessionid)
{
  mct_list_t *list = NULL;
  mct_port_t *port = NULL;
  boolean rc = FALSE;
  mct_pipeline_session_data_t *session_data =
    (mct_pipeline_session_data_t *)set_buf;

  AF_HIGH("module %p, sessionid = 0x%x", module, sessionid);
  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "af")) {
    AF_ERR("error: invalid module");
    return FALSE;
  }
  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
           &sessionid, af_module_find_port);
  if (list == NULL) {
    AF_ERR("error: empty port list");
    return FALSE;
  }
  if (!(Q3A_LIB_UPDATE_MASK & session_data->set_session_mask)) {
    return TRUE; /* Not an error but msg not for this module */
  }

  port = MCT_PORT_CAST(list->data);
  rc = af_port_set_session_data(port, (void *)&session_data->q3a_lib_info,
    session_data, &sessionid);
  return rc;
}

/** af_module_start_session:
 *    @module:   The af module
 *    @identity: The session identity
 *
 * This function will be called upon starting of a new session in order to
 * create the AF port, to initialize it and to add it to the AF module
 *
 * Return TRUE on success, FALSE on failure
 **/
static boolean af_module_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  boolean    rc = FALSE;
  mct_port_t *port = NULL;
  mct_list_t *list = NULL;

  AF_LOW("AF Module - start session");
  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "af")){
    AF_LOW("Module name does not match. Returning!");
    return FALSE;
  }

  MCT_OBJECT_LOCK(module);

  if (MCT_MODULE_NUM_SINKPORTS(module) != 0) {
    list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
      &sessionid, af_module_find_port);
    if (list != NULL) {
      rc = TRUE;
      goto start_done;
    }
  }

  /* Now need to create a new sink port */
  port = mct_port_create("af_sink");
  if (port == NULL) {
    AF_ERR("Failed to create AF port");
    goto start_done;
  }

  if (af_port_init(port, &sessionid) == FALSE) {
    AF_ERR("Failed to initialize AF Port")
    goto port_init_error;
  }


  if (mct_module_add_port(module, port) == FALSE) {
    AF_ERR("mct_module add port failed");
    goto port_add_error;
  }

  rc = TRUE;
  goto start_done;

port_add_error:
  af_port_deinit(port);
port_init_error:
  mct_port_destroy(port);
start_done:
  MCT_OBJECT_UNLOCK(module);
  return rc;
}

/** af_module_stop_session:
 *    @module:   The af module
 *    @identity: stream|session identity
 *
 *  Remove AF port from the AF module, deinitialize it and destroy it.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean af_module_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  mct_list_t* list;
  mct_port_t *port = NULL;
  AF_LOW("AF Module - stop session");
  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "af")){
    AF_ERR("Module name does not match.");
    return FALSE;
  }

  MCT_OBJECT_LOCK(module);
  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
    &sessionid, af_module_find_port);
  if (list == NULL) {
    MCT_OBJECT_UNLOCK(module);
    return FALSE;
  }

  port = MCT_PORT_CAST(list->data);
  AF_LOW("Remove AF port from the list!");
  mct_module_remove_port(module, port);

  AF_LOW("Deinit AF port");
  af_port_deinit(port);

  AF_LOW("Destroy AF port");
  mct_port_destroy(port);
  MCT_OBJECT_UNLOCK(module);

  return TRUE;
}

/** af_module_query_mod:
 *   @module:    The af module
 *   @query_buf: buffer with the module's capabilities
 *   @sessionid: session identity
 *
 * This function will set the AF capabilities of the stats module
 *
 * Return TRUE on success, FALSE on failure
 **/
boolean af_module_query_mod(mct_module_t *module, void *buf,
  unsigned int sessionid)
{
  (void) module;
  (void) sessionid;
  mct_pipeline_cap_t *query_buf = (mct_pipeline_cap_t *) buf;
  query_buf->stats_cap.max_num_focus_areas = 1;
  query_buf->stats_cap.supported_focus_modes_cnt = 7;
  query_buf->stats_cap.supported_focus_modes[0] = CAM_FOCUS_MODE_AUTO;
  query_buf->stats_cap.supported_focus_modes[1] = CAM_FOCUS_MODE_INFINITY;
  query_buf->stats_cap.supported_focus_modes[2] = CAM_FOCUS_MODE_FIXED;
  query_buf->stats_cap.supported_focus_modes[3] = CAM_FOCUS_MODE_MACRO;
  query_buf->stats_cap.supported_focus_modes[4] =
    CAM_FOCUS_MODE_CONTINOUS_VIDEO;
  query_buf->stats_cap.supported_focus_modes[5] =
    CAM_FOCUS_MODE_CONTINOUS_PICTURE;
  query_buf->stats_cap.supported_focus_modes[6] = CAM_FOCUS_MODE_MANUAL;
  query_buf->stats_cap.video_stablization_supported = TRUE;

  query_buf->stats_cap.supported_focus_algos[0] = CAM_FOCUS_ALGO_AUTO;
  query_buf->stats_cap.supported_focus_algos[1] = CAM_FOCUS_ALGO_SPOT;
  query_buf->stats_cap.supported_focus_algos[2] = CAM_FOCUS_ALGO_CENTER_WEIGHTED;
  query_buf->stats_cap.supported_focus_algos[3] = CAM_FOCUS_ALGO_AVERAGE;
  query_buf->stats_cap.supported_focus_algos_cnt = CAM_FOCUS_ALGO_MAX;

  query_buf->stats_cap.scene_mode_overrides[2].af_mode = CAM_FOCUS_MODE_FIXED; //CAM_FOCUS_MODE_INFINITY
  query_buf->stats_cap.scene_mode_overrides[3].af_mode = AF_MODE_NORMAL; //find a corresp. mode
  query_buf->stats_cap.scene_mode_overrides[4].af_mode = AF_MODE_NORMAL;
  query_buf->stats_cap.scene_mode_overrides[5].af_mode = CAM_FOCUS_MODE_FIXED;
  query_buf->stats_cap.scene_mode_overrides[6].af_mode = AF_MODE_NORMAL;
  query_buf->stats_cap.scene_mode_overrides[7].af_mode = AF_MODE_NORMAL;
  query_buf->stats_cap.scene_mode_overrides[8].af_mode = AF_MODE_NORMAL;
  query_buf->stats_cap.scene_mode_overrides[9].af_mode = CAM_FOCUS_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[9].af_mode = CAM_FOCUS_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[10].af_mode = CAM_FOCUS_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[11].af_mode = CAM_FOCUS_MODE_MACRO;
  query_buf->stats_cap.scene_mode_overrides[12].af_mode = AF_MODE_NORMAL;
  query_buf->stats_cap.scene_mode_overrides[13].af_mode = CAM_FOCUS_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[14].af_mode = AF_MODE_NORMAL;
  query_buf->stats_cap.scene_mode_overrides[15].af_mode = CAM_FOCUS_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[16].af_mode = CAM_FOCUS_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[17].af_mode = CAM_FOCUS_MODE_AUTO;
  query_buf->stats_cap.scene_mode_overrides[18].af_mode = CAM_FOCUS_MODE_AUTO;
  return TRUE;
}

/** af_module_deinit:
 *   @module: The AF module
 *
 * This function deinits the AF module. Will be called by Stats
 * module and triggered by MCT.
 *
 * Return nothing
 **/
void af_module_deinit(mct_module_t *module)
{
  if (!module || strcmp(MCT_OBJECT_NAME(module), "af")) {
    return;
  }

  /* Remove all ports of this module */
  mct_list_traverse(MCT_OBJECT_CHILDREN(module), af_module_remove_port,
    module);

  mct_list_free_list(MCT_OBJECT_CHILDREN(module));

  mct_module_destroy(module);
}

/** af_module_init:
 *    @name: The name of this stats interface module ("af").
 *
 * This function will create the AF module and will set the API function
 * pointers with the AF module's functions.
 *
 * Return this stats interface module on success, otherwise return NULL.
 **/
mct_module_t* af_module_init(const char *name)
{
  int i;
  mct_module_t *af;

  if (strcmp(name, "af")) {
    return NULL;
  }

  af = mct_module_create("af");
  if (!af) {
    return NULL;
  }

  mct_module_set_set_mod_func(af, af_module_set_mod);
  mct_module_set_query_mod_func(af, af_module_query_mod);
  mct_module_set_start_session_func(af, af_module_start_session);
  mct_module_set_stop_session_func(af, af_module_stop_session);
  mct_module_set_session_data_func(af, af_module_set_session_data);

  return af;
}

/** af_module_get_port:
 *    @module:    The AF module
 *    @sessionid: session identity
 *
 *  This function will get the AF port for the given identity.
 *
 * On success return the found AF port, otherwise return NULL.
 **/
mct_port_t *af_module_get_port(mct_module_t *module, unsigned int sessionid)
{
  mct_list_t *list = NULL;

  if (!module || strcmp(MCT_OBJECT_NAME(module), "af")) {
    return FALSE;
  }

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
    &sessionid, af_module_find_port);
  if (list == NULL) {
    return NULL;
  }

  return ((mct_port_t *)list->data);
}
