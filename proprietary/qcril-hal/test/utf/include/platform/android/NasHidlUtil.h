/*===========================================================================
 *
 *    Copyright (c) 2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#pragma once

#include "android/hardware/radio/1.4/IRadio.h"
#include "ril_utf_ril_api.h"
#include "ril.h"

using namespace ::android::hardware;
using namespace ::android::hardware::radio;

void fillCellIdentityGsm(const V1_2::CellIdentityGsm &in, RIL_CellIdentityGsm_v12 &out);
void fillCellIdentityWcdma(const V1_2::CellIdentityWcdma &in, RIL_CellIdentityWcdma_v12 &out);
void fillCellIdentityCdma(const V1_2::CellIdentityCdma &in, RIL_CellIdentityCdma &out);
void fillCellIdentityLte(const V1_2::CellIdentityLte &in, RIL_CellIdentityLte_v12 &out);
void fillCellIdentityTdscdma(const V1_2::CellIdentityTdscdma &in, RIL_CellIdentityTdscdma &out);

void fillCellIdentityGsm(const V1_0::CellIdentityGsm &in, RIL_CellIdentityGsm_v12 &out);
void fillCellIdentityWcdma(const V1_0::CellIdentityWcdma &in, RIL_CellIdentityWcdma_v12 &out);
void fillCellIdentityCdma(const V1_0::CellIdentityCdma &in, RIL_CellIdentityCdma &out);
void fillCellIdentityLte(const V1_0::CellIdentityLte &in, RIL_CellIdentityLte_v12 &out);
void fillCellIdentityTdscdma(const V1_0::CellIdentityTdscdma &in, RIL_CellIdentityTdscdma &out);

template <typename T>
void convertHidlCellIdentityResponsetoRil(T &cellIdentity, RIL_CellIdentity_v16 &rilCellIdentity) {
    rilCellIdentity.cellInfoType = (RIL_CellInfoType)cellIdentity.cellInfoType;
    switch (rilCellIdentity.cellInfoType) {
    case RIL_CELL_INFO_TYPE_GSM:
        fillCellIdentityGsm(cellIdentity.cellIdentityGsm[0], rilCellIdentity.cellIdentityGsm);
        break;
    case RIL_CELL_INFO_TYPE_WCDMA:
        fillCellIdentityWcdma(cellIdentity.cellIdentityWcdma[0], rilCellIdentity.cellIdentityWcdma);
        break;
    case RIL_CELL_INFO_TYPE_CDMA:
        fillCellIdentityCdma(cellIdentity.cellIdentityCdma[0], rilCellIdentity.cellIdentityCdma);
        break;
    case RIL_CELL_INFO_TYPE_LTE:
        fillCellIdentityLte(cellIdentity.cellIdentityLte[0], rilCellIdentity.cellIdentityLte);
        break;
    case RIL_CELL_INFO_TYPE_TD_SCDMA:
        fillCellIdentityTdscdma(cellIdentity.cellIdentityTdscdma[0], rilCellIdentity.cellIdentityTdscdma);
        break;
    default:
        break;
    }
}

template <typename T>
void convertHidlVoiceRegistrationStateResponseToRil(T& hidlData,
        RIL_VoiceRegistrationStateResponse& rilData) {
    rilData.regState = (RIL_RegState)hidlData.regState;
    rilData.rat = (RIL_RadioTechnology)hidlData.rat;
    rilData.cssSupported = hidlData.cssSupported;
    rilData.roamingIndicator = hidlData.roamingIndicator;
    rilData.systemIsInPrl = hidlData.systemIsInPrl;
    rilData.defaultRoamingIndicator = hidlData.defaultRoamingIndicator;
    rilData.reasonForDenial = hidlData.reasonForDenial;
    convertHidlCellIdentityResponsetoRil(hidlData.cellIdentity, rilData.cellIdentity);
}

template <typename T>
void convertHidlDataRegistrationStateResponseToRil(T& hidlData,
        RIL_DataRegistrationStateResponse& rilData) {
    rilData.regState = (RIL_RegState)hidlData.regState;
    rilData.rat = (RIL_RadioTechnology)hidlData.rat;
    rilData.reasonDataDenied = hidlData.reasonDataDenied;
    rilData.maxDataCalls = hidlData.maxDataCalls;
    convertHidlCellIdentityResponsetoRil(hidlData.cellIdentity, rilData.cellIdentity);
}

void convertHidlDataRegistrationStateResponseToRil_1_4(
        const V1_4::DataRegStateResult& hidlData,
        RIL_DataRegistrationStateResponse& rilData);

ril_request_operator_resp_from_qcril_t* convertHidlOperatorReponseToRil(
    const hidl_string& longName, const hidl_string& shortName, const hidl_string& numeric);
ril_request_query_available_networks_resp_t* convertHidlLegacyNetworkscanResponseToRil(
    const hidl_vec<V1_0::OperatorInfo>& networkInfos);

void convertHidlSignalStrengthToRil(const V1_0::SignalStrength& in,
        RIL_SignalStrength_v10& out);
void convertHidlSignalStrengthToRil(const V1_2::SignalStrength& in,
        RIL_SignalStrength_v10& out);
void convertHidlSignalStrengthToRil(const V1_4::SignalStrength& in,
        RIL_SignalStrength_v10& out);
void convertToRil(const V1_0::RadioBandMode& in, RIL_RadioBandMode& out);
void convertToHidl(const RIL_RadioBandMode& in, V1_0::RadioBandMode& out);
void convertToRil(const hidl_string& imei, const hidl_string& imeisv, const hidl_string& esn,
        const hidl_string& meid, ril_request_device_identity_type_t* out);
void convertToRil(const hidl_vec<V1_4::EmergencyNumber>& in,
        ril_unsol_emergency_list_t *out);
void convertNetworkScanRequestToHidl(const RIL_NetworkScanRequest &in,
                                     V1_2::NetworkScanRequest &out);
void convertNetworkScanResultToRil(const V1_4::NetworkScanResult &in, RIL_NetworkScanResult &out);
