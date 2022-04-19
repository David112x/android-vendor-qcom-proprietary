/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QMI_RIL_UTF

#define LOG_TAG "RILC"
#define TAG "RadioConfig_Impl"

#include "RadioConfigImpl_1_1.h"
#include "RadioConfigModule_1_1.h"
#include "UnSolMessages/RadioConfigClientConnectedMessage.h"
#include "interfaces/nas/nas_types.h"
#include "interfaces/nas/RilRequestGetPhoneCapabilityMessage.h"

using android::hardware::radio::V1_0::RadioError;
using android::hardware::radio::V1_0::RadioResponseInfo;
using android::hardware::radio::V1_0::RadioResponseType;
using android::hardware::radio::config::V1_1::PhoneCapability;
using android::hardware::radio::config::V1_1::ModemInfo;
using android::hardware::Status;
using android::hardware::Void;

config::V1_1::PhoneCapability convertPhoneCapabilityToHal(const qcril::interfaces::RIL_PhoneCapability &ril_phoneCap) {
    config::V1_1::PhoneCapability hal_phoneCap;

    hal_phoneCap.maxActiveData = ril_phoneCap.maxActiveData;
    hal_phoneCap.maxActiveInternetData = ril_phoneCap.maxActiveInternetData;
    hal_phoneCap.isInternetLingeringSupported = ril_phoneCap.isInternetLingeringSupported;

    hal_phoneCap.logicalModemList.resize(ril_phoneCap.logicalModemList.size());

    for (int i = 0; i < ril_phoneCap.logicalModemList.size(); i++) {
        hal_phoneCap.logicalModemList[i].modemId = ril_phoneCap.logicalModemList[i].modemId;
    }

    return hal_phoneCap;
}

void RadioConfigImpl_1_1::createRadioConfigModule() {
    mModule = new RadioConfigModule_1_1();
    if (mModule != nullptr) {
        mModule->init();
    }
}

android::status_t RadioConfigImpl_1_1::registerAsService(const std::string &serviceName) {
    QCRIL_LOG_INFO("RadioConfigImpl_1_1:registerAsService(%s)", serviceName.c_str());
    createRadioConfigModule();
    return config::V1_1::IRadioConfig::registerAsService(serviceName);
}

sp<config::V1_1::IRadioConfigResponse> RadioConfigImpl_1_1::getResponseCallback_1_1() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mResponseCb_1_1;
}

const QcrildConfigVersion &RadioConfigImpl_1_1::getVersion() {
    static QcrildConfigVersion version(1, 1);
    return version;
}

static void __attribute__((constructor)) registerRadioConfigImpl_1_1();
void registerRadioConfigImpl_1_1()
{
    QCRIL_LOG_INFO("Calling registerRadioConfigImpl_1_1");
    getRadioConfigFactory().registerRadioConfigImpl<RadioConfigImpl_1_1>(nullptr);
}

int convertToHidl(rildata::ResponseError_t rilError, RadioError &hidlError) {
    switch (rilError) {
        case rildata::ResponseError_t::NO_ERROR:
            hidlError = RadioError::NONE;
            break;
        case rildata::ResponseError_t::NOT_AVAILABLE:
            hidlError = RadioError::RADIO_NOT_AVAILABLE;
            break;
        case rildata::ResponseError_t::INTERNAL_ERROR:
            hidlError = RadioError::INTERNAL_ERR;
            break;
        case rildata::ResponseError_t::INVALID_ARGUMENT:
            hidlError = RadioError::INVALID_ARGUMENTS;
            break;
        default:
            hidlError = RadioError::INTERNAL_ERR;
            break;
    }
    return static_cast<int>(hidlError);
}

Return<void> RadioConfigImpl_1_1::setResponseFunctions_nolock(
    const sp<IRadioConfigResponse>& radioConfigResponse,
    const sp<IRadioConfigIndication>& radioConfigIndication
) {
    {
        /* Set Response cbs for 1_1 here */
        mIndicationCb_1_1 = config::V1_1::IRadioConfigIndication::castFrom(radioConfigIndication).withDefault(nullptr);
        mResponseCb_1_1 = config::V1_1::IRadioConfigResponse::castFrom(radioConfigResponse).withDefault(nullptr);

        //V1_1::IRadioConfigIndication does not have any indication API, so clients may set only
        //IRadioConfigResponse in setResponseFunctions().
        //setting both mIndicationCb_1_1,mResponseCb_1_1 to nullptr is not required.
        //each request implementation should have proper nullptr checks when sending response.

        QCRIL_LOG_DEBUG("mResponseCb_1_1: %s. mIndicationCb_1_1: %s",
                mResponseCb_1_1 ? "<not null>" : "<null>",
                mIndicationCb_1_1 ? "<not null>" : "<null>");

        auto msg = std::make_shared<rildata::RadioConfigClientConnectedMessage>();
        if (msg != nullptr) {
            QCRIL_LOG_DEBUG("RadioConfigImpl_1_1: broadcasting client connected");
            msg->broadcast();
        } else {
            QCRIL_LOG_ERROR("RadioConfigImpl_1_1:: failed to allocate RadioConfigClientConnectedMessage");
        }
        return RadioConfigImpl::setResponseFunctions_nolock(radioConfigResponse, radioConfigIndication);;
    }
}

Return<void> RadioConfigImpl_1_1::setResponseFunctions(
        const sp<IRadioConfigResponse>& radioConfigResponse,
        const sp<IRadioConfigIndication>& radioConfigIndication
) {
    QCRIL_LOG_INFO("RadioConfigImpl_1_1::setResponseFunctions");

    {
        std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
        RadioConfigImpl_1_1::setResponseFunctions_nolock(radioConfigResponse, radioConfigIndication);
    }
    return Status::ok();
} /* RadioConfigImpl_1_1::setResponseFunctions */


void RadioConfigImpl_1_1::clearCallbacks()
{
  QCRIL_LOG_INFO("clearCallbacks");
  mResponseCb_1_1   = NULL;
  mIndicationCb_1_1 = NULL ;

  RadioConfigImpl::clearCallbacks();
}

Return<void> RadioConfigImpl_1_1::getPhoneCapability(int32_t serial)
{
    QCRIL_LOG_INFO("RadioConfigImpl_1_1:: getPhoneCapability serial=%d", serial);

    auto msg = std::make_shared<RilRequestGetPhoneCapabilityMessage>();

    if (msg)
    {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          sp<config::V1_1::IRadioConfigResponse> resp_cb = getResponseCallback_1_1();

          if (resp_cb != nullptr)
          {
              std::shared_ptr<qcril::interfaces::RilPhoneCapabilityResult_t> ril_phoneCapability = nullptr;
              config::V1_1::PhoneCapability hidl_phoneCapability = {};
              RadioResponseInfo resp_info = {RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
              (void)solicitedMsg;

              if (status == Message::Callback::Status::SUCCESS && resp != nullptr)
              {
                  resp_info.error = (RadioError)resp->errorCode;
                  ril_phoneCapability = std::static_pointer_cast<qcril::interfaces::RilPhoneCapabilityResult_t>(resp->data);
              }

              if (ril_phoneCapability != nullptr)
              {
                  hidl_phoneCapability = convertPhoneCapabilityToHal(ril_phoneCapability->phoneCap);
              }

              Return<void> ret = resp_cb->getPhoneCapabilityResponse(resp_info, hidl_phoneCapability);
              if (!ret.isOk())
              {
                  QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
              }
          }
          else
          {
              QCRIL_LOG_ERROR("RadioConfigImpl_1_1:: callback is null");
          }
      }));
      msg->setCallback(&cb);
      msg->dispatch();
    }
    else
    {
      sp<config::V1_1::IRadioConfigResponse> resp_cb = getResponseCallback_1_1();
      config::V1_1::PhoneCapability hidl_phoneCapability = {};
      RadioResponseInfo resp_info = {RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};

      Return<void> ret = resp_cb->getPhoneCapabilityResponse(resp_info, hidl_phoneCapability);
      if (!ret.isOk())
      {
          QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }

    return Void();

} /* RadioConfigImpl_1_1::getPhoneCapability */

Return<void> RadioConfigImpl_1_1::setPreferredDataModem(int32_t serial, uint8_t modemId)
{
    QCRIL_LOG_INFO("RadioConfigImpl_1_1:: setPreferredDataModem serial=%d modemId=%d", serial, modemId);
    auto msg = std::make_shared<rildata::SetPreferredDataModemRequestMessage>(modemId);
    auto cb = std::bind(&RadioConfigImpl_1_1::setPreferredDataModemResponse,
                        this, serial, std::placeholders::_3);
    GenericCallback<rildata::SetPreferredDataModemResponse_t> responseCb(cb);
    if (msg != nullptr) {
        msg->setCallback(&responseCb);
        msg->dispatch();
    } else {
        QCRIL_LOG_ERROR("RadioConfigImpl_1_1:: failed to allocate setPreferredDataModem message");
        rildata::SetPreferredDataModemResponse_t errorResponse = {
        .errCode = rildata::SetPreferredDataModemResult_t::DDS_SWITCH_FAILED
        };
        setPreferredDataModemResponse(serial,
                std::make_shared<rildata::SetPreferredDataModemResponse_t>(errorResponse));
    }
    return Status::ok();
} /* RadioConfigImpl_1_1::setPreferredDataModem */

void RadioConfigImpl_1_1::setPreferredDataModemResponse(
        int32_t serial,
        std::shared_ptr<rildata::SetPreferredDataModemResponse_t> response
) {
    QCRIL_LOG_INFO("RadioConfigImpl_1_1:: setPreferredDataModemResponse ENTRY");
    auto respCb = getResponseCallback_1_1();
    if (respCb != nullptr)
    {
        RadioError error = RadioError::INTERNAL_ERR;
        if (response != nullptr) {
            convertToHidl(response->toResponseError(), error);
        }
        RadioResponseInfo info = {
        .type = RadioResponseType::SOLICITED,
        .serial = serial,
        .error = error
        };
        Return<void> ret = respCb->setPreferredDataModemResponse(info);
        if (!ret.isOk())
        {
          QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_INFO("RadioConfigImpl_1_1:: callback is null");
    }
} /* RadioConfigImpl_1_1::setPreferredDataModemResponse */

Return<void> RadioConfigImpl_1_1::getModemsConfig(int32_t serial)
{
    QCRIL_LOG_INFO("serial=%d", serial);

    sp<config::V1_1::IRadioConfigResponse> resp_cb = getResponseCallback_1_1();
    config::V1_1::ModemsConfig hidl_modemsConfig = {};
    /* As of now this request is not supported */
    QCRIL_LOG_INFO("getModemsConfig is not supported");
    RadioResponseInfo resp_info = {RadioResponseType::SOLICITED, serial,
       RadioError::REQUEST_NOT_SUPPORTED};

    if (resp_cb != nullptr)
    {
        Return<void> ret = resp_cb->getModemsConfigResponse(resp_info, hidl_modemsConfig);
        if (!ret.isOk())
        {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }

    } else {
        QCRIL_LOG_INFO("callback is null");
    }

    return Void();

} /* RadioConfigImpl_1_1::getModemsConfig */

Return<void> RadioConfigImpl_1_1::setModemsConfig(
        int32_t serial,
        const config::V1_1::ModemsConfig& modemConfig
) {
    QCRIL_LOG_INFO("serial=%d", serial);
    QCRIL_LOG_INFO("numOfLiveModems=%d", modemConfig.numOfLiveModems);

    sp<config::V1_1::IRadioConfigResponse> resp_cb = getResponseCallback_1_1();
    /* As of now this request is not supported */
    QCRIL_LOG_INFO("setModemsConfig is not supported");
    RadioResponseInfo resp_info = {RadioResponseType::SOLICITED, serial,
       RadioError::REQUEST_NOT_SUPPORTED};

    if (resp_cb != nullptr)
    {
        Return<void> ret = resp_cb->setModemsConfigResponse(resp_info);
        if (!ret.isOk())
        {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_INFO("callback is null");
    }

    return Void();

} /* RadioConfigImpl_1_1::setModemsConfig */
#endif
