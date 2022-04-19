/******************************************************************************
#  Copyright (c) 2015 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_ssr_if.h
  @brief   SSR API's for test case dev

  DESCRIPTION

  API's for modifying SSR messages used in RIL UTF tests.

  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef RIL_UTF_SSR_IF_H_
#define RIL_UTF_SSR_IF_H_

#include "ril_utf_if.h"

typedef class ril_utf_ssr_if : public ril_utf_node_base_t {
public:
//==================================================
// SSR Interfaces
//==================================================

/*
  CLIENT_ERROR_CALLBACK
*/
void update_default_ssr_client_error_callback(qcril_qmi_client_e_type error_type);
/*
  CLIENT_NOTIFY_CALLBACK
*/
void update_default_ssr_client_notify_callback(qcril_qmi_client_e_type client_type);

} SSRTestnode;

#endif
