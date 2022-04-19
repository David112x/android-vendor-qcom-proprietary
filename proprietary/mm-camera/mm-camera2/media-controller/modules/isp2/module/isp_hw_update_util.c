/* isp_hw_update_list.c
 *
 * Copyright (c) 2012-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* kernel headers */
#include <media/msmb_isp.h>

/* isp headers */
#include "isp_hw_update_util.h"
#include "isp_util.h"
#include "isp_log.h"
#include "camera_dbg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

int bet_hw_params_fd[ISP_HW_MAX]; //used for BET only

/** isp_hw_update_util_debug:
 *
 *  @hw_update_list: hw update list to be printed
 *
 *  Print hw update list
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_hw_update_util_debug(
  struct msm_vfe_cfg_cmd_list **hw_update_list)
{
  boolean ret = TRUE;
  struct msm_vfe_cfg_cmd_list *list = NULL;
  list = *hw_update_list;
  ISP_DBG("hw_update_list %p, head %p", hw_update_list, list);
  if (list) {
    while (list->next) {
      ISP_DBG("next %p ,(size: %d, cmd_len %d)",
        list->next, list->next_size, list->cfg_cmd.cmd_len);
      list = list->next;
    }
  }
  return ret;
} /* isp_hw_update_util_debug */

/** isp_hw_update_util_enqueue:
 *
 *  @hw_update_params: hw update params where hw update list
 *                   is stored per ISP
 *  @hw_update_list: hw update list to be enqued
 *  @hw_id: ISP hw ID
 *
 *  Enqueue hw update list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_hw_update_util_enqueue(
  isp_hw_update_params_t *hw_update_params,
  struct msm_vfe_cfg_cmd_list *hw_update_list, isp_hw_id_t hw_id)
{
  isp_hw_update_list_params_t *hw_update_list_params = NULL;
  struct msm_vfe_cfg_cmd_list *head = NULL, *next = NULL;

  if (!hw_update_params || !hw_update_list || (hw_id >= ISP_HW_MAX)) {
    ISP_ERR("failed: %p %p hw_id %d", hw_update_params, hw_update_list, hw_id);
    return FALSE;
  }

  /* Enqueue in hw update list */
   PTHREAD_MUTEX_LOCK(&hw_update_params->mutex);
  hw_update_list_params = &hw_update_params->hw_update_list_params[hw_id];
  if (!hw_update_list_params->cur_hw_update_list) {
    hw_update_list_params->cur_hw_update_list = hw_update_list;
  } else {
    ISP_ERR("line_dbg new hw update list is not NULL!");
    head = hw_update_list_params->cur_hw_update_list;
    while (head->next) {
      head = head->next;
    }
    head->next = hw_update_list;
    head->next_size = sizeof(*hw_update_list);
  }
  PTHREAD_MUTEX_UNLOCK(&hw_update_params->mutex);

  return TRUE;
} /* isp_hw_update_util_enqueue */

/** isp_hw_update_util_free:
 *
 *  @hw_update_list: hw update list
 *
 *  Free hw update list params
 *
 *  Return none
 **/
static void isp_hw_update_util_free(struct msm_vfe_cfg_cmd_list *hw_update_list)
{
  struct msm_vfe_cfg_cmd_list *head = NULL, *next = NULL;

  head = hw_update_list;
  while (head) {
    next = head->next;
    ISP_LOG_LIST("free hw_update %p cfg_cmd %p", head, head->cfg_cmd.cfg_cmd);
    free(head->cfg_cmd.cfg_data);
    head->cfg_cmd.cfg_data = NULL;
    free(head->cfg_cmd.cfg_cmd);
    head->cfg_cmd.cfg_cmd = NULL;
    free(head);
    head = next;
  }
} /* isp_hw_update_util_free */

/** isp_hw_update_util_free_cur:
 *
 *  @hw_update_params: hw update params
 *
 *  Free cur hw update list params
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_hw_update_util_free_cur(isp_hw_update_params_t *hw_update_params,
  isp_hw_id_t hw_id)
{
  isp_hw_update_list_params_t *hw_params = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update_list = NULL;

  if (!hw_update_params) {
    ISP_ERR("failed: hw_update_params %p", hw_update_params);
    return FALSE;
  }

  /* Drain hw update queue */
   PTHREAD_MUTEX_LOCK(&hw_update_params->mutex);
  hw_params = &hw_update_params->hw_update_list_params[hw_id];
  hw_update_list = hw_params->cur_hw_update_list;
  if (hw_update_list) {
    isp_hw_update_util_free(hw_update_list);
    hw_params->cur_hw_update_list = NULL;
  }
   PTHREAD_MUTEX_UNLOCK(&hw_update_params->mutex);

  return TRUE;
}

/** isp_hw_update_util_free_all:
 *
 *  @hw_update_params: hw update params
 *
 *  Free hw update list params
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_hw_update_util_free_all(isp_hw_update_params_t *hw_update_params)
{
  isp_hw_update_list_params_t *hw_params = NULL;
  isp_hw_id_t                  hw_index;
  struct msm_vfe_cfg_cmd_list *hw_update_list = NULL;

  if (!hw_update_params) {
    ISP_ERR("failed: hw_update_params %p", hw_update_params);
    return FALSE;
  }

  /* Drain hw update queue */
   PTHREAD_MUTEX_LOCK(&hw_update_params->mutex);
  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    hw_params = &hw_update_params->hw_update_list_params[hw_index];
    hw_update_list = hw_params->cur_hw_update_list;
    if (hw_update_list) {
      isp_hw_update_util_free(hw_update_list);
      hw_params->cur_hw_update_list = NULL;
    }
  }
   PTHREAD_MUTEX_UNLOCK(&hw_update_params->mutex);

  return TRUE;
} /* isp_hw_update_util_free_all */

/** isp_hw_update_util_request_reg_update:
 *
 *  @session_param: session param handle
 *
 * Send Reg update CMD
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_hw_update_util_request_reg_update(
  isp_session_param_t *session_param)
{
  boolean                      ret = TRUE;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_hw_update_list_params_t *hw_params = NULL;
  isp_hw_id_t                  hw_index;
  int                          rc = 0;
  uint32_t                     frame_id;
  enum msm_vfe_input_src       frame_src = VFE_PIX_0;

  if (!session_param) {
    ISP_ERR("failed: session_param %p", session_param);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;

  PTHREAD_MUTEX_LOCK(&session_param->state_mutex);
  frame_id = session_param->curr_frame_id;
  session_param->reg_update_info.reg_update_state =
    ISP_REG_UPDATE_STATE_REQUESTED;
  PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);

  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_id_t session_hw_id = session_param->hw_id[hw_index];
    /* For offline ISP reg update is issued in separate call */
    if (session_param->offline_num_isp > 0 &&
      session_param->offline_hw_id[0] == session_hw_id)
      continue;

    if (session_hw_id >= ISP_HW_MAX)
      continue;

    PTHREAD_MUTEX_LOCK(&hw_update_params->mutex);
    hw_params = &hw_update_params->hw_update_list_params[session_hw_id];
    if (hw_params->fd <= 0) {
      PTHREAD_MUTEX_UNLOCK(&hw_update_params->mutex);
      continue;
    }
    PTHREAD_MUTEX_UNLOCK(&hw_update_params->mutex);

    rc = ioctl(hw_params->fd, VIDIOC_MSM_ISP_REG_UPDATE_CMD,
           &frame_src);
    if (rc < 0) {
      ISP_ERR("failed: ret %d", ret);
      ret = FALSE;
      break;
    }
  }

  if (session_param->curr_frame_id > frame_id) {
    ISP_DBG("warning: reg_cmd ioctl happened on frame boundary");
  }

  return ret;
}

/** isp_hw_update_util_do_ioctl_in_hw_update_params:
 *
 *  @isp_resource: ISP resource handle
 *  @hw_update_params: hw update params for all ISP's
 *
 *  Perform ioctl on all ISP's if hw update params are
 *  available
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_hw_update_util_do_ioctl_in_hw_update_params(
  isp_resource_t *isp_resource, isp_session_param_t *session_param)
{
  boolean                      ret = TRUE;
  int32_t                      rc = 0;
  isp_hw_id_t                  hw_index = 0;
  isp_hw_update_list_params_t *hw_params = NULL;
  struct msm_vfe_cfg_cmd_list *head = NULL, *next = NULL;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_module_enable_info_t    *cur_module_enable_info = NULL;

  if (!isp_resource || !session_param) {
    ISP_ERR("failed: invalid param %p %p", isp_resource, session_param);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;

  if (hw_update_params->skip_hw_update == TRUE) {
    ISP_ERR("<cds_dbg> failed! skip hw update!");
    return TRUE;
  }

  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_id_t session_hw_id = session_param->hw_id[hw_index];
    if (session_param->offline_num_isp > 0 &&
      session_hw_id == session_param->offline_hw_id[0])
      continue;
    if (session_hw_id >= ISP_HW_MAX)
      continue;
    hw_params = &hw_update_params->hw_update_list_params[session_hw_id];
    if (hw_params->fd <= 0) {
      continue;
    }
    bet_hw_params_fd[session_hw_id] = hw_params->fd;
    /* If module_cfg is needed, call pipeline function to perform
     * separate ioctl
     */

    cur_module_enable_info = &hw_params->stats_params->module_enable_info;

    if (cur_module_enable_info->reconfig_needed == TRUE) {
      ret = isp_resource_update_module_cfg(isp_resource, session_hw_id, hw_params->fd,
        cur_module_enable_info, session_param->sensor_fmt);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_resource_update_module_cfg hw_id %d", session_hw_id);
      }
      ret = isp_util_send_submod_enable_to_pipeline(isp_resource,
        session_param);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_util_send_submod_enable_to_pipeline\n");
      }
      cur_module_enable_info->reconfig_needed = FALSE;
    }
    if (hw_params->cur_hw_update_list) {
//      isp_hw_update_util_debug(&hw_params->cur_hw_update_list);
      rc = ioctl(hw_params->fd, VIDIOC_MSM_VFE_REG_LIST_CFG,
        hw_params->cur_hw_update_list);
      if (rc < 0) {
        ISP_ERR("failed: rc %d", rc);
        ret = FALSE;
        break;
      }
#ifdef VIDIOC_MSM_ISP_AHB_CLK_CFG
      isp_pipeline_t *isp_pipeline = isp_resource->isp_resource_info[session_hw_id].isp_pipeline;
      if (isp_pipeline->is_ahb_clk_cfg_supported == TRUE) {
        if (!session_param->fast_aec_mode &&
          MSM_ISP_CAMERA_AHB_SVS_VOTE != hw_params->ahb_clk_vote) {
          struct msm_isp_ahb_clk_cfg ahb_cgf;
          ahb_cgf.vote = MSM_ISP_CAMERA_AHB_SVS_VOTE;
          rc = ioctl(hw_params->fd, VIDIOC_MSM_ISP_AHB_CLK_CFG, &ahb_cgf);
          if (rc < 0) {
            ISP_ERR("failed: to set ahb cfg to %x error %d\n",
                    MSM_ISP_CAMERA_AHB_SVS_VOTE, errno);
            ret = FALSE;
            break;
          }
          hw_params->ahb_clk_vote = MSM_ISP_CAMERA_AHB_SVS_VOTE;
        }
      }
#endif

    }

    ret = isp_hw_update_util_free_cur(&session_param->hw_update_params,
      session_hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed!");
    }
  }

  return ret;
}

/** isp_hw_update_util_do_ioctl:
 *
 *  @fd: fd of specific hw
 *  @hw_update_list: hw update list to be passed to kernel
 *
 *  Perform ioctl on given fd and free hw_update_list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_hw_update_util_do_ioctl(int32_t fd,
  struct msm_vfe_cfg_cmd_list *hw_update_list)
{
  boolean ret = TRUE;
  int32_t rc = 0;

  if ((fd < 0) || !hw_update_list) {
    ISP_ERR("failed: fd %d hw_update_list %p", fd, hw_update_list);
    return FALSE;
  }

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_LIST_CFG, hw_update_list);
  if (rc < 0) {
    ISP_ERR("failed: rc %d", rc);
    ret = FALSE;
    goto ERROR_IOCTL;
  }

  isp_hw_update_util_free(hw_update_list);

  return ret;

ERROR_IOCTL:
  isp_hw_update_util_free(hw_update_list);
  return ret;
} /* isp_hw_update_util_do_ioctl */

/** isp_hw_update_util_do_ioctl_in_hw_id_update_params:
 *
 *  @isp_resource: ISP resource handle
 *  @hw_update_params: hw update params for all ISP's
 *
 *  Perform ioctl on all ISP's if hw update params are
 *  available
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_hw_update_util_do_ioctl_in_hw_id_update_params(
  isp_resource_t *isp_resource, isp_session_param_t *session_param,
  isp_hw_id_t hw_id)
{
  boolean                      ret = TRUE;
  int32_t                      rc = 0;
  isp_hw_update_list_params_t *hw_params = NULL;
  struct msm_vfe_cfg_cmd_list *head = NULL, *next = NULL;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_module_enable_info_t    *cur_module_enable_info = NULL;
  enum msm_vfe_input_src       frame_src = VFE_PIX_0;
  cam_format_t                input_format;

  if (!isp_resource || !session_param) {
    ISP_ERR("failed: invalid param %p %p", isp_resource, session_param);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;

  PTHREAD_MUTEX_LOCK(&hw_update_params->mutex);
  if (hw_update_params->skip_hw_update == TRUE) {
    ISP_ERR("failed! skip hw update!");
    PTHREAD_MUTEX_UNLOCK(&hw_update_params->mutex);
    return TRUE;
  }
  PTHREAD_MUTEX_UNLOCK(&hw_update_params->mutex);

  hw_params = &hw_update_params->hw_update_list_params[hw_id];
  if (hw_params->fd <= 0) {
    ISP_ERR(" VFE%d fd %x", hw_id, hw_params->fd);
    return FALSE;
  }
  /* If module_cfg is needed, call pipeline function to perform
   * separate ioctl
   */
  cur_module_enable_info = &hw_params->stats_params->module_enable_info;
  if (cur_module_enable_info->reconfig_needed == TRUE) {
    if (session_param->offline_num_isp > 0 &&
      session_param->offline_hw_id[0] == hw_id) {
      input_format = session_param->offline_input_cfg.output_format;
    } else {
      input_format = session_param->sensor_fmt;
    }
    ret = isp_resource_update_module_cfg(isp_resource, hw_id,
      hw_params->fd, cur_module_enable_info, input_format);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_resource_update_module_cfg hw_id %d", hw_id);
    }
    cur_module_enable_info->reconfig_needed = FALSE;
  }
  if (hw_params->cur_hw_update_list) {
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_ISP_HW_UPDATE);
    isp_hw_update_util_debug(&hw_params->cur_hw_update_list);
    rc = ioctl(hw_params->fd, VIDIOC_MSM_VFE_REG_LIST_CFG,
      hw_params->cur_hw_update_list);
    ATRACE_CAMSCOPE_END(CAMSCOPE_ISP_HW_UPDATE);
    if (rc < 0) {
      ISP_ERR("failed: rc %d", rc);
    }
    /* Issue ioctl to offline ISP */
    rc = ioctl(hw_params->fd, VIDIOC_MSM_ISP_REG_UPDATE_CMD,
           &frame_src);
    if (rc < 0) {
      ISP_ERR(" REG_UPDATE_CMD failed: ret %d", ret);
    }
  }

  ret = isp_hw_update_util_free_cur(hw_update_params, hw_id);
  if (ret == FALSE) {
    ISP_ERR("failed!");
  }


  return ret;
}
