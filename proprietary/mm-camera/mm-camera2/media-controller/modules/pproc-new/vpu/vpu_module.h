/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef VPU_MODULE_H
#define VPU_MODULE_H

#include <pthread.h>
#include <stdbool.h>
#include "modules.h"
#include "mct_list.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_object.h"
#include "cam_types.h"
#include "cam_intf.h"
#include "mct_module.h"
#include "mct_pipeline.h"
#include "mct_stream.h"
#include "vpu_port.h"
#include "vpu_client.h"
#include "pp_buf_mgr.h"

#define VPU_MODULE_MAX_STREAMS      4
#define VPU_MODULE_MAX_SESSIONS     2

#define VPU_NUM_NATIVE_BUFFERS      4

/* macros for unpacking identity */
#define VPU_GET_STREAM_ID(identity) ((identity) & 0x0000FFFF)
#define VPU_GET_SESSION_ID(identity) (((identity) & 0xFFFF0000) >> 16)

/* data structure to store the ACK in list */
typedef struct _vpu_module_ack_t {
  isp_buf_divert_ack_t isp_buf_divert_ack;
  struct timeval in_time, out_time;
} vpu_module_ack_t;

/* list for storing ACKs */
typedef struct _vpu_module_ack_list_t {
  mct_list_t        *list;
  uint32_t           size;
  pthread_mutex_t    mutex;
} vpu_module_ack_list_t;


typedef struct _vpu_module_stream_params_t vpu_module_stream_params_t;

/* stream specific parameters */
typedef struct _vpu_module_stream_params_t {
  uint32_t                     identity;
  /* identity which is streamed on, used for native buffers */
  uint32_t                     native_identity;
  cam_stream_type_t            stream_type;
  boolean                      is_stream_on;
  pthread_mutex_t              mutex;
  mct_stream_info_t            stream_info;
  mct_stream_info_t            isp_output_info;
  bool                         is_stream_off_requested;
  vpu_module_stream_params_t   *linked_stream_params;
  struct v4l2_format           format;
  bool                         img_fmt_ready;
  bool                         is_native_buf;
} vpu_module_stream_params_t;

/* session specific parameters */
typedef struct _vpu_module_session_params_t {
  uint32_t                    session_id;
  int32_t                     stream_count;
  vpu_module_stream_params_t *stream_params[VPU_MODULE_MAX_STREAMS];
} vpu_module_session_params_t;


typedef struct _vpu_module_ctrl_t {
  mct_module_t                *p_module;
  vpu_module_ack_list_t       ack_list;
  pthread_mutex_t             mutex;
  int32_t                     session_count;
  void                        *buf_mgr;
  pp_native_buf_mgr_t         native_buf_mgr;
  vpu_module_session_params_t *session_params[VPU_MODULE_MAX_SESSIONS];
  vpu_client_t                client;
  uint32_t                    client_identity;
  bool                        bypass;
} vpu_module_ctrl_t;

/* Module functions */

mct_module_t* vpu_module_init(const char *name);

void vpu_module_deinit(mct_module_t *mod);

static void vpu_module_set_mod (mct_module_t *module,
  mct_module_type_t module_type, uint32_t identity);

static boolean vpu_module_query_mod(mct_module_t *module, void *query_buf,
  uint32_t sessionid);

static boolean vpu_module_start_session(mct_module_t *module,
  uint32_t sessionid);

static boolean vpu_module_stop_session(mct_module_t *module,
  uint32_t sessionid);

static vpu_module_ctrl_t* vpu_module_create_vpu_ctrl(void);

static int32_t vpu_module_destroy_vpu_ctrl(vpu_module_ctrl_t *ctrl);

static int32_t vpu_module_send_ack_event_upstream(vpu_module_ctrl_t *ctrl,
  isp_buf_divert_ack_t isp_ack);

int32_t vpu_module_process_downstream_event(mct_module_t* module,
  mct_event_t *event);

int32_t vpu_module_process_upstream_event(mct_module_t* module,
  mct_event_t *event);

int32_t vpu_module_send_event_downstream(mct_module_t* module,
   mct_event_t* event);

int32_t vpu_module_send_event_upstream(mct_module_t* module,
   mct_event_t* event);

int32_t vpu_module_notify_add_stream(mct_module_t* module, mct_port_t* port,
  mct_stream_info_t* stream_info);

int32_t vpu_module_notify_remove_stream(mct_module_t* module,
  uint32_t identity);

int32_t vpu_module_put_new_ack_in_list(vpu_module_ctrl_t *ctrl,
  isp_buf_divert_ack_t isp_ack);

void vpu_module_client_cb(vpu_client_event_t event, void *arg, void *userdata);

/* utility functions */

mct_port_t* vpu_module_find_port_with_identity(mct_module_t *module,
  mct_port_direction_t dir, uint32_t identity);

vpu_module_ack_t* vpu_module_find_ack_from_list(vpu_module_ctrl_t *ctrl,
  int32_t buf_idx);

int32_t vpu_module_get_params_for_identity(vpu_module_ctrl_t* ctrl,
  uint32_t identity, vpu_module_session_params_t** session_params,
  vpu_module_stream_params_t** stream_params);

int32_t vpu_module_get_params_for_session_id(vpu_module_ctrl_t* ctrl,
  uint32_t session_id, vpu_module_session_params_t** session_params);

/* Event handling functions */

int32_t vpu_module_handle_buf_divert_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpu_module_handle_buf_divert_ack_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpu_module_handle_streamon_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpu_module_handle_streamoff_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpu_module_handle_chromatix_ptr_event(mct_module_t* module,
    mct_event_t* event);

int32_t vpu_module_handle_aec_update_event(mct_module_t* module,
    mct_event_t* event);

int32_t vpu_module_handle_set_parm_event(mct_module_t* module,
    mct_event_t* event);

int32_t vpu_module_handle_isp_out_dim_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpu_module_handle_iface_div_req_event(mct_module_t* module,
  mct_event_t* event);


#endif
