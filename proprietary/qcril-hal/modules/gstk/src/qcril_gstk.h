#ifndef QCRIL_GSTK_QMI_H
#define QCRIL_GSTK_QMI_H
/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#  ---------------------------------------------------------------------------
#******************************************************************************/
/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "modules/gstk/qcril_gstk_srvc.h"
#include "modules/gstk/GstkSendTerminalResponseRequestMsg.h"
#include "modules/gstk/GstkSendEnvelopeRequestMsg.h"
#include "modules/gstk/GstkSendSetupCallResponseRequestMsg.h"
#include "modules/gstk/GstkReportSTKIsRunningMsg.h"

/*===========================================================================
                        INTERNAL DEFINITIONS AND TYPES
===========================================================================*/

/* Envelope command Tags */
#define QCRIL_GSTK_ENV_CMD_MENU_SELECTION_TAG                     0xD3
#define QCRIL_GSTK_ENV_CMD_CALL_CONTROL_TAG                       0xD4
#define QCRIL_GSTK_ENV_CMD_SMS_PP_DOWNLOAD_TAG                    0xD1
#define QCRIL_GSTK_ENV_CMD_GEO_LOC_RPT_TAG                        0xDD
#define QCRIL_GSTK_ENV_CMD_EVT_DOWNLOAD_TAG                       0xD6

/* Envelope and proactive command tag lengths */
#define QCRIL_GSTK_PROACTIVE_CMD_TAG_LENGTH                       0x01
#define QCRIL_GSTK_ENV_CMD_TAG_LENGTH                             0x01

/* Generic Lengths associated with TLVs as part of GSTK */
#define QCRIL_GSTK_ENVELOPE_CMD_EVENT_LIST_LEN                    0x03
#define QCRIL_GSTK_TLV_TAG_LENGTH                                 0x01


/*===========================================================================
  FUNCTION:  qcril_gstk_reset_state
===========================================================================*/
/*!
  @brief
    Reset GSTK.

  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_reset_state
(
  boolean is_refresh
);

/*===========================================================================
  FUNCTION:  qcril_gstk_init_state
===========================================================================*/
/*!
  @brief
    Init GSTK.

  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
int qcril_gstk_init_state
(
  void
);

/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_service_is_running
===========================================================================*/
/*!
  @brief
    Handles RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING

  @param[in]  req_ptr  Android RIL request message

  @return
    RIL_Errno

  @return
    Nothing
*/
/*=========================================================================*/
void qcril_gstk_qmi_request_stk_service_is_running
(
  std::shared_ptr<GstkReportSTKIsRunningMsg> req_ptr,
  bool                                       m_ready
);

/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim
===========================================================================*/
/*!
  @brief
    Handles RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM.

    This request arrives from the SIM Toolkit application after the user
    has accepted or rejected a call originating from a SETUP_CALL command,
    and the request data indicates which.

  @param[in]  req_ptr  Android RIL request message

  @return
    None
*/
/*=========================================================================*/
void qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim
(
  std::shared_ptr<GstkSendSetupCallResponseRequestMsg> req_ptr
);

/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_send_terminal_response
===========================================================================*/
/*!
  @brief
    Handles RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE

    For proactive commands for which the Terminal Response originates from
    the Android Toolkit application, this function arranges for that
    response to be sent to the card.

  @param[in]  req_ptr  Android RIL request message

  @return
    None
*/
/*=========================================================================*/
void qcril_gstk_qmi_request_stk_send_terminal_response
(
  std::shared_ptr<GstkSendTerminalResponseRequestMsg> req_ptr
);

/*===========================================================================

FUNCTION:  qcril_gstk_qmi_request_stk_send_envelope_command
===========================================================================*/
/*!
  @brief
    Handles RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND.

  @param[in]  req_ptr  Android RIL request message

  @return
    RIL_Errno

  @msc
  @endmsc

  @see
*/
/*=========================================================================*/
void qcril_gstk_qmi_request_stk_send_envelope_command
(
  std::shared_ptr<GstkSendEnvelopeRequestMsg> req_ptr
);

/*===========================================================================

FUNCTION:  qcril_gstk_process_qmi_cat_ind
===========================================================================*/
/*!
  @brief
    Handles QMI CAT indications.

  @param[in]  ind_id  Qmi Cat indication id
  @param[in]  ind_ptr Qmi cat indication message

  @return
    None

  @msc
  @endmsc

  @see
*/
/*=========================================================================*/
void qcril_gstk_process_qmi_cat_ind
(
  qmi_cat_indication_id_type       ind_id,
  qcril_gstk_qmi_ind_params_type  *ind_ptr
);

/*===========================================================================

FUNCTION:  qcril_gstk_process_qmi_cat_cb
===========================================================================*/
/*!
  @brief
    Handles QMI CAT async callback.

  @param[in]  cb_ptr Qmi cat callback message

  @return
    None

  @msc
  @endmsc

  @see
*/
/*=========================================================================*/
void qcril_gstk_process_qmi_cat_cb
(
  qmi_cat_rsp_data_type    *cb_ptr,
  std::shared_ptr<Message>  req_ptr
);

/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_hexstring_to_bin
===========================================================================*/
/*!
  @brief
    Convert a hex string to an array of binary bytes.

  @param[out] dest_ptr  Pointer to destination area. Must not be NULL.
                        Size of dest area must be atleast twice the size of
                        source area.
  @param[in]  src_ptr   Pointer to source area. Must not be NULL.
  @param[in]  length    Length (in bytes) of the data pointed to by src_ptr.
                        The length of the data output via dest_ptr will be
                        half this.

  @return
    Nothing
 */
/*=========================================================================*/
void qcril_gstk_qmi_hexstring_to_bin
(
  uint8_t       *dest_ptr,
  const uint8_t *src_ptr,
  uint32_t       length
);

/*===========================================================================
  FUNCTION:  qcril_gstk_convert_req_type_to_rsp_id
===========================================================================*/
/*!
  @brief
    Map request type to response type.

  @param[in]  req_id   Request id.

  @return
    qmi_cat_rsp_id_type
 */
/*=========================================================================*/
qmi_cat_rsp_id_type qcril_gstk_convert_req_type_to_rsp_id
(
  qcril_gstk_request_type req_id
);

#endif /* QCRIL_GSTK_QMI_H */

