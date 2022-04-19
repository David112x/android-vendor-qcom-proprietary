/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QMI_RIL_UTF

#define LOG_TAG "RILC"
#define TAG "RadioConfig_Impl_1_2"

#include "RadioConfigImpl_1_2.h"
#include "RadioConfigModule_1_2.h"
#include "modules/uim/UimGetSlotStatusRequestMsg.h"

using android::hardware::radio::V1_0::CardState;
using android::hardware::radio::V1_0::RadioResponseInfo;
using android::hardware::radio::V1_0::RadioResponseType;
using android::hardware::radio::V1_0::RadioIndicationType;
using android::hardware::radio::V1_0::RadioError;
using android::hardware::Status;
using android::hardware::Void;


void RadioConfigImpl_1_2::createRadioConfigModule()
{
  mModule = new RadioConfigModule_1_2();
  if (mModule != nullptr) {
      mModule->init();
  }
}

android::status_t RadioConfigImpl_1_2::registerAsService
(
  const std::string &serviceName
)
{
  QCRIL_LOG_INFO("RadioConfigImpl_1_2:registerAsService(%s)", serviceName.c_str());
  createRadioConfigModule();

  /* Here we need to register with 1.1 because there is no interface for 1.2 service
     rather we are providing support for 1.2 reponses and indications for clients */
  return config::V1_1::IRadioConfig::registerAsService(serviceName);
}

sp<config::V1_2::IRadioConfigResponse> RadioConfigImpl_1_2::getResponseCallback_1_2()
{
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mResponseCb_1_2;
}

sp<config::V1_2::IRadioConfigIndication> RadioConfigImpl_1_2::getIndicationCallback_1_2()
{
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mIndicationCb_1_2;
}

const QcrildConfigVersion &RadioConfigImpl_1_2::getVersion()
{
  static QcrildConfigVersion version(1, 2);
  return version;
}

static void __attribute__((constructor)) registerRadioConfigImpl_1_2();
void registerRadioConfigImpl_1_2()
{
  QCRIL_LOG_INFO("Calling registerRadioConfigImpl_1_2");
   getRadioConfigFactory().registerRadioConfigImpl<RadioConfigImpl_1_2>(nullptr);
}

Return<void> RadioConfigImpl_1_2::setResponseFunctions
(
  const sp<IRadioConfigResponse>& radioConfigResponse,
  const sp<IRadioConfigIndication>& radioConfigIndication
)
{
  QCRIL_LOG_INFO("RadioConfigImpl_1_2::setResponseFunctions");

  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);

    mIndicationCb_1_2 = config::V1_2::IRadioConfigIndication::castFrom(radioConfigIndication).withDefault(nullptr);
    mResponseCb_1_2 = config::V1_2::IRadioConfigResponse::castFrom(radioConfigResponse).withDefault(nullptr);

    if (mResponseCb_1_2 == nullptr || mIndicationCb_1_2 == nullptr)
    {
      mResponseCb_1_2 = nullptr;
      mIndicationCb_1_2 = nullptr;
    }

    QCRIL_LOG_DEBUG("mResponseCb_1_2: %s. mIndicationCb_1_2: %s",
                    mResponseCb_1_2 ? "<not null>" : "<null>",
                    mIndicationCb_1_2 ? "<not null>" : "<null>");

    RadioConfigImpl_1_1::setResponseFunctions_nolock(radioConfigResponse, radioConfigIndication);
  }
  return Status::ok();
} /* RadioConfigImpl_1_2::setResponseFunctions */

void RadioConfigImpl_1_2::clearCallbacks()
{
  QCRIL_LOG_INFO("clearCallbacks");
  mResponseCb_1_2   = NULL;
  mIndicationCb_1_2 =NULL ;

  RadioConfigImpl_1_1::clearCallbacks();
}

/*===========================================================================

FUNCTION:  RadioConfigImpl::getSimSlotsStatus

===========================================================================*/
Return<void> RadioConfigImpl_1_2::getSimSlotsStatus
(
  int32_t serial
)
{
  auto msg = std::make_shared<UimGetSlotStatusRequestMsg>();

  QCRIL_LOG_INFO("Serial: %d", serial);

  sp<config::V1_2::IRadioConfigResponse>  res_cb_1_2  = getResponseCallback_1_2();

  if(res_cb_1_2 == nullptr)
  {
    return RadioConfigImpl_1_1::getSimSlotsStatus(serial);
  }
  else
  {
    if (msg)
    {
     GenericCallback<RIL_UIM_SlotsStatusInfo> cb(
         ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                         std::shared_ptr<RIL_UIM_SlotsStatusInfo> responseDataPtr) -> void
     {
       std::vector<android::hardware::radio::config::V1_2::SimSlotStatus> slot_status;
       sp<config::V1_2::IRadioConfigResponse>   res_cb = getResponseCallback_1_2();
       RadioResponseInfo  rsp_info = {RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};

       if (solicitedMsg && responseDataPtr &&
           status == Message::Callback::Status::SUCCESS)
       {
         rsp_info.error = (RadioError)responseDataPtr->err;

         if (rsp_info.error == RadioError::NONE)
         {
           slot_status.resize(responseDataPtr->slot_status.size());
           for (int index = 0; index < responseDataPtr->slot_status.size(); index++)
           {
             hidl_string    hidl_eid = {};
             char         * eid      = nullptr;

             convertUimSlotStatusToHal(responseDataPtr->slot_status.at(index),
                                       slot_status[index].base);

             if(responseDataPtr->slot_status.at(index).eid.size() != 0)
             {
               eid = radio_config_bin_to_hexstring
                       (responseDataPtr->slot_status.at(index).eid.data(),
                        responseDataPtr->slot_status.at(index).eid.size());
             }

             if(eid != nullptr)
             {
               hidl_eid.setToExternal(eid, strlen(eid));
             }

             slot_status[index].eid = hidl_eid;

            if(eid != nullptr)
            {
              delete[] eid;
            }
           }
         }
       }

       if (res_cb != nullptr)
       {
          Return<void> ret = res_cb->getSimSlotsStatusResponse_1_2(rsp_info, slot_status);

          if (!ret.isOk())
          {
              QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
          }
       }
       else
       {
         QCRIL_LOG_INFO("RadioConfigImpl_1_2:: callback is null");
       }
     }));
     msg->setCallback(&cb);
     msg->dispatch();
    }
    else
    {
     std::vector<android::hardware::radio::config::V1_2::SimSlotStatus> slot_status = {};
     RadioResponseInfo  rsp_info = {RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};

     Return<void> ret = res_cb_1_2->getSimSlotsStatusResponse_1_2(rsp_info, slot_status);

     if (!ret.isOk())
     {
       QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
     }
    }
  }

  return Void();
}


/*===========================================================================

FUNCTION:  RadioConfigImpl::sendSlotStatusIndication

===========================================================================*/
void RadioConfigImpl_1_2::sendSlotStatusIndication
(
  const std::shared_ptr<UimSlotStatusInd> msg_ptr
)
{
  std::vector<android::hardware::radio::config::V1_2::SimSlotStatus> slotStatus = {};
  sp<config::V1_2::IRadioConfigIndication>   ind_cb = getIndicationCallback_1_2();

  QCRIL_LOG_INFO("handleSlotStatusIndiaction");

  if(msg_ptr == nullptr)
  {
    return;
  }

  if (ind_cb == nullptr)
  {
    RadioConfigImpl_1_1::sendSlotStatusIndication(msg_ptr);
    return;
  }

  slotStatus.resize(msg_ptr->get_status().size());

  for (int index = 0; index < slotStatus.size(); index++)
  {
    hidl_string    hidl_eid = {};
    char         * eid      = nullptr;

    convertUimSlotStatusToHal(msg_ptr->get_status().at(index), slotStatus[index].base);

    if(msg_ptr->get_status().at(index).eid.size() != 0)
    {
      eid = radio_config_bin_to_hexstring(msg_ptr->get_status().at(index).eid.data(),
                                          msg_ptr->get_status().at(index).eid.size());
    }

    if(eid != nullptr)
    {
      hidl_eid.setToExternal(eid, strlen(eid));
    }

    slotStatus[index].eid = hidl_eid;

    if(eid != nullptr)
    {
      delete[] eid;
    }
  }

  Return<void> ret = ind_cb->simSlotsStatusChanged_1_2(V1_0::RadioIndicationType::UNSOLICITED, slotStatus);

  if (!ret.isOk())
  {
    QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
  }
} /* RadioConfigImpl::sendSlotStatusIndication */
#endif
