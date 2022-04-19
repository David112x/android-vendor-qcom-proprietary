/******************************************************************************

                   N E T M G R _ T A R G E T . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_target.c
  @brief   Target related implementations

  DESCRIPTION
  Implementation of NetMgr's target related function.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <dlfcn.h>
#include "netmgr.h"
#include "netmgr_util.h"
#include "netmgr_exec.h"
#include "netmgr_cb.h"
#include "netmgr_qmi_cb.h"
#include "netmgr_config.h"
#include "netmgr_ssr.h"
#include "netmgr_target.h"
#include "netmgr_qmi_dms.h"

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define NETMGR_TARGET_CB_NAME "TARGET"

#define NETMGR_EXT_TARGET_INIT_FUNC "__netmgr_ext_target_init"
typedef int netmgr_ext_target_init_f
(
  const char * phys_net_dev,
  void       * (*modem_is_handler)(void *),
  void       * (*modem_oos_handler)(void *)
);

static netmgr_qmi_cb_ep_info_t ep_info;

/*===========================================================================
                      INTERNAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION:  __netmgr_target_is_handler
===========================================================================*/
/*!
@brief
  Default In-service handler

@return
  None

@note

*/
/*=========================================================================*/
static void *__netmgr_target_is_handler
(
  void *arg
)
{
  netmgr_exec_cmd_t *cmd = NULL;
  int i = 0;

  (void) arg;

#if defined(FEATURE_DS_LINUX_ANDROID) && !defined(NETMGR_OFFTARGET)
  netmgr_util_wakelock_acquire();
#endif

  if (NETMGR_GET_MODEM_SSR_STATE() != NETMGR_MODEM_OOS_STATE)
  {
    netmgr_log_med("Not in SSR, ignoring service up event!\n");
    return NULL;
  }

  /* Update the SSR state */
  NETMGR_SET_MODEM_SSR_STATE(NETMGR_MODEM_IS_STATE);

  /* Post sys_ind cmd to do rmnet global init */
  /* Posting QMI Indication command on the first active link */
  for (i = 0; i < netmgr_main_cfg.max_links_in_total; ++i)
  {
    if (TRUE == netmgr_qmi_cfg.link_array[i].enabled)
    {
      break;
    }
  }

  if (i == netmgr_main_cfg.max_links_in_total)
  {
    netmgr_log_err("Cannot find any active link");
    return NULL;
  }

  /* allocate a command object */
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);

  /* set command object parameters */
  cmd->data.type = NETMGR_QMI_MSG_CMD;
  cmd->data.link = i;
  cmd->data.info.qmi_msg.type = NETMGR_QMI_SYS_IND_CMD;
  cmd->data.info.qmi_msg.data.sys_ind.event_id = NETMGR_QMI_SYS_EVENT_MODEM_IN_SERVICE_IND;
  cmd->data.info.qmi_msg.data.sys_ind.link = 0;

  /* Post command for processing in the command thread context */
  if (NETMGR_SUCCESS != netmgr_exec_put_cmd(cmd))
  {
    netmgr_log_err("Failed to put commmand\n");
    netmgr_exec_release_cmd(cmd);
  }
  /* End - global init command posting */

  netmgr_l2s_format_msg(0, "rcvd %s","MDM_IS_IND");

  /* Service is back up, post IN_SERVICE to command thread*/
  for (i = 0; i < netmgr_main_cfg.max_links_in_total; i++)
  {
    if (FALSE == netmgr_qmi_cfg.link_array[i].enabled)
    {
      /* Link not enabled don't post cmd */
      continue;
    }

    netmgr_log_high("link[%d] active. Posting cmd to netmgr_exec\n", i);
    /* allocate a command object */
    cmd = netmgr_exec_get_cmd();
    NETMGR_ASSERT(cmd);

    /* set command object parameters */
    cmd->data.type = NETMGR_MODEM_IS_EV;
    cmd->data.link = i;

    cmd->data.info.qmi_msg.type = NETMGR_QMI_SYS_IND_CMD;
    cmd->data.info.qmi_msg.data.sys_ind.event_id = NETMGR_QMI_SYS_EVENT_MODEM_IN_SERVICE_IND;
    cmd->data.info.qmi_msg.data.sys_ind.link = 0;

    /* Post command for processing in the command thread context */
    if (NETMGR_SUCCESS != netmgr_exec_put_cmd(cmd))
    {
      netmgr_log_err("Failed to put commmand\n");
      netmgr_exec_release_cmd(cmd);
    }
  }

  netmgr_l2s_format_msg(0, "Posted %s\\nfor all links","MDM_IS_IND");

  /* Post IS_COMPLETE message after processing of all links */
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);
  cmd->cmd.execute_f = netmgr_exec_process_msg ;
  cmd->data.type = NETMGR_IS_COMPLETE_MSG ;
  cmd->data.link = 0;

  if (NETMGR_FAILURE == netmgr_exec_put_cmd(cmd))
  {
    NETMGR_ABORT("Unable to post is_complete msg. We cant recover from here.");
  }

  netmgr_qmi_dms_send_info();

  return NULL;
}

/*===========================================================================
  FUNCTION:  __netmgr_target_oos_handler
===========================================================================*/
/*!
@brief
  Default Out-of-service handler

@return
  None

@note

*/
/*=========================================================================*/
void *__netmgr_target_oos_handler
(
  void *arg
)
{
  netmgr_exec_cmd_t *cmd = NULL;
  int i = 0;

  (void) arg;

#if defined(FEATURE_DS_LINUX_ANDROID) && !defined(NETMGR_OFFTARGET)
  netmgr_util_wakelock_acquire();
#endif

  /* Post sys_ind cmd to do rmnet global clean up */
  /* Posting QMI Indication command on the first active link */
  for (i = 0; i < netmgr_main_cfg.max_links_in_total; ++i)
  {
    if (TRUE == netmgr_ctl_port_array[i].enabled)
    {
      break;
    }
  }

  if (i == netmgr_main_cfg.max_links_in_total)
  {
    netmgr_log_err("Cannot find any active link");
    return NULL;
  }

  /* Update the SSR state */
  NETMGR_SET_MODEM_SSR_STATE(NETMGR_MODEM_OOS_STATE);
  NETMGR_DECR_PROCESS_READY_REQ();

  /* allocate a command object */
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);

  /* set command object parameters */
  cmd->data.type = NETMGR_QMI_MSG_CMD;
  cmd->data.link = i;
  cmd->data.info.qmi_msg.type = NETMGR_QMI_SYS_IND_CMD;
  cmd->data.info.qmi_msg.data.sys_ind.event_id = NETMGR_QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND;
  cmd->data.info.qmi_msg.data.sys_ind.link = 0;

  /* Post command for processing in the command thread context */
  if (NETMGR_SUCCESS != netmgr_exec_put_cmd(cmd))
  {
    netmgr_log_err("Failed to put commmand\n");
    netmgr_exec_release_cmd(cmd);
  }
  /* End - global cleanup command posting */

  /* Search for active links and post sm cmd for each link */
  netmgr_l2s_format_msg(0, "rcvd %s", "MDM_OOS_IND");

  for (i = 0; i < netmgr_main_cfg.max_links_in_total; i++)
  {
    if (FALSE == netmgr_ctl_port_array[i].enabled)
    {
      /* Link not enabled don't post cmd */
      continue;
    }
    netmgr_log_high("link[%d] active. Posting cmd to netmgr_exec\n", i);
    /* allocate a command object */
    cmd = netmgr_exec_get_cmd();
    NETMGR_ASSERT(cmd);

    /* set command object parameters */
    cmd->data.type = NETMGR_MODEM_OOS_EV;
    cmd->data.link = i;

    cmd->data.info.qmi_msg.type = NETMGR_QMI_SYS_IND_CMD;
    cmd->data.info.qmi_msg.data.sys_ind.event_id = NETMGR_QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND;
    cmd->data.info.qmi_msg.data.sys_ind.link = 0;

    /* Post command for processing in the command thread context */
    if (NETMGR_SUCCESS != netmgr_exec_put_cmd(cmd))
    {
      netmgr_log_err("failed to put commmand\n");
      netmgr_exec_release_cmd(cmd);
    }
  }

  netmgr_l2s_format_msg(0, "Posted %s\\nfor all links","MDM_OOS_IND");

  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);
  cmd->cmd.execute_f = netmgr_exec_process_msg ;
  cmd->data.type = NETMGR_OOS_COMPLETE_MSG ;
  cmd->data.link = 0;

  if (NETMGR_FAILURE == netmgr_exec_put_cmd(cmd))
  {
    netmgr_log_err("Unable to post oos_complete msg");
    netmgr_exec_release_cmd(cmd);
  }

  return NULL;
}

/*===========================================================================
  FUNCTION: netmgr_target_is_handler
===========================================================================*/
/*!
@brief
  Modem in-service handler

@return
  None

@note

*/
/*=========================================================================*/
static void *netmgr_target_is_handler
(
  void *arg
)
{
  ep_info.qmi_inst = (qmi_service_instance)((intptr_t)arg);

  __netmgr_target_is_handler(arg);

  return NULL;
}

/*===========================================================================
  FUNCTION: netmgr_target_oos_handler
===========================================================================*/
/*!
@brief
  Modem out-of-service handler

@return
  None

@note

*/
/*=========================================================================*/
static void *netmgr_target_oos_handler
(
  void *arg
)
{
  (void)arg;

  netmgr_cb_runall(NETMGR_QMI_CB_TABLE,
                   NETMGR_QMI_CB_EP_CLOSING,
                   &ep_info);

  __netmgr_target_oos_handler(arg);

  return NULL;
}

/*===========================================================================
  FUNCTION: netmgr_target_ready_handler
===========================================================================*/
/*!
@brief
  Modem ready handler (modem port opened)

@return
  None

@note

*/
/*=========================================================================*/
static void *netmgr_target_ready_handler
(
  void *arg
)
{
  (void)arg;

  netmgr_cb_runall(NETMGR_QMI_CB_TABLE,
                   NETMGR_QMI_CB_EP_OPENED,
                   &ep_info);

  return NULL;
}

/*===========================================================================
  FUNCTION: netmgr_default_target_init
===========================================================================*/
/*!
@brief
  Default target initialization

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE

@note

*/
/*=========================================================================*/
static int netmgr_default_target_init()
{
  ds_ep_type_t tmp_ep_type = 0;
  int          tmp_ep_id = 0;
  int          ret = NETMGR_FAILURE;

  ds_get_epid(netmgr_main_cfg.phys_net_dev, &tmp_ep_type, &tmp_ep_id);
  ep_info.qmi_inst = 0; // Will be set after modem in-service
  ep_info.ep_type = (uint32_t)tmp_ep_type;
  ep_info.iface_id = (uint32_t)tmp_ep_id;
  ep_info.data_net_dev = netmgr_main_cfg.phys_net_dev;

  do
  {
    /* Set modem IS/OOS handlers */
    netmgr_cb_insert_cb(NETMGR_QMI_CB_TABLE,
                        NETMGR_QMI_CB_MODEM_IS_EX,
                        NETMGR_TARGET_CB_NAME,
                        netmgr_target_is_handler);

    netmgr_cb_insert_cb(NETMGR_QMI_CB_TABLE,
                        NETMGR_QMI_CB_MODEM_OOS_EX,
                        NETMGR_TARGET_CB_NAME,
                        netmgr_target_oos_handler);

    /* Set modem ready (port opened) handler */
    netmgr_cb_insert_cb(NETMGR_QMI_CB_TABLE,
                        NETMGR_QMI_CB_MODEM_IS_COMPLETE,
                        NETMGR_TARGET_CB_NAME,
                        netmgr_target_ready_handler);

    /* Initialize SSR module and wait for modem up */
    if (NETMGR_FAILURE == netmgr_ssr_init(
               NULL, TRUE, QMI_CLIENT_INSTANCE_ANY, 0))
    {
      netmgr_log_err("SSR module init failed\n");
      break;
    }

    ret = NETMGR_SUCCESS;

  } while (0);

  return ret;
}

/*===========================================================================
                      EXTERNAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION: netmgr_target_init
===========================================================================*/
/*!
@brief
  Initialize target

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE

@note

*/
/*=========================================================================*/
int netmgr_target_init()
{
  ds_target_t target;
  char *err = NULL;
  void *dl_handle = NULL;
  const char *lib_name = NULL;
  netmgr_ext_target_init_f *ext_target_init = NULL;

  target = ds_get_target();
  if ((DS_TARGET_FUSION_SM8150_SDX50 == target) ||
      (DS_TARGET_FUSION_SDA855_SDX55 == target) ||
      (DS_TARGET_FUSION_SDA865_SDX55 == target))
  {
    lib_name = "libnetmgr_nr_fusion.so";
  }

  if (NULL != lib_name)
  {
    do
    {
      dl_handle = dlopen(lib_name, RTLD_NOW);
      if (NULL == dl_handle)
      {
          err = (char *)dlerror();
          if(err != NULL)
          {
              netmgr_log_err("Failed to open %s: %s", lib_name, err);
          }
          break;
      }

      ext_target_init = (netmgr_ext_target_init_f *)
                   dlsym(dl_handle, NETMGR_EXT_TARGET_INIT_FUNC);

      if (NULL == ext_target_init)
      {
          err = (char *)dlerror();
          if(err != NULL)
          {
              netmgr_log_err("%s not found in %s: %s",
                      NETMGR_EXT_TARGET_INIT_FUNC, lib_name, err);
          }
        break;
      }

      netmgr_log_high("%s ext target init...", ds_get_target_str(target));
      return ext_target_init(netmgr_main_cfg.phys_net_dev,
                             __netmgr_target_is_handler,
                             __netmgr_target_oos_handler);
    } while (0);
  }

  netmgr_log_high("Default target init...");
  return netmgr_default_target_init();
}
