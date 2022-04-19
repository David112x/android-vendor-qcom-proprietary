/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "SecureElement.h"
#include "gpqese-api.h"
#include "eSEClient.h"

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {
namespace implementation {

static std::mutex m;
static sp<ISecureElementHalCallback> clientCB = NULL;

// Methods from ::android::hardware::secure_element::V1_0::ISecureElement follow.
Return<void> SecureElement::init(const
    sp<ISecureElementHalCallback> &clientCallback) {

  if(ese_update != ESE_UPDATE_COMPLETED) {
    ALOGE("SecureElement::init JCOP or Loader service download is in progress");
    clientCB = clientCallback;
    clientCB->onStateChange(false);
    setSeCallBack(clientCallback);
    return Void();
  }

  TEEC_Result result = ese_init();
  if (result == TEEC_SUCCESS) {
    clientCB = clientCallback;
    clientCB->onStateChange(true);
    return Void();
  }
  ALOGE("SecureElement::init return FAILED");
  return Void();
}

Return<void> SecureElement::getAtr(getAtr_cb _hidl_cb) {
  // There's no ATR available on the p61
  hidl_vec<uint8_t> response;
  _hidl_cb(response);
  return Void();
}

Return<bool> SecureElement::isCardPresent() {
  TEEC_Result result = gpqese_open();
  if (result == TEEC_SUCCESS) {
    return true;
  } else if (clientCB != NULL) {
    clientCB->onStateChange(false);
  }
  return false;
}

Return<void> SecureElement::transmit(const hidl_vec<uint8_t> &data,
                                     transmit_cb _hidl_cb) {
  std::lock_guard<std::mutex> lock(m);
  uint8_t rApdu[GPQESE_MAX_RAPDU_SIZE] = {};
  int rLen = 0;
  hidl_vec<uint8_t> respApdu ;
  TEEC_Result result = transmitApdu(data, rApdu, &rLen);

  if (result == TEEC_SUCCESS) {
    respApdu.resize(rLen);
    memscpy(&respApdu[0], rLen, rApdu, rLen);
    print_text("Received :", &respApdu[0], rLen);
  } else {
      ALOGE("Transmit FAILED !");
  }
  _hidl_cb(respApdu);
  return Void();
}


static Return<SecureElementStatus> doCloseChannel(uint8_t channelNumber) {
  TEEC_Result result = closeGPChannel(channelNumber);

  if (result == TEEC_SUCCESS) {
    return SecureElementStatus::SUCCESS;
  }

  ALOGE("SecureElement::closeChannel FAILED");
  return SecureElementStatus::FAILED;
}

Return<void> SecureElement::openLogicalChannel(const hidl_vec<uint8_t> &aid,
    uint8_t p2, openLogicalChannel_cb _hidl_cb) {
  std::lock_guard<std::mutex> lock(m);
  uint8_t rApdu[GPQESE_MAX_RAPDU_SIZE] = {};
  SecureElementStatus status = SecureElementStatus::FAILED;
  LogicalChannelResponse resApduBuff;
  memset(&resApduBuff, 0x00, sizeof(resApduBuff));
  resApduBuff.channelNumber = 0xff;
  int rLen = 0;
  int cNumber = 0;
  TEEC_Result result = openChannel(aid, p2, &cNumber, rApdu, &rLen, false);

  if ((result == TEEC_SUCCESS) && (rLen > 1)) {
    resApduBuff.channelNumber = cNumber;
    resApduBuff.selectResponse.resize(rLen);
    memscpy(&resApduBuff.selectResponse[0], rLen, rApdu, rLen);
    print_text("OpenLogical Success : Select Response :",
               &resApduBuff.selectResponse[0], rLen);
    status = SecureElementStatus::SUCCESS;
  } else if (result == TEEC_ERROR_OUT_OF_MEMORY) {
    ALOGE("SecureElement::OPENLOGICAL - No more channel available");
    resApduBuff.channelNumber = 0xff;
    status = SecureElementStatus::CHANNEL_NOT_AVAILABLE;
  } else if (result == TEEC_ERROR_ITEM_NOT_FOUND) {
    ALOGE("SecureElement::OPENLOGICAL - Applet not found");
    resApduBuff.channelNumber = cNumber;
    status = SecureElementStatus::NO_SUCH_ELEMENT_ERROR;
    doCloseChannel(resApduBuff.channelNumber);
  }

  _hidl_cb(resApduBuff, status);
  return Void();
}

Return<void> SecureElement::openBasicChannel(const hidl_vec<uint8_t> &aid,
    uint8_t p2, openBasicChannel_cb _hidl_cb) {
  std::lock_guard<std::mutex> lock(m);
  uint8_t rApdu[GPQESE_MAX_RAPDU_SIZE] = {};
  SecureElementStatus status = SecureElementStatus::FAILED;
  hidl_vec<uint8_t> resApduBuff;
  int rLen = 0;
  int cNumber = 0;
  TEEC_Result result = openChannel(aid, p2, &cNumber, rApdu, &rLen, true);

  if (result == TEEC_SUCCESS) {
    resApduBuff.resize(rLen);
    memscpy(&resApduBuff[0], rLen, rApdu, rLen);
    print_text("OpenBasic Success : Select Response :", &resApduBuff[0], rLen);
    status = SecureElementStatus::SUCCESS;
  } else if (result == TEEC_ERROR_OUT_OF_MEMORY) {
    ALOGE("SecureElement::OpenBasic - No more channel available");
    status = SecureElementStatus::CHANNEL_NOT_AVAILABLE;
  } else if (result == TEEC_ERROR_ITEM_NOT_FOUND) {
    ALOGE("SecureElement::OpenBasic - Applet not found");
    status = SecureElementStatus::NO_SUCH_ELEMENT_ERROR;
    doCloseChannel(0);
  }

  _hidl_cb(resApduBuff, status);
  return Void();
}

Return<SecureElementStatus> SecureElement::closeChannel(uint8_t channelNumber) {
  std::lock_guard<std::mutex> lock(m);
  return doCloseChannel(channelNumber);
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android
