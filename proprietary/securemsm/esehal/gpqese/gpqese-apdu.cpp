/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "gpqese-apdu.h"

uint8_t static rApdu[GPQESE_MAX_RAPDU_SIZE] = {0x00};
static mChannelInfo_t channelsInfo[MAX_CHANNELS];


struct EseSgBuffer tx = {
  .c_base = {}, .len = 0,
};

struct EseSgBuffer rx = {
  .base = rApdu, .len = GPQESE_MAX_RAPDU_SIZE,
};

bool allChannelsClosed() {
  for (int i = 0; i < MAX_CHANNELS; i++) {
    if (channelsInfo[i].isOpened == true) {
      ALOGD("Channel number %d still open", i);
      return false;
    }
  }
  return true;
}

int ese_open()
{
  TEEC_Result result = TEEC_ERROR_GENERIC;
  const hidl_vec<uint8_t> aid;
  int rLen = 0;
  int cNumber = 0;

  result = gpqese_open();
  if (result != TEEC_SUCCESS) {
    ALOGE("gpqese_open failed");
    return STATUS_FAILURE;
  }

  //open basic channel for JCOP use case
  result = openChannel(aid, 0, &cNumber, rApdu, &rLen, true);
  if (result != TEEC_SUCCESS) {
    ALOGE("open basic channel failed");
    result = gpqese_close();
    if (result != TEEC_SUCCESS)
      ALOGE("gpqese_close failed");
    return STATUS_FAILURE;
  }
  return STATUS_SUCCESS;
}

int ese_close()
{
  TEEC_Result result = TEEC_ERROR_GENERIC;

  /*
   * close BasicChannel, it was opened in ese_open()
   * channel close will take care of gpqese_close as well
   */
  result = closeGPChannel(0);
  if (result != TEEC_SUCCESS) {
    ALOGE("close BasicChannel failed");
    return STATUS_FAILURE;
  }
  return STATUS_SUCCESS;
}

TEEC_Result ese_init()
{
  TEEC_Result result = TEEC_ERROR_GENERIC;

  result = gpqese_open();
  if (result != TEEC_SUCCESS) {
    ALOGE("gpqese_open failed");
    return result;
  }
  return result;
}

int resetJcopUpdate()
{
  TEEC_Result result = TEEC_ERROR_GENERIC;
  int32_t ret;

  ret = gpqese_generic(GPQESE_CMD_GENERIC_TAG_HARD_RESET, &rx, &result);
  if (result != TEEC_SUCCESS || ret < 0) {
    ALOGE("HARD_RESET failed");
    return STATUS_FAILURE;
  }
  return STATUS_SUCCESS;
}

void prepare_and_do_transceive(const uint8_t *tx_buff, int32_t tx_len, uint8_t *respApdu, int32_t rx_len, bool isRawTransmit, int32_t *rx_len_out)
{
  TEEC_Result result = TEEC_ERROR_GENERIC;
  uint32_t channelNum;

  tx.c_base = tx_buff;
  tx.len = tx_len;

  if (isRawTransmit)
    channelNum = BASIC_CHANNEL_NUM;
  else
    channelNum = INVALID_CHANNEL_NUM;

  *rx_len_out = gpqese_transceive_common(&tx, &rx, &result, isRawTransmit, channelNum);
  if ((*rx_len_out > 1) && (result == TEEC_SUCCESS)) {
    //rx_len - Max size of receive buffer
    if (*rx_len_out < rx_len)
      memscpy(respApdu, *rx_len_out, rApdu, GPQESE_MAX_RAPDU_SIZE);
    else
      memscpy(respApdu, rx_len, rApdu, GPQESE_MAX_RAPDU_SIZE);
  }
}

TEEC_Result openChannel(const hidl_vec<uint8_t> &aid, uint8_t p2, int *cNumber,
                        uint8_t *respApdu, int *rLen, bool isBasic) {

  uint8_t *tApdu = nullptr;
  int tApduSize = 0;
  TEEC_Result result = TEEC_ERROR_GENERIC;

  if (rLen == nullptr) {
    ALOGE("rLen is null");
    return result;
  } else if (respApdu == nullptr) {
    ALOGE("respApdu is null");
    return result;
  } else if (cNumber == nullptr) {
    ALOGE("cNumber is null");
    return result;
  }

  //Let's check if the eSE is powered ON, and the TA ready.
  result = gpqese_open();
  if (result != TEEC_SUCCESS) {
    return result;
  }

  if (!isBasic) {
    tx.c_base = MC_OPEN;
    tx.len  = sizeof(MC_OPEN);
    *rLen = gpqese_transceive(&tx, &rx, &result);
    if (result != TEEC_SUCCESS) {
      goto clean_up;
    }
  }

  if ((*rLen == 3) || isBasic) {
    if (((rApdu[1] == 0x90) && (rApdu[2] == 0x00)) || isBasic) {
      /* If AID is not present, Le would be part of SELECT itself and byte for Lc is not required */
      tApduSize = sizeof(SELECT) + aid.size() + ((aid.size() > 0) ? 1 : 0);
      tApdu = new (std::nothrow) uint8_t[tApduSize];
      if (NULL == tApdu) {
        ALOGE("Unable to create tApdu");
        result = TEEC_ERROR_OUT_OF_MEMORY;
        goto clean_up;
      }
      memscpy(tApdu, tApduSize, SELECT, sizeof(SELECT)); //Select command

      if (isBasic) {
        tApdu[0] = 0x00; //Channel Number
      } else {
        tApdu[0] = rApdu[0]; //Channel Number
      }

      tApdu[3] = p2; //p2
      tApdu[4] = aid.size(); //aid.size()
      memscpy(tApdu + 5, tApduSize-5, aid.data(), aid.size()); //AID
      tApdu[tApduSize - 1] = 0x00;
      tx.c_base = tApdu;
      tx.len = tApduSize;
      if ((tApdu[0] < 0) || (tApdu[0] >= MAX_CHANNELS)) {
        ALOGE("Invalid channel number :  %d", tApdu[0]);
        result = TEEC_ERROR_GENERIC;
        goto clean_up;
      }
      *cNumber = tApdu[0];
      channelsInfo[*cNumber].isOpened = true;
      *rLen = gpqese_transceive(&tx, &rx, &result);
      delete[] tApdu;
      if ((*rLen > 1) && (result == TEEC_SUCCESS)) {
        memscpy(respApdu, *rLen, rApdu, sizeof(rApdu));

        if (rApdu[*rLen - 2] == 0x69 && rApdu[*rLen - 1] == 0x85) {
          ALOGE("Condition of use not satisfied");
          return  TEEC_ERROR_ITEM_NOT_FOUND;
        } else if ((rApdu[*rLen - 2] == 0x6A && rApdu[*rLen - 1] == 0x82) ||
                   (rApdu[*rLen - 2] == 0x69 && rApdu[*rLen - 1] == 0x99)) {
          ALOGE("Applet can't be selected");
          return TEEC_ERROR_ITEM_NOT_FOUND;
        } else {
          memscpy(channelsInfo[*cNumber].aid, MAX_AID_SIZE, aid.data(), aid.size());
          return TEEC_SUCCESS;
        }
      } else if (result != TEEC_SUCCESS) {
          ALOGE("transceive failed");
          goto clean_up;
      } else {
        result = TEEC_ERROR_GENERIC;
        goto clean_up;
      }
    }
  } else if ((*rLen == 2) && (rApdu[0] == 0x6A && rApdu[1] == 0x81)) {
    result = TEEC_ERROR_OUT_OF_MEMORY; // No more channel available
    goto clean_up;
  } else {
    ALOGE("OpenChannel failed - ret = %d", *rLen);
    goto clean_up;
  }

  return result;
clean_up:
  gpqese_close();
  return result;
}

TEEC_Result transmitApdu (const hidl_vec<uint8_t> &data, uint8_t *respApdu,
                          int *rLen) {

  TEEC_Result result = TEEC_ERROR_GENERIC;
  if (respApdu == nullptr) {
    ALOGE("respApdu is null");
    return result;
  } else if (rLen == nullptr) {
    ALOGE("rLen is null");
    return result;
  }
  //Let's check if the eSE is powered ON, and the TA ready.
  result = gpqese_open();

  if (result != TEEC_SUCCESS) {
    return result;
  }

  tx.c_base = data.data();
  tx.len = data.size();
  *rLen = gpqese_transceive(&tx, &rx, &result);

  if (TEEC_SUCCESS != result) {
    ALOGE("transmit failed !");
    return result;
  }

  if (*rLen > 1) {  // minimum rApdu expected is SW
    memscpy(respApdu, *rLen, rApdu, sizeof(rApdu));

    if (memsApducmp(SELECT, sizeof(SELECT), tx.c_base,
                tx.len)) { //check for close basic channel cmd.
      ALOGD("Close basic channel detected");
      channelsInfo[0].isOpened = false;

      if (allChannelsClosed()) {
        return gpqese_close();
      }
    }
  }

  return result;
}

TEEC_Result closeGPChannel(uint8_t channelNumber) {
  uint8_t *tApdu = nullptr;
  TEEC_Result result = TEEC_ERROR_GENERIC;
  int tApduSize = 0;
  int ret = -1;
  if ((channelNumber < 0) || (channelNumber >= MAX_CHANNELS)) {
    ALOGE("Channel number not consistent : %d", channelNumber);
    return result;
  }
  if (channelNumber == 0) {
    tApduSize = sizeof(SELECT);
    tApdu = new (std::nothrow) uint8_t[tApduSize];
    if (NULL == tApdu) {
      ALOGE("Unable to create tApdu");
      return TEEC_ERROR_OUT_OF_MEMORY;
    }
    memscpy(tApdu, tApduSize, SELECT, tApduSize); //Empty Select command
  } else {
    tApduSize = sizeof(MC_CLOSE);
    tApdu = new (std::nothrow) uint8_t[tApduSize];
    if (NULL == tApdu) {
      ALOGE("Unable to create tApdu");
      return TEEC_ERROR_OUT_OF_MEMORY;
    }
    memscpy(tApdu, tApduSize, MC_CLOSE, tApduSize);
    tApdu[0] = channelNumber;
    tApdu[3] = channelNumber;
  }

  tx.c_base = tApdu;
  tx.len  = tApduSize;
  ret = gpqese_transceive(&tx, &rx, &result);
  delete[] tApdu;
  if ((ret < 1) || (result != TEEC_SUCCESS)) {
    ALOGE("Close Channel failed ");
    return result;
  }
  if ((result == TEEC_SUCCESS) && ((rApdu[ret - 2] == 0x90)
                                   && (rApdu[ret - 1] == 0x00))) {
    channelsInfo[channelNumber].isOpened = false;

    if (allChannelsClosed()) {
      if (gpqese_close() == TEEC_SUCCESS) {
        return result;
      }
    }

    return result;
  }

  ALOGE("Close Channel failed");
  return result;
}
