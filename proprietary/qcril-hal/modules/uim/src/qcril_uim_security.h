/******************************************************************************
#  Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef QCRIL_UIM_SECURITY_H
#define QCRIL_UIM_SECURITY_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "modules/uim/UimEnterSimPinRequestMsg.h"
#include "modules/uim/UimEnterSimPukRequestMsg.h"
#include "modules/uim/UimChangeSimPinRequestMsg.h"
#include "modules/uim/UimEnterDePersoRequestMsg.h"
#include "modules/uim/UimPersoRequestMsg.h"
#include "modules/uim/UimRemoteSimlockRequestMsg.h"
#include "modules/uim/UimGetPersoStatusRequestMsg.h"
#include "modules/uim/UimGetFacilityLockRequestMsg.h"
#include "modules/uim/UimSetFacilityLockRequestMsg.h"

#include "UimVerifyPINMsg.h"
#include "qcril_uim.h"

/* Android property the temperory unlock status */
#define QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS         "persist.vendor.radio.uim_temp_unlock"
#define QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_TRUE    "1"
#define QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_FALSE   "0"

/*===========================================================================

  FUNCTION:  qcril_uim_pin_resp

===========================================================================*/
/*!
    @brief
    Handle PIN operation confirmations

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_pin_resp
(
  qcril_uim_callback_params_type  * params_ptr,
  const std::shared_ptr<UimVerifyPINMsg> req_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_deperso_resp

===========================================================================*/
/*!
    @brief
    Handle deperso operation confirmation

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_deperso_resp
(
  qcril_uim_callback_params_type  * params_ptr,
  const std::shared_ptr<UimEnterDePersoRequestMsg> req_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_perso_resp

===========================================================================*/
/*!
    @brief
    Handle perso operation confirmation

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_perso_resp
(
  qcril_uim_callback_params_type  * params_ptr,
  const std::shared_ptr<UimPersoRequestMsg>   req_ptr
);

/*===========================================================================
  FUNCTION:  qcril_uim_remote_unlock_resp
===========================================================================*/
/*!
    @brief
    Handle remote unlock response
    @return
    None
*/
/*=========================================================================*/
void qcril_uim_remote_unlock_resp
(
  qcril_uim_callback_params_type  * params_ptr,
  const std::shared_ptr<UimRemoteSimlockRequestMsg> req_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_request_set_pin_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS request from
    the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_set_pin_status
(
  const std::shared_ptr<UimSetFacilityLockRequestMsg> req_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_request_get_pin_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_pin_status
(
  const std::shared_ptr<UimGetFacilityLockRequestMsg> req_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_process_simlock_temp_unlock_ind

===========================================================================*/
/*!
    @brief
    Main function for processing QMI SIMLOCK temporary unlock indication.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_simlock_temp_unlock_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_check_and_send_temp_unlock_expiry_ind

===========================================================================*/
/*!
    @brief
    Function to check the temperory unlock status and send expiry indication

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_check_and_send_temp_unlock_expiry_ind
(
  void
);

/*===========================================================================

  FUNCTION:  qcril_uim_request_enter_pin

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_ENTER_SIM_PIN/RIL_REQUEST_ENTER_SIM_PIN2 requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_enter_pin
(
  const std::shared_ptr<UimEnterSimPinRequestMsg> msg_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_request_enter_puk

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_ENTER_SIM_PUK/RIL_REQUEST_ENTER_SIM_PUK2 requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_enter_puk
(
  const std::shared_ptr<UimEnterSimPukRequestMsg> msg_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_request_change_pin

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CHANGE_SIM_PIN/RIL_REQUEST_CHANGE_SIM_PIN2 requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_change_pin
(
  const std::shared_ptr<UimChangeSimPinRequestMsg> msg_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_request_enter_perso_key

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION or
    QCRIL_EVT_HOOK_ENTER_DEPERSONALIZATION_REQ requests from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_enter_perso_key
(
  const std::shared_ptr<UimEnterDePersoRequestMsg> msg_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_request_perso_reactivate

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_PERSONALIZATION_REACTIVATE_REQ requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_perso_reactivate
(
  const std::shared_ptr<UimPersoRequestMsg> req_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_request_perso_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_PERSONALIZATION_STATUS_REQ requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_perso_status
(
  const std::shared_ptr<UimGetPersoStatusRequestMsg> req_ptr
);

/*=========================================================================

  FUNCTION:  qcril_uim_request_remote_sim_lock_unlock

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_REMOTE_SIM_LOCK_UNLOCK request from QCRIL.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_remote_sim_lock_unlock
(
  const std::shared_ptr<UimRemoteSimlockRequestMsg> req_ptr
);

#endif /* QCRIL_UIM_SECURITY_H */
