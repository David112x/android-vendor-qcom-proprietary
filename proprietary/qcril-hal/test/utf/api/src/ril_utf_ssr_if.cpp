/******************************************************************************
#  Copyright (c) 2015 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_ssr_if.cpp
  @brief   MSIM API's for test case dev

  DESCRIPTION

  API's for modifying RILD simulation for IPC communication
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_ssr_if.h"
//---------------------------------------------------------------------------------------
//==================================================
// SSR Interfaces
//==================================================

/*
  CLIENT_ERROR_CALLBACK
*/
void ril_utf_ssr_if::update_default_ssr_client_error_callback(qcril_qmi_client_e_type error_type)
{
  payload = malloc(sizeof(qcril_qmi_client_e_type));
  payload_len = sizeof(qcril_qmi_client_e_type);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ssr_error_cb;

  qcril_qmi_client_e_type *ptr = (qcril_qmi_client_e_type*) payload;
  *ptr = error_type;
}

/*
  CLIENT_NOTIFY_CALLBACK
*/
void ril_utf_ssr_if::update_default_ssr_client_notify_callback(qcril_qmi_client_e_type client_type)
{
  payload = malloc(sizeof(qcril_qmi_client_e_type));
  payload_len = sizeof(qcril_qmi_client_e_type);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ssr_notify_cb;

  qcril_qmi_client_e_type *ptr = (qcril_qmi_client_e_type*) payload;
  *ptr = client_type;
}

