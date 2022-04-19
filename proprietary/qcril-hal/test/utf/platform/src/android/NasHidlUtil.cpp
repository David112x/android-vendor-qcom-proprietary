/*===========================================================================
 *
 *    Copyright (c) 2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#include "platform/android/NasHidlUtil.h"
#include "ril_utf_core.h"

static void convertHidlMccMncToRil(const hidl_string &in, char* out) {
    strlcpy(out, in.c_str(), MAX_RIL_MCC_MNC_SIZE);
}

static void convertHidlOperatorNamesToRil(const V1_2::CellIdentityOperatorNames& in,
        RIL_CellIdentityOperatorNames& out) {
    strlcpy(out.alphaShort, in.alphaShort.c_str(), MAX_OPERATOR_NAME_LEN);
    strlcpy(out.alphaLong, in.alphaLong.c_str(), MAX_OPERATOR_NAME_LEN);
}

void fillCellIdentityGsm(const V1_0::CellIdentityGsm &in, RIL_CellIdentityGsm_v12 &out) {
    convertHidlMccMncToRil(in.mcc, out.mcc);
    convertHidlMccMncToRil(in.mnc, out.mnc);
    out.lac = in.lac;
    out.cid = in.cid;
    out.arfcn = in.arfcn;
    out.bsic = in.bsic;
}

void fillCellIdentityWcdma(const V1_0::CellIdentityWcdma &in, RIL_CellIdentityWcdma_v12 &out) {
    convertHidlMccMncToRil(in.mcc, out.mcc);
    convertHidlMccMncToRil(in.mnc, out.mnc);
    out.lac = in.lac;
    out.cid = in.cid;
    out.psc = in.psc;
    out.uarfcn = in.uarfcn;
}

void fillCellIdentityCdma(const V1_0::CellIdentityCdma &in, RIL_CellIdentityCdma &out) {
    out.networkId = in.networkId;
    out.systemId = in.systemId;
    out.basestationId = in.baseStationId;
    out.longitude = in.longitude;
    out.latitude = in.latitude;
}

void fillCellIdentityLte(const V1_0::CellIdentityLte &in, RIL_CellIdentityLte_v12 &out) {
    convertHidlMccMncToRil(in.mcc, out.mcc);
    convertHidlMccMncToRil(in.mnc, out.mnc);
    out.ci = in.ci;
    out.pci = in.pci;
    out.tac = in.tac;
    out.earfcn = in.earfcn;
}

void fillCellIdentityTdscdma(const V1_0::CellIdentityTdscdma &in, RIL_CellIdentityTdscdma &out) {
    convertHidlMccMncToRil(in.mcc, out.mcc);
    convertHidlMccMncToRil(in.mnc, out.mnc);
    out.lac = in.lac;
    out.cid = in.cid;
    out.cpid = in.cpid;
}

void fillCellIdentityGsm(const V1_2::CellIdentityGsm &in, RIL_CellIdentityGsm_v12 &out) {
    fillCellIdentityGsm(in.base, out);
    convertHidlOperatorNamesToRil(in.operatorNames, out.operatorNames);
}

void fillCellIdentityWcdma(const V1_2::CellIdentityWcdma &in, RIL_CellIdentityWcdma_v12 &out) {
    fillCellIdentityWcdma(in.base, out);
    convertHidlOperatorNamesToRil(in.operatorNames, out.operatorNames);
}

void fillCellIdentityCdma(const V1_2::CellIdentityCdma &in, RIL_CellIdentityCdma &out) {
    fillCellIdentityCdma(in.base, out);
    convertHidlOperatorNamesToRil(in.operatorNames, out.operatorNames);
}

void fillCellIdentityLte(const V1_2::CellIdentityLte &in, RIL_CellIdentityLte_v12 &out) {
    fillCellIdentityLte(in.base, out);
    convertHidlOperatorNamesToRil(in.operatorNames, out.operatorNames);
    out.bandwidth = in.bandwidth;
}

void fillCellIdentityTdscdma(const V1_2::CellIdentityTdscdma &in, RIL_CellIdentityTdscdma &out) {
    fillCellIdentityTdscdma(in.base, out);
    convertHidlOperatorNamesToRil(in.operatorNames, out.operatorNames);
    // RIL structure has no uarfcn field
}

ril_request_operator_resp_from_qcril_t* convertHidlOperatorReponseToRil(
        const hidl_string& longName, const hidl_string& shortName, const hidl_string& numeric) {
    // use malloc since framework will free it use free()
    auto ptr = (ril_request_operator_resp_from_qcril_t*)malloc(
            sizeof(ril_request_operator_resp_from_qcril_t));
    if (ptr == nullptr)
        return nullptr;
    memset(ptr->operator_info_array, 0, NAS_OPERATOR_RESP_MAX_ARR_SIZE);

    auto operatorResp = (ril_request_operator_resp_t*)malloc(
            sizeof(ril_request_operator_resp_t));
    if (operatorResp == nullptr) {
        free(ptr);
        return nullptr;
    }
    memset(operatorResp, '\0', sizeof(*operatorResp));
    strlcpy(operatorResp->long_eons, longName.c_str(), sizeof(operatorResp->long_eons));
    strlcpy(operatorResp->short_eons, shortName.c_str(), sizeof(operatorResp->short_eons));
    strlcpy(operatorResp->mcc_mnc_ascii, numeric.c_str(), sizeof(operatorResp->mcc_mnc_ascii));

    ptr->operator_info_array[0] = operatorResp->long_eons;
    ptr->operator_info_array[1] = operatorResp->short_eons;
    ptr->operator_info_array[2] = operatorResp->mcc_mnc_ascii;

    return ptr;
}

static void fillNetworkStatus(V1_0::OperatorStatus status, char* buf, size_t len) {
    switch (status) {
    case V1_0::OperatorStatus::UNKNOWN:
        strlcpy(buf, "unknown", len);
        break;
    case V1_0::OperatorStatus::AVAILABLE:
        strlcpy(buf, "available", len);
        break;
    case V1_0::OperatorStatus::CURRENT:
        strlcpy(buf, "current", len);
        break;
    case V1_0::OperatorStatus::FORBIDDEN:
    default:
        strlcpy(buf, "forbidden", len);
        break;
    }
}

ril_request_query_available_networks_resp_t* convertHidlLegacyNetworkscanResponseToRil(
        const hidl_vec<V1_0::OperatorInfo>& networkInfos) {
    // use malloc since framework will free it use free()
    auto ptr = (ril_request_query_available_networks_resp_t *)malloc(
            sizeof(ril_request_query_available_networks_resp_t));
    if (ptr == nullptr)
        return ptr;
    memset(ptr, 0, sizeof(*ptr));

    unsigned int nw_cnt = networkInfos.size();
    if (nw_cnt == 0) { // keep the same logic as copy_ril_request_query_available_networks_resp()
        return ptr;
    }
    auto scanResp = (qcril_qmi_nw_scan_resp_helper_type *)malloc(
            nw_cnt * sizeof(qcril_qmi_nw_scan_resp_helper_type));
    if (scanResp == nullptr) {
        free(ptr);
        return nullptr;
    }
    memset(scanResp, 0, sizeof(*scanResp));

    unsigned int idx = 0;
    for (unsigned int item = 0; item < nw_cnt; item++) {
        ptr->nw_scan_info_array[idx++] = scanResp[item].long_eons;
        strlcpy(scanResp[item].long_eons, networkInfos[item].alphaLong.c_str(),
                sizeof(scanResp[item].long_eons));
        ptr->nw_scan_info_array[idx++] = scanResp[item].short_eons;
        strlcpy(scanResp[item].short_eons, networkInfos[item].alphaShort.c_str(),
                sizeof(scanResp[item].short_eons));
        ptr->nw_scan_info_array[idx++] = scanResp[item].mccmnc_info;
        strlcpy(scanResp[item].mccmnc_info, networkInfos[item].operatorNumeric.c_str(),
                sizeof(scanResp[item].mccmnc_info));
        ptr->nw_scan_info_array[idx++] = scanResp[item].network_status;
        fillNetworkStatus(networkInfos[item].status, scanResp[item].network_status,
                sizeof(scanResp[item].network_status));
    }
    return ptr;
}

int convertToRil(const V1_0::GsmSignalStrength &in, RIL_GW_SignalStrength &out) {
    out.signalStrength = in.signalStrength;
    out.bitErrorRate = in.bitErrorRate;
    return 0;
}

int convertToRil(const V1_0::GsmSignalStrength &in, RIL_WCDMA_SignalStrength &out) {
    out.signalStrength = in.signalStrength;
    out.bitErrorRate = in.bitErrorRate;
    return 0;
}

int convertToRil(const V1_0::WcdmaSignalStrength &in, RIL_GW_SignalStrength &out) {
    out.signalStrength = in.signalStrength;
    out.bitErrorRate = in.bitErrorRate;
    return 0;
}

int convertToRil(const V1_0::WcdmaSignalStrength &in, RIL_WCDMA_SignalStrength &out) {
    out.signalStrength = in.signalStrength;
    out.bitErrorRate = in.bitErrorRate;
    return 0;
}

int convertToRil(const V1_2::WcdmaSignalStrength &in, RIL_WCDMA_SignalStrength &out) {
    int ret = convertToRil(in.base, out);
    if (ret) return ret;
    out.rscp = in.rscp;
    out.ecio = in.ecno;
    return 0;
}

int convertToRil(const V1_0::CdmaSignalStrength &in, RIL_CDMA_SignalStrength &out) {
    out.dbm = in.dbm;
    out.ecio = in.ecio;
    return 0;
}

int convertToRil(const V1_0::EvdoSignalStrength &in, RIL_EVDO_SignalStrength &out) {
    out.dbm = in.dbm;
    out.ecio = in.ecio;
    out.signalNoiseRatio = in.signalNoiseRatio;
    return 0;
}

int convertToRil(const V1_0::LteSignalStrength &in, RIL_LTE_SignalStrength_v8 &out) {
    out.signalStrength = in.signalStrength;
    out.rsrp = in.rsrp;
    out.rsrq = in.rsrq;
    out.rssnr = in.rssnr;
    out.cqi = in.cqi;
    out.timingAdvance = in.timingAdvance;

    // Fixup LTE for backwards compatibility
    // signalStrength: -1 -> 99
    if (out.signalStrength == 99) {
        out.signalStrength = -1;
    }

    // rsrp: INT_MAX -> 1, all other positive value to negative
    if (out.rsrp == INT_MAX) {
        out.rsrp = -1;
    } else if (out.rsrp > 1) {
        out.rsrp = -out.rsrp;
    }

    // rsrq: INT_MAX -> -1
    if (out.rsrq == INT_MAX) {
        out.rsrq = -1;
    }

    // cqi: INT_MAX -> -1
    if (out.cqi == INT_MAX) {
        out.cqi = -1;
    }
    return 0;
}

int convertToRil(const V1_0::TdScdmaSignalStrength &in, RIL_TD_SCDMA_SignalStrength &out) {
    out.rscp = in.rscp;
    return 0;
}

int convertToRil(const V1_2::TdscdmaSignalStrength &in, RIL_TD_SCDMA_SignalStrength &out) {
    out.rscp = in.rscp;
    return 0;
}

void convertHidlSignalStrengthToRil(const V1_0::SignalStrength& in,
        RIL_SignalStrength_v10& out) {
    convertToRil(in.gw, out.GW_SignalStrength);
    convertToRil(in.gw, out.WCDMA_SignalStrength);
    convertToRil(in.cdma, out.CDMA_SignalStrength);
    convertToRil(in.evdo, out.EVDO_SignalStrength);
    convertToRil(in.lte, out.LTE_SignalStrength);
    convertToRil(in.tdScdma, out.TD_SCDMA_SignalStrength);
}

void convertHidlSignalStrengthToRil(const V1_2::SignalStrength& in,
        RIL_SignalStrength_v10& out) {
    convertToRil(in.gsm, out.GW_SignalStrength);
    convertToRil(in.wcdma, out.WCDMA_SignalStrength);
    convertToRil(in.cdma, out.CDMA_SignalStrength);
    convertToRil(in.evdo, out.EVDO_SignalStrength);
    convertToRil(in.lte, out.LTE_SignalStrength);
    convertToRil(in.tdScdma, out.TD_SCDMA_SignalStrength);
}

void convertHidlSignalStrengthToRil(const V1_4::SignalStrength& in,
        RIL_SignalStrength_v10& out) {
    convertToRil(in.gsm, out.GW_SignalStrength);
    convertToRil(in.wcdma, out.WCDMA_SignalStrength);
    convertToRil(in.cdma, out.CDMA_SignalStrength);
    convertToRil(in.evdo, out.EVDO_SignalStrength);
    convertToRil(in.lte, out.LTE_SignalStrength);
    convertToRil(in.tdscdma, out.TD_SCDMA_SignalStrength);
}

void convertHidlDataRegistrationStateResponseToRil_1_4(
        const V1_4::DataRegStateResult& hidlData,
        RIL_DataRegistrationStateResponse& rilData) {
    convertHidlDataRegistrationStateResponseToRil(hidlData.base, rilData);
}

void convertToRil(const hidl_string& imei, const hidl_string& imeisv, const hidl_string& esn,
        const hidl_string& meid, ril_request_device_identity_type_t* out) {
    strlcpy(out->imei, imei.c_str(), IMEI_MAX_STR_SIZE);
    strlcpy(out->imeisv, imeisv.c_str(), IMEISV_MAX_STR_SIZE);
    strlcpy(out->esn, esn.c_str(), ESN_MAX_STR_SIZE);
    strlcpy(out->meid, meid.c_str(), MEID_MAX_STR_SIZE);
}

void convertToRil(const hidl_vec<V1_4::EmergencyNumber>& in,
        ril_unsol_emergency_list_t *out) {
    out->ecc_list.emergency_list_length = in.size();
    for(int i=0; i< in.size(); i++) {
        out->ecc_list.emergency_numbers[i].category = in[i].categories;
        out->ecc_list.emergency_numbers[i].sources = in[i].sources;
        strlcpy(out->ecc_list.emergency_numbers[i].number,
                in[i].number.c_str(), MAX_EMERGENCY_NUMBER_LENGTH);
        strlcpy(out->ecc_list.emergency_numbers[i].mcc,
                in[i].mcc.c_str(), MAX_RIL_MCC_MNC_SIZE);
        strlcpy(out->ecc_list.emergency_numbers[i].mnc,
                in[i].mnc.c_str(), MAX_RIL_MCC_MNC_SIZE);
        for ( int j = 0; j < in[i].urns.size(); j++ ) {
            strlcpy(out->ecc_list.emergency_numbers[i].urn[j], in[i].urns[j].c_str(), MAX_URN_LENGTH);
        }
    }
}

hidl_vec<V1_1::RadioAccessSpecifier>
convertRadioAccessSpecifier(const uint32_t len, const RIL_RadioAccessSpecifier in[MAX_RADIO_ACCESS_NETWORKS]) {
  hidl_vec<V1_1::RadioAccessSpecifier> out;
  if (len) {
    out.resize(len);
    for (uint32_t i = 0; i < len; i++) {
      out[i].radioAccessNetwork = (V1_1::RadioAccessNetworks)in[i].radio_access_network;
      if (out[i].radioAccessNetwork == V1_1::RadioAccessNetworks::GERAN) {
        out[i].geranBands.resize(in[i].bands_length);
        for (uint32_t j = 0; j < in[i].bands_length; j++) {
          out[i].geranBands[j] = (V1_1::GeranBands) in[i].bands.geran_bands[j];
        }
      } else if (out[i].radioAccessNetwork == V1_1::RadioAccessNetworks::UTRAN) {
        out[i].utranBands.resize(in[i].bands_length);
        for (uint32_t j = 0; j < in[i].bands_length; j++) {
          out[i].utranBands[j] = (V1_1::UtranBands) in[i].bands.utran_bands[j];
        }
      } else if (out[i].radioAccessNetwork == V1_1::RadioAccessNetworks::EUTRAN) {
        out[i].eutranBands.resize(in[i].bands_length);
        for (uint32_t j = 0; j < in[i].bands_length; j++) {
          out[i].eutranBands[j] = (V1_1::EutranBands) in[i].bands.eutran_bands[j];
        }
      }
      out[i].channels.resize(in[i].channels_length);
      for (uint32_t j = 0; j < in[i].channels_length && i < MAX_CHANNELS; j++) {
        out[i].channels[j] = in[i].channels[j];
      }
    }
  }
  return out;
}

void convertNetworkScanRequestToHidl(const RIL_NetworkScanRequest &in,
                                     V1_2::NetworkScanRequest &out) {
  out.type = V1_1::ScanType::ONE_SHOT;
  if (in.type == RIL_ONE_SHOT) {
    out.type = V1_1::ScanType::ONE_SHOT;
  } else if (in.type == RIL_PERIODIC) {
    out.type = V1_1::ScanType::PERIODIC;
  }
  out.interval = in.interval;
  out.specifiers = convertRadioAccessSpecifier(in.specifiers_length, in.specifiers);
  out.maxSearchTime = in.maxSearchTime;
  out.incrementalResults = in.incrementalResults;
  out.incrementalResultsPeriodicity = in.incrementalResultsPeriodicity;
  out.mccMncs.resize(in.mccMncLength);
  for (uint32_t j = 0; j < in.mccMncLength && j < MAX_MCC_MNC_LIST_SIZE; j++) {
    out.mccMncs[j] = in.mccMncs[j];
  }
}
void convertNetworkScanResultToRil(const V1_4::NetworkScanResult &in, RIL_NetworkScanResult &out) {
  out.status = (RIL_ScanStatus)in.status;
  out.error = (RIL_Errno)in.error;
  out.network_infos_length = in.networkInfos.size();
  out.network_infos = (RIL_CellInfo_v12*)malloc(sizeof(RIL_CellInfo_v12)*out.network_infos_length);
  //qcril_malloc2(out.network_infos, out.network_infos_length);
  if (out.network_infos) {
    // TODO
  }
}
