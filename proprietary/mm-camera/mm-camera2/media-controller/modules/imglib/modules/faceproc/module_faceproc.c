/**********************************************************************
*  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <linux/media.h>
#include "modules.h"
#include "mct_module.h"
#include "module_faceproc.h"
#include "mct_stream.h"
#include "mct_port.h"
#include "mct_pipeline.h"
#include "mct_profiler.h"


#define FD_ENABLE(p) ((p->config.fd_feature_mask & FACE_PROP_ENABLE_FD) \
  && p->p_fd_chromatix->enable)

/**
 * STATIC function declarations
 **/
static mct_port_t *module_faceproc_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir);

/**
 * Validate session id
 **/
#define FP_VALIDATE_SESSION_ID(event, p_client, i) \
  ((event->identity != p_client->fp_stream[i].identity) && \
  (IMGLIB_SESSIONID(event->identity) == \
  IMGLIB_SESSIONID(p_client->fp_stream[i].identity)))

/**
 * Function: faceproc_get_stream_by_id
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_client: faceproc client
 *   @identity: input identity
 *
 * Return values:
 *     -1 on fail
 *
 * Notes: none
 **/
int faceproc_get_stream_by_id(faceproc_client_t *p_client,
  uint32_t identity)
{
  uint32_t i = 0;
  int s_idx = -1;
  for (i = 0; i < p_client->ref_count; i++) {
    if (identity == p_client->fp_stream[i].identity) {
      s_idx = i;
      break;
    }
  }
  return s_idx;
}

/**
 * Function: faceproc_get_stream_by_type
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_client: faceproc client
 *   @identity: input identity
 *
 * Return values:
 *     -1 if invalid
 *     index for the stream
 *
 * Notes: none
 **/
int faceproc_get_stream_by_type(faceproc_client_t *p_client,
  cam_stream_type_t type)
{
  uint32_t i = 0;
  int s_idx = -1;
  for (i = 0; i < p_client->ref_count; i++) {
    if (type == p_client->fp_stream[i].stream_info->stream_type) {
      s_idx = i;
      break;
    }
  }

  return s_idx;
}

/**
 * Function: module_faceproc_find_client
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_fp_data: faceproc client
 *   @p_input: input data
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_faceproc_find_client(void *p_fp_data, void *p_input)
{
  faceproc_client_t *p_client = (faceproc_client_t *)p_fp_data;
  uint32_t identity = *((uint32_t *)p_input);

  return (p_client->fp_stream[0].identity == identity) ? TRUE : FALSE;
}


/**
 * Function: module_faceproc_find_client_by_session
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_fp_data: faceproc client
 *   @p_input: input data
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_faceproc_find_client_by_session(void *p_fp_data,
  void *p_input)
{
  if (!p_fp_data || !p_input)
    return FALSE;

  faceproc_client_t *p_client = (faceproc_client_t *)p_fp_data;
  uint32_t identity = *((uint32_t *)p_input);

  return (IMGLIB_SESSIONID(p_client->fp_stream[0].identity) ==
    IMGLIB_SESSIONID(identity)) ? TRUE : FALSE;
}

/**
 * Function: module_faceproc_find_session_params
 *
 * Description: This method is used to find the the session based parameters
 *
 * Arguments:
 *   @p_data: data in the mct list
 *   @p_input: input data to be seeked
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_faceproc_find_session_params(void *p_data, void *p_input)
{

  faceproc_session_params_t *stored_param = p_data;
  uint32_t session_id = *((uint32_t *)p_input);

  return (stored_param->session_id == session_id) ? TRUE : FALSE;
}

/**
 * Function: module_faceproc_find_identity
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_data: data in the mct list
 *   @p_input: input data to be seeked
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean __unused module_faceproc_find_identity(void *p_data,
  void *p_input)
{
  uint32_t *p_identity = (uint32_t *)p_data;
  uint32_t identity = *((uint32_t *)p_input);

  return (*p_identity == identity) ? TRUE : FALSE;
}

/**
 * Function: module_faceproc_set_multi_client
 *
 * Description: Callback function provided in mct_list_traverse used
 *  for updating multi session clients.
 *
 * Arguments:
 *   @p_data: pointer to client structure.
 *   @p_user: Pointer to multi session flag.
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: This function holds client mutex.
 **/
static boolean module_faceproc_set_multi_client(void *p_data, void *p_user)
{
  boolean *p_multi_client = (boolean *)p_user;
  faceproc_client_t *p_client = (faceproc_client_t *)p_data;

  if (!p_multi_client || !p_client) {
    return FALSE;
  }

  pthread_mutex_lock(&p_client->mutex);
  p_client->multi_client = *p_multi_client;
  pthread_mutex_unlock(&p_client->mutex);

  return TRUE;
}

/**
 * Function: module_faceproc_update_multi_client
 *
 * Description: Update multi session flag in all active clients.
 *
 * Arguments:
 *   @p_mod: Face proc private data
 *
 * Return values:
 *     Imglib error codes
 *
 * Notes: It should be protected with module mutex before calling.
 **/
static int module_faceproc_update_multi_client(module_faceproc_t *p_mod)
{
  boolean multi_client;

  if (!p_mod) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  if (p_mod->client_cnt && p_mod->fp_client) {
    multi_client = p_mod->client_cnt > 1 ? TRUE : FALSE;
    mct_list_traverse(p_mod->fp_client,
      module_faceproc_set_multi_client, &multi_client);
  }

  return IMG_SUCCESS;
}

/**
 * Function: module_faceproc_store_session_param
 *
 * Description: This method is used to store per session based parameters
 *
 * Arguments:
 *   @p_mod: Face proc private data
 *   @p_client: Client handle to apply the restored parameters
 *   @param: Event parameters which need to be stored
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int module_faceproc_store_session_param(module_faceproc_t *p_mod,
  faceproc_client_t *p_client, mct_event_control_parm_t *param)
{
  if (!p_mod || !param)
    return IMG_ERR_INVALID_INPUT;

  switch(param->type) {
  case CAM_INTF_PARM_FD:
  case CAM_INTF_PARM_USERZOOM: {
    uint32_t session_id = IMGLIB_SESSIONID(p_client->fp_stream[0].identity);
    mct_list_t *p_temp_list;
    faceproc_session_params_t *stored_param;

    if (NULL == param->parm_data)
      return IMG_ERR_INVALID_INPUT;

    /* Find settings per session id */
    p_temp_list = mct_list_find_custom(p_mod->session_parms, &session_id,
      module_faceproc_find_session_params);
    if (!p_temp_list)
      return IMG_ERR_GENERAL;

    stored_param = (faceproc_session_params_t *) p_temp_list->data;
    stored_param->valid_params = TRUE;

    if (CAM_INTF_PARM_FD == param->type) {
      cam_fd_set_parm_t *p_fd_set_parm = (cam_fd_set_parm_t *)param->parm_data;
      stored_param->param.fd_enable = FALSE;

      if (p_fd_set_parm->fd_mode &
        (CAM_FACE_PROCESS_MASK_DETECTION | CAM_FACE_PROCESS_MASK_FOCUS))
        stored_param->param.fd_enable = TRUE;

      stored_param->param.fr_enable = FALSE;
      if (CAM_FACE_PROCESS_MASK_RECOGNITION & p_fd_set_parm->fd_mode)
        stored_param->param.fr_enable = TRUE;
    } else if (CAM_INTF_PARM_USERZOOM == param->type) {
      cam_zoom_info_t *p_zoom_val = (cam_zoom_info_t *)param->parm_data;
      stored_param->param.zoom_val = *p_zoom_val;
    }

    break;
  }
  default:
    break;
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_faceproc_restore_session_param
 *
 * Description: This method is used to restore per session based parameters
 *
 * Arguments:
 *   @p_mod: Face proc private data
 *   @p_client: Client handle to apply the restored parameters
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int module_faceproc_restore_session_param(module_faceproc_t *p_mod,
  faceproc_client_t *p_client)
{
  mct_list_t *p_temp_list;
  faceproc_session_params_t *stored_param;
  uint32_t session_id;

  if (!p_mod || !p_client)
    return IMG_ERR_INVALID_INPUT;

  session_id = IMGLIB_SESSIONID(p_client->fp_stream[0].identity);

  /* Find settings per session id */
  p_temp_list = mct_list_find_custom(p_mod->session_parms, &session_id,
    module_faceproc_find_session_params);
  if (!p_temp_list)
    return IMG_ERR_GENERAL;

  stored_param = p_temp_list->data;
  if (!stored_param)
    return IMG_ERR_GENERAL;

  /* Do not restore session modes when client is in face register mode */
  if ((FALSE == stored_param->valid_params) ||
    (p_client->mode == FACE_REGISTER))
    return IMG_SUCCESS;

  if (stored_param->param.fr_enable)
    module_faceproc_client_set_mode(p_client, FACE_RECOGNIZE, TRUE);
  else if (stored_param->param.fd_enable)
    module_faceproc_client_set_mode(p_client, FACE_DETECT, TRUE);

  p_client->zoom_val = stored_param->param.zoom_val;
  return IMG_SUCCESS;
}

/**
 * Function: module_faceproc_create_session_param
 *
 * Description: This method is used to create session parameters
 * it will add new list with parameters session based
 *
 * Arguments:
 *   @p_mod: Face proc private data
 *   @session_id Session id
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int module_faceproc_create_session_param(module_faceproc_t *p_mod,
  uint32_t session_id)
{
  faceproc_session_params_t *stored_param;

  if (!p_mod)
    return IMG_ERR_INVALID_INPUT;

  stored_param = malloc(sizeof(*stored_param));
  if (NULL == stored_param)
    return IMG_ERR_NO_MEMORY;

  memset(stored_param, 0x0, sizeof(faceproc_session_params_t));
  stored_param->session_id = session_id;
  stored_param->valid_params = FALSE;
  stored_param->param.fd_enable = FALSE;
  stored_param->param.fr_enable = FALSE;
  memset(&stored_param->param.zoom_val, 0, sizeof(cam_zoom_info_t));

  p_mod->session_parms = mct_list_append(p_mod->session_parms,
    stored_param, NULL, NULL);

  return (p_mod->session_parms) ? IMG_SUCCESS : IMG_ERR_GENERAL;
}

/**
 * Function: module_faceproc_destroy_session_param
 *
 * Description: This method is used to destroy session parameters
 * it will add new list with parameters session based
 *
 * Arguments:
 *   @p_mod: Face proc private data
 *   @session_id Session id
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int module_faceproc_destroy_session_param(module_faceproc_t *p_mod,
  uint32_t session_id)
{
  faceproc_session_params_t *stored_param;
  mct_list_t *p_temp_list;

  if (!p_mod)
    return IMG_ERR_INVALID_INPUT;

  /* Find paramters per session id */
  p_temp_list = mct_list_find_custom(p_mod->session_parms, &session_id,
    module_faceproc_find_session_params);
  if (!p_temp_list)
    return IMG_ERR_INVALID_INPUT;


  stored_param = ( faceproc_session_params_t *)p_temp_list->data;
  p_mod->session_parms = mct_list_remove(p_mod->session_parms, stored_param);
  free(stored_param);

  return IMG_SUCCESS;
}

/**
 * Function: module_faceproc_acquire_port
 *
 * Description: This function is used to acquire the port
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @port: mct port pointer
 *   @stream_info: stream information
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_faceproc_acquire_port(mct_module_t *p_mct_mod,
  mct_port_t *port,
  mct_stream_info_t *stream_info)
{
  int rc = IMG_SUCCESS;
  unsigned int p_identity ;
  mct_list_t *p_temp_list = NULL;
  faceproc_client_t *p_client = NULL;
  module_faceproc_t *p_mod = NULL;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return FALSE;
  }
  p_identity =  stream_info->identity;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {
    /* create faceproc client */
    IDBG_MED("%s:%d]: Port %s is sink port",  __func__, __LINE__,
      MCT_PORT_NAME(port));
    rc = module_faceproc_client_create(p_mct_mod, port, p_identity,
      stream_info);
    if (IMG_SUCCEEDED(rc)) {
      /* add the client to the list */
      p_client = (faceproc_client_t *) port->port_private;
      p_temp_list = mct_list_append(p_mod->fp_client, p_client, NULL, NULL);
      if (NULL == p_temp_list) {
        IDBG_ERROR("%s:%d] list append failed", __func__, __LINE__);
        rc = IMG_ERR_GENERAL;
        goto error;
      }
      p_mod->fp_client = p_temp_list;

      p_mod->client_cnt++;
      module_faceproc_update_multi_client(p_mod);

    }
  } else {
    /* update the internal connection with source port */
    IDBG_MED("%s:%d]: Port %s is src port",  __func__, __LINE__,
      MCT_PORT_NAME(port));
    p_temp_list = mct_list_find_custom(p_mod->fp_client, &p_identity,
      module_faceproc_find_client);
    if (NULL != p_temp_list) {
      p_client = p_temp_list->data;
      port->port_private = p_client;
      IDBG_MED("%s:%d] found client %p", __func__, __LINE__, p_client);
    } else {
      IDBG_ERROR("%s:%d] cannot find the client", __func__, __LINE__);
      goto error;
    }
  }
  IDBG_MED("%s:%d] port %p port_private %p X", __func__, __LINE__,
    port, port->port_private);
  return GET_STATUS(rc);

error:

  IDBG_ERROR("%s:%d] Error X", __func__, __LINE__);
  return FALSE;
}

/**
 * Function: module_faceproc_handle_streamon
 *
 * Description: Function to handle faceproc streamon
 *
 * Arguments:
 *   @p_mod: faceproc module pointer
 *   @p_client: faceproc client pointer
 *   @identity: Identity of the stream
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
int module_faceproc_handle_streamon(module_faceproc_t *p_mod,
  faceproc_client_t *p_client, uint32_t identity)
{
  IMG_UNUSED(p_mod);

  int rc = IMG_SUCCESS;
  uint32_t i;
  int s_idx;

  s_idx = faceproc_get_stream_by_id(p_client, identity);
  if (s_idx < 0) {
    IDBG_ERROR("Cannot find stream idx for stream identity 0x%x", identity);
    return IMG_ERR_GENERAL;
  }

  if (!p_client->fp_stream[s_idx].buff_configured) {
    IDBG_ERROR("Stream 0x%x not ready", identity);
    return IMG_ERR_GENERAL;
  }

  /*Skip if it is not processing stream*/
  if (!p_client->fp_stream[s_idx].processing_stream) {
    return IMG_SUCCESS;
  }

  rc = module_faceproc_client_map_buffers(p_client, s_idx);
  if (IMG_SUCCEEDED(rc)) {
    rc = module_faceproc_client_start(p_client);
    if (IMG_SUCCEEDED(rc)) {
      /* clear the per frame control queue */
      for (i = 0; i < p_client->fpc_q_size; i++) {
      /* flush and remove the elements of the queue */
        img_q_flush_and_destroy(&p_client->p_fpc_q[i]);
      }
    } else {
      IDBG_ERROR("Error cannot start %d", rc);
    }
  } else {
    IDBG_ERROR("Error cannot map buffers %d", rc);
  }

  memset(&p_client->fd_profile, 0x0, sizeof(img_profiling_t));

  if (p_client->debug_settings.profile_fd_level >= FD_PROFILE_LEVEL_STREAM) {
    // fd_profile.start_time[] usage:
    //   Use 0 to (PROFILE_MAX_VAL-2) for saving a particular
    //   frame processing time. Since the BUF_DIVERTs come asynchronously,
    //   we need to store start time in different indices for different frames.
    //   Calculate the frame processing time while sending the ACK (that when
    //   we finished processing this frame) using the index based on frameId.
    //   Use (PROFILE_MAX_VAL-1) for saving StreamOn time. Calculate total
    //   stream time in StreamOff based on this.

    // Save StreamOn time in last index.
    IMG_TIMER_START(p_client->fd_profile.start_time[PROFILE_MAX_VAL - 1]);

    // For fps print every second
    p_client->fd_profile.intermediate1[PROFILE_MAX_VAL - 1] =
      p_client->fd_profile.start_time[PROFILE_MAX_VAL - 1];

    IDBG_INFO("FDPROFILE-Timeline StreamOn at 0.000");
  }

  p_client->current_frame_cnt = 0;

  return rc;
}

/**
 * Function: module_faceproc_handle_streamoff
 *
 * Description: Function to handle faceproc streamoff
 *
 * Arguments:
 *   @p_mod: faceproc module pointer
 *   @p_client: faceproc client pointer
 *   @identity: Identity of the stream
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
int module_faceproc_handle_streamoff(module_faceproc_t *p_mod,
  faceproc_client_t *p_client, uint32_t identity)
{
  int rc = IMG_SUCCESS;
  uint32_t i;

  if ((p_client->state != IMGLIB_STATE_STARTED) &&
    (p_client->state != IMGLIB_STATE_PROCESSING) &&
    (p_client->state != IMGLIB_STATE_START_PENDING)) {
     IDBG_ERROR("%s:%d] client not started state %d", __func__, __LINE__,
       p_client->state);
     return IMG_SUCCESS;
  }

  if (p_client->debug_settings.profile_fd_level >= FD_PROFILE_LEVEL_STREAM) {
    uint32_t stream_time =
      IMG_TIMER_END(p_client->fd_profile.start_time[PROFILE_MAX_VAL - 1],
      p_client->fd_profile.end_time[PROFILE_MAX_VAL - 1],
      "FDStreamTime", IMG_TIMER_MODE_MS);

    uint32_t processing_time =
      p_client->fd_profile.total_time_in_ms[FD_FRAME_PROCESSING_TIME];

    IDBG_INFO("FDPROFILE : TotalStreamTime=%u ms, TotalProcessingTime=%u ms",
      stream_time, processing_time);
    IDBG_INFO("FDPROFILE : TotalFrames=%d, FrameProcessed=%d, "
      "FramesSkipped=%d",
      p_client->fd_profile.num_frames_total,
      p_client->fd_profile.num_frames_processed,
      p_client->fd_profile.num_frames_skipped);

    if (processing_time && p_client->fd_profile.num_frames_processed) {
      IDBG_INFO("FDPROFILE : Avg PrcessingTime of single Frame=%.2f (FPS=%.2f)",
        (float)processing_time /
        (float)p_client->fd_profile.num_frames_processed,
        (float)(p_client->fd_profile.num_frames_processed * 1000) /
        (float)processing_time);
    }

    if (stream_time && p_client->fd_profile.num_frames_processed) {
      IDBG_INFO("FDPROFILE : FDFPS=%.2f, InputStreamFPS=%.2f",
        (float)(p_client->fd_profile.num_frames_processed * 1000) /
        (float)stream_time,
        (float)(p_client->fd_profile.num_frames_total * 1000) /
        (float)stream_time);

      if (p_client->fd_profile.num_frames_skipped == 0) {
        IDBG_INFO("FDPROFILE : FD module's FPS is limitted by STREAM input")
      }
    }
  }

  rc = module_faceproc_client_stop(p_client, identity);
  if (IMG_SUCCEEDED(rc))
    rc = module_faceproc_client_unmap_buffers(p_client, identity);
  else
    IDBG_ERROR("%s:%d] Error cannot stop %d", __func__, __LINE__, rc);

  /* Flush the queue */
  IDBG_MED("%s:%d] queue count %d", __func__, __LINE__,
    img_q_count(&p_mod->msg_thread.msg_q));
  img_q_flush_traverse(&p_mod->msg_thread.msg_q,
    module_faceproc_client_flush, p_client);

  /* clear the per frame control queue */
  for (i = 0; i < p_client->fpc_q_size; i++) {
    /* flush and remove the elements of the queue */
    img_q_flush_and_destroy(&p_client->p_fpc_q[i]);
  }

  return rc;
}

/**
 * Function: module_faceproc_port_event_func
 *
 * Description: Event handler function for the faceproc port
 *
 * Arguments:
 *   @p_mod: faceproc module pointer
 *   @p_client: faceproc client pointer
 *   @event: MCT event pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_faceproc_handle_sof_set_parm(module_faceproc_t *p_mod,
  faceproc_client_t *p_client,
  mct_event_t* event)
{
  int rc = IMG_SUCCESS;
  mct_event_super_control_parm_t *param = NULL;
  uint32_t index = 0;
  if (!p_client || !event) {
    IDBG_ERROR("%s:%d] failed: invalid params client %p event %p ",
      __func__, __LINE__, p_client, event);
    return FALSE;
  }

  IDBG_MED("%s:%d] frame_id %d", __func__, __LINE__,
    event->u.ctrl_event.current_frame_id);
  param =
    (mct_event_super_control_parm_t *)event->u.ctrl_event.control_event_data;
  if (!param) {
    IDBG_ERROR("%s:%d] failed: param NULL", __func__, __LINE__);
    return FALSE;
  }

  /* Handle all set params */
  for (index = 0; index < param->num_of_parm_events; index++) {
    rc = module_faceproc_client_handle_ctrl_parm(p_client,
      &param->parm_events[index],
      (int32_t)event->u.ctrl_event.current_frame_id);
    if (IMG_SUCCEEDED(rc)){
      module_faceproc_store_session_param(p_mod, p_client,
          &param->parm_events[index]);
    }
  }
  return TRUE;
}

/**
 * Function: module_faceproc_port_event_func
 *
 * Description: Event handler function for the faceproc port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_faceproc_port_event_func(mct_port_t *port,
  mct_event_t *event)
{
  int rc = IMG_SUCCESS;
  mct_module_t *p_mct_mod = NULL;
  module_faceproc_t *p_mod = NULL;
  faceproc_client_t *p_client;
  int s_idx;
  faceproc_stream_t *p_cur_fstream;

  if (!port || !event) {
    IDBG_ERROR("invalid input");
    return FALSE;
  }

  IDBG_LOW("port %p id %x E", port, event->identity);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("invalid module");
    return FALSE;
  }

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("faceproc module NULL");
    return FALSE;
  }

  p_client = (faceproc_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("faceproc client NULL");
    return FALSE;
  }


  s_idx = faceproc_get_stream_by_id(p_client, event->identity);
  if (s_idx < 0) {
    IDBG_ERROR("Invalid index for identity %x", event->identity);
    return FALSE;
  }
  p_cur_fstream = &p_client->fp_stream[s_idx];

  IDBG_LOW("type %d s_idx %d", event->type, s_idx);

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    mct_event_control_t *p_ctrl_event = &event->u.ctrl_event;
    IDBG_LOW("Ctrl type %d", p_ctrl_event->type);

    switch (p_ctrl_event->type) {
    case MCT_EVENT_CONTROL_STREAMON: {
      if (!FD_ENABLE(p_client)) {
        /* hack to prevent faceproc from execution*/
        IDBG_HIGH("###Disable faceproc");
        break;
      }

      /* Stream on only processing stream */
      if (p_client->fp_stream[s_idx].processing_stream) {
        /* restore the parameters */
        module_faceproc_restore_session_param(p_mod, p_client);
        pthread_mutex_lock(&p_mod->mutex);
        pthread_mutex_lock(&p_client->mutex);
        if (!p_client->active) {
          p_client->active = TRUE;
          p_mod->active = TRUE;
          IDBG_MED("p mod active now");
          if (p_client->fp_stream[s_idx].streamon) {
            IDBG_HIGH("FD_STREAMON already done for stream %x",
              p_client->fp_stream[s_idx].identity);
          } else {
            IDBG_HIGH("FD_STREAMON for stream %x",
              p_client->fp_stream[s_idx].identity);
            p_client->fp_stream[s_idx].streamon = TRUE;
            rc = module_faceproc_handle_streamon(p_mod, p_client, event->identity);
          }
        }
        else {
          IDBG_HIGH("p mod was active already");
        }
        pthread_mutex_unlock(&p_client->mutex);
        pthread_mutex_unlock(&p_mod->mutex);
      }

      break;
    }
    case MCT_EVENT_CONTROL_STREAMOFF: {
      if (p_client->fp_stream[s_idx].processing_stream) {
        pthread_mutex_lock(&p_mod->mutex);
        pthread_mutex_lock(&p_client->mutex);
        if (!p_client->active) {
          IDBG_ERROR("%s:%d] FD_STREAMOFF Not active %x", __func__, __LINE__,
            p_client->fp_stream[s_idx].identity);
        } else if (p_client->fp_stream[s_idx].streamon) {
          IDBG_HIGH("%s:%d] FD_STREAMOFF %d %x", __func__, __LINE__,
            p_client->mode,
            p_client->fp_stream[s_idx].identity);
          p_client->fp_stream[s_idx].streamon = FALSE;
          rc = module_faceproc_handle_streamoff(p_mod, p_client,event->identity);
          p_mod->active = FALSE;
        }

        pthread_mutex_unlock(&p_client->mutex);
        pthread_mutex_unlock(&p_mod->mutex);
      }

      break;
    }
    case MCT_EVENT_CONTROL_SET_PARM: {
      rc = module_faceproc_client_handle_ctrl_parm(p_client,
        p_ctrl_event->control_event_data, 0);
      if (IMG_SUCCEEDED(rc))
        module_faceproc_store_session_param(p_mod, p_client,
          p_ctrl_event->control_event_data);
      break;
    }

    case MCT_EVENT_CONTROL_PARM_STREAM_BUF: {
      cam_stream_parm_buffer_t *parm_buf;

      if (!event->u.ctrl_event.control_event_data) {
        IDBG_ERROR("%s:%d] Invalid input argument ", __func__, __LINE__);
        rc = IMG_ERR_INVALID_INPUT;
        break;
      }
      parm_buf = event->u.ctrl_event.control_event_data;
      if (parm_buf->type == CAM_STREAM_PARAM_TYPE_SET_FLIP) {
        p_client->flip_mask = parm_buf->flipInfo.flip_mask;
      }
      if (parm_buf->type != CAM_STREAM_PARAM_TYPE_DO_REPROCESS) {
        IDBG_MED("%s:%d] We handle only do reprocess skip identity %x",
          __func__, __LINE__,
          event->identity);
        break;
      }

      /* Start with face recognition process */
      parm_buf->reprocess.ret_val = -1;
      rc = module_faceproc_client_configure_buffers(p_client, NULL, 0);
      if (IMG_ERROR(rc))
        break;
      rc = module_faceproc_client_process_buffers(p_client);
      if (IMG_ERROR(rc))
        break;
      if (p_client->result[0].num_faces_detected)
        parm_buf->reprocess.ret_val = p_client->result[0].roi[0].unique_id;


      break;
    }
    case MCT_EVENT_CONTROL_SOF: {
      MCT_PROF_LOG_BEG(PROF_FACEPROC_SOF);
      if (p_client->active) {
        ATRACE_BEGIN("faceproc SOF");
        rc = module_faceproc_client_handle_sof(p_client,
          p_ctrl_event->current_frame_id);
        ATRACE_END();
      } else {
        IDBG_HIGH("%s:%d] active %d ", __func__, __LINE__, p_client->active);
        rc = IMG_SUCCESS;
      }
      MCT_PROF_LOG_END();
      break;
    }

    case MCT_EVENT_CONTROL_SET_SUPER_PARM: {
      MCT_PROF_LOG_BEG(PROF_FACEPROC_SP);
      module_faceproc_handle_sof_set_parm(p_mod, p_client, event);
      MCT_PROF_LOG_END();
      break;
    }

    default:
      break;
    }
    break;
  }
  case MCT_EVENT_MODULE_EVENT: {
    mct_event_module_t *p_mod_event = &event->u.module_event;
    IDBG_LOW("%s:%d] Mod type %d", __func__, __LINE__, p_mod_event->type);

    switch (p_mod_event->type) {
    case MCT_EVENT_MODULE_BUF_DIVERT: {
      mod_img_msg_t msg;
      isp_buf_divert_t *p_buf_divert =
        (isp_buf_divert_t *)p_mod_event->module_event_data;
      uint32_t profile_indx = p_buf_divert->buffer.sequence % (PROFILE_MAX_VAL - 1);

      p_client->fd_profile.num_frames_total++;
      if (p_client->debug_settings.profile_fd_level >= FD_PROFILE_LEVEL_STREAM) {
        IMG_TIMER_START(p_client->fd_profile.start_time[profile_indx]);
        IDBG_MED("profile_indx=%d, Frame[%d] start time %u",
          profile_indx, p_buf_divert->buffer.sequence,
          GET_TIME_IN_MILLIS(p_client->fd_profile.start_time[profile_indx]));
      }

      IDBG_MED("%s:%d] FD_DBG MCT_EVENT_MODULE_BUF_DIVERT %x %x, div %p %d, "
        "frame %d", __func__, __LINE__, event->identity,
        p_client->fp_stream[s_idx].identity, p_buf_divert,
        p_buf_divert->native_buf, p_buf_divert->buffer.sequence);

      if (!p_buf_divert->native_buf &&
        !p_client->fp_stream[s_idx].buff_configured) {
        IDBG_ERROR("%s %d] Buffers not configured for stream %d, Cannot start"
          "face detection on frame %d", __func__, __LINE__, s_idx,
          p_buf_divert->buffer.sequence);
        p_buf_divert->is_locked = FALSE;
        p_buf_divert->ack_flag = TRUE;
        break;
      }
      IDBG_MED("%s MCT_EVENT_MODULE_BUF_DIVERT Frameid %d", __func__,
        p_buf_divert->buffer.sequence);
      if (TRUE == p_client->fd_async) {
        msg.port = msg.data.buf_divert.port = port;
        msg.type = MOD_IMG_MSG_DIVERT_BUF;
        msg.data.buf_divert.buf_divert = *p_buf_divert;
        msg.data.buf_divert.identity = event->identity;
        msg.data.buf_divert.p_exec = module_faceproc_client_buf_divert_exec;
        msg.data.buf_divert.userdata = (void *)p_client;
        module_imglib_send_msg(&p_mod->msg_thread, &msg);

        /* indicate that the buffer is not consumed */
        p_buf_divert->is_locked = FALSE;
        p_buf_divert->ack_flag = FALSE;
      } else {
        boolean ack;
        mod_img_msg_buf_divert_t msg_divert;
        memset(&msg_divert, 0x00, sizeof(msg_divert));

        msg_divert.port = port;
        msg_divert.buf_divert = *p_buf_divert;
        msg_divert.identity = event->identity;

        ack = module_faceproc_client_handle_buf_divert(p_client, &msg_divert);
        if (ack) {
          p_buf_divert->ack_flag = TRUE;
          p_buf_divert->is_buf_dirty = FALSE;
          p_buf_divert->buffer_access = msg_divert.buf_divert.buffer_access;
        } else {
          p_buf_divert->ack_flag = FALSE;
          p_buf_divert->is_buf_dirty = FALSE;
        }
        p_buf_divert->is_locked = FALSE;
      }

     if (p_client->debug_settings.profile_fd_level >= FD_PROFILE_LEVEL_STREAM) {
        if (p_buf_divert->ack_flag == TRUE) {
          uint32_t delta =
           IMG_TIMER_END(p_client->fd_profile.start_time[profile_indx],
            p_client->fd_profile.end_time[profile_indx],
            "FD_FrameSkip", IMG_TIMER_MODE_MS);
          p_client->fd_profile.total_time_in_ms[FD_FRAME_PROCESSING_TIME] +=
            delta;
          IDBG_MED("profile_indx=%d, Frame[%d] skipped time %u",
            profile_indx, p_buf_divert->buffer.sequence, delta);
        }
      }

      break;
    }
    case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
      sensor_out_info_t *sensor_info;
      sensor_info = (sensor_out_info_t *)p_mod_event->module_event_data;

      IDBG_MED("MCT_EVENT_MODULE_SET_STREAM_CONFIG, w = %u, h = %u, "
        "sensor_mount_angle=%d",
        sensor_info->dim_output.width, sensor_info->dim_output.height,
        sensor_info->sensor_mount_angle);
      p_client->main_dim.width = (int32_t)sensor_info->dim_output.width;
      p_client->main_dim.height = (int32_t)sensor_info->dim_output.height;
      p_client->sensor_mount_angle = (int32_t)sensor_info->sensor_mount_angle;
      break;
    }
    case MCT_EVENT_MODULE_STREAM_CROP: {
      mct_bus_msg_stream_crop_t *s_crop =
          (mct_bus_msg_stream_crop_t *)p_mod_event->module_event_data;

      IDBG_MED("%s:%d, MCT_EVENT_MODULE_STREAM_CROP %x", __func__,
        __LINE__, event->identity);

      rc = module_faceproc_client_set_scale_ratio(p_client, s_crop, s_idx);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("%s:%d] Can not set scale ratio ", __func__, __LINE__);
      }
      break;
    }
    case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
      mct_stream_info_t *stream_info =
        (mct_stream_info_t *)(event->u.module_event.module_event_data);
      if(!stream_info) {
        IDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return FALSE;
      }
      IDBG_HIGH("%s:%d] MCT_EVENT_MODULE_ISP_OUTPUT_DIM stream info %dx%d "
        "identity %x %x",
        __func__, __LINE__,
        stream_info->dim.width, stream_info->dim.height,
        event->identity, p_client->fp_stream[0].identity);

      /* map the buffers */
      rc = module_faceproc_client_configure_buffers(p_client, stream_info, s_idx);
      if (IMG_SUCCEEDED(rc)) {
        module_faceproc_client_set_default_scale_ratio(p_client,
          stream_info, s_idx);
      } else {
        IDBG_ERROR("%s:%d] Fail to configure buffers", __func__, __LINE__);
      }
    }
      break;

    case MCT_EVENT_MODULE_GRAVITY_VECTOR_UPDATE: {
      uint8_t i;
      int32_t orientation;
      mct_event_gravity_vector_update_t *gravity_update =
        (mct_event_gravity_vector_update_t *)
        (event->u.module_event.module_event_data);
      p_client->gravity_info.accuracy = gravity_update->accuracy;
      for (i = 0; i < 3; i++){
        p_client->gravity_info.gravity[i] = gravity_update->gravity[i];
        p_client->gravity_info.lin_accel[i] = gravity_update->lin_accel[i];
      }
      rc = img_common_align_gravity_to_camera(
         &p_client->gravity_info.gravity[0],
         p_client->sensor_mount_angle, (int) p_mod->session_data.position);

       if (IMG_SUCCEEDED(rc)) {
          rc = img_common_get_orientation_angle(
            &p_client->gravity_info.gravity[0], &orientation);
          if (rc == IMG_SUCCESS) {
            module_faceproc_client_apply_flip_on_orientation(
              p_client->flip_mask, &orientation);
            module_faceproc_client_set_rotation(p_client, orientation);
          }
        }
    }
    break;
    case MCT_EVENT_MODULE_QUERY_DIVERT_TYPE: {
     if (p_client->fp_stream[s_idx].processing_stream) {
       uint32_t *divert_mask = event->u.module_event.module_event_data;
       if (!divert_mask) {
         rc = IMG_ERR_INVALID_INPUT;
        } else {
         *divert_mask |= PPROC_DIVERT_UNPROCESSED;
        }
     }
     break;
    }
    case MCT_EVENT_MODULE_IFACE_REQUEST_PP_DIVERT:
      if (p_cur_fstream->processing_stream) {
        pp_buf_divert_request_t *divert_request =
          (pp_buf_divert_request_t *)(event->u.module_event.module_event_data);
        divert_request->need_divert = TRUE;
        divert_request->min_stride = p_mod->fd_preview_size.min_stride;
        divert_request->min_scanline = p_mod->fd_preview_size.min_scanline;

        if (p_mod->engine == FACEPROC_ENGINE_HW) {
          divert_request->num_additional_buffers +=
            FD_NUMBER_OF_ADDITIONAL_BUFFERS;
        }

        IDBG_MED("MCT_EVENT_MODULE_IFACE_REQUEST_PP_DIVERT"
          " identity=%x, stream type %d engine %d, "
          "num_additional_buffers=%d",
          p_cur_fstream->stream_info->identity,
          p_cur_fstream->stream_info->stream_type,
          p_mod->engine,
          divert_request->num_additional_buffers);
      }
      break;
    case MCT_EVENT_MODULE_QUERY_LEDFD_FD_CAPABILITY: {
      faceproc_capbility_aec_t *fd_capability =
        (faceproc_capbility_aec_t *)(event->u.module_event.module_event_data);
      rc = module_faceproc_client_fill_fd_caps(p_client, fd_capability);
      break;
    }
    case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
      stats_update_t *stats_update =
        (stats_update_t *)(event->u.module_event.module_event_data);
      if (stats_update) {

        aec_led_est_state_t state = stats_update->aec_update.est_state;
        if (state == AEC_EST_START) {
          p_client->turbo_mode = TRUE;
          IDBG_MED("Changing to turbo mode");
        } else if ((state == AEC_EST_DONE) || (state == AEC_EST_OFF) ||
            (state == AEC_EST_NO_LED_DONE) || (state ==AEC_EST_DONE_SKIP)) {
          p_client->turbo_mode = FALSE;
          IDBG_MED("Changing to normal mode");
        }

        rc = module_faceproc_client_set_lux_idx(p_client,
          stats_update->aec_update.lux_idx);
        rc = module_faceproc_client_set_offset_onlux(p_client,
          stats_update->aec_update.lux_idx);
      }
      break;
    }
    case MCT_EVENT_MODULE_STATS_AF_UPDATE: {
      stats_update_t *stats_update =
        (stats_update_t *)(event->u.module_event.module_event_data);
     IDBG_MED("MCT_EVENT_MODULE_STATS_AF_UPDATE af_status %d",
       stats_update->af_update.af_status);
      break;
    }
    case MCT_EVENT_MODULE_IFACE_REQUEST_META_STREAM_INFO: {
      uint32_t i;
      pp_native_buf_info_t *p_nbuf_info;
      pp_meta_stream_info_request_t *p_mstream_req =
        (pp_meta_stream_info_request_t *)
        (event->u.module_event.module_event_data);
      IDBG_MED("MCT_EVENT_MODULE_IFACE_REQUEST_META_STREAM_INFO num %d",
        p_mstream_req->num_streams);

      for (i = 0; i < p_mstream_req->num_streams; i++) {
        p_nbuf_info = &p_mstream_req->buf_info[i];
        if (CAM_STREAM_TYPE_ANALYSIS == p_nbuf_info->stream_type) {
          break; /* only needed for analysis stream now. break here */
       }
      }

      if (i < p_mstream_req->num_streams) {
        if (FACEPROC_ENGINE_DSP == p_mod->engine) {
          p_nbuf_info->need_adsp_heap = TRUE;
        }
        if (FACEPROC_ENGINE_HW == p_mod->engine) {
          p_nbuf_info->num_additional_buffers +=
            FD_NUMBER_OF_ADDITIONAL_BUFFERS;
        } else {
          if (p_client && p_client->p_fd_chromatix &&
            p_client->p_fd_chromatix->input_pending_buf) {
              p_nbuf_info->num_additional_buffers +=
                FD_NUMBER_OF_ADDITIONAL_BUFFERS;
          }
        }
      }
      break;
    }
    default:
      break;
    }
    break;
  }
  default:
    /* forward the event */
    break;
  }
  return GET_STATUS(rc);
}

/**
 * Function: module_faceproc_port_ext_link
 *
 * Description: This method is called when the user establishes
 *              link.
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_faceproc_port_ext_link(unsigned int identity,
  mct_port_t* port, mct_port_t *peer)
{
  IMG_UNUSED(identity);

  mct_module_t *p_mct_mod = NULL;
  module_faceproc_t *p_mod = NULL;
  faceproc_client_t *p_client = NULL;

  if (!port || !peer) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d] port %p E", __func__, __LINE__, port);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (faceproc_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] invalid client", __func__, __LINE__);
    return FALSE;
  }

  if (MCT_PORT_PEER(port) && (MCT_PORT_PEER(port) != peer)) {
    IDBG_ERROR("%s:%d] link already established", __func__, __LINE__);
    return FALSE;
  }

  MCT_PORT_PEER(port) = peer;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {
    /* start faceproc client in case of dynamic module */
  } else {
    /* do nothing for source port */
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return TRUE;
}

/**
 * Function: module_faceproc_port_unlink
 *
 * Description: This method is called when the user disconnects
 *              the link.
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_faceproc_port_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  IMG_UNUSED(identity);

  mct_module_t *p_mct_mod = NULL;
  module_faceproc_t *p_mod = NULL;
  faceproc_client_t *p_client = NULL;

  if (!port || !peer) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return;
  }

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return;
  }

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return;
  }

  p_client = (faceproc_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] faceproc client NULL", __func__, __LINE__);
    return;
  }

  if (MCT_PORT_IS_SINK(port)) {
    /* stop the client in case of dynamic module */
  } else {
    /* do nothing for source port*/
  }

  if (1 == MCT_OBJECT_REFCOUNT(port)) {
    MCT_PORT_PEER(port) = NULL;
  }

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return;
}

/**
 * Function: module_faceproc_port_set_caps
 *
 * Description: This method is used to set the capabilities
 *
 * Arguments:
 *   @port: mct port pointer
 *   @caps: mct port capabilities
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_faceproc_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  IMG_UNUSED(port);
  IMG_UNUSED(caps);

  return TRUE;
}

/**
 * Function: module_faceproc_port_check_caps_reserve
 *
 * Description: This function is used to reserve the port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @peer_caps: pointer to peer capabilities
 *   @stream_info: stream information
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_faceproc_port_check_caps_reserve(mct_port_t *port,
  void *peer_caps, void *vstream_info)
{
  boolean rc = TRUE;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)vstream_info;
  mct_module_t *p_mct_mod = NULL;
  module_faceproc_t *p_mod = NULL;
  mct_port_caps_t *p_peer_caps = (mct_port_caps_t *)peer_caps;
  mct_port_caps_t *p_caps = (mct_port_caps_t *)&port->caps;
  faceproc_client_t *p_client = NULL;
  mct_list_t *p_temp_list = NULL;

  IDBG_MED("%s:%d] id %x  stream_type %d E", __func__, __LINE__,
    stream_info->identity, stream_info->stream_type);
  if (!port || !stream_info || !p_peer_caps) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  if (p_peer_caps->port_caps_type != MCT_PORT_CAPS_FRAME) {
    IDBG_ERROR("%s:%d] invalid capabilitied, cannot connect port %x",
      __func__, __LINE__, p_peer_caps->port_caps_type);
    return FALSE;
  }

  IDBG_MED("%s:%d] caps type %d format %d", __func__, __LINE__,
    p_caps->port_caps_type,
    p_caps->u.frame.format_flag);
  if (!(p_caps->port_caps_type == MCT_PORT_CAPS_FRAME) &&
    (p_caps->u.frame.format_flag == MCT_PORT_CAP_FORMAT_YCBCR)) {
    IDBG_ERROR("%s:%d] port caps not matching", __func__, __LINE__);
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);
  if (port->port_private) {
    /* port is already reserved */
    p_client = (faceproc_client_t *)port->port_private;
    IDBG_MED("%s:%d] port is reserved %x %x", __func__, __LINE__,
      p_client->fp_stream[0].identity, stream_info->identity);
    if ((IMGLIB_SESSIONID(p_client->fp_stream[0].identity) ==
      IMGLIB_SESSIONID(stream_info->identity)) &&
      (p_client->ref_count < MAX_FD_STREAM)) {
      rc = TRUE;
    } else {
      IDBG_ERROR("%s:%d] Cannot reserve port", __func__, __LINE__);
      rc = FALSE;
    }
  } else {
    /* acquire port only for new sessions */
    p_temp_list = mct_list_find_custom(p_mod->fp_client,
      &stream_info->identity,
      module_faceproc_find_client_by_session);
    IDBG_MED("%s:%d] client entry %p", __func__, __LINE__,
      p_temp_list);
    if (NULL != p_temp_list) {
      port->port_private = p_client = p_temp_list->data;
      IDBG_MED("%s:%d] client ptr %p", __func__, __LINE__,
        p_client);
    } else {
      rc = module_faceproc_acquire_port(p_mct_mod, port, stream_info);
      if (FALSE == rc) {
        IDBG_ERROR("%s:%d] Error acquiring port", __func__, __LINE__);
        goto error;
      }
    }

    p_client = (faceproc_client_t *)port->port_private;
  }

  if (TRUE == rc) {
    int ret = module_faceproc_client_add_stream(p_client, stream_info, port);
    if (IMG_ERROR(ret)) {
      IDBG_ERROR("%s:%d] Error add stream to client", __func__, __LINE__);
      goto error;
    }
  }

  MCT_OBJECT_REFCOUNT(port)++;
  /* add stream type to the mask */
  if (MCT_PORT_IS_SINK(port)) {
    if (stream_info->stream_type != CAM_STREAM_TYPE_PARM) {
      if (p_client->streamid_mask == 0) {
        module_faceproc_client_comp_create(p_client);
      }
      p_client->streamid_mask |= (1 << IMGLIB_STREAMID(stream_info->identity));
    }
  }

  IDBG_HIGH("%s:%d] Port %s for stream_type %d, identity 0x%x ref_cnt %d"
    " mask %x",
    __func__, __LINE__, MCT_PORT_NAME(port), stream_info->stream_type,
    stream_info->identity,
    MCT_OBJECT_REFCOUNT(port),
    p_client->streamid_mask);
  pthread_mutex_unlock(&p_mod->mutex);
  IDBG_MED("%s:%d] ref_cnt %d %d mask %x X", __func__, __LINE__,
    MCT_OBJECT_REFCOUNT(port),
    p_client->ref_count,
    p_client->streamid_mask);
  return TRUE;

error:
  pthread_mutex_unlock(&p_mod->mutex);
  IDBG_ERROR("%s:%d] Error X", __func__, __LINE__);
  return FALSE;
}

/**
 * Function: module_faceproc_port_check_caps_unreserve
 *
 * Description: This method is used to unreserve the port
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_faceproc_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  int rc = IMG_SUCCESS;
  mct_module_t *p_mct_mod = NULL;
  module_faceproc_t *p_mod = NULL;
  faceproc_client_t *p_client = NULL;

  IDBG_MED("%s:%d] E id %x", __func__, __LINE__, identity);
  if (!port) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (faceproc_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] faceproc client NULL", __func__, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);

  if (MCT_PORT_IS_SINK(port)) {
    IDBG_MED("%s:%d] ref_cnt %d", __func__, __LINE__, p_client->ref_count);

    if (p_client->ref_count <= 1) {
      /* First remove client from module list */
      p_mod->fp_client = mct_list_remove(p_mod->fp_client,
        p_client);
      /* Update module client count */
      if (p_mod->client_cnt) {
        p_mod->client_cnt--;
        module_faceproc_update_multi_client(p_mod);
      } else {
        IDBG_ERROR("%s:%d] Invalid client count", __func__, __LINE__);
      }
      IDBG_MED("%s:%d] Module client count %d", __func__, __LINE__,
        p_mod->client_cnt);
      /* destroy the client */
      port->port_private = NULL;
      module_faceproc_client_destroy(p_client, identity);
      p_client = NULL;

      /*Todo: free port??*/
    } else {
      pthread_mutex_lock(&p_client->mutex);
      /* remove stream type from the mask */
      int s_idx = faceproc_get_stream_by_id(p_client, identity);
      if (s_idx < 0) {
        pthread_mutex_unlock(&p_client->mutex);
        IDBG_ERROR("%s:%d] Invalid index for identity %x",
          __func__, __LINE__, identity);
        goto error;
      }

      faceproc_stream_t *p_fp_stream = &p_client->fp_stream[s_idx];

      if (p_fp_stream->stream_info) {
        cam_stream_type_t stream_type = p_fp_stream->stream_info->stream_type;
        if (stream_type != CAM_STREAM_TYPE_PARM) {
          int stream_id = IMGLIB_STREAMID(p_fp_stream->stream_info->identity);
          p_client->streamid_mask &= ~(1 << stream_id);
          if (p_client->streamid_mask == 0) {
            /* only session stream in present */
            module_faceproc_client_comp_destroy(p_client);
          }
        }
      }
      pthread_mutex_unlock(&p_client->mutex);

      /* Remove stream from client  */
      rc = module_faceproc_client_remove_stream(p_client, identity);
      if (IMG_ERROR(rc))  {
        IDBG_ERROR("%s:%d] Can not remove stream", __func__, __LINE__);
        goto error;
      }
    }
  }

  if (--(MCT_OBJECT_REFCOUNT(port)) <= 0) {
    port->port_private = NULL;
  }
  IDBG_HIGH("%s:%d] Port %s for identity 0x%x ref_cnt %d mask %x",
    __func__, __LINE__, MCT_PORT_NAME(port), identity,
    MCT_OBJECT_REFCOUNT(port),
    p_client ? p_client->streamid_mask : 0xffffffff);
  pthread_mutex_unlock(&p_mod->mutex);

  /* for debugging */
  if (p_client) {
    IDBG_MED("%s:%d] ref_cnt %d %d mask %x X", __func__, __LINE__,
      MCT_OBJECT_REFCOUNT(port),
      p_client->ref_count,
      p_client->streamid_mask);
  } else {
    IDBG_MED("%s:%d] ref_cnt %d X", __func__, __LINE__,
      MCT_OBJECT_REFCOUNT(port));
  }
  return GET_STATUS(rc);

error:
  pthread_mutex_unlock(&p_mod->mutex);
  IDBG_ERROR("%s:%d] Error rc = %d X", __func__, __LINE__, rc);
  return FALSE;
}

/**
 * Function: module_faceproc_simulate_port_streamon
 *
 * Description: Currently Stream ON is passed to
 *  module directly, our module is handling this functionality
 *  on his ports, use this to redirect from module to port functions
 *
 * Arguments:
 *   @stream: mct_stream_t
 *   @module mct_module_t
 *   @port: mct_port_t
 *   @event: mct_event_t
 *
 * Return values:
 *     error/success
 *
 * Notes: Remove this function when Mct will use port functions
 *  for Source module
 **/
static boolean module_faceproc_simulate_port_streamon(mct_stream_t *stream,
  mct_module_t *module, mct_port_t *port, mct_event_t *event)
{
  boolean port_sucess;

  if (!(stream && module && port && port->check_caps_reserve))
    return FALSE;
  if (!(port->check_caps_unreserve && port->ext_link && port->un_link))
    return FALSE;

  port_sucess = port->check_caps_reserve(port, &port->caps,
    &stream->streaminfo);
  if (FALSE == port_sucess)
    return port_sucess;

  port_sucess = port->ext_link(stream->streaminfo.identity, port, port);
  if (FALSE == port_sucess) {
    port->check_caps_unreserve(port, stream->streaminfo.identity);
    return port_sucess;
  }

  port->event_func(port, event);
  if (FALSE == port_sucess) {
    port->un_link(stream->streaminfo.identity, port, port);
    port->check_caps_unreserve(port, stream->streaminfo.identity);
  }
  return port_sucess;
}

/**
 * Function: module_faceproc_simulate_port_streamoff
 *
 * Description: Currently Stream ON is passed to
 *  module directly, our module is handling this functionality
 *  on his ports, use this to redirect from module to port functions
 *
 * Arguments:
 *   @stream: mct_stream_t
 *   @module mct_module_t
 *   @port: mct_port_t
 *   @event: mct_event_t
 *
 * Return values:
 *     error/success
 *
 * Notes: Remove this function when Mct will use port functions
 *  for Source module
 **/
static boolean module_faceproc_simulate_port_streamoff(mct_stream_t *stream,
  mct_module_t *module, mct_port_t *port, mct_event_t *event)
{
  if (!(stream && module && port))
    return FALSE;
  if (!(port->check_caps_unreserve && port->event_func && port->un_link))
    return FALSE;

  port->event_func(port, event);
  port->un_link(stream->streaminfo.identity, port, port);
  port->check_caps_unreserve(port, stream->streaminfo.identity);

  return TRUE;
}

/**
 * Function: module_faceproc_process_event
 *
 * Description: Event handler function for the faceproc module
 *
 * Arguments:
 *   @streamid: stream id
 *   @p_mct_mod: mct module pointer
 *   @p_event: mct event
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_faceproc_process_event(mct_module_t *module,
  mct_event_t *event)
{
  mct_stream_t *stream;
  mct_port_t *port;
  module_faceproc_t *p_mod;
  int rc = IMG_SUCCESS;

  if (!(module && module->module_private) || !event) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  stream = MCT_STREAM_CAST((MCT_MODULE_PARENT(module))->data);
  if (!stream) {
    IDBG_MED("Error get STREAM EVENT ID ");
    goto out;
  }

  port = MCT_PORT_CAST((MCT_MODULE_CHILDREN(module))->data);
  if (!port) {
    IDBG_MED("Error get PORT ID ");
    goto out;
  }

  p_mod = (module_faceproc_t *)module->module_private;

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    mct_event_control_t *p_ctrl_event = &event->u.ctrl_event;
    IDBG_MED("%s:%d] Ctrl type %d", __func__, __LINE__, p_ctrl_event->type);
    switch (p_ctrl_event->type) {
    case MCT_EVENT_CONTROL_STREAMON: {
      if (p_mod->module_type == MCT_MODULE_FLAG_SOURCE)
          module_faceproc_simulate_port_streamon(stream, module, port ,event);
      break;
    }
    case MCT_EVENT_CONTROL_STREAMOFF:
      if (p_mod->module_type  == MCT_MODULE_FLAG_SOURCE)
          module_faceproc_simulate_port_streamoff(stream, module, port, event);
      break;
    case MCT_EVENT_CONTROL_PARM_STREAM_BUF:
    case MCT_EVENT_CONTROL_SET_PARM:
      if (port->event_func)
        port->event_func(port, event);
      break;
    default:
      break;

    }
    break;
  }
  case MCT_EVENT_MODULE_EVENT:
  default:
    /* forward the event */
    break;
  }

out:
  return GET_STATUS(rc);
}

/**
 * Function: module_faceproc_request_new_port
 *
 * Description: This function is called by the mct framework
 *         when new port needs to be created
 *
 * Arguments:
 *   @stream_info: stream information
 *   @direction: direction of port
 *   @module: mct module pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
mct_port_t *module_faceproc_request_new_port(void *vstream_info,
  mct_port_direction_t direction,
  mct_module_t *module,
  void *peer_caps)
{
  mct_stream_info_t *stream_info = (mct_stream_info_t *)vstream_info;
  boolean rc = IMG_SUCCESS;
  module_faceproc_t *p_mod = NULL;
  mct_port_t *p_port = NULL;
  mct_port_caps_t *p_peer_caps = (mct_port_caps_t *)peer_caps;
  faceproc_client_t *p_client = NULL;
  mct_list_t *p_temp_list = NULL;
  int ret;

  if (!module || !stream_info) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return NULL;
  }

  if (p_peer_caps) {
    if(p_peer_caps->port_caps_type != MCT_PORT_CAPS_FRAME) {
      IDBG_ERROR("%s:%d] invalid capabilities, cannot connect port %x",
      __func__, __LINE__, p_peer_caps->port_caps_type);
      return NULL;
    }
    IDBG_MED("%s:%d] caps type %d format %d", __func__, __LINE__,
      p_peer_caps->port_caps_type,
      p_peer_caps->u.frame.format_flag);
  }

  p_mod = (module_faceproc_t *)module->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return NULL;
  }

  pthread_mutex_lock(&p_mod->mutex);
  p_port = module_faceproc_create_port(module, direction);
  if (NULL == p_port) {
    IDBG_ERROR("%s:%d] Error creating port", __func__, __LINE__);
    goto error;
  }

  /*acquire port only for new sessions */
  p_temp_list = mct_list_find_custom(p_mod->fp_client,
    &stream_info->identity,
    module_faceproc_find_client_by_session);
  IDBG_MED("%s:%d] client entry %p", __func__, __LINE__,
    p_temp_list);
  if (NULL != p_temp_list) {
    p_client = p_temp_list->data;
    p_port->port_private = p_client;
  } else {
    rc = module_faceproc_acquire_port(module, p_port, stream_info);
    if (FALSE == rc) {
      IDBG_ERROR("%s:%d] Error acquiring port", __func__, __LINE__);
      goto error;
    }
  }

  p_client = (faceproc_client_t *)p_port->port_private;
  ret = module_faceproc_client_add_stream(p_client, stream_info, p_port);
  if (IMG_ERROR(ret)) {
    IDBG_ERROR("%s:%d] Error add stream to client", __func__, __LINE__);
    goto error;
  }

  /* add stream type to the mask */
  if (MCT_PORT_IS_SINK(p_port)) {
    if (stream_info->stream_type != CAM_STREAM_TYPE_PARM) {
      if (p_client->streamid_mask == 0) {
        module_faceproc_client_comp_create(p_client);
      }
      p_client->streamid_mask |= (1 << IMGLIB_STREAMID(stream_info->identity));
    }
  }

  MCT_OBJECT_REFCOUNT(p_port)++;
  IDBG_MED("%s:%d] ref_cnt %d %d X", __func__, __LINE__,
    MCT_OBJECT_REFCOUNT(p_port),
    p_client->ref_count);
  pthread_mutex_unlock(&p_mod->mutex);
  return p_port;

error:
  IDBG_ERROR("%s:%d] error %d X", __func__, __LINE__,
    rc);
  pthread_mutex_unlock(&p_mod->mutex);
  return NULL;
}

/**
 * Function: module_faceproc_start_session
 *
 * Description: This function is called when a new camera
 *              session is started
 *
 * Arguments:
 *   @module: mct module pointer
 *   @sessionid: session id
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_faceproc_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_faceproc_t *p_mod;

  if (!module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_faceproc_t *)module->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  /* Add session settings */
  module_faceproc_create_session_param(p_mod, sessionid);

  /* create message thread */
  pthread_mutex_lock(&p_mod->mutex);
  if (!p_mod->session_cnt)
    rc = module_imglib_create_msg_thread(&p_mod->msg_thread);
  p_mod->session_cnt++;
  pthread_mutex_unlock(&p_mod->mutex);
  IDBG_HIGH("%s:%d] session cnt %d", __func__, __LINE__,
    p_mod->session_cnt);

  return GET_STATUS(rc);
}

/**
 * Function: module_faceproc_start_session
 *
 * Description: This function is called when the camera
 *              session is stopped
 *
 * Arguments:
 *   @module: mct module pointer
 *   @sessionid: session id
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_faceproc_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_faceproc_t *p_mod;

  if (!module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_faceproc_t *)module->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  pthread_mutex_lock(&p_mod->mutex);
  /* Check if this need to be here */
  module_faceproc_destroy_session_param(p_mod, sessionid);

  IDBG_HIGH("%s:%d] session cnt %d", __func__, __LINE__,
    p_mod->session_cnt);
  /* destroy message thread */
  p_mod->session_cnt--;
  if (!p_mod->session_cnt)
    rc = module_imglib_destroy_msg_thread(&p_mod->msg_thread);
  pthread_mutex_unlock(&p_mod->mutex);
  return GET_STATUS(rc);
}

/**
 * Function: module_faceproc_set_mod
 *
 * Description: This function is used to set the faceproc module
 *
 * Arguments:
 *   @module: mct module pointer
 *   @module_type: module type
 *   @identity: id of the stream
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void module_faceproc_set_mod(mct_module_t *module,
  unsigned int module_type,
  unsigned int identity)
{
  module_faceproc_t *p_mod;

  if (!(module && module->module_private)) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return;
  }

  /* Do not touch module mode since currently is only one instance module */
  p_mod = (module_faceproc_t *)module->module_private;
  p_mod->module_type = module_type;
  mct_module_add_type(module, module_type, identity);

  return;
}

/**
 * Function: module_faceproc_query_mod
 *
 * Description: This function is used to query the faceproc module info
 *
 * Arguments:
 *   @module: mct module pointer
 *   @query_buf: pipeline capability
 *   @sessionid: session identity
 *
 * Return values:
 *     success/failure
 *
 * Notes: none
 **/
static boolean module_faceproc_query_mod(mct_module_t *module, void *query_buf,
  unsigned int sessionid)
{
  mct_pipeline_cap_t *p_mct_cap = (mct_pipeline_cap_t *)query_buf;
  mct_pipeline_imaging_cap_t *p_cap = NULL;
  mct_pipeline_sensor_cap_t *p_sensor_cap = NULL;
  module_faceproc_t *p_mod;
  int32_t ind;
  cam_analysis_info_t *p_analysis_info;
  mct_list_t *p_temp_list = NULL;
  faceproc_client_t *p_client = NULL;
  uint32_t packed_session_id = IMGLIB_PACK_IDENTITY(sessionid, 0);

  if (!query_buf || !module || !module->module_private) {
    IDBG_ERROR("NULL pointer");
    return FALSE;
  }
  p_mod = (module_faceproc_t *)module->module_private;

  p_cap = &p_mct_cap->imaging_cap;
  p_sensor_cap = &p_mct_cap->sensor_cap;

  // Disable FD capabilities for Mono camera
  if (p_sensor_cap->color_arrangement == CAM_FILTER_ARRANGEMENT_Y) {
    p_cap->analysis_info[CAM_ANALYSIS_INFO_FD_STILL].valid = false;
    p_cap->analysis_info[CAM_ANALYSIS_INFO_FD_VIDEO].valid = false;
    return TRUE;
  }

  p_cap->max_num_roi = MAX_FACES_TO_DETECT;
  p_cap->feature_mask |= CAM_QCOM_FEATURE_FACE_DETECTION |
    CAM_QCOM_FEATURE_REGISTER_FACE;

  p_temp_list = mct_list_find_custom(p_mod->fp_client,
    &packed_session_id, module_faceproc_find_client_by_session);
  if (p_temp_list != NULL) {
    p_client = p_temp_list->data;
    p_cap->face_bsgc = p_client->enable_bsgc;
  }

  // Fill Analysis information for STILL (camera preview) mode
  p_analysis_info = &p_cap->analysis_info[CAM_ANALYSIS_INFO_FD_STILL];

  p_analysis_info->analysis_max_res.width = p_mod->fd_preview_size.max_width;
  p_analysis_info->analysis_max_res.height =  p_mod->fd_preview_size.max_height;

  p_analysis_info->analysis_padding_info.width_padding = CAM_PAD_TO_16;
  p_analysis_info->analysis_padding_info.height_padding = CAM_PAD_TO_8;
  p_analysis_info->analysis_padding_info.plane_padding = CAM_PAD_TO_16;
  p_analysis_info->analysis_padding_info.min_stride =
    p_mod->fd_preview_size.min_stride;
  p_analysis_info->analysis_padding_info.min_scanline =
    p_mod->fd_preview_size.min_scanline;
  p_analysis_info->analysis_padding_info.offset_info.offset_x = 0;
  p_analysis_info->analysis_padding_info.offset_info.offset_y = 0;

  p_analysis_info->analysis_recommended_res.width =
    RECOMMENDED_ANALYSIS_WIDTH;
  p_analysis_info->analysis_recommended_res.height =
    RECOMMENDED_ANALYSIS_HEIGHT;

  p_analysis_info->analysis_format = CAM_FORMAT_Y_ONLY;

  if (p_mod->engine == FACEPROC_ENGINE_HW) {
    p_analysis_info->hw_analysis_supported = TRUE;
  } else {
    p_analysis_info->hw_analysis_supported = FALSE;
  }
  p_analysis_info->valid = TRUE;

  /*Updating min stride, scanline & buf_alignment in the common cap */
  p_mct_cap->common_cap.min_stride =
    mct_util_calculate_lcm( p_mct_cap->common_cap.min_stride,
    p_analysis_info->analysis_padding_info.min_stride);
  p_mct_cap->common_cap.min_scanline =
    mct_util_calculate_lcm( p_mct_cap->common_cap.min_scanline,
    p_analysis_info->analysis_padding_info.min_scanline);
  p_mct_cap->common_cap.buf_alignment =
    mct_util_calculate_lcm( p_mct_cap->common_cap.buf_alignment,
    0);

  // Fill Analysis information for Video mode
  p_analysis_info = &p_cap->analysis_info[CAM_ANALYSIS_INFO_FD_VIDEO];
  *p_analysis_info = p_cap->analysis_info[CAM_ANALYSIS_INFO_FD_STILL];

  // Overwrite the values which we want to be different for Video
  p_analysis_info->analysis_max_res.width = p_mod->fd_video_size.max_width;
  p_analysis_info->analysis_max_res.height =  p_mod->fd_video_size.max_height;

  p_analysis_info->analysis_padding_info.min_stride =
    p_mod->fd_video_size.min_stride;
  p_analysis_info->analysis_padding_info.min_scanline =
    p_mod->fd_video_size.min_scanline;

  p_analysis_info->valid = TRUE;

  if ((p_cap->supported_scene_modes_cnt + 1) >= CAM_SCENE_MODE_MAX) {
    IDBG_ERROR("Failed cannot add scenemode");
    return FALSE;
  }
  ind = p_cap->supported_scene_modes_cnt;
  p_cap->supported_scene_modes[ind] = CAM_SCENE_MODE_FACE_PRIORITY;
  p_cap->scene_mode_overrides[ind].ae_mode  = CAM_AE_MODE_ON;
  p_cap->scene_mode_overrides[ind].awb_mode  = CAM_WB_MODE_AUTO;
  p_cap->scene_mode_overrides[ind].af_mode  = CAM_FOCUS_MODE_AUTO;
  p_cap->supported_scene_modes_cnt++;

  IDBG_HIGH("Caps ROI %d scene_mode_cnt %d HW analysis supported %d",
    p_cap->max_num_roi,
    p_cap->supported_scene_modes_cnt,
    p_analysis_info->hw_analysis_supported);

  return TRUE;
}

/**
 * Function: module_faceproc_free_port
 *
 * Description: This function is used to free the faceproc ports
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static boolean module_faceproc_free_port(void *data, void *user_data)
{
  mct_port_t *p_port = (mct_port_t *)data;
  mct_module_t *p_mct_mod = (mct_module_t *)user_data;
  boolean rc = FALSE;

  IDBG_MED("%s:%d port %p p_mct_mod %p", __func__, __LINE__, p_port,
    p_mct_mod);
  if (!p_port || !p_mct_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return TRUE;
  }
  rc = mct_module_remove_port(p_mct_mod, p_port);
  if (rc == FALSE) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
  }
  mct_port_destroy(p_port);
  return TRUE;
}

/**
 * Function: module_faceproc_create_port
 *
 * Description: This function is used to create a port and link with the
 *              module
 *
 * Arguments:
 *   none
 *
 * Return values:
 *     MCTL port pointer
 *
 * Notes: none
 **/
mct_port_t *module_faceproc_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir)
{
  char portname[PORT_NAME_LEN];
  mct_port_t *p_port = NULL;
  int status = IMG_SUCCESS;
  int index = 0;

  if (!p_mct_mod || (MCT_PORT_UNKNOWN == dir)) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }

  index = (MCT_PORT_SINK == dir) ? p_mct_mod->numsinkports :
    p_mct_mod->numsrcports;
  /*portname <mod_name>_direction_portIndex*/
  snprintf(portname, sizeof(portname), "%s_d%d_i%d",
    MCT_MODULE_NAME(p_mct_mod), dir, index);
  p_port = mct_port_create(portname);
  if (NULL == p_port) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }
  IDBG_MED("%s:%d portname %s", __func__, __LINE__, portname);

  p_port->direction = dir;
  p_port->port_private = NULL;
  p_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;
  p_port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR;
  /*Todo: fill the size flag*/

  /* override the function pointers */
  p_port->check_caps_reserve    = module_faceproc_port_check_caps_reserve;
  p_port->check_caps_unreserve  = module_faceproc_port_check_caps_unreserve;
  p_port->ext_link              = module_faceproc_port_ext_link;
  p_port->un_link               = module_faceproc_port_unlink;
  p_port->set_caps              = module_faceproc_port_set_caps;
  p_port->event_func            = module_faceproc_port_event_func;

  /* add port to the module */
  if (!mct_module_add_port(p_mct_mod, p_port)) {
    IDBG_ERROR("%s: Set parent failed", __func__);
    status = IMG_ERR_GENERAL;
    goto error;
  }

  if (MCT_PORT_SRC == dir)
    p_mct_mod->numsrcports++;
  else
    p_mct_mod->numsinkports++;

  IDBG_MED("%s:%d ", __func__, __LINE__);
  return p_port;

error:

  IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
  if (p_port) {
    mct_port_destroy(p_port);
    p_port = NULL;
  }
  return NULL;
}

/** module_faceproc_set_session_data: set session data
 *
 *  @module: faceproc module handle
 *  @set_buf: set buffer handle that has session data
 *  @sessionid: session id for which session data shall be
 *            applied
 *
 *  This function provides session data that has per frame
 *  control parameters
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_faceproc_set_session_data(mct_module_t *module,
  void *set_buf, unsigned int sessionid)
{
  IMG_UNUSED(sessionid);

  mct_pipeline_session_data_t *frame_ctrl_data;
  module_faceproc_t *p_mod;

  /* Validate input parameters */
  if (!(module && module->module_private) || !set_buf) {
    IDBG_ERROR("%s:%d] Error: invalid params %p %p",
      __func__, __LINE__, module, set_buf);
    return FALSE;
  }

  p_mod = (module_faceproc_t *)module->module_private;

  frame_ctrl_data = (mct_pipeline_session_data_t *)set_buf;
  IDBG_MED("%s:%d] Per frame control %d %d", __func__, __LINE__,
    frame_ctrl_data->max_pipeline_frame_applying_delay,
    frame_ctrl_data->max_pipeline_meta_reporting_delay);

  p_mod->session_data = *frame_ctrl_data;

  return TRUE;
}

/** module_faceproc_get_max_size: Get component max supported size
 *
 *  @p_core_ops: Faceproc component core ops
 *  @engine: Engine type
 *  @p_fd_preview_size: Pointer to fill info for preview/camera mode.
 *  @p_fd_preview_size: Pointer to fill info for video mode.
 *
 *  This function provides max supported faceproc component size.
 *
 *  Return: img error code
 **/
static int module_faceproc_get_max_size(img_core_ops_t *p_core_ops,
  faceproc_engine_t engine,
  faceproc_frame_cfg_t *p_fd_preview_size,
  faceproc_frame_cfg_t *p_fd_video_size)
{
  img_component_ops_t p_comp;
  int rc;

  /* Validate input parameters */
  if (!p_core_ops || !p_fd_preview_size || !p_fd_video_size) {
    IDBG_ERROR("Error: invalid input %p %p %p",
      p_core_ops, p_fd_preview_size, p_fd_video_size);
    return IMG_ERR_INVALID_INPUT;
  }

  rc = IMG_COMP_CREATE(p_core_ops, &p_comp);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("create failed %d", rc);
    return rc;
  }
  IDBG_MED("create DONE");

  rc = IMG_COMP_INIT(&p_comp, NULL, NULL);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("init failed %d", rc);
    return rc;
  }
  IDBG_MED("INIT DONE ");

  /* Check component max supported resolution */
  if (engine == FACEPROC_ENGINE_HW) {
    p_fd_preview_size->max_width = RECOMMENDED_ANALYSIS_WIDTH;
    p_fd_preview_size->max_height = RECOMMENDED_ANALYSIS_HEIGHT;
  } else  {
    p_fd_preview_size->max_width = MAX_FD_WIDTH;
    p_fd_preview_size->max_height = MAX_FD_HEIGHT;
  }
  p_fd_preview_size->min_stride = 0;
  p_fd_preview_size->min_scanline = 0;

  char cwidth[PROPERTY_VALUE_MAX];
  char cheight[PROPERTY_VALUE_MAX];
  property_get("persist.vendor.camera.fdwidth", cwidth, "0");
  property_get("persist.vendor.camera.fdheight", cheight, "0");
  if (atoi(cwidth) && atoi(cheight)) {
    p_fd_preview_size->max_width = atoi(cwidth);
    p_fd_preview_size->max_height = atoi(cheight);
  }

  rc = IMG_COMP_SET_PARAM(&p_comp, QWD_FACEPROC_TRY_SIZE,
    (void *)p_fd_preview_size);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Try size failed %d", rc);
    return rc;
  }
  IDBG_MED("SET PARAM DONE");

  if (engine == FACEPROC_ENGINE_HW) {
    p_fd_video_size->max_width = RECOMMENDED_ANALYSIS_VIDEO_WIDTH;
    p_fd_video_size->max_height = RECOMMENDED_ANALYSIS_VIDEO_HEIGHT;

    if (atoi(cwidth) && atoi(cheight)) {
      p_fd_video_size->max_width = atoi(cwidth);
      p_fd_video_size->max_height = atoi(cheight);
    }

    p_fd_video_size->min_stride = 0;
    p_fd_video_size->min_scanline = 0;

    rc = IMG_COMP_SET_PARAM(&p_comp, QWD_FACEPROC_TRY_SIZE,
      (void *)p_fd_video_size);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Try size failed %d", rc);
      return rc;
    }
  } else {
    *p_fd_video_size = *p_fd_preview_size;
  }

  rc = IMG_COMP_DEINIT(&p_comp);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Deinit failed %d", rc);
    return rc;
  }
  IDBG_MED("DEINIT DONE");

  IDBG_MED("Max supported size  %dx%d min stride %d min scanline %d",
    p_fd_preview_size->max_width,
    p_fd_preview_size->max_height,
    p_fd_preview_size->min_stride,
    p_fd_preview_size->min_scanline);

  return IMG_SUCCESS;
}
/**
 * Function: module_faceproc_free_mod
 *
 * Description: This function is used to free the faceproc module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_faceproc_deinit(mct_module_t *p_mct_mod)
{
  module_faceproc_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_list_t* p_list;
  faceproc_engine_t eng_type;

  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s:%d] MCTL module NULL", __func__, __LINE__);
    return;
  }

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return;
  }

  pthread_mutex_destroy(&p_mod->mutex);
  pthread_cond_destroy(&p_mod->cond);

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SINKPORTS(p_mct_mod), p_mct_mod,
      module_imglib_get_next_from_list);
    if (p_list)
      module_faceproc_free_port(p_list->data, p_mct_mod);
  } while (p_list);

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SRCPORTS(p_mct_mod), p_mct_mod,
      module_imglib_get_next_from_list);
    if (p_list)
      module_faceproc_free_port(p_list->data, p_mct_mod);
  } while (p_list);

  IDBG_MED("%s:%d] delete the clients", __func__, __LINE__);
  /*Todo*/

  for (eng_type = FACEPROC_ENGINE_HW; eng_type < FACEPROC_ENGINE_MAX;
    eng_type++) {
    p_core_ops = &p_mod->core_ops[eng_type];
    if (!p_core_ops->handle) {
      continue;
    }
    IDBG_MED("%s:%d lib_ref_cnt %d", __func__, __LINE__, p_mod->lib_ref_count);
    if (p_mod->lib_ref_count) {
      IMG_COMP_UNLOAD(p_core_ops);
    }
  }

  img_fcv_put_lib_hdl(p_mod->p_fcv_hdl);

  close(p_mod->ion_fd);
  p_mod->client_cnt = 0;
  mct_module_destroy(p_mct_mod);
}

/** module_faceproc_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the faceproc module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_faceproc_init(const char *name)
{
  mct_module_t *p_mct_mod = NULL;
  module_faceproc_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_port_t *p_port = NULL;
  int rc = 0;
  uint32_t i = 0;
  uint32_t fd_comp_mask;
  faceproc_engine_t eng_type;
  char *fd_comp_name[] = {
    "qcom.faceproc_hw",
    "qcom.faceproc_dsp",
    "qcom.faceproc",
  };

  fd_comp_mask = module_imglib_common_get_prop(
    "persist.vendor.camera.imglib.fd_comp", "7");

  IDBG_HIGH("fd_comp_mask %x", fd_comp_mask);
  IDBG_HIGH("E");
  p_mct_mod = mct_module_create(name);
  if (NULL == p_mct_mod) {
    IDBG_ERROR("cannot allocate mct module");
    return NULL;
  }
  p_mod = malloc(sizeof(module_faceproc_t));
  if (NULL == p_mod) {
    IDBG_ERROR("failed");
    goto error;
  }

  p_mct_mod->module_private = (void *)p_mod;
  memset(p_mod, 0, sizeof(module_faceproc_t));

  pthread_mutex_init(&p_mod->mutex, NULL);
  pthread_cond_init(&p_mod->cond, NULL);

  p_mod->ion_fd = open("/dev/ion", O_RDONLY);
  p_mod->force_cache_op = module_imglib_common_get_prop(
    "persist.vendor.camera.force_cache_ops", "0");
  p_mod->engine = FACEPROC_ENGINE_MAX;
  for (eng_type = FACEPROC_ENGINE_HW; eng_type < FACEPROC_ENGINE_MAX;
    eng_type++) {
    if (0 == (fd_comp_mask & (1 << eng_type)))
      continue;
    rc = img_core_get_comp(IMG_COMP_FACE_PROC, fd_comp_name[eng_type],
      &p_mod->core_ops[eng_type]);
    if (IMG_SUCCEEDED(rc)) {
      rc = IMG_COMP_LOAD(&p_mod->core_ops[eng_type], NULL);
      IDBG_HIGH("Acquire %s %p %d",
        fd_comp_name[eng_type],
        p_mod->core_ops[eng_type].handle, rc);
      if (IMG_SUCCEEDED(rc) && (p_mod->engine == FACEPROC_ENGINE_MAX)) {
        p_mod->engine = eng_type;
      }
    } else {
      IDBG_ERROR("Cannot acquire %s %d",
        fd_comp_name[eng_type], rc);
    }
  }

  if (p_mod->engine == FACEPROC_ENGINE_MAX) {
    IDBG_ERROR("Cannot acquire any comp");
    goto error;
  }

  /* Get component max supported size */
  p_core_ops = &p_mod->core_ops[p_mod->engine];
  rc = module_faceproc_get_max_size(p_core_ops, p_mod->engine,
    &p_mod->fd_preview_size, &p_mod->fd_video_size);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Get max size failed %d", rc);
  }

  p_mod->lib_ref_count++;
  p_mod->fp_client = NULL;
  p_mod->client_cnt = 0;

  IDBG_LOW("create static ports");
  /* create static ports */
  for (i = 0; i < MAX_FD_STATIC_PORTS; i++) {
    p_port = module_faceproc_create_port(p_mct_mod, MCT_PORT_SINK);
    if (NULL == p_port) {
      IDBG_ERROR("create port failed");
      goto error;
    }
  }

  p_mct_mod->process_event    = module_faceproc_process_event;
  p_mct_mod->set_mod          = module_faceproc_set_mod;
  p_mct_mod->query_mod        = module_faceproc_query_mod;
  p_mct_mod->request_new_port = module_faceproc_request_new_port;
  p_mct_mod->start_session    = module_faceproc_start_session;
  p_mct_mod->stop_session     = module_faceproc_stop_session;
  p_mct_mod->set_session_data = module_faceproc_set_session_data;

  if (img_fcv_get_lib_hdl(&(p_mod->p_fcv_hdl)) != IMG_SUCCESS) {
    IDBG_ERROR("couldnt get fastcv handle");
    goto error;
  }

  IDBG_LOW("Exit");
  return p_mct_mod;

error:

  if (p_mod) {
    module_faceproc_deinit(p_mct_mod);
  } else if (p_mct_mod) {
    mct_module_destroy(p_mct_mod);
  }
  return NULL;
}
