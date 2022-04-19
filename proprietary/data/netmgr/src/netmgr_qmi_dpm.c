/**
  @file
  netmgr_qmi_dpm.c

  @brief
  Netmgr QMI DPM (QMI Data Port Mapper service) implementation.

  @details
  Netmgr QMI DPM component is used to configure embedded ports on the modem.
  The configuration specifies how many ports to open, what are the control
  ports and what are the data ports (hw-accel and software data ports). It
  can also specify the mapping between control and data ports. The configuration
  is choosen through configdb or hard-coded.

  The following functions are exported:
  netmgr_qmi_dpm_init()
  netmgr_qmi_dpm_open_port()
  netmgr_qmi_dpm_close_port()

  The ports are opened using QCCI APIs (they must go over IPC-router since
  QMUXD transport would not be ready yet).

*/
/*===========================================================================

  Copyright (c) 2013-2016, 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/03/13   hm/nd   Initial version

===========================================================================*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include "ds_util.h"
#include "ds_cmdq.h"
#include <linux/msm_rmnet.h>
#include "data_port_mapper_v01.h"
#include "qmi_client.h"
#include "qmi_cci_target_ext.h"
#include "qmi_client_instance_defs.h"
#include "netmgr_main.h"
#include "netmgr_qmi_dpm.h"
#include "netmgr_netlink.h"
#include "netmgr_util.h"
#include "netmgr_cmdq.h"
#include "ds_string.h"
#include <cutils/properties.h>
#include "netmgr_rmnet.h"
#include "netmgr_qmi.h"
#include "netmgr_exec.h"
#include "netmgr_config.h"

/*===========================================================================

                               DATA DEFINITIONS

===========================================================================*/

#define NETMGR_QMI_DPM_TIMEOUT        10000
#define NETMGR_QMI_DPM_MAX_RETRIES    5

typedef struct
{
  boolean is_shim_supported;
}netmgr_qmi_dpm_modem_cap_t;

typedef struct
{
  /* configdb related information */
  boolean use_configdb;
  boolean configdb_inited;

  /* QMI portmapper service info */
  qmi_client_type             dpm_clid;
  boolean                     dpm_inited;

  /* Last sent open_req/close_reqs (for debug purposes) */
  dpm_open_port_req_msg_v01   open_req;
  boolean                     open_req_sent;
  dpm_close_port_req_msg_v01  close_req;

  boolean                     modem_cap_queried;
  netmgr_qmi_dpm_modem_cap_t  modem_cap;
} netmgr_qmi_dpm_state_t;

static netmgr_qmi_dpm_state_t netmgr_qmi_dpm_state;

qmi_client_os_params         dpm_os_params;
qmi_client_os_params         dpm_os_params_main;

typedef struct
{
  unsigned char             dpm_service_is_up;
  pthread_t                 thread_id;
  pthread_mutex_t           cnd_mutex;
  pthread_cond_t            cnd_var;
} netmgr_qmi_dpm_signal_t;

static int netmgr_qmi_dpm_process_modem_caps( void );

static int netmgr_qmi_dpm_port_close(void);

/*===========================================================================
  FUNCTION  netmgr_qmi_dpm_get_use_qmuxd
===========================================================================*/
/*!
@brief
  Check android property to force use qmuxd even in presence of shim support
  in modem.

@return
  TRUE if force use qmuxd set to TRUE, FALSE otherwise

@note
  - Side Effects
*/
/*=========================================================================*/
static int
netmgr_qmi_get_use_qmuxd( void )
{
  netmgr_log_med("netmgrd: %s: use_qmuxd [%d]", __func__ , netmgr_main_cfg.use_qmuxd);
  return netmgr_main_cfg.use_qmuxd;
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_process_sys_event
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Called in response to DPM events as part of command executor module

    @params None.
    @return None.
*/
/*=========================================================================*/
void netmgr_qmi_dpm_process_sys_event
(
  netmgr_dpm_event_t   evt
)
{
  switch (evt)
  {
  case NETMGR_QMI_DPM_OOS_EV:
    /* We have received SSR notification, clean up existing DPM
     * handle. */

    netmgr_log_med("%s(): Releasing dpm client",__func__);
    (void) qmi_client_release(netmgr_qmi_dpm_state.dpm_clid);

    /* Reset internal flags */
    netmgr_qmi_dpm_state.dpm_inited = FALSE;
    netmgr_qmi_dpm_state.open_req_sent = FALSE;
    /* Clean local state */
    netmgr_qmi_dpm_state.modem_cap_queried = FALSE;
    memset(&netmgr_qmi_dpm_state.modem_cap, 0x0,
           sizeof(netmgr_qmi_dpm_modem_cap_t));
    break;

  case NETMGR_QMI_DPM_IS_EV:
    /* We have received SSR recovery message */
    netmgr_log_med("%s(): Initializing DPM client and opening port...",__func__);

    netmgr_log_med("%s(): Unregistering QMUX transport", __func__);
    qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
    qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
    qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
    qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);
    netmgr_qmi_dpm_initalize();
    break;

  default:
    netmgr_log_err("%s(): Unsupported operation", __func__);
    break;
  }
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_init
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Initializes the QMI DPM module.

    @details
    Performs the QCCI client library initialization to be able to send
    and receive DPM messages.

    @params None.
    @return NETMGR_SUCCESS: in case of success.
    @return NETMGR_FAILURE: in case of error.
    @dependencies None.
*/
static int netmgr_qmi_dpm_init_client(void)
{
  qmi_idl_service_object_type  dpm_svc_obj;
  qmi_client_error_type        rc;

  /* Initialize QCCI client to send the open_port request */
  dpm_svc_obj = dpm_get_service_object_v01();
  if (dpm_svc_obj == NULL)
  {
    netmgr_log_err("%s(): DPM service not available", __func__);
    return NETMGR_FAILURE;
  }

  memset(&dpm_os_params, 0, sizeof(dpm_os_params));
  rc = qmi_client_init_instance(dpm_svc_obj,
                                QMI_CLIENT_INSTANCE_ANY,
                                NULL,
                                NULL,
                                &dpm_os_params,
                                netmgr_main_cfg.dpm_retry_timeout,
                                &netmgr_qmi_dpm_state.dpm_clid);

  if (rc != QMI_NO_ERR)
  {
    netmgr_log_err("%s(): qmi_client_init_instance failed, err %d\n", __func__, rc);
    return NETMGR_FAILURE;
  }

  netmgr_log_med("netmgr_qmi_dpm_init(): qmi_client_init_instance SUCCESS, "
      "client: 0x%x\n", (unsigned int)netmgr_qmi_dpm_state.dpm_clid);

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_port_open
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Forms a dpm_open_port_req_msg_v01 message and sends to the modem.

    @details
    This method forms a dpm_open_port_req_msg_v01 and sends it to Q6 modem.

    This method needs to be sent over IPC router transport since QMUX would
    not be available during boot time. Once this message is received by Q6
    modem, it is going to open the ports on its ends and QMUXD boot-up is
    unblocked.

    The configuration used is from configdb XML file if it exists. Otherwise
    default hardcoded configuration is utilized.

    @params None.
    @return NETMGR_SUCCESS: in case of success.
    @return NETMGR_FAILURE: in case of error.

    @dependencies
    netmgr_qmi_dpm_init should be complete.
*/
static int netmgr_qmi_dpm_port_open(void)
{
  dpm_open_port_req_msg_v01* open_req = NULL;
  dpm_open_port_resp_msg_v01 open_resp;
  int rc = NETMGR_FAILURE;
  int ret = NETMGR_FAILURE;
  qmi_client_error_type qmi_err = QMI_NO_ERR;

  netmgr_log_med("netmgr_qmi_dpm_port_open(): ENTRY\n");

#ifdef NETMGR_DPM_CONFIG_DB
  open_req = netmgr_qmi_dpm_port_open_config_db();
#else

  if (TRUE != netmgr_qmi_dpm_state.modem_cap_queried)
  {
    netmgr_log_med("%s: process modem capability", __func__);
    ret = netmgr_qmi_dpm_process_modem_caps();
    if ( NETMGR_SUCCESS != ret)
    {
       netmgr_log_err("%s: error querying modem capability", __func__);
       netmgr_log_med("%s: proceeding with default modem cap", __func__);
    }
  }
  else
  {
    netmgr_log_med("%s: modem cap queried already. continuing dpm_port_open",
                    __func__);
  }

  open_req = (dpm_open_port_req_msg_v01 *)
    netmgr_malloc(sizeof(dpm_open_port_req_msg_v01));
  if (NULL == open_req)
  {
    netmgr_log_err("failed to allocate memory for port_open_msg\n");
    goto bail;
  }

  if (netmgr_qmi_dpm_state.open_req_sent)
  {
    netmgr_log_med("Ports already opened, closing them first\n");
    if (NETMGR_SUCCESS != netmgr_qmi_dpm_port_close())
    {
      netmgr_log_med("Close port failed, exiting!\n");
      goto bail;
    }
    netmgr_qmi_dpm_state.open_req_sent = FALSE;
  }

  memset(open_req, 0, sizeof(dpm_open_port_req_msg_v01));
  if (FALSE == netmgr_qmi_dpm_state.modem_cap.is_shim_supported ||
      TRUE == netmgr_qmi_get_use_qmuxd())
  {
    /* Open appropriate control channels when shim layer support
       is not present in the modem. */
    netmgr_log_med("%s: %s add control ports to open req",__func__,
                    (TRUE == netmgr_qmi_dpm_state.modem_cap.is_shim_supported ?
                    "Force use QMUXD" :"Shim not supported!"));
    open_req->control_port_list_valid = TRUE;
    open_req->control_port_list_len = 1;
    strlcpy (open_req->control_port_list[0].port_name, netmgr_main_cfg.smd_ch_name,
             QMI_DPM_PORT_NAME_MAX_V01);
    open_req->control_port_list[0].default_ep_id.ep_type = netmgr_main_cfg.ep_type;
    open_req->control_port_list[0].default_ep_id.iface_id = (uint32_t)netmgr_main_cfg.epid;
  }
  else
  {
    netmgr_log_med("%s: Shim supported!Not opening control ports",__func__);
  }

  open_req->hardware_data_port_list_valid = TRUE;
  open_req->hardware_data_port_list_len = 1;
  open_req->hardware_data_port_list[0].ep_id.ep_type =  netmgr_main_cfg.ep_type;
  open_req->hardware_data_port_list[0].ep_id.iface_id = (uint32_t)netmgr_main_cfg.epid;
  open_req->hardware_data_port_list[0].hardware_ep_pair.consumer_pipe_num = (uint32_t)netmgr_main_cfg.consumer_pipe_num;
  open_req->hardware_data_port_list[0].hardware_ep_pair.producer_pipe_num = (uint32_t)netmgr_main_cfg.producer_pipe_num;

  open_req->software_data_port_list_valid = FALSE;
#endif

  /* Send open_req */
  qmi_err = qmi_client_send_msg_sync(netmgr_qmi_dpm_state.dpm_clid,
                                     QMI_DPM_OPEN_PORT_REQ_V01,
                                     (void *)open_req,
                                     sizeof(dpm_open_port_req_msg_v01),
                                     (void*)&open_resp,
                                     sizeof(dpm_open_port_resp_msg_v01),
                                     NETMGR_QMI_DPM_TIMEOUT);
  if (QMI_NO_ERR != qmi_err)
  {
    netmgr_log_err("Send open_req failed, err %d\n", qmi_err);

    /* Open request failed */
    netmgr_qmi_dpm_state.open_req_sent = FALSE;
    goto bail;
  }

  /* Open request was sent successfully */
  netmgr_qmi_dpm_state.open_req_sent = TRUE;
  rc = NETMGR_SUCCESS;

bail:
  /* Store open_req for debug purposes */
  if (rc == NETMGR_SUCCESS && NULL != open_req)
  {
    netmgr_qmi_dpm_state.open_req = *open_req;
  }

  if (NULL != open_req)
  {
    netmgr_free(open_req);
  }

  netmgr_log_med("netmgr_qmi_dpm_port_open(): EXIT, rc %d\n", rc);
  return rc;
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_port_close
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Forms a dpm_close_port_req_msg_v01 message and sends to the modem.

    @details
    This method forms a dpm_close_port_req_msg_v01 and sends it to Q6 modem.
    Message is sent using IPC router transport.

    @params None.
    @return NETMGR_SUCCESS: in case of success.
    @return NETMGR_FAILURE: in case of error.

    @dependencies
    netmgr_qmi_dpm_init should be complete.
    netmgr_qmi_dpm_port_open should be complete, otherwise this is a NOP.
*/
static int netmgr_qmi_dpm_port_close(void)
{
  netmgr_log_err("netmgr_qmi_dpm_port_close(): Not supported currently\n");
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_process_modem_caps
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Query modem for capabilities supported by modem.

    @details
    Request modem to provide information about the capabilities it supports.

    @params None.
    @return None.
*/
static int netmgr_qmi_dpm_process_modem_caps(void)
{
#ifdef FEATURE_QMUXD_DEPRECATION
  dpm_get_supported_capabilities_req_msg_v01 cap_req;
  dpm_get_supported_capabilities_resp_msg_v01 cap_resp;
  int ret = NETMGR_FAILURE;
  qmi_client_error_type qmi_err = QMI_NO_ERR;

  netmgr_log_low("%s: Entry",__func__);

  memset( &cap_req, 0x0, sizeof(dpm_get_supported_capabilities_req_msg_v01));
  memset( &cap_resp, 0x0, sizeof(dpm_get_supported_capabilities_resp_msg_v01));

  qmi_err = qmi_client_send_msg_sync(netmgr_qmi_dpm_state.dpm_clid,
                                     QMI_DPM_GET_SUPPORTED_CAPABILITIES_REQ_V01,
                                     (void*) &cap_req,
                                     sizeof(dpm_get_supported_capabilities_req_msg_v01),
                                     (void*) &cap_resp,
                                     sizeof(dpm_get_supported_capabilities_resp_msg_v01),
                                     NETMGR_QMI_DPM_TIMEOUT);
  if (QMI_NO_ERR != qmi_err)
  {
    netmgr_log_err("%s: error getting supported capabilities qmi_err[%d]",
                   __func__, qmi_err);
    goto bail;
  }

  if (cap_resp.resp.result != QMI_RESULT_SUCCESS_V01)
  {
    netmgr_log_err("%s(): Query capability failed with result [%d], err [0x%x]",
                   __func__, cap_resp.resp.result, cap_resp.resp.error);

    /* Since querying modem capability did not work, we need to use iwlan client */
    netmgr_main_cfg.iwlan_client_enable = TRUE;
    goto bail;
  }

  netmgr_qmi_dpm_state.modem_cap_queried = TRUE;
  if (TRUE == cap_resp.is_shim_supported_valid &&
      TRUE == cap_resp.is_shim_supported )
  {
    netmgr_qmi_dpm_state.modem_cap.is_shim_supported = TRUE;
    netmgr_log_med("%s: Shim supported on modem", __func__);
  }
  else
  {
    netmgr_log_med("%s:Shim layer not supported", __func__);
  }

  /* Check whether modem supports wlan event sync capability
     If modem supports sync capability then we need to disable
     iwlan client and CNE would be responsible for sending WLAN
     indication to modem. If sync capability is not supported
     then we fall back to enabling the iwlan client within netmgr */
  if (TRUE == cap_resp.is_rev_ip_sync_supported_valid &&
        TRUE == cap_resp.is_rev_ip_sync_supported)
  {
    netmgr_log_low("%s(): sync capability supported", __func__);
    netmgr_main_cfg.iwlan_client_enable = FALSE;
  }
  else
  {
    netmgr_log_low("%s(): sync capability not supported", __func__);
    netmgr_main_cfg.iwlan_client_enable = TRUE;
  }

  ret = NETMGR_SUCCESS;
bail:
  netmgr_log_low("%s: Exit ret[%d]", __func__, ret);
  return ret;
#else
  return NETMGR_SUCCESS;
#endif
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_initialize
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Initializes the QMI DPM module.

    @details
    Performs the QCCI client library initialization and sends the open
    request to the Modem

    @params None.
    @return NETMGR_SUCCESS: in case of success.
    @return NETMGR_FAILURE: in case of error.
    @dependencies QMUXD transports should be unregistered before entering this
    function.
*/
void netmgr_qmi_dpm_initalize(void)
{
  boolean dpm_inited = netmgr_qmi_dpm_state.dpm_inited;

  while (FALSE == dpm_inited)
  {
    netmgr_qmi_dpm_state.open_req_sent = FALSE;
    memset(&netmgr_qmi_dpm_state.open_req, 0, sizeof(netmgr_qmi_dpm_state.open_req));
    memset(&netmgr_qmi_dpm_state.close_req, 0, sizeof(netmgr_qmi_dpm_state.close_req));

    if (NETMGR_SUCCESS != netmgr_qmi_dpm_init_client())
    {
      netmgr_log_err("%s(): DPM client init failed", __func__);
      continue;
    }

    if (NETMGR_SUCCESS != netmgr_qmi_dpm_port_open())
    {
      netmgr_log_err("%s(): DPM port open failed", __func__);
      (void) qmi_client_release(netmgr_qmi_dpm_state.dpm_clid);
      continue;
    }

    dpm_inited = TRUE;
  }

  netmgr_qmi_dpm_state.dpm_inited = dpm_inited;

  /* Check whether open request was sent successfully before registering back transports
   * It is possible for modem to go down in between dpm_init and dpm_port_open and in that
   * case we cannot register back the transports because it will block the notifier thread
   * within qmi framework */
  if (FALSE == netmgr_qmi_dpm_state.modem_cap.is_shim_supported ||
      TRUE == netmgr_qmi_get_use_qmuxd() )
  {
    netmgr_log_med("netmgr_qmi_dpm_init(): Open request successful,"
                   " registering back QMUX transports");
    qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
    qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
    qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
    qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);
  }
  else
  {
    netmgr_log_med("%s Not using qmuxd. Not re-registering for QMUX transport",
                   __func__);
  }
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_initialize_blocking
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Initializes the QMI DPM module. Use this function only from the main
    thread init sequence and not anywhere else.

    @details
    This is a wrapper function netmgr_qmi_dpm_initialize_blocking over
    netmgr_qmi_dpm_initialize and this function only additionally
    waits for the DPM service to be UP before rquesting a service
    client from QCCI. This is to avoid tight looping in the cases
    where the modem is not presumed to be UP and running always.

    @params None.
    @return NETMGR_SUCCESS: in case of success.
    @return NETMGR_FAILURE: in case of error.
    @dependencies None.
*/
int netmgr_qmi_dpm_initialize_blocking(void)
{
  netmgr_log_med("%s(): Unregistering QMUX transport", __func__);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);

  netmgr_qmi_dpm_initalize();

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_log_state
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Prints debug messages about QMI DPM state in netmgr.

    @details
    Prints information on whether QMI DPM is inited, what was the last
    message sent etc.

    @params None.
    @return None.
*/
void netmgr_qmi_dpm_log_state(void)
{

  dpm_open_port_req_msg_v01   *open_req;
  unsigned int i;

  netmgr_log_med("====netmgr_qmi_dpm_log_state====\n");
  netmgr_log_med("....use_config_db: %d\n", netmgr_qmi_dpm_state.use_configdb);
  netmgr_log_med("....configdb_inited: %d\n", netmgr_qmi_dpm_state.configdb_inited);
  netmgr_log_med("....dpm_clid: %d\n", (uint32_t)(netmgr_qmi_dpm_state.dpm_clid));
  netmgr_log_med("....dpm_inited: %d\n", netmgr_qmi_dpm_state.dpm_inited);
  netmgr_log_med("....modem_cap_queried: %d\n", netmgr_qmi_dpm_state.modem_cap_queried);
  netmgr_log_med("....modem_cap.is_shim_supported: %d\n",
                 netmgr_qmi_dpm_state.modem_cap.is_shim_supported);
  netmgr_log_med("....close_req: %p",&netmgr_qmi_dpm_state.close_req);
  netmgr_log_med("....open_req_sent: %d\n", netmgr_qmi_dpm_state.open_req_sent);
  netmgr_log_med("....open_req:\n");

  open_req = &netmgr_qmi_dpm_state.open_req;
  netmgr_log_med("......control_port_list_valid: %d\n", open_req->control_port_list_valid);
  netmgr_log_med("......control_port_list_len: %d\n", open_req->control_port_list_len);
  netmgr_log_med("......control_port_list:\n");
  for (i = 0; i < open_req->control_port_list_len && i < QMI_DPM_PORT_MAX_NUM_V01; i++)
  {
    netmgr_log_med("........control_port_list[%d].port_name: %s\n", i, open_req->control_port_list[i].port_name);
    netmgr_log_med("........control_port_list[%d].default_ep_id.ep_type: %d\n", i, open_req->control_port_list[i].default_ep_id.ep_type);
    netmgr_log_med("........control_port_list[%d].default_ep_id.iface_id: 0x%x\n", i, open_req->control_port_list[i].default_ep_id.iface_id);
  }

  netmgr_log_med("......hardware_data_port_list_valid: %d\n", open_req->hardware_data_port_list_valid);
  netmgr_log_med("......hardware_data_port_list_len: %d\n", open_req->hardware_data_port_list_len);
  for (i = 0; i < open_req->hardware_data_port_list_len && i < QMI_DPM_PORT_MAX_NUM_V01; i++)
  {
    netmgr_log_med("........hardware_data_port_list[%d].ep_id.ep_type: %d\n", i, open_req->hardware_data_port_list[i].ep_id.ep_type);
    netmgr_log_med("........hardware_data_port_list[%d].ep_id.iface_id: 0x%x\n", i, open_req->hardware_data_port_list[i].ep_id.iface_id);
    netmgr_log_med("........hardware_data_port_list[%d].hardware_ep_pair.consumer_pipe_num: %d\n", i, open_req->hardware_data_port_list[i].hardware_ep_pair.consumer_pipe_num);
    netmgr_log_med("........hardware_data_port_list[%d].hardware_ep_pair.producer_pipe_num: %d\n", i, open_req->hardware_data_port_list[i].hardware_ep_pair.producer_pipe_num);
  }

  netmgr_log_med("......software_data_port_list_valid: %d\n", open_req->software_data_port_list_valid);
  netmgr_log_med("====netmgr_qmi_dmp_log_state====\n");
}
