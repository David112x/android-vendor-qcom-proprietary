/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "gpqese-api.h"
#include "gpqese/gpqese-apdu.h"

static android::sp<ISecureElementHalCallback> seCallback;

Return<void> setSeCallBack(const android::sp<ISecureElementHalCallback>& clientCallback) {
  seCallback = clientCallback;
  return Void();
}

Return<void> initSEService() {

  if(!seCallback) {
     ALOGE("secure element callback not registered yet, skip initialization");
     return Void();
  }

  TEEC_Result result = gpqese_open();
  if (result == TEEC_SUCCESS) {
    seCallback->onStateChange(true);
  } else {
    seCallback->onStateChange(false);
  }

  return Void();
}
