/* isp_hw_update_util.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ISP_HW_UPDATE_UTIL_H__
#define __ISP_HW_UPDATE_UTIL_H__

/* kernel headers */
#include <media/msmb_isp.h>

/* mctl headers */
#include "mtype.h"

/* isp headers */
#include "isp_module.h"
#include "isp_defs.h"

boolean isp_hw_update_util_enqueue(
  isp_hw_update_params_t *hw_update_params,
  struct msm_vfe_cfg_cmd_list *hw_update_list, isp_hw_id_t hw_id);

boolean isp_hw_update_util_do_ioctl_in_hw_update_params(
  isp_resource_t *isp_resource, isp_session_param_t *session_param);

boolean isp_hw_update_util_do_ioctl(int32_t fd,
  struct msm_vfe_cfg_cmd_list *hw_update_list);

boolean isp_hw_update_util_request_reg_update(
  isp_session_param_t *session_param);

boolean isp_hw_update_util_free_all(isp_hw_update_params_t *hw_update_params);

boolean isp_hw_update_util_free_cur(isp_hw_update_params_t *hw_update_params,
  isp_hw_id_t hw_id);

boolean isp_hw_update_util_do_ioctl_in_hw_id_update_params(
  isp_resource_t *isp_resource, isp_session_param_t *session_param,
  isp_hw_id_t hw_id);

#endif
