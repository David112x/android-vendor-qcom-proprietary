
/*===========================================================================

  Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "framework/Log.h"
#include "modules/uim/UimQmiUimRequestMsg.h"
#include "modules/uim_service/UimGetRilPropertySyncMsg.h"
#include "qcril_uim.h"
#include "qcril_uim_sap.h"
#include "qcril_uim_gba.h"
#include "qcril_uim_card.h"
#include "qcril_uim_file.h"
#include "qcril_uim_security.h"
#include "qcril_uim_refresh.h"
#include "qcril_uim_util.h"
#include "qcril_uim_restart.h"
#include "qcril_uim_slot_mapping.h"
#include "UimPBRRefreshRegisterReqMsg.h"
#include "UimModule.h"
#include <vector>
#include <string.h>

#define TAG "QCRIL_UIM"

/* Android property to enable supply voltage feature */
#define QCRIL_UIM_PROPERTY_FEATURE_VCC                 "persist.vendor.radio.qcril_uim_vcc_feature"
#define QCRIL_UIM_PROP_FEATURE_VCC_ENABLED_VALUE       "1"

/* Android property for procedure bytes */
#define QCRIL_UIM_PROP_PROCEDURE_BYTES                 "persist.vendor.radio.procedure_bytes"
#define QCRIL_UIM_PROP_RETRUN_PROCEDURE_BYTES_VALUE    "RETURN"
#define QCRIL_UIM_PROP_SKIP_PROCEDURE_BYTES_VALUE      "SKIP"

/* Global variable with QCRIL status */
qcril_uim_struct_type   qcril_uim;

/*=========================================================================

  FUNCTION:  qmi_uim_card_init_callback

===========================================================================*/
/*!
    @brief
    Special case callback for the intial card status QMI commands.

    @return
    None
*/
/*=========================================================================*/
void qmi_uim_card_init_callback
(
  qmi_uim_rsp_data_type        * rsp_data_ptr
)
{
  qmi_uim_indication_data_type     ind_data;
  qcril_uim_indication_params_type ind_param;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity check */
  if (rsp_data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL rsp_data_ptr, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  memset(&ind_data, 0, sizeof(qmi_uim_indication_data_type));

  if(rsp_data_ptr->qmi_err_code >= 0)
  {
      ind_data.status_change_ind.card_status = rsp_data_ptr->rsp_data.get_card_status_rsp.card_status;

      ind_data.status_change_ind.card_status_validity = rsp_data_ptr->rsp_data.get_card_status_rsp.card_status_validity;

      memset(&ind_param, 0x00, sizeof(ind_param));

      ind_param.ind_id      = QMI_UIM_SRVC_STATUS_CHANGE_IND_MSG;
      ind_param.ind_data    = ind_data;

      qcril_uim_process_status_change_ind(&ind_param);
  }
} /* qmi_uim_card_init_callback */


/*=========================================================================

  FUNCTION:  qcril_uim_reset_state

===========================================================================*/
/*!
    @brief
    Reset state of QCRIL_UIM at power up and whenever modem resets.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_reset_state
(
  void
)
{
  /* Clean up refresh info */
  qcril_uim_init_refresh_info();

  /* Clean up Long APDU & select response info, if any */
  qcril_uim_cleanup_long_apdu_info();
  qcril_uim_cleanup_select_response_info();

  /* Initialize global variables */
  memset(&qcril_uim, 0, sizeof(qcril_uim_struct_type));
  qcril_uim.prov_session_info.session_state_gw_indexes[0] =
                                QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;
  qcril_uim.prov_session_info.session_state_gw_indexes[1] =
                                QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;
  qcril_uim.prov_session_info.session_state_gw_indexes[2] =
                                QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;
  qcril_uim.prov_session_info.session_state_1x_indexes[0] =
                                QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;
  qcril_uim.prov_session_info.session_state_1x_indexes[1] =
                                QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;
  qcril_uim.prov_session_info.session_state_1x_indexes[2] =
                                QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;

  qcril_uim_init_card_status(&qcril_uim.card_status);

  qcril_uim.pin1_info[0].slot = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  qcril_uim.pin1_info[1].slot = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
} /* qcril_uim_reset_state */


/*=========================================================================

  FUNCTION:  qcril_uim_init_state

===========================================================================*/
/*!
    @brief
    Initializes QMI_UIM service and state of QCRIL_UIM. Called during
    initial power up and whenever modem restarts.

    @return
    None.
*/
/*=========================================================================*/
int qcril_uim_init_state
(
  bool *uim_ready_state
)
{
  int                            qmi_err_code        = 0;
  qmi_uim_event_reg_params_type  event_reg_params    = {};
  qmi_uim_rsp_data_type          rsp_data            = {};
  char                           temp_buff[100]      = {0};
  uint8_t                        slot                = 0;
  std::shared_ptr<std::string>   respPtr             = NULL;
  std::shared_ptr<UimGetRilPropertySyncMsg> prop_ptr = NULL;

  if (uim_ready_state == nullptr)
  {
    return -1;
  }

  /* Read procedure bytes property, default return procedure bytes if value not set */
  prop_ptr = std::make_shared<UimGetRilPropertySyncMsg>(QCRIL_UIM_PROP_PROCEDURE_BYTES);
  if (prop_ptr != NULL &&
      (prop_ptr->dispatchSync(respPtr) == Message::Callback::Status::SUCCESS)&&
      respPtr != NULL &&
      strcasecmp(respPtr->c_str(), QCRIL_UIM_PROP_SKIP_PROCEDURE_BYTES_VALUE) == 0)
  {
    qcril_uim.procedure_bytes = QMI_UIM_APDU_SKIP_PROCEDURE_BYTES;
  }
  else
  {
    qcril_uim.procedure_bytes = QMI_UIM_APDU_RETURN_PROCEDURE_BYTES;
  }

  /* Get the saved temparory unlock status of the slot */
  (void)snprintf(temp_buff, 99, "%s_%d", QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS, qmi_ril_get_sim_slot());
  prop_ptr = std::make_shared<UimGetRilPropertySyncMsg>(temp_buff);
  if (prop_ptr != NULL &&
      (prop_ptr->dispatchSync(respPtr) == Message::Callback::Status::SUCCESS)&&
      respPtr != NULL &&
      strcmp(respPtr->c_str(), QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_TRUE) == 0)
  {
    qcril_uim.temp_unlock_status = QMI_UIM_TRUE;
  }

  /* Register for both legacy and extended card status events to accomodate
     modems supporting either event. */
  memset(&event_reg_params, 0, sizeof(qmi_uim_event_reg_params_type));
  event_reg_params.card_status                = QMI_UIM_TRUE;
  event_reg_params.extended_card_status       = QMI_UIM_TRUE;
  event_reg_params.slot_status                = QMI_UIM_TRUE;
  event_reg_params.simlock_temp_unlock_status = QMI_UIM_TRUE;
  event_reg_params.recovery                   = QMI_UIM_TRUE;

  prop_ptr = std::make_shared<UimGetRilPropertySyncMsg>(QCRIL_UIM_PROPERTY_FEATURE_VCC);
  if (prop_ptr != NULL &&
      (prop_ptr->dispatchSync(respPtr) == Message::Callback::Status::SUCCESS)&&
      respPtr != NULL &&
      strcmp(respPtr->c_str(),QCRIL_UIM_PROP_FEATURE_VCC_ENABLED_VALUE) == 0)
  {
    event_reg_params.supply_voltage_status = QMI_UIM_TRUE;
  }

  event_reg_params.sap_connection = QMI_UIM_TRUE;

  qmi_err_code = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_EVENT_REG,
                                             &event_reg_params,
                                             &rsp_data);
  if (qmi_err_code != 0)
  {
    QCRIL_LOG_INFO( "event_reg_params qmi_err_code :%d", qmi_err_code);
    return -1;
  }

  /* Get slot status status just in case modem is done initializing already */
  memset(&rsp_data, 0x00, sizeof(rsp_data));
  qmi_err_code = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_SLOTS_STATUS,
                                             nullptr,
                                             &rsp_data);
  if (qmi_err_code == 0 && rsp_data.qmi_err_code == 0 &&
      rsp_data.rsp_data.slots_status_rsp.no_of_slots > 0)
  {
    for (slot = 0; slot < rsp_data.rsp_data.slots_status_rsp.no_of_slots && slot < QMI_UIM_MAX_SLOT_COUNT; slot++)
    {
      int  logical_slot = (int)rsp_data.rsp_data.slots_status_rsp.slot_status[slot].logical_slot - 1;

      if (logical_slot < QMI_UIM_MAX_CARD_COUNT &&
          logical_slot >= 0 &&
          rsp_data.rsp_data.slots_status_rsp.slot_status[slot].slot_state == QMI_UIM_SLOT_STATE_ACTIVE)
      {
        qcril_uim.card_status.card[logical_slot].physical_slot_id = slot;
      }
    }
  }
  else
  {
    /* Initialize the physical_slot_id with its own logical slot instance */
    for (slot = 0; slot < QMI_UIM_MAX_CARD_COUNT; slot++)
    {
      qcril_uim.card_status.card[slot].physical_slot_id = slot;
    }
  }

  /* Get card status just in case modem is done initializing already */
  memset(&rsp_data, 0x00, sizeof(rsp_data));
  qmi_err_code = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_CARD_STATUS,
                                             nullptr,
                                             &rsp_data);
  if (qmi_err_code != 0)
  {
    QCRIL_LOG_INFO("QCRIL_UIM_REQUEST_GET_CARD_STATUS qmi_err_code :%d", qmi_err_code);
    return -1;
  }

  /* Mandatory to set uim ready state before reading the sim state from modem and
     populate the SIM state */
  *uim_ready_state = true;

  qmi_uim_card_init_callback(&rsp_data);

  /* Get modem temparory unlock status and send expiry indication if expired */
  qcril_uim_check_and_send_temp_unlock_expiry_ind();
  return 0;
} /* qcril_uim_init_state */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*=========================================================================

  FUNCTION:  qcril_uim_process_qmi_callback

===========================================================================*/
void qcril_uim_process_qmi_callback
(
  qcril_uim_callback_params_type *rsp_data_ptr,
  std::shared_ptr<Message>        req_ptr
)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity checks */
  if ((rsp_data_ptr == NULL) || (req_ptr == NULL))
  {
    QCRIL_LOG_ERROR("Invalid input, NULL callback_params_ptr");
    return;
  }

  QCRIL_LOG_DEBUG( "qcril_uim_process_qmi_callback: Response for resp: %d \n",
                    rsp_data_ptr->qmi_rsp_data.rsp_id);

  switch(rsp_data_ptr->qmi_rsp_data.rsp_id)
  {
    case QMI_UIM_SRVC_READ_TRANSPARENT_RSP_MSG:
      {
        std::shared_ptr<UimReadTransparentReqMsg> msg_ptr =
             std::static_pointer_cast<UimReadTransparentReqMsg>(req_ptr);
        std::shared_ptr<Message>   msg = msg_ptr->get_request_msg();

        if (msg == NULL)
        {
          return;
        }

        if (string(UimGetImsiRequestMsg::MESSAGE_NAME) == msg->get_message_name())
        {
          qcril_uim_get_imsi_resp(rsp_data_ptr,
                                  msg_ptr);
        }
        else if (string(UimGetMccMncRequestMsg::MESSAGE_NAME) == msg->get_message_name())
        {
          qcril_uim_get_mcc_mnc_resp(rsp_data_ptr,
                                     msg_ptr);
        }
        else if (string(UimGetGIDRequestMsg::MESSAGE_NAME) == msg->get_message_name())
        {
          qcril_uim_get_gid_resp(rsp_data_ptr,
                                 msg_ptr);
        }
        else
        {
          qcril_uim_read_binary_resp(rsp_data_ptr,
                                     msg_ptr);
        }
      }
      break;

    case QMI_UIM_SRVC_READ_RECORD_RSP_MSG:
      {
        std::shared_ptr<UimReadRecordReqMsg> msg_ptr =
             std::static_pointer_cast<UimReadRecordReqMsg>(req_ptr);
        std::shared_ptr<Message>   msg = msg_ptr->get_request_msg();

        if (msg == NULL)
        {
          return;
        }

        if (string(UimPBRRefreshRegisterReqMsg::MESSAGE_NAME) == msg->get_message_name())
        {
          qcril_uim_refresh_read_pbr_record_resp(rsp_data_ptr, msg_ptr);
        }
        else
        {
          qcril_uim_read_record_resp(rsp_data_ptr,
                                     msg_ptr);
        }
      }
      break;

    case QMI_UIM_SRVC_WRITE_TRANSPARENT_RSP_MSG:
      qcril_uim_update_binary_resp(rsp_data_ptr,
          std::static_pointer_cast<UimUpdateBinaryReqMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_WRITE_RECORD_RSP_MSG:
      qcril_uim_update_record_resp(rsp_data_ptr,
          std::static_pointer_cast<UimUpdateRecordReqMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_GET_FILE_ATTRIBUTES_RSP_MSG:
      {
        std::shared_ptr<UimGetAttributesReqMsg> msg_ptr =
             std::static_pointer_cast<UimGetAttributesReqMsg>(req_ptr);
        std::shared_ptr<Message>   msg = msg_ptr->get_request_msg();

        if (msg == NULL)
        {
          return;
        }

        if (string(UimPBRRefreshRegisterReqMsg::MESSAGE_NAME) == msg->get_message_name())
        {
          qcril_uim_refresh_get_pbr_attr_resp(rsp_data_ptr, msg_ptr);
        }
        else
        {
          qcril_uim_get_response_resp(rsp_data_ptr,
                                      msg_ptr);
        }
      }
      break;

    case QMI_UIM_SRVC_REFRESH_REGISTER_RSP_MSG:
      qcril_uim_refresh_register_resp(rsp_data_ptr,
          std::static_pointer_cast<UimRefreshRegisterReqMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG:
      qcril_uim_pin_resp(rsp_data_ptr,
          std::static_pointer_cast<UimVerifyPINMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG:
      if (string(UimVerifyPIN2Msg::MESSAGE_NAME) == req_ptr->get_message_name())
      {
        qcril_uim_process_internal_verify_pin_command_callback(rsp_data_ptr,
          std::static_pointer_cast<UimVerifyPIN2Msg>(req_ptr));
      }
      else
      {
        qcril_uim_pin_resp(rsp_data_ptr,
            std::static_pointer_cast<UimVerifyPINMsg>(req_ptr));
      }
      break;

    case QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG:
      qcril_uim_pin_resp(rsp_data_ptr,
          std::static_pointer_cast<UimVerifyPINMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG:
      qcril_uim_pin_resp(rsp_data_ptr,
          std::static_pointer_cast<UimVerifyPINMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_DEPERSONALIZATION_RSP_MSG:
      qcril_uim_deperso_resp(rsp_data_ptr,
          std::static_pointer_cast<UimEnterDePersoRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_GET_SERVICE_STATUS_RSP_MSG:
      qcril_uim_get_fdn_status_resp(rsp_data_ptr,
            std::static_pointer_cast<UimGetFacilityLockRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_SET_SERVICE_STATUS_RSP_MSG:
      qcril_uim_set_fdn_status_resp(rsp_data_ptr,
            std::static_pointer_cast<UimSetFacilityLockRequestMsg>(req_ptr));
      break;

     case QMI_UIM_SRVC_AUTHENTICATE_RSP_MSG:
      qcril_uim_sim_authenticate_resp(rsp_data_ptr, req_ptr);
      break;

    case QMI_UIM_SRVC_LOGICAL_CHANNEL_RSP_MSG:
      qcril_uim_close_logical_channel_resp(rsp_data_ptr,
          std::static_pointer_cast<UimSIMCloseChannelRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_OPEN_LOGICAL_CHANNEL_RSP_MSG:
      qcril_uim_open_logical_channel_resp(rsp_data_ptr,
          std::static_pointer_cast<UimSIMOpenChannelRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_SEND_APDU_RSP_MSG:
      qcril_uim_send_apdu_resp(rsp_data_ptr,
          std::static_pointer_cast<UimTransmitAPDURequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_GET_ATR_RSP_MSG:
      qcril_uim_get_atr_resp(rsp_data_ptr,
          std::static_pointer_cast<UimGetAtrRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_SEND_STATUS_RSP_MSG:
      qcril_uim_send_status_resp(rsp_data_ptr,
          std::static_pointer_cast<UimGetStatusReqMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_RESELECT_RSP_MSG:
      qcril_uim_reselect_resp(rsp_data_ptr,
          std::static_pointer_cast<UimReselectReqMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_SUPPLY_VOLTAGE_RSP_MSG:
      qcril_uim_voltage_supply_resp(rsp_data_ptr,
          std::static_pointer_cast<UimVoltageStatusRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_PERSONALIZATION_RSP_MSG:
      qcril_uim_perso_resp(rsp_data_ptr,
          std::static_pointer_cast<UimPersoRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_POWER_DOWN_RSP_MSG:
    case QMI_UIM_SRVC_POWER_UP_RSP_MSG:
      qcril_uim_power_resp(rsp_data_ptr,
         std::static_pointer_cast<UimCardPowerReqMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_GET_PHYSICAL_SLOT_MAPPING_RSP_MSG:
      qcril_uim_get_physical_slot_mapping_resp(rsp_data_ptr,
          std::static_pointer_cast<UimGetPhysicalSlotMappingRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_SWITCH_SLOT_EXT_RSP_MSG:
      qcril_uim_switch_slot_resp(rsp_data_ptr,
          std::static_pointer_cast<UimSwitchSlotRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_SET_APDU_BEHAVIOR_RSP_MSG:
      qcril_uim_set_apdu_behavior_resp(rsp_data_ptr,
          std::static_pointer_cast<UimSetAPDUBehaviorRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_GET_SLOTS_STATUS_RSP_MSG:
      qcril_uim_get_slots_status_resp(rsp_data_ptr,
          std::static_pointer_cast<UimGetSlotStatusRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_GET_CARD_STATUS_RSP_MSG:
    case QMI_UIM_SRVC_EVENT_REG_RSP_MSG:
      break;

    case QMI_UIM_SRVC_CHANGE_PROV_SESSION_RSP_MSG:
      qcril_uim_change_prov_session_callback(&rsp_data_ptr->qmi_rsp_data,
          std::static_pointer_cast<UimChangeSubscriptionReqMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_SAP_CONNECTION_RSP_MSG:
      qcril_uim_sap_qmi_handle_sap_connection_resp(rsp_data_ptr,
          std::static_pointer_cast<UimSAPConnectionRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_SAP_REQUEST_RSP_MSG:
      qcril_uim_sap_qmi_handle_sap_request_resp(rsp_data_ptr,
          std::static_pointer_cast<UimSAPRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_GBA_RSP_MSG:
      qcril_uim_gba_resp(rsp_data_ptr,
          std::static_pointer_cast<UimGBAInitRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_GBA_IMPI_RSP_MSG:
      qcril_uim_gba_get_impi_resp(rsp_data_ptr,
          std::static_pointer_cast<UimGBAGetIMPIRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_TERMINAL_CAPABILITY_RSP_MSG:
      if(string(UimGetLPATermialCapabilitesRequestMsg::MESSAGE_NAME) == req_ptr->get_message_name())
      {
        qcril_uim_get_terminal_capability_resp(rsp_data_ptr,
          std::static_pointer_cast<UimGetLPATermialCapabilitesRequestMsg>(req_ptr));
      }
      else if(string(UimSetLPATermialCapabilitesRequestMsg::MESSAGE_NAME) == req_ptr->get_message_name())
      {
        qcril_uim_set_terminal_capability_resp(rsp_data_ptr,
          std::static_pointer_cast<UimSetLPATermialCapabilitesRequestMsg>(req_ptr));
      }
      else
      {
        QCRIL_LOG_ERROR("Invalid request id %s corresponding to terminal capability response",
                        req_ptr->get_message_name().c_str());
      }
      break;

    case QMI_UIM_SRVC_REMOTE_UNLOCK_RSP_MSG:
      qcril_uim_remote_unlock_resp(rsp_data_ptr,
          std::static_pointer_cast<UimRemoteSimlockRequestMsg>(req_ptr));
      break;

    case QMI_UIM_SRVC_ADD_PROFILE_RSP_MSG:
    case QMI_UIM_SRVC_SET_SIM_PROFILE_RSP_MSG:
    case QMI_UIM_SRVC_GET_SIM_PROFILES_RSP_MSG:
    case QMI_UIM_SRVC_DELETE_PROFILE_RSP_MSG:
    case QMI_UIM_SRVC_UPDATE_PROFILE_NICKNAME_RSP_MSG:
    case QMI_UIM_SRVC_EUICC_MEMORY_RESET_RSP_MSG:
    case QMI_UIM_SRVC_USER_CONSENT_RSP_MSG:
    case QMI_UIM_SRVC_GET_EID_RSP_MSG:
    case QMI_UIM_SRVC_GET_PROFILE_INFO_RSP_MSG:
    case QMI_UIM_SRVC_GET_SET_SERVER_ADDRESS_RSP_MSG:
       qcril_uim_lpa_process_qmi_callback(rsp_data_ptr, 
            std::static_pointer_cast<UimLpaReqMessage>(req_ptr));
       break;
    default:
      break;
  }
} /* qcril_uim_process_qmi_callback */


/*=========================================================================

  FUNCTION:  qcril_uim_process_qmi_indication

===========================================================================*/
void qcril_uim_process_qmi_indication
(
  const std::shared_ptr<UimQmiUimIndicationMsg> ind_ptr
)
{
  qcril_uim_indication_params_type * ind_param_ptr = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity checks */
  if (ind_ptr == NULL || ind_ptr->get_message() == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process request");
    return;
  }

  ind_param_ptr = (qcril_uim_indication_params_type*)ind_ptr->get_message();

  switch(ind_ptr->get_ind_id())
  {
    case QMI_UIM_SRVC_STATUS_CHANGE_IND_MSG:
      qcril_uim_process_status_change_ind(ind_param_ptr);
      break;

    case QMI_UIM_SRVC_REFRESH_IND_MSG:
      qcril_uim_process_refresh_ind(ind_param_ptr);
      break;

    case QMI_UIM_SRVC_SEND_APDU_IND_MSG:
      qcril_uim_process_send_apdu_ind(ind_param_ptr);
      break;

    case QMI_UIM_SRVC_RECOVERY_IND_MSG:
      qcril_uim_process_recovery_ind(ind_param_ptr);
      break;

    case QMI_UIM_SRVC_SUPPLY_VOLTAGE_IND_MSG:
      qcril_uim_process_supply_voltage_ind(ind_param_ptr);
      break;

    case QMI_UIM_SRVC_SIMLOCK_TEMP_UNLOCK_IND_MSG:
      qcril_uim_process_simlock_temp_unlock_ind(ind_param_ptr);
      break;

    case QMI_UIM_SRVC_SLOT_STATUS_CHANGE_IND_MSG:
      qcril_uim_process_slot_status_change_ind(ind_param_ptr);
      break;

    case QMI_UIM_SRVC_SAP_IND_MSG:
      qcril_uim_sap_ind(&ind_param_ptr->ind_data.sap_ind);
      break;

    case QMI_UIM_SRVC_LPA_APP_PROFILE_IND_MSG:
      qcril_uim_lpa_add_profile_ind(ind_param_ptr);
      break;

    default:
      QCRIL_LOG_ERROR("Unsupported indication! 0x%x\n", ind_param_ptr->ind_id);
      break;
  }
} /* qcril_uim_process_qmi_indication */


/*=========================================================================

  FUNCTION:  qcril_uim_send_qmi_sync_msg

===========================================================================*/
/*!
    @brief
    QMI UIM send qmi sync message.

    @return
    None
*/
/*=========================================================================*/
int qcril_uim_send_qmi_sync_msg
(
  uint32                        msg_id,
  const void                  * params,
  qmi_uim_rsp_data_type       * rsp_data_ptr
)
{
  int  ret         = -1;
  auto req_msg_ptr = std::make_shared<UimQmiUimRequestMsg>(msg_id,
                                                           params,
                                                           (void *)rsp_data_ptr,
                                                           nullptr);

  if (req_msg_ptr != nullptr)
  {
    std::shared_ptr<int>  respPtr        = nullptr;

    if ((req_msg_ptr->dispatchSync(respPtr) == Message::Callback::Status::SUCCESS) &&
        (respPtr != nullptr))
    {
      ret = *respPtr;
    }
  }

  return ret;
} /* qcril_uim_send_qmi_sync_msg */


/*=========================================================================

  FUNCTION:  qcril_uim_send_qmi_async_msg

===========================================================================*/
/*!
    @brief
    QMI UIM send qmi async message.

    @return
    None
*/
/*=========================================================================*/

int qcril_uim_send_qmi_async_msg
(
  qcril_uim_request_type                     request_type,
  void                                     * param_data_ptr,
  std::shared_ptr<Message>                   req_ptr
)
{
  UimModule *uModule = getUimModule();

  if (uModule == nullptr)
  {
    return -1;
  }

  return uModule->UimSendQmiUimRequest(request_type,
                                       param_data_ptr,
                                       req_ptr);
} /* qcril_uim_send_qmi_async_msg */


#ifdef QMI_RIL_UTF
void qcril_qmi_hal_uim_module_cleanup()
{
    UimModule *uModule = getUimModule();

    uModule->qcrilHalUimModuleCleanup();
    qcril_uim_reset_state();
}
#endif
