#ifndef QCRIL_UIM_CARD_H
#define QCRIL_UIM_CARD_H
/*===========================================================================

  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "qcril_uim.h"
#include "modules/uim/UimGetCardStatusRequestMsg.h"
#include "modules/uim/UimVoltageStatusRequestMsg.h"
#include "modules/uim/UimCardPowerReqMsg.h"
#include "modules/uim/UimCardStatusIndMsg.h"
#include "modules/uim/UimGetAtrRequestMsg.h"
#include "modules/uim/UimChangeSubscriptionReqMsg.h"
#include "modules/uim_remote/UimRmtCardAppInitStatusIndMsg.h"

/*===========================================================================
  ENUM:  UimTimerCancelReason
  NORMAL - Cancel timer due to any situation
  SSR    - Cancel timer due to ongoing SSR
===========================================================================*/
typedef enum _UimTimerCancelReason {
    TIMER_CANCEL_REASON_NORMAL = 0,
    TIMER_CANCEL_REASON_MODEM_SSR = 1
} UimTimerCancelReason;

/*===========================================================================

  FUNCTION:  qcril_uim_init_card_status

===========================================================================*/
/*!
    @brief
    Initializes the global card status structure.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_init_card_status
(
  qmi_uim_card_status_type  * card_status_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_request_get_sim_status

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_SIM_STATUS request from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_sim_status
(
  const std::shared_ptr<UimGetCardStatusRequestMsg> msg_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_process_status_change_ind

===========================================================================*/
/*!
    @brief
    Main function for processing QMI card status changed indication. Based
    on the indication received, if needed, it updates the global card status,
    ret_ptr and sends card events internal to QCRIL (CM & PBM).

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_status_change_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr
);


/*===========================================================================

  FUNCTION:  qcril_uim_update_cm_card_status

===========================================================================*/
/*!
    @brief
    Update QCRIL(CM) card status per UIM card state.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_update_cm_card_status
(
  int                          slot,
  qcril_uim_card_status_e_type new_card_status
);


/*=========================================================================

  FUNCTION:  qcril_uim_process_change_subscription

===========================================================================*/
/*!
    @brief
    Processes the subscription activation/deactivation requests from QCRIL
    NAS.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_process_change_subscription
(
  std::shared_ptr<UimChangeSubscriptionReqMsg> msg_ptr
);

/*=========================================================================

  FUNCTION:  qcril_uim_update_prov_session_type

===========================================================================*/
/*!
    @brief
    Updates the global provisioning session status type based on the passed
    session type.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_update_prov_session_type
(
  qmi_uim_session_type                  session_type,
  qcril_uim_prov_session_state_type     session_state
);

/*=========================================================================

  FUNCTION:  qcril_uim_get_atr_resp

===========================================================================*/
/*!
    @brief
    Processes the response for get ATR command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_get_atr_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimGetAtrRequestMsg> req_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_process_recovery_ind

===========================================================================*/
/*!
    @brief
    Main function for processing QMI recovery complete indication.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_recovery_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr
);

/*=========================================================================

  FUNCTION:  qcril_uim_voltage_supply_resp

===========================================================================*/
/*!
    @brief
    Processes the response for voltage supply command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_voltage_supply_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimVoltageStatusRequestMsg> req_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_process_supply_voltage_ind

===========================================================================*/
/*!
    @brief
    Main function for processing QMI supply voltage indication.
    Note that the data type for QCRIL_EVT_HOOK_UNSOL_UICC_VOLTAGE_STATUS is
    RIL_UiccVoltageStatus

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_supply_voltage_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_direct_get_card_status

===========================================================================*/
/*!
    @brief
    Returns card status from UIM RIL cache

    @return
    None
*/
/*=========================================================================*/
RIL_UIM_Errno qcril_uim_direct_get_card_status
(
  uint8               slot,
  std::shared_ptr<RIL_UIM_CardStatus> ril_card_status_ptr
);

/*=========================================================================

  FUNCTION:  qcril_uim_power_resp

===========================================================================*/
/*!
    @brief
    Processes the response for power up/down command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_power_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  std::shared_ptr<UimCardPowerReqMsg>   msg_ptr
);

/*=========================================================================

  FUNCTION:  qcril_uim_change_prov_session_callback

===========================================================================*/
/*!
    @brief
    Callback for the change provisioning session. It sends an event to
    QCRIL(CM) regarding the status of the activate/deactivate command along
    with the session type.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_change_prov_session_callback
(
  qmi_uim_rsp_data_type                              * rsp_data,
  const std::shared_ptr<UimChangeSubscriptionReqMsg>   req_ptr
);

/*=========================================================================

  FUNCTION:  qcril_uim_cancel_card_state_check_timer

===========================================================================*/
/*!
    @brief
    Cancels the card state check timer

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_cancel_card_state_check_timer
(
  UimTimerCancelReason reason
);

/*=========================================================================

  FUNCTION:  qcril_uim_request_voltage_status

===========================================================================*/
/*!
    @brief
     Handles QCRIL_EVT_HOOK_UICC_VOLTAGE_STATUS_REQ request from QCRIL.
     Note that both the request & response have no data payload.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_voltage_status
(
  const std::shared_ptr<UimVoltageStatusRequestMsg> req_ptr
);

/*=========================================================================

  FUNCTION:  qcril_uim_request_uicc_power

===========================================================================*/
void qcril_uim_request_uicc_power
(
  const std::shared_ptr<UimCardPowerReqMsg> msg_ptr
);

/*=========================================================================

  FUNCTION:  qcril_uim_process_get_atr

===========================================================================*/
/*!
    @brief
     Handles RIL_REQUEST_SIM_GET_ATR and QCRIL_EVT_HOOK_SLOT_GET_ATR_REQ
     request from QCRIL.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_atr
(
  const std::shared_ptr<UimGetAtrRequestMsg> req_ptr
);

/*=========================================================================

  FUNCTION:  qcril_uim_to_uim_rmt_card_app_init_state

===========================================================================*/
/*!
    @brief
    Utility function to derive cardApp state for UIM remote test client usage
    purpose

    @return
    Card App Init status type
*/
/*=========================================================================*/
UimRmtCardAppInitStateType qcril_uim_to_uim_rmt_card_app_init_state
(
  qmi_uim_app_state_type    app_state,
  qmi_uim_perso_state_type  perso_state
);

/*=========================================================================

  FUNCTION:  qcril_uim_determine_card_app_init_status_and_send_indication

===========================================================================*/
/*!
    @brief
    Based on the indication received, this function checks only specific
    fields for the SIM, USIM, RUIM or CSIM Apps, derives a CARD Initialization
        status for UIM Remote internal test clients.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_determine_card_app_init_status_and_send_indication
( uint8 slot,
  const qmi_uim_card_status_type * new_status_ptr
);
#endif /* QCRIL_UIM_CARD_H */

