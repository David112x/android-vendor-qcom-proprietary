/* q3a_thread_ext.c
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "q3a_thread_ext.h"
#include "q3a_thread.h"


/******************* Start global thread handlers *******************/
/**q3a_ext_thread_share_info_t
 *
 * @is_sharing: Is the algo in the thread been share.
 * @camera_id_sharing_algo: The algo instance from this camera_id port,
                                            is the one been use.
 **/
typedef struct {
  boolean is_sharing;
  uint8_t camera_id_sharing_algo;
} q3a_ext_thread_share_info_t;

/** q3a_ext_thread_info_t
 *
 * Save global thread information and status.
 *
 * @ptr: Thread instance reference
 * @is_valid: Thread is created or not
 * @ref_cnt: Number of clients to the thread
 * @is_thread_running: Is the thread running or already stopped
 * @camera_id_sharing_algo: This camera is sharing it's algo instance to all
 **/
typedef struct {
  void *ptr;
  boolean is_valid;
  uint8_t ref_cnt;
  boolean is_thread_running;
  q3a_ext_thread_share_info_t share;
} q3a_ext_thread_info_t;

/** q3a_ext_aecawb_port_data_t
 *
 * Save information about the port been use, this info will be used to restore
 * data as needed.
 *
 * For 3A thread extension: when new thread is created for the same 3A module,
 * it will save the original thread data in this structure.
 * In case of unlinking from the thread, ports could retrive the original Q3A thread
 * using saved here.
 *
 *    @is_valid_data:       If true, original thread has been saved and is available.
 *    @saved_thread_data:   Place holder of the thread queue context.
 *    @original_camera_id:   Camera id of the saved thread.
 *    @aec_algo_instance: Saved original/initial AEC algo instance for this port.
 *    @awb_algo_instance: Saved original/initial AWB algo instance for this port.
**/
typedef struct {
  boolean is_valid_data;
  uint8_t original_camera_id;
  q3a_thread_data_t *saved_thread_data;
  void *aec_algo_instance;
  void *awb_algo_instance;
} q3a_ext_aecawb_port_data_t;

/** q3a_ext_af_port_data_t
 *
 * Save information about the port been use, this info will be used to restore
 * data as needed.
 *
 * For 3A thread extension: when new thread is created for the same 3A module,
 * it will save the original thread data in this structure.
 * In case of unlinking from the thread, ports could retrive the original Q3A thread
 * using saved here.
 *
 *    @is_valid_data:       If true, original thread has been saved and is available.
 *    @saved_thread_data:   Place holder of the thread queue context.
 *    @original_camera_id:   Camera id of the saved thread.
 *    @af_algo_instance: Saved original/initial AWB algo instance for this port.
**/
typedef struct {
  boolean is_valid_data;
  uint8_t original_camera_id;
  q3a_thread_data_t *saved_thread_data;
  void *af_algo_instance;
} q3a_ext_af_port_data_t;

typedef struct {
  q3a_ext_thread_info_t ext_thread;
  q3a_ext_aecawb_port_data_t ext_port_data[MAX_3A_MULTI_CAMERA_ID];
  pthread_mutex_t cbMutex;
} q3a_ext_aecawb_thread_register_t;

typedef struct {
  q3a_ext_thread_info_t ext_thread;
  q3a_ext_af_port_data_t ext_port_data[MAX_3A_MULTI_CAMERA_ID];
  pthread_mutex_t cbMutex;
} q3a_ext_af_thread_register_t;

static pthread_once_t gThreadAecAwbInit = PTHREAD_ONCE_INIT;
q3a_ext_aecawb_thread_register_t gAECAWBThreadRegister;

static pthread_once_t gThreadAFInit = PTHREAD_ONCE_INIT;
q3a_ext_af_thread_register_t gThreadAFRegister;

/** q3a_thread_ext_aecawb_init_register:
 *
 * This function initalizes the mutex initalization of the call back registers
 **/
static void q3a_thread_ext_aecawb_init_register(){
  if (0 != pthread_mutex_init(&gAECAWBThreadRegister.cbMutex, NULL)) {
     Q3A_ERR("Failed to Initialize Callback Register");
  }
}

/** q3a_thread_ext_aecawb_initialize_register:
 *
 * This function can be called any no of times by any threads but
 * it ensures that the initialization happens only once
 *  Returns 0 on success and -error code  on failure
 **/
int q3a_thread_ext_aecawb_initialize_register(){
  return pthread_once(&gThreadAecAwbInit, q3a_thread_ext_aecawb_init_register);
}

/** q3a_thread_ext_af_init_register:
 *
 * This function initalizes the mutex initalization of the call back registers
 **/
static void q3a_thread_ext_af_init_register(){
  if(0 != pthread_mutex_init(&gThreadAFRegister.cbMutex, NULL)) {
     Q3A_ERR("Failed to Initialize Callback Register");
  }
}

/** q3a_thread_ext_af_initialize_register:
 *
 * This function can be calledn any no of times by any threads but
 * it ensures that the initialization happens only once
  *  Returns 0 on success and -error code  on failure
 **/
int q3a_thread_ext_af_initialize_register(){
  return pthread_once(&gThreadAFInit, q3a_thread_ext_af_init_register);
}

/******************* End global thread handlers *******************/


/** q3a_thread_ext_aecawb_get_thread_obj:
 *
 * Request and set AEC & AWB information needed for thread
 *
 * Return TRUE on success
 **/
boolean q3a_thread_ext_aecawb_get_thread_obj(mct_port_t *mct_port)
{
  boolean rc = FALSE;
  aec_port_private_t *aec_port = (aec_port_private_t *)(mct_port->port_private);
  uint8_t camera_id = gAECAWBThreadRegister.ext_thread.ref_cnt - 1;
  q3a_thread_aecawb_data_t *single_awbaec_thread =
    (q3a_thread_aecawb_data_t *)gAECAWBThreadRegister.ext_thread.ptr;
  mct_event_t event;
  q3a_thread_aecawb_get_port_data_t get_obj;

  if (!gAECAWBThreadRegister.ext_thread.is_valid) {
    Q3A_ERR("Fail get AEC/AWB data, invalid data");
    return FALSE;
  }
  Q3A_HIGH("Call AEC & AWB get_ext_thread");
  STATS_MEMSET(&get_obj, 0, sizeof(q3a_thread_aecawb_get_port_data_t));

  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = aec_port->reserved_id;
  event.u.module_event.current_frame_id = 0;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AECAWB_GET_THREAD_OBJECT;
  event.u.module_event.module_event_data = (void *)&get_obj;
  rc = MCT_PORT_EVENT_FUNC(mct_port)(mct_port, &event);
  if (FALSE == rc ||
      NULL == get_obj.camera.aec_obj || NULL == get_obj.camera.aec_obj->aec ||
      NULL == get_obj.camera.awb_obj || NULL == get_obj.camera.awb_obj->awb) {
    Q3A_ERR("Get AEC or AWB data fail: GET_THREAD_OBJECT");
    return rc;
  }

  /* Save port data */
  gAECAWBThreadRegister.ext_port_data[camera_id].is_valid_data = TRUE;
  gAECAWBThreadRegister.ext_port_data[camera_id].original_camera_id =
    get_obj.camera_id;
  gAECAWBThreadRegister.ext_port_data[camera_id].saved_thread_data =
    get_obj.thread_data;

  if (NULL == get_obj.camera.aec_obj) {
    Q3A_ERR("aec_obj is not updated");
  } else {
    gAECAWBThreadRegister.ext_port_data[camera_id].aec_algo_instance =
      get_obj.camera.aec_obj->aec;
  }
  if (NULL == get_obj.camera.awb_obj) {
    Q3A_ERR("awb_obj is not updated");
  } else {
    gAECAWBThreadRegister.ext_port_data[camera_id].awb_algo_instance =
      get_obj.camera.awb_obj->awb;
  }
  /* Save port reference in thread */
  single_awbaec_thread->camera[camera_id] = get_obj.camera;

  return rc;
}

/** q3a_thread_ext_aecawb_set_thread_obj:
 *
 * Request and set AEC & AWB information needed for thread
 *
 * Return TRUE on success
 **/
boolean q3a_thread_ext_aecawb_set_thread_obj(mct_port_t *mct_port)
{
  boolean rc = FALSE;
  aec_port_private_t *aec_port = (aec_port_private_t *)(mct_port->port_private);
  uint8_t camera_id = gAECAWBThreadRegister.ext_thread.ref_cnt - 1;
  q3a_thread_aecawb_data_t *single_awbaec_thread =
    (q3a_thread_aecawb_data_t *)gAECAWBThreadRegister.ext_thread.ptr;
  mct_event_t event;
  q3a_thread_aecawb_set_port_data_t set_obj;

  if (!gAECAWBThreadRegister.ext_thread.is_valid) {
    Q3A_ERR("Fail get AEC/AWB data, invalid data");
    return FALSE;
  }
  Q3A_HIGH("Call AEC & AWB set_ext_thread");
  set_obj.camera_id = camera_id;
  set_obj.awbaec_thread_data = single_awbaec_thread->thread_data;

  /* Assign algorithm instance to be use, for sharing and non-sharing mode */
  if (gAECAWBThreadRegister.ext_thread.share.is_sharing) {
    uint8_t share_camera_id =
      gAECAWBThreadRegister.ext_thread.share.camera_id_sharing_algo;
    set_obj.aec_algo_instance =
      gAECAWBThreadRegister.ext_port_data[share_camera_id].aec_algo_instance;
    set_obj.awb_algo_instance =
      gAECAWBThreadRegister.ext_port_data[share_camera_id].awb_algo_instance;
  } else {
    set_obj.aec_algo_instance =
      gAECAWBThreadRegister.ext_port_data[camera_id].aec_algo_instance;
    set_obj.awb_algo_instance =
      gAECAWBThreadRegister.ext_port_data[camera_id].awb_algo_instance;
  }

  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = aec_port->reserved_id;
  event.u.module_event.current_frame_id = 0;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AECAWB_SET_THREAD_OBJECT;
  event.u.module_event.module_event_data = (void *)&set_obj;
  rc = MCT_PORT_EVENT_FUNC(mct_port)(mct_port, &event);
  if (FALSE == rc) {
    Q3A_ERR("Get AEC or AWB data fail: SET_THREAD_OBJECT");
    return rc;
  }

  /* Mark as camera register compleated */
  single_awbaec_thread->thread_data->num_of_registered_cameras =
    gAECAWBThreadRegister.ext_thread.ref_cnt;

  return rc;
}

/** q3a_thread_ext_aecawb_release_aecawb_obj_ref:
 *
 * Request the release of reference from AEC &AWB to the current thread, instead use saved values.
 *
 * Return: void
 **/
void q3a_thread_ext_aecawb_release_aecawb_obj_ref(mct_port_t *mct_port)
{
  boolean rc = FALSE;
  aec_port_private_t *aec_port = (aec_port_private_t *)(mct_port->port_private);
  uint8_t camera_id = aec_port->camera_id;
  mct_event_t event;
  q3a_thread_restore_thread_data_t restore_thread_data;

  if (!gAECAWBThreadRegister.ext_port_data[camera_id].is_valid_data) {
    Q3A_HIGH("Thread data not valid, nothing to release");
    return;
  }

  restore_thread_data.camera_id =
    gAECAWBThreadRegister.ext_port_data[camera_id].original_camera_id;
  restore_thread_data.q3a_thread_data =
    gAECAWBThreadRegister.ext_port_data[camera_id].saved_thread_data;
  restore_thread_data.aec_algo_instance =
    gAECAWBThreadRegister.ext_port_data[camera_id].aec_algo_instance;
  restore_thread_data.awb_algo_instance =
    gAECAWBThreadRegister.ext_port_data[camera_id].awb_algo_instance;

  Q3A_HIGH("Call AEC & AWB release ext_thread, saved_aecawb_data_t: %p",
    restore_thread_data.q3a_thread_data);
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = aec_port->reserved_id;
  event.u.module_event.current_frame_id = 0;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AECAWB_RESTORE_EXT_THREAD_OBJECT;
  event.u.module_event.module_event_data = (void *)&restore_thread_data;
  rc = MCT_PORT_EVENT_FUNC(mct_port)(mct_port, &event);
  if (FALSE == rc) {
    Q3A_ERR("Get AEC or AWB data fail: AECAWB_RELEASE_EXT_THREAD_OBJECT");
  }

  return;
}

/** q3a_thread_ext_af_get_thread_obj:
 *
 * Request and set AF information needed for thread
 *
 * Return TRUE on success
 **/
boolean q3a_thread_ext_af_get_thread_obj(mct_port_t *mct_port)
{
  boolean rc = FALSE;
  af_port_private_t *af_port = (af_port_private_t *)(mct_port->port_private);
  uint8_t camera_id = gThreadAFRegister.ext_thread.ref_cnt - 1;
  q3a_thread_af_data_t *single_af_thread =
    (q3a_thread_af_data_t *)gThreadAFRegister.ext_thread.ptr;
  mct_event_t event;
  q3a_thread_af_get_port_data_t get_obj;

  if (!gThreadAFRegister.ext_thread.is_valid) {
    Q3A_ERR("Fail get AF data, invalid data");
    return FALSE;
  }
  Q3A_HIGH("Call AF get_ext_thread");
  STATS_MEMSET(&get_obj, 0, sizeof(q3a_thread_af_get_port_data_t));

  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = af_port->reserved_id;
  event.u.module_event.current_frame_id = 0;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AF_GET_THREAD_OBJECT;
  event.u.module_event.module_event_data = (void *)&get_obj;
  rc = MCT_PORT_EVENT_FUNC(mct_port)(mct_port, &event);
  if (FALSE == rc ||
      NULL == get_obj.camera.af_obj || NULL == get_obj.camera.af_obj->af) {
    Q3A_ERR("Get AF data fail: GET_THREAD_OBJECT");
    return rc;
  }

  /* Save port data */
  gThreadAFRegister.ext_port_data[camera_id].is_valid_data = TRUE;
  gThreadAFRegister.ext_port_data[camera_id].original_camera_id =
    get_obj.camera_id;
  gThreadAFRegister.ext_port_data[camera_id].saved_thread_data =
    get_obj.thread_data;

  if (NULL == get_obj.camera.af_obj) {
    Q3A_ERR("af_obj is not updated");
  } else {
    gThreadAFRegister.ext_port_data[camera_id].af_algo_instance =
      get_obj.camera.af_obj->af;
  }

  /* Save port reference in thread */
  single_af_thread->camera[camera_id] = get_obj.camera;

  return rc;
}

/** q3a_thread_ext_af_set_thread_obj:
 *
 * Request and set AF information needed for thread
 *
 * Return TRUE on success
 **/
boolean q3a_thread_ext_af_set_thread_obj(mct_port_t *mct_port)
{
  boolean rc = FALSE;
  af_port_private_t *af_port = (af_port_private_t *)(mct_port->port_private);
  uint8_t camera_id = gThreadAFRegister.ext_thread.ref_cnt - 1;
  q3a_thread_af_data_t *single_af_thread =
    (q3a_thread_af_data_t *)gThreadAFRegister.ext_thread.ptr;
  mct_event_t event;
  q3a_thread_af_set_port_data_t set_obj;

  if (!gThreadAFRegister.ext_thread.is_valid) {
    Q3A_ERR("Fail get AF data, invalid data");
    return FALSE;
  }
  Q3A_HIGH("Call AF set_ext_thread");
  set_obj.camera_id = camera_id;
  set_obj.af_thread_data = single_af_thread->thread_data;

  /* Assign algorithm instance to be use, for sharing and non-sharing mode */
  if (gThreadAFRegister.ext_thread.share.is_sharing) {
    uint8_t share_camera_id =
      gThreadAFRegister.ext_thread.share.camera_id_sharing_algo;
    set_obj.af_algo_instance =
      gThreadAFRegister.ext_port_data[share_camera_id].af_algo_instance;
  } else {
    set_obj.af_algo_instance =
      gThreadAFRegister.ext_port_data[camera_id].af_algo_instance;
  }

  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = af_port->reserved_id;
  event.u.module_event.current_frame_id = 0;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AF_SET_THREAD_OBJECT;
  event.u.module_event.module_event_data = (void *)&set_obj;
  rc = MCT_PORT_EVENT_FUNC(mct_port)(mct_port, &event);
  if (FALSE == rc) {
    Q3A_ERR("Get AF data fail: SET_THREAD_OBJECT");
    return rc;
  }

  /* Mark as camera register compleated */
  single_af_thread->thread_data->num_of_registered_cameras =
    gThreadAFRegister.ext_thread.ref_cnt;

  return rc;
}

/** q3a_thread_ext_af_release_af_obj_ref:
 *
 * Request the release of reference from AF to the current thread, instead use saved values.
 *
 * Return: void
 **/
void q3a_thread_ext_af_release_af_obj_ref(mct_port_t *mct_port)
{
  boolean rc = FALSE;
  af_port_private_t *af_port = (af_port_private_t *)(mct_port->port_private);
  uint8_t camera_id = af_port->camera_id;
  mct_event_t event;
  q3a_thread_restore_thread_data_t restore_thread_data;

  if (!gThreadAFRegister.ext_port_data[camera_id].is_valid_data) {
    Q3A_HIGH("Thread data not valid, nothing to release");
    return;
  }

  restore_thread_data.camera_id =
    gThreadAFRegister.ext_port_data[camera_id].original_camera_id;
  restore_thread_data.q3a_thread_data =
    gThreadAFRegister.ext_port_data[camera_id].saved_thread_data;
  restore_thread_data.af_algo_instance =
    gThreadAFRegister.ext_port_data[camera_id].af_algo_instance;

  Q3A_HIGH("Call AF release ext_thread, saved_af_data_t: %p",
    restore_thread_data.q3a_thread_data);
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = af_port->reserved_id;
  event.u.module_event.current_frame_id = 0;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AF_RESTORE_EXT_THREAD_OBJECT;
  event.u.module_event.module_event_data = (void *)&restore_thread_data;
  rc = MCT_PORT_EVENT_FUNC(mct_port)(mct_port, &event);
  if (FALSE == rc) {
    Q3A_ERR("Get AF data fail: AF_RELEASE_EXT_THREAD_OBJECT");
  }

  return;
}

/** ext_thread_aecawb_handler
 *    @aecawb_data: The pointer to the aecawb thread data
 *
 *  This is the aecawb thread that will run until it receives the STOP message.
 *  While running, it will dequeue messages from the thread's queue and process
 *  them. If there are no messages to process (queue is empty), the thread will
 *  sleep until it gets signaled.
 *
 *  Return NULL
 **/
static void* q3a_thread_ext_aecawb_thread_handler(void *aecawb_data)
{
  q3a_thread_aecawb_data_t *aecawb = (q3a_thread_aecawb_data_t *)aecawb_data;
  q3a_thread_aecawb_msg_t  *msg = NULL;
  q3a_thread_aecawb_camera_data_t *camera_aecawb = NULL;
  q3a_thread_ctrl_t *thread_ctrl = NULL;
  void *aec_algo_obj = NULL;
  void *awb_algo_obj = NULL;
  aec_object_t *aec_ops = NULL;
  awb_ops_t awb_ops;
  int                      exit_flag = 0;
  int                      rc = 0;
  uint8_t                  camera_id = 0;
  uint8_t                  num_of_cams = 0;

  Q3A_HIGH("Starting AECAWB thread handler");

  aecawb->thread_data->active = 1;
  sem_post(&aecawb->thread_data->sem_launch);

  do {
    Q3A_LOW("Waiting for message");
    pthread_mutex_lock(&aecawb->thread_data->thread_mutex);
    while ((aecawb->thread_data->msg_q->length == 0) &&
      (aecawb->thread_data->p_msg_q->length == 0)) {
      pthread_cond_wait(&aecawb->thread_data->thread_cond,
        &aecawb->thread_data->thread_mutex);
    }
    pthread_mutex_unlock(&aecawb->thread_data->thread_mutex);

    /* Get the message */
    pthread_mutex_lock(&aecawb->thread_data->msg_q_lock);
    /*Pop from priority queue first and if its empty pop from normal queue*/
    msg = (q3a_thread_aecawb_msg_t *)
      mct_queue_pop_head(aecawb->thread_data->p_msg_q);

    if (!msg) {
      msg = (q3a_thread_aecawb_msg_t *)
        mct_queue_pop_head(aecawb->thread_data->msg_q);
    }
    pthread_mutex_unlock(&aecawb->thread_data->msg_q_lock);
    if (!msg) {
      Q3A_ERR(" msg NULL");
      continue;
    }

    /* Get corresponding camera data short references */
    camera_id = msg->camera_id;
    camera_aecawb = &aecawb->camera[camera_id];
    thread_ctrl = &aecawb->thread_data->thread_ctrl[camera_id];
    num_of_cams = aecawb->thread_data->num_of_registered_cameras;
    aec_algo_obj = aecawb->camera[camera_id].aec_obj->aec;
    awb_algo_obj = aecawb->camera[camera_id].awb_obj->awb;
    aec_ops = aecawb->camera[camera_id].aec_obj;
    awb_ops = aecawb->camera[camera_id].awb_obj->awb_ops;


    /* Flush the queue if it is stopping. Free the enqueued messages and
     * signal the sync message owners to release their resources */
    if (aecawb->thread_data->active == 0 || num_of_cams > MAX_3A_MULTI_CAMERA_ID) {
      if (msg->type != MSG_STOP_THREAD) {
        if (msg->sync_flag == TRUE) {
          sem_post(&msg->sync_obj->msg_sem);
          /* Don't free msg, the sender will do */
          msg = NULL;
        } else {
          /* ACK the unused the STATS buffer from ISP */
          switch (msg->type) {
            case MSG_BG_AEC_STATS:
            case MSG_BE_AEC_STATS:
            case MSG_HDR_BE_AEC_STATS:
              camera_aecawb->aec_stats_cb(camera_aecawb->aec_port, msg->u.stats);
              break;
            case MSG_BG_AWB_STATS:
              camera_aecawb->awb_stats_cb(camera_aecawb->awb_port, msg->u.stats);
              /* For offline stats processing, we may need to post semaphore
                 so that mediacontroller thread won't block waiting for semaphore.
                 Since sem_post is handled in offline awb callback, we'll call
                 callback with dummy output so that we do sem_post before flushing
                 this message. */
              if (Q3A_STATS_STREAM_OFFLINE == msg->u.stats->isp_stream_type) {
                awb_output_data_t output;
                STATS_MEMSET(&output, 0, sizeof(awb_output_data_t));
                output.type = AWB_UPDATE_OFFLINE;
                camera_aecawb->awb_cb(&output, camera_aecawb->awb_port);
              }
              break;
            default:
              break;
          }

          /* Free memory allocated by caller inside the message */
          q3a_thread_aecawb_free_msg(&msg);
        }
        continue;
      }
    }

    /* Process message accordingly */
    Q3A_LOW("cam:%d: wake up type=%d, flag=%d", msg->camera_id, msg->type, msg->sync_flag);
    switch (msg->type) {
    case MSG_AEC_SET: {
      if(aec_ops->set_parameters){
        aec_ops->set_parameters(&msg->u.aec_set_parm, NULL, 0, aec_algo_obj);
      } else {
        Q3A_ERR(" Error: set_parameters is null");
      }
    }
      break;

    case MSG_AEC_GET: {
      if (aec_ops->get_parameters) {
        aec_ops->get_parameters(&msg->u.aec_get_parm, aec_algo_obj);
      }
    }
      break;

    case MSG_AEC_STATS: {
        // TODO: shall remove the legacy yuv handling in port
    }
      break;

    case MSG_BE_AEC_STATS:
    case MSG_HDR_BE_AEC_STATS:
    case MSG_AEC_STATS_HDR:
    case MSG_BG_AEC_STATS: {
      /* Send local output copy to avoid reset of custom data
         and map custom parameters data in local output variable */
      aec_output_data_t output_arr[MAX_3A_MULTI_CAMERA_ID];
      uint8_t cam_cnt = 0;
      STATS_MEMSET(output_arr, 0, MAX_3A_MULTI_CAMERA_ID*sizeof(aec_output_data_t));
      for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
        output_arr[cam_cnt].aec_custom_param =
          aecawb->camera[cam_cnt].aec_obj->output.aec_custom_param;
        output_arr[cam_cnt].type = AEC_OUTPUT_UPDATE_PENDING;
      }

      if ((thread_ctrl->aec_bg_be_stats_cnt < 3) ||
        (msg->type == MSG_AEC_STATS_HDR)) {
        if (!thread_ctrl->no_stats_mode) {
          ATRACE_BEGIN("Camera:AEC");
          rc = aec_ops->process(msg->u.stats, aec_algo_obj,
            output_arr, num_of_cams);
          ATRACE_END();

          for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
            /* Copy back custom data, in case it was deleted */
            output_arr[cam_cnt].aec_custom_param =
              aecawb->camera[cam_cnt].aec_obj->output.aec_custom_param;
            aecawb->camera[cam_cnt].aec_obj->output = output_arr[cam_cnt];

            /* Verify if update is requied */
            if (output_arr[cam_cnt].type != AEC_OUTPUT_UPDATE_PENDING) {
              aecawb->camera[cam_cnt].aec_obj->output = output_arr[cam_cnt];
              aecawb->camera[cam_cnt].aec_cb(&(aecawb->camera[cam_cnt].aec_obj->output),
                aecawb->camera[cam_cnt].aec_port);
            }
          }
        } else {
          rc = TRUE;
          Q3A_HIGH("  no_stats_mode awb");
        }
      }

      if (msg->type == MSG_BG_AEC_STATS ||
        msg->type == MSG_BE_AEC_STATS ||
        msg->type == MSG_HDR_BE_AEC_STATS) {
        camera_aecawb->aec_stats_cb(camera_aecawb->aec_port, msg->u.stats);
      }

      if (msg->u.stats) {
        switch (msg->type) {
          case MSG_AEC_STATS:
          case MSG_BG_AEC_STATS:
          case MSG_BE_AEC_STATS:
          case MSG_HDR_BE_AEC_STATS:
            /* Stats used by AEC with buffer allocated by ISP */
            if (thread_ctrl->aec_bg_be_stats_cnt) {
              pthread_mutex_lock(&aecawb->thread_data->msg_q_lock);
              thread_ctrl->aec_bg_be_stats_cnt--;
              pthread_mutex_unlock(&aecawb->thread_data->msg_q_lock);
            }
            break;
          default:
            break;
        }
      }
    }
      break;

    case MSG_AWB_SEND_EVENT: {
      /* Send local output copy to avoid reset of custom data
         and map custom parameters data in local output variable */
      awb_output_data_t output_arr[MAX_3A_MULTI_CAMERA_ID];
      uint8_t cam_cnt = 0;
      STATS_MEMSET(output_arr, 0, MAX_3A_MULTI_CAMERA_ID*sizeof(awb_output_data_t));
      for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
        output_arr[cam_cnt].awb_custom_param =
          aecawb->camera[cam_cnt].awb_obj->output.awb_custom_param;
        output_arr[cam_cnt].type = AWB_OUTPUT_UPDATE_PENDING;
      }

      if (awb_ops.set_parameters) {
        /* Query AWB output */
        msg->type = MSG_AWB_SET;
        awb_ops.set_parameters(&msg->u.awb_set_parm,
          output_arr, num_of_cams, awb_algo_obj);

        for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
          /* Copy back custom data, in case it was deleted */
          output_arr[cam_cnt].awb_custom_param =
            aecawb->camera[cam_cnt].awb_obj->output.awb_custom_param;

          /* Verify if update is requied */
          if (output_arr[cam_cnt].type != AWB_OUTPUT_UPDATE_PENDING) {
            output_arr[cam_cnt].type = AWB_SEND_OUTPUT_EVENT;
            aecawb->camera[cam_cnt].awb_obj->output = output_arr[cam_cnt];
            aecawb->camera[cam_cnt].awb_cb(&output_arr[cam_cnt],
              aecawb->camera[cam_cnt].awb_port);
          }
        }
      }
    }
      break;

    case MSG_AEC_SEND_EVENT: {
      /* Send local output copy to avoid reset of custom data
         and map custom parameters data in local output variable */
      aec_output_data_t output_arr[MAX_3A_MULTI_CAMERA_ID];
      uint8_t cam_cnt = 0;
      STATS_MEMSET(output_arr, 0, MAX_3A_MULTI_CAMERA_ID*sizeof(aec_output_data_t));

      for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
        output_arr[cam_cnt].aec_custom_param =
          camera_aecawb->aec_obj->output.aec_custom_param;
        output_arr[cam_cnt].type = AEC_OUTPUT_UPDATE_PENDING;
      }

      if (aec_ops->set_parameters) {
        /* Query AEC output and sent it out*/
        msg->type = MSG_AEC_SET;
        aec_ops->set_parameters(&msg->u.aec_set_parm,
          output_arr, num_of_cams, aec_algo_obj);

        for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
          /* Copy back custom data, in case it was deleted */
          output_arr[cam_cnt].aec_custom_param =
            aecawb->camera[cam_cnt].aec_obj->output.aec_custom_param;

          /* Verify if update is requied */
          if (output_arr[cam_cnt].type != AEC_OUTPUT_UPDATE_PENDING) {
            aecawb->camera[cam_cnt].aec_cb(&output_arr[cam_cnt],
              aecawb->camera[cam_cnt].aec_port);
          }
        }

      }
    }
      break;

    case MSG_AWB_SET: {
      if (awb_ops.set_parameters) {
        awb_ops.set_parameters(&msg->u.awb_set_parm, NULL, 0, awb_algo_obj);
      }
    }
      break;
    case MSG_AWB_GET: {
      awb_ops.get_parameters(&msg->u.awb_get_parm, awb_algo_obj);
    }
      break;

    case MSG_BG_AWB_STATS: {
      /* Send local output copy to avoid reset of custom data
         and map custom parameters data in local output variable */
      awb_output_data_t output_arr[MAX_3A_MULTI_CAMERA_ID];
      uint8_t cam_cnt = 0;
      awb_output_type_t awb_overwrite_output_type = AWB_OUTPUT_UPDATE_PENDING;

      STATS_MEMSET(output_arr, 0, MAX_3A_MULTI_CAMERA_ID*sizeof(awb_output_data_t));
      for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
        output_arr[cam_cnt].awb_custom_param =
          camera_aecawb->awb_obj->output.awb_custom_param;
        output_arr[cam_cnt].type = AWB_OUTPUT_UPDATE_PENDING;
     }

      /* For offline stats processing */
      if (Q3A_STATS_STREAM_OFFLINE == msg->u.stats->isp_stream_type) {
        awb_ops.process(msg->u.stats, awb_algo_obj, output_arr, num_of_cams);
        awb_overwrite_output_type = AWB_UPDATE_OFFLINE;
      } else {
        if (thread_ctrl->awb_bg_stats_cnt < 3) {
          if (!thread_ctrl->no_stats_mode) {
            ATRACE_BEGIN("Camera:AWB");
            awb_ops.process(msg->u.stats, awb_algo_obj, output_arr, num_of_cams);
            ATRACE_END();
            awb_overwrite_output_type = AWB_UPDATE;
          } else {
            Q3A_HIGH("no_stats_mode awb");
          }
        }
      }

      for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
        /* Copy back custom data, in case it was deleted */
        output_arr[cam_cnt].awb_custom_param =
          aecawb->camera[cam_cnt].awb_obj->output.awb_custom_param;
        aecawb->camera[cam_cnt].awb_obj->output = output_arr[cam_cnt];

        /* Verify if update is requied */
        if (output_arr[cam_cnt].type != AWB_OUTPUT_UPDATE_PENDING) {
          output_arr[cam_cnt].type = awb_overwrite_output_type;
          aecawb->camera[cam_cnt].awb_obj->output = output_arr[cam_cnt];
          aecawb->camera[cam_cnt].awb_cb(&(aecawb->camera[cam_cnt].awb_obj->output),
            aecawb->camera[cam_cnt].awb_port);
        }
      }

      camera_aecawb->awb_stats_cb(camera_aecawb->awb_port, msg->u.stats);
      if (thread_ctrl->awb_bg_stats_cnt) {
        pthread_mutex_lock(&aecawb->thread_data->msg_q_lock);
        thread_ctrl->awb_bg_stats_cnt--;
        pthread_mutex_unlock(&aecawb->thread_data->msg_q_lock);
      }

    }
      break;

   case MSG_STOP_THREAD: {
     exit_flag = 1;
   }
     break;

    default: {
    }
      break;
    } /* end switch (msg->type) */
    if (msg->sync_flag == TRUE) {
      sem_post(&msg->sync_obj->msg_sem);
      /*don't free msg, the sender will do*/
      msg = NULL;
    } else {
      q3a_thread_aecawb_free_msg(&msg);
    }
  } while (!exit_flag);

  return NULL;
} /* aecawb_thread_handler */

static boolean q3a_thread_ext_aecawb_create(
  q3a_thread_aecawb_data_t *thread_aecawb_data, mct_port_t *port)
{
  boolean rc = TRUE;
  aec_port_private_t *aec_port = (aec_port_private_t *)(port->port_private);

  if (NULL == thread_aecawb_data) {
    Q3A_ERR("NULL ext data");
    return FALSE;
  }

  rc = q3a_thread_aecawb_start(thread_aecawb_data, q3a_thread_ext_aecawb_thread_handler,
    "CAM_EXT_AECAWB");

  return rc;
}

boolean q3a_thread_ext_aecawb_create_single_thread(mct_port_t *mct_port)
{
  boolean rc = FALSE;
  boolean is_lock_acquired = FALSE;
  q3a_thread_aecawb_data_t *thread_aecawb_data = NULL;
  aec_port_private_t *aec_port = (aec_port_private_t *)(mct_port->port_private);
  int test_dual_out = 0;

  do {
    if (0 != pthread_mutex_lock(&gAECAWBThreadRegister.cbMutex)) {
       Q3A_ERR("Failed to Lock Mutex for thread register");
      break;
    }
    is_lock_acquired = TRUE;
    if (gAECAWBThreadRegister.ext_thread.ref_cnt >= MAX_3A_MULTI_CAMERA_ID) {
      Q3A_ERR("Reach max limit of supported cameras: %d", MAX_3A_MULTI_CAMERA_ID);
      FALSE == rc;
      break;
    }

    Q3A_HIGH("AECAWB: Camera: %u, session_id: %u",
      gAECAWBThreadRegister.ext_thread.ref_cnt,
      GET_SESSION_ID(aec_port->reserved_id));
    /* Verify if thread as been created */
    if (FALSE == gAECAWBThreadRegister.ext_thread.is_valid) {/*Create thread*/
      /* Allocate resources for the new thread */
      thread_aecawb_data = q3a_thread_aecawb_init();
      if (NULL == thread_aecawb_data) {
        Q3A_ERR("Fail to init ext thread");
        break;
      }

      rc = q3a_thread_ext_aecawb_create(thread_aecawb_data, mct_port);
      if (FALSE == rc) {
        Q3A_ERR("Fail to create ext aecawb thread");
        break;
      }

      /* Set sharing configuration */
      gAECAWBThreadRegister.ext_thread.share.is_sharing = TRUE;
      gAECAWBThreadRegister.ext_thread.share.camera_id_sharing_algo = 0;

      /* Test */
      STATS_TEST_DUAL_OUTPUT(test_dual_out);
      if (test_dual_out) {
        int share_aecawb_algo = 0;
        STATS_TEST_SHARE_AECAWB_ALGO(share_aecawb_algo);
        gAECAWBThreadRegister.ext_thread.share.is_sharing = share_aecawb_algo;
      }

      gAECAWBThreadRegister.ext_thread.ptr = thread_aecawb_data;
      gAECAWBThreadRegister.ext_thread.is_valid = TRUE;
      gAECAWBThreadRegister.ext_thread.is_thread_running = TRUE;
      gAECAWBThreadRegister.ext_thread.ref_cnt++;
    }else {
      /* Register in the already created thread */
      gAECAWBThreadRegister.ext_thread.ref_cnt++;
      rc = TRUE; /* not an error */

      Q3A_HIGH("AECAWB: Thread already created: %p, increase ref_cnt = %d",
        gAECAWBThreadRegister.ext_thread.ptr,
        gAECAWBThreadRegister.ext_thread.ref_cnt);
    }
  }while(0);
  if (is_lock_acquired) {
    Q3A_HIGH("AECAWB: camera_id: %u, is_sharing: %u, camera_id_sharing_algo: %d",
      gAECAWBThreadRegister.ext_thread.ref_cnt - 1,
      gAECAWBThreadRegister.ext_thread.share.is_sharing,
      gAECAWBThreadRegister.ext_thread.share.camera_id_sharing_algo);
    if (FALSE == rc && thread_aecawb_data) {
      Q3A_ERR("Handling error: de-init ext thread");
      q3a_thread_aecawb_deinit(thread_aecawb_data);
    }

    if (0 != pthread_mutex_unlock(&gAECAWBThreadRegister.cbMutex)) {
      Q3A_ERR("Failed to Unlock Mutex for ext Thread Register");
    }
  }
  return rc;
}

void q3a_thread_ext_aecawb_destroy_single_thread()
{
  boolean rc = FALSE;
  boolean is_lock_acquired = FALSE;
  q3a_thread_aecawb_data_t *thread_aecawb = NULL;
  do {
    if (0 != pthread_mutex_lock(&gAECAWBThreadRegister.cbMutex)) {
       Q3A_ERR("Failed to Lock Mutex for thread register");
      break;
    }
    is_lock_acquired = TRUE;
    if (FALSE == gAECAWBThreadRegister.ext_thread.is_valid) {
      Q3A_HIGH("Warning thread is already gone!");
      break;
    }
    gAECAWBThreadRegister.ext_thread.ref_cnt--;
    thread_aecawb = (q3a_thread_aecawb_data_t *)gAECAWBThreadRegister.ext_thread.ptr;
    thread_aecawb->thread_data->num_of_registered_cameras--;

    Q3A_HIGH("ref_cnt: %d", gAECAWBThreadRegister.ext_thread.ref_cnt);
    if (gAECAWBThreadRegister.ext_thread.is_thread_running) {
      Q3A_HIGH("Not the last instance, but stop thread now!");
      rc = q3a_thread_aecawb_stop(thread_aecawb);
      if (FALSE == rc) {
        Q3A_ERR("Fail to stop ext thread");
        break;
      }
      gAECAWBThreadRegister.ext_thread.is_thread_running = FALSE;
    }

    /* Verify if last reference */
    if (0 == gAECAWBThreadRegister.ext_thread.ref_cnt) {
      Q3A_HIGH("Last instance: De-init thread");
      q3a_thread_aecawb_deinit(thread_aecawb);
      gAECAWBThreadRegister.ext_thread.is_valid = FALSE;
    }
  } while(0);

  if (is_lock_acquired && 0 != pthread_mutex_unlock(&gAECAWBThreadRegister.cbMutex)) {
    Q3A_ERR("Failed to Unlock Mutex for Stop ext Thread");
  }

  return;
}


/** q3a_thread_ext_af_handler
 *    @af_data: The pointer to the af thread data
 *
 *  This is the af thread that will run until it receives the STOP message.
 *  While running, it will dequeue messages from the thread's queue and process
 *  them. If there are no messages to process (queue is empty), the thread will
 *  sleep until it gets signaled.
 *
 *  Return NULL
 **/
static void* q3a_thread_ext_af_handler(void *af_data)
{
  q3a_thread_af_data_t *af = (q3a_thread_af_data_t *)af_data;
  q3a_thread_af_msg_t  *msg = NULL;
  int                  exit_flag = 0;
  uint8_t              camera_id = 0;
  uint8_t              num_of_cams = 0;
  q3a_thread_af_camera_data_t *camera_af = NULL;
  void                 *af_algo_obj = NULL;
  q3a_thread_ctrl_t    *af_thread_ctrl = NULL;
  af_ops_t             af_ops;

  af->thread_data->active = 1;
  sem_post(&af->thread_data->sem_launch);
  Q3A_HIGH("Starting AF thread handler");

  do {
    Q3A_LOW(" Waiting for message");

    pthread_mutex_lock(&af->thread_data->thread_mutex);
    while ((af->thread_data->msg_q->length == 0) &&
      (af->thread_data->p_msg_q->length == 0)) {
      pthread_cond_wait(&af->thread_data->thread_cond,
        &af->thread_data->thread_mutex);
    }
    pthread_mutex_unlock(&af->thread_data->thread_mutex);
    /* Get the message */
    pthread_mutex_lock(&af->thread_data->msg_q_lock);
    /*Pop from priority queue first and if its empty pop from normal queue*/
    msg = (q3a_thread_af_msg_t *) mct_queue_pop_head(af->thread_data->p_msg_q);

    if (!msg) {
      msg = (q3a_thread_af_msg_t *) mct_queue_pop_head(af->thread_data->msg_q);
    }
    pthread_mutex_unlock(&af->thread_data->msg_q_lock);

    if (!msg) {
      Q3A_ERR(" msg NULL");
      continue;
    }

    /* Select AF port and algo data */
    camera_id = msg->camera_id;
    af_thread_ctrl = &af->thread_data->thread_ctrl[camera_id];
    num_of_cams = af->thread_data->num_of_registered_cameras;
    camera_af = &af->camera[camera_id];
    af_algo_obj = af->camera[camera_id].af_obj->af;
    af_ops = af->camera[camera_id].af_obj->af_ops;


    /* Flush the queue if it is stopping. Free the enqueued messages and
     * signal the sync message owners to release their resources */
    if (af->thread_data->active == 0 || num_of_cams > MAX_3A_MULTI_CAMERA_ID) {
      if (msg->type != MSG_AF_STOP_THREAD) {
        if ((msg->type == MSG_AF_STATS) || (msg->type == MSG_BF_STATS)) {
          camera_af->af_stats_cb(camera_af->af_port, msg->u.stats);
        }
        if (msg->sync_flag == TRUE) {
           sem_post(&msg->sync_obj->msg_sem);
           /*don't free msg, the sender will do*/
           msg = NULL;
        } else {
          q3a_thread_af_free_msg(&msg);
        }
        continue;
      }
    }

    /* Process message accordingly */
    Q3A_LOW(" Got the message of type: %d for cam: %u", msg->type, msg->camera_id);
    switch (msg->type) {
    case MSG_AF_START: {
      /* Send local output copy to avoid reset of custom data
          and map custom parameters data in local output variable */
      uint8_t cam_cnt = 0;
      af_output_data_t output_arr[MAX_3A_MULTI_CAMERA_ID];

      STATS_MEMSET(output_arr, 0, MAX_3A_MULTI_CAMERA_ID*sizeof(af_output_data_t));
      /* Copy custom data and camera_id into output */
      for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
        output_arr[cam_cnt].af_custom_param =
          af->camera[cam_cnt].af_obj->output.af_custom_param;
        output_arr[cam_cnt].type = AF_OUTPUT_UPDATE_PENDING;
      }


      ATRACE_BEGIN("AF_START");
      af_ops.set_parameters(&msg->u.af_set_parm, output_arr, num_of_cams,
        af_algo_obj);

      for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
        /* Copy back custom data, in case it was deleted */
        output_arr[cam_cnt].af_custom_param =
          af->camera[cam_cnt].af_obj->output.af_custom_param;
        af->camera[cam_cnt].af_obj->output = output_arr[cam_cnt];

        /* Verify if update is requied */
        if (output_arr[cam_cnt].type != AF_OUTPUT_UPDATE_PENDING) {
          af->camera[cam_cnt].af_obj->output = output_arr[cam_cnt];
          af->camera[cam_cnt].af_cb(&af->camera[cam_cnt].af_obj->output,
            af->camera[cam_cnt].af_port);
        }
      }
      ATRACE_END();
    }
      break;

    case MSG_AF_CANCEL: {
      /* Send local output copy to avoid reset of custom data
         and map custom parameters data in local output variable */
      uint8_t cam_cnt = 0;
      af_output_data_t output_arr[MAX_3A_MULTI_CAMERA_ID];

      STATS_MEMSET(output_arr, 0, MAX_3A_MULTI_CAMERA_ID*sizeof(af_output_data_t));
      /* Copy custom data and camera_id into output */
      for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
        output_arr[cam_cnt].af_custom_param =
          af->camera[cam_cnt].af_obj->output.af_custom_param;
        output_arr[cam_cnt].type = AF_OUTPUT_UPDATE_PENDING;
      }

      af_ops.set_parameters(&msg->u.af_set_parm, output_arr, num_of_cams,
        af_algo_obj);

      for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
        /* Copy back custom data, in case it was deleted */
        output_arr[cam_cnt].af_custom_param =
          af->camera[cam_cnt].af_obj->output.af_custom_param;
        af->camera[cam_cnt].af_obj->output = output_arr[cam_cnt];

        /* Verify if update is requied */
        if (output_arr[cam_cnt].type != AF_OUTPUT_UPDATE_PENDING) {
          af->camera[cam_cnt].af_obj->output = output_arr[cam_cnt];
          af->camera[cam_cnt].af_cb(&af->camera[cam_cnt].af_obj->output,
            af->camera[cam_cnt].af_port);
        }
      }
    }
      break;
    case MSG_AF_SEND_EVENT: {
      /* Send local output copy to avoid reset of custom data
         and map custom parameters data in local output variable */
      af_output_data_t output;
      output.af_custom_param = camera_af->af_obj->output.af_custom_param;

      output.type = AF_OUTPUT_SEND_EVENT;
      camera_af->af_cb(&output, camera_af->af_port);
    }
      break;

    case MSG_AF_GET: {
      af_ops.get_parameters(&msg->u.af_get_parm, af_algo_obj);
    }
      break;

    case MSG_AF_SET: {
      /* Send local output copy to avoid reset of custom data
         and map custom parameters data in local output variable */
      uint8_t cam_cnt = 0;
      af_output_data_t output_arr[MAX_3A_MULTI_CAMERA_ID];

      STATS_MEMSET(output_arr, 0, MAX_3A_MULTI_CAMERA_ID*sizeof(af_output_data_t));
      /* Copy custom data and camera_id into output */
      for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
        output_arr[cam_cnt].af_custom_param =
          af->camera[cam_cnt].af_obj->output.af_custom_param;
        output_arr[cam_cnt].type = AF_OUTPUT_UPDATE_PENDING;
      }

      ATRACE_BEGIN("AF_SET");
      if (af_ops.set_parameters(&msg->u.af_set_parm, output_arr, num_of_cams,
          af_algo_obj)) {
        for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
          /* Copy back custom data, in case it was deleted */
          output_arr[cam_cnt].af_custom_param =
            af->camera[cam_cnt].af_obj->output.af_custom_param;
          af->camera[cam_cnt].af_obj->output = output_arr[cam_cnt];

          /* Verify if update is requied */
          if (output_arr[cam_cnt].type != AF_OUTPUT_UPDATE_PENDING) {
            af->camera[cam_cnt].af_obj->output = output_arr[cam_cnt];
            af->camera[cam_cnt].af_cb(&af->camera[cam_cnt].af_obj->output,
              af->camera[cam_cnt].af_port);
          }
        }
      }
      ATRACE_END();
    }
      break;

    case MSG_AF_STATS:
    case MSG_BF_STATS: {
      /*AF_OUTPUT_EZ_METADATA help to stop MSG_AF_SET case,when update af info to metadata*/
      if (!af_thread_ctrl->no_stats_mode) {
        uint8_t cam_cnt = 0;
        af_output_data_t output_arr[MAX_3A_MULTI_CAMERA_ID];
        ATRACE_BEGIN("Camera:AF");
        /* Send local output copy to avoid reset of custom data
           and map custom parameters data in local output variable */
        /* Copy custom data and camera_id into output */
        for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
          output_arr[cam_cnt].af_custom_param =
            af->camera[cam_cnt].af_obj->output.af_custom_param;
          output_arr[cam_cnt].type = AF_OUTPUT_UPDATE_PENDING;
        }


        af_ops.process(msg->u.stats, output_arr, num_of_cams, af_algo_obj);

        for (cam_cnt = 0; cam_cnt < num_of_cams; cam_cnt++) {
          /* Copy back custom data, in case it was deleted */
          output_arr[cam_cnt].af_custom_param =
            af->camera[cam_cnt].af_obj->output.af_custom_param;

          /* Verify if update is requied */
          if (output_arr[cam_cnt].type != AF_OUTPUT_UPDATE_PENDING) {
            af->camera[cam_cnt].af_obj->output = output_arr[cam_cnt];
            af->camera[cam_cnt].af_cb(&af->camera[cam_cnt].af_obj->output,
              af->camera[cam_cnt].af_port);
          }
        }

        ATRACE_END();
      } else {
        Q3A_HIGH("  no_stats_mode");
        camera_af->af_cb(&camera_af->af_obj->output, camera_af->af_port);
      }

      camera_af->af_stats_cb(camera_af->af_port, msg->u.stats);
    }
      break;

    case MSG_AF_STOP_THREAD: {
      exit_flag = 1;
    }
      break;

    default: {
    }
      break;
    }
    if (msg->sync_flag == TRUE) {
      sem_post(&msg->sync_obj->msg_sem);
      /* Don't free msg, the sender will do */
      msg = NULL;
    } else {
      q3a_thread_af_free_msg(&msg);
    }
  } while (!exit_flag);
  return NULL;
} /* af_thread_handler */

static boolean q3a_thread_ext_af_create(q3a_thread_af_data_t *thread_af_data,
  mct_port_t *mct_port) {
  boolean rc = TRUE;
  af_port_private_t *af_port = (af_port_private_t *)(mct_port->port_private);

  if (NULL == thread_af_data || NULL == af_port) {
    Q3A_ERR("NULL ext data");
    return FALSE;
  }

  rc = q3a_thread_af_start(thread_af_data, q3a_thread_ext_af_handler,
    "CAM_EXT_AF");

  return rc;
}

boolean q3a_thread_ext_af_create_single_thread(mct_port_t *mct_port)
{
  boolean rc = FALSE;
  boolean is_lock_acquired = FALSE;
  af_port_private_t *af_port = (af_port_private_t *)(mct_port->port_private);
  q3a_thread_af_data_t *thread_af_data = NULL;
  int test_dual_out = 0;
  int share_af_algo = 0;

  do {
    if (0 != pthread_mutex_lock(&gThreadAFRegister.cbMutex)) {
       Q3A_ERR("Failed to Lock Mutex for thread register");
      break;
    }
    is_lock_acquired = TRUE;
    if (gThreadAFRegister.ext_thread.ref_cnt >= MAX_3A_MULTI_CAMERA_ID) {
      Q3A_ERR("Reach max limit of supported cameras: %d", MAX_3A_MULTI_CAMERA_ID);
      FALSE == rc;
      break;
    }
    Q3A_HIGH("AF: Camera: %u, session_id: %u", gThreadAFRegister.ext_thread.ref_cnt,
      GET_SESSION_ID(af_port->reserved_id));

    /* Verify if thread as been created */
    if (FALSE == gThreadAFRegister.ext_thread.is_valid) {/*Create thread*/
      thread_af_data = q3a_thread_af_init();
      if (NULL == thread_af_data) {
        Q3A_ERR("Fail to init ext thread");
        break;
      }

      rc = q3a_thread_ext_af_create(thread_af_data, mct_port);
      if (FALSE == rc) {
        Q3A_ERR("Fail to create ext aecawb thread");
        break;
      }

      /* Set sharing configuration */
      gThreadAFRegister.ext_thread.share.is_sharing = TRUE;
      gThreadAFRegister.ext_thread.share.camera_id_sharing_algo = 0;

      /* Test */
      STATS_TEST_DUAL_OUTPUT(test_dual_out);
      if (test_dual_out) {
        int share_aecawb_algo = 0;
        STATS_TEST_SHARE_AF_ALGO(share_af_algo);
        gThreadAFRegister.ext_thread.share.is_sharing = share_af_algo;
      }

      gThreadAFRegister.ext_thread.ptr = thread_af_data;
      gThreadAFRegister.ext_thread.is_valid = TRUE;
      gThreadAFRegister.ext_thread.is_thread_running = TRUE;
      gThreadAFRegister.ext_thread.ref_cnt++;
    } else {
      gThreadAFRegister.ext_thread.ref_cnt++;
      rc = TRUE; /* not an error */


      Q3A_HIGH("AF: Thread already created: %p, increase ref_cnt = %d",
        gThreadAFRegister.ext_thread.ptr,
        gThreadAFRegister.ext_thread.ref_cnt);
    }
  } while(0);
  if (is_lock_acquired) {
    Q3A_HIGH("AF: camera_id: %u, is_sharing: %u, camera_id_sharing_algo: %d",
      gThreadAFRegister.ext_thread.ref_cnt - 1,
      gThreadAFRegister.ext_thread.share.is_sharing,
      gThreadAFRegister.ext_thread.share.camera_id_sharing_algo);
    if (FALSE == rc) {
      Q3A_ERR("Handling error: de-init ext thread");
      if (thread_af_data) {
        q3a_thread_af_deinit(thread_af_data);
      }
    }
    if (0 != pthread_mutex_unlock(&gThreadAFRegister.cbMutex)) {
      Q3A_ERR("Failed to Unlock Mutex for ext Thread Register");
    }
  }
  return rc;
}

void q3a_thread_ext_af_destroy_single_thread()
{
  boolean rc = FALSE;
  boolean is_lock_acquired = FALSE;
  q3a_thread_af_data_t *ext_thread_af = NULL;
  do {
    if (0 != pthread_mutex_lock(&gThreadAFRegister.cbMutex)) {
       Q3A_ERR("Failed to Lock Mutex for thread register");
      break;
    }
    is_lock_acquired = TRUE;
    if (FALSE == gThreadAFRegister.ext_thread.is_valid) {
      Q3A_ERR("Error thread is already gone!");
      break;
    }
    gThreadAFRegister.ext_thread.ref_cnt--;
    ext_thread_af = (q3a_thread_af_data_t *)gThreadAFRegister.ext_thread.ptr;
    ext_thread_af->thread_data->num_of_registered_cameras--;

    Q3A_HIGH("ref_cnt: %d", gThreadAFRegister.ext_thread.ref_cnt);
    if (gThreadAFRegister.ext_thread.is_thread_running) {
      Q3A_HIGH("Not the last instance, but stop thread now!");
      rc = q3a_thread_af_stop(ext_thread_af);
      if (FALSE == rc) {
        Q3A_ERR("Fail to stop ext thread");
        break;
      }
      gThreadAFRegister.ext_thread.is_thread_running = FALSE;
      break;
    }

    Q3A_HIGH("Last instance: De-init thread");
    q3a_thread_af_deinit(ext_thread_af);
    ext_thread_af = NULL;
    gThreadAFRegister.ext_thread.ptr = NULL;
    gThreadAFRegister.ext_thread.is_valid = FALSE;
  } while(0);

  if (is_lock_acquired && 0 != pthread_mutex_unlock(&gThreadAFRegister.cbMutex)) {
    Q3A_ERR("Failed to Unlock Mutex for Stop ext Thread");
  }

  return;
}

