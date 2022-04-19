/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
  @file    ril_utf_stubs.h
  @brief   RIL UTF QMI stubs for non QMI RIL functions

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_defs.h"

qmi_qmux_if_hndl_t
qmi_service_get_qmux_if_handle(void)
{
  qmi_qmux_if_hndl_t temp;
  return temp;
}

int qmi_qmux_if_send_if_msg_to_qmux
(
  qmi_qmux_if_hndl_t        qmux_if_client_handle,
  qmi_qmux_if_msg_id_type   msg_id,
  qmi_connection_id_type    conn_id,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  int                       *qmi_err_code,
  unsigned int              timeout
)
{
  return 0;
}

