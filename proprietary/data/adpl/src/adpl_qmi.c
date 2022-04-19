/******************************************************************************

                        ADPL_QMI.C

******************************************************************************/

/******************************************************************************

  @file    adpl_qmi.c
  @brief   Accelerated Data Path Logging module for ADPL logging. This file
           has functions which interact with QMI APIs.

  DESCRIPTION
  Has functions which interact with QMI APIs for ADPL logging.

  ---------------------------------------------------------------------------
  Copyright (c) 2014-2015,2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/25/18   vv         Initial version

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <fcntl.h>

#include "adpl.h"
#include "wireless_data_administrative_service_v01.h"
#include "data_filter_service_v01.h"
#include "wireless_data_service_v01.h"

/*===========================================================================

FUNCTION ADPL_QMI_WDA_SET_DATA_FORMAT()

DESCRIPTION
  Initializes a WDA QMI client and set data format

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int adpl_qmi_wda_set_data_format
(
   int aggr_size,
   int mhi_ep_type,
   int mhi_id
)
{

  qmi_idl_service_object_type                              adpl_wda_service_object;
  qmi_client_error_type                                    qmi_error, qmi_err_code = QMI_NO_ERR;
  qmi_client_os_params                                     adpl_wda_os_params;
  int                                                      ret, rc = ADPL_SUCCESS;
  qmi_client_type                                          adpl_wda_handle;
  wda_set_data_format_req_msg_v01                          request;
  wda_set_data_format_resp_msg_v01                         response;
  qmi_service_info info;
/*---------------------------------------------------------------------------*/

  LOG_MSG_INFO1("adpl_qmi_wda_set_data_format()", 0, 0, 0);

/*-----------------------------------------------------------------------------
  Deregister QMUXD ports since we want the client init request to go over IPC router
------------------------------------------------------------------------------*/
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);

/*-----------------------------------------------------------------------------
  Obtain a WDS client for ADPL
  - get the service object
  - obtain the client
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Get the service object
------------------------------------------------------------------------------*/
  adpl_wda_service_object = wda_get_service_object_v01();
  if (adpl_wda_service_object == NULL)
  {
    LOG_MSG_ERROR("ADPL DPM service object not available",
                   0, 0, 0);
    return ADPL_FAILURE;
  }

  memset(&adpl_wda_os_params, 0, sizeof(qmi_client_os_params));
/*-----------------------------------------------------------------------------
  Client init
------------------------------------------------------------------------------*/

  qmi_error = qmi_client_init_instance(adpl_wda_service_object,
                                       QMI_CLIENT_INSTANCE_ANY,
                                       NULL,
                                       NULL,
                                       &adpl_wda_os_params,
                                       ADPL_QMI_MSG_TIMEOUT_VALUE,
                                       &adpl_wda_handle);

  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not init DPM client %d", qmi_error, 0, 0);
    rc = ADPL_FAILURE;
    goto bail;
  }

  LOG_MSG_INFO1("Got wda client handle", 0, 0, 0);

  memset(&request, 0, sizeof(wda_set_data_format_req_msg_v01));
  memset(&response, 0, sizeof(wda_set_data_format_resp_msg_v01));

  LOG_MSG_INFO1("Set data format: EP type: %d, EP ID: %d, aggr size: %d",
                mhi_ep_type, mhi_id, aggr_size);
  request.ep_id_valid = TRUE;
  request.ep_id.iface_id = mhi_id;
  request.ep_id.ep_type = mhi_ep_type;
  request.dl_data_aggregation_max_size_valid = TRUE;
  request.dl_data_aggregation_max_size = aggr_size;

  ret = qmi_client_send_msg_sync(adpl_wda_handle,
                                 QMI_WDA_SET_DATA_FORMAT_REQ_V01,
                                 &request,
                                 sizeof(wda_set_data_format_req_msg_v01),
                                 &response,
                                 sizeof(wda_set_data_format_resp_msg_v01),
                                 ADPL_QMI_MAX_TIMEOUT);

  if (QMI_NO_ERR != ret)
  {
    LOG_MSG_ERROR("Error sending QMI WDA_SET_DATA_FORMAT message: %d", ret,0,0);
    rc = ADPL_FAILURE;
  }

  LOG_MSG_INFO1("Sent wda set data format", 0, 0, 0);

bail:
  ret = qmi_client_release(adpl_wda_handle);
  if (ret != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not release WDA client %d", qmi_error, 0, 0);
    rc = ADPL_FAILURE;
  }

  return rc;
}
