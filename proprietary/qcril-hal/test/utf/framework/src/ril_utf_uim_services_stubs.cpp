/******************************************************************************
  @file    ril_utf_uim_services_stubs.cpp
  @brief   RIL UTF IMS HIDL implementation

  DESCRIPTION

  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

extern "C" {
#include "ril_utf_defs.h"
#include "qcril_uim_remote.h"
#include "qcril_uim_http.h"
#include "qcril_uim_lpa.h"
#include "qcril_uim_security.h"
}

// common/uim/uim_lpa_service.cc
extern "C" {

uint8_t qcril_uim_lpa_register_service
(
  int instanceId
)
{
  (void)instanceId;
  return FALSE;
} /* qcril_uim_lpa_register_service */


void qcril_uim_lpa_http_txn_completed_response
(
  void  * token_ptr,
  qcril_uim_http_result_type result
)
{
  (void)token_ptr;
  (void)result;
} /* qcril_uim_lpa_http_txn_completed_response */


/*===========================================================================

FUNCTION:  qcril_uim_lpa_user_response

===========================================================================*/
void qcril_uim_lpa_user_response
(
  void                         * token_ptr,
  qcril_uim_lpa_user_resp_type * user_resp_ptr
)
{
  (void)token_ptr;
  (void)user_resp_ptr;
} /* qcril_uim_lpa_user_response */


/*===========================================================================

FUNCTION:  qcril_uim_lpa_add_profile_progress_ind

===========================================================================*/
void qcril_uim_lpa_add_profile_progress_ind
(
  qcril_uim_lpa_add_profile_progress_ind_type * prog_ind_ptr
)
{
  (void)prog_ind_ptr;
} /* qcril_uim_lpa_add_profile_progress_ind */


/*===========================================================================

FUNCTION:  qcril_uim_lpa_http_txn_indication

===========================================================================*/
void qcril_uim_lpa_http_txn_indication
(
  qcril_uim_http_transaction_ind_type * txn_ind_ptr
)
{
  (void)txn_ind_ptr;
} /* qcril_uim_lpa_http_txn_indication */


}


// common/uim/uim_remote_client_service.cc

extern "C" uint8_t qcril_uim_remote_client_register_service
(
int instanceId
)
{
  (void)instanceId;
  return FALSE;
} /* qcril_uim_remote_client_register_service */


/*===========================================================================

FUNCTION:  qcril_uim_remote_client_event_rsp

===========================================================================*/
extern "C" void qcril_uim_remote_client_event_rsp
(
int32_t                           token,
qcril_uim_remote_event_resp_type  event_rsp
)
{
  (void)token;
  (void)event_rsp;
} /* qcril_uim_remote_client_event_rsp */


/*===========================================================================

FUNCTION:  qcril_uim_remote_client_apdu_rsp

===========================================================================*/
extern "C" void qcril_uim_remote_client_apdu_rsp
(
  int32_t                          token,
  qcril_uim_remote_apdu_resp_type  apdu_rsp
)
{
  (void)token;
  (void)apdu_rsp;
} /* qcril_uim_remote_client_apdu_rsp */


/*===========================================================================

FUNCTION:  qcril_uim_remote_client_send_apdu_ind

===========================================================================*/
extern "C" void qcril_uim_remote_client_send_apdu_ind
(
  uint32_t   apdu_len,
  uint8_t  * apdu_ptr
)
{
  (void)apdu_len;
  (void)apdu_ptr;
} /* qcril_uim_remote_client_send_apdu_ind */


/*===========================================================================

FUNCTION:  qcril_uim_remote_client_connect_ind

===========================================================================*/
extern "C" void qcril_uim_remote_client_connect_ind()
{
  // dummy
} /* qcril_uim_remote_client_connect_ind */


/*===========================================================================

FUNCTION:  qcril_uim_remote_client_disconnect_ind

===========================================================================*/
extern "C" void qcril_uim_remote_client_disconnect_ind()
{
  // dummy
} /* qcril_uim_remote_client_disconnect_ind */


/*===========================================================================

FUNCTION:  qcril_uim_remote_client_power_up_ind

===========================================================================*/
extern "C" void qcril_uim_remote_client_power_up_ind
(
  bool                                has_time_out,
  int32_t                             time_out,
  bool                                has_voltage_class,
  qcril_uim_remote_voltage_class_type voltage_class
)
{
  (void)has_time_out;
  (void)time_out;
  (void)has_voltage_class;
  (void)voltage_class;
} /* qcril_uim_remote_client_power_up_ind */


/*===========================================================================

FUNCTION:  qcril_uim_remote_client_power_down_ind

===========================================================================*/
extern "C" void qcril_uim_remote_client_power_down_ind
(
  bool                             has_mode,
  qcril_uim_remote_power_down_mode mode
)
{
  (void)has_mode;
  (void)mode;
} /* qcril_uim_remote_client_power_down_ind */


/*===========================================================================

FUNCTION:  qcril_uim_remote_client_reset_ind

===========================================================================*/
extern "C" void qcril_uim_remote_client_reset_ind()
{
  // dummy
} /* qcril_uim_remote_client_reset_ind */


// common/uim/uim_service.cc

extern "C" void qcril_uim_remote_simlock_response
(
  void                                    * token_ptr,
  qcril_uim_remote_simlock_response_type    response,
  qcril_uim_remote_simlock_operation_type   op_type,
  uint8                                   * simlock_rsp_ptr,
  uint32                                    simlock_rsp_len,
  qcril_uim_remote_simlock_version          version,
  qcril_uim_remote_simlock_status           status
)
{
  // dummy
} /* qcril_uim_remote_simlock_response */

extern "C" uint8_t qcril_uim_register_service
(
  int instanceId
)
{
  return TRUE;
} /* qcril_uim_register_service */

// common/uim/uim_remote_server_service.cc
extern "C" uint8_t qcril_uim_remote_server_register_service
(
  int instanceId
)
{
  return TRUE;
} /* qcril_uim_remote_server_register_service */

// qcril_qmi/qti_radio/qcril_qmi_qti_radio_service.c
extern "C" void qtiGetAtrResponse(RIL_Token token, RIL_Errno error, char *buf, int bufLen) {
  // dummy
}

namespace android {
  int gDarwinCantLoadAllObjects = 1;
}
