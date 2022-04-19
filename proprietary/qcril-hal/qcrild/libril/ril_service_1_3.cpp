/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
* Not a Contribution.
*/
/*
 * Copyright (c) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "RILC"
#define TAG "RILQ_ril_1_3"

#include <memory>
#include <android/hardware/radio/1.3/IRadio.h>
#include <android/hardware/radio/1.3/IRadioResponse.h>
#include <android/hardware/radio/1.3/IRadioIndication.h>

#ifndef QMI_RIL_UTF
#include <hwbinder/IPCThreadState.h>
#include <hwbinder/ProcessState.h>
#endif
#include <ril_service_1_3.h>
#include "RilServiceModule_1_3.h"
#include <log/log.h>
#ifndef QMI_RIL_UTF
#include <hidl/HidlTransportSupport.h>
#endif
#include <utils/SystemClock.h>
#include <inttypes.h>
#include <framework/Log.h>
#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include <framework/UnSolicitedMessage.h>
#include <modules/nas/NasModule.h>

#include "interfaces/nas/nas_types.h"

#define INVALID_HEX_CHAR 16

using namespace android::hardware::radio;
using namespace android::hardware::radio::V1_0;
using namespace android::hardware::radio::deprecated::V1_0;
#ifndef QMI_RIL_UTF
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
#endif
using ::android::hardware::Return;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using android::CommandInfo;
using android::RequestInfo;
using android::requestToString;
using android::sp;
using RadioContext = RadioImpl_1_3::RadioContext;

extern bool dispatchStrings(int serial, int slotId, int request, bool allowEmpty, int countStrings, ...);

sp<RadioImpl_1_3> radioImpl_1_3;

void RadioImpl_1_3::clearCallbacks() {
    RLOGD("RadioImpl_1_3::clearCallbacks");
    mRadioResponseV1_3 = NULL;
    mRadioIndicationV1_3 = NULL;
    RadioImpl_1_2::clearCallbacks();
}

Return<void> RadioImpl_1_3::setResponseFunctions_nolock(
        const ::android::sp<IRadioResponse>& radioResponseParam,
        const ::android::sp<IRadioIndication>& radioIndicationParam) {
    RLOGD("RadioImpl_1_3::setResponseFunctions_nolock");

    mRadioResponseV1_3 = V1_3::IRadioResponse::castFrom(radioResponseParam).withDefault(nullptr);
    mRadioIndicationV1_3 = V1_3::IRadioIndication::castFrom(radioIndicationParam).withDefault(nullptr);

    if (mRadioResponseV1_3 == nullptr || mRadioIndicationV1_3 == nullptr) {
        mRadioResponseV1_3 = nullptr;
        mRadioIndicationV1_3 = nullptr;
    }

    QCRIL_LOG_DEBUG("radioResponseParamV1_3: %s. radioIndicationV1_3: %s",
            mRadioResponseV1_3 ? "<not null>" : "<null>",
            mRadioIndicationV1_3 ? "<not null>" : "<null>");

    RadioImpl_1_2::setResponseFunctions_nolock(
            radioResponseParam,
            radioIndicationParam);

    return Void();
}

Return<void> RadioImpl_1_3::setResponseFunctions(
        const ::android::sp<IRadioResponse>& radioResponseParam,
        const ::android::sp<IRadioIndication>& radioIndicationParam) {
    QCRIL_LOG_DEBUG("setResponseFunctions_1_3. radioResponseParam: %s. radioIndicationParam: %s",
            radioResponseParam ? "<not null>" : "<null>",
            radioIndicationParam ? "<not null>" : "<null>");
    RLOGD("setResponseFunctions_1_3");

    radioImpl_1_3 = this;

    qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radioServiceRwlockPtr->lock();

    RadioImpl_1_3::setResponseFunctions_nolock(
            radioResponseParam,
            radioIndicationParam);

    radioServiceRwlockPtr->unlock();

#ifndef QMI_RIL_UTF
    // client is connected. Send initial indications.
    android::onNewCommandConnect((RIL_SOCKET_ID) mSlotId);
#endif

    return Void();
}

Return<void> RadioImpl_1_3::enableModem(int32_t serial, bool on) {
    #if VDBG
    RLOGD("enableModem: serial %d on %d", serial, on);
    #endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestEnableModemMessage>(ctx, on);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_3, enableModemResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_3, enableModemResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl_1_3::getModemStackStatus(int32_t serial) {
    #if VDBG
    RLOGD("getModemStackStatus: serial %d on %d", serial, on);
    #endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetModemStackStatusMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            bool on = false;
            RadioResponseInfo respInfo = {};
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                auto rilModemStackResult =
                  std::static_pointer_cast<qcril::interfaces::RilGetModemStackResult_t>(resp->data);
                if(rilModemStackResult != nullptr) {
                    on = (bool)rilModemStackResult->state;
                }
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_3, getModemStackStatusResponse,
                respInfo, on);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        bool on = false;
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_3, getModemStackStatusResponse,
            respInfo, on); 
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

RadioError fillSetSystemSelectionChannelRequest(const ::android::hardware::hidl_vec<::android::hardware::radio::V1_1::RadioAccessSpecifier>& specifiers,
        RIL_SysSelChannels &ril_request) {
    if (specifiers.size() > MAX_RADIO_ACCESS_NETWORKS) {
        return RadioError::INVALID_ARGUMENTS;
    }
    for (size_t i = 0; i < specifiers.size(); ++i) {
        if (specifiers[i].geranBands.size() > MAX_BANDS ||
            specifiers[i].utranBands.size() > MAX_BANDS ||
            specifiers[i].eutranBands.size() > MAX_BANDS) {
            return RadioError::INVALID_ARGUMENTS;
        }
        ril_request.specifiers_length = specifiers.size();
        const V1_1::RadioAccessSpecifier& ras_from = specifiers[i];
        RIL_RadioAccessSpecifier& ras_to = ril_request.specifiers[i];
        ras_to.radio_access_network = (RIL_RadioAccessNetworks) ras_from.radioAccessNetwork;
        const std::vector<uint32_t> * bands = nullptr;
        switch (specifiers[i].radioAccessNetwork) {
            case V1_1::RadioAccessNetworks::GERAN:
                ras_to.bands_length = ras_from.geranBands.size();
                bands = (std::vector<uint32_t> *) &ras_from.geranBands;
                break;
            case V1_1::RadioAccessNetworks::UTRAN:
                ras_to.bands_length = ras_from.utranBands.size();
                bands = (std::vector<uint32_t> *) &ras_from.utranBands;
                break;
            case V1_1::RadioAccessNetworks::EUTRAN:
                ras_to.bands_length = ras_from.eutranBands.size();
                bands = (std::vector<uint32_t> *) &ras_from.eutranBands;
                break;
            default:
                return RadioError::INVALID_ARGUMENTS;
        }
        // safe to copy to geran_bands because it's a union member
        for (size_t idx = 0; idx < ras_to.bands_length; ++idx) {
            ras_to.bands.geran_bands[idx] = (RIL_GeranBands) (*bands)[idx];
        }
    }
    return RadioError::NONE;
}

Return<void> RadioImpl_1_3::setSystemSelectionChannels(int32_t serial, bool specifyChannels,
        const hidl_vec<V1_1::RadioAccessSpecifier>& specifiers) {
    #if VDBG
    RLOGD("setSystemSelectionChannels: serial %d, channel info valid - %d",
        serial, specifyChannels);
    #endif
    std::shared_ptr<RilRequestSetSysSelChannelsMessage> msg;
    RIL_SysSelChannels ril_info;
    memset(&ril_info, 0, sizeof(ril_info));

    auto res = RadioError::NONE;

    // If specifyChannels is true scan specific bands otherwise scan all bands
    // If specifier length is zero, RIL should scan all bands.
    if (specifyChannels == true) {
        res = fillSetSystemSelectionChannelRequest(specifiers, ril_info);
    }

    if (res == RadioError::NONE) {
        std::shared_ptr<RadioContext> ctx = getContext(serial);
        if ((msg = std::make_shared<RilRequestSetSysSelChannelsMessage>(ctx, ril_info)) == nullptr) {
            res = RadioError::NO_MEMORY;
        }
    }

    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_3, setSystemSelectionChannelsResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, res};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_3, setSystemSelectionChannelsResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

/***************************************************************************************************
 * RESPONSE FUNCTIONS
 * Functions above are used for requests going from framework to vendor code. The ones below are
 * responses for those requests coming back from the vendor code.
 **************************************************************************************************/

std::shared_ptr<RadioContext> RadioImpl_1_3::getContext(int32_t serial)
{
    std::shared_ptr<RadioContext> ctx;
    ctx = std::make_shared<RadioContext>(sp<RadioImpl_1_3>(this), serial);
    return ctx;
}

::android::status_t RadioImpl_1_3::registerAsService(const std::string &serviceName) {
    QCRIL_LOG_INFO("RadioImpl_1_3::registerAsService(%s)", serviceName.c_str());
    createRilServiceModule();
    return V1_3::IRadio::registerAsService(serviceName);
}

static void __attribute__((constructor)) registerRadioImpl_1_3();
void registerRadioImpl_1_3()
{
    QCRIL_LOG_INFO("Calling registerRadioImpl_1_3");
    getQcrildServiceFactory().registerRadioImpl<RadioImpl_1_3>(nullptr);
}

Module* RadioImpl_1_3::getRilServiceModule() {
    return mModule;
}

void RadioImpl_1_3::createRilServiceModule() {
    mModule = new RilServiceModule_1_3(getInstanceId());
    if (mModule != nullptr) {
        mModule->init();
    }
}

const QcrildServiceVersion &RadioImpl_1_3::getVersion() {
    static QcrildServiceVersion version(1, 3);
    return version;
}

RadioImpl_1_3::RadioImpl_1_3(qcril_instance_id_e_type instance): RadioImpl_1_2(instance) {
    QCRIL_LOG_INFO("RadioImpl_1_3()");
}
