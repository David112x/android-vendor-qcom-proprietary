/* template_util.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __DEMOSAIC40_UTIL_H__
#define __DEMOSAIC40_UTIL_H__

/* mctl headers*/
#include "mtype.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_stream.h"

/* isp headers */
#include <media/msmb_isp.h>

boolean template_util_get_port_from_module(mct_module_t *module,
  mct_port_t **port, mct_port_direction_t direction, uint32_t identity);

boolean template_util_append_cfg(struct msm_vfe_cfg_cmd_list **hw_update_list,
  struct msm_vfe_cfg_cmd_list *hw_update);

boolean template_util_append_stream_info(
  template_t *session_param, mct_stream_info_t *stream_info);

boolean template_util_remove_stream_info(template_t *session_param,
  uint32_t identity);

boolean template_util_get_stream_info(template_t *session_param,
  uint32_t identity, mct_stream_info_t **stream_info);

boolean template_util_get_session_param(template_priv_t *template_priv,
  unsigned int session_id, template_t **session_param);

boolean template_util_create_session_param(template_priv_t *template_priv,
  unsigned int session_id, template_t **template);

boolean template_util_remove_session_param(template_priv_t *template_priv,
  unsigned int session_id);

#endif
