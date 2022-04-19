/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <string>
#include <telephony/ril.h>
#include <optional>

// Message Callback structure
struct RilSendSmsResult_t {
  RIL_Errno rilErr;
  int       messageRef;
  string    ackPDU;
  int       errorCode;
  std::optional<bool> bLteOnlyReg;
  inline RilSendSmsResult_t(RIL_Errno err): rilErr(err),
        messageRef(-1), errorCode(-1) {}
  inline RilSendSmsResult_t(RIL_Errno err, int ref, int code):
        rilErr(err), messageRef(ref), errorCode(code) {}
  inline RilSendSmsResult_t(RIL_Errno err, int ref, int code, bool lteOnly):
        rilErr(err), messageRef(ref), errorCode(code), bLteOnlyReg(lteOnly) {}
};

struct RilWriteSmsToSimResult_t {
  RIL_Errno rilErr;
  int32_t   recordNumber;
  inline RilWriteSmsToSimResult_t(RIL_Errno err, int32_t index = -1):
        rilErr(err), recordNumber(index) {}
};

struct RilGetSmscAddrResult_t {
  RIL_Errno rilErr;
  string    smscAddr;
  inline RilGetSmscAddrResult_t(RIL_Errno err):
        rilErr(err) {}
  template<typename T>
  inline RilGetSmscAddrResult_t(RIL_Errno err, T&& addr):
        rilErr(err), smscAddr(std::forward<T>(addr)) {}
};

struct RilGetGsmBroadcastConfigResult_t {
  RIL_Errno rilErr;
  std::vector<RIL_GSM_BroadcastSmsConfigInfo> configList;
  inline RilGetGsmBroadcastConfigResult_t(RIL_Errno err):
        rilErr(err) {}
  template<typename T>
  inline RilGetGsmBroadcastConfigResult_t(RIL_Errno err, T&& list):
        rilErr(err), configList(std::forward<T>(list)) {}
};

struct RilGetCdmaBroadcastConfigResult_t {
  RIL_Errno rilErr;
  std::vector<RIL_CDMA_BroadcastSmsConfigInfo> configList;
  inline RilGetCdmaBroadcastConfigResult_t(RIL_Errno err):
        rilErr(err) {}
  template<typename T>
  inline RilGetCdmaBroadcastConfigResult_t(RIL_Errno err, T&& list):
        rilErr(err), configList(std::forward<T>(list)) {}
};

struct RilGetImsRegistrationResult_t {
  RIL_Errno rilErr;
  bool isRegistered;
  RIL_RadioTechnologyFamily ratFamily;
  inline RilGetImsRegistrationResult_t(RIL_Errno err):
        rilErr(err), isRegistered(false), ratFamily(RADIO_TECH_3GPP) {}
};
