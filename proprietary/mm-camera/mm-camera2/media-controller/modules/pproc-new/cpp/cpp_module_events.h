/*============================================================================

  Copyright (c) 2013, 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef CPP_MODULE_EVENTS_H
#define CPP_MODULE_EVENTS_H

#include "modules.h"
#include "chromatix.h"
#include "chromatix_common.h"
#include "mct_queue.h"
#include "mct_list.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_object.h"
#include "cam_types.h"
#include "mct_module.h"
#include "mct_pipeline.h"
#include "mct_stream.h"
#include "camera_dbg.h"
#include "cpp_port.h"
#include "cpp_module.h"
#include "cpp_log.h"
#include "eztune_diagnostics.h"

/* cpp_decide_hfr_skip:
 *
 * Decides if a frame needs to be skipped. if @frame_id is 0, its not skipped.
 * @count number of frames are skipped after the 0th frame.
 * The pattern repeats.
 *
 **/
#define cpp_decide_hfr_skip(frame_id, count) \
  ((((frame_id) % ((count)+1)) == 0) ? FALSE : TRUE)

int32_t cpp_module_handle_buf_divert_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_isp_out_dim_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_isp_drop_buffer(mct_module_t* module,
  uint32_t frame_id, bool is_slave, cam_stream_type_t stream_type,
  uint32_t session_id);

int32_t cpp_module_handle_aec_manual_update(mct_module_t *module,
  mct_event_t *event);

int32_t cpp_module_handle_aec_update_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_chromatix_ptr_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_stream_crop_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_stream_fovc_crop_factor_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_stream_query_output_size_event(mct_module_t *module,
  mct_event_t *event);

int32_t cpp_module_handle_stream_dualcam_shift_offset_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_stream_dualcam_shift_margins_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_inform_lpm_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_dis_update_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_set_parm_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_set_stream_parm_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_sof_set_parm_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_streamon_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_streamoff_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_stream_cfg_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_load_chromatix_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_set_output_buff_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_sof_notify(mct_module_t *module,
  mct_event_t *event);

int32_t cpp_module_handle_sleep_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_process_set_param_event(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cam_intf_parm_type_t type, void *parm_data,
  boolean *param_processed, uint32_t current_frame_id);

int32_t cpp_module_update_aec(mct_module_t *module,
  uint32_t identity, cpp_params_aec_trigger_info_t *aec_trigger);

int32_t cpp_module_process_frame_control(mct_module_t *module,
  cpp_per_frame_params_t *per_frame_params, uint32_t identity,
  uint32_t cur_frame_id, uint32_t process_type);

int32_t cpp_module_handle_update_buf_info(mct_module_t* module,
  mct_event_t* event, boolean delete_buf);

int32_t cpp_module_handle_face_event(mct_module_t* module,
  mct_event_t* event);
int32_t cpp_module_request_pproc_divert_info(mct_module_t* module,
  mct_event_t *event);
int32_t cpp_module_handle_post_hdr_curves(mct_module_t* module,
  mct_event_t* event);
int32_t cpp_module_handle_sensor_query_output_size(
  mct_module_t* module, mct_event_t *event);

#endif
