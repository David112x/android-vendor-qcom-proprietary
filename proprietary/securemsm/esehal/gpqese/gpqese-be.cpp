/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "gpqese-be.h"

using namespace android;

using ::vendor::qti::esepowermanager::V1_0::IEsePowerManager;
using ::android::hidl::base::V1_0::IBase;

static TEEC_Result result = TEEC_SUCCESS;
static TEEC_Context context;
static TEEC_Session session;
static TEEC_Operation operation;
static TEEC_SharedMemory input;
static TEEC_SharedMemory output;
static bool contextInitialised;
static bool sessionOpen;
static bool iseSEGPOpen;
static uint32_t resultOrigin;
static std::mutex m;
int nq_node = -1;

static sp<IEsePowerManager> mHal;

static bool esepmInit() {
  if (mHal != nullptr) {
    return true;
  }

  mHal = IEsePowerManager::getService();
  return (mHal != nullptr);
}

struct notifyDeath : IBase {
  void notify() {
    ALOGD("Death notified");
  }
};

void closeNode() {
  if (close(nq_node)) {
    ALOGE("%s: eSE ioctl close failed : %s", __func__, strerror(errno));
  }

  nq_node = -1;
  return;
}

static int nativeeSEPowerOn () {
  int ret = -1;
  ALOGD("nativeeSEPowerON");
  sp<notifyDeath> callback = new notifyDeath();

  if (callback == nullptr) {
    ALOGE("NULL callback");
    return ret;
  }

  if (esepmInit()) {
    return (mHal->powerOn(callback));
  } else {
    ALOGE("mHal is null");
    return ret;
  }
}

static int nativeeSEPowerOff() {
  int ret = -1;
  ALOGD("nativeeSEPowerOff");

  if (esepmInit()) {
    return (mHal->powerOff() == 0);
  } else {
    ALOGE("mHal is null");
    return ret;
  }
}

int nativeeSEGetState() {
  int ret = -1;

  if (esepmInit()) {
    return (mHal->getState());
  } else {
    ALOGE("mHal is null");
    return ret;
  }
}

static TEEC_Result do_close() {

  TEEC_Result partialResult = result;

  if (partialResult != TEEC_SUCCESS) {
    ALOGE("ESE ERROR MESSAGE - let's close everything : 0x%08X", result);
  }

  operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE,
                                          TEEC_NONE);
  result = TEEC_InvokeCommand(&session, GPQESE_CMD_CLOSE, &operation,
                              &resultOrigin);

  if ((result != TEEC_SUCCESS) && (partialResult == TEEC_SUCCESS)) {
    // We only want to log the error if we're in a "normal close".
    ALOGE("ESE ERROR MESSAGE  0x%08X", result);
  }

  if (sessionOpen) {
    TEEC_CloseSession(&session);
  }

  if (contextInitialised) {
    TEEC_FinalizeContext(&context);
  }

  if (!nativeeSEPowerOff()) {
    ALOGE("Failure to power off the eSE");
  }

  iseSEGPOpen = false;
  sessionOpen = false;
  contextInitialised = false;
  return result;
}

TEEC_Result gpqese_close() {
  std::lock_guard<std::mutex> lock(m);
  return do_close();
}

TEEC_Result gpqese_open() {
  std::lock_guard<std::mutex> lock(m);

  if (contextInitialised && sessionOpen && iseSEGPOpen) {
    return TEEC_SUCCESS;
  }

  if (!contextInitialised) {
    /* Create the Context */
    result = TEEC_InitializeContext(teeName, &context);

    if (result != TEEC_SUCCESS) {
      ALOGE("TEEC_InitializeContext() ERROR MESSAGE  0x%08X", result);
      return result;
    }

    contextInitialised = true;
  }

  if (!nativeeSEPowerOn()) {
    ALOGE("eSE not powered on");
    return result;
  }

  if (!sessionOpen) {
    /* Open a session with the GP TA */
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE,
                                            TEEC_NONE);
    result = TEEC_OpenSession(&context, &session, &uuidEseGP, TEEC_LOGIN_USER, NULL,
                              &operation, &resultOrigin);

    if (result != TEEC_SUCCESS) {
      ALOGE("TEEC_OpenSession() failed : 0x%08X", result);
      return do_close();
    }

    sessionOpen = true;
  }

  if (!iseSEGPOpen) {
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE,
                                            TEEC_NONE);
    result = TEEC_InvokeCommand(&session, GPQESE_CMD_OPEN, &operation,
                                &resultOrigin);

    if (result != TEEC_SUCCESS) {
      ALOGE("GPQESE_CMD_OPEN failed : 0x%08X", result);
      return do_close();
    }

    iseSEGPOpen = true;
  }

  return result;
}

uint32_t gpqese_transceive_common(struct EseSgBuffer *tx, struct EseSgBuffer *rx,
                           TEEC_Result *result, bool isRawTransmit, uint32_t channelNum) {
  uint32_t ret = -1;
  uint32_t commandID;
  bool iseSEPower;
  std::lock_guard<std::mutex> lock(m);

  if (tx == nullptr) {
    ALOGE("tx buffer is null");
    return ret;
  } else if (rx == nullptr) {
    ALOGE("rx buffer is null");
    return ret;
  } else if (result == nullptr) {
    ALOGE("result is null");
    return ret;
  } else if (tx->len > GPQESE_MAX_CAPDU_SIZE) {
    ALOGE("APDU too long %6" PRIu32" max is %6" PRIu32"", tx->len,
        GPQESE_MAX_CAPDU_SIZE);
    return ret;
  }

  iseSEPower = nativeeSEGetState();
  if (!contextInitialised || !sessionOpen || !iseSEGPOpen || !iseSEPower) {
    ALOGE("Transceive failed : Context initialized : %d, Session open : %d, eSE open %d, eSEPowerState : %d",
          contextInitialised, sessionOpen, iseSEGPOpen, iseSEPower);
    return ret;
  }

  print_text("tApdu to be sent :", tx->c_base, tx->len);

  //INPUT MANAGEMENT
  if (isRawTransmit) {
    operation.paramTypes = TEEC_PARAM_TYPES(
                           TEEC_MEMREF_PARTIAL_INPUT,  // Input buffer
                           TEEC_MEMREF_PARTIAL_OUTPUT, // Output Buffer
                           TEEC_VALUE_INPUT,
                           TEEC_NONE);

    operation.params[2].value.a = channelNum;
    commandID = GPQESE_CMD_TRANSCEIVE_RAW;
  } else {
    operation.paramTypes = TEEC_PARAM_TYPES(
                           TEEC_MEMREF_PARTIAL_INPUT,  // Input buffer
                           TEEC_MEMREF_PARTIAL_OUTPUT, // Output Buffer
                           TEEC_NONE,
                           TEEC_NONE);
    commandID = GPQESE_CMD_TRANSCEIVE;
  }

  input.size = tx->len;
  input.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
  *result = TEEC_AllocateSharedMemory(&context, &input);

  if (*result != TEEC_SUCCESS) {
    ALOGE("AllocateSharedMemory failed() returned 0x%08X", *result);
    if (do_close() != TEEC_SUCCESS) {
      ALOGE("do_close() failed");
    }
    return ret;
  }

  memscpy(input.buffer, input.size, (void *) &tx->c_base[0], tx->len);
  operation.params[0].memref.parent = &input;
  operation.params[0].memref.offset = 0;
  operation.params[0].memref.size = tx->len;


  //OUTPUT MANAGEMENT
  output.size = GPQESE_MAX_RAPDU_SIZE;
  output.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
  *result = TEEC_AllocateSharedMemory(&context, &output);

  if (*result != TEEC_SUCCESS) {
    ALOGE("AllocateSharedMemory failed() returned 0x%08X", *result);
    TEEC_ReleaseSharedMemory(&input);
    if (do_close() != TEEC_SUCCESS) {
      ALOGE("do_close() failed");
    }
    return ret;
  }

  memset(output.buffer, 0x0, output.size);
  operation.params[1].memref.parent = &output;
  operation.params[1].memref.offset = 0;
  operation.params[1].memref.size = output.size;
  *result = TEEC_InvokeCommand(&session, commandID, &operation,
                               &resultOrigin);

  if (*result != TEEC_SUCCESS) {
    ALOGE("Transceive failed() returned 0x%08X", *result);
    TEEC_ReleaseSharedMemory(&input);
    TEEC_ReleaseSharedMemory(&output);
    if (*result == TEEC_ERROR_TARGET_DEAD) {
      if (do_close() != TEEC_SUCCESS) {
        ALOGE("do_close() failed");
      }
    }
    return ret;
  }

  uint32_t size_read = (uint32_t)operation.params[1].memref.size;
  memscpy(rx->base, rx->len, (uint8_t *)output.buffer, size_read);

  print_text("rApdu received :", rx->base, size_read);

  TEEC_ReleaseSharedMemory(&input);
  TEEC_ReleaseSharedMemory(&output);
  return size_read;
}

uint32_t gpqese_transceive(struct EseSgBuffer *tx, struct EseSgBuffer *rx,
                           TEEC_Result *result) {

  return gpqese_transceive_common(tx, rx, result, false, INVALID_CHANNEL_NUM);
}

int32_t gpqese_generic(GPQESE_Cmd_Generic_Tag reqType, struct EseSgBuffer *rx,
                           TEEC_Result *result) {

  int32_t ret = -1;
  std::lock_guard<std::mutex> lock(m);

  //check for valid reqType
  if ((reqType != GPQESE_CMD_GENERIC_TAG_GET_ATR) &&
      (reqType != GPQESE_CMD_GENERIC_TAG_INTERFACE_RESET) &&
      (reqType != GPQESE_CMD_GENERIC_TAG_HARD_RESET) &&
      (reqType != GPQESE_CMD_GENERIC_TAG_IFS)) {
    ALOGE("Invalid reqType for generic command");
    return ret;
  }

  operation.paramTypes = TEEC_PARAM_TYPES(
                           TEEC_VALUE_INPUT,  // Input value
                           TEEC_MEMREF_PARTIAL_OUTPUT, // Output Buffer
                           TEEC_NONE,
                           TEEC_NONE);
  //INPUT MANAGEMENT
  operation.params[0].value.a = reqType;

  //OUTPUT MANAGEMENT
  output.size = GPQESE_MAX_RAPDU_SIZE;
  output.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
  *result = TEEC_AllocateSharedMemory(&context, &output);

  if (*result != TEEC_SUCCESS) {
    ALOGE("AllocateSharedMemory failed() returned 0x%08X", *result);
    return ret;
  }

  memset(output.buffer, 0x0, output.size);
  operation.params[1].memref.parent = &output;
  operation.params[1].memref.offset = 0;
  operation.params[1].memref.size = output.size;
  *result = TEEC_InvokeCommand(&session, GPQESE_CMD_GENERIC, &operation,
                               &resultOrigin);

  if (*result != TEEC_SUCCESS) {
    ALOGE("Transceive failed() returned 0x%08X", *result);
    TEEC_ReleaseSharedMemory(&output);
    return ret;
  }
  uint32_t size_read = (uint32_t)operation.params[1].memref.size;
  memscpy(rx->base, rx->len, (uint8_t *)output.buffer, size_read);

  print_text("rApdu received :", rx->base, size_read);
  TEEC_ReleaseSharedMemory(&output);
  return size_read;
}
