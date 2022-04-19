/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <android/hardware/radio/config/1.0/IRadioConfig.h>
#include <android/hardware/radio/config/1.0/types.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Mutex.h>

extern "C"
{
  #include "qcril_uim_qcci.h"
  #include "qcril_uim.h"
  #include "qcril_uim_util.h"
  #include "qcril_uim_card.h"
}

using namespace android::hardware::radio;
using namespace android::hardware::radio::config;
using namespace android::hardware::radio::config::V1_0;
using namespace android::hardware::radio::V1_0;
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::android::hardware::Return;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using ::android::hardware::hidl_death_recipient;
using ::android::Mutex;
using android::sp;
using android::hardware::Status;

struct RadioConfigImpl : public IRadioConfig, public hidl_death_recipient {
  sp<IRadioConfigIndication> mIndicationCb;
  sp<IRadioConfigResponse> mResponseCb;

  Mutex mCallbackLock;

  void clearCallbacks();

  void serviceDied( uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);

  Return<void> setResponseFunctions(const sp<IRadioConfigResponse>& radioConfigResponse,
                                    const sp<IRadioConfigIndication>& radioConfigIndication);
  Return<void> getSimSlotsStatus(int32_t serial);
  Return<void> setSimSlotsMapping(int32_t serial, const hidl_vec<uint32_t>& slotMap);
};

static sp<RadioConfigImpl> radioConfigService = NULL;


/*===========================================================================

FUNCTION:  convertUimQmiSlotStatusToHal

===========================================================================*/
static int convertUimQmiSlotStatusToHal
(
  const qmi_uim_slots_status_type *status_rsp_ptr,
  SimSlotStatus ** slot_status_out_ptr
)
{
  uint32_t         i               = 0;
  SimSlotStatus  * slot_status_ptr = NULL;

  if (status_rsp_ptr == NULL || slot_status_out_ptr == NULL || status_rsp_ptr->no_of_slots == 0)
  {
    return 0;
  }

  slot_status_ptr = new SimSlotStatus[status_rsp_ptr->no_of_slots]{};

  if (slot_status_ptr == NULL)
  {
    return 0;
  }

  *slot_status_out_ptr = slot_status_ptr;

  for (; i < status_rsp_ptr->no_of_slots; i++)
  {
    switch(status_rsp_ptr->slot_status[i].card_state)
    {
      case QMI_UIM_SLOT_CARD_STATE_ABSENT:
        slot_status_ptr[i].cardState = CardState::ABSENT;
        break;

      case QMI_UIM_SLOT_CARD_STATE_PRESENT:
        slot_status_ptr[i].cardState = CardState::PRESENT;
        break;

      default:
        slot_status_ptr[i].cardState = CardState::ERROR;
        break;
    }

    switch(status_rsp_ptr->slot_status[i].slot_state)
    {
      case QMI_UIM_SLOT_STATE_ACTIVE:
        slot_status_ptr[i].slotState = SlotState::ACTIVE;
        break;

      default:
        slot_status_ptr[i].slotState = SlotState::INACTIVE;
        break;
    }

    if(slot_status_ptr[i].slotState== SlotState::ACTIVE)
    {
      switch(status_rsp_ptr->slot_status[i].logical_slot)
      {
        case QMI_UIM_SLOT_1:
          slot_status_ptr[i].logicalSlotId = 0;
          break;
        case QMI_UIM_SLOT_2:
          slot_status_ptr[i].logicalSlotId = 1;
          break;
        case QMI_UIM_SLOT_3:
          slot_status_ptr[i].logicalSlotId = 2;
          break;
        default:
          QCRIL_LOG_ERROR("%s", "Invalid input, incorrect logical slot id ");
          break;
      }
    }

    if (slot_status_ptr[i].cardState == CardState::PRESENT)
    {
      char * iccid = nullptr;
      char * atr   = nullptr;

      iccid = qcril_uim_util_iccid_to_string(status_rsp_ptr->slot_status[i].iccid,
                                             status_rsp_ptr->slot_status[i].iccid_len);

      if (status_rsp_ptr->slot_status[i].atr_len == 0)
      {
        /* Attempt to get the ATR only when slot is active */
        if (slot_status_ptr[i].slotState == SlotState::ACTIVE)
        {
          qmi_uim_get_atr_params_type       get_atr_params = {};
          qmi_uim_rsp_data_type             rsp_data       = {};
          int                               qmi_err_code   = 0;

          get_atr_params.slot = status_rsp_ptr->slot_status[i].logical_slot;

          /* get atr */
          qmi_err_code = qcril_qmi_uim_get_atr(qcril_uim.qmi_handle,
                                               &get_atr_params,
                                               NULL,
                                               NULL,
                                               &rsp_data);
          if (qmi_err_code == 0 && rsp_data.qmi_err_code == 0)
          {
            atr = qcril_uim_alloc_bin_to_hexstring(rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr,
                                                   rsp_data.rsp_data.get_atr_rsp.atr_response.data_len);

            if (rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr != NULL)
            {
              qcril_free(rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr);
              rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr = NULL;
            }
          }
        }
      }
      else
      {
        atr = qcril_uim_alloc_bin_to_hexstring(status_rsp_ptr->slot_status[i].atr,
                                               status_rsp_ptr->slot_status[i].atr_len);
      }

      if (atr != nullptr)
      {
        hidl_string hidl_atr = {};

        hidl_atr.setToExternal(atr, strlen(atr));
        slot_status_ptr[i].atr = hidl_atr;

        qcril_free(atr);
      }

      if (iccid != nullptr)
      {
        hidl_string hidl_iccid = {};

        hidl_iccid.setToExternal(iccid, strlen(iccid));
        slot_status_ptr[i].iccid = hidl_iccid;

        qcril_free(iccid);
      }
    }
  }
  return i;
} /* convertUimQmiSlotStatusToHal */


/*===========================================================================

  FUNCTION:  getNumberOfActiveSlots

===========================================================================*/
/*!
    @brief
      This function calculates the number of logically active slots

    @return
       Number of logically active slots
*/
/*=========================================================================*/
static uint32_t getNumberOfActiveSlots
(
  const qmi_uim_slots_status_type *status_rsp_ptr
)
{
  uint32_t  number_of_active_slots = 0;
  uint32_t  index                  = 0;

  if (status_rsp_ptr == NULL || status_rsp_ptr->no_of_slots == 0)
  {
    return 0;
  }

  for (; index < status_rsp_ptr->no_of_slots; index++)
  {
    if(status_rsp_ptr->slot_status[index].slot_state == QMI_UIM_SLOT_STATE_ACTIVE)
    {
      number_of_active_slots++;
    }
  }

  return number_of_active_slots;
} /* convertUimQmiSlotStatusToHal */


/*===========================================================================

FUNCTION:  RadioConfigProcessSlotStatusInd

===========================================================================*/
extern "C" void RadioConfigProcessSlotStatusInd
(
  const qmi_uim_slots_status_type *status_rsp_ptr
)
{
  SimSlotStatus              * slot_status_ptr = NULL;
  uint32_t                     slot_count      = 0;
  hidl_vec<SimSlotStatus>      slotStatus      = {};
  sp<IRadioConfigIndication>   ind_cb          =
    (radioConfigService == nullptr) ? nullptr : radioConfigService->mIndicationCb;

  QCRIL_LOG_INFO("RadioConfigProcessSlotStatusInd");

  if (status_rsp_ptr == nullptr ||
      ind_cb == nullptr)
  {
    return;
  }

  if (getNumberOfActiveSlots(status_rsp_ptr) == 0)
  {

    /* This is invalid since number of logically active slots should be greater than 0.
       Hence,we need to ignore the indication */
    QCRIL_LOG_ERROR("Ignoring invalid slot status change indication");
    return;
  }

  slot_count = convertUimQmiSlotStatusToHal(status_rsp_ptr, &slot_status_ptr);

  slotStatus.setToExternal(slot_status_ptr, slot_count);

  Return<void> ret = ind_cb->simSlotsStatusChanged(RadioIndicationType::UNSOLICITED, slotStatus);

  if (!ret.isOk())
  {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }

  if (slot_status_ptr != NULL)
  {
    delete[] slot_status_ptr;
    slot_status_ptr = NULL;
  }
} /* RadioConfigProcessSlotStatusInd */


/*===========================================================================

FUNCTION:  RadioConfigImpl::setResponseFunctions

===========================================================================*/
Return<void> RadioConfigImpl::setResponseFunctions
(
  const sp<IRadioConfigResponse>& radioConfigResponse,
  const sp<IRadioConfigIndication>& radioConfigIndication
)
{
  qmi_uim_rsp_data_type                    rsp_data         = {};
  int                                      qmi_err_code     = 0;

  QCRIL_LOG_INFO("RadioConfigImpl::setResponseFunctions");

  {
    Mutex::Autolock lock(mCallbackLock);

    if (mResponseCb != nullptr)
    {
      mResponseCb->unlinkToDeath(this);
    }

    mIndicationCb = radioConfigIndication;
    mResponseCb = radioConfigResponse;

    if (mResponseCb != nullptr)
    {
      mResponseCb->linkToDeath(this, 0);
    }
  }

  qmi_err_code = qcril_qmi_uim_get_slots_status(qcril_uim.qmi_handle,
                                                NULL,
                                                NULL,
                                                &rsp_data);
  if (qmi_err_code == 0 && rsp_data.qmi_err_code == 0 &&
      rsp_data.rsp_data.slots_status_rsp.no_of_slots > 0)
  {
    RadioConfigProcessSlotStatusInd(&rsp_data.rsp_data.slots_status_rsp);
  }
  return Status::ok();
} /* RadioConfigImpl::setResponseFunctions */


/*===========================================================================

FUNCTION:  RadioConfigImpl::serviceDied

===========================================================================*/
void RadioConfigImpl::serviceDied
(
 uint64_t,
 const ::android::wp<::android::hidl::base::V1_0::IBase> &
)
{
  QCRIL_LOG_INFO("RadioConfigImpl::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
} /* RadioConfigImpl::serviceDied */


/*===========================================================================

FUNCTION:  RadioConfigImpl::clearCallbacks

===========================================================================*/
void RadioConfigImpl::clearCallbacks()
{
  QCRIL_LOG_INFO("RadioConfigImpl::clearCallbacks");

  Mutex::Autolock lock(mCallbackLock);

  if (mResponseCb != nullptr)
  {
    mResponseCb = nullptr;
  }

  if (mIndicationCb != nullptr)
  {
    mIndicationCb = nullptr;
  }
} /* RadioConfigImpl::clearCallbacks() */


/*===========================================================================

FUNCTION:  RadioConfigImpl::getSimSlotsStatus

===========================================================================*/
Return<void> RadioConfigImpl::getSimSlotsStatus
(
  int32_t serial
)
{
  RadioResponseInfo                        responseInfo    = {};
  qmi_uim_rsp_data_type                    rsp_data        = {};
  int                                      qmi_err_code    = 0;
  SimSlotStatus                          * slot_status_ptr = nullptr;
  hidl_vec<SimSlotStatus>                  slotStatus      = {};
  uint32_t                                 slot_count      = 0;
  sp<IRadioConfigResponse>                 res_cb          = mResponseCb;

  QCRIL_LOG_INFO("Serial: %d", serial);

  if (res_cb == nullptr)
  {
    QCRIL_LOG_INFO("mResponseCb == NULL");
    return Void();
  }

  responseInfo.serial = serial;
  responseInfo.type = RadioResponseType::SOLICITED;
  responseInfo.error = RadioError::NONE;

  do
  {
    qmi_err_code = qcril_qmi_uim_get_slots_status(qcril_uim.qmi_handle,
                                                  NULL,
                                                  NULL,
                                                  &rsp_data);
    if (qmi_err_code != QMI_ERR_NONE_V01 ||
        rsp_data.qmi_err_code != QMI_ERR_NONE_V01 ||
        rsp_data.rsp_data.slots_status_rsp.no_of_slots == 0)
    {
      uint32_t                slot_id         = 0;

      if (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_DSDA)  ||
          qmi_ril_is_feature_supported(QMI_RIL_FEATURE_DSDA2) ||
          qmi_ril_is_feature_supported(QMI_RIL_FEATURE_DSDS))
      {
        slot_count = 2;
      }
      else if (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_TSTS))
      {
        slot_count = 3;
      }
      else
      {
        slot_count = 1;
      }

      slot_status_ptr = new SimSlotStatus[slot_count]{};

      if (slot_status_ptr == nullptr)
      {
        responseInfo.error = RadioError::INTERNAL_ERR;
        break;
      }

      for (; slot_id < slot_count && slot_id < QMI_UIM_MAX_CARD_COUNT; slot_id++)
      {
        RIL_CardStatus_v6        card_status = {};

        slot_status_ptr[slot_id].slotState = SlotState::ACTIVE;
        slot_status_ptr[slot_id].logicalSlotId = slot_id;

        if (qcril_uim_direct_get_card_status(slot_id, &card_status) == RIL_E_SUCCESS)
        {
          switch(card_status.card_state)
          {
            case RIL_CARDSTATE_ABSENT:
              slot_status_ptr[slot_id].cardState = CardState::ABSENT;
              break;

            case RIL_CARDSTATE_PRESENT:
              slot_status_ptr[slot_id].cardState = CardState::PRESENT;
              break;

            default:
              slot_status_ptr[slot_id].cardState = CardState::ERROR;
              break;
          }

          if (slot_status_ptr[slot_id].cardState == CardState::PRESENT)
          {
            hidl_string                   atr                            = {};
            hidl_string                   iccid                          = {};
            uint8_t                       iccid_raw[QCRIL_UIM_ICCID_LEN] = {};
            uint8_t                       iccid_len                      = QCRIL_UIM_ICCID_LEN;
            qmi_uim_get_atr_params_type   get_atr_params                 = {};
            qmi_uim_rsp_data_type         rsp_data                       = {};
            int                           qmi_err_code                   = 0;
            char                        * iccid_ptr                      = NULL;
            char                        * atr_ptr                        = NULL;

            if (qcril_uim_read_iccid(slot_id, iccid_raw, &iccid_len) == RIL_E_SUCCESS)
            {
              iccid_ptr = qcril_uim_util_iccid_to_string(iccid_raw, iccid_len);
              if (iccid_ptr != NULL)
              {
                iccid.setToExternal(iccid_ptr, strlen(iccid_ptr));
              }
            }

            /* get atr */
            get_atr_params.slot = (qmi_uim_slot_type)(slot_id + QMI_UIM_SLOT_1);
            qmi_err_code = qcril_qmi_uim_get_atr(qcril_uim.qmi_handle,
                                                 &get_atr_params,
                                                 NULL,
                                                 NULL,
                                                 &rsp_data);
            if (qmi_err_code == 0 && rsp_data.qmi_err_code == 0)
            {
              atr_ptr = qcril_uim_alloc_bin_to_hexstring(
                                rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr,
                                rsp_data.rsp_data.get_atr_rsp.atr_response.data_len);

              if (rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr != NULL)
              {
                qcril_free(rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr);
                rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr = NULL;
              }

              if (atr_ptr != NULL)
              {
                atr.setToExternal(atr_ptr, strlen(atr_ptr));
              }
            }

            slot_status_ptr[slot_id].atr = atr;
            slot_status_ptr[slot_id].iccid = iccid;

            if (iccid_ptr != NULL)
            {
              qcril_free(iccid_ptr);
              iccid_ptr = NULL;
            }

            if (atr_ptr != NULL)
            {
              qcril_free(atr_ptr);
              atr_ptr = NULL;
            }
          }
        }
      }
    }
    else
    {
      if (getNumberOfActiveSlots(&rsp_data.rsp_data.slots_status_rsp) == 0)
      {

      /* This is invalid since number of logically active slots should be greater than 0.
         Hence,we need to throw error */
        QCRIL_LOG_ERROR("Ignoring invalid slot status change indication");
        responseInfo.error = RadioError::INTERNAL_ERR;
        break;
      }

      slot_count = convertUimQmiSlotStatusToHal(&rsp_data.rsp_data.slots_status_rsp,
                                                &slot_status_ptr);
    }
  } while(0);

  if(slot_status_ptr != NULL)
  {
    slotStatus.setToExternal(slot_status_ptr, slot_count);
  }

  Return<void> ret = res_cb->getSimSlotsStatusResponse(responseInfo, slotStatus);

  if (!ret.isOk())
  {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }

  if (slot_status_ptr != NULL)
  {
    delete[] slot_status_ptr;
    slot_status_ptr = NULL;
  }

  return Void();
} /* RadioConfigImpl::getSimSlotsStatus */


/*===========================================================================

FUNCTION:  RadioConfigImpl::setSimSlotsMapping

===========================================================================*/
Return<void> RadioConfigImpl::setSimSlotsMapping
(
  int32_t                   serial,
  const hidl_vec<uint32_t>& slotMap
)
{
  qmi_uim_physical_slot_mapping_type       params         = {};
  uint32_t                                 slot_id        = 0;
  RadioResponseInfo                        responseInfo   = {};
  qmi_uim_rsp_data_type                    rsp_data       = {};
  int                                      qmi_err_code   = 0;
  sp<IRadioConfigResponse>                 res_cb          = mResponseCb;


  QCRIL_LOG_INFO("%d", serial);

  if (res_cb == nullptr)
  {
    QCRIL_LOG_INFO("mResponseCb == NULL");
    return Void();
  }

  responseInfo.serial = serial;
  responseInfo.type = RadioResponseType::SOLICITED;
  responseInfo.error = RadioError::NONE;

  do
  {
    params.num_slots = slotMap.size();
    if (params.num_slots > QMI_UIM_MAX_CARD_COUNT)
    {
      responseInfo.error = RadioError::INVALID_ARGUMENTS;
      break;
    }

    for(slot_id = 0; slot_id < params.num_slots; slot_id++)
    {
      /* Find slot info */
      switch(slotMap.data()[slot_id])
      {
        case 0:
          params.logical_slot[slot_id] = QMI_UIM_PHY_SLOT_1;
          break;
        case 1:
          params.logical_slot[slot_id] = QMI_UIM_PHY_SLOT_2;
          break;
        case 2:
          params.logical_slot[slot_id] = QMI_UIM_PHY_SLOT_3;
          break;
        default:
          /* Send the response, actual response will be in payload */
          responseInfo.error = RadioError::INVALID_ARGUMENTS;
          break;
      }

      if (responseInfo.error != RadioError::NONE)
      {
        break;
      }
    }

    qmi_err_code = qcril_qmi_uim_switch_slot_ext(qcril_uim.qmi_handle,
                                                 &params,
                                                 NULL,
                                                 NULL,
                                                 &rsp_data);

    if (qmi_err_code != QMI_ERR_NONE_V01 ||
        rsp_data.qmi_err_code != QMI_ERR_NONE_V01)
    {
      responseInfo.error = RadioError::INTERNAL_ERR;
      break;
    }
  } while(0);

  Return<void> ret = res_cb->setSimSlotsMappingResponse(responseInfo);

  if (!ret.isOk())
  {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }

  return Void();
} /* RadioConfigImpl::setSimSlotsMapping */


/*===========================================================================

FUNCTION:  RadioConfigRegisterService

===========================================================================*/
extern "C" void RadioConfigRegisterService
(
  void
)
{
  android::status_t          ret = android::OK;

  if (qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID)
  {
    return;
  }

  if (radioConfigService == NULL)
  {
    radioConfigService = new RadioConfigImpl{};

    if (radioConfigService != NULL)
    {
      ret = radioConfigService->registerAsService("default");
    }
  }
  QCRIL_LOG_INFO("radio_config_register_service status=%d", ret);
} /* RadioConfigRegisterService */
