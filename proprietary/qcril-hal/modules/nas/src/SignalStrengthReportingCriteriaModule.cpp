/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#define TAG "RILQ_SSRepCritMod"
#include <functional>
#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include <framework/Dispatcher.h>
#include <modules/qmi/QmiIndMessage.h>
#include <modules/qmi/QmiAsyncResponseMessage.h>
#include <modules/nas/NasSetSignalStrengthCriteria.h>
#include <modules/nas/qcril_qmi_nas.h>
#include <modules/qmi/NasModemEndPoint.h>
#include <modules/qmi/ModemEndPointFactory.h>
#include <modules/qmi/EndpointStatusIndMessage.h>
#include <network_access_service_v01.h>
#include <modules/nas/qcril_nas_legacy.h>
#include <qcril_qmi_client.h>

using std::placeholders::_1;

class SignalStrengthReportingCriteriaModule : public Module {
  public:
    SignalStrengthReportingCriteriaModule();
    ~SignalStrengthReportingCriteriaModule() {}
    void init();

  private:
    bool mReady = false;
    qtimutex::QtiRecursiveMutex mMutex;
    nas_config_sig_info2_req_msg_v01 req = {
        .nr5g_rsrp_delta_valid = 1,
        .nr5g_rsrp_delta = QMI_RIL_NAS_RSRP_DELTA,
        .nr5g_snr_delta_valid = 1,
        .nr5g_snr_delta = QMI_RIL_NAS_SNR_DELTA,
        .nr5g_sig_rpt_config_valid = 1,
        .nr5g_sig_rpt_config = {
            .rpt_rate = NR5G_COMMON_RSSI_REPORTING_RATE_3_SEC_V01,
            .avg_period = NR5G_COMMON_RSSI_AVERAGING_PERIOD_5_SEC_V01
        },
    };

    void handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg);
    void handleNasQmiIndMessage(std::shared_ptr<QmiIndMessage> msg);
    void handleQmiAsyncRespMessage(std::shared_ptr<QmiAsyncResponseMessage> msg);
    void handleNasEndpointStatusIndMessage(std::shared_ptr<EndpointStatusIndMessage> msg);
    void handleNasSetSignalStrengthCriteria(std::shared_ptr<NasSetSignalStrengthCriteria> msg);
    void cleanUp() {
        memset(&req, 0, sizeof(req));
    }

};

static load_module<SignalStrengthReportingCriteriaModule> the_module;

SignalStrengthReportingCriteriaModule &getSignalStrengthReportingCriteriaModule() {
    return the_module.get_module();
}

SignalStrengthReportingCriteriaModule::SignalStrengthReportingCriteriaModule() {
    mName = "SignalStrengthReportingCriteriaModule";
    ModemEndPointFactory<NasModemEndPoint>::getInstance().buildEndPoint();

    mMessageHandler = {
        HANDLER(QcrilInitMessage, SignalStrengthReportingCriteriaModule::handleQcrilInit),
        HANDLER_MULTI(QmiIndMessage, "NAS_QMI_IND", SignalStrengthReportingCriteriaModule::handleNasQmiIndMessage),
        // End Point Status Indication
        HANDLER_MULTI(EndpointStatusIndMessage, "NAS_ENDPOINT_STATUS_IND", SignalStrengthReportingCriteriaModule::handleNasEndpointStatusIndMessage),
        // Qmi Async Response
        HANDLER(QmiAsyncResponseMessage, SignalStrengthReportingCriteriaModule::handleQmiAsyncRespMessage),
        HANDLER(NasSetSignalStrengthCriteria, SignalStrengthReportingCriteriaModule::handleNasSetSignalStrengthCriteria),

    };
}

void SignalStrengthReportingCriteriaModule::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
    QCRIL_LOG_DEBUG("[%s]: Handling msg = %s", mName.c_str(), msg->dump().c_str());
    /* Init QMI services.*/
    GenericCallback<string> gcb([](std::shared_ptr<Message> msg, Message::Callback::Status status, std::shared_ptr<void> payload) -> void {
        (void)msg;
        (void)status;
        (void)payload;
        QCRIL_LOG_DEBUG("");
    });
    ModemEndPointFactory<NasModemEndPoint>::getInstance().buildEndPoint()->requestSetup("", &gcb);
}

void SignalStrengthReportingCriteriaModule::init()
{
    QCRIL_LOG_DEBUG("init");
    Module::init();
}

void SignalStrengthReportingCriteriaModule::handleNasQmiIndMessage(std::shared_ptr<QmiIndMessage> msg)
{
    QCRIL_LOG_DEBUG("Handling msg = %s", msg->dump().c_str() );
}

void SignalStrengthReportingCriteriaModule::handleQmiAsyncRespMessage(std::shared_ptr<QmiAsyncResponseMessage> msg)
{
    QCRIL_LOG_DEBUG("Handling msg = %s", msg->dump().c_str() );
}

void SignalStrengthReportingCriteriaModule::handleNasEndpointStatusIndMessage(std::shared_ptr<EndpointStatusIndMessage> msg)
{
    QCRIL_LOG_DEBUG("Handling msg = %s", msg->dump().c_str() );
    if (msg->getState() == ModemEndPoint::State::OPERATIONAL) {
        mReady = true;
    } else {
        mReady = false;
    }
}

#define MAX_THRESHOLDS 32
void fillGeran(nas_config_sig_info2_req_msg_v01 &config, const SignalStrengthCriteriaEntry &entry) {
    unsigned sz = entry.thresholds.size();

    if (sz > 0) {
        // Can't have deltas and thresholds at the same time
        config.gsm_rssi_delta_valid = FALSE;
        if (sz >= MAX_THRESHOLDS) sz = MAX_THRESHOLDS;
        config.gsm_rssi_threshold_list_valid = TRUE;
        QCRIL_LOG_DEBUG("Setting threshold_list_len to %u", sz);
        config.gsm_rssi_threshold_list_len = sz;
        for (unsigned i = 0; i < sz; i++) {
            config.gsm_rssi_threshold_list[i] = entry.thresholds[i] * 10;
        }
    } else if (entry.delta != 0) {
        config.gsm_rssi_threshold_list_valid = FALSE;
        config.gsm_rssi_delta_valid = TRUE;
        config.gsm_rssi_delta = entry.delta * 10;
    } else {
        config.gsm_rssi_threshold_list_valid = FALSE;
        config.gsm_rssi_delta_valid = TRUE;
        config.gsm_rssi_delta = NasSetSignalStrengthCriteria::RSSI_DELTA;
    }
}

void fillUtran(nas_config_sig_info2_req_msg_v01 &config, const SignalStrengthCriteriaEntry &entry) {
    unsigned sz = entry.thresholds.size();

    if (sz > 0) {
        // Can't have deltas and thresholds at the same time
        config.wcdma_rssi_delta_valid = FALSE;
        if (sz >= MAX_THRESHOLDS) sz = MAX_THRESHOLDS;
        config.wcdma_rssi_threshold_list_valid = TRUE;
        QCRIL_LOG_DEBUG("Setting threshold_list_len to %u", sz);
        config.wcdma_rssi_threshold_list_len = sz;
        for (unsigned i = 0; i < sz; i++) {
            config.wcdma_rssi_threshold_list[i] = entry.thresholds[i] * 10;
        }
    } else if (entry.delta != 0) {
        config.wcdma_rssi_threshold_list_valid = FALSE;
        config.wcdma_rssi_delta_valid = TRUE;
        config.wcdma_rssi_delta = entry.delta * 10;
    } else {
        config.wcdma_rssi_threshold_list_valid = FALSE;
        config.wcdma_rssi_delta_valid = TRUE;
        config.wcdma_rssi_delta = NasSetSignalStrengthCriteria::RSSI_DELTA;
    }
}

void fillEutran(nas_config_sig_info2_req_msg_v01 &config, const SignalStrengthCriteriaEntry &entry) {
    unsigned sz = entry.thresholds.size();

    if (sz > 0) {
        // Can't have deltas and thresholds at the same time
        config.lte_rsrp_delta_valid = FALSE;
        if (sz >= MAX_THRESHOLDS) sz = MAX_THRESHOLDS;
        config.lte_rsrp_threshold_list_valid = TRUE;
        QCRIL_LOG_DEBUG("Setting threshold_list_len to %u", sz);
        config.lte_rsrp_threshold_list_len = sz;
        for (unsigned i = 0; i < sz; i++) {
            config.lte_rsrp_threshold_list[i] = entry.thresholds[i] * 10;
        }
    } else if (entry.delta != 0) {
        config.lte_rsrp_threshold_list_valid = FALSE;
        config.lte_rsrp_delta_valid = TRUE;
        config.lte_rsrp_delta = entry.delta * 10;
    } else {
        config.lte_rsrp_threshold_list_valid = FALSE;
        config.lte_rssi_delta_valid = TRUE;
        config.lte_rssi_delta = NasSetSignalStrengthCriteria::RSSI_DELTA;
    }
}

void fillCdma(nas_config_sig_info2_req_msg_v01 &config, const SignalStrengthCriteriaEntry &entry) {
    unsigned sz = entry.thresholds.size();

    if (sz > 0) {
        // Can't have deltas and thresholds at the same time
        config.cdma_rssi_delta_valid = FALSE;
        if (sz >= MAX_THRESHOLDS) sz = MAX_THRESHOLDS;
        config.cdma_rssi_threshold_list_valid = TRUE;
        QCRIL_LOG_DEBUG("Setting threshold_list_len to %u", sz);
        config.cdma_rssi_threshold_list_len = sz;
        for (unsigned i = 0; i < sz; i++) {
            config.cdma_rssi_threshold_list[i] = entry.thresholds[i] * 10;
        }
    } else if (entry.delta != 0) {
        config.cdma_rssi_threshold_list_valid = FALSE;
        config.cdma_rssi_delta_valid = TRUE;
        config.cdma_rssi_delta = entry.delta * 10;
    } else {
        config.cdma_rssi_threshold_list_valid = FALSE;
        config.cdma_rssi_delta_valid = TRUE;
        config.cdma_rssi_delta = NasSetSignalStrengthCriteria::RSSI_DELTA;
    }
}

void updateCriteria(nas_config_sig_info2_req_msg_v01 &out, const std::vector<SignalStrengthCriteriaEntry> &criteria) {
    QCRIL_LOG_FUNC_ENTRY();

    for (SignalStrengthCriteriaEntry entry: criteria) {
        QCRIL_LOG_DEBUG("Found entry criteria");
        switch(entry.ran) {
            case GERAN:
                QCRIL_LOG_DEBUG("RAN: GERAN");
                fillGeran(out, entry);
                break;
            case UTRAN:
                QCRIL_LOG_DEBUG("RAN: UTRAN");
                fillUtran(out, entry);
                break;
            case EUTRAN:
                QCRIL_LOG_DEBUG("RAN: EUTRAN");
                fillEutran(out, entry);
                break;
            case CDMA:
                QCRIL_LOG_DEBUG("RAN: CDMA");
                fillCdma(out, entry);
                break;
            case IWLAN:
                QCRIL_LOG_DEBUG("RAN: IWLAN Unsupported");
                break;
            default:
                QCRIL_LOG_ERROR("RAN: Unknown value %d", entry.ran);
                break;
        }
    }
    QCRIL_LOG_FUNC_RETURN();
}

void SignalStrengthReportingCriteriaModule::handleNasSetSignalStrengthCriteria(std::shared_ptr<NasSetSignalStrengthCriteria> msg)
{
    QCRIL_LOG_DEBUG("Handling msg = NasSetSignalStrengthCriteria");
    if (msg) {
        QCRIL_LOG_DEBUG("Contents: %s", msg->dump().c_str());
        std::shared_ptr<RIL_Errno> rsp = nullptr;
        Message::Callback::Status status = Message::Callback::Status::FAILURE;

        if (mReady) {
            QCRIL_LOG_DEBUG("Making shared_ptr");
            rsp = std::make_shared<RIL_Errno>(RIL_E_SUCCESS);
            QCRIL_LOG_DEBUG("Calling updateCriteria");
            updateCriteria(req, msg->getCriteria());
            QCRIL_LOG_DEBUG("Returned from updateCriteria");
            qmi_client_error_type err = qmi_client_nas_send_async(QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01,
                    &req,
                    sizeof(req),
                    sizeof(nas_config_sig_info2_resp_msg_v01),
                    [msg] (unsigned int, std::shared_ptr<void> qmi_response, unsigned int, void *, qmi_client_error_type qmi_client_error) -> void {
                        QCRIL_LOG_DEBUG("Received response from QMI");
                        RIL_Errno ril_req_res =
                            qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                                    qmi_client_error,
                                    &(std::static_pointer_cast<nas_config_sig_info2_resp_msg_v01>(qmi_response)->resp) );
                        std::shared_ptr<RIL_Errno> rsp = std::make_shared<RIL_Errno>(ril_req_res);
                        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, rsp);
                    },
                    NULL);
            if (err) {
                QCRIL_LOG_DEBUG("Received QMI error: %d", err);
                rsp = std::make_shared<RIL_Errno>(RIL_E_SYSTEM_ERR);
                msg->sendResponse(msg, Message::Callback::Status::SUCCESS, rsp);
            }
        } else {
            QCRIL_LOG_DEBUG("Responding not ready");
            rsp = std::make_shared<RIL_Errno>(RIL_E_RADIO_NOT_AVAILABLE);
            msg->sendResponse(msg, status, rsp);
        }

    }
}

