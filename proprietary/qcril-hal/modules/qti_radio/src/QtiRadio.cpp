/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "qcril_qmi_qti_radio_service.h"
#include <cstring>
#include <framework/QcrilInitMessage.h>
#include "QtiRadio.h"
#include <framework/Log.h>

using namespace vendor::qti::hardware::radio::qtiradio::V2_1;

static load_module<QtiRadio> sQtiRadio;

QtiRadio *getQtiRadio() {
    return &(sQtiRadio.get_module());
}

/*
 * 1. Indicate your preference for looper.
 * 2. Subscribe to the list of messages via mMessageHandler.
 * 3. Follow RAII practice.
 */
QtiRadio::QtiRadio() {
  mName = "QtiRadio";

  using std::placeholders::_1;
  mMessageHandler = {
 #ifndef QMI_RIL_UTF // avoid race condition in UTF test case.
    HANDLER(QcrilInitMessage, QtiRadio::handleQcrilInit),
 #endif
    HANDLER(Nas5gConnectionIndMessage, QtiRadio::handleNas5gConnectionIndMessage),
    HANDLER(Nas5gStatusIndMessage, QtiRadio::handleNas5gStatusIndMessage),
    HANDLER(NasEndcDcnrIndMessage, QtiRadio::handleNasEndcDcnrIndMessage),
    HANDLER(Nas5gSignalStrengthIndMessage, QtiRadio::handleNas5gSignalStrengthIndMessage),
    HANDLER(Nas5gConfigInfoIndMessage, QtiRadio::handleNas5gConfigInfoIndMessage),
    HANDLER(NasUpperLayerIndInfoIndMessage, QtiRadio::handleNasUpperLayerIndInfoIndMessage),
#ifndef QMI_RIL_UTF
    HANDLER(rildata::DataNrIconTypeIndMessage, QtiRadio::handleDataNrIconTypeIndMessage),
#endif
  };
}

/* Follow RAII.
*/
QtiRadio::~QtiRadio() {}

/*
 * Module specific initialization that does not belong to RAII .
 */
void QtiRadio::init() {
  Module::init();
}

/*
 * List of individual private handlers for the subscribed messages.
 */
void QtiRadio::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  Log::getInstance().d("[" + mName + "]: get_instance_id = "
        + std::to_string(msg->get_instance_id()));

  implementation::qcril_qmi_qti_radio_service_init (msg->get_instance_id ());
}

void QtiRadio::handleNas5gConnectionIndMessage(
  std::shared_ptr<Nas5gConnectionIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  using namespace vendor::qti::hardware::radio::qtiradio::V2_1;
  using namespace vendor::qti::hardware::radio::qtiradio::V1_0::implementation;
  onNrBearerAllocationChange(msg->is5gBearerStatus());
}

void QtiRadio::handleNas5gStatusIndMessage(
        std::shared_ptr<Nas5gStatusIndMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    using namespace vendor::qti::hardware::radio::qtiradio::V2_1;
    using namespace vendor::qti::hardware::radio::qtiradio::V1_0::implementation;
    on5gStatusChange((five_g_status) msg->is5gEnabled());
}

void QtiRadio::handleNasEndcDcnrIndMessage(
        std::shared_ptr<NasEndcDcnrIndMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    using namespace vendor::qti::hardware::radio::qtiradio::V2_1;
    using namespace vendor::qti::hardware::radio::qtiradio::V1_0::implementation;
    five_g_endc_dcnr endc_dcnr_info;
    endc_dcnr_info.endc_available = (int32_t) msg->isEndcAvailable();
    endc_dcnr_info.restrict_dcnr = (int32_t) msg->isDcnrRestricted();
    onNrDcParamChange(endc_dcnr_info);
}

void QtiRadio::handleNas5gSignalStrengthIndMessage(
        std::shared_ptr<Nas5gSignalStrengthIndMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    using namespace vendor::qti::hardware::radio::qtiradio::V2_1;
    using namespace vendor::qti::hardware::radio::qtiradio::V1_0::implementation;
    five_g_signal_strength signal_strength;
    signal_strength.rsrp = msg->getRsrp();
    signal_strength.snr = msg->getSnr();
    onSignalStrengthChange(signal_strength);
}

void QtiRadio::handleNas5gConfigInfoIndMessage(
        std::shared_ptr<Nas5gConfigInfoIndMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    using namespace vendor::qti::hardware::radio::qtiradio::V2_1;
    using namespace vendor::qti::hardware::radio::qtiradio::V1_0::implementation;

    if (msg->is5gSA()) {
        on5gConfigInfoChange(FIVE_G_CONFIG_TYPE_SA);
    } else {
        on5gConfigInfoChange(FIVE_G_CONFIG_TYPE_NSA);
    }
}

void QtiRadio::handleNasUpperLayerIndInfoIndMessage(
        std::shared_ptr<NasUpperLayerIndInfoIndMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    using namespace vendor::qti::hardware::radio::qtiradio::V2_1;
    using namespace vendor::qti::hardware::radio::qtiradio::V1_0::implementation;

    five_g_upper_layer_ind_info upli_info;
    upli_info.upper_layer_ind_info_status = (five_g_upper_layer_ind_status)
                                                msg->isUpperLayerIndInfoAvailable();
    upli_info.plmn_list_status = (five_g_plmn_info_list_status)
                                                msg->isPlmnsListInfoAvailable();
    onUpperLayerIndInfoChange(upli_info);
}

#ifndef QMI_RIL_UTF
void QtiRadio::handleDataNrIconTypeIndMessage(
        std::shared_ptr<rildata::DataNrIconTypeIndMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    using namespace vendor::qti::hardware::radio::qtiradio::V1_0::implementation;
    five_g_icon_type rilIconType;

    if(msg->isNone()){
        rilIconType = FIVE_G_ICON_TYPE_NONE;
    } else if(msg->isBasic()) {
        rilIconType = FIVE_G_ICON_TYPE_BASIC;
    } else if(msg->isUwb()) {
        rilIconType = FIVE_G_ICON_TYPE_UWB;
    } else {
        rilIconType = FIVE_G_ICON_TYPE_INVALID;
    }

    onNrIconTypeChange(rilIconType);
}
#endif
