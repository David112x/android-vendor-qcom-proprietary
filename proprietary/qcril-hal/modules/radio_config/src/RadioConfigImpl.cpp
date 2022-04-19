/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QMI_RIL_UTF

#define LOG_TAG "RILC"
#define TAG "RadioConfig_Impl"

#include "RadioConfigImpl.h"
#include "modules/uim/UimGetSlotStatusRequestMsg.h"
#include "modules/uim/UimSwitchSlotRequestMsg.h"
#include "RadioConfigModule.h"

using android::hardware::radio::V1_0::CardState;
using android::hardware::radio::V1_0::RadioResponseInfo;
using android::hardware::radio::V1_0::RadioResponseType;
using android::hardware::radio::V1_0::RadioIndicationType;
using android::hardware::radio::V1_0::RadioError;
using android::hardware::radio::config::V1_0::SimSlotStatus;
using android::hardware::radio::config::V1_0::SlotState;
using android::hardware::Status;
using android::hardware::Void;

extern sp<RadioConfigImpl> radioConfigService;


/*===========================================================================

FUNCTION:  getRadioConfig

===========================================================================*/
sp<RadioConfigImpl> getRadioConfig()
{
  return radioConfigService;
} /* getRadioConfig */


/*===========================================================================

FUNCTION:  registerRadioConfigImpl()

===========================================================================*/
static void __attribute__((constructor)) registerRadioConfigImpl();
void registerRadioConfigImpl()
{
    QCRIL_LOG_INFO("Calling registerRadioConfigImpl");
    getRadioConfigFactory().registerRadioConfigImpl<RadioConfigImpl>(nullptr);
} /* registerRadioConfigImpl */


/*===========================================================================

FUNCTION:  RadioConfigImpl::sendSlotStatusIndication

===========================================================================*/
void RadioConfigImpl::sendSlotStatusIndication
(
  const std::shared_ptr<UimSlotStatusInd> msg_ptr
)
{
  std::vector<SimSlotStatus>   slotStatus = {};
  sp<IRadioConfigIndication>   ind_cb     = getIndicationCallback();

  QCRIL_LOG_INFO("handleSlotStatusIndiaction");

  if (msg_ptr == nullptr ||
      ind_cb == nullptr)
  {
    return;
  }

  slotStatus.resize(msg_ptr->get_status().size());

  for (uint8 index = 0; index < slotStatus.size(); index++)
  {
    convertUimSlotStatusToHal(msg_ptr->get_status().at(index), slotStatus[index]);
  }

  Return<void> ret = ind_cb->simSlotsStatusChanged(RadioIndicationType::UNSOLICITED, slotStatus);

  if (!ret.isOk())
  {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }
} /* RadioConfigImpl::sendSlotStatusIndication */


/*===========================================================================

FUNCTION:  RadioConfigImpl::clearCallbacks

===========================================================================*/
void RadioConfigImpl::clearCallbacks()
{
  QCRIL_LOG_INFO("clearCallbacks");

  mIndicationCb = NULL;
  mResponseCb = NULL;
} /* RadioConfigImpl::clearCallbacks */


/*===========================================================================

FUNCTION:  RadioConfigImpl::clearCallbacks

===========================================================================*/
void RadioConfigImpl::serviceDied
(
  uint64_t,
  const ::android::wp<::android::hidl::base::V1_0::IBase> &
)
{
  QCRIL_LOG_DEBUG("RadioImpl::serviceDied: Client died. Cleaning up callbacks");
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);

  clearCallbacks();
} /* RadioConfigImpl::serviceDied */


/*===========================================================================

FUNCTION:  RadioConfigImpl::setResponseFunctions_nolock

===========================================================================*/
Return<void> RadioConfigImpl::setResponseFunctions_nolock
(
  const sp<IRadioConfigResponse>& radioConfigResponse,
  const sp<IRadioConfigIndication>& radioConfigIndication
)
{
  if (mResponseCb != nullptr) {
      mResponseCb->unlinkToDeath(this);
  }

  mResponseCb = radioConfigResponse;
  mIndicationCb = radioConfigIndication;

  QCRIL_LOG_DEBUG("radioConfigResponse: %s. radioConfigIndication: %s",
          mResponseCb ? "<not null>" : "<null>",
          mIndicationCb ? "<not null>" : "<null>");

  if (mResponseCb != nullptr) {
      mResponseCb->linkToDeath(this, 0);
  }

  return Status::ok();
} /* RadioConfigImpl::setResponseFunctions_nolock */


/*===========================================================================

FUNCTION:  RadioConfigImpl::setResponseFunctions

===========================================================================*/
Return<void> RadioConfigImpl::setResponseFunctions
(
  const sp<IRadioConfigResponse>& radioConfigResponse,
  const sp<IRadioConfigIndication>& radioConfigIndication
)
{
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  QCRIL_LOG_INFO("RadioConfigImpl:setResponseFunctions");
  return setResponseFunctions_nolock(radioConfigResponse, radioConfigIndication);
}


/*===========================================================================

FUNCTION:  RadioConfigImpl::getResponseCallback

===========================================================================*/
sp<IRadioConfigResponse> RadioConfigImpl::getResponseCallback()
{
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mResponseCb;
} /* RadioConfigImpl::getResponseCallback */


/*===========================================================================

FUNCTION:  RadioConfigImpl::getIndicationCallback

===========================================================================*/
sp<IRadioConfigIndication> RadioConfigImpl::getIndicationCallback()
{
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mIndicationCb;
} /* RadioConfigImpl::getIndicationCallback */


/*===========================================================================

FUNCTION:  RadioConfigImpl::getRadioConfigModule

===========================================================================*/
Module* RadioConfigImpl::getRadioConfigModule()
{
  return mModule;
} /* RadioConfigImpl::getRadioConfigModule */


/*===========================================================================

FUNCTION:  RadioConfigImpl::createRadioConfigModule

===========================================================================*/
void RadioConfigImpl::createRadioConfigModule()
{
  mModule = new RadioConfigModule();
  if (mModule != nullptr)
  {
    mModule->init();
  }
} /* RadioConfigImpl::createRadioConfigModule */


/*===========================================================================

FUNCTION:  RadioConfigImpl::getVersion

===========================================================================*/
const QcrildConfigVersion &RadioConfigImpl::getVersion()
{
  static QcrildConfigVersion version(1,0);
  return version;
} /* RadioConfigImpl::getVersion */


/*===========================================================================

FUNCTION:  RadioConfigImpl::registerAsService

===========================================================================*/
::android::status_t RadioConfigImpl::registerAsService
(
  const std::string &serviceName
)
{
    QCRIL_LOG_INFO("RadioConfigImpl:registerAsService(%s)", serviceName.c_str());
    createRadioConfigModule();
    return IRadioConfig::registerAsService(serviceName);
} /* RadioConfigImpl::registerAsService */


/*===========================================================================

FUNCTION:  RadioConfigImpl::getSimSlotsStatus

===========================================================================*/
Return<void> RadioConfigImpl::getSimSlotsStatus
(
  int32_t serial
)
{
  auto msg = std::make_shared<UimGetSlotStatusRequestMsg>();

  QCRIL_LOG_INFO("Serial: %d", serial);

  if (msg)
  {
    GenericCallback<RIL_UIM_SlotsStatusInfo> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_SlotsStatusInfo> responseDataPtr) -> void
    {
      sp<IRadioConfigResponse>   res_cb      = getResponseCallback();
      std::vector<SimSlotStatus> slot_status = {};
      RadioResponseInfo          rsp_info    = {RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};

      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        rsp_info.error = (RadioError)responseDataPtr->err;

        if (rsp_info.error == RadioError::NONE)
        {
          slot_status.resize(responseDataPtr->slot_status.size());

          for (uint8 index = 0; index < responseDataPtr->slot_status.size(); index++)
          {
            convertUimSlotStatusToHal(responseDataPtr->slot_status.at(index), slot_status[index]);
          }
        }
      }
      Return<void> ret = res_cb->getSimSlotsStatusResponse(rsp_info, slot_status);

      if (!ret.isOk())
      {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    sp<IRadioConfigResponse>   res_cb      = getResponseCallback();
    std::vector<SimSlotStatus> slot_status = {};
    RadioResponseInfo          rsp_info    = {RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};

    Return<void> ret = res_cb->getSimSlotsStatusResponse(rsp_info, slot_status);

    if (!ret.isOk())
    {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
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
  auto msg = std::make_shared<UimSwitchSlotRequestMsg>(slotMap);

  QCRIL_LOG_INFO("Serial: %d", serial);

  if (msg)
  {
    GenericCallback<RIL_UIM_Errno> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void
    {
      sp<IRadioConfigResponse>   res_cb      = getResponseCallback();
      RadioResponseInfo          rsp_info    = {RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};

      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        rsp_info.error = (RadioError)*(responseDataPtr.get());
      }
      Return<void> ret = res_cb->setSimSlotsMappingResponse(rsp_info);

      if (!ret.isOk())
      {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    sp<IRadioConfigResponse>   res_cb      = getResponseCallback();
    RadioResponseInfo          rsp_info    = {RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};

    Return<void> ret = res_cb->setSimSlotsMappingResponse(rsp_info);

    if (!ret.isOk())
    {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }

  return Void();
} /* RadioConfigImpl::setSimSlotsMapping */


/*===========================================================================

FUNCTION:  RadioConfigImpl::RadioConfigImpl

===========================================================================*/
RadioConfigImpl::RadioConfigImpl()
{
  QCRIL_LOG_INFO("RadioConfigImpl()");
} /* RadioConfigImpl::RadioConfigImpl */


/*===========================================================================

FUNCTION:  getRadioConfigFactory

===========================================================================*/
RadioConfigFactory &getRadioConfigFactory()
{
    static RadioConfigFactory configFactory{};
    return configFactory;
} /* getRadioConfigFactory */

#endif
