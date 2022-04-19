/*===========================================================================
 *
 *    Copyright (c) 2018 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#include "platform/android/VoiceHidlUtil.h"

int convertDialInfotoHidl(void *data, size_t datalen, V1_0::Dial &dialInfo)
{
    auto payload = static_cast<RIL_Dial *>(data);
    dialInfo.address = payload->address;
    dialInfo.clir = (V1_0::Clir)payload->clir;

    if (payload->uusInfo) {
      dialInfo.uusInfo.resize(1);
      dialInfo.uusInfo[0].uusType = (V1_0::UusType) payload->uusInfo->uusType;
      dialInfo.uusInfo[0].uusDcs = (V1_0::UusDcs) payload->uusInfo->uusDcs;
      if (payload->uusInfo->uusData && payload->uusInfo->uusLength) {
        char *nullTermStr = strndup(payload->uusInfo->uusData, payload->uusInfo->uusLength);
        dialInfo.uusInfo[0].uusData = nullTermStr;
        free(nullTermStr);
      }
    }
    return 0;
}

V1_0::CallForwardInfoStatus convertCallForwardInfoStatustoHidl(int in) {
  switch(in) {
    case 0:
      return V1_0::CallForwardInfoStatus::DISABLE;
    case 1:
      return V1_0::CallForwardInfoStatus::ENABLE;
    case 2:
      return V1_0::CallForwardInfoStatus::INTERROGATE;
    case 3:
      return V1_0::CallForwardInfoStatus::REGISTRATION;
    case 4:
      return V1_0::CallForwardInfoStatus::ERASURE;
    default:
      return V1_0::CallForwardInfoStatus::DISABLE;
  }
  return V1_0::CallForwardInfoStatus::DISABLE;
}

int convertCallForwardInfoStatustoUtf(V1_0::CallForwardInfoStatus in) {
  switch (in) {
  case V1_0::CallForwardInfoStatus::DISABLE:
    return 0;
  case V1_0::CallForwardInfoStatus::ENABLE:
    return 1;
  case V1_0::CallForwardInfoStatus::INTERROGATE:
    return 2;
  case V1_0::CallForwardInfoStatus::REGISTRATION:
    return 3;
  case V1_0::CallForwardInfoStatus::ERASURE:
    return 4;
  default:
    return 0;
  }
  return 0;
}

int convertCallForwardInfotoHidl(void *data, size_t datalen, V1_0::CallForwardInfo &info) {
  auto payload = static_cast<RIL_CallForwardInfo *>(data);
  info.status = convertCallForwardInfoStatustoHidl(payload->status);
  info.reason = payload->reason;
  info.serviceClass = payload->serviceClass;
  info.toa = payload->toa;
  if (payload->number) {
    info.number = payload->number;
  }
  info.timeSeconds = payload->timeSeconds;
  return 0;
}

int convertCallForwardInfotoUtf(RIL_CallForwardInfo &out, const V1_0::CallForwardInfo &in) {
  out.status = convertCallForwardInfoStatustoUtf(in.status);
  out.reason = in.reason;
  out.serviceClass = in.serviceClass;
  out.toa = in.toa;
  if (in.number.size()) {
    out.number = strdup(in.number.c_str());
  }
  out.timeSeconds = in.timeSeconds;
  return 0;
}
