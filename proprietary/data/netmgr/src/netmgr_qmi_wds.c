/******************************************************************************

                       N E T M G R _ Q M I _W D S . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_qmi_wds.c
  @brief   Network Manager QMI wireless data service helper

  DESCRIPTION
  Network Manager QMI wireless data service helper

******************************************************************************/
/*===========================================================================

  Copyright (c) 2015-2016, 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include "qmi_client.h"
#include "qmi_cci_target.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_common.h"
#include "netmgr_exec.h"
#include "netmgr_main.h"
#include "qmi_client_instance_defs.h"
#include "qmi_platform.h"
#include "qmi_client_helper.h"
#include "netmgr_qmi_wds.h"
#include "netmgr_util.h"
#include "netmgr_kif.h"
#include "qmi_i.h"
#include "qmi_util.h"


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

static qmi_cci_os_signal_type  netmgr_qmi_wds_os_params;

#define NETMGR_QMI_RESULT_CODE_TYPE_ID       0x0002

#define NETMGR_QMI_WDS_ROUTE_LOOK_UP_MSG_ID          0xFFF2

/* Route look-up response TLV ID's */
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_TYPE_REQ_TLV_ID  0x01
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_IFACE_TLV_ID     0x01
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_QMI_INST_TLV_ID  0x10
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_MUX_ID_TLV_ID    0x13

/* Parameter indication bits */
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_TECH_PREF_PARAM                      0x00000001
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_PROFILE_IDX_PARAM                    0x00000002
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_APN_NAME_PARAM                       0x00000004
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_IP_FAMILY_PREF_PARAM                 0x00000008
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_PROFILE_IDX_3GPP2_PARAM              0x00000010
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_XTENDED_TECH_PREF_PARAM              0x00000020
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_DATA_CALL_ORIGIN_PARAM               0x00000040
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_APN_TYPE_PARAM                       0x00000080
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_APN_TYPE_MASK_PARAM                  0x00000100
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_BRINGUP_BY_APN_NAME_PARAM            0x00000200
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_BRINGUP_BY_APN_TYPE_PARAM            0x00000400

/* Start NW Interface TLV ID's */
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_APN_NAME_REQ_TLV_ID                  0x14
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_IP_FAMILY_PREF_REQ_TLV_ID            0x19
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_TECH_PREF_REQ_TLV_ID                 0x30
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_PROFILE_IDX_REQ_TLV_ID               0x31
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_PROFILE_IDX_3GPP2_REQ_TLV_ID         0x32
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_XTENDED_TECH_PREF_REQ_TLV_ID         0x34
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_DATA_CALL_TYPE_TLV_ID                0x35
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_APN_TYPE_REQ_TLV_ID                  0x38
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_BRINGUP_BY_APN_NAME_REQ_TLV_ID       0x3C
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_BRINGUP_BY_APN_TYPE_REQ_TLV_ID       0x3D
#define NETMGR_QMI_WDS_ROUTE_LOOKUP_APN_TYPE_MASK_REQ_TLV_ID             0x3E

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_ind_cb
===========================================================================*/
/*!
@brief
 Main callback function regisgterd during client init. It posts a command to
 do the required processing in the Command Thread context.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static
void netmgr_qmi_wds_ind_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                           *ind_buf,
  unsigned int                   ind_buf_len,
  void                           *ind_cb_data
)
{
  int link;
  netmgr_exec_cmd_t *cmd = NULL;
  qmi_client_error_type   qmi_err = QMI_INTERNAL_ERR;
  void *dest = NULL;
  int dest_len = 0;

  NETMGR_LOG_FUNC_ENTRY;

  /* Get link id from user data ptr */
  link = (int)((uintptr_t)ind_cb_data);

  /* Verify link id */
  NETMGR_ASSERT(netmgr_kif_verify_link(link) == NETMGR_SUCCESS);

  /* Allocate a command object */
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);

#if defined(FEATURE_DS_LINUX_ANDROID) && !defined(NETMGR_OFFTARGET)
  netmgr_util_wakelock_acquire();
#endif

  netmgr_log_med("%s(): received indication=0x%x on link=%d\n", __func__, msg_id, link);

  switch (msg_id) {
  case QMI_WDS_PKT_SRVC_STATUS_IND_V01:
    dest = &cmd->data.info.qmi_msg.data.wds_ind.ind_data.pkt_srvc_status;
    dest_len = sizeof(cmd->data.info.qmi_msg.data.wds_ind.ind_data.pkt_srvc_status);
    break;

  case QMI_WDS_EVENT_REPORT_IND_V01:
    dest = &cmd->data.info.qmi_msg.data.wds_ind.ind_data.event_report;
    dest_len = sizeof(cmd->data.info.qmi_msg.data.wds_ind.ind_data.event_report);
    break;

  case QMI_WDS_EXTENDED_IP_CONFIG_IND_V01:
    dest = &cmd->data.info.qmi_msg.data.wds_ind.ind_data.ext_ip_ind;
    dest_len = sizeof(cmd->data.info.qmi_msg.data.wds_ind.ind_data.ext_ip_ind);
    break;

#ifdef FEATURE_DATA_IWLAN
  case QMI_WDS_REVERSE_IP_TRANSPORT_CONNECTION_IND_V01:
    dest = &cmd->data.info.qmi_msg.data.wds_ind.ind_data.rev_ip_conn_ind,
    dest_len = sizeof(cmd->data.info.qmi_msg.data.wds_ind.ind_data.rev_ip_conn_ind);
    break;
#endif /* FEATURE_DATA_IWLAN */

  case QMI_WDS_CONFIGURED_THROUGHPUT_INFO_IND_V01:
    dest = &cmd->data.info.qmi_msg.data.wds_ind.ind_data.tput_info_ind,
    dest_len = sizeof(cmd->data.info.qmi_msg.data.wds_ind.ind_data.tput_info_ind);
    break;

  case QMI_WDS_REBIND_DEFAULT_FLOW_INFO_IND_V01:
    dest = &cmd->data.info.qmi_msg.data.wds_ind.ind_data.rebind_ind,
    dest_len = sizeof(cmd->data.info.qmi_msg.data.wds_ind.ind_data.rebind_ind);
    break;

  default:
    netmgr_log_err("received unsupported indication type 0x%x\n", msg_id);
    goto bail;
  }

  qmi_err = qmi_client_message_decode(user_handle,
                                      QMI_IDL_INDICATION,
                                      msg_id,
                                      ind_buf,
                                      ind_buf_len,
                                      dest,
                                      dest_len);
  if (QMI_NO_ERR != qmi_err) {
    goto bail;
  }

  cmd->data.type                     = NETMGR_QMI_MSG_CMD;
  cmd->data.link                     = link;
  cmd->data.info.qmi_msg.type        = NETMGR_QMI_WDS_IND_CMD;
  cmd->data.info.qmi_msg.data.wds_ind.user_hndl = user_handle;
  cmd->data.info.qmi_msg.data.wds_ind.link   = link;
  cmd->data.info.qmi_msg.data.wds_ind.ind_id = msg_id;

  /* Post command for processing in the command thread context */
  if (NETMGR_SUCCESS != netmgr_exec_put_cmd(cmd)) {
    NETMGR_ABORT("%s(): failed to put commmand\n", __func__);
    goto bail;
  }

  return;

bail:
  if (NULL != cmd) {
    netmgr_exec_release_cmd(cmd);
  }
  NETMGR_LOG_FUNC_EXIT;
}

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_init_route_look_up_client
===========================================================================*/
/*!
@brief
  Initialize QMI Route look up WDS handle

@return
  int - NETMGR_QMI_WDS_SUCCESS on successful operation,
        NETMGR_QMI_WDS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_init_route_look_up_client
(
  int                      link,
  const char               *dev_str,
  qmi_client_type          *clnt_hndl
)
{
  qmi_client_error_type rc = QMI_INTERNAL_ERR;

  (void)link;

  if (!clnt_hndl || !dev_str) {
    netmgr_log_err("%s(): invalid input params!\n", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_FUNC_ENTRY;

  memset(&netmgr_qmi_wds_os_params, 0, sizeof(qmi_cci_os_signal_type));

  *clnt_hndl = NETMGR_QMI_CLIENT_INVALID;

  rc = qmi_client_wds_init_instance(dev_str,
                                    wds_get_service_object_v01(),
                                    QMI_CLIENT_INSTANCE_ANY,
                                    NULL,
                                    NULL,
                                    &netmgr_qmi_wds_os_params,
                                    NETMGR_QMI_TIMEOUT,
                                    clnt_hndl);
  if (QMI_NO_ERR != rc) {
    netmgr_log_err("%s(): failed on qmi_client_wds_init_instance with rc=%d!\n", __func__, rc);
    goto bail;
  }

  NETMGR_LOG_FUNC_EXIT;

  return NETMGR_SUCCESS;

bail:
  if (NETMGR_QMI_CLIENT_INVALID != *clnt_hndl) {
    qmi_client_release(*clnt_hndl);
  }

  NETMGR_LOG_FUNC_EXIT;

  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_init_client
===========================================================================*/
/*!
@brief
  Initialize QMI WDS handle

@return
  int - NETMGR_QMI_WDS_SUCCESS on successful operation,
        NETMGR_QMI_WDS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_init_client
(
  int                      link,
  const char               *dev_str,
  wds_ip_family_enum_v01   ip_family,
  qmi_client_type          *clnt_hndl
)
{
  qmi_client_error_type rc = QMI_INTERNAL_ERR;

  if (!clnt_hndl || !dev_str) {
    netmgr_log_err("%s(): invalid input params!\n", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_FUNC_ENTRY;

  memset(&netmgr_qmi_wds_os_params, 0, sizeof(qmi_cci_os_signal_type));

  *clnt_hndl = NETMGR_QMI_CLIENT_INVALID;

  rc = qmi_client_wds_init_instance(dev_str,
                                    wds_get_service_object_v01(),
                                    QMI_CLIENT_INSTANCE_ANY,
                                    netmgr_qmi_wds_ind_cb,
                                    (void *)(uintptr_t)link,
                                    &netmgr_qmi_wds_os_params,
                                    NETMGR_QMI_TIMEOUT,
                                    clnt_hndl);
  if (QMI_NO_ERR != rc) {
    netmgr_log_err("%s(): failed on qmi_client_wds_init_instance with rc=%d!\n", __func__, rc);
    goto bail;
  } else {
    wds_set_client_ip_family_pref_req_msg_v01 req;
    wds_set_client_ip_family_pref_resp_msg_v01 resp;

    memset(&req, 0, sizeof(req));
    memset(&resp, 0, sizeof(resp));

    req.ip_preference = ip_family;

    /* Set the ip preference */
    rc = qmi_client_send_msg_sync(*clnt_hndl,
                                  QMI_WDS_SET_CLIENT_IP_FAMILY_PREF_REQ_V01,
                                  (void *)&req,
                                  sizeof(req),
                                  (void *)&resp,
                                  sizeof(resp),
                                  NETMGR_QMI_TIMEOUT);

    if (rc != QMI_NO_ERR || QMI_RESULT_SUCCESS_V01 != resp.resp.result) {
      netmgr_log_err("%s(): failed to set ip_family=%d with rc=%d result=%d err=%d!\n",
                     __func__, ip_family, rc, resp.resp.result, resp.resp.error);
      goto bail;
    }
  }

  NETMGR_LOG_FUNC_EXIT;

  return NETMGR_SUCCESS;

bail:
  if (NETMGR_QMI_CLIENT_INVALID != *clnt_hndl) {
    qmi_client_release(*clnt_hndl);
  }

  NETMGR_LOG_FUNC_EXIT;

  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_release_client
===========================================================================*/
/*!
@brief
  Release QMI WDS handle

@return
  int - NETMGR_QMI_WDS_SUCCESS on successful operation,
        NETMGR_QMI_WDS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_release_client
(
  int                      link,
  qmi_client_type          clnt_hndl
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_hndl) {
    netmgr_log_err("%s(): invalid clnt_hndl on link=%d!\n",
                   __func__, link);
    goto bail;
  }

  rc = qmi_client_release(clnt_hndl);

  if (QMI_NO_ERR != rc) {
    netmgr_log_err("%s(): failed to release clnt_hndl=%p on link=%d rc=%d!\n",
                   __func__, clnt_hndl, link, rc);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_ind_reg
===========================================================================*/
/*!
@brief
  Register for WDS indications

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_ind_reg
(
  int                      link,
  qmi_client_type          clnt_hndl,
  wds_ip_family_enum_v01   ip_family
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_hndl) {
    netmgr_log_err("%s(): invalid clnt_hndl on link=%d!\n",
                   __func__, link);
    goto bail;
  }

  wds_indication_register_req_msg_v01 req;
  wds_indication_register_resp_msg_v01 resp;

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  req.report_extended_ip_config_change_valid = TRUE;
  req.report_extended_ip_config_change = TRUE;

#ifdef FEATURE_DS_LINUX_ANDROID
  if (netmgr_main_get_dynamic_tc_ul_baserate())
  {
    if ((link == 0) && (ip_family == WDS_IP_FAMILY_IPV4_V01))
    {
      req.report_configured_throughput_valid = 1;
      req.report_configured_throughput = 1;
    }
  }
#endif /* FEATURE_DS_LINUX_ANDROID */

  req.report_default_flow_rebind_valid = 1;
  req.report_default_flow_rebind = 1;

  /* Send registration */
  rc = qmi_client_send_msg_sync(clnt_hndl,
                                QMI_WDS_INDICATION_REGISTER_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void *)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR || QMI_RESULT_SUCCESS_V01 != resp.resp.result) {
    netmgr_log_err("%s(): failed for link=%d with rc=%d result=%d err=%d!\n",
                   __func__, link, rc, resp.resp.result, resp.resp.error);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_bind_subscription
===========================================================================*/
/*!
@brief
  Bind the subscription to WDS_DONT_CARE_SUBS_V01

@return
  int - NETMGR_QMI_WDS_SUCCESS on successful operation,
        NETMGR_QMI_WDS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_bind_subscription
(
  int                      link,
  qmi_client_type          clnt_hndl
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret = NETMGR_FAILURE;
  wds_bind_subscription_req_msg_v01 req;
  wds_bind_subscription_resp_msg_v01 resp;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_hndl) {
    netmgr_log_err("%s(): invalid clnt_hndl on link=%d!\n",
                   __func__, link);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  req.subscription = WDS_DONT_CARE_SUBS_V01;

  /* Send ext ip config registration */
  rc = qmi_client_send_msg_sync(clnt_hndl,
                                QMI_WDS_BIND_SUBSCRIPTION_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void *)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR || QMI_RESULT_SUCCESS_V01 != resp.resp.result) {
    netmgr_log_err("%s(): failed for link=%d with rc=%d result=%d err=%d!\n",
                   __func__, link, rc, resp.resp.result, resp.resp.error);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_bearer_tech_ex_ind_reg
===========================================================================*/
/*!
@brief
  Register for bearer tech ex indications

@return
  int - NETMGR_QMI_WDS_SUCCESS on successful operation,
        NETMGR_QMI_WDS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_bearer_tech_ex_ind_reg
(
  int                      link,
  qmi_client_type          clnt_hndl
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret = NETMGR_FAILURE;
  wds_set_event_report_req_msg_v01 req;
  wds_set_event_report_resp_msg_v01 resp;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_hndl) {
    netmgr_log_err("%s(): invalid clnt_hndl on link=%d!\n",
                   __func__, link);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  req.report_data_bearer_tech_ex_valid = TRUE;
  req.report_data_bearer_tech_ex = TRUE;

  /* Send bearer tech ex registration */
  rc = qmi_client_send_msg_sync(clnt_hndl,
                                QMI_WDS_SET_EVENT_REPORT_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void *)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR || QMI_RESULT_SUCCESS_V01 != resp.resp.result) {
    netmgr_log_err("%s(): failed for link=%d with rc=%d result=%d err=%d!\n",
                   __func__, link, rc, resp.resp.result, resp.resp.error);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_get_runtime_settings
===========================================================================*/
/*!
@brief
  Query the Modem for runtime settings

@return
  int - NETMGR_QMI_WDS_SUCCESS on successful operation,
        NETMGR_QMI_WDS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_get_runtime_settings
(
  int                                    link,
  qmi_client_type                        clnt_hndl,
  uint32_t                               req_mask,
  wds_get_runtime_settings_resp_msg_v01  *runtime_set
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret = NETMGR_FAILURE;
  wds_get_runtime_settings_req_msg_v01 req;
  wds_get_runtime_settings_resp_msg_v01 resp;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_hndl) {
    netmgr_log_err("%s(): invalid clnt_hndl on link=%d!\n",
                   __func__, link);
    goto bail;
  }

  if (!runtime_set) {
    netmgr_log_err("%s(): invalid input!\n",
                   __func__);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  req.requested_settings_valid = TRUE;
  req.requested_settings = req_mask;

  /* Send bearer tech ex registration */
  rc = qmi_client_send_msg_sync(clnt_hndl,
                                QMI_WDS_GET_RUNTIME_SETTINGS_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void *)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR) {
    netmgr_log_err("%s(): failed for link=%d with rc=%d result=%d err=%d!\n",
                   __func__, link, rc, resp.resp.result, resp.resp.error);
    goto bail;
  }

  *runtime_set = resp;

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
    FUNCTION  netmgr_qmi_wds_set_bind_subscription
===========================================================================*/
/*!
@brief
  Bind the subscription to given configuration

@return
  int - NETMGR_SUCCESS     on successful operation
        NETMGR_FAILURE     if there was an error sending QMI message

*/
/*=========================================================================*/
int
netmgr_qmi_wds_set_bind_subscription
(
  qmi_client_type                     clnt_handle,
  wds_bind_subscription_enum_v01      subs_id
)
{
  int rc;
  int ret = NETMGR_FAILURE;
  wds_bind_subscription_req_msg_v01   req;
  wds_bind_subscription_resp_msg_v01  resp;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_handle) {
    netmgr_log_err("%s(): invalid clnt_hndl \n",
                   __func__);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  req.subscription = subs_id;

  rc = qmi_client_send_msg_sync (clnt_handle,
                                 QMI_WDS_BIND_SUBSCRIPTION_REQ_V01,
                                 (void *)&req,
                                 sizeof(req),
                                 (void*)&resp,
                                 sizeof(resp),
                                 NETMGR_QMI_TIMEOUT);

  if ( rc != QMI_NO_ERR || resp.resp.result != QMI_RESULT_SUCCESS_V01)
  {
    netmgr_log_err("%s(): failed with rc=%d, qmi_err=%d",
                   __func__, rc, resp.resp.error);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}



/*===========================================================================
    FUNCTION  netmgr_qmi_wds_set_client_ip_pref
===========================================================================*/
/*!
@brief
  Set IP family preference for a client

@return
  int - NETMGR_SUCCESS   on successful operation
        NETMGR_FAILURE   if there was an error sending QMI message

*/
/*=========================================================================*/
static int
netmgr_qmi_wds_set_client_ip_pref
(
  qmi_client_type           clnt_handle,
  wds_ip_family_enum_v01    ip_family
)
{
  int rc;
  int ret = NETMGR_FAILURE;
  wds_set_client_ip_family_pref_req_msg_v01 req;
  wds_set_client_ip_family_pref_resp_msg_v01 resp;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_handle) {
    netmgr_log_err("%s(): invalid clnt_hndl \n",
                   __func__);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  req.ip_preference = ip_family;

  rc = qmi_client_send_msg_sync(clnt_handle,
                                QMI_WDS_SET_CLIENT_IP_FAMILY_PREF_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void*)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR || resp.resp.result != QMI_RESULT_SUCCESS_V01)
  {
    netmgr_log_err("%s(): failed with rc=%d, qmi_err=%d",
                   __func__, rc, resp.resp.error);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}


/*===========================================================================
    FUNCTION  netmgr_qmi_wds_get_pkt_srvc_status
===========================================================================*/
/*!
@brief
  Get the packet service status from the modem

@return
  int - DSI_SUCCESS   on successful operation
        DSI_ERROR   if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_get_pkt_srvc_status
(
  qmi_client_type              clnt_handle,
  wds_ip_family_enum_v01       ip_family,
  netmgr_link_status_type     *pkt_srvc_status
)
{
  int rc;
  int ret = NETMGR_FAILURE;
  wds_get_pkt_srvc_status_req_msg_v01 req;
  wds_get_pkt_srvc_status_resp_msg_v01 resp;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_handle) {
    netmgr_log_err("%s(): invalid clnt_hndl \n",
                   __func__);
    goto bail;
  }

  if (!pkt_srvc_status)
  {
    netmgr_log_err("%s(): Invalid input. pkt_srvc_status=[%p]\n",
                   __func__, pkt_srvc_status);
    goto bail;
  }

  if(NETMGR_SUCCESS != netmgr_qmi_wds_set_client_ip_pref(clnt_handle,
                                                         ip_family))
  {
    netmgr_log_err("%s(): failed to set client IP preference", __func__);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  rc = qmi_client_send_msg_sync(clnt_handle,
                                QMI_WDS_GET_PKT_SRVC_STATUS_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void*)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR || resp.resp.result != QMI_RESULT_SUCCESS_V01)
  {
    if(resp.resp.result != QMI_RESULT_SUCCESS_V01 &&
        resp.resp.error == QMI_ERR_INCOMPATIBLE_STATE_V01)
    {
      netmgr_log_med("%s(): iface is busy on other subs. "
                    "Setting srvc status as connected", __func__);
      *pkt_srvc_status = WDS_CONNECTION_STATUS_CONNECTED_V01;
    }
    else
    {
      netmgr_log_err("%s(): failed with rc=%d, qmi_err=%d",
                     __func__, rc, resp.resp.error);
      goto bail;
    }
  }
  else
  {
    *pkt_srvc_status = (netmgr_link_status_type)resp.connection_status;
  }
  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}



/*===========================================================================
    FUNCTION  netmgr_qmi_wds_bind_mux_data_port
===========================================================================*/
/*!
@brief
  Bind Mux ID for a give data port instance

@return
  int - NETMGR_SUCCESS   on successful operation
        NETMGR_FAILURE   if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_bind_mux_data_port
(
  qmi_client_type                           clnt_handle,
  const wds_bind_mux_data_port_req_msg_v01 *params
)
{
  int rc;
  int ret = NETMGR_FAILURE;
  wds_bind_mux_data_port_resp_msg_v01 resp;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_handle) {
    netmgr_log_err("%s(): invalid clnt_hndl \n",
                   __func__);
    goto bail;
  }

  if (!params)
  {
    netmgr_log_err("%s(): Invalid input. params [%p]\n", __func__, params);
    goto bail;
  }

  memset(&resp, 0, sizeof(resp));

  rc = qmi_client_send_msg_sync(clnt_handle,
                                QMI_WDS_BIND_MUX_DATA_PORT_REQ_V01,
                                (void *)params,
                                sizeof(*params),
                                (void*)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if ( rc != QMI_NO_ERR || resp.resp.result != QMI_RESULT_SUCCESS_V01)
  {
    netmgr_log_err("%s(): failed with rc=%d, qmi_err=%d\n",
                   __func__, rc, resp.resp.error);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}


/*===========================================================================
  FUNCTION  netmgr_qmi_wds_write_optional_route_look_up_tlvs
===========================================================================*/
/*!
@brief
  Takes the input UMTS profile data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int
netmgr_qmi_wds_write_optional_route_look_up_tlvs
(
  unsigned char                        **tx_buf,
  int                                   *tx_buf_len,
  netmgr_qmi_wds_route_look_up_params_type *params
)
{
  wds_call_type_enum_v01    data_call_origin;
  int ret = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  /* Profile index param */
  if (params->params_mask & NETMGR_QMI_WDS_ROUTE_LOOKUP_PROFILE_IDX_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                NETMGR_QMI_WDS_ROUTE_LOOKUP_PROFILE_IDX_REQ_TLV_ID,
                                1,
                                (void *)&params->profile_index) < 0)
    {
      goto bail;
    }
  }
  if (params->params_mask & NETMGR_QMI_WDS_ROUTE_LOOKUP_PROFILE_IDX_3GPP2_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                NETMGR_QMI_WDS_ROUTE_LOOKUP_PROFILE_IDX_3GPP2_REQ_TLV_ID,
                                1,
                                (void *)&params->profile_index_3gpp2) < 0)
    {
      goto bail;
    }
  }
  /* APN name */
  if (params->params_mask & NETMGR_QMI_WDS_ROUTE_LOOKUP_APN_NAME_PARAM)
  {
    size_t name_len = strlen (params->apn_name);
    if (name_len > QMI_WDS_APN_NAME_MAX_V01)
    {
      goto bail;
    }

    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                NETMGR_QMI_WDS_ROUTE_LOOKUP_APN_NAME_REQ_TLV_ID,
                                name_len,
                                (void *)params->apn_name) < 0)
    {
      goto bail;
    }
  }

  if (params->params_mask & NETMGR_QMI_WDS_ROUTE_LOOKUP_XTENDED_TECH_PREF_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                NETMGR_QMI_WDS_ROUTE_LOOKUP_XTENDED_TECH_PREF_REQ_TLV_ID,
                                2,
                                (void *)&params->xtended_tech_pref) < 0)
    {
      goto bail;
    }
  }

  /* IP family preference */
  if (params->params_mask & NETMGR_QMI_WDS_ROUTE_LOOKUP_IP_FAMILY_PREF_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                NETMGR_QMI_WDS_ROUTE_LOOKUP_IP_FAMILY_PREF_REQ_TLV_ID,
                                1,
                                (void *)&params->ip_family_pref) < 0)
    {
      goto bail;
    }
  }

  data_call_origin = (params->params_mask & NETMGR_QMI_WDS_ROUTE_LOOKUP_DATA_CALL_ORIGIN_PARAM) ?
                                  params->data_call_origin : WDS_CALL_TYPE_EMBEDDED_CALL_V01;

  if (qmi_util_write_std_tlv (tx_buf,
                              tx_buf_len,
                              NETMGR_QMI_WDS_ROUTE_LOOKUP_DATA_CALL_TYPE_TLV_ID,
                              1,
                              (void *)&data_call_origin) < 0)
  {
    goto bail;
  }

  /* Bringup by APN name */
  if (params->params_mask & NETMGR_QMI_WDS_ROUTE_LOOKUP_BRINGUP_BY_APN_NAME_PARAM)
  {
    if (qmi_util_write_std_tlv(tx_buf,
                               tx_buf_len,
                               NETMGR_QMI_WDS_ROUTE_LOOKUP_BRINGUP_BY_APN_NAME_REQ_TLV_ID,
                               1,
                               (void *) &params->bringup_by_apn_name) < 0)
    {
      goto bail;
    }
  }

  /* Bringup by APN type */
  if (params->params_mask & NETMGR_QMI_WDS_ROUTE_LOOKUP_BRINGUP_BY_APN_TYPE_PARAM)
  {
    if (qmi_util_write_std_tlv(tx_buf,
                               tx_buf_len,
                               NETMGR_QMI_WDS_ROUTE_LOOKUP_BRINGUP_BY_APN_TYPE_REQ_TLV_ID,
                               1,
                               (void *) &params->bringup_by_apn_type) < 0)
    {
      goto bail;
    }
  }

  /* APN type */
  if (params->params_mask & NETMGR_QMI_WDS_ROUTE_LOOKUP_APN_TYPE_PARAM)
  {
    if (qmi_util_write_std_tlv(tx_buf,
                               tx_buf_len,
                               NETMGR_QMI_WDS_ROUTE_LOOKUP_APN_TYPE_REQ_TLV_ID,
                               4,
                               (void *) &params->apn_type) < 0)
    {
      goto bail;
    }
  }

  /* APN type mask */
  if (params->params_mask & NETMGR_QMI_WDS_ROUTE_LOOKUP_APN_TYPE_MASK_PARAM)
  {
    if (qmi_util_write_std_tlv(tx_buf,
                               tx_buf_len,
                               NETMGR_QMI_WDS_ROUTE_LOOKUP_APN_TYPE_MASK_REQ_TLV_ID,
                               8,
                               (void *) &params->apn_type_mask) < 0)
    {
      goto bail;
    }
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}



/*===========================================================================
  FUNCTION  netmgr_qmi_wds_srvc_extract_return_code
===========================================================================*/
/*!
@brief
  Utility function to extract the error code TLV values from the given buffer

@return
  QMI_NO_ERR if operation was sucessful
  QMI_INTERNA_ERR on failure
*/
/*=========================================================================*/
int
netmgr_qmi_wds_srvc_extract_return_code
(
  unsigned char          *msg,
  int                     msg_size,
  qmi_response_type_v01  *resp
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  uint16_t result;
  uint32_t error;
  int rc = QMI_INTERNAL_ERR;

  if (!msg || !resp)
  {
    netmgr_log_err("%s(): invalid input", __func__);
    goto bail;
  }

  memset(resp, 0, sizeof(*resp));

  while (msg_size > 0)
  {
    if (qmi_util_read_std_tlv (&msg,
                               &msg_size,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      goto bail;
    }

    /* Read the result and error code values */
    if (NETMGR_QMI_RESULT_CODE_TYPE_ID == type)
    {
      READ_16_BIT_VAL (value_ptr,result);
      resp->result = (qmi_result_type_v01) result;
      READ_16_BIT_VAL (value_ptr,error);
      resp->error = (qmi_error_type_v01) error;

      rc = QMI_NO_ERR;
      break;
    }
  }

bail:
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_route_look_up
===========================================================================*/
/*!
@brief
  Queries modem for route look up

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int
netmgr_qmi_wds_route_look_up
(
  qmi_client_type                             clnt_handle,
  netmgr_qmi_wds_route_look_up_params_type    *params,
  netmgr_qmi_wds_route_look_up_rsp_type       *rsp_data
)
{
  int rc, ret = NETMGR_FAILURE;
  unsigned char     msg[QMI_MAX_STD_MSG_SIZE], *tmp_msg_ptr;
  int               msg_size;
  unsigned int actual_resp_len = 0;
  int tmp_resp_len;

  qmi_response_type_v01 resp;
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  NETMGR_LOG_FUNC_ENTRY;

  if ((!params) || (!rsp_data))
  {
    netmgr_log_err("%s(): Invalid input. params=[%p], rsp_data=[%p]\n",
                   __func__, params, rsp_data);
    goto bail;
  }

  tmp_msg_ptr = msg;
  msg_size = sizeof(msg);

  /*Fill in the Mandatory TLV */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              NETMGR_QMI_WDS_ROUTE_LOOKUP_TYPE_REQ_TLV_ID,
                              1,
                              (void *)&params->route_lookup) < 0)
  {
    goto bail;
  }

  /* Configure optional TLV's if the params are present */
  if (params->params_mask != 0)
  {
    if (NETMGR_SUCCESS != netmgr_qmi_wds_write_optional_route_look_up_tlvs(&tmp_msg_ptr,
                                                                           &msg_size,
                                                                           params))
    {
      goto bail;
    }
  }

  rc = qmi_client_send_raw_msg_sync(clnt_handle,
                                    NETMGR_QMI_WDS_ROUTE_LOOK_UP_MSG_ID,
                                    msg,
                                    sizeof(msg) - msg_size,
                                    msg,
                                    sizeof(msg),
                                    &actual_resp_len,
                                    NETMGR_QMI_TIMEOUT);

  if(rc != QMI_NO_ERR || actual_resp_len == 0)
  {
    netmgr_log_err("%s(): failed with rc=%d!", __func__, rc);
    goto bail;
  }

  if (QMI_NO_ERR != netmgr_qmi_wds_srvc_extract_return_code(msg,
                                                            actual_resp_len,
                                                            &resp))
  {
      netmgr_log_med("%s(): failed to extract error code", __func__);
      goto bail;
  }
  else if(QMI_RESULT_SUCCESS_V01 != resp.result)
  {
    netmgr_log_med("%s(): failed with qmi_err=%d", __func__, resp.error);

    /* Dont treat Interface not found  as API failure but consider it
       as no instance found */
    if(QMI_ERR_INTERFACE_NOT_FOUND_V01 == resp.error)
    {
      memset(rsp_data, 0, sizeof(netmgr_qmi_wds_route_look_up_rsp_type));
      rsp_data->qmi_inst_is_valid = FALSE;
      rsp_data->mux_id_is_valid = FALSE;
      ret = NETMGR_SUCCESS;
      rsp_data->result = resp.result;
    }
    goto bail;
  }

  /* Get route look_up information */

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Initialize indication that we go qmi_inst to FALSE */
  memset(rsp_data, 0, sizeof(netmgr_qmi_wds_route_look_up_rsp_type));
  rsp_data->qmi_inst_is_valid = FALSE;

  /* Loop through all TLV's and process each one */
  tmp_resp_len = (int)actual_resp_len;
  while (msg_size > 0)
  {
    if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                               &tmp_resp_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      break;
    }

    switch (type)
    {
      case NETMGR_QMI_WDS_ROUTE_LOOKUP_IFACE_TLV_ID:
        {
          READ_32_BIT_VAL (value_ptr,rsp_data->iface_handle);
          READ_32_BIT_VAL (value_ptr,rsp_data->priority);
          READ_16_BIT_VAL (value_ptr,rsp_data->tech_name);
          ret = NETMGR_SUCCESS;
        }
        break;

      /* Modem would send either QMI instance or MUX id but not both */
      case NETMGR_QMI_WDS_ROUTE_LOOKUP_QMI_INST_TLV_ID:
      case NETMGR_QMI_WDS_ROUTE_LOOKUP_MUX_ID_TLV_ID:
        {
          rsp_data->qmi_inst_is_valid = TRUE;
          READ_8_BIT_VAL (value_ptr,rsp_data->qmi_inst);
          if(NETMGR_QMI_WDS_ROUTE_LOOKUP_MUX_ID_TLV_ID == type)
          {
            --rsp_data->qmi_inst;
            netmgr_log_med("Mapped MUX ID to QMI inst [%d]",(int)rsp_data->qmi_inst);
          }
        }
        break;

      default:
        {
          netmgr_log_med("%s(): unknown response TLV type = 0x%x",
                          __func__, (unsigned int)type);
        }
        break;
    }
  }

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}


/*===========================================================================
  FUNCTION  netmgr_qmi_wds_get_configured_throughput_info
===========================================================================*/
/*!
@brief
  Query the Modem for configured throughput info for default SUB

@return
  int - NETMGR_QMI_WDS_SUCCESS on successful operation,
        NETMGR_QMI_WDS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_get_configured_throughput_info
(
  qmi_client_type                                 clnt_hndl,
  wds_get_configured_throughput_info_resp_msg_v01  *tput_info
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret = NETMGR_FAILURE;
  wds_get_configured_throughput_info_req_msg_v01 req;
  wds_get_configured_throughput_info_resp_msg_v01 resp;

  NETMGR_LOG_FUNC_ENTRY;

  if ((NETMGR_QMI_CLIENT_INVALID == clnt_hndl)|| (NULL == tput_info)) {
    netmgr_log_err("%s(): invalid clnt_hndl or tput_info ptr",__func__);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  /* Send throughput_Info_request */
  rc = qmi_client_send_msg_sync(clnt_hndl,
                                QMI_WDS_GET_CONFIGURED_THROUGHPUT_INFO_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void *)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR) {
    netmgr_log_err("%s(): failed for link 0 with rc=%d result=%d err=%d!\n",
                   __func__,  rc, resp.resp.result, resp.resp.error);
    goto bail;
  }

  *tput_info = resp;

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}


#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_qmi_wds_rev_ip_ind_reg
===========================================================================*/
/*!
@brief
  Register for reverse IP transport indications

@return
  int - NETMGR_QMI_WDS_SUCCESS on successful operation,
        NETMGR_QMI_WDS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_rev_ip_ind_reg
(
  int                      link,
  qmi_client_type          clnt_hndl
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret = NETMGR_FAILURE;
  wds_reverse_ip_transport_connection_ind_registration_req_msg_v01 req;
  wds_reverse_ip_transport_connection_ind_registration_resp_msg_v01 resp;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_hndl) {
    netmgr_log_err("%s(): invalid clnt_hndl on link=%d!\n",
                   __func__, link);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  req.register_for_ind = TRUE;

  /* Send ext ip config registration */
  rc = qmi_client_send_msg_sync(clnt_hndl,
                                QMI_WDS_REVERSE_IP_TRANSPORT_CONNECTION_IND_REGISTRATION_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void *)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR || QMI_RESULT_SUCCESS_V01 != resp.resp.result) {
    netmgr_log_err("%s(): failed for link=%d with rc=%d result=%d err=%d!\n",
                   __func__, link, rc, resp.resp.result, resp.resp.error);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_rev_ip_config_complete
===========================================================================*/
/*!
@brief
  Report reverse adapter configuration result to the Modem

@return
  int - NETMGR_QMI_WDS_SUCCESS on successful operation,
        NETMGR_QMI_WDS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_rev_ip_config_complete
(
  int                                       link,
  qmi_client_type                           clnt_hndl,
  uint32_t                                  txn_id,
  netmgr_qmi_wds_rev_ip_config_result_type  result
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret = NETMGR_FAILURE;
  wds_reverse_ip_transport_config_complete_req_msg_v01 req;
  wds_reverse_ip_transport_config_complete_resp_msg_v01 resp;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_hndl) {
    netmgr_log_err("%s(): invalid clnt_hndl on link=%d!\n",
                   __func__, link);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  req.config_result = result;
  req.transaction_id = txn_id;

  /* Send ext ip config registration */
  rc = qmi_client_send_msg_sync(clnt_hndl,
                                QMI_WDS_REVERSE_IP_TRANSPORT_CONFIG_COMPLETE_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void *)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR || QMI_RESULT_SUCCESS_V01 != resp.resp.result) {
    netmgr_log_err("%s(): failed for link=%d with rc=%d result=%d err=%d!\n",
                   __func__, link, rc, resp.resp.result, resp.resp.error);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_get_ipsec_sa_config
===========================================================================*/
/*!
@brief
  Query the IPSec config from the Modem

@return
  int - NETMGR_QMI_WDS_SUCCESS on successful operation,
        NETMGR_QMI_WDS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_get_ipsec_sa_config
(
  int                                          link,
  qmi_client_type                              clnt_hndl,
  wds_get_ipsec_static_sa_config_resp_msg_v01  *result
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret = NETMGR_FAILURE;
  wds_get_ipsec_static_sa_config_req_msg_v01 req;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_hndl) {
    netmgr_log_err("%s(): invalid clnt_hndl on link=%d!\n",
                   __func__, link);
    goto bail;
  }

  if (!result) {
    netmgr_log_err("%s(): invalid input!\n",
                   __func__);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(result, 0, sizeof(*result));

  rc = qmi_client_send_msg_sync(clnt_hndl,
                                QMI_WDS_GET_IPSEC_STATIC_SA_CONFIG_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void *)result,
                                sizeof(*result),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR || QMI_RESULT_SUCCESS_V01 != result->resp.result) {
    netmgr_log_err("%s(): failed for link=%d with rc=%d result=%d err=%d!\n",
                   __func__, link, rc, result->resp.result, result->resp.error);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_initiate_esp_rekey
===========================================================================*/
/*!
@brief
  Initiate ESP re-key for the iWLAN call corresponding to the client handle

@return
  int - NETMGR_QMI_WDS_SUCCESS on successful operation,
        NETMGR_QMI_WDS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_initiate_esp_rekey
(
  int              link,
  qmi_client_type  clnt_hndl
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret = NETMGR_FAILURE;
  wds_initiate_esp_rekey_req_msg_v01  req;
  wds_initiate_esp_rekey_resp_msg_v01  resp;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_hndl) {
    netmgr_log_err("%s(): invalid clnt_hndl on link=%d!\n",
                   __func__, link);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  rc = qmi_client_send_msg_sync(clnt_hndl,
                                QMI_WDS_INITIATE_ESP_REKEY_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void *)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR || QMI_RESULT_SUCCESS_V01 != resp.resp.result) {
    netmgr_log_err("%s(): failed for link=%d with rc=%d result=%d err=%d!\n",
                   __func__, link, rc, resp.resp.result, resp.resp.error);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_set_data_coalescing
===========================================================================*/
/*!
@brief
  Set the data coalescing settings

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_set_data_coalescing
(
  int                link,
  qmi_client_type    clnt_hndl,
  boolean            rsc,
  boolean            rsb
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret = NETMGR_FAILURE;
  wds_set_data_coalescing_req_msg_v01 req;
  wds_set_data_coalescing_resp_msg_v01 resp;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_hndl) {
    netmgr_log_err("%s(): invalid clnt_hndl on link=%d!\n",
                   __func__, link);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  req.coalescing_info.tcp_coalescing = rsc;
  req.coalescing_info.udp_coalescing = rsb;

  rc = qmi_client_send_msg_sync(clnt_hndl,
                                QMI_WDS_SET_DATA_COALESCING_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void *)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR || QMI_RESULT_SUCCESS_V01 != resp.resp.result)
  {
    netmgr_log_err("%s(): failed for link=%d with rc=%d result=%d err=%d!\n",
                   __func__, link, rc, resp.resp.result, resp.resp.error);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_get_data_coalescing
===========================================================================*/
/*!
@brief
  Get the data coalescing settings

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_wds_get_data_coalescing
(
  int                    link,
  qmi_client_type        clnt_hndl,
  boolean                *rsc,
  boolean                *rsb
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret = NETMGR_FAILURE;
  wds_get_data_coalescing_req_msg_v01 req;
  wds_get_data_coalescing_resp_msg_v01 resp;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_QMI_CLIENT_INVALID == clnt_hndl || rsc == NULL || rsb == NULL)
  {
    netmgr_log_err("%s(): invalid input parameters on link=%d!\n", __func__, link);
    goto bail;
  }

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  rc = qmi_client_send_msg_sync(clnt_hndl,
                                QMI_WDS_GET_DATA_COALESCING_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void *)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (rc != QMI_NO_ERR || QMI_RESULT_SUCCESS_V01 != resp.resp.result)
  {
    netmgr_log_err("%s(): failed for link=%d with rc=%d result=%d err=%d!\n",
                   __func__, link, rc, resp.resp.result, resp.resp.error);
    goto bail;
  }

  *rsc = resp.coalescing_info.tcp_coalescing;
  *rsb = resp.coalescing_info.udp_coalescing;

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}
