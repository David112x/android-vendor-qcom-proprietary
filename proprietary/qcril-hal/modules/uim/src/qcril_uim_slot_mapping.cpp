/*===========================================================================

  Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "framework/Log.h"
#include "modules/uim/qcril_uim_srvc.h"
#include "modules/uim/UimSlotStatusInd.h"
#include "qcril_uim.h"
#include "qcril_uim_util.h"
#include "qcril_uim_slot_mapping.h"


#define TAG "QCRIL_UIM_SLOT_MAPPING"
/*===========================================================================

                               GLOBAL VARIABLES

===========================================================================*/


/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/
/*===========================================================================

  FUNCTION:  qcril_uim_util_get_number_of_logically_active_slots

===========================================================================*/
/*!
    @brief
      This calculates the number of logically active slots from slot status info

    @return
      Number of logically active slots in modem
*/
/*=========================================================================*/
uint8_t qcril_uim_util_get_number_of_logically_active_slots
(
  const std::vector<RIL_UIM_SlotStatus>   &slot_status
)
{
  uint8_t  number_of_active_slots = 0;

  if(slot_status.empty())
  {
    return 0;
  }

  for (auto it = slot_status.begin(); it != slot_status.end(); it++)
  {
    if(it->slot_state == RIL_UIM_PHYSICAL_SLOT_STATE_ACTIVE)
    {
      number_of_active_slots++;
    }
  }

  return number_of_active_slots;

}


static void qcril_uim_copy_slot_status_info
(
  std::vector<RIL_UIM_SlotStatus>  &slot_status,
  const qmi_uim_slots_status_type  *status_ptr
)
{
  if (status_ptr == NULL)
  {
    return;
  }

  if(status_ptr->no_of_slots <= QMI_UIM_MAX_SLOT_COUNT)
  {
    slot_status.resize(status_ptr->no_of_slots);

    for (uint8_t i = 0; i < status_ptr->no_of_slots; i++)
    {
      switch(status_ptr->slot_status[i].card_state)
      {
        case QMI_UIM_SLOT_CARD_STATE_ABSENT:
          slot_status[i].card_state = RIL_UIM_CARDSTATE_ABSENT;
          break;
        case QMI_UIM_SLOT_CARD_STATE_PRESENT:
          slot_status[i].card_state = RIL_UIM_CARDSTATE_PRESENT;
          break;
        default:
          slot_status[i].card_state = RIL_UIM_CARDSTATE_ERROR;
          break;
      }

      slot_status[i].slot_state =
        (RIL_UIM_SlotState)status_ptr->slot_status[i].slot_state;

      slot_status[i].iccid.resize(status_ptr->slot_status[i].iccid_len);
      memcpy(&slot_status[i].iccid[0],
             status_ptr->slot_status[i].iccid,
             status_ptr->slot_status[i].iccid_len);

      slot_status[i].atr.resize(status_ptr->slot_status[i].atr_len);
      memcpy(&slot_status[i].atr[0],
             status_ptr->slot_status[i].atr,
             status_ptr->slot_status[i].atr_len);

      if(status_ptr->slot_status[i].eid_len > 0)
      {
        slot_status[i].eid.resize(status_ptr->slot_status[i].eid_len);
         memcpy(&slot_status[i].eid[0],
                status_ptr->slot_status[i].eid,
                status_ptr->slot_status[i].eid_len);
      }

      if(slot_status[i].slot_state == RIL_UIM_PHYSICAL_SLOT_STATE_ACTIVE)
      {
        switch(status_ptr->slot_status[i].logical_slot)
        {
          case QMI_UIM_SLOT_1:
            slot_status[i].logical_slot = 0;
            break;
          case QMI_UIM_SLOT_2:
            slot_status[i].logical_slot = 1;
            break;
          case QMI_UIM_SLOT_3:
            slot_status[i].logical_slot = 2;
            break;
          default:
            QCRIL_LOG_ERROR("Invalid input, incorrect logical slot id ");
            QCRIL_ASSERT(0);
            return;
        }
        /* Cache physical slot info in card status global */
        if (slot_status[i].logical_slot < QMI_UIM_MAX_CARD_COUNT)
        {
          qcril_uim.card_status.card[slot_status[i].logical_slot].physical_slot_id = i;
        }
      }
      QCRIL_LOG_INFO("LOGICAL_ID-0x%x, CARD STATE 0x%x, ACTIVITY_STATUS-0x%x",
                                                slot_status[i].logical_slot,
                                                slot_status[i].card_state,
                                                slot_status[i].slot_state);
    }
  }
} /* qcril_uim_copy_slot_status_info */


/*=========================================================================

  FUNCTION:  qcril_uim_get_physical_slot_mapping_resp

===========================================================================*/
/*!
    @brief
    Processes the response for physical slot mapping command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_get_physical_slot_mapping_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimGetPhysicalSlotMappingRequestMsg>  req_ptr
)
{
  RIL_UIM_Errno  ril_err  = RIL_UIM_E_INTERNAL_ERR;
  auto           rsp_ptr  = std::make_shared<RIL_UIM_PhySlotMap>();

  /* Sanity checks */
  if (params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_get_physical_slot_mapping_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code );

  if (rsp_ptr != NULL)
  {
    if(params_ptr->qmi_rsp_data.rsp_data.phy_slot_mapping_rsp.num_slots <= QMI_UIM_MAX_CARD_COUNT)
    {
      rsp_ptr->slotMap.resize(params_ptr->qmi_rsp_data.rsp_data.phy_slot_mapping_rsp.num_slots);

      for (uint8_t i = 0; i < rsp_ptr->slotMap.size(); i++)
      {
        switch(params_ptr->qmi_rsp_data.rsp_data.phy_slot_mapping_rsp.logical_slot[i])
        {
          case QMI_UIM_PHY_SLOT_1:
            rsp_ptr->slotMap[i] = 0;
            break;
          case QMI_UIM_PHY_SLOT_2:
            rsp_ptr->slotMap[i] = 1;
            break;
          case QMI_UIM_PHY_SLOT_3:
            rsp_ptr->slotMap[i] = 2;
            break;
          default:
            ril_err = RIL_UIM_E_INTERNAL_ERR;
            break;
        }
      }
    }
    rsp_ptr->err = ril_err;
  }

  /* Send the response */
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_get_physical_slot_mapping_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_get_slots_status_resp

===========================================================================*/
/*!
    @brief
    Processes the response for get slots status.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_get_slots_status_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimGetSlotStatusRequestMsg> req_ptr
)
{
  RIL_UIM_Errno              ril_err                          = RIL_UIM_E_INTERNAL_ERR;
  auto                       rsp_ptr                          = std::make_shared<RIL_UIM_SlotsStatusInfo>();
  uint8_t                    number_of_logically_active_slots = 0;

  /* Sanity checks */
  if (params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_get_slots_status_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code );

  if(rsp_ptr != NULL)
  {
    rsp_ptr->err = ril_err;

    if(ril_err == RIL_UIM_E_SUCCESS)
    {
      qcril_uim_copy_slot_status_info(rsp_ptr->slot_status,
                                  &params_ptr->qmi_rsp_data.rsp_data.slots_status_rsp);

      number_of_logically_active_slots =
        qcril_uim_util_get_number_of_logically_active_slots(rsp_ptr->slot_status);

      if (number_of_logically_active_slots == 0)
      {
        QCRIL_LOG_ERROR("Invalid number of logically active slots");
        rsp_ptr->err = RIL_UIM_E_MODEM_ERR;
      }
    }
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_get_slots_status_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_switch_slot_resp

===========================================================================*/
/*!
    @brief
    Processes the response for switch physical slot command.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_switch_slot_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimSwitchSlotRequestMsg>  req_ptr
)
{
  RIL_UIM_Errno                     ril_err  = RIL_UIM_E_SUCCESS;
  std::shared_ptr<RIL_UIM_Errno>    rsp_ptr  = NULL;

  /* Sanity checks */
  if (params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  if (params_ptr->qmi_rsp_data.qmi_err_code == QMI_UIM_SERVICE_ERR_NO_EFFECT)
  {
    ril_err = RIL_UIM_E_SUCCESS;
  }
  else
  {
    ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);
  }

  QCRIL_LOG_DEBUG( "qcril_uim_switch_slot_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code );

  rsp_ptr = std::make_shared<RIL_UIM_Errno>(ril_err);
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_switch_slot_resp */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_uim_request_get_physical_slot_mapping

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_GET_PHYSICAL_SLOT_MAPPING_REQ request from the
    framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_physical_slot_mapping
(
  const std::shared_ptr<UimGetPhysicalSlotMappingRequestMsg>  req_ptr
)
{
  uint8                               dummy   = 0;
  std::shared_ptr<RIL_UIM_PhySlotMap> rsp_ptr = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity check */
  if (req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_GET_PHYSICAL_SLOT_MAPPING,
                                   (void *)&dummy,
                                   req_ptr) >= 0)
  {
    return;
  }

  rsp_ptr = std::make_shared<RIL_UIM_PhySlotMap>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = RIL_UIM_E_MODEM_ERR;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_get_physical_slot_mapping */


/*===========================================================================

  FUNCTION:  qcril_uim_request_get_slots_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_UIM_REQUEST_GET_SLOT_STATUS request from the
    framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_slots_status
(
  const std::shared_ptr<UimGetSlotStatusRequestMsg> req_ptr
)
{
  std::shared_ptr<RIL_UIM_SlotsStatusInfo> rsp_ptr = NULL;
  uint8                                    dummy   = 0;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity check */
  if (req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_GET_SLOTS_STATUS,
                                   (void *)&dummy,
                                   req_ptr) >= 0)
  {
    return;
  }

  rsp_ptr = std::make_shared<RIL_UIM_SlotsStatusInfo>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = RIL_UIM_E_MODEM_ERR;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
}/* qcril_uim_request_get_slots_status */


/*===========================================================================

  FUNCTION:  qcril_uim_request_switch_slot

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_SWITCH_SLOT_REQ request from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_switch_slot
(
  const std::shared_ptr<UimSwitchSlotRequestMsg>  req_ptr
)
{
  qmi_uim_physical_slot_mapping_type  params            = {};
  RIL_UIM_Errno                       ril_error         = RIL_UIM_E_INTERNAL_ERR;
  std::shared_ptr<RIL_UIM_Errno>      rsp_ptr           = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity check */
  if (req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  const std::vector<uint32_t> logic_phy_mapping = req_ptr->get_slot_mapping();
  params.num_slots  = logic_phy_mapping.size();

  do
  {
    if(params.num_slots > QMI_UIM_MAX_CARD_COUNT)
    {
      ril_error = RIL_UIM_E_INVALID_ARGUMENTS;
      break;
    }

    for(uint8_t i = 0; i < params.num_slots; i++)
    {
      /* Find slot info */
      switch(logic_phy_mapping[i])
      {
        case 0:
          params.logical_slot[i] = QMI_UIM_PHY_SLOT_1;
          break;
        case 1:
          params.logical_slot[i] = QMI_UIM_PHY_SLOT_2;
          break;
        case 2:
          params.logical_slot[i] = QMI_UIM_PHY_SLOT_3;
          break;
        default:
          /* Send the response, actual response will be in payload */
          ril_error = RIL_UIM_E_INVALID_ARGUMENTS;
          break;
      }
    }

    if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_SWITCH_SLOT_EXT,
                                     (void *)&params,
                                     req_ptr) >= 0)
    {
      return;
    }

    ril_error = RIL_UIM_E_MODEM_ERR;
  } while(0);

  rsp_ptr = std::make_shared<RIL_UIM_Errno>(ril_error);

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_switch_slot */


/*===========================================================================

  FUNCTION:  qcril_uim_process_slot_status_change_ind

===========================================================================*/
/*!
    @brief
    Main function for processing QMI slot status changed indication. Based
    on the indication received, if needed, it updates the global slot status.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_slot_status_change_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr
)
{
  std::vector<RIL_UIM_SlotStatus>   status                           = {};
  std::shared_ptr<UimSlotStatusInd> ind_ptr                          = NULL;
  uint8_t                           number_of_logically_active_slots = 0;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (ind_param_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid inputs, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  /* Below event is not supported in RIL UT */
  /* Slot status change notification is sent with oem hook message with slot id and
        slot status.  */
  qcril_uim_copy_slot_status_info(status,
                                  &ind_param_ptr->ind_data.slot_status_change_ind);

  number_of_logically_active_slots = qcril_uim_util_get_number_of_logically_active_slots(status);

  if (number_of_logically_active_slots == 0)
  {
    QCRIL_LOG_ERROR("Invalid number of logically active slots");
    return;
  }

  ind_ptr = std::make_shared<UimSlotStatusInd>(status);

  if (ind_ptr != NULL)
  {
    ind_ptr->broadcast();
  }
} /* qcril_uim_process_slot_status_change_ind */
