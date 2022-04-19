/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include <variant>
#include <string>
#include <vector>
#include "MessageCommon.h"

using namespace std;

namespace rildata {

enum class CellInfoType_t : int32_t {
  NONE = 0,
  GSM = 1,
  CDMA = 2,
  LTE = 3,
  WCDMA = 4,
  TD_SCDMA = 5
};

struct CellIdentityOperatorNames_t {
  //-- Radio 1.0 --//
  string alphaLong;
  string alphaShort;
};

struct CellIdentityGsm_t {
  //-- Radio 1.0 --//
  string mcc;
  string mnc;
  int32_t lac;
  int32_t cid;
  int32_t arfcn;
  uint8_t bsic;
  //-- Radio 1.2 --//
  CellIdentityOperatorNames_t operatorNames;
};

struct CellIdentityWcdma_t {
  //-- Radio 1.0 --//
  string mcc;
  string mnc;
  int32_t lac;
  int32_t cid;
  int32_t psc;
  int32_t uarfcn;
  //-- Radio 1.2 --//
  CellIdentityOperatorNames_t operatorNames;
};

struct CellIdentityCdma_t {
  //-- Radio 1.0 --//
  int32_t networkId;
  int32_t systemId;
  int32_t baseStationId;
  int32_t longitude;
  int32_t latitude;
  //-- Radio 1.2 --//
  CellIdentityOperatorNames_t operatorNames;
};

struct CellIdentityLte_t {
  //-- Radio 1.0 --//
  string mcc;
  string mnc;
  int32_t ci;
  int32_t pci;
  int32_t tac;
  int32_t earfcn;
  //-- Radio 1.2 --//
  CellIdentityOperatorNames_t operatorNames;
  int32_t bandwidth;
};

struct CellIdentityTdscdma_t {
  //-- Radio 1.0 --//
  string mcc;
  string mnc;
  int32_t lac;
  int32_t cid;
  int32_t cpid;
  //-- Radio 1.2 --//
  int32_t uarfcn;
  CellIdentityOperatorNames_t operatorNames;
};

struct CellIdentity_t {
  //-- Radio 1.0 --//
  CellInfoType_t cellInfoType;
  vector<CellIdentityGsm_t> cellIdentityGsm;
  vector<CellIdentityWcdma_t> cellIdentityWcdma;
  vector<CellIdentityCdma_t> cellIdentityCdma;
  vector<CellIdentityLte_t> cellIdentityLte;
  vector<CellIdentityTdscdma_t> cellIdentityTdscdma;
};

struct LteVopsInfo_t {
  //-- Radio 1.3 --//
  bool isVopsSupported;
  bool isEmcBearerSupported;
};

struct VopsInfo_t {
  //-- Radio 1.3 --//
  bool isVopsInfoValid;     // for monostate in union
  LteVopsInfo_t lteVopsInfo;
};

struct RadioDataRegistrationStateResult_t {
  ResponseError_t respErr;
  //-- Radio 1.0 --//
  DataRegState_t regState;
  int32_t rat;
  int32_t reasonDataDenied;
  int32_t maxDataCalls;
  CellIdentity_t cellIdentity;
  //-- Radio 1.3 --//
  VopsInfo_t vopsInfo;
  bool isDcNrRestricted;
  bool isNrAvailable;
};

class GetRadioDataRegistrationStateRequestMessage : public SolicitedMessage<RadioDataRegistrationStateResult_t>,
                          public add_message_id<GetRadioDataRegistrationStateRequestMessage> {
  public:

    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.GetRadioDataRegistrationStateRequestMessage";

    inline GetRadioDataRegistrationStateRequestMessage():
      SolicitedMessage<RadioDataRegistrationStateResult_t>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }
    ~GetRadioDataRegistrationStateRequestMessage() = default;

    string dump(){return mName;}
};

}
