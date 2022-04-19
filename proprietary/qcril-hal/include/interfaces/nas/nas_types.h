/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include "interfaces/common.h"
#include <string>
#include <vector>

namespace qcril {
namespace interfaces {

struct RilGetVoiceRegResult_t : public qcril::interfaces::BasePayload {
  RIL_VoiceRegistrationStateResponse respData;
};

struct RilGetDataRegResult_t : public qcril::interfaces::BasePayload {
  RIL_DataRegistrationStateResponse respData;
};

struct RilGetOperatorResult_t: public qcril::interfaces::BasePayload {
  std::string longName;
  std::string shortName;
  std::string numeric;
  template<typename T1, typename T2, typename T3>
  inline RilGetOperatorResult_t(T1&& _longName, T2&& _shortName, T3&& _numeric):
        longName(std::forward<T1>(_longName)), shortName(std::forward<T2>(_shortName)),
        numeric(std::forward<T3>(_numeric)) {}
};

struct RilGetSelectModeResult_t: public qcril::interfaces::BasePayload {
  bool bManual;
  explicit inline RilGetSelectModeResult_t(bool manual) : bManual(manual) {}
};

enum class NetworkStatus {UNKNOWN, AVAILABLE, CURRENT, FORBIDDEN};
struct NetworkInfo {
  std::string alphaLong;
  std::string alphaShort;
  std::string operatorNumeric;
  NetworkStatus status;
};
struct RilGetAvailNetworkResult_t: public qcril::interfaces::BasePayload {
  std::vector<NetworkInfo> info;
  template<typename T>
  inline RilGetAvailNetworkResult_t(T _info):
        info(std::forward<T>(_info)) {}
};

struct RilGetModePrefResult_t: public qcril::interfaces::BasePayload {
  int pref;
  inline RilGetModePrefResult_t(int value): pref(value) {}
};

struct RilGetSignalStrengthResult_t: public qcril::interfaces::BasePayload {
  RIL_SignalStrength respData;
};

struct RilGetVoiceTechResult_t: public qcril::interfaces::BasePayload {
  int rat;
  inline RilGetVoiceTechResult_t(int value): rat(value) {}
};

struct RilNeighboringCell_t {
  std::string cid;
  int rssi;
};

struct RilGetNeighborCellIdResult_t: public qcril::interfaces::BasePayload {
  std::vector<RilNeighboringCell_t> cellList;
  template<typename T>
  inline RilGetNeighborCellIdResult_t(T list):
        cellList(std::forward<T>(list)) {}
};

struct RilGetCdmaSubscriptionResult_t: public qcril::interfaces::BasePayload {
  std::string mdn;
  std::string hSid;
  std::string hNid;
  std::string min;
  std::string prl;

  template<typename T1, typename T2, typename T3, typename T4, typename T5>
  inline RilGetCdmaSubscriptionResult_t(T1 _mdn, T2 _hSid, T3 _hNid, T4 _min, T5 _prl):
    mdn(std::forward<T1>(_mdn)), hSid(std::forward<T2>(_hSid)),
    hNid(std::forward<T3>(_hNid)), min(std::forward<T4>(_min)),
    prl(std::forward<T5>(_prl)) {}
};

struct RilQueryAvailBandsResult_t: public qcril::interfaces::BasePayload {
  std::vector<int> bandList;
  template<typename T>
  inline RilQueryAvailBandsResult_t(T list):
    bandList(std::forward<T>(list)) {}
};

struct RilRadioCapResult_t: public qcril::interfaces::BasePayload {
  RIL_RadioCapability respData;
};

struct RilGetModemStackResult_t: public qcril::interfaces::BasePayload {
    bool state;
    inline RilGetModemStackResult_t(bool stackstate): state(stackstate) {}
};

enum class RIL_EccNumberSourceMask {
    DEFAULT = 1 << 0,
    NETWORK_SIGNALING = 1 << 1,
    SIM = 1 << 2,
    MODEM_CONFIG = 1 << 3
};

struct RIL_EmergencyNumber_t: public qcril::interfaces::BasePayload {
    std::string number;
    std::string mcc;
    std::string mnc;
    uint64_t categories;
    vector<std::string> urns;
    RIL_EccNumberSourceMask sources;
};

typedef std::vector<std::shared_ptr<qcril::interfaces::RIL_EmergencyNumber_t>> RIL_EccList;
typedef std::unordered_map<std::string, std::shared_ptr<RIL_EmergencyNumber_t>> RIL_EccMapType;

typedef struct {
    uint8_t modemId;
} RIL_ModemInfo;

typedef struct {
    uint8_t maxActiveData;
    uint8_t maxActiveInternetData;
    bool isInternetLingeringSupported;
    std::vector<RIL_ModemInfo> logicalModemList;
} RIL_PhoneCapability;

struct RilPhoneCapabilityResult_t: public qcril::interfaces::BasePayload {
    RIL_PhoneCapability phoneCap;
    inline RilPhoneCapabilityResult_t(RIL_PhoneCapability pc): phoneCap(pc) {}
};

struct RilQuery5GStatusResult_t: public qcril::interfaces::BasePayload {
  five_g_status status;
  inline RilQuery5GStatusResult_t(five_g_status _status): status(_status) {}
};

struct RilQueryEndcStatusResult_t: public qcril::interfaces::BasePayload {
  endc_status status;
  inline RilQueryEndcStatusResult_t(endc_status _status): status(_status) {}
};

struct RilQueryNrDcParamResult_t: public qcril::interfaces::BasePayload {
  five_g_endc_dcnr dc;
  inline RilQueryNrDcParamResult_t(five_g_endc_dcnr _dc): dc(_dc) {}
};

struct RilQueryNrBearAllocResult_t: public qcril::interfaces::BasePayload {
  five_g_bearer_status status;
  inline RilQueryNrBearAllocResult_t(five_g_bearer_status _status):
        status(_status) {}
};

struct RilQueryNrSignalStrengthResult_t: public qcril::interfaces::BasePayload {
  five_g_signal_strength signal;
  inline RilQueryNrSignalStrengthResult_t(five_g_signal_strength _signal):
        signal(_signal) {}
};

struct RilQueryUpperLayerIndInfoResult_t: public qcril::interfaces::BasePayload {
  five_g_upper_layer_ind_info upli_info;
  inline RilQueryUpperLayerIndInfoResult_t(five_g_upper_layer_ind_info _upli_info):
        upli_info(_upli_info) {}
};

struct RilQuery5gConfigInfoResult_t: public qcril::interfaces::BasePayload {
  five_g_config_type config_info;
  inline RilQuery5gConfigInfoResult_t(five_g_config_type _config_info):
        config_info(_config_info) {}
};

struct RilQueryCdmaRoamingPrefResult_t : public qcril::interfaces::BasePayload {
  int mPrefType;
  inline RilQueryCdmaRoamingPrefResult_t(int value): mPrefType(value) {}
};

struct RilGetCdmaSubscriptionSourceResult_t : public qcril::interfaces::BasePayload {
  RIL_CdmaSubscriptionSource mSource;
  inline RilGetCdmaSubscriptionSourceResult_t(RIL_CdmaSubscriptionSource value): mSource(value) {}
};

}  // namespace interfaces
}  // namespace qcril
