/* mct_controller.c
 *
 * This file contains the media controller implementation. All commands coming
 * from the server arrive here first. There is one media controller per
 * session.
 *
 * Copyright (c) 2012-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "cam_ker_headers.h"
#include "media_controller.h"
#include "mct_controller.h"
#include "mct_profiler.h"
#include "mct_pipeline.h"
#include "mct_bus.h"
#include "cam_intf.h"
#include "camera_dbg.h"
#include "mm_camera_shim.h"
#include <cutils/properties.h>
#include <server_debug.h>
#include "supermct.h"

volatile uint32_t mct_debug_level = MCT_DEBUG_PRIO_HIGH;
volatile uint32_t mct_debug_module = MODULE_MCT;

mct_list_t *mcts = NULL;

static void* mct_controller_thread_run(void *data);
static void* mct_bus_handler_thread_run(void *data);
static boolean mct_controller_handle_SOF_proc(
  mct_pipeline_t *pipeline, mct_bus_msg_t *bus_msg,
  mct_superparam_type_t send_with_params);
static boolean mct_controller_send_DC_superparam(mct_pipeline_t *pipeline,
  mct_stream_t *parm_stream);

static boolean mct_controller_close_thread(mct_controller_t *mct)
{
  mct_serv_msg_t *msg = NULL;
  mct_bus_msg_t *bus_msg_destroy = NULL;

  /*Closing MCT Serv thread */
  msg = calloc(1, sizeof(mct_serv_msg_t));
  if (!msg) {
    CLOGE(CAM_MCT_MODULE, "Calloc failed while closing serv thread");
    return FALSE;
  }
  msg->msg_type = SERV_MSG_HAL;
  msg->u.hal_msg.id = MSM_CAMERA_DEL_SESSION;

  pthread_mutex_lock(&mct->serv_msg_q_lock);
  mct_queue_push_tail(mct->serv_cmd_q, msg);
  pthread_mutex_unlock(&mct->serv_msg_q_lock);
  /*Signal main thread to terminate */
  pthread_mutex_lock(&mct->mctl_mutex);
  mct->serv_cmd_q_counter++;
  pthread_cond_signal(&mct->mctl_cond);
  pthread_mutex_unlock(&mct->mctl_mutex);
  pthread_join(mct->mct_tid, NULL);
  CLOGI(CAM_MCT_MODULE, "CAM_MctServ thread closed");

  /*Closing MCT Bus thread */
  bus_msg_destroy = (mct_bus_msg_t*)malloc(sizeof(mct_bus_msg_t));
  if (!bus_msg_destroy) {
    CLOGE(CAM_MCT_MODULE, "Malloc failed while closing bus thread");
    return FALSE;
  }
  memset(bus_msg_destroy, 0, sizeof(mct_bus_msg_t));
  bus_msg_destroy->type = MCT_BUS_MSG_CLOSE_CAM;
  pthread_mutex_lock(&mct->pipeline->bus->priority_q_lock);
  mct_queue_push_tail(mct->pipeline->bus->priority_queue, bus_msg_destroy);
  pthread_mutex_unlock(&mct->pipeline->bus->priority_q_lock);
  /*Signal bus_handle thread to terminate */
  pthread_mutex_lock(&mct->mctl_bus_handle_mutex);
  pthread_cond_signal(&mct->mctl_bus_handle_cond);
  pthread_mutex_unlock(&mct->mctl_bus_handle_mutex);
  pthread_join(mct->mct_bus_handle_tid, NULL);
  CLOGI(CAM_MCT_MODULE, "CAM_MctBus thread closed");

  return TRUE;
}


/** mct_controller_new:
 *   @mods: modules list
 *   @session_idx: session index
 *   @serv_fd: file descriptor for MCT to communicate
 *             back to imaging server
 *   @callback: callback provided by HAL to communicate to
                     HAL incase of error or notifications.
 *    Return: Status of back-end.
 *    Description:
 *    create a new Media Controller object. This creates
 *    a new pipeline and starts a session on all inited modules
 *
 * This function executes in SERVER context
 **/
cam_status_t mct_controller_new(mct_list_t *mods,
  unsigned int session_idx, int serv_fd, void *event_cb)
{
  mct_controller_t *mct = NULL;
  pthread_t         tid;
  pthread_t         bus_hdl_tid;
  cam_status_t ret_type = CAM_STATUS_FAILED;
  char main_thread_name[16];
  char bus_thread_name[16];

  cam_set_dbg_log_properties();

  CAMSCOPE_INIT(CAMSCOPE_SECTION_MMCAMERA);

  mct = (mct_controller_t *)malloc(sizeof(mct_controller_t));
  if (!mct)
    goto mct_error;
  memset(mct, 0, sizeof(mct_controller_t));
  CLOGI(CAM_MCT_MODULE,
        "Creating new mct_controller with session-id %d", session_idx);

  mct->pipeline = mct_pipeline_new(session_idx, mct);
  if (!mct->pipeline)
    goto pipeline_error;

  mct->pipeline->modules = mods;

  /* Add MCT as pipeline's parent */
  MCT_PIPELINE_LOCK(mct->pipeline->object);
  MCT_PIPELINE_PARENT(mct->pipeline->object) =
    mct_list_append(MCT_PIPELINE_PARENT(mct->pipeline->object), mct, NULL, NULL);
  if (!MCT_PIPELINE_PARENT(mct->pipeline->object)) {
    CLOGE(CAM_MCT_MODULE, "Couldn't append MCT as parent of pipeline");
    MCT_PIPELINE_UNLOCK(mct->pipeline->object);
    mct_pipeline_destroy(mct->pipeline);
    goto pipeline_error;
  }
  MCT_PIPELINE_UNLOCK(mct->pipeline->object);

  /* Add Pipeline as MCT's child */
  MCT_LOCK(mct);
  MCT_CHILDREN(mct) =
    mct_list_append(MCT_CHILDREN(mct),
                    mct->pipeline, NULL, NULL);
  if (!MCT_CHILDREN(mct)) {
    CLOGE(CAM_MCT_MODULE, "Couldn't append pipeline as MCT's child");
    MCT_UNLOCK(mct);
    mct_pipeline_destroy(mct->pipeline);
    goto pipeline_error;
  }
  MCT_UNLOCK(mct);

  mct->serv_cmd_q = mct_queue_new;
  if (!mct->serv_cmd_q) {
     ret_type = CAM_STATUS_FAILED;
     mct_pipeline_destroy(mct->pipeline);
     goto pipeline_error;
  }

  mct_queue_init(mct->serv_cmd_q);

  pthread_mutex_init(&mct->mctl_thread_started_mutex, NULL);
  pthread_cond_init(&mct->mctl_thread_started_cond, NULL);

  mct->serv_cmd_q_counter = 0;

  pthread_mutex_init(&mct->mctl_mutex, NULL);
  pthread_cond_init(&mct->mctl_cond, NULL);

  pthread_mutex_init(&mct->serv_msg_q_lock, NULL);

  pthread_mutex_init (&mct->mctl_state_mutex, NULL);
  pthread_cond_init(&mct->mctl_state_cond, NULL);

#ifndef DAEMON_PRESENT
  pthread_mutex_init(&mct->server_lock, NULL);
  pthread_mutex_init (&mct->serv_res_cond_lock, NULL);
  pthread_condattr_init(&mct->serv_res_condattr);
  pthread_condattr_setclock(&mct->serv_res_condattr, CLOCK_MONOTONIC);
  pthread_cond_init(&mct->serv_res_cond, &mct->serv_res_condattr);

  mct->serv_res_q = mct_queue_new;
  if (!mct->serv_res_q) {
    ret_type = CAM_STATUS_FAILED;
    pthread_mutex_destroy(&mct->server_lock);
    pthread_mutex_destroy(&mct->serv_res_cond_lock);
    pthread_cond_destroy(&mct->serv_res_cond);
    pthread_condattr_destroy(&mct->serv_res_condattr);
    mct_pipeline_destroy(mct->pipeline);
    goto pipeline_error;
  }
  mct_queue_init(mct->serv_res_q);
#endif

  mct->event_cb = event_cb;
  mct->bus_thread_state = MCT_THREAD_STATE_IDLE;

/* When the daemon is present serv_fd is used for communication between daemon and MCT
  where as in absence of daemon config_fd i.e. video0 node which when opened in shimlayer
  context is passed along to MCT communicate with camera kernel.*/
#ifdef DAEMON_PRESENT
  mct->serv_fd  = serv_fd;
#else
  mct->config_fd = serv_fd;
#endif

  /*Starting CAM_MctServ thread */
  pthread_mutex_lock(&mct->mctl_thread_started_mutex);
  if (pthread_create(&tid, NULL, mct_controller_thread_run, mct)) {
    pthread_mutex_unlock(&mct->mctl_thread_started_mutex);
    goto main_thread_error;
  }
  snprintf(main_thread_name, sizeof(main_thread_name),
    "CAM_MctServ_%d", session_idx);
  pthread_setname_np(tid, main_thread_name);

  pthread_cond_wait(&mct->mctl_thread_started_cond,
    &mct->mctl_thread_started_mutex);
  pthread_mutex_unlock(&mct->mctl_thread_started_mutex);
  mct->mct_tid = tid;
  CLOGD(CAM_MCT_MODULE,"CAM_MctServ thread started");

  /*Starting CAM_MctBus thread */
  pthread_mutex_init(&mct->mctl_bus_handle_mutex, NULL);
  pthread_cond_init(&mct->mctl_bus_handle_cond, NULL);
  pthread_mutex_lock(&mct->mctl_bus_handle_mutex);
  if (pthread_create(&bus_hdl_tid, NULL, mct_bus_handler_thread_run, mct)) {
    pthread_mutex_unlock(&mct->mctl_bus_handle_mutex);
    goto bus_handle_thread_error;
  }
  snprintf(bus_thread_name, sizeof(bus_thread_name),
    "CAM_MctBus_%d", session_idx);
  pthread_setname_np(bus_hdl_tid, bus_thread_name);
  pthread_mutex_unlock(&mct->mctl_bus_handle_mutex);
  mct->mct_bus_handle_tid = bus_hdl_tid;
  mct->pipeline->bus->mct_mutex = &mct->mctl_bus_handle_mutex;
  mct->pipeline->bus->mct_cond  = &mct->mctl_bus_handle_cond;
  CLOGD(CAM_MCT_MODULE, "CAM_MctBus thread started");

  ret_type = mct_pipeline_start_session(mct->pipeline);
  if (CAM_STATUS_SUCCESS != ret_type) {
    CLOGE(CAM_MCT_MODULE, "Start session failed with status %d", ret_type);
    goto start_session_error;
  }

  if (!(mcts = mct_list_append(mcts, mct, NULL, NULL))) {
    goto start_session_error;
  }

  return CAM_STATUS_SUCCESS;

start_session_error:
  mct_pipeline_stop_session(mct->pipeline);
  mct_controller_close_thread(mct);
bus_handle_thread_error:
  pthread_cond_destroy(&mct->mctl_bus_handle_cond);
  pthread_mutex_destroy(&mct->mctl_bus_handle_mutex);
main_thread_error:
  pthread_cond_destroy(&mct->mctl_thread_started_cond);
  pthread_mutex_destroy(&mct->mctl_thread_started_mutex);
  pthread_mutex_destroy(&mct->serv_msg_q_lock);
  pthread_cond_destroy(&mct->mctl_cond);
  pthread_mutex_destroy(&mct->mctl_mutex);
  pthread_mutex_destroy(&mct->mctl_state_mutex);
  pthread_cond_destroy(&mct->mctl_state_cond);
  mct_queue_free(mct->serv_cmd_q);
  mct->serv_cmd_q = NULL;
  mct_pipeline_destroy(mct->pipeline);
pipeline_error:
  free(mct);
  mct = NULL;
mct_error:
  return ret_type;
}

/** mct_controller_find_session:
 *    @d1: media controller objective
 *    @d2: session index
 *
 * To find a MCT from MCTs list based on session index.
 *
 *  Return TRUE if MCT exists.
 **/
static boolean mct_controller_find_session(void *d1, void *d2)
{
  return ((((mct_controller_t *)d1)->pipeline->session) ==
          *(unsigned int *)d2 ? TRUE : FALSE);
}


/** mct_controller_destroy:
 *    @sessionIdx: the corresponding session index's
 *       MCT to be removed
 *
 **/
boolean mct_controller_destroy(unsigned int session_idx)
{
  mct_serv_msg_t   *msg;
  mct_controller_t *mct;
  mct_list_t       *mct_list;
  mct_bus_msg_t *bus_msg_destroy;
  mct_pipeline_t *pipeline = NULL;
  boolean super_parm_flush = FALSE;

  mct_list = mct_list_find_custom(mcts, &session_idx,
        mct_controller_find_session);
  if (!mct_list) {
    return FALSE;
  }
  mct = (mct_controller_t *)mct_list->data;
  CLOGI(CAM_MCT_MODULE,
        "Initiating destroy sequence for session = %d", session_idx);
  pipeline = mct->pipeline;
  msg = calloc(1, sizeof(mct_serv_msg_t));
  if (!msg)
    return FALSE;

  msg->msg_type = SERV_MSG_HAL;
  msg->u.hal_msg.id = MSM_CAMERA_DEL_SESSION;

  pthread_mutex_lock(&mct->serv_msg_q_lock);
  mct_queue_push_tail(mct->serv_cmd_q, msg);
  pthread_mutex_unlock(&mct->serv_msg_q_lock);

  pthread_mutex_lock(&mct->mctl_mutex);
  mct->serv_cmd_q_counter++;
  pthread_cond_signal(&mct->mctl_cond);
  pthread_mutex_unlock(&mct->mctl_mutex);
  pthread_join(mct->mct_tid, NULL);
  CLOGI(CAM_MCT_MODULE, "serv_thread closed");

  bus_msg_destroy = (mct_bus_msg_t*)malloc(sizeof(mct_bus_msg_t));
  if(!bus_msg_destroy)
    return FALSE;
  memset(bus_msg_destroy, 0, sizeof(mct_bus_msg_t));
  bus_msg_destroy->type = MCT_BUS_MSG_CLOSE_CAM;
  pthread_mutex_lock(&mct->pipeline->bus->priority_q_lock);
  mct_queue_push_tail(mct->pipeline->bus->priority_queue, bus_msg_destroy);
  pthread_mutex_unlock(&mct->pipeline->bus->priority_q_lock);
  /*Signal bus_handle thread to terminate */
  pthread_mutex_lock(&mct->mctl_bus_handle_mutex);
  pthread_cond_signal(&mct->mctl_bus_handle_cond);
  pthread_mutex_unlock(&mct->mctl_bus_handle_mutex);
  pthread_join(mct->mct_bus_handle_tid, NULL);
  CLOGI(CAM_MCT_MODULE, "bus_handler thread closed");

  /* Destroy Supermct */
  if (TRUE == is_supermct_exist(mct->pipeline)) {
    supermct_destroy(mct->pipeline);
  }

  MCT_LOCK(mct);
  /* Remove supermct from MCT parent object */
  if (MCT_PARENT(mct)) {
    mct_list_free_list(MCT_PARENT(mct));
  }
  MCT_PARENT(mct) = NULL;
  MCT_NUM_PARENTS(mct) = 0;

  /* Remove all Pipelines from MCT children object */
  if (MCT_CHILDREN(mct)) {
    mct_list_free_list(MCT_CHILDREN(mct));
  }
  MCT_CHILDREN(mct) = NULL;
  MCT_NUM_CHILDREN(mct) = 0;
  MCT_UNLOCK(mct);

  /* Remove all MCTs from pipeline parent object */
  MCT_PIPELINE_LOCK(mct->pipeline->object);
  if (MCT_PIPELINE_PARENT(mct->pipeline->object)) {
    mct_list_free_list(MCT_PIPELINE_PARENT(mct->pipeline->object));
  }
  MCT_PIPELINE_PARENT(mct->pipeline->object) = NULL;
  MCT_PIPELINE_UNLOCK(mct->pipeline->object);

  /*Destroy super_param queue if it exists */
  pthread_mutex_lock(&pipeline->super_param_q_lock);
  if (!MCT_QUEUE_IS_EMPTY(pipeline->super_param_queue)) {
    super_parm_flush = TRUE;
  }
  pthread_mutex_unlock(&pipeline->super_param_q_lock);
  if (super_parm_flush) {
    mct_pipeline_flush_super_param_queue(pipeline);
  }

  pthread_mutex_lock(&pipeline->super_param_q_lock);
  free(pipeline->super_param_queue);
  pipeline->super_param_queue = NULL;
  pthread_mutex_unlock(&pipeline->super_param_q_lock);
  pthread_mutex_destroy(&pipeline->super_param_q_lock);

  /*Destroying Pipeline */
  mct_pipeline_stop_session(pipeline);
  mct_pipeline_destroy(pipeline);

  pthread_cond_destroy(&mct->mctl_thread_started_cond);
  pthread_mutex_destroy(&mct->mctl_thread_started_mutex);
  pthread_mutex_destroy(&mct->serv_msg_q_lock);
  pthread_mutex_destroy(&mct->mctl_mutex);
  pthread_mutex_destroy(&mct->mctl_bus_handle_mutex);
  pthread_mutex_destroy(&mct->mctl_state_mutex);
  pthread_cond_destroy(&mct->mctl_state_cond);
#ifndef DAEMON_PRESENT
  pthread_mutex_destroy (&mct->serv_res_cond_lock);
  pthread_cond_destroy(&mct->serv_res_cond);
  pthread_condattr_destroy(&mct->serv_res_condattr);
  pthread_mutex_destroy(&mct->server_lock);
  if (!MCT_QUEUE_IS_EMPTY(mct->serv_res_q)) {
    mct_queue_free(mct->serv_res_q);
  } else {
    free(mct->serv_res_q);
  }
#endif
  if (!MCT_QUEUE_IS_EMPTY(mct->serv_cmd_q)) {
    mct_queue_free(mct->serv_cmd_q);
  }else
    free(mct->serv_cmd_q);

  mcts = mct_list_remove(mcts, mct);
  free(mct);
  mct = NULL;
  CAMSCOPE_DESTROY(CAMSCOPE_SECTION_MMCAMERA);
  CLOGI(CAM_MCT_MODULE, "X Successfully closed mct_controller session %d",
    session_idx);
  return TRUE;
}

/** mct_controller_proc_servmsg:
 *    @servMsg: the message to be posted
 *
 * Post imaging server message to Media Controller's message
 *    message queue.
 *
 * This function executes in Imaging Server context
 **/
boolean mct_controller_proc_serv_msg(mct_serv_msg_t *serv_msg)
{
  mct_controller_t *mct;
  mct_list_t       *mct_list;
  mct_serv_msg_t   *msg;
  mct_process_ret_t *res_msg = NULL;
  unsigned int     session;
  boolean ret = FALSE;
  boolean ret_time = FALSE;
  int rc = 0;
  struct timespec timedwait;
  char prop[PROPERTY_VALUE_MAX];
  int enable_stuck = 0;

  switch (serv_msg->msg_type) {
  case SERV_MSG_DS: {
    session = serv_msg->u.ds_msg.session;
  }
    break;

  case SERV_MSG_HAL: {
    struct msm_v4l2_event_data *data =
      (struct msm_v4l2_event_data *)(serv_msg->u.hal_msg.u.data);
    session = data->session_id;
  }
    break;

  default:
    CLOGE(CAM_MCT_MODULE, "Invalid serv_msg->msg_type %d", serv_msg->msg_type);
    return FALSE;
  }

  mct_list = mct_list_find_custom(mcts, &session,
    mct_controller_find_session);
  if (!mct_list) {
    CLOGE(CAM_MCT_MODULE, "Failed to find session.");
    return FALSE;
  }

  mct = (mct_controller_t *)mct_list->data;

  msg = malloc(sizeof(mct_serv_msg_t));
  if (!msg) {
    CLOGE(CAM_MCT_MODULE, "Failed to allocate memory. errno %d", errno);
    return FALSE;
  }

  *msg = *serv_msg;

  /* Push message to Media Controller Message Queue
   * and post signal to Media Controller */
#ifndef DAEMON_PRESENT
  pthread_mutex_lock(&mct->server_lock);
  pthread_mutex_lock(&mct->serv_msg_q_lock);
  mct_queue_push_tail(mct->serv_cmd_q, msg);
  pthread_mutex_unlock(&mct->serv_msg_q_lock);

  pthread_mutex_lock(&mct->mctl_mutex);
  mct->serv_cmd_q_counter++;
  pthread_cond_signal(&mct->mctl_cond);
  pthread_mutex_unlock(&mct->mctl_mutex);

  pthread_mutex_lock(&mct->serv_res_cond_lock);
  if (!mct->serv_res_q->length) {
    ret_time = mct_util_get_timeout(MCT_THREAD_TIMEOUT, &timedwait);
    if (!ret_time) {
      CLOGE(CAM_MCT_MODULE, "Failed to get timeout interval");
      pthread_mutex_unlock(&mct->serv_res_cond_lock);
      pthread_mutex_unlock(&mct->server_lock);
      ret = FALSE;
      return ret;
    }
    rc = pthread_cond_timedwait(&mct->serv_res_cond, &mct->serv_res_cond_lock,
      &timedwait);
    if (rc == ETIMEDOUT) {
      CLOGE(CAM_MCT_MODULE, "Timedout in processing HAL command type =%d",
        serv_msg->msg_type);
      pthread_mutex_unlock(&mct->serv_res_cond_lock);
      pthread_mutex_unlock(&mct->server_lock);
      /*Debugging logic to get tombstone incase of timeout */
      property_get("persist.vendor.camera.mctstuck.enable", prop, "0");
      enable_stuck = atoi(prop);
      if (enable_stuck == 1) {
        raise(SIGABRT);
        raise(SIGTERM);
      }
      mct->timedout_status = TRUE;
      ret = FALSE;
      return ret;
    }
  }

  res_msg = (mct_process_ret_t *)mct_queue_pop_head(mct->serv_res_q);
  if (!res_msg) {
    CLOGE(CAM_MCT_MODULE, "Something has gone wrong should not be here");
    pthread_mutex_unlock(&mct->serv_res_cond_lock);
    pthread_mutex_unlock(&mct->server_lock);
    return FALSE;
  }
  pthread_mutex_unlock(&mct->serv_res_cond_lock);

  if (res_msg->u.serv_msg_ret.error == TRUE) {
    ret = TRUE;
  }
  pthread_mutex_unlock(&mct->server_lock);
  free(res_msg);
  res_msg = NULL;
#else
  pthread_mutex_lock(&mct->serv_msg_q_lock);
  mct_queue_push_tail(mct->serv_cmd_q, msg);
  pthread_mutex_unlock(&mct->serv_msg_q_lock);

  pthread_mutex_lock(&mct->mctl_mutex);
  mct->serv_cmd_q_counter++;
  pthread_cond_signal(&mct->mctl_cond);
  pthread_mutex_unlock(&mct->mctl_mutex);
  ret = TRUE;
#endif

  return ret;
}

/** mct_controller_check_pipeline
 *    @pipeline: pipeline objective
 *
 * Check the pipeline's vadility
 **/
static boolean mct_controller_check_pipeline(mct_pipeline_t *pipeline)
{
  return ((pipeline->add_stream    &&
           pipeline->remove_stream &&
           pipeline->send_event    &&
           pipeline->set_bus       &&
           pipeline->get_bus) ? TRUE : FALSE);
}

/** mct_controller_proc_servmsg_internal:
 *    @mct: Media Controller Object
 *    @msg: message object from imaging server
 *
 * Media Controller process Imaging Server messages
 * Return: mct_process_ret_t
 *
 * This function executes in Media Controller's thread context
 **/
static mct_process_ret_t mct_controller_proc_serv_msg_internal(
  mct_controller_t *mct, mct_serv_msg_t *msg)
{
  mct_process_ret_t ret;
  mct_pipeline_t    *pipeline;

  memset(&ret, 0x00, sizeof(mct_process_ret_t));
  ret.type = MCT_PROCESS_RET_SERVER_MSG;
  ret.u.serv_msg_ret.error = TRUE;

  if (!mct || !msg || !mct->pipeline) {
    CLOGE(CAM_MCT_MODULE, "Sanity check failed");
    ret.u.serv_msg_ret.error = TRUE;
    return ret;
  }

  ret.u.serv_msg_ret.msg = *msg;
  pipeline = mct->pipeline;

  if (!mct_controller_check_pipeline(pipeline)) {
    CLOGE(CAM_MCT_MODULE, "Pipeline check failed");
    ret.u.serv_msg_ret.error = TRUE;
    return ret;
  }

  switch (msg->msg_type) {
  case SERV_MSG_DS: {
    if ((msg->u.ds_msg.operation == CAM_MAPPING_TYPE_FD_BUNDLED_MAPPING ||
       msg->u.ds_msg.operation == CAM_MAPPING_TYPE_FD_MAPPING)
       && pipeline->map_buf) {
      ret.u.serv_msg_ret.error = pipeline->map_buf(&msg->u.ds_msg, pipeline);
    } else if (msg->u.ds_msg.operation == CAM_MAPPING_TYPE_FD_UNMAPPING &&
        pipeline->unmap_buf) {
      ret.u.serv_msg_ret.error = pipeline->unmap_buf(&msg->u.ds_msg, pipeline);
    }
  }
    break;

  case SERV_MSG_HAL:
    if (pipeline->process_serv_msg)
      ret.u.serv_msg_ret.error = pipeline->process_serv_msg(&msg->u.hal_msg,
        pipeline);
    break;

  default:
    break;
  }
  return ret;
}

/** mct_controller_handle_offline_meta_proc:
 *    @pipeline: structure of mct_controller_t
 *    @bus_msg: Incoming bus_msg to process
 *
 *  This function handle offline metadata bus message
 **/

/**Name: mct_controller_handle_offline_meta_proc:
 *
 * Arguments/Fields:
 *    @Pipeline: structure of mct_pipeline_t
 *    @bus_msg: bus messages to process
 *
 *  Return: boolean;
 *    TRUE on success; FALSE on failure
 *
 *  Description:
 *    This function handle offline metadata bus message
 **/
static boolean mct_controller_handle_offline_meta_proc(
   mct_pipeline_t *pipeline, mct_bus_msg_t *bus_msg)
{
  struct msm_buf_mngr_info buf_info;
  mct_stream_t *offline_raw_stream = NULL;
  mct_pipeline_get_stream_info_t info;
  mct_stream_t *output_stream = NULL;
  boolean ret = TRUE;
  mct_bus_msg_isp_rd_done_t *isp_rd_done_msg = NULL;
  int status = 0;

  /* Obtain Raw stream to issue commands on */
  if (MCT_PIPELINE_NUM_CHILDREN(pipeline) > 0) {
    info.check_type   = CHECK_FEATURE_MASK;
    info.stream_type = CAM_STREAM_TYPE_OFFLINE_PROC;
    info.feature_mask  = CAM_QCOM_FEATURE_RAW_PROCESSING;
    offline_raw_stream = mct_pipeline_get_stream(pipeline, &info);
    if (!offline_raw_stream) {
      CLOGE(CAM_MCT_MODULE, "No offline stream available at SOF");
      return FALSE;
    }
  } else {
    CLOGE(CAM_MCT_MODULE, "No child present for pipeline");
    return FALSE;
  }
  isp_rd_done_msg = (mct_bus_msg_isp_rd_done_t *)bus_msg->msg;
  /* Sending Offline proc command to modules */
  ret = mct_stream_send_offline_meta(offline_raw_stream, isp_rd_done_msg);

  /*Buf done for Offline metadata buffer */
  buf_info.index = pipeline->offline_meta.output_buf_idx;
  buf_info.frame_id = isp_rd_done_msg->frame_id;
  buf_info.stream_id = pipeline->offline_meta.stream_id;
  buf_info.session_id = MCT_PIPELINE_SESSION(
  MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(offline_raw_stream)->data));

  CLOGD(CAM_MCT_MODULE,
        "Sending offline metadata buffer index %d", buf_info.index);
  status = ioctl(pipeline->offline_meta.buf_mgr_fd,
      VIDIOC_MSM_BUF_MNGR_BUF_DONE, &buf_info);
  if (status < 0) {
    CLOGE(CAM_MCT_MODULE, "Failed to do buf_done errno: %s!!! for buffer with"
        "index: %d, stream id: %x, session id: %d", strerror(errno),
        buf_info.index, buf_info.stream_id, buf_info.session_id);
    ret = FALSE;
  }
  return ret;
}

/**Name: mct_controller_handle_HW_error:
 *
 * Arguments/Fields:
 *    @Pipeline: structure of mct_pipeline_t
 *
 *  Return: boolean;
 *    TRUE on success; FALSE on failure
 *
 *  Description:
 *    This function handle hw error bus message
 **/

static boolean mct_controller_handle_HW_error(mct_pipeline_t *pipeline)
{
  boolean ret = TRUE;
  int status = 0;
  mct_stream_t *parm_stream;
  mct_pipeline_get_stream_info_t info;

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Pipeline not present");
    return FALSE;
  }
  /* Sending on Session based stream */
  if (MCT_PIPELINE_NUM_CHILDREN(pipeline) > 0) {
    info.check_type   = CHECK_TYPE;
    info.stream_type  = CAM_STREAM_TYPE_PARM;
    parm_stream = mct_pipeline_get_stream(pipeline, &info);
    if (!parm_stream) {
      CLOGE(CAM_MCT_MODULE, "No parm stream available");
      return FALSE;
    }
  } else {
    CLOGE(CAM_MCT_MODULE, "No child present for pipeline");
    return FALSE;
  }
  /* Sending HW error to modules */
  ret = mct_stream_send_hw_error(parm_stream);

  return ret;
}

/** mct_controller_notify_hal_internal_event:
 *   @pipeline: Pointer to pipeline object
 *   @command: Command to be notified to HAL
 *
 *   Open up a channel for posting asynchronous notifications
 *   to HAL by MCT. It packs the command in a v4l2 event
 *   structure and writes into mct_msg_wt_fd pipe.
 *   return: Success/Failure
 **/
static boolean mct_controller_notify_hal_internal_event(
 mct_pipeline_t *pipeline, cam_event_type_t command)
{
  mct_process_ret_t  proc_ret;
  struct v4l2_event *event;
  cam_event_t cam_event;
  int rc;
  mct_controller_t *mct = NULL;
  mm_camera_shim_event_handler_func evt_cb;

if (!pipeline) {
   CLOGE(CAM_MCT_MODULE, "Pipeline ptr NULL");
   return FALSE;
  }

#ifndef DAEMON_PRESENT
  mct = pipeline->controller;
  if (!mct) {
    CLOGE(CAM_MCT_MODULE, "NULL mct_controller object.");
    return FALSE;
  }
  cam_event.server_event_type = command;
  cam_event.status = CAM_STATUS_SUCCESS;

  evt_cb = (mm_camera_shim_event_handler_func)mct->event_cb;
  if (!evt_cb) {
    CLOGE(CAM_MCT_MODULE, "Callback not registered");
    return FALSE;
  }
  rc = evt_cb(pipeline->session,&cam_event);
  if (rc == FALSE) {
    CLOGE(CAM_MCT_MODULE, "Failed to send callback cmd = %d", command);
    return FALSE;
  }
#else
  proc_ret.type = MCT_PROCESS_NOTIFY_SERVER_MSG;
  proc_ret.u.serv_msg_ret.error = TRUE;

  /* Populate v4l2 event with necessary arguments */
  event = &proc_ret.u.serv_msg_ret.msg.u.hal_msg;
  struct msm_v4l2_event_data *data =
    (struct msm_v4l2_event_data *)(&(event->u.data[0]));
  data->session_id = pipeline->session;
  data->command    = command;

  /* Now post notify event on pipe */
  if (pipeline->controller) {
    write(pipeline->controller->serv_fd, &proc_ret, sizeof(mct_process_ret_t));
  } else {
    CLOGE(CAM_MCT_MODULE,
     "NULL mct_controller object. Not found serv_fd to write on");
    return FALSE;
  }
#endif

  return TRUE;
}


boolean mct_controller_send_cb(mct_pipeline_t *pipeline,
  mct_bus_msg_type_t type)
{
  boolean rc = FALSE;
  mct_pipeline_get_stream_info_t info;
  mct_stream_t *parm_stream = NULL;

  switch (type) {

    case MCT_BUS_MSG_SEND_HW_ERROR: {
      CLOGE(CAM_MCT_MODULE, "FATAL: Sending HW_Error");
      rc = mct_controller_notify_hal_internal_event(pipeline,
      CAM_EVENT_TYPE_DAEMON_DIED);
      info.check_type  = CHECK_INDEX;
      info.stream_index  = MCT_SESSION_STREAM_ID;
      parm_stream = mct_pipeline_get_stream(pipeline, &info);
      if (!parm_stream) {
        CLOGE(CAM_MCT_MODULE, "Couldn't find session stream");
        rc = FALSE;
        break;
      }
      parm_stream->state = MCT_ST_STATE_BAD;
    }
      break;
    case MCT_BUS_MSG_EZTUNE_JPEG: {
      rc = mct_controller_notify_hal_internal_event(pipeline,
        CAM_EVENT_TYPE_INT_TAKE_JPEG);
    }
      break;
    case MCT_BUS_MSG_EZTUNE_RAW: {
      rc = mct_controller_notify_hal_internal_event(pipeline,
        CAM_EVENT_TYPE_INT_TAKE_RAW);
    }
      break;
    case MCT_BUS_MSG_CAC_STAGE_DONE: {
      rc = mct_controller_notify_hal_internal_event(pipeline,
        CAM_EVENT_TYPE_CAC_DONE);
    }
      break;
    default: {
      CLOGE(CAM_MCT_MODULE, "Unsupported bus msg Type = %d", type);
    }
      break;
  }

  return rc;
}

/**  mct_controller_start_debug_thread
 *
 *    @mct: Media Controller Object
 *    @bus_msg: message object from bus
 *
 * Return: boolean
 *
 * This function is invoked when a possible SOF freeze is detected and it starts
 * CAM_mct_freeze thread which tries to dump data into file from various hw and
 * sends an ioctl to kernel intimating it about possible SOF freeze when daemon
 * is not present.
 **/

static void mct_controller_start_debug_thread(mct_controller_t *mct,
  mct_bus_msg_t *bus_msg)
{
  int sof_debug, rc;
  pthread_attr_t attr;
  mct_pipeline_t *pipeline = NULL;
  struct msm_v4l2_event_data data;

  sof_debug = *(int *)bus_msg->msg;
  pipeline = mct->pipeline;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&pipeline->thread_data.pid, &attr,
    &server_debug_dump_data_for_sof_freeze, &sof_debug);
  pthread_setname_np(pipeline->thread_data.pid, "CAM_mct_freeze");

#ifndef DAEMON_PRESENT
  data.session_id = pipeline->session;
  data.status = sof_debug;
  rc = ioctl(mct->config_fd, MSM_CAM_V4L2_IOCTL_NOTIFY_DEBUG, &data);
  if (rc < 0) {
    CLOGE(CAM_MCT_MODULE, "Failed to send Notify debug to kernel errno =%s",
      strerror(errno));
  }
#endif

  return;
}

/** mct_controller_proc_bus_msg_internal:
 *    Media Controller process Bus messages
 *
 *    @mct: Media Controller Object
 *    @bus_msg: message object from bus
 *
 * Return: mct_process_ret_t
 *
 * This function processes all bus messages sent on the priority queue
 **/
static mct_process_ret_t mct_controller_proc_bus_msg_internal(
  mct_controller_t *mct, mct_bus_msg_t *bus_msg)
{
  mct_process_ret_t ret;
  mct_pipeline_t    *pipeline;
  mct_bus_msg_t int_bus_msg;
  mct_bus_msg_error_message_t err_msg;
  mct_process_ret_t int_ret;

  ret.u.bus_msg_ret.error = TRUE;
  ret.type = MCT_PROCESS_RET_BUS_MSG;

  if (!mct || !bus_msg || !mct->pipeline) {
    return ret;
  }
  if (!mct_controller_check_pipeline(mct->pipeline)) {
    return ret;
  }
  mct_controller_set_thread_state(mct, MCT_THREAD_STATE_RUNNING);
  ret.u.bus_msg_ret.error = FALSE;
  ret.u.bus_msg_ret.msg_type = bus_msg->type;
  ret.u.bus_msg_ret.session = bus_msg->sessionid;
  pipeline = mct->pipeline;

  if (bus_msg->type == MCT_BUS_MSG_NOTIFY_KERNEL) {
    mct_controller_start_debug_thread(mct, bus_msg);
    ret.type = MCT_PROCESS_DUMP_INFO;
    ret.u.bus_msg_ret.sof_debug = *(int *)bus_msg->msg;
    return ret;
  }

  if (bus_msg->type == MCT_BUS_MSG_CONTROL_REQUEST_FRAME) {
    ret.u.bus_msg_ret.error = FALSE;
    if (!mct_pipeline_control_hw_state(pipeline, bus_msg->msg)) {
      CLOGE(CAM_MCT_MODULE, "Failed to set HW module state");
    }
    return ret;
  }

#ifndef DAEMON_PRESENT
  if (bus_msg->type == MCT_BUS_MSG_SEND_HW_ERROR ||
    bus_msg->type == MCT_BUS_MSG_EZTUNE_JPEG ||
    bus_msg->type == MCT_BUS_MSG_EZTUNE_RAW) {
    ret.u.bus_msg_ret.error = mct_controller_send_cb(pipeline, bus_msg->type);
    if (ret.u.bus_msg_ret.error == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Failed to send callback to HAL for type = %d",
        bus_msg->type);
      return ret;
    }
    return ret;
  }
#else
  if (bus_msg->type == MCT_BUS_MSG_SEND_HW_ERROR) {
    CLOGE(CAM_MCT_MODULE, "MCT_BUS_MSG_SEND_HW_ERROR");
    mct_controller_handle_HW_error(pipeline);
    ret.type = MCT_PROCESS_RET_ERROR_MSG;
    return ret;
  }

  if (bus_msg->type == MCT_BUS_MSG_EZTUNE_JPEG ||
      bus_msg->type == MCT_BUS_MSG_EZTUNE_RAW) {
    ret.type = MCT_PROCESS_RET_BUS_MSG;
    return ret;
  }
#endif

  if (MCT_BUS_INTMSG_PROC_SOF_WO_PARAM == bus_msg->type) {
   if (!mct_controller_handle_SOF_proc(pipeline, bus_msg,
     MCT_SUPERPARAM_DC_WO_PARAM)) {
     ret.u.bus_msg_ret.error = FALSE;
     CLOGE(CAM_MCT_MODULE, "Failed to process SOF without param");
     return ret;
   }
  }

  if (MCT_BUS_INTMSG_PROC_SOF_W_PARAM == bus_msg->type) {
    if (!mct_controller_handle_SOF_proc(pipeline, bus_msg,
      MCT_SUPERPARAM_DC_W_PARAM)) {
      ret.u.bus_msg_ret.error = FALSE;
      CLOGE(CAM_MCT_MODULE, "Failed to process SOF with param");
      return ret;
    }
  }

  if (MCT_BUS_MSG_ISP_SOF == bus_msg->type) {
      pthread_mutex_lock(&pipeline->stream_ops_lock);
    if (!mct_controller_handle_SOF_proc(pipeline, bus_msg,
      MCT_SUPERPARAM_SINGLE_CAM)) {
      ret.u.bus_msg_ret.error = FALSE;
      pthread_mutex_unlock(&pipeline->stream_ops_lock);
      CLOGE(CAM_MCT_MODULE, "Failed to issue SOF cmd to all modules.");
      return ret;
    }
      pthread_mutex_unlock(&pipeline->stream_ops_lock);
  }

  if (MCT_BUS_MSG_ISP_RD_DONE == bus_msg->type) {
   if (!mct_controller_handle_offline_meta_proc(pipeline, bus_msg)) {
     ret.u.bus_msg_ret.error = FALSE;
     CLOGE(CAM_MCT_MODULE, "ISP RD done control event failed");
     return ret;
   }
  }

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_MCT_BUS_PROC);
  if (pipeline->process_bus_msg)
    ret.u.bus_msg_ret.error = pipeline->process_bus_msg(bus_msg, pipeline);
  ATRACE_CAMSCOPE_END(CAMSCOPE_MCT_BUS_PROC);

  if (pipeline->start_int_streamoff_on) {
    CLOGI(CAM_MCT_MODULE, "Internal streamon and off start");
    pipeline->int_streamoff = 0;
    memset(&int_bus_msg, 0, sizeof(mct_bus_msg_t));
    memset(&err_msg, 0, sizeof(mct_bus_msg_error_message_t));
    int_bus_msg.type = MCT_BUS_MSG_ERROR_MESSAGE;
    err_msg = MCT_ERROR_MSG_INT_RESTART_STREAMING;
    int_bus_msg.msg = &err_msg;
    if (pipeline->process_bus_msg) {
      int_ret.u.bus_msg_ret.error = pipeline->process_bus_msg(&int_bus_msg,
       pipeline);
      CLOGE(CAM_MCT_MODULE, "retval MCT_ERROR_MSG_INT_RESTART_STREAMING: %d",
            int_ret.u.bus_msg_ret.error);
    }
    pipeline->start_int_streamoff_on = 0;
    CLOGI(CAM_MCT_MODULE, "Internal streamon and off End");
  }
#ifndef DAEMON_PRESENT
  /* Sending CAC Stage done notification to HAL. It is sent after metadata buf_done as the
   single bus msg is used for notification and to send metadata to HAL */
  if (bus_msg->type == MCT_BUS_MSG_CAC_STAGE_DONE) {
    ret.u.bus_msg_ret.error = mct_controller_send_cb(pipeline, bus_msg->type);
    if (ret.u.bus_msg_ret.error == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Failed to send callback to HAL for type = %d",
        bus_msg->type);
      return ret;
    }
  }
#endif
  mct_controller_set_thread_state(mct, MCT_THREAD_STATE_IDLE);
  return ret;
}

/** Name: mct_controller_send_superparam:
 *  Arguments/Fields:
 *    @pipeline: structure of mct_controller_t
 *    @parm_stream: Session stream on which super events are sent
 *  Description:
 *     Routine to process super-param events in pipeline queue.
 *     It de-queues super-params based on the per-frame-control logic
 *     and sends them to downstream modules while recording frame number
 *     mapping and notifying HAL when queue becomes empty.
 **/

static boolean mct_controller_send_superparam(mct_pipeline_t *pipeline,
  mct_stream_t *parm_stream)
{
  boolean ret = TRUE;
  mct_event_super_control_parm_t *super_event = NULL;
  uint32_t i, queue_length =0;

  if (!pipeline || !parm_stream) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr detected: pipeline [%p], parm_stream [%p]",
      pipeline, parm_stream);
    return FALSE;
  }

  //previous book-keeping;
  uint32_t overall_delay = pipeline->session_data.max_pipeline_frame_applying_delay +
    pipeline->session_data.max_pipeline_meta_reporting_delay;

  uint32_t prev_frame_id = (pipeline->max_ack_id < overall_delay)?
                          pipeline->max_ack_id:pipeline->max_ack_id - overall_delay;
  if (parm_stream->isp_kstate_ptr
    && (((parm_stream->current_frame_id < parm_stream->isp_kstate_ptr->kernel_sofid)
    || (parm_stream->isp_kstate_ptr->kernel_sofid
    && (prev_frame_id >= parm_stream->isp_kstate_ptr->kernel_sofid)))
    ||  parm_stream->isp_kstate_ptr->drop_reconfig)) {
    CLOGI(CAM_MCT_MODULE, "Skipping sof (current_frame, kernel_frame)= [%d, %d] drop_reconfig %u\n",
      parm_stream->current_frame_id, parm_stream->isp_kstate_ptr->kernel_sofid,
      parm_stream->isp_kstate_ptr->drop_reconfig);
     return TRUE;
   }

  if(pipeline->delay_dequeue_count)
  {
    CLOGI(CAM_MCT_MODULE, "Bypass super-param settings on this frame");
    pipeline->delay_dequeue_count--;
  } else {
    /* First send out super param settings if any */
    /* if HAL1 send all superparams in queue */
    do {
      pthread_mutex_lock(&pipeline->super_param_q_lock);
      super_event = (mct_event_super_control_parm_t *)
        mct_queue_pop_head(pipeline->super_param_queue);
      queue_length = pipeline->super_param_queue->length;

      pthread_mutex_unlock(&pipeline->super_param_q_lock);
      if(super_event) {
        /* Unblock HAL to send new super params
        when queue becomes empty */
        if (0 == queue_length) {
          if (pipeline->hal_version != CAM_HAL_V1) {
            ret &= mct_controller_notify_hal_internal_event(pipeline,
              CAM_EVENT_TYPE_DAEMON_PULL_REQ);
          }
        }

        /* Book-keep frame number for reference */
        if (pipeline->hal_version == CAM_HAL_V3 &&
          super_event->frame_number != UINT32_MAX) {
          ret &= mct_stream_map_frame_number(pipeline,
             super_event->frame_number, parm_stream->current_frame_id);
        }
        /*Apply super param settings to all modules */
        ret &= mct_stream_send_super_event(parm_stream, super_event);

        for (i = 0; i < super_event->num_of_parm_events; i++) {
          if (super_event->parm_events[i].parm_data) {
            free(super_event->parm_events[i].parm_data);
            super_event->parm_events[i].parm_data = NULL;
          }
        }
        if (super_event->parm_events) {
          free(super_event->parm_events);
          super_event->parm_events = NULL;
        }
        free(super_event);
        super_event = NULL;
      } else {
        break;
      }
      if(CAM_HAL_V3 == pipeline->hal_version) {
        break;
      }
    } while (1);
  }
  return ret;
}

static boolean mct_controller_send_sleep_awake(mct_pipeline_t *pipeline,
  mct_stream_t *parm_stream)
{
  boolean rc = TRUE;
  mct_bus_msg_ctrl_request_frame_t ctrl;

  if(!pipeline || !parm_stream) {
    CLOGE(CAM_MCT_MODULE, "Pipeline or Stream NULL %p  %p", pipeline, parm_stream);
    return FALSE;
  }

  if (pipeline->aux_cam) {
    if (pipeline->lpm_owner && pipeline->hal_version == CAM_HAL_V3) {
      if (parm_stream->current_frame_id > parm_stream->hw_sleep_frame_id) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.request_flag = FALSE;
        ctrl.req_mode = FRM_REQ_MODE_NONE;
        ctrl.lpm_req_bit = 0;
        rc = mct_pipeline_control_hw_state(pipeline, &ctrl);
        if (!rc) {
          CLOGE(CAM_MCT_MODULE, "Failed to set HW state for session %d",
            pipeline->session);
          return rc;
        }
        rc = mct_pipeline_send_module_sleep(pipeline);
        if (!rc) {
          CLOGE(CAM_MCT_MODULE, "Failed to send module to sleep session %d",
            pipeline->session);
        }
      }
    } else {
      if (parm_stream->current_frame_id > parm_stream->hw_sleep_frame_id) {
        if (pipeline->updated_hw_state != pipeline->module_hw_state) {
          if (pipeline->updated_hw_state == MCT_MODULE_STATE_AWAKE) {
            rc= mct_pipeline_send_module_wakeup(pipeline);
            if (!rc) {
              CLOGE(CAM_MCT_MODULE, "Failed to set HW state for session %d",
                pipeline->session);
              return rc;
            }
          } else if (pipeline->updated_hw_state == MCT_MODULE_STATE_ASLEEP) {
          rc = mct_pipeline_send_module_sleep(pipeline);
          if (!rc) {
            CLOGE(CAM_MCT_MODULE, "Failed to set HW state for session %d",
              pipeline->session);
            return rc;
          }
        }
       }
     }
    }
  }
  return rc;
}

/** mct_controller_handle_SOF_proc:
 *    @pipeline: structure of mct_controller_t
 *    @bus_msg: Incoming bus_msg to process
 * Media Controller SOF processing routine
 **/
static boolean mct_controller_handle_SOF_proc(
  mct_pipeline_t *pipeline, mct_bus_msg_t *bus_msg,
  mct_superparam_type_t send_with_params)
{
  boolean ret = TRUE;
  unsigned int current_sof_delay = 0;
  int delay = 0;
  mct_stream_t *parm_stream = NULL;
  mct_pipeline_get_stream_info_t info;
  mct_event_control_t event_data;
  mct_event_t cmd_event;
  mct_bus_msg_isp_sof_t *isp_sof_bus_msg = NULL;
  mct_stream_t *meta_stream = NULL;
  metadata_buffer_t *metadata_buffer = NULL;
  mct_bus_msg_ctrl_request_frame_t ctrl;
  uint32_t remap_start_id;

  CAMSCOPE_UPDATE_FLAGS(CAMSCOPE_SECTION_MMCAMERA, kpi_camscope_flags);

  if (!pipeline || !bus_msg) {
    CLOGE(CAM_MCT_MODULE, "Invalid ptr detected: pipeline: %p, bus_msg: %p",
      pipeline, bus_msg);
    return FALSE;
  }
  isp_sof_bus_msg = bus_msg->msg;
  /* Obtain session stream to issue commands on */
  if (MCT_PIPELINE_NUM_CHILDREN(pipeline) > 0) {
    info.check_type   = CHECK_INDEX;
    info.stream_index  = MCT_SESSION_STREAM_ID;
    parm_stream = mct_pipeline_get_stream(pipeline, &info);
    if (!parm_stream) {
      CLOGE(CAM_MCT_MODULE, "No session stream available at SOF");
      return FALSE;
    }
  } else {
    CLOGE(CAM_MCT_MODULE, "No child present for pipeline");
    return FALSE;
  }
  parm_stream->current_frame_id = isp_sof_bus_msg->frame_id;

  if (pipeline->hal_version == CAM_HAL_V3) {
    if (isp_sof_bus_msg->kstate_ptr &&
      MCT_REFCOUNT_GET(isp_sof_bus_msg->kstate_ptr, refc)) {
      parm_stream->isp_kstate_ptr = isp_sof_bus_msg->kstate_ptr->isp_kstate_ptr;
      if (parm_stream->isp_kstate_ptr) {
          if (parm_stream->current_frame_id > parm_stream->isp_kstate_ptr->kernel_sofid) {
            CLOGE(CAM_MCT_MODULE, "Invalid case ! bus_msg frame_id %d cannot be greater than kernel_sofid %d",
                parm_stream->current_frame_id, parm_stream->isp_kstate_ptr->kernel_sofid);
          } else {
            current_sof_delay = parm_stream->isp_kstate_ptr->kernel_sofid
             - parm_stream->current_frame_id;
          }
      }
    } else {
      parm_stream->isp_kstate_ptr = NULL;
      CLOGE(CAM_MCT_MODULE, "Invalid kstate ptr");
      return FALSE;
    }
  }

  if (pipeline->deferred_state == CAM_DEFER_START) {
    CLOGI(CAM_MCT_MODULE, "In defer state, processing the queue.");
    pipeline->deferred_state = CAM_DEFER_PROCESS;
    ret = mct_pipeline_process_defer_queue(pipeline);
  }
  /* Validate incoming frame id */
  if (!pipeline->int_streamoff) {
    ret = mct_bus_sof_tracker(pipeline->bus, parm_stream->current_frame_id);
    if (ret == FALSE) {
      goto error;
    }
  } else {
    pipeline->bus->prev_sof_id = 0;
  }
  /* No action currently taken on results of tracker */

  if (!isp_sof_bus_msg->regs_not_updated) {
    /* Send super-params queued in super_param_queue */
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_MCT_SUP_PARAMS);
    if ((!pipeline->int_streamoff) &&
       (MCT_SUPERPARAM_SINGLE_CAM == send_with_params)) {
      ret &= mct_controller_send_superparam(pipeline, parm_stream);
    }

    if ((!pipeline->int_streamoff) &&
       (MCT_SUPERPARAM_DC_W_PARAM == send_with_params)) {
      ret &= mct_controller_send_DC_superparam(pipeline, parm_stream);
    }
    ATRACE_CAMSCOPE_END(CAMSCOPE_MCT_SUP_PARAMS);
  }

  if ((isp_sof_bus_msg->frame_id) &&
      (pipeline->max_ack_id == isp_sof_bus_msg->frame_id) &&
      (pipeline->int_streamoff == 1))
    pipeline->start_int_streamoff_on = 1;

  /* Send any special events queued in param_queue */
  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_MCT_SPEC_EVT);
  ret &= mct_pipeline_send_special_event(pipeline, parm_stream);
  ATRACE_CAMSCOPE_END(CAMSCOPE_MCT_SPEC_EVT);

  /*Obtain the metadata stream */
  info.check_type   = CHECK_TYPE;
  info.stream_type  = CAM_STREAM_TYPE_METADATA;
  meta_stream = mct_pipeline_get_stream(pipeline, &info);

  if (!meta_stream) {
   CLOGE(CAM_MCT_MODULE, "Metadata stream not present");
   goto error;
  }

  if (meta_stream->state != MCT_ST_STATE_RUNNING) {
    CLOGE(CAM_MCT_MODULE, "Metadata stream is not running");
    goto error;
  }

  /* Get the metadata buffer before sending the control */
  /* SOF to other modules */
  ret &= mct_stream_get_metadata_buffer(meta_stream, &metadata_buffer);

  /* If Buffer present fill up the metadata bus_messages */
  /* already in bus_queue */
  if (meta_stream->metadata_stream.get_buf_err == FALSE) {
    ret &= mct_stream_update_metadata_buffer(meta_stream,
      metadata_buffer);
  }

  if (((pipeline->hal_version != CAM_HAL_V3) || (parm_stream->isp_kstate_ptr != NULL))
          && (pipeline->sof_delay || current_sof_delay)) {
    CLOGI(CAM_MCT_MODULE, "(sofdelay,curr_sofdelay) = (%d,%d) ksof %d, drop_reconf %u, sof %u",
      pipeline->sof_delay, current_sof_delay,
      (isp_sof_bus_msg->kstate_ptr != NULL && isp_sof_bus_msg->kstate_ptr->isp_kstate_ptr != NULL) ?
      isp_sof_bus_msg->kstate_ptr->isp_kstate_ptr->kernel_sofid : 0,
      (parm_stream->isp_kstate_ptr != NULL) ? parm_stream->isp_kstate_ptr->drop_reconfig: 0,
      isp_sof_bus_msg->frame_id);
  } else if ((pipeline->hal_version == CAM_HAL_V3)
     && (parm_stream->isp_kstate_ptr == NULL)) {
    CLOGE(CAM_MCT_MODULE, "kptr NULL");
  }

  if (isp_sof_bus_msg->regs_not_updated && !pipeline->sof_delay)
    delay = 1;
  /* Remap pending request if mct thread is delayed.*/
  if (current_sof_delay && current_sof_delay >= pipeline->sof_delay) {
    if (pipeline->sof_delay == 0) {
      delay = current_sof_delay;
    } else if(pipeline->sof_delay > current_sof_delay) {
      delay = pipeline->sof_delay - current_sof_delay + 1;
    } else {
      delay = current_sof_delay - pipeline->sof_delay + 1;
    }
  }

  remap_start_id = isp_sof_bus_msg->frame_id + 1;
  if (isp_sof_bus_msg->regs_not_updated ||
    (parm_stream->isp_kstate_ptr && parm_stream->isp_kstate_ptr->drop_reconfig)) {
    remap_start_id = isp_sof_bus_msg->frame_id;
  }
  isp_sof_bus_msg->remap_start_id = remap_start_id;

  if (parm_stream->isp_kstate_ptr && parm_stream->isp_kstate_ptr->drop_reconfig) {
     parm_stream->isp_kstate_ptr->drop_reconfig = 0;
     delay = delay ? delay: 1;
  }

  if (delay) {
    CLOGI(CAM_MCT_MODULE, "Remap pending request by %d, l=%d, c=%d, "
      "reg_update_fail=%d", delay, pipeline->sof_delay, current_sof_delay,
      isp_sof_bus_msg->regs_not_updated);
    mct_stream_update_all_frame_id_by(pipeline, delay, remap_start_id);
    isp_sof_bus_msg->tardy = delay;
  }

  pipeline->sof_delay = current_sof_delay;

  /*Then send the SOF event down to modules*/
  ATRACE_BEGIN_SNPRINTF(32, "CTRL_BCSOF %d", isp_sof_bus_msg->frame_id);
  ret &= mct_stream_send_ctrl_sof(parm_stream, isp_sof_bus_msg);
  ATRACE_END();

  if (ret == FALSE && meta_stream) {
    if (!mct_stream_put_metadata_buffer(meta_stream)) {
      CLOGE(CAM_MCT_MODULE, "Put_buf metadata to kernel Buf idx =%d failed",
        meta_stream->metadata_stream.current_buf_idx);
    }
  }

  /*Updating the sleep frame id, there are chances that it is stale */
  if (pipeline->state_change || pipeline->wake_mode == FRM_REQ_MODE_CONTINUOUS) {
    if (pipeline->aux_cam) {
      parm_stream->hw_sleep_frame_id = parm_stream->current_frame_id +
      pipeline->sleep_duration;
      pipeline->state_change = FALSE;
    }
  }
 /*Updating the sleep frame id, if there are any set param queued */
  if (pipeline->set_param_cnt > 0) {
     parm_stream->hw_sleep_frame_id = parm_stream->current_frame_id +
     pipeline->sleep_duration;
     pipeline->set_param_cnt--;
  }

  /*Set HW to sleep if HW active frame time has expired.*/
  mct_controller_send_sleep_awake(pipeline, parm_stream);

  error:
  if (parm_stream->isp_kstate_ptr)
    MCT_REFCOUNT_PUT(isp_sof_bus_msg->kstate_ptr, refc);

  return ret;
}


/** mct_controller_thread_run:
 *    @data: structure of mct_controller_t
 *
 * Media Controller Thread
 **/
static void* mct_controller_thread_run(void *data)
{
  mct_controller_t  *mct_this;
  mct_process_ret_t *mct_ret = NULL;
  mct_process_ret_t proc_ret;
  mct_serv_msg_t    *msg;

  mct_this = (mct_controller_t *)data;
  mct_this->mct_tid = pthread_self();

  /* signal condition variable */
  pthread_mutex_lock(&mct_this->mctl_thread_started_mutex);
  pthread_cond_signal(&mct_this->mctl_thread_started_cond);
  pthread_mutex_unlock(&mct_this->mctl_thread_started_mutex);

  do {
    pthread_mutex_lock(&mct_this->mctl_mutex);
    if (!mct_this->serv_cmd_q_counter) {
      pthread_cond_wait(&mct_this->mctl_cond, &mct_this->mctl_mutex);
    }
    pthread_mutex_unlock(&mct_this->mctl_mutex);
    /* Received Signal from HAL to process HAL/DS Message */

    while (1) {
      pthread_mutex_lock(&mct_this->serv_msg_q_lock);
      msg = (mct_serv_msg_t *)mct_queue_pop_head(mct_this->serv_cmd_q);
      pthread_mutex_unlock(&mct_this->serv_msg_q_lock);

      if (!msg) {
        break;
      }
      pthread_mutex_lock(&mct_this->mctl_mutex);
      mct_this->serv_cmd_q_counter--;
      pthread_mutex_unlock(&mct_this->mctl_mutex);

#ifndef DAEMON_PRESENT
      mct_ret = (mct_process_ret_t *)malloc (sizeof(mct_process_ret_t));
      if (!mct_ret) {
        CLOGE(CAM_MCT_MODULE, "Out of Memory, Malloc failed");
        goto close_mct;
      }
      *mct_ret = mct_controller_proc_serv_msg_internal(mct_this, msg);
      free(msg);
      msg = NULL;

      if (mct_ret->type == MCT_PROCESS_RET_SERVER_MSG  &&
        mct_ret->u.serv_msg_ret.msg.msg_type == SERV_MSG_HAL  &&
        mct_ret->u.serv_msg_ret.msg.u.hal_msg.id == MSM_CAMERA_DEL_SESSION) {
        free(mct_ret);
        mct_ret = NULL;
        goto close_mct;
      }

      if (mct_this->timedout_status) {
        CLOGE(CAM_MCT_MODULE, "Timedout: Status is already sent, dropping this ack");
        free(mct_ret);
        mct_ret = NULL;
        mct_this->timedout_status = FALSE;
        continue;
      }

      /* Send the response */
      pthread_mutex_lock(&mct_this->serv_res_cond_lock);
      mct_queue_push_tail(mct_this->serv_res_q, mct_ret);
      pthread_cond_signal(&mct_this->serv_res_cond);
      pthread_mutex_unlock(&mct_this->serv_res_cond_lock);

#else
      proc_ret = mct_controller_proc_serv_msg_internal(mct_this, msg);
      free(msg);
      msg = NULL;

      if (proc_ret.type == MCT_PROCESS_RET_SERVER_MSG  &&
        proc_ret.u.serv_msg_ret.msg.msg_type == SERV_MSG_HAL  &&
        proc_ret.u.serv_msg_ret.msg.u.hal_msg.id == MSM_CAMERA_DEL_SESSION) {
        goto close_mct;
      }
      /* Based on process result, need to send event to server */
      write(mct_this->serv_fd, &proc_ret, sizeof(mct_process_ret_t));
#endif

    }

  } while(1);
close_mct:
  return NULL;
}

/** Name: mct_controller_get_thread_state
 *
 *  Arguments/Fields:
 *    @mct: Pointer to mct_controller object
 *    @state: State of SOF_proc thread
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    Retreives current state of sof_processing thread.
 *
 **/
boolean mct_controller_get_thread_state (mct_controller_t *mct,
  mct_thread_state_type *state)
{
  if (!mct || !state) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr detected!mct: %p, state: %p", mct, state);
    return FALSE;
  }
  pthread_mutex_lock(&mct->mctl_state_mutex);
  *state = mct->bus_thread_state;
  pthread_mutex_unlock(&mct->mctl_state_mutex);

  return TRUE;
}

/** Name: mct_controller_set_thread_state
 *
 *  Arguments/Fields:
 *    @mct: Pointer to mct_controller object
 *    @state: Thread state to be assigned.
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    Sets current state of sof_processing thread.
 *    Additionally signals listeners when thread goes to IDLE state
 *
 **/
boolean mct_controller_set_thread_state (mct_controller_t *mct,
  mct_thread_state_type state)
{
  if (!mct) {
    CLOGE(CAM_MCT_MODULE, "NULL controller object!");
    return FALSE;
  } else {
    pthread_mutex_lock(&mct->mctl_state_mutex);
    mct->bus_thread_state = state;
    if (MCT_THREAD_STATE_IDLE == mct->bus_thread_state) {
      /* Signal the end of processing current SOF */
      pthread_cond_signal(&mct->mctl_state_cond);
    }
    pthread_mutex_unlock(&mct->mctl_state_mutex);
  }
  return TRUE;
}

/** mct_bus_handler_thread_run:
 *    @data: structure of mct_controller_t
 *
 * Media Controller Bus message processing Thread
 **/
static void* mct_bus_handler_thread_run(void *data)
{
  mct_process_ret_t  proc_ret;
  mct_bus_msg_t     *bus_msg;
  mct_controller_t* mct_obj = (mct_controller_t *)data;
  if(!mct_obj) {
    CLOGE(CAM_MCT_MODULE, "Invalid pointer for mct object");
    return NULL;
  }
  /* This worker thread is invoked at frame SOF.
  *  It offloads processing of HAL super_param settings
  *  and SOF processing from the main mct thread.
  */
  do {
    pthread_mutex_lock(&mct_obj->mctl_bus_handle_mutex);
    if(!mct_obj->pipeline->bus->priority_queue->length) {
      pthread_cond_wait(&mct_obj->mctl_bus_handle_cond,
        &mct_obj->mctl_bus_handle_mutex);
    }
    pthread_mutex_unlock(&mct_obj->mctl_bus_handle_mutex);

    /* Received Signal from Pipeline Bus */
    while (1) {
      pthread_mutex_lock(&mct_obj->pipeline->bus->priority_q_lock);
      bus_msg = (mct_bus_msg_t *)mct_queue_pop_head
        (mct_obj->pipeline->bus->priority_queue);
      pthread_mutex_unlock(&mct_obj->pipeline->bus->priority_q_lock);

      if (!bus_msg) {
        break;
      }
      if (bus_msg->type == MCT_BUS_MSG_CLOSE_CAM) {
        goto thread_exit;
      }
      proc_ret = mct_controller_proc_bus_msg_internal(mct_obj, bus_msg);

      if (bus_msg->msg) {
        free(bus_msg->msg);
        bus_msg->msg = NULL;
      }

      if (bus_msg) {
        free(bus_msg);
        bus_msg = NULL;
      }
#ifdef DAEMON_PRESENT
      if (proc_ret.type == MCT_PROCESS_RET_ERROR_MSG ||
          proc_ret.type == MCT_PROCESS_DUMP_INFO ||
          (proc_ret.type == MCT_PROCESS_RET_BUS_MSG &&
          proc_ret.u.bus_msg_ret.msg_type == MCT_BUS_MSG_CAC_STAGE_DONE) ||
          (proc_ret.type == MCT_PROCESS_RET_BUS_MSG &&
          proc_ret.u.bus_msg_ret.msg_type == MCT_BUS_MSG_EZTUNE_JPEG) ||
          (proc_ret.type == MCT_PROCESS_RET_BUS_MSG &&
          proc_ret.u.bus_msg_ret.msg_type == MCT_BUS_MSG_EZTUNE_RAW)) {
        write(mct_obj->serv_fd, &proc_ret, sizeof(mct_process_ret_t));
      }
#endif
    }
  } while(1);

thread_exit:
  if (bus_msg->msg) {
    free(bus_msg->msg);
    bus_msg->msg = NULL;
  }

  if (bus_msg) {
    free(bus_msg);
    bus_msg = NULL;
  }
  return NULL;
}


/** Name: mct_controller_send_DC_superparam:
 *  Arguments/Fields:
 *    @pipeline: structure of mct_controller_t
 *    @parm_stream: Session stream on which super events are sent
 *  Description:
 *     Routine to process super-param events in pipeline queue.
 *     It de-queues super-params based on the per-frame-control logic
 *     and sends them to downstream modules while recording frame number
 *     mapping and notifying HAL when queue becomes empty.
 **/

static boolean mct_controller_send_DC_superparam(mct_pipeline_t *pipeline,
  mct_stream_t *parm_stream)
{
  boolean ret = TRUE;
  mct_event_super_control_parm_t *super_event = NULL;
  uint32_t i, queue_length =0;

  if (!pipeline || !parm_stream) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr detected: pipeline [%p], parm_stream [%p]",
      pipeline, parm_stream);
    return FALSE;
  }

  //previous book-keeping;
  uint32_t overall_delay = pipeline->session_data.max_pipeline_frame_applying_delay +
    pipeline->session_data.max_pipeline_meta_reporting_delay;

  uint32_t prev_frame_id = (pipeline->max_ack_id < overall_delay)?
                          (pipeline->max_ack_id):(pipeline->max_ack_id - overall_delay);
  if (parm_stream->isp_kstate_ptr
   && (((parm_stream->current_frame_id < parm_stream->isp_kstate_ptr->kernel_sofid)
   || (parm_stream->isp_kstate_ptr->kernel_sofid
   && (prev_frame_id >= parm_stream->isp_kstate_ptr->kernel_sofid)))
   || parm_stream->isp_kstate_ptr->drop_reconfig
   || parm_stream->isp_kstate_ptr->dual_cam_drop_detected
   || parm_stream->isp_kstate_ptr->dual_cam_drop)) {
   CLOGI(CAM_MCT_MODULE, "Skipping sof (current_frame, kernel_frame)= [%d, %d] drop_reconfig %u \
   dual_cam_drop_detected %d dual_cam_drop %d\n",parm_stream->current_frame_id,
   parm_stream->isp_kstate_ptr->kernel_sofid, parm_stream->isp_kstate_ptr->drop_reconfig,
   parm_stream->isp_kstate_ptr->dual_cam_drop_detected,parm_stream->isp_kstate_ptr->dual_cam_drop);
   return TRUE;
   }

  if(pipeline->delay_dequeue_count)
  {
    CLOGI(CAM_MCT_MODULE, "Bypass super-param settings on this frame");
    pipeline->delay_dequeue_count--;
  } else {
    if(is_supermct_exist(pipeline))
    {
        supermct_lock_sof_update();
    }
    /* First send out super param settings if any */
    /* if HAL1 send all superparams in queue */
    do {
      pthread_mutex_lock(&pipeline->super_param_q_lock);
      super_event = (mct_event_super_control_parm_t *)
        mct_queue_look_at_head(pipeline->super_param_queue);

      queue_length = pipeline->super_param_queue->length;

      CLOGL(CAM_MCT_MODULE,"mct_queue_look_at_head queue_length=%d",
        queue_length);

      pthread_mutex_unlock(&pipeline->super_param_q_lock);
      if(super_event) {
        /* Unblock HAL to send new super params
        when queue becomes empty */
        if (0 == queue_length) {
          if (pipeline->hal_version != CAM_HAL_V1) {
            ret &= mct_controller_notify_hal_internal_event(pipeline,
              CAM_EVENT_TYPE_DAEMON_PULL_REQ);
          }
        }
        if ((pipeline->hal_version == CAM_HAL_V3)||((((super_event->applying_frame_id) &&
          (super_event->applying_frame_id <= parm_stream->current_frame_id)) ||
          ((super_event->applying_frame_id == 0) && (super_event->is_sync_param == 0))) &&
          (pipeline->hal_version == CAM_HAL_V1))) {

            pthread_mutex_lock(&pipeline->super_param_q_lock);
            super_event = (mct_event_super_control_parm_t *)
            mct_queue_pop_head(pipeline->super_param_queue);

          queue_length = pipeline->super_param_queue->length;
          pthread_mutex_unlock(&pipeline->super_param_q_lock);

          CLOGL(CAM_MCT_MODULE,"mct_queue_pop_head queue_length=%d",
            queue_length);

          /* Book-keep frame number for reference */
          if (pipeline->hal_version == CAM_HAL_V3 &&
            super_event->frame_number != UINT32_MAX) {
            ret &= mct_stream_map_frame_number(pipeline,
               super_event->frame_number, parm_stream->current_frame_id);
          }

          CLOGL(CAM_MCT_MODULE,"Frame_ids: Applying [%d] current [%d]",
            super_event->applying_frame_id, parm_stream->current_frame_id);
          CLOGL(CAM_MCT_MODULE,"sending super param [%p] frame_number [%d] is_sync_param [%d]",
            super_event, super_event->frame_number, super_event->is_sync_param);

          /* Apply super param settings to all modules */
          ret &= mct_stream_send_super_event(parm_stream, super_event);

          for (i = 0; i < super_event->num_of_parm_events; i++) {
            if (super_event->parm_events[i].parm_data) {
              free(super_event->parm_events[i].parm_data);
              super_event->parm_events[i].parm_data = NULL;
            }
          }
          if (super_event->parm_events) {
            free(super_event->parm_events);
            super_event->parm_events = NULL;
          }
          free(super_event);
          super_event = NULL;
        } else {
          break;
        }
      } else {
        break;
      }
      if(CAM_HAL_V3 == pipeline->hal_version) {
        break;
      }
    } while (1);
    if(is_supermct_exist(pipeline))
    {
        supermct_unlock_sof_update();
    }
  }
  return ret;
}
