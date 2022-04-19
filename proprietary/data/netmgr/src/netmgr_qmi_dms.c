
/*****************************************************************************

      N E T M G R _ Q M I _D M S . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_qmi_dms.c
  @brief   Network Manager QMI Device Management service helper

  DESCRIPTION
  Network Manager QMI device management service helper

******************************************************************************/
/*===========================================================================

  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef NETMGR_OFFTARGET

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <sys/utsname.h>
#include "qmi_client.h"
#include "netmgr_util.h"
#include "netmgr_qmi.h"
#include "netmgr_qmi_dms.h"
#include "device_management_service_v01.h"

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/
static qmi_cci_os_signal_type  netmgr_qmi_dms_os_params;


/*===========================================================================

  FUNCTION:  netmgr_qmi_dms_init

===========================================================================*/
/*!
    @brief
    Initialize the QMI DMS services for Netmgr

    @return
    qmi_client_type client handle if success
    NETMGR_QMI_CLIENT_INVALID if failure
*/
/*=========================================================================*/
static qmi_client_type netmgr_qmi_dms_init
(
  void
)
{
  qmi_idl_service_object_type  dms_svc_obj;
  qmi_client_type              client = NETMGR_QMI_CLIENT_INVALID;
  qmi_client_error_type        rc;

  dms_svc_obj = dms_get_service_object_v01();
  if (dms_svc_obj == NULL)
  {
    netmgr_log_err("%s(): DMS service not available\n", __func__);
    return client;
  }

  memset(&netmgr_qmi_dms_os_params, 0, sizeof(qmi_cci_os_signal_type));

  rc = qmi_client_init_instance(dms_svc_obj,
                                QMI_CLIENT_INSTANCE_ANY,
                                NULL,
                                NULL,
                                &netmgr_qmi_dms_os_params,
                                NETMGR_QMI_TIMEOUT,
                                &client);

  if (rc != QMI_NO_ERR)
  {
    netmgr_log_err("%s(): client init failed, err %d\n", __func__, rc);
    return client;
  }

  netmgr_log_low("Allocated DMS client\n");
  return client;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_dms_release_client
===========================================================================*/
/*!
@brief
  Release QMI DMS handle

@return
  None

*/
/*=========================================================================*/
static void netmgr_qmi_dms_release_client
(
  qmi_client_type client
)
{
  qmi_client_error_type rc;

  if (NETMGR_QMI_CLIENT_INVALID == client)
  {
    return;
  }

  rc = qmi_client_release(client);
  if (QMI_NO_ERR != rc)
  {
    netmgr_log_err("%s(): failed to release client rc=%d\n", __func__, rc);
    return;
  }

  netmgr_log_low("Released DMS client\n");
}

/*===========================================================================
  FUNCTION  netmgr_qmi_dms_set_ap_ver
===========================================================================*/
/*!
@brief
  Send AP version information.

@return
  None

*/
/*=========================================================================*/
static void netmgr_qmi_dms_set_ap_ver
(
  qmi_client_type client
)
{
  dms_set_ap_version_req_msg_v01 req;
  dms_set_ap_version_resp_msg_v01 resp;
  qmi_client_error_type rc;
  struct utsname n;

  memset(&n, 0, sizeof(n));
  if (uname(&n))
  {
    netmgr_log_err("%s(): failed to get uname\n", __func__);
    return;
  }

  netmgr_log_high("%s %s %s %s %s\n", n.sysname, n.nodename,
                  n.release, n.version, n.machine);

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  req.sw_image_type_valid = TRUE;
  if (strstr(n.release, "perf+") != NULL)
  {
    req.sw_image_type = DMS_DEVICE_IMAGE_PERF_V01;
  }
  else
  {
    req.sw_image_type = DMS_DEVICE_IMAGE_NON_PERF_V01;
  }

  rc = qmi_client_send_msg_sync(client,
                                QMI_DMS_SET_AP_VERSION_REQ_V01,
                                &req,
                                sizeof(req),
                                &resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (QMI_NO_ERR != rc || QMI_RESULT_SUCCESS_V01 != resp.resp.result)
  {
    netmgr_log_err("%s(): failed to send msg, rc=%d, err=%d\n",
                   __func__, rc, resp.resp.error);
    return;
  }

  netmgr_log_low("%s(): success\n", __func__);
}

#endif /* NETMGR_OFFTARGET */

/*===========================================================================
  FUNCTION  netmgr_qmi_dms_send_info
===========================================================================*/
/*!
@brief
  Send device management information.

@return
  None

*/
/*=========================================================================*/
void netmgr_qmi_dms_send_info
(
  void
)
{
#ifndef NETMGR_OFFTARGET
  qmi_client_type client;

  client = netmgr_qmi_dms_init();
  if (NETMGR_QMI_CLIENT_INVALID == client)
  {
    return;
  }

  netmgr_qmi_dms_set_ap_ver(client);

  netmgr_qmi_dms_release_client(client);
#endif /* NETMGR_OFFTARGET */
}
