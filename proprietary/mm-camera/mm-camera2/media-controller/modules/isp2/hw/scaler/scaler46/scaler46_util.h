/* scaler46_util.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SCALER46_UTIL_H__
#define __SCALER46_UTIL_H__

/* mctl headers*/
#include "mtype.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_stream.h"
#include "isp_common.h"

/* isp headers */
#include <media/msmb_isp.h>

boolean scaler46_util_get_port_from_module(mct_module_t *module,
  mct_port_t **port, mct_port_direction_t direction, uint32_t identity);

boolean scaler46_util_compare_identity(void *data1, void *data2);

boolean scaler46_util_append_cfg(struct msm_vfe_cfg_cmd_list **hw_update_list,
  struct msm_vfe_cfg_cmd_list *hw_update);

boolean scaler46_util_append_stream_info(mct_port_t *port,
  mct_stream_info_t *stream_info);

boolean scaler46_util_remove_stream_info(mct_port_t *port,
  uint32_t identity);

boolean scaler46_util_single_HW_write(void* cmd_offset,
  uint32_t cmd_len, uint32_t hw_reg_offset, uint32_t reg_num,
  uint32_t cmd_data_offset, uint32_t cmd_type,
  struct msm_vfe_cfg_cmd_list **hw_update_list);

boolean scaler46_util_send_event(mct_module_t *module,
  mct_event_t *event);

boolean scaler46_util_setloglevel(const char *name,
  isp_log_sub_modules_t mod_id);

#endif
