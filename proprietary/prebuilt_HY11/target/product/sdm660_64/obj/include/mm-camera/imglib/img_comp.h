/***************************************************************************
* Copyright (c) 2013-2017, 2019 Qualcomm Technologies, Inc.                *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
****************************************************************************/

#ifndef __IMG_COMP_H__
#define __IMG_COMP_H__

#include "img_common.h"
#include <dlfcn.h>
#include "img_meta.h"

/** QIMG_DENOISE_CMD_OFF
 *
 *  Userdefined command offset for denoise component
 **/
#define QIMG_DENOISE_CMD_OFF   0x100

/** QIMG_FACEPROC_CMD_OFF
 *
 *  Userdefined command offset for faceproc component
 **/
#define QIMG_FACEPROC_CMD_OFF  0x200

/** QIMG_HDR_CMD_OFF
 *
 *  Userdefined command offset for HDR component
 **/
#define QIMG_HDR_CMD_OFF       0x300

/** QIMG_CAC_CMD_OFF
 *
 *  Userdefined command offset for CAC component
 **/
#define QIMG_CAC_CMD_OFF       0x400


/** QIMG_DENOISE_PARAM_OFF
 *
 *  Userdefined paramter offset for denoise component
 **/
#define QIMG_DENOISE_PARAM_OFF    0x100

/** QIMG_FACEPROC_PARAM_OFF
 *
 *  Userdefined paramter offset for faceproc component
 **/
#define QIMG_FACEPROC_PARAM_OFF   0x200

/** QIMG_HDR_PARAM_OFF
 *
 *  Userdefined paramter offset for HDR component
 **/
#define QIMG_HDR_PARAM_OFF        0x300

/** QIMG_CAC_PARAM_OFF
 *
 *  Userdefined paramter offset for CAC component
 **/
#define QIMG_CAC_PARAM_OFF        0x400

/** QIMG_LIB2D_PARAM_OFF
 *
 *  Userdefined paramter offset for lib2d component
 **/
#define QIMG_LIB2D_PARAM_OFF        0x500

/**
 *
 *  Common parameters used by all components
 **/
#define QIMG_PARAM_FRAME_INFO 0
#define QIMG_PARAM_MODE       1
#define QIMG_CAMERA_DUMP      2
#define QIMG_PARAM_CAPS       3
#define QIMG_PARAM_FRAME_OPS  4
#define QIMG_PARAM_DCRF       5
#define QIMG_DCRF_RESULT      6
#define QIMG_PARAM_SRC_DIM    7
#define QIMG_PARAM_DST_DIM    8
#define QIMG_PARAM_YUV_FMT    9
#define QIMG_PARAM_CROP_DIM   10
#define QIMG_PARAM_FRAME_RATE 11
#define QIMG_PARAM_SRC_BUF_SZ 12
#define QIMG_PARAM_DST_BUF_SZ 13
// MAX downscale factor
#define QIMG_PARAM_MAX_DS     14
#define QIMG_PARAM_SET_LIB_CALLBACK 15
#define QIMG_PARAM_LIB_CONFIG 16
#define QIMG_PARAM_IS_MASTER 17
#define QIMG_PARAM_INIT_UPDATE 18
#define QIMG_PARAM_ALGO_CAPS 19
#define QIMG_PARAM_FRAME_DROP 20

/** img_event_type
 *  QIMG_EVT_ERROR: error event
 *  QIMG_EVT_DONE: event called when the component has completed
 *                 execution
 *  QIMG_EVT_BUF_DONE: event called when the component has
 *                    completed processing the buffer
 *  QIMG_EVT_FACE_PROC: event called when the face processing is
 *                     completed
 *  QIMG_EVT_IMG_BUF_DONE: event is called when the component
 *                     has completed processing the buffer.
 *                     payload of img_frame_t is sent
 *  QIMG_EVT_META_BUF_DONE: event is called when the component
 *                     has completed processing the meta buffer.
 *                     payload of img_meta_t is sent
 *  QIMG_EVT_IMG_OUT_BUF_DONE: event is called when the component
 *                     has completed processing the output
 *                     buffer. payload of img_frame_t is sent
 *  QIMG_EVT_DCRF_DONE: event is called when the component
 *                     has completed processing the dcrf algorithm
 *  QIMG_EVT_COMP_CONNECTION_FAILED: event is called when the component
 *                     has lost its connection with underlying library
 *  QIMG_EVT_FRAME_DROP: event is called when the component is
 *              wants to indicate that a frame needs to be
 *              dropped.
 *  QIMG_EVT_MAX: MAX enum for img_event_type
 *
 *  Imaging event type
 **/
typedef enum {
  QIMG_EVT_ERROR,
  QIMG_EVT_DONE,
  QIMG_EVT_BUF_DONE,
  QIMG_EVT_FACE_PROC,
  QIMG_EVT_EARLY_CB_DONE,
  QIMG_EVT_IMG_BUF_DONE,
  QIMG_EVT_META_BUF_DONE,
  QIMG_EVT_IMG_OUT_BUF_DONE,
  QIMG_EVT_DCRF_DONE,
  QIMG_EVT_COMP_CONNECTION_FAILED,
  QIMG_EVT_FRAME_DROP,
  QIMG_EVT_MAX,
} img_event_type;

/** img_event_t
 *   @type: event type
 *   @status: event status
 *   @p_frame: frame pointer
 *   @p_meta: metabuf pointer
 *
 *   Event data structure
 **/
typedef struct {
  img_event_type type;
  union {
    int status;
    img_frame_t *p_frame;
    img_meta_t *p_meta;
    uint32_t frameid;
  }d;
} img_event_t;

/** comp_state_t
 *   IMG_STATE_IDLE: Idle state, component is uninitialized
 *   IMG_STATE_INIT: State indicates that the component is
 *                   initialized
 *   IMG_STATE_STARTED: State indicates that the component is
 *                      active
 *   IMG_STATE_STOP_REQUESTED: State indicates that the stop is
 *                            is issued from the client and is
 *                            in the process of stopping
 *   IMG_STATE_STOPPED: State indicates that the component has
 *                      stopped execution
 *   IMG_STATE_START_PENDING: State indicates that the start is
 *                            is issued from the client and is
 *                            in the process of starting
 **/
typedef enum {
  IMG_STATE_IDLE,
  IMG_STATE_INIT,
  IMG_STATE_STARTED,
  IMG_STATE_STOP_REQUESTED,
  IMG_STATE_STOPPED,
  IMG_STATE_START_PENDING,
} comp_state_t;

/** img_event_notification_t
 *   IMG_COMP_EVENT_FLUSH: Flush has started. Inform component
 *                   to start handling Flush event.
 *   IMG_COMP_EVENT_ABORT: StreamOff has started. Inform component
 *                   to start handling Abort.
 **/
typedef enum {
  IMG_COMP_EVENT_FLUSH,
  IMG_COMP_EVENT_ABORT,
} comp_event_notification_t;

/**
 *   type of the image frame
 **/
#define IMG_IN                  0x1
#define IMG_OUT                 0x2
#define IMG_IN_OUT (IMG_IN|IMG_OUT)
#define IMG_META                0x4

/** img_type_t
 *
 *  image type IMG_IN/IMG_OUT/IMG_IN_OUT/IMG_META
 **/
typedef int img_type_t;

/** img_cmd_type
 *
 *  command type
 **/
typedef int img_cmd_type;

/** img_param_type
 *
 *  paramter type
 **/
typedef int img_param_type;

/** notify_cb
 *
 *  Notify callback function
 **/
typedef int (*notify_cb) (void* p_appdata, img_event_t *p_event);

/** img_component_ops_t
*    @init: function pointer for initialization functionality
*    @deinit:  function pointer for uninitialization
*           functionality
*    @set_parm: function pointer for set parameter functionality
*    @get_parm: function pointer for get parameter functionality
*    @set_callback: function pointer for setcallback
*                 functionality
*    @start: function pointer for start functionality
*    @abort: function pointer for abort functionality
*    @process: function pointer for process functionality
*    @queue_buffer: function pointer for queue buffer
*    @deque_buffer: function pointer for dequeue buffer
*    @queue_metabuffer: function pointer for queue meta buffer
*    @flush: function pointer for handling flush. By definition, this is
*          blocking call until component finishes its current work.
*    @notify_event: function pointer to notify the required events to
*           Component. By definition, this just informs the component
*           that something has started, for exa : Flush so that component
*           can start handling that event.
*    @deque_buffer_by_fid: function pointer to dequeue buffer
*           by frame id
*    @handle: component handle
*    @callback: pointer to frameproc callback function
*    Component ops table
**/
typedef struct {
  //
  int (*init)(void *handle, void* p_userdata, void *p_data);
  //
  int (*deinit)(void *handle);
  //
  int (*set_parm)(void *handle, img_param_type param, void *p_data);
  //
  int (*get_parm)(void *handle, img_param_type param, void* p_data);
  //
  int (*set_callback)(void *handle, notify_cb notify);
  //
  int (*start)(void *handle, void *p_data);
  //
  int (*abort)(void *handle, void *p_data);
  //
  int (*process)(void *handle, img_cmd_type cmd, void *p_data);
  //
  int (*queue_buffer)(void *handle, img_frame_t *p_frame, img_type_t type);
  //
  int (*deque_buffer)(void *handle, img_frame_t **pp_frame);
  //
  int (*queue_metabuffer)(void *handle, img_meta_t *p_meta);
  //
  comp_state_t (*get_state)(void *handle);
  //
  int (*flush)(void *handle, img_type_t type);
  //
  int (*notify_event)(void *handle, comp_event_notification_t type);
  //
  int (*callback)(void *userdata);
  //
  void *handle;
  //
  int (*deque_buffer_by_fid)(void *handle, void **pp_buf, uint32_t fid,
    img_type_t type);
  //
} img_component_ops_t;

/**
 * Function: img_comp_check_state
 *
 * Description: Check if requested state transition is allowed or not.
 *
 * Arguments:
 *   @p_mutex: Component lock protecting the state
 *   @p_state: Pointer to component state.
 *   @new_state: State need to be checked.
 *
 * Return values:
 *   IMG_SUCCESS - Transition allowed / IMG_ERR - Transition not allowed.
 *
 * Note: Caller needs to perform NULL check for p_mutex and
 * p_state before calling. This function does not perform NULL
 * check.
 **/
int img_comp_check_state(pthread_mutex_t *p_mutex, comp_state_t *p_state,
  comp_state_t new_state);

/**
 * Function: img_comp_set_state
 *
 * Description: Set component state, state will be set only if state transition
 *   is allowed.
 *
 * Arguments:
 *   @p_mutex: Component lock protecting the state
 *   @p_state: Pointer to component state.
 *   @new_state: State need to be set.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Note: Caller needs to perform NULL check for p_mutex and
 * p_state before calling. This function does not perform NULL
 * check.
 **/
int img_comp_set_state(pthread_mutex_t *p_mutex, comp_state_t *p_state,
  comp_state_t new_state);

/**
 * Function: img_comp_get_curr_state
 *
 * Description: Check if component is in given state.
 *
 * Arguments:
 *   @p_mutex: Component lock protecting the state
 *   @p_state: Pointer to jpeg dma component instance.
 *
 * Return values:
 *   Current state
 *
 * Note: Caller needs to perform NULL check for p_mutex and
 * p_state before calling. This function does not perform NULL
 * check.
 **/
comp_state_t img_comp_get_curr_state(pthread_mutex_t *p_mutex,
  comp_state_t *p_state);

/**
 * Function: img_comp_is_curr_state
 *
 * Description: Check if component is in given state.
 *
 * Arguments:
 *   @p_mutex: Component lock protecting the state
 *   @p_state: Pointer to component state.
 *   @state: State need to be checked.
 *
 * Return values:
 *   true if state match
 *   false if state does not match
 *
 * Note: Caller needs to perform NULL check for p_mutex and
 * p_state before calling. This function does not perform NULL
 * check.
 **/
bool img_comp_is_curr_state(pthread_mutex_t *p_mutex, comp_state_t *p_state,
  comp_state_t state);

/**
 * Function: img_comp_find_buffer_by_fid
 *
 * Description: This function is used to find buffer in queue by
 * frame id
 *
 * Input parameters:
 *   @p_nodedata: The pointer to node in queue.
 *   @p_userdata: Userdata i.e frame id
 *
 * Return values:
 *   1 if buffer found, 0 if not
 *
 * Notes:
 **/
int img_comp_find_buffer_by_fid(void *p_nodedata, void *p_userdata);

#endif //__IMG_COMP_H__
