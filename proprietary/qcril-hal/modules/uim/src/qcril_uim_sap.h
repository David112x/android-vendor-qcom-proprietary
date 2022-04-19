#ifndef QCRIL_UIM_SAP_H
#define QCRIL_UIM_SAP_H
/*===========================================================================

  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "modules/uim/qcril_uim_srvc.h"
#include "modules/uim/UimCardStatusIndMsg.h"
#include "modules/uim/UimSAPRequestMsg.h"
#include "modules/uim/UimSAPConnectionRequestMsg.h"
#include "qcril_uim.h"

/*===========================================================================

TYPES and Definitions

===========================================================================*/


/*===========================================================================

                            FUNCTIONS

===========================================================================*/
/*===========================================================================

FUNCTION:  qcril_uim_sap_process_card_status

===========================================================================*/
/*!
    @brief
    This function handles the card state change indication.

    @return
    None.

*/
/*=========================================================================*/
void qcril_uim_sap_process_card_status
(
  qmi_uim_card_state_type   card_state,
  qmi_uim_card_error_type   err_type,
  uint8                     slot
);

/*===========================================================================

FUNCTION:  qcril_uim_sap_send_request

===========================================================================*/
/*!
   @brief
   This function sends SAP request to qcril_uim_qcci

   @return
   int.

*/
/*=========================================================================*/
void qcril_uim_sap_send_request
(
  const std::shared_ptr<UimSAPRequestMsg>  req_ptr
);

/*===========================================================================

FUNCTION:  qcril_uim_sap_connection_request

===========================================================================*/
/*!
   @brief
   This function sends SAP connection request

   @return
   int.

*/
/*=========================================================================*/
void qcril_uim_sap_connection_request
(
  const std::shared_ptr<UimSAPConnectionRequestMsg> req_ptr
);


/*===========================================================================

FUNCTION:  qcril_uim_sap_ind

===========================================================================*/
/*!
    @brief
    This is the SAP indication callback implementation for the QMI interface.

    @return
    None.

*/
/*=========================================================================*/
void qcril_uim_sap_ind
(
  qmi_uim_sap_indication_params_type   * ind_data_ptr
);

/*=========================================================================

FUNCTION:  qcril_uim_sap_qmi_handle_sap_connection_resp

===========================================================================*/
/*!
    @brief
    Handles the SAP connection request callback. Based on the response
    received from the modem, respective packed response types are constructed
    and the onRequestComplete is called. This completes the original request
    called on the RIL SAP socket.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_sap_qmi_handle_sap_connection_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimSAPConnectionRequestMsg> req_ptr
);

/*=========================================================================

FUNCTION:  qcril_uim_sap_qmi_handle_sap_request_resp

===========================================================================*/
/*!
    @brief
    Handles the SAP request callback. Based on the response
    received from the modem, respective packed response types are constructed
    and the onRequestComplete is called. This completes the original request
    called on the RIL SAP socket.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_sap_qmi_handle_sap_request_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimSAPRequestMsg>  req_ptr
);

#endif /* QCRIL_UIM_SAP_H */
