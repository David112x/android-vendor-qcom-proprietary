/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __GPQESE_APDU_H_
#define __GPQESE_APDU_H_
#endif

#include "gpqese-be.h"
#include <utils/Log.h>
#define MAX_CHANNELS    5
#define MAX_AID_SIZE    16
#define STATUS_SUCCESS  0
#define STATUS_FAILURE  1

using ::android::hardware::hidl_vec;

static const uint8_t MC_OPEN[] = {0x00, 0x70, 0x00, 0x00, 0x01};
static const uint8_t MC_CLOSE[] = {0x01, 0x70, 0x80, 0x01};
static const uint8_t SELECT[] =  {0x00, 0xA4, 0x04, 0x00, 0x00};

typedef struct mChannelInfo {
  bool isOpened = false;
  uint8_t aid[MAX_AID_SIZE] = {0x00};
} mChannelInfo_t;

TEEC_Result ese_init();
TEEC_Result openChannel(const hidl_vec<uint8_t> &aid, uint8_t p2, int *cNumber,
                        uint8_t *respApdu, int *rLen, bool isBasic);
TEEC_Result transmitApdu(const hidl_vec<uint8_t> &data, uint8_t *respApdu,
                         int *rLen);
TEEC_Result closeGPChannel(uint8_t channelNumber);
