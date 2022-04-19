/*===========================================================================
 *
 *    Copyright (c) 2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#include "qcril_qmi_qti_radio_service.h"
#include "ril_utf_hidl_services.h"
#include "ril_utf_ril_api.h"
#include "platform/common/CommonPlatform.h"

#include "platform/android/IQtiRadioResponseClientImpl.h"
#include "platform/android/IQtiRadioIndicationClientImpl.h"

using namespace vendor::qti::hardware::radio::qtiradio::V1_0::implementation;


class AndroidQtiRadio : public CommonPlatform {
public:
    AndroidQtiRadio() {
        utfQtiRadio = nullptr;
        bFirstCall = true;
    }
    void Register(RIL_RadioFunctions *callbacks) override;
    int OnRequest(int request, void *data, size_t datalen,
        RIL_Token t) override;
private:
    ::android::sp<vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio> utfQtiRadio;
    bool bFirstCall;
};

void AndroidQtiRadio::Register(RIL_RadioFunctions *callbacks) {
    if (!bFirstCall) return;
    qcril_qmi_qti_radio_service_init(0);
    utfQtiRadio = vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio::getService("slot1");
    if (utfQtiRadio != nullptr) {
        RIL_UTF_DEBUG("type of utfQtiRadio is %s", typeid(*utfQtiRadio).name());
        // create the HIDL clients
        sp<IQtiRadioResponseClientImpl> respClient = new IQtiRadioResponseClientImpl();
        sp<IQtiRadioIndicationClientImpl> indClient = new IQtiRadioIndicationClientImpl();
        utfQtiRadio->setCallback(respClient, indClient);
    }
    bFirstCall = false;
}

int AndroidQtiRadio::OnRequest(int request, void *data, size_t datalen,
        RIL_Token t) {
    switch (request) {
    case RIL_REQUEST_ENABLE_5G:
        if (utfQtiRadio != nullptr) {
            utfQtiRadio->enable5g(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    default:
        return -1;
    }
}

static void __attribute__((constructor)) registerPlatform() {
    static AndroidQtiRadio qtiRadio;
    setPlatform(&qtiRadio);
}
