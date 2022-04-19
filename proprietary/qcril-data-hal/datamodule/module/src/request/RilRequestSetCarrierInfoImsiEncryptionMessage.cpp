/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#include <string.h>
#include "request/RilRequestSetCarrierInfoImsiEncryptionMessage.h"

namespace rildata {

RilRequestSetCarrierInfoImsiEncryptionMessage::~RilRequestSetCarrierInfoImsiEncryptionMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }

  qcril_request_params_type p = params.get_params();
  RIL_CarrierInfoForImsiEncryption *pInfo = (RIL_CarrierInfoForImsiEncryption *)p.data;
  if (pInfo != nullptr) {
    if (pInfo->mcc != nullptr)
      delete pInfo->mcc;
    if (pInfo->mnc != nullptr)
      delete pInfo->mnc;
    if (pInfo->carrierKey != nullptr)
      delete pInfo->carrierKey;
    if (pInfo->keyIdentifier != nullptr)
      delete pInfo->keyIdentifier;
  }
}


void RilRequestSetCarrierInfoImsiEncryptionMessage::deepCopy(qcril_request_params_type &p) {
  unsigned size;
  RIL_CarrierInfoForImsiEncryption *pInfo = (RIL_CarrierInfoForImsiEncryption *)p.data;

  if (pInfo != NULL) {
    RIL_CarrierInfoForImsiEncryption *new_info = new RIL_CarrierInfoForImsiEncryption;

    size = strlen(pInfo->mcc) + 1;
    new_info->mcc = new char[size];
    if(new_info->mcc != nullptr) {
      strlcpy(new_info->mcc, pInfo->mcc, size);
    }

    size = strlen(pInfo->mnc) + 1;
    new_info->mnc = new char[size];
    if(new_info->mnc != nullptr) {
      strlcpy(new_info->mnc, pInfo->mnc, size);
    }

    new_info->carrierKeyLength = pInfo->carrierKeyLength;
    new_info->carrierKey = new uint8_t[new_info->carrierKeyLength];
    if(new_info->carrierKey != nullptr) {
      memcpy(new_info->carrierKey, pInfo->carrierKey, new_info->carrierKeyLength);
    }

    size = strlen(pInfo->keyIdentifier) + 1;
    new_info->keyIdentifier = new char[size];
    if(new_info->keyIdentifier != nullptr) {
      strlcpy(new_info->keyIdentifier, pInfo->keyIdentifier, size);
    }

    new_info->expirationTime = pInfo->expirationTime;

    p.data = new_info;
  }
}

string RilRequestSetCarrierInfoImsiEncryptionMessage::dump() {
  return mName;
}

}// namespace
