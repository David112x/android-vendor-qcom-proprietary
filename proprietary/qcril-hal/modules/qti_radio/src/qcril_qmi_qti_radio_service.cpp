/******************************************************************************
  @ file    qcril_qmi_qti_radio_service.cpp
  @brief   qcril qmi - qtiRadioService_service

  DESCRIPTION
    Implements the server side of the IQtiqtiRadioService interface. Handles RIL
    requests and responses and indications to be received and sent to client
    respectively

  ---------------------------------------------------------------------------

  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#define TAG "RILQ"

#include <cutils/properties.h>
#include "qcril_qmi_qti_radio_service.h"
#include "qcril_log.h"
#include "qcril_other.h"
#include "QtiRadioContext.h"
#include "modules/uim/UimGetAtrRequestMsg.h"
#include "qcril_reqlist.h"
#include <modules/android/qcril_android_event_context.h>
#include "modules/android/ril_request_info.h"
#include "modules/sms/RilRequestCdmaSendSmsMessage.h"
#include "modules/qti_radio/RilRequestNrMessageCallback.h"
#ifndef QMI_RIL_UTF
#include "GetDataNrIconTypeMessage.h"
#endif
#include "SetCarrierInfoImsiEncryptionMessage.h"

#include "interfaces/nas/RilRequestSet5GStatusMessage.h"
#include "interfaces/nas/RilRequestQuery5GStatusMessage.h"
#include "interfaces/nas/RilRequestQueryNrDcParamMessage.h"
#include "interfaces/nas/RilRequestQueryNrBearAllocationMessage.h"
#include "interfaces/nas/RilRequestQueryNrSignalStrengthMessage.h"
#include "interfaces/nas/RilRequestQueryUpperLayerIndInfoMessage.h"
#include "interfaces/nas/RilRequestQuery5gConfigInfoMessage.h"
#include "interfaces/nas/RilRequestEnableEndcMessage.h"
#include "interfaces/nas/RilRequestQueryEndcStatusMessage.h"
#include "interfaces/nas/nas_types.h"

using namespace vendor::qti::hardware::radio::qtiradio::V1_0::implementation;
using ::android::hardware::Void;
using ::vendor::qti::hardware::radio::qtiradio::V2_0::Status;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V1_0 {
namespace implementation {

static sp<QtiRadioImpl> qtiRadioService = nullptr;

QtiRadioImpl::QtiRadioImpl(qcril_instance_id_e_type instanceId) {
  mResponseCb = nullptr;
  mIndicationCb = nullptr;
  mResponseCbV2_0 = nullptr;
  mIndicationCbV2_0 = nullptr;
  mResponseCbV2_1 = nullptr;
  mIndicationCbV2_1 = nullptr;
  mResponseCbV2_2 = nullptr;
  mIndicationCbV2_2 = nullptr;
  mResponseCbV2_3 = nullptr;
  mResponseCbV2_4 = nullptr;
  mInstanceId = instanceId;
}

/*
 * Service Died callback.
 */
void QtiRadioImpl::serviceDied(uint64_t,
                               const ::android::wp<::android::hidl::base::V1_0::IBase> &) {
  QCRIL_LOG_DEBUG("QtiRadioImpl::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
}

/*
 * Resets indication callback upon client's death
 */
void QtiRadioImpl::clearCallbacks() {
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  if (mIndicationCb != nullptr) {
    mIndicationCb = nullptr;
  }
  if (mResponseCb != nullptr) {
    mResponseCb = nullptr;
  }
  if (mResponseCbV2_0 != nullptr) {
    mResponseCbV2_0 = nullptr;
  }
  if (mIndicationCbV2_0 != nullptr) {
    mIndicationCbV2_0 = nullptr;
  }
  if (mResponseCbV2_1 != nullptr) {
    mResponseCbV2_1 = nullptr;
  }
  if (mIndicationCbV2_1 != nullptr) {
    mIndicationCbV2_1 = nullptr;
  }
  if (mResponseCbV2_2 != nullptr) {
    mResponseCbV2_2 = nullptr;
  }
  if (mIndicationCbV2_2 != nullptr) {
    mIndicationCbV2_2 = nullptr;
  }
  if (mResponseCbV2_3 != nullptr) {
    mResponseCbV2_3 = nullptr;
  }
  if (mResponseCbV2_4 != nullptr) {
    mResponseCbV2_4 = nullptr;
  }
}

//===========================================================================
// QtiRadioImpl::setCallback
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Registers the callbacks for IQtiRadioResponse and IQtiRadioIndication
    passed by the client as a parameter

    @return
    None
*/
/*=========================================================================*/
Return<void> QtiRadioImpl::setCallback(const sp<IQtiRadioResponse> &responseCallback,
                                       const sp<IQtiRadioIndication> &indicationCallback) {
  QCRIL_LOG_DEBUG("QtiRadioImpl::setCallback");
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  if (mResponseCb != nullptr) {
    mResponseCb->unlinkToDeath(this);
  }
  mIndicationCb = indicationCallback;
  mResponseCb = responseCallback;
  if (mResponseCb != nullptr) {
    mResponseCb->linkToDeath(this, 0);

    mResponseCbV2_0 = V2_0::IQtiRadioResponse::castFrom(mResponseCb).withDefault (nullptr);
    mIndicationCbV2_0 = V2_0::IQtiRadioIndication::castFrom(mIndicationCb).withDefault (nullptr);

    mResponseCbV2_1 = V2_1::IQtiRadioResponse::castFrom(responseCallback).withDefault (nullptr);
    mIndicationCbV2_1 = V2_1::IQtiRadioIndication::castFrom(indicationCallback).withDefault (nullptr);

    mResponseCbV2_2 = V2_2::IQtiRadioResponse::castFrom(responseCallback).withDefault (nullptr);
    mIndicationCbV2_2 = V2_2::IQtiRadioIndication::castFrom(indicationCallback).withDefault (nullptr);

    mResponseCbV2_3 = V2_3::IQtiRadioResponse::castFrom(responseCallback).withDefault (nullptr);
    mResponseCbV2_4 = V2_4::IQtiRadioResponse::castFrom(responseCallback).withDefault (nullptr);
  }

  return Void();
}

sp<IQtiRadioResponse> QtiRadioImpl::getResponseCallback() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mResponseCb;
}

sp<IQtiRadioIndication> QtiRadioImpl::getIndicationCallback() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mIndicationCb;
}

sp<V2_0::IQtiRadioResponse> QtiRadioImpl::getResponseCallbackV2_0() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mResponseCbV2_0;
}

sp<V2_0::IQtiRadioIndication> QtiRadioImpl::getIndicationCallbackV2_0() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mIndicationCbV2_0;
}

sp<V2_1::IQtiRadioResponse> QtiRadioImpl::getResponseCallbackV2_1() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    return mResponseCbV2_1;
}

sp<V2_1::IQtiRadioIndication> QtiRadioImpl::getIndicationCallbackV2_1() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    return mIndicationCbV2_1;
}

sp<V2_2::IQtiRadioResponse> QtiRadioImpl::getResponseCallbackV2_2() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    return mResponseCbV2_2;
}

sp<V2_2::IQtiRadioIndication> QtiRadioImpl::getIndicationCallbackV2_2() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    return mIndicationCbV2_2;
}

sp<V2_3::IQtiRadioResponse> QtiRadioImpl::getResponseCallbackV2_3() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    return mResponseCbV2_3;
}

sp<V2_4::IQtiRadioResponse> QtiRadioImpl::getResponseCallbackV2_4() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    return mResponseCbV2_4;
}

//===========================================================================
// QtiRadioImpl::getAtr
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the getAtr request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> QtiRadioImpl::getAtr(int32_t serial) {
  QCRIL_LOG_ERROR("QtiRadioImpl::getAtr serial=%d", serial);
  std::shared_ptr<UimGetAtrRequestMsg> req = std::make_shared<UimGetAtrRequestMsg>(qmi_ril_get_process_instance_id());
  if (req) {
      GenericCallback<UimAtrRspParam> cb(
          ([serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                          std::shared_ptr<UimAtrRspParam> resp) -> void {
              if (solicitedMsg && resp &&
                  status == Message::Callback::Status::SUCCESS &&
                  resp->err == RIL_UIM_E_SUCCESS) {
                  implementation::qtiGetAtrResponse(serial, RIL_E_SUCCESS, resp->atr.c_str(), (int)resp->atr.length());
              }
              else {
                  implementation::qtiGetAtrResponse(serial, RIL_E_INTERNAL_ERR, NULL, 0);
              }
      }));
      req->setCallback(&cb);
      req->dispatch();
  }
  return Void();
}

Return<void> QtiRadioImpl::sendCdmaSms(int32_t serial,
        const android::hardware::radio::V1_0::CdmaSmsMessage& sms, bool expectMore) {
    QCRIL_LOG_DEBUG("QtiRadioImpl:: sendCdmaSms serial=%d expectMore=%d", serial, expectMore);
    RIL_CDMA_SMS_Message rcsm = {};
#ifndef QMI_RIL_UTF
        constructCdmaSms(rcsm, sms);
#endif
    if(expectMore) {
        rcsm.expectMore = 1;
    }
    std::shared_ptr<RilRequestCdmaSendSmsMessage>msg =
            std::make_shared<RilRequestCdmaSendSmsMessage>(rcsm);
    if (msg) {
        GenericCallback<RilSendSmsResult_t> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RilSendSmsResult_t> responseDataPtr) -> void {
                (void) status;
                if (solicitedMsg && responseDataPtr) {
                    QtiRadioResponseInfo responseInfo = {};
                    android::hardware::radio::V1_0::SendSmsResult result = {};
                    if (!responseDataPtr) {
                        QCRIL_LOG_ERROR("Invalid response: NULL");
                        populateResponseInfo(responseInfo, serial, QtiRadioResponseType::SOLICITED,
                                RIL_E_INVALID_RESPONSE);
                        result.ackPDU = hidl_string();
                    } else {
                        populateResponseInfo(responseInfo, serial, QtiRadioResponseType::SOLICITED,
                                responseDataPtr->rilErr);
                        result.messageRef = responseDataPtr->messageRef;
                        result.ackPDU = responseDataPtr->ackPDU;
                        result.errorCode = responseDataPtr->errorCode;
                    }

                    auto respCb = getResponseCallbackV2_0();
                    if (respCb) {
                        Return<void> ret = respCb->sendCdmaSmsResponse(responseInfo, result);
                        if (!ret.isOk()) {
                            QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                                ret.description().c_str());
                        }
                    }
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        QtiRadioResponseInfo responseInfo = {QtiRadioResponseType::SOLICITED,
            serial, QtiRadioError::GENERIC_FAILURE};
        android::hardware::radio::V1_0::SendSmsResult result = {-1, hidl_string(), -1};
        auto respCb = getResponseCallbackV2_0();
        if (respCb != nullptr) {
            Return<void> ret = respCb->sendCdmaSmsResponse(responseInfo, result);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                                ret.description().c_str());
            }
        }
    }
    return Void();
}

Return<void> QtiRadioImpl::enable5g(int32_t serial) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::enable5g, serial=%d", serial);

    auto msg = std::make_shared<RilRequestSet5GStatusMessage>(getContext(serial),
            FIVE_G_MODE_INCLUSIVE);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            (void)msg;
            (void)status;
            auto respCb = getResponseCallbackV2_0();
            if (respCb != nullptr) {
                RIL_Errno rilErr = (resp == nullptr) ? RIL_E_NO_MEMORY : resp->errorCode;
                Return<void> ret = respCb->onEnable5gResponse(serial,
                        static_cast<uint32_t>(rilErr), Status::SUCCESS);
                if (!ret.isOk()) {
                    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
                }
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        auto respCb = getResponseCallbackV2_0();
        if (respCb != nullptr) {
            Return<void> ret = respCb->onEnable5gResponse(serial,
                static_cast<uint32_t>(RIL_E_NO_MEMORY), Status::SUCCESS);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
            }
        }
    }

    return Void();
}

Return<void> QtiRadioImpl::disable5g(int32_t serial) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::disable5g, serial=%d", serial);

    auto msg = std::make_shared<RilRequestSet5GStatusMessage>(getContext(serial),
            FIVE_G_MODE_DISABLED);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            (void)msg;
            (void)status;
            auto respCb = getResponseCallbackV2_0();
            if (respCb != nullptr) {
                RIL_Errno rilErr = (resp == nullptr) ? RIL_E_NO_MEMORY : resp->errorCode;
                Return<void> ret = respCb->onDisable5gResponse(serial,
                        static_cast<uint32_t>(rilErr), Status::SUCCESS);
                if (!ret.isOk()) {
                    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
                }
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        auto respCb = getResponseCallbackV2_0();
        if (respCb != nullptr) {
            Return<void> ret = respCb->onDisable5gResponse(serial,
                static_cast<uint32_t>(RIL_E_NO_MEMORY), Status::SUCCESS);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
            }
        }
    }

    return Void();
}

Return<void> QtiRadioImpl::enable5gOnly(int32_t serial) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::enable5gOnly, serial=%d", serial);

    auto msg = std::make_shared<RilRequestSet5GStatusMessage>(getContext(serial),
            FIVE_G_MODE_EXCLUSIVE);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            (void)msg;
            (void)status;
            auto respCb = getResponseCallbackV2_0();
            if (respCb != nullptr) {
                RIL_Errno rilErr = (resp == nullptr) ? RIL_E_NO_MEMORY : resp->errorCode;
                Return<void> ret = respCb->onEnable5gOnlyResponse(serial,
                        static_cast<uint32_t>(rilErr), Status::SUCCESS);
                if (!ret.isOk()) {
                    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
                }
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        auto respCb = getResponseCallbackV2_0();
        if (respCb != nullptr) {
            Return<void> ret = respCb->onEnable5gOnlyResponse(serial,
                static_cast<uint32_t>(RIL_E_NO_MEMORY), Status::SUCCESS);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
            }
        }
    }

    return Void();
}

Return<void> QtiRadioImpl::query5gStatus(int32_t serial) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::query5gStatus, serial=%d", serial);

    auto msg = std::make_shared<RilRequestQuery5GStatusMessage>(getContext(serial));
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            (void)msg;
            (void)status;
            auto respCb = getResponseCallbackV2_0();
            if (respCb != nullptr) {
                RIL_Errno rilErr = RIL_E_GENERIC_FAILURE;
                std::shared_ptr<qcril::interfaces::RilQuery5GStatusResult_t> payload;
                V2_0::EnableStatus status = V2_0::EnableStatus::INVALID;

                if (resp != nullptr) {
                    rilErr = resp->errorCode;
                    payload = std::static_pointer_cast<qcril::interfaces::RilQuery5GStatusResult_t>(resp->data);
                } else {
                    rilErr = RIL_E_NO_MEMORY;
                }

                if (rilErr == RIL_E_SUCCESS && payload != nullptr) {
                    status = (payload->status == FIVE_G_STATUS_DISABLED) ?
                        V2_0::EnableStatus::DISABLED : V2_0::EnableStatus::ENABLED;
                }

                Return<void> ret = respCb->on5gStatusResponse(serial,
                        static_cast<uint32_t>(rilErr), status);
                if (!ret.isOk()) {
                    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
                }
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        auto respCb = getResponseCallbackV2_0();
        if (respCb != nullptr) {
            Return<void> ret = respCb->on5gStatusResponse(serial,
                static_cast<uint32_t>(RIL_E_NO_MEMORY), V2_0::EnableStatus::INVALID);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
            }
        }
    }

    return Void();
}

Return<void> QtiRadioImpl::enableEndc (int32_t serial, bool enable) {
    QCRIL_LOG_DEBUG ("QtiRadioImpl::enableEndc, serial=%d enable:%d", serial, enable);

    auto msg = std::make_shared<RilRequestEnableEndcMessage>(getContext(serial),
            enable);
    if(msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            (void)msg;
            (void)status;
            RIL_Errno rilErr = RIL_E_SYSTEM_ERR;
            if(status != Message::Callback::Status::SUCCESS) {
                QCRIL_LOG_ERROR("Message::Callback::Status : %d", status);
                sp<V2_3::IQtiRadioResponse> respCb = getResponseCallbackV2_3();
                if (respCb != nullptr) {
                    Return<void> ret = respCb->onEnableEndcResponse(serial,
                            static_cast<uint32_t>(rilErr), Status::SUCCESS);
                    if (!ret.isOk()) {
                        QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                                ret.description().c_str());
                    }
                }
            } else {
                sp<V2_3::IQtiRadioResponse> respCb = getResponseCallbackV2_3();
                if (respCb != nullptr) {
                    rilErr = (resp == nullptr) ? RIL_E_NO_MEMORY : resp->errorCode;
                    Return<void> ret = respCb->onEnableEndcResponse(serial,
                            static_cast<uint32_t>(rilErr), Status::SUCCESS);
                    if (!ret.isOk()) {
                        QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                                ret.description().c_str());
                    }
                }
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        sp<V2_3::IQtiRadioResponse> respCb = getResponseCallbackV2_3();
        if(respCb != nullptr) {
            Return<void> ret = respCb->onEnableEndcResponse(serial,
                static_cast<uint32_t>(RIL_E_NO_MEMORY), Status::SUCCESS);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                        ret.description().c_str());
            }
        }
    }
    return Void();
}

Return<void> QtiRadioImpl::queryEndcStatus (int32_t serial) {
    QCRIL_LOG_DEBUG ("QtiRadioImpl::queryEndcStatus, serial=%d", serial);
    auto msg = std::make_shared<RilRequestQueryEndcStatusMessage>(getContext(serial));
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            (void)msg;
            (void)status;
            RIL_Errno rilErr = RIL_E_SYSTEM_ERR;
            V2_3::EndcStatus endcStatus = V2_3::EndcStatus::INVALID;
            if(status != Message::Callback::Status::SUCCESS) {
                QCRIL_LOG_ERROR("Message::Callback::Status : %d", status);
                sp<V2_3::IQtiRadioResponse> respCb = getResponseCallbackV2_3();
                if(resp != nullptr) {
                    Return<void> ret = respCb->onEndcStatusResponse(serial,
                            static_cast<uint32_t>(rilErr), endcStatus);
                    if (!ret.isOk()) {
                        QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                                ret.description().c_str());
                    }
                }
            } else {
                sp<V2_3::IQtiRadioResponse> respCb = getResponseCallbackV2_3();
                if (respCb != nullptr) {
                    std::shared_ptr<qcril::interfaces::RilQueryEndcStatusResult_t> payload;

                    if (resp != nullptr) {
                        rilErr = resp->errorCode;
                        payload = std::static_pointer_cast<qcril::interfaces::
                                RilQueryEndcStatusResult_t>(resp->data);
                    } else {
                        rilErr = RIL_E_NO_MEMORY;
                    }

                    if (rilErr == RIL_E_SUCCESS && payload != nullptr) {
                        endcStatus = (payload->status == ENDC_STATUS_DISABLED) ?
                            V2_3::EndcStatus::DISABLED : V2_3::EndcStatus::ENABLED;
                    }

                    Return<void> ret = respCb->onEndcStatusResponse(serial,
                            static_cast<uint32_t>(rilErr), endcStatus);
                    if (!ret.isOk()) {
                        QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                                ret.description().c_str());
                    }
                }
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        sp<V2_3::IQtiRadioResponse> respCb = getResponseCallbackV2_3();
        if (respCb != nullptr) {
            Return<void> ret = respCb->onEndcStatusResponse(serial,
                static_cast<uint32_t>(RIL_E_NO_MEMORY), V2_3::EndcStatus::INVALID);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                        ret.description().c_str());
            }
        }
    }

    return Void();
}

Return<void> QtiRadioImpl::queryNrDcParam(int32_t serial) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::queryNrDcParam, serial=%d", serial);

    auto msg = std::make_shared<RilRequestQueryNrDcParamMessage>(getContext(serial));
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            (void)msg;
            (void)status;
            auto respCb = getResponseCallbackV2_0();
            if (respCb != nullptr) {
                RIL_Errno rilErr = RIL_E_GENERIC_FAILURE;
                std::shared_ptr<qcril::interfaces::RilQueryNrDcParamResult_t> payload;
                V2_0::DcParam params;
                initialize(params);

                if (resp != nullptr) {
                    rilErr = resp->errorCode;
                    payload = std::static_pointer_cast<qcril::interfaces::RilQueryNrDcParamResult_t>(resp->data);
                } else {
                    rilErr = RIL_E_NO_MEMORY;
                }

                if (rilErr == RIL_E_SUCCESS && payload != nullptr) {
                    params = convert_five_g_endc_dcnr(payload->dc);
                }

                Return<void> ret = respCb->onNrDcParamResponse(serial,
                        static_cast<uint32_t>(rilErr), params);
                if (!ret.isOk()) {
                    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
                }
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        auto respCb = getResponseCallbackV2_0();
        if (respCb != nullptr) {
            V2_0::DcParam params;
            initialize(params);

            Return<void> ret = respCb->onNrDcParamResponse(serial,
                    static_cast<uint32_t>(RIL_E_NO_MEMORY), params);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
            }
        }
    }

    return Void();
}

Return<void> QtiRadioImpl::queryNrBearerAllocation(int32_t serial) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::queryNrBearerAllocation, serial=%d", serial);

    auto msg = std::make_shared<RilRequestQueryNrBearAllocationMessage>(getContext(serial));
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            (void)msg;
            (void)status;
            auto respCb = getResponseCallbackV2_0();
            if (respCb != nullptr) {
                RIL_Errno rilErr = RIL_E_GENERIC_FAILURE;
                V2_0::BearerStatus bearStatus = V2_0::BearerStatus::INVALID;
                std::shared_ptr<qcril::interfaces::RilQueryNrBearAllocResult_t> payload;

                if (resp != nullptr) {
                    rilErr = resp->errorCode;
                    payload = std::static_pointer_cast<qcril::interfaces::RilQueryNrBearAllocResult_t>(
                            resp->data);
                } else {
                    rilErr = RIL_E_NO_MEMORY;
                }

                if (rilErr == RIL_E_SUCCESS && payload != nullptr) {
                    bearStatus = convert_five_g_bearer_status(payload->status);
                }

                Return<void> ret = respCb->onNrBearerAllocationResponse(serial,
                        static_cast<uint32_t>(rilErr), bearStatus);
                if (!ret.isOk()) {
                    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
                }
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        auto respCb = getResponseCallbackV2_0();
        if (respCb != nullptr) {
            Return<void> ret = respCb->onNrBearerAllocationResponse(serial,
                    static_cast<uint32_t>(RIL_E_NO_MEMORY), V2_0::BearerStatus::INVALID);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
            }
        }
    }

    return Void();
}

Return<void> QtiRadioImpl::queryNrSignalStrength(int32_t serial) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::queryNrSignalStrength, serial=%d", serial);

    auto msg = std::make_shared<RilRequestQueryNrSignalStrengthMessage>(getContext(serial));
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            (void)msg;
            (void)status;
            auto respCb = getResponseCallbackV2_0();
            if (respCb != nullptr) {
                RIL_Errno rilErr = RIL_E_GENERIC_FAILURE;
                std::shared_ptr<qcril::interfaces::RilQueryNrSignalStrengthResult_t> payload;
                V2_0::SignalStrength signalStrength;
                initialize(signalStrength);

                if (resp != nullptr) {
                    rilErr = resp->errorCode;
                    payload = std::static_pointer_cast<qcril::interfaces::RilQueryNrSignalStrengthResult_t>(
                            resp->data);
                } else {
                    rilErr = RIL_E_NO_MEMORY;
                }

                if (rilErr == RIL_E_SUCCESS && payload != nullptr) {
                    signalStrength = convert_five_g_signal_strength(payload->signal);
                }

                Return<void> ret = respCb->onSignalStrengthResponse(serial,
                        static_cast<uint32_t>(rilErr), signalStrength);
                if (!ret.isOk()) {
                    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
                }
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        auto respCb = getResponseCallbackV2_0();
        if (respCb != nullptr) {
            V2_0::SignalStrength signalStrength;
            initialize(signalStrength);

            Return<void> ret = respCb->onSignalStrengthResponse(serial,
                    static_cast<uint32_t>(RIL_E_NO_MEMORY), signalStrength);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
            }
        }
    }

    return Void();
}

Return<void> QtiRadioImpl::queryUpperLayerIndInfo(int32_t serial) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::queryUpperLayerIndInfo, serial=%d", serial);

    auto msg = std::make_shared<RilRequestQueryUpperLayerIndInfoMessage>(getContext(serial));
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            (void) msg;
            (void) status;
            auto respCb = getResponseCallbackV2_1();
            if (respCb != nullptr) {
                RIL_Errno rilErr = RIL_E_GENERIC_FAILURE;
                std::shared_ptr<qcril::interfaces::RilQueryUpperLayerIndInfoResult_t> payload;
                V2_1::UpperLayerIndInfo upliInfo;
                initialize(upliInfo);

                if (resp != nullptr) {
                    rilErr = resp->errorCode;
                    payload = std::static_pointer_cast<qcril::interfaces::RilQueryUpperLayerIndInfoResult_t>(
                            resp->data);
                } else {
                    rilErr = RIL_E_NO_MEMORY;
                }

                if (rilErr == RIL_E_SUCCESS && payload != nullptr) {
                    upliInfo = convert_five_g_upper_layer_ind_info(payload->upli_info);
                }

                Return<void> ret = respCb->onUpperLayerIndInfoResponse(serial,
                        static_cast<uint32_t>(rilErr), upliInfo);
                if (!ret.isOk()) {
                    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
                }
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        auto respCb = getResponseCallbackV2_1();
        if (respCb != nullptr) {
            V2_1::UpperLayerIndInfo upliInfo;
            initialize(upliInfo);

            Return<void> ret = respCb->onUpperLayerIndInfoResponse(serial,
                    static_cast<uint32_t>(RIL_E_NO_MEMORY), upliInfo);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
            }
        }
    }

    return Void();
}

Return<void> QtiRadioImpl::query5gConfigInfo(int32_t serial) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::query5gConfigInfo, serial=%d", serial);

    auto msg = std::make_shared<RilRequestQuery5gConfigInfoMessage>(getContext(serial));
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            (void) msg;
            (void) status;
            auto respCb = getResponseCallbackV2_1();

            if (respCb != nullptr) {
                RIL_Errno rilErr = RIL_E_GENERIC_FAILURE;
                V2_1::ConfigType configInfo = V2_1::ConfigType::INVALID;
                std::shared_ptr<qcril::interfaces::RilQuery5gConfigInfoResult_t> payload;

                if (resp != nullptr) {
                    rilErr = resp->errorCode;
                    payload = std::static_pointer_cast<qcril::interfaces::RilQuery5gConfigInfoResult_t>(
                            resp->data);
                } else {
                    rilErr = RIL_E_NO_MEMORY;
                }

                if (rilErr == RIL_E_SUCCESS && payload != nullptr) {
                    configInfo = convert_five_g_config_info(payload->config_info);
                }

                Return<void> ret = respCb->on5gConfigInfoResponse(serial,
                        static_cast<uint32_t>(rilErr), configInfo);
                if (!ret.isOk()) {
                    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
                }
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        auto respCb = getResponseCallbackV2_1();
        if (respCb != nullptr) {
            Return<void> ret = respCb->on5gConfigInfoResponse(serial,
                    static_cast<uint32_t>(RIL_E_NO_MEMORY), V2_1::ConfigType::INVALID);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
            }
        }
    }

    return Void();
}

Return<void> QtiRadioImpl::queryNrIconType(int32_t serial) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::queryNrIconType, serial=%d", serial);
    RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial);
    if (token) {
#ifndef QMI_RIL_UTF
        std::shared_ptr<rildata::GetDataNrIconTypeMessage> msg =
            std::make_shared<rildata::GetDataNrIconTypeMessage>();
        if(msg)
        {
            QtiRadioGetDataNrIconTypeMessageCb cb(token);
            msg->setCallback(&cb);
            msg->dispatch();
        }
#endif
    }
    return Void();
}

void QtiRadioImpl::queryNrIconTypeResponse(RIL_Token token, RIL_Errno err_num,
        five_g_icon_type iconType) {
    if (mResponseCbV2_2 != nullptr) {
        int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(token);
        QCRIL_LOG_DEBUG("QtiRadioImpl::queryNrIconTypeResponse serial=%d", serial);
        Return<void> ret = mResponseCbV2_2->onNrIconTypeResponse(serial,
                static_cast<uint32_t>(err_num), convert_five_g_icon_type(iconType));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_ERROR("QtiRadioImpl::queryNrIconTypeResponse respCb NULL");
    }
}

void QtiRadioImpl::on5gStatusChange(five_g_status status) {
    QCRIL_LOG_DEBUG("QtiRadioImpl:::on5gStatusChange");
    if (mIndicationCbV2_0 != nullptr) {
        Return<void> ret = mIndicationCbV2_0->on5gStatusChange(convert_five_g_status(status));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_ERROR("QtiRadioImpl:::on5gStatusChange indCb NULL");
    }
}

void QtiRadioImpl::onNrDcParamChange(five_g_endc_dcnr dcParam) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::onNrDcParamChange");
    if (mIndicationCbV2_0 != nullptr) {
        Return<void> ret = mIndicationCbV2_0->onNrDcParamChange(convert_five_g_endc_dcnr(dcParam));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_ERROR("QtiRadioImpl::onNrDcParamChange indCb NULL");
    }
}

void QtiRadioImpl::onNrBearerAllocationChange(five_g_bearer_status bearerStatus) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::onNrBearerAllocationChange");
    if (mIndicationCbV2_1 != nullptr) {
        Return<void> ret = mIndicationCbV2_1->onNrBearerAllocationChange_2_1(
            convert_five_g_bearer_status_2_1(bearerStatus));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    } else if (mIndicationCbV2_0 != nullptr) {
        Return<void> ret = mIndicationCbV2_0->onNrBearerAllocationChange(convert_five_g_bearer_status(bearerStatus));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_ERROR("QtiRadioImpl::onNrBearerAllocationChange indCb NULL");
    }
}

void QtiRadioImpl::onSignalStrengthChange(five_g_signal_strength signalStrength) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::onSignalStrengthChange");
    if (mIndicationCbV2_0 != nullptr) {
        Return<void> ret = mIndicationCbV2_0->onSignalStrengthChange(convert_five_g_signal_strength(signalStrength));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_ERROR("QtiRadioImpl::onSignalStrengthChange indCb NULL");
    }
}

void QtiRadioImpl::onUpperLayerIndInfoChange(five_g_upper_layer_ind_info upli_info) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::onUpperLayerIndInfoChange");
    if (mIndicationCbV2_1 != nullptr) {
        Return<void> ret = mIndicationCbV2_1->onUpperLayerIndInfoChange(
            convert_five_g_upper_layer_ind_info(upli_info));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_ERROR("QtiRadioImpl::onUpperLayerIndInfoChange indCb NULL");
    }
}

void QtiRadioImpl::on5gConfigInfoChange(five_g_config_type config) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::on5gConfigInfoChange");
    if (mIndicationCbV2_1 != nullptr) {
        Return<void> ret = mIndicationCbV2_1->on5gConfigInfoChange(
            convert_five_g_config_info(config));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_ERROR("QtiRadioImpl::on5gConfigInfoChange indCb NULL");
    }
}

void QtiRadioImpl::onNrIconTypeChange(five_g_icon_type iconType) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::onNrIconTypeChange");
    if (mIndicationCbV2_2 != nullptr) {
        Return<void> ret = mIndicationCbV2_2->onNrIconTypeChange(
            convert_five_g_icon_type(iconType));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_ERROR("QtiRadioImpl::onNrIconTypeChange indCb NULL");
    }
}


void QtiRadioImpl::getAtrResponse(int serial, RIL_Errno error, const char *buf, int bufLen) {
  sp<V1_0::IQtiRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    QtiRadioResponseInfo responseInfo;
    hidl_string data;
    QCRIL_LOG_ERROR("QtiRadioImpl::getAtrResponse serial=%d", serial);
    responseInfo.serial = serial;
    responseInfo.type = QtiRadioResponseType::SOLICITED;
    responseInfo.error = (QtiRadioError)error;
    if (buf != nullptr) {
      data.setToExternal(buf, bufLen);
    }
    Return<void> ret = respCb->getAtrResponse(responseInfo, data);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  } else {
    QCRIL_LOG_ERROR("QtiRadioImpl::getAtrResponse respCb NULL");
  }
}

Return<int32_t> QtiRadioImpl::getPropertyValueInt(const hidl_string& prop, int32_t def) {
    int prop_val = 0;
    if (!prop.empty()) {
        qmi_ril_get_property_value_from_integer(prop.c_str(), &prop_val, def);
    }
    QCRIL_LOG_DEBUG("Property: %s, Value: %d", prop.c_str(), prop_val);
    return prop_val;
}

Return<bool> QtiRadioImpl::getPropertyValueBool(const hidl_string& prop, bool def) {
    boolean prop_val = FALSE;
    if (!prop.empty()) {
        qmi_ril_get_property_value_from_boolean(prop.c_str(), &prop_val, (boolean) def);
    }
    QCRIL_LOG_DEBUG("Property: %s, Value: %b", prop.c_str(), prop_val);
    return (bool) prop_val;
}

#ifndef QMI_RIL_UTF
Return<void> QtiRadioImpl::getPropertyValueString(const hidl_string& prop, const hidl_string& def,
        V2_3::IQtiRadio::getPropertyValueString_cb _hidl_cb) {
    char prop_val[PROPERTY_VALUE_MAX] = {0};
    if (!prop.empty()) {
        qmi_ril_get_property_value_from_string(prop.c_str(), prop_val, def.c_str());
    }
    QCRIL_LOG_DEBUG("Property: %s, Value: %s", prop.c_str(), prop_val);
    _hidl_cb(prop_val);

    return Void();
}
#endif

Return<void> QtiRadioImpl::setCarrierInfoForImsiEncryption(int32_t serial,
        const V2_4::ImsiEncryptionInfo& data) {
    QCRIL_LOG_DEBUG ("QtiRadioImpl::setCarrierInfoForImsiEncryption, serial=%d", serial);

    rildata::ImsiEncryptionInfo_t imsiEncryption;
    imsiEncryption.mcc = data.base.mcc;
    imsiEncryption.mnc = data.base.mnc;
    imsiEncryption.carrierKey = data.base.carrierKey;
    imsiEncryption.keyIdentifier = data.base.keyIdentifier;
    imsiEncryption.expiryTime = data.base.expirationTime;
    imsiEncryption.keyType = (rildata::PublicKeyType_t)data.keyType;

    auto msg = std::make_shared<rildata::SetCarrierInfoImsiEncryptionMessage>(imsiEncryption);
    if (msg != nullptr) {
        GenericCallback<RIL_Errno> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<RIL_Errno> resp) -> void {
            (void)msg;
            (void)status;
            if(status != Message::Callback::Status::SUCCESS) {
                QCRIL_LOG_ERROR("Message::Callback::Status : %d", status);
                QtiRadioResponseInfo responseInfo = {QtiRadioResponseType::SOLICITED,
                        serial, QtiRadioError::GENERIC_FAILURE};
                sp<V2_4::IQtiRadioResponse> respCb = getResponseCallbackV2_4();
                if(respCb != nullptr) {
                    Return<void> ret = respCb->setCarrierInfoForImsiEncryptionResponse(
                            responseInfo);
                    if (!ret.isOk()) {
                        QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                                ret.description().c_str());
                    }
                }
            } else {
                QtiRadioError error;
                if (resp != nullptr) {
                    error = (QtiRadioError)(*resp);
                } else {
                    error = QtiRadioError::GENERIC_FAILURE;
                }
                QtiRadioResponseInfo responseInfo = {QtiRadioResponseType::SOLICITED, serial,
                        error};
                sp<V2_4::IQtiRadioResponse> respCb = getResponseCallbackV2_4();
                if(respCb != nullptr) {
                    Return<void> ret = respCb->setCarrierInfoForImsiEncryptionResponse(
                            responseInfo);
                    if (!ret.isOk()) {
                        QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                                ret.description().c_str());
                    }
                }

            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        QCRIL_LOG_ERROR("Unable to create msg SetCarrierInfoImsiEncryptionMessage");
        QtiRadioResponseInfo responseInfo = {QtiRadioResponseType::SOLICITED,
                serial, QtiRadioError::GENERIC_FAILURE};
        sp<V2_4::IQtiRadioResponse> respCb = getResponseCallbackV2_4();
        if(respCb != nullptr) {
            Return<void> ret = respCb->setCarrierInfoForImsiEncryptionResponse(responseInfo);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                        ret.description().c_str());
            }
        }
    }

    return Void();
}


void QtiRadioImpl::sendQtiIndication() {}

/*
 * Register qtiRadioService service with Service Manager
 */
void qcril_qmi_qti_radio_service_init(int instanceId) {
  android::status_t ret = android::OK;
  QCRIL_LOG_ERROR("qcril_qmi_qti_radio_service_init");
  qtiRadioService = new QtiRadioImpl((qcril_instance_id_e_type)instanceId);
  std::string serviceName = "slot";
  QCRIL_LOG_INFO("qcril_qmi_qti_radio_service_init adding for slot %d", instanceId);
  ret = qtiRadioService->registerAsService(serviceName + std::to_string(instanceId + 1));
  QCRIL_LOG_INFO("qtiRadioServiceRegisterService instanceId=%d status=%d", instanceId, ret);
}

V2_0::EnableStatus convert_five_g_status(five_g_status status) {
    switch (status) {
        case FIVE_G_STATUS_DISABLED:
            return V2_0::EnableStatus::DISABLED;
        case FIVE_G_STATUS_ENABLED:
            return V2_0::EnableStatus::ENABLED;
        default:
            return V2_0::EnableStatus::INVALID;
    }
}

V2_0::DcParam convert_five_g_endc_dcnr(five_g_endc_dcnr &endc_dcnr_info) {
    V2_0::DcParam dc_param;

    switch (endc_dcnr_info.endc_available) {
        case 0:
            dc_param.endc = V2_0::EndcAvailableType::ENDC_UNAVAILABLE;
            break;
        case 1:
            dc_param.endc = V2_0::EndcAvailableType::ENDC_AVAILABLE;
            break;
        default:
            dc_param.endc = V2_0::EndcAvailableType::INVALID;
            break;
    }

    switch (endc_dcnr_info.restrict_dcnr) {
        case 0:
            dc_param.dcnr = V2_0::RestrictDcnrType::DCNR_UNRESTRICTED;
            break;
        case 1:
            dc_param.dcnr = V2_0::RestrictDcnrType::DCNR_RESTRICTED;
            break;
        default:
            dc_param.dcnr = V2_0::RestrictDcnrType::INVALID;
            break;
    }

    return dc_param;
}

void initialize(V2_0::DcParam &dc_param) {
    dc_param.endc = V2_0::EndcAvailableType::INVALID;
    dc_param.dcnr = V2_0::RestrictDcnrType::INVALID;
}

V2_0::SignalStrength convert_five_g_signal_strength(
        five_g_signal_strength &signal_strength) {
    V2_0::SignalStrength ss;
    ss.rsrp = signal_strength.rsrp;
    ss.snr = signal_strength.snr;
    return ss;
}

void initialize(V2_0::SignalStrength &ss) {
    ss.rsrp = std::numeric_limits<int32_t>::min();
    ss.snr = std::numeric_limits<int32_t>::min();
}

V2_0::BearerStatus convert_five_g_bearer_status(five_g_bearer_status bearer_status) {
    switch (bearer_status) {
        case FIVE_G_BEARER_STATUS_NOT_ALLOCATED:
            return V2_0::BearerStatus::NOT_ALLOCATED;
        case FIVE_G_BEARER_STATUS_ALLOCATED:
        case FIVE_G_BEARER_STATUS_MMW_ALLOCATED:
            return V2_0::BearerStatus::ALLOCATED;
        default:
            return V2_0::BearerStatus::INVALID;
    }
}

V2_1::BearerStatus convert_five_g_bearer_status_2_1(five_g_bearer_status bearer_status) {
    switch (bearer_status) {
        case FIVE_G_BEARER_STATUS_NOT_ALLOCATED:
            return V2_1::BearerStatus::NOT_ALLOCATED;
        case FIVE_G_BEARER_STATUS_ALLOCATED:
            return V2_1::BearerStatus::ALLOCATED;
        case FIVE_G_BEARER_STATUS_MMW_ALLOCATED:
            return V2_1::BearerStatus::MMW_ALLOCATED;
        default:
            return V2_1::BearerStatus::INVALID;
    }
}

V2_1::PlmnInfoListStatus convert_five_g_plmn_list_info_status(five_g_plmn_info_list_status plmn_list_status) {
    switch (plmn_list_status) {
        case FIVE_G_PLMN_LIST_UNAVAILABLE:
            return V2_1::PlmnInfoListStatus::UNAVAILABLE;
        case FIVE_G_PLMN_LIST_AVAILABLE:
            return V2_1::PlmnInfoListStatus::AVAILABLE;
        default:
            return V2_1::PlmnInfoListStatus::INVALID;
    }
}

V2_1::UpperLayerIndStatus convert_five_g_upper_layer_ind_status(five_g_upper_layer_ind_status upli_status) {
    switch (upli_status) {
        case FIVE_G_UPPER_LAYER_IND_UNAVAILABLE:
            return V2_1::UpperLayerIndStatus::UNAVAILABLE;
        case FIVE_G_UPPER_LAYER_IND_AVAILABLE:
            return V2_1::UpperLayerIndStatus::AVAILABLE;
        default:
            return V2_1::UpperLayerIndStatus::INVALID;
    }
}

V2_1::UpperLayerIndInfo convert_five_g_upper_layer_ind_info(five_g_upper_layer_ind_info &five_g_upli_info) {
    V2_1::UpperLayerIndInfo upli_info;

    upli_info.plmnInfoList   = convert_five_g_plmn_list_info_status(five_g_upli_info.plmn_list_status);
    upli_info.upplerLayerInd = convert_five_g_upper_layer_ind_status(five_g_upli_info.upper_layer_ind_info_status);

    return upli_info;
}

void initialize(V2_1::UpperLayerIndInfo &upli_info) {
    upli_info.plmnInfoList = V2_1::PlmnInfoListStatus::INVALID;
    upli_info.upplerLayerInd = V2_1::UpperLayerIndStatus::INVALID;
}

V2_1::ConfigType convert_five_g_config_info(five_g_config_type config) {
    switch (config) {
        case FIVE_G_CONFIG_TYPE_NSA:
            return V2_1::ConfigType::NSA_CONFIGURATION;
        case FIVE_G_CONFIG_TYPE_SA:
            return V2_1::ConfigType::SA_CONFIGURATION;
        default:
            return V2_1::ConfigType::INVALID;
    }
}

V2_2::NrIconType convert_five_g_icon_type(five_g_icon_type iconType) {
    switch (iconType) {
        case FIVE_G_ICON_TYPE_NONE:
            return V2_2::NrIconType::TYPE_NONE;
        case FIVE_G_ICON_TYPE_BASIC:
            return V2_2::NrIconType::TYPE_5G_BASIC;
        case FIVE_G_ICON_TYPE_UWB:
            return V2_2::NrIconType::TYPE_5G_UWB;
        default:
            return V2_2::NrIconType::INVALID;
    }
}

void qtiGetAtrResponse(int token, RIL_Errno error, const char *buf, int bufLen) {
  if (qtiRadioService != nullptr) {
    qtiRadioService->getAtrResponse(token, error, buf, bufLen);
  }
}

void on5gStatusChange(five_g_status status) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->on5gStatusChange(status);
    }
}

void onNrDcParamChange(five_g_endc_dcnr dcParam) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->onNrDcParamChange(dcParam);
    }
}

void onNrBearerAllocationChange(five_g_bearer_status bearerStatus) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->onNrBearerAllocationChange(bearerStatus);
    }
}

void onSignalStrengthChange(five_g_signal_strength signalStrength) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->onSignalStrengthChange(signalStrength);
    }
}

void queryNrIconTypeResponse(RIL_Token token, RIL_Errno err_num, five_g_icon_type iconType) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->queryNrIconTypeResponse(token, err_num, iconType);
    }
}

void onUpperLayerIndInfoChange(five_g_upper_layer_ind_info upli_info) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->onUpperLayerIndInfoChange(upli_info);
    }
}

void on5gConfigInfoChange(five_g_config_type config) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->on5gConfigInfoChange(config);
    }
}

void onNrIconTypeChange(five_g_icon_type iconType) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->onNrIconTypeChange(iconType);
    }
}

RIL_Token qcril_qmi_qti_convert_radio_token_to_ril_token(uint32_t oem_token) {
  RIL_Token ret = qcril_malloc(sizeof(uint32_t));
  if (nullptr != ret) {
    uint32_t *tmp = (uint32_t *)ret;
    *tmp = oem_token ^ 0xc0000000;
  }
  return ret;
} // qcril_qmi_oem_convert_oem_token_to_ril_token

void populateResponseInfo(QtiRadioResponseInfo& responseInfo, int serial,
        QtiRadioResponseType responseType, RIL_Errno e) {
    responseInfo.serial = serial;
    responseInfo.type = responseType;
    responseInfo.error = (QtiRadioError) e;
}

uint32_t qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(RIL_Token ril_token) {
  uint32_t ret = 0xFFFFFFFF;
  if (ril_token) {
    ret = (*((uint32_t *)ril_token)) ^ 0xc0000000;
    QCRIL_LOG_INFO("oem token: %d", ret);
    qcril_free((void *)ril_token);
  } else {
    QCRIL_LOG_ERROR("ril_token is NULL");
  }

  return ret;
} // qcril_qmi_oem_free_and_convert_ril_token_to_oem_token

} // namespace implementation
} // namespace V1_0
} // namespace qtiradio
} // namespace radio
} // namespace hardware
} // namespace qti
} // namespace vendor
