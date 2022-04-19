/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "modules/qmi/NasModemEndPoint.h"
#include "modules/qmi/QmiServiceUpIndMessage.h"
#include "modules/qmi/QmiSetupRequest.h"

using std::to_string;
constexpr const char *NasModemEndPoint::NAME;

std::string toString(nas_radio_if_enum_v01 radio_if) {
    switch (radio_if) {
        case NAS_RADIO_IF_NO_SVC_V01: return "NO_SVC";
        case NAS_RADIO_IF_CDMA_1X_V01: return "CDMA_1X";
        case NAS_RADIO_IF_CDMA_1XEVDO_V01: return "CDMA_1XEVDO";
        case NAS_RADIO_IF_AMPS_V01: return "AMPS";
        case NAS_RADIO_IF_GSM_V01: return "GSM";
        case NAS_RADIO_IF_UMTS_V01: return "UMTS";
        case NAS_RADIO_IF_WLAN_V01: return "WLAN";
        case NAS_RADIO_IF_GPS_V01: return "GPS";
        case NAS_RADIO_IF_LTE_V01: return "LTE";
        case NAS_RADIO_IF_TDSCDMA_V01: return "TDSCDMA";
        case NAS_RADIO_IF_NR5G_V01: return "NR5G";
        default: return "<INVALID>";
    }
}
std::string toString(nas_active_band_enum_v01 active_band) {
    switch(active_band) {
        case NAS_ACTIVE_BAND_BC_0_V01: return "BC_0";
        case NAS_ACTIVE_BAND_BC_1_V01: return "BC_1";
        case NAS_ACTIVE_BAND_BC_3_V01: return "BC_3";
        case NAS_ACTIVE_BAND_BC_4_V01: return "BC_4";
        case NAS_ACTIVE_BAND_BC_5_V01: return "BC_5";
        case NAS_ACTIVE_BAND_BC_6_V01: return "BC_6";
        case NAS_ACTIVE_BAND_BC_7_V01: return "BC_7";
        case NAS_ACTIVE_BAND_BC_8_V01: return "BC_8";
        case NAS_ACTIVE_BAND_BC_9_V01: return "BC_9";
        case NAS_ACTIVE_BAND_BC_10_V01: return "BC_10";
        case NAS_ACTIVE_BAND_BC_11_V01: return "BC_11";
        case NAS_ACTIVE_BAND_BC_12_V01: return "BC_12";
        case NAS_ACTIVE_BAND_BC_13_V01: return "BC_13";
        case NAS_ACTIVE_BAND_BC_14_V01: return "BC_14";
        case NAS_ACTIVE_BAND_BC_15_V01: return "BC_15";
        case NAS_ACTIVE_BAND_BC_16_V01: return "BC_16";
        case NAS_ACTIVE_BAND_BC_17_V01: return "BC_17";
        case NAS_ACTIVE_BAND_BC_18_V01: return "BC_18";
        case NAS_ACTIVE_BAND_BC_19_V01: return "BC_19";
        case NAS_ACTIVE_BAND_GSM_450_V01: return "GSM_450";
        case NAS_ACTIVE_BAND_GSM_480_V01: return "GSM_480";
        case NAS_ACTIVE_BAND_GSM_750_V01: return "GSM_750";
        case NAS_ACTIVE_BAND_GSM_850_V01: return "GSM_850";
        case NAS_ACTIVE_BAND_GSM_900_EXTENDED_V01: return "GSM_900_EXTENDED";
        case NAS_ACTIVE_BAND_GSM_900_PRIMARY_V01: return "GSM_900_PRIMARY";
        case NAS_ACTIVE_BAND_GSM_900_RAILWAYS_V01: return "GSM_900_RAILWAYS";
        case NAS_ACTIVE_BAND_GSM_1800_V01: return "GSM_1800";
        case NAS_ACTIVE_BAND_GSM_1900_V01: return "GSM_1900";
        case NAS_ACTIVE_BAND_WCDMA_2100_V01: return "WCDMA_2100";
        case NAS_ACTIVE_BAND_WCDMA_PCS_1900_V01: return "WCDMA_PCS_1900";
        case NAS_ACTIVE_BAND_WCDMA_DCS_1800_V01: return "WCDMA_DCS_1800";
        case NAS_ACTIVE_BAND_WCDMA_1700_US_V01: return "WCDMA_1700_US";
        case NAS_ACTIVE_BAND_WCDMA_850_V01: return "WCDMA_850";
        case NAS_ACTIVE_BAND_WCDMA_800_V01: return "WCDMA_800";
        case NAS_ACTIVE_BAND_WCDMA_2600_V01: return "WCDMA_2600";
        case NAS_ACTIVE_BAND_WCDMA_900_V01: return "WCDMA_900";
        case NAS_ACTIVE_BAND_WCDMA_1700_JAPAN_V01: return "WCDMA_1700_JAPAN";
        case NAS_ACTIVE_BAND_WCDMA_1500_JAPAN_V01: return "WCDMA_1500_JAPAN";
        case NAS_ACTIVE_BAND_WCDMA_850_JAPAN_V01: return "WCDMA_850_JAPAN";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_1_V01: return "E_UTRA_OPERATING_BAND_1";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_2_V01: return "E_UTRA_OPERATING_BAND_2";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_3_V01: return "E_UTRA_OPERATING_BAND_3";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_4_V01: return "E_UTRA_OPERATING_BAND_4";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_5_V01: return "E_UTRA_OPERATING_BAND_5";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_6_V01: return "E_UTRA_OPERATING_BAND_6";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_7_V01: return "E_UTRA_OPERATING_BAND_7";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_8_V01: return "E_UTRA_OPERATING_BAND_8";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_9_V01: return "E_UTRA_OPERATING_BAND_9";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_10_V01: return "E_UTRA_OPERATING_BAND_10";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_11_V01: return "E_UTRA_OPERATING_BAND_11";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_12_V01: return "E_UTRA_OPERATING_BAND_12";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_13_V01: return "E_UTRA_OPERATING_BAND_13";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_14_V01: return "E_UTRA_OPERATING_BAND_14";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_17_V01: return "E_UTRA_OPERATING_BAND_17";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_33_V01: return "E_UTRA_OPERATING_BAND_33";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_34_V01: return "E_UTRA_OPERATING_BAND_34";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_35_V01: return "E_UTRA_OPERATING_BAND_35";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_36_V01: return "E_UTRA_OPERATING_BAND_36";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_37_V01: return "E_UTRA_OPERATING_BAND_37";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_38_V01: return "E_UTRA_OPERATING_BAND_38";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_39_V01: return "E_UTRA_OPERATING_BAND_39";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_40_V01: return "E_UTRA_OPERATING_BAND_40";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_18_V01: return "E_UTRA_OPERATING_BAND_18";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_19_V01: return "E_UTRA_OPERATING_BAND_19";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_20_V01: return "E_UTRA_OPERATING_BAND_20";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_21_V01: return "E_UTRA_OPERATING_BAND_21";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_24_V01: return "E_UTRA_OPERATING_BAND_24";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_25_V01: return "E_UTRA_OPERATING_BAND_25";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_41_V01: return "E_UTRA_OPERATING_BAND_41";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_42_V01: return "E_UTRA_OPERATING_BAND_42";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_43_V01: return "E_UTRA_OPERATING_BAND_43";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_23_V01: return "E_UTRA_OPERATING_BAND_23";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_26_V01: return "E_UTRA_OPERATING_BAND_26";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_32_V01: return "E_UTRA_OPERATING_BAND_32";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_125_V01: return "E_UTRA_OPERATING_BAND_125";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_126_V01: return "E_UTRA_OPERATING_BAND_126";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_127_V01: return "E_UTRA_OPERATING_BAND_127";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_28_V01: return "E_UTRA_OPERATING_BAND_28";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_29_V01: return "E_UTRA_OPERATING_BAND_29";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_30_V01: return "E_UTRA_OPERATING_BAND_30";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_66_V01: return "E_UTRA_OPERATING_BAND_66";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_250_V01: return "E_UTRA_OPERATING_BAND_250";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_46_V01: return "E_UTRA_OPERATING_BAND_46";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_27_V01: return "E_UTRA_OPERATING_BAND_27";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_31_V01: return "E_UTRA_OPERATING_BAND_31";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_71_V01: return "E_UTRA_OPERATING_BAND_71";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_47_V01: return "E_UTRA_OPERATING_BAND_47";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_48_V01: return "E_UTRA_OPERATING_BAND_48";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_67_V01: return "E_UTRA_OPERATING_BAND_67";
        case NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_68_V01: return "E_UTRA_OPERATING_BAND_68";
        case NAS_ACTIVE_BAND_TDSCDMA_BAND_A_V01: return "TDSCDMA_BAND_A";
        case NAS_ACTIVE_BAND_TDSCDMA_BAND_B_V01: return "TDSCDMA_BAND_B";
        case NAS_ACTIVE_BAND_TDSCDMA_BAND_C_V01: return "TDSCDMA_BAND_C";
        case NAS_ACTIVE_BAND_TDSCDMA_BAND_D_V01: return "TDSCDMA_BAND_D";
        case NAS_ACTIVE_BAND_TDSCDMA_BAND_E_V01: return "TDSCDMA_BAND_E";
        case NAS_ACTIVE_BAND_TDSCDMA_BAND_F_V01: return "TDSCDMA_BAND_F";
        case NAS_ACTIVE_BAND_NR5G_BAND_1_V01: return "NR5G_BAND_1";
        case NAS_ACTIVE_BAND_NR5G_BAND_2_V01: return "NR5G_BAND_2";
        default: return "<INVALID>";
    }
}

void NasModemEndPoint::requestSetup(string clientToken,
    GenericCallback<string>* callback) {
  Log::getInstance().d("[NasModemEndPoint]: requestSetup");
  auto setupMsg = std::make_shared<QmiSetupRequest>
    (clientToken, 0, nullptr, callback);
  mModule->dispatch(setupMsg);
}

void NasModemEndPoint::registerForIndications(QmiNasRegisterForIndRequest::IndicationList list,
        std::shared_ptr<MessageContext> ctx, QmiNasRegisterForIndRequest::cb_t cb)
{
    auto msg = std::make_shared<QmiNasRegisterForIndRequest>(list, cb);
    if (msg) {
        msg->setContext(ctx);
        mModule->dispatch(msg);
    }
}

void NasModemEndPoint::registerForRfBandInfoInd(bool enable,
        std::shared_ptr<MessageContext> ctx, QmiNasRegisterForIndRequest::cb_t cb)
{
    typename QmiNasRegisterForIndRequest::IndicationList list {{QMI_NAS_RF_BAND_INFO_IND_V01,enable}};
    auto msg = std::make_shared<QmiNasRegisterForIndRequest>(list, cb);
    if (msg) {
        msg->setContext(ctx);
        mModule->dispatch(msg);
    }
}

void NasModemEndPoint::getLteCarrierAggregationEventInfo(PhysChanInfo &phyChanInfo) {
    nas_get_lte_cphy_ca_info_req_msg_v01 request = {};
    nas_get_lte_cphy_ca_info_resp_msg_v01 response = {};

    qmi_client_error_type qmiError = sendRawSync(QMI_NAS_GET_LTE_CPHY_CA_INFO_REQ_MSG_V01,
            &request, sizeof(request), &response, sizeof(response));

    if (!qmiError) {
        phyChanInfo.pcell_info_valid = response.pcell_info_valid;
        if (phyChanInfo.pcell_info_valid) {
            phyChanInfo.radio_if_valid = true;
            phyChanInfo.radio_if = NAS_RADIO_IF_LTE_V01;
            phyChanInfo.physical_cell_id_valid = true;
            phyChanInfo.physical_cell_id = response.pcell_info.pci;
            phyChanInfo.pcell_info = response.pcell_info;
        }

        phyChanInfo.scell_info_list_valid = response.cphy_scell_info_list_valid;
        if (phyChanInfo.scell_info_list_valid) {
            phyChanInfo.scell_info_list_len = response.cphy_scell_info_list_len;
            for (unsigned int i = 0;
                    i < phyChanInfo.scell_info_list_len
                            && i < sizeof(phyChanInfo.scell_info_list) / sizeof(phyChanInfo.scell_info_list[0]);
                    i++) {
                phyChanInfo.scell_info_list[i] = response.cphy_scell_info_list[i];
            }
        }
    } else {
        Log::getInstance().d("[NasModemEndPoint]: Failed to get LTE carrier aggregation event information due to QMI error "
                + std::to_string(qmiError) + ".");
    }
}

void NasModemEndPoint::getRfBandInfo(PhysChanInfo &phyChanInfo) {
    nas_get_rf_band_info_req_msg_v01 request = {};
    nas_get_rf_band_info_resp_msg_v01 response = {};

    qmi_client_error_type qmiError = sendRawSync(QMI_NAS_GET_RF_BAND_INFO_REQ_MSG_V01,
            &request, sizeof(request), &response, sizeof(response));

    if (!qmiError) {
        phyChanInfo.bandwidth_info_list_valid = response.nas_rf_bandwidth_info_valid;
        if (phyChanInfo.bandwidth_info_list_valid) {
            phyChanInfo.bandwidth_info_list_len = response.nas_rf_bandwidth_info_len;
            for (unsigned int i = 0;
                    i < phyChanInfo.bandwidth_info_list_len
                            && i < sizeof(phyChanInfo.bandwidth_info_list) / sizeof(phyChanInfo.bandwidth_info_list[0]);
                    i++) {
                phyChanInfo.bandwidth_info_list[i] = response.nas_rf_bandwidth_info[i];
            }
        }
    } else {
        Log::getInstance().d("[NasModemEndPoint]: Failed to get RF band information due to QMI error "
                + std::to_string(qmiError) + ".");
    }
}

void NasModemEndPoint::getPhysicalCellId(PhysChanInfo &phyChanInfo) {
    nas_get_cell_location_info_resp_msg_v01 response = {};

    qmi_client_error_type qmiError = sendRawSync(QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01,
            nullptr, 0, &response, sizeof(response));

    if (!qmiError) {
        if (response.lte_intra_valid) {
            phyChanInfo.radio_if_valid = true;
            phyChanInfo.radio_if = NAS_RADIO_IF_LTE_V01;
            phyChanInfo.physical_cell_id_valid = true;
            phyChanInfo.physical_cell_id = response.lte_intra.serving_cell_id;
        } else if (response.umts_info_valid && response.umts_info.cell_id != std::numeric_limits<uint32_t>::max()) {
            phyChanInfo.radio_if_valid = true;
            phyChanInfo.radio_if = NAS_RADIO_IF_UMTS_V01;
            phyChanInfo.physical_cell_id_valid = true;
            phyChanInfo.physical_cell_id = response.umts_info.cell_id;
        } else if (response.geran_info_valid && response.geran_info.cell_id != std::numeric_limits<uint32_t>::max()) {
            phyChanInfo.radio_if_valid = true;
            phyChanInfo.radio_if = NAS_RADIO_IF_GSM_V01;
            phyChanInfo.physical_cell_id_valid = true;
            phyChanInfo.physical_cell_id = response.geran_info.cell_id;
        }
    } else {
        Log::getInstance().d("[NasModemEndPoint]: Failed to get physical cell ID due to QMI error "
                + std::to_string(qmiError) + ".");
    }
}

void NasModemEndPoint::getPhysChanConfig(PhysChanInfo &phyChanInfo) {
    getLteCarrierAggregationEventInfo(phyChanInfo);
    getRfBandInfo(phyChanInfo);
    getPhysicalCellId(phyChanInfo);
}

