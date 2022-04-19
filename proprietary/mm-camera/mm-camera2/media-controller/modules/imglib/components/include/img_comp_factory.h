/***************************************************************************
* Copyright (c) 2013-2016 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
****************************************************************************/

#ifndef __IMG_COMP_FACTORY_H__
#define __IMG_COMP_FACTORY_H__

#include "img_comp.h"

/** img_comp_role_t
 *   IMG_COMP_DENOISE: Component role is denoising
 *   IMG_COMP_HDR: component role is HDR
 *   IMG_COMP_FACE_PROC: component role is faceproc
 *   IMG_COMP_CAC - component role is CAC
 *   IMG_COMP_GEN_FRAME_PROC - component role is generic frame
 *                          processing
 *   IMG_COMP_LIB2D - component role is LIB2D
 *   IMG_COMP_GEN_2D - component role is GEN_2D
 *   IMG_COMP_MULTI_FRAME_PROC - component role multi frameproc
 *   IMG_COMP_ROLE_MAX - max value of component role
 *
 *   Role of the component
 **/
typedef enum {
  IMG_COMP_DENOISE,
  IMG_COMP_HDR,
  IMG_COMP_FACE_PROC,
  IMG_COMP_CAC,
  IMG_COMP_GEN_FRAME_PROC,
  IMG_COMP_DUAL_FRAME_PROC,
  IMG_COMP_LIB2D,
  IMG_COMP_GEN_2D,
  IMG_COMP_MULTI_FRAME_PROC,
  IMG_COMP_ROLE_MAX,
} img_comp_role_t;

/** img_core_ops_t
 *   @create: function pointer for the component create function
 *   @load: function pointer for the component load
 *   @unload: function pointer for the component unload
 *   @alloc: function pointer for the component alloc
 *   @dealloc: function pointer for the component dealloc
 *   @bind: function pointer for the component to bind 2 sessions
 *   @unbind: function pointer for the component to unbind 2 sessions
 *   @preload_needed: function ptr to indicate whether preload
 *                  is needed. preload_needed and valid
 *                  alloc/delloc function pts are needed
 *   @handle: client handle
 *
 *   This structure represent the core ops for the component
 **/
typedef struct {
  //
  int (*create)(void*, img_component_ops_t *);
  //
  int (*load)(const char*, void**);
  //
  int (*unload)(void*);
  //
  int (*alloc)(void *, void *);

  int (*dealloc)(void *);

  int (*bind)(void*, void*);

  int (*unbind)(void*, void*);
  /* pass the handle to determine whether preload is needed */
  bool (*preload_needed)(void*);

  void *handle;
} img_core_ops_t;

/** img_core_get_comp
 *   @role: role of the component
 *   @name: name of the component
 *   @p_ops: pointer to the core ops structure
 *
 *   This function is used to get the component given the name
 *   and the role
 **/
int img_core_get_comp(img_comp_role_t role, char *name,
  img_core_ops_t *p_ops);


/** IMG_COMP_CREATE
 *   @p_ops: pointer to the core ops structure
 *   @p_comp: pointer to the component structure
 *
 *   This function is used to create the component
 **/
#define IMG_COMP_CREATE(p_ops, p_comp) \
  (p_ops)->create((p_ops)->handle, p_comp)

/** IMG_COMP_LOAD
 *   @p_ops: pointer to the core ops structure
 *   @name: Library name (Optional), if not used pass NULL
 *
 *   This function is used to load the component
 **/
#define IMG_COMP_LOAD(p_ops, name) ({\
  int rc = IMG_ERR_GENERAL; \
  if (p_ops) { \
    (p_ops)->handle = NULL; \
    rc = (p_ops)->load(name, &((p_ops)->handle)); \
  } \
  rc; \
})

/** IMG_COMP_UNLOAD
 *   @p_ops: pointer to the core ops structure
 *
 *   This function is used to unload the component
 **/
#define IMG_COMP_UNLOAD(p_ops) \
  (p_ops)->unload((p_ops)->handle)

/** IMG_COMP_INIT
 *   @p_comp: pointer to the component
 *   @p_userdata: pointer to the userdata
 *   @p_data: pointer to the data
 *
 *   This function is used to initialize the component
 **/
#define IMG_COMP_INIT(p_comp, p_userdata, p_data) \
  (p_comp)->init((p_comp)->handle, (p_userdata), (p_data))

/** IMG_COMP_DEINIT
 *   @p_comp: pointer to the component
 *
 *   This function is used to uninitialize the component
 **/
#define IMG_COMP_DEINIT(p_comp) \
  (p_comp)->deinit((p_comp)->handle);

/** IMG_COMP_SET_PARAM
 *   @p_comp: pointer to the component
 *   @param: pointer to the image parameter type
 *   @p_data: pointer to the parameter data
 *
 *   This function is used to set the component parameters
 **/
#define IMG_COMP_SET_PARAM(p_comp, param, p_data) \
  (p_comp)->set_parm((p_comp)->handle, (param), (p_data));

/** IMG_COMP_GET_PARAM
 *   @p_comp: pointer to the component
 *   @param: pointer to the image parameter type
 *   @p_data: pointer to the parameter data
 *
 *   This function is used to get the component parameters
 **/
#define IMG_COMP_GET_PARAM(p_comp, param, p_data) \
  (p_comp)->get_parm((p_comp)->handle, (param), (p_data));

/** IMG_COMP_SET_CB
 *   @p_comp: pointer to the component
 *   @notify: pointer to notify callback function
 *
 *   This function is used to set the callback
 **/
#define IMG_COMP_SET_CB(p_comp, notify) \
  (p_comp)->set_callback((p_comp)->handle, (notify));

/** IMG_COMP_START
 *   @p_comp: pointer to the component
 *   @p_data: pointer to data
 *
 *   This function is used to start the component
 **/
#define IMG_COMP_START(p_comp, p_data) \
  (p_comp)->start((p_comp)->handle, (p_data));

/** IMG_COMP_ABORT
 *   @p_comp: pointer to the component
 *   @p_data: pointer to data
 *
 *   This function is used to abort the component
 **/
#define IMG_COMP_ABORT(p_comp, p_data) \
  (p_comp)->abort((p_comp)->handle, (p_data));

/** IMG_COMP_PROCESS
 *   @p_comp: pointer to the component
 *   @cmd: command type
 *   @p_data: pointer to data
 *
 *   This function is used to send command to the component
 **/
#define IMG_COMP_PROCESS(p_comp, cmd, p_data) \
  (p_comp)->process((p_comp)->handle, (cmd), (p_data));

/** IMG_COMP_Q_BUF
 *   @p_comp: pointer to the component
 *   @p_frame: frame to be queued to the component
 *   @type: image type (main image or thumbnail image)
 *
 *   This function is used to queue the buffer to the component
 **/
#define IMG_COMP_Q_BUF(p_comp, p_frame, type) \
  (p_comp)->queue_buffer((p_comp)->handle, (p_frame), (type));

/** IMG_COMP_DQ_BUF
 *   @p_comp: pointer to the component
 *   @pp_frame: double pointer for the frame to be dequeued from
 *            the component
 *
 *   This function is used to dequeue the buffer from the
 *   component
 **/
#define IMG_COMP_DQ_BUF(p_comp, pp_frame) \
  (p_comp)->deque_buffer((p_comp)->handle, (pp_frame));

/** IMG_COMP_DQ_BUF_BY_FID
 *   @p_comp: pointer to the component
 *   @pp_buf: double pointer for the buffer to be dequeued from
 *            the component
 *   @fid: frame id
 *   @type: frame type input or output
 *
 *   This function is used to dequeue the buffer from the
 *   component based on frame id
 **/
#define IMG_COMP_DQ_BUF_BY_FID(p_comp, pp_buf, fid, type) ({\
  if((p_comp)->deque_buffer_by_fid) { \
    (p_comp)->deque_buffer_by_fid((p_comp)->handle, (pp_buf), (fid), (type)); \
  } \
})

/** IMG_COMP_GET_STATE
 *   @p_comp: pointer to the component
 *
 *   This function is used to get the state of the component
 **/
#define IMG_COMP_GET_STATE(p_comp) \
  (p_comp)->get_state((p_comp)->handle);

/** IMG_COMP_Q_META_BUF
 *   @p_comp: pointer to the component
 *   @p_metaframe: meta frame to be queued to the component
 *   @type: image type (main image or thumbnail image)
 *
 *   This function is used to queue the buffer to the component
 **/
#define IMG_COMP_Q_META_BUF(p_comp, p_metaframe) \
  (p_comp)->queue_metabuffer((p_comp)->handle, (p_metaframe));

/** IMG_COMP_FLUSH
 *   @p_comp: pointer to the component
 *   @type: image type
 *
 *   This function is used to flush the Component queues
 **/
#define IMG_COMP_FLUSH(p_comp, type) \
  (p_comp)->flush((p_comp)->handle, (type));

/** IMG_COMP_NOTIFY
 *   @p_comp: pointer to the component
 *   @event: Notify event type
 *
 *   This function is used to notify the Component about event
 **/
#define IMG_COMP_NOTIFY(p_comp, event) \
  (p_comp)->notify_event((p_comp)->handle, (event));

/** IMG_CORE_PRELOAD
 *   @p_ops: pointer to the core ops structure
 *   @p_params: pointer to the alloc params
 *
 *   This function is used to preload the component
 **/
#define IMG_CORE_PRELOAD(p_ops, p_params) ({ \
  int32_t rc = IMG_SUCCESS; \
  if (IMG_COMP_PRELOAD_SUPPORTED(p_ops)) \
    rc = (p_ops)->alloc((p_ops)->handle, p_params); \
  rc; \
})

/** IMG_CORE_SHUTDOWN
 *   @p_ops: pointer to the core ops structure
 *
 *   This function is used to shutdown the component
 **/
#define IMG_CORE_SHUTDOWN(p_ops) ({ \
  int32_t rc = IMG_SUCCESS; \
  if (IMG_COMP_PRELOAD_SUPPORTED(p_ops)) \
   rc = (p_ops)->dealloc((p_ops)->handle); \
  rc; \
})

 /** IMG_COMP_BIND
  *   @p_ops: pointer to the core ops structure
  *   @p_comp1: pointer to the 1st component
  *   @p_comp2: pointer to the 2nd component
  *
  *   This function is used to bind components
  **/
#define IMG_COMP_BIND(p_ops, p_comp1, p_comp2) ({ \
  int32_t rc = IMG_SUCCESS; \
  if (p_comp2 != NULL) \
    rc = (p_ops)->bind((p_comp1)->handle, (p_comp2)->handle); \
  else \
    rc = (p_ops)->bind((p_comp1)->handle, NULL); \
  rc; \
})

 /** IMG_COMP_UNBIND
  *   @p_ops: pointer to the core ops structure
  *   @p_comp1: pointer to the 1st component
  *   @p_comp2: pointer to the 2nd component
  *
  *   This function is used to deallocate memory
  **/
#define IMG_COMP_UNBIND(p_ops, p_comp1, p_comp2) \
     (p_ops)->unbind((p_comp1)->handle, (p_comp2)->handle);


/** IMG_COMP_PRELOAD_SUPPORTED
 *   @p_ops: pointer to the core ops structure
 *
 *   This function is to check whether the preload is supported
 **/
#define IMG_COMP_PRELOAD_SUPPORTED(p_ops) ({ \
  bool preload_supported = ((p_ops)->alloc && (p_ops)->dealloc); \
  if (preload_supported && (p_ops)->preload_needed) { \
    preload_supported = (p_ops)->preload_needed((p_ops)->handle); \
  } \
  preload_supported; \
})

#endif //__IMG_COMP_FACTORY_H__
