/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __GPQESE_BE_H_
#define __GPQESE_BE_H_
#endif

#include <mutex>
#include <TEE_client_api.h>
#include <sys/ioctl.h>
#include <vendor/qti/esepowermanager/1.0/IEsePowerManager.h>
#include <EsePowerManager.h>
#include <inttypes.h>
#include "gpqese-utils.h"

#define GPQESE_CMD_TRANSCEIVE     0x10000000
#define GPQESE_CMD_OPEN           0x10000001
#define GPQESE_CMD_CLOSE          0x10000002
#define GPQESE_CMD_TRANSCEIVE_RAW 0x10000003
#define GPQESE_CMD_GENERIC        0x20000000 // @see GPQESE_Cmd_Generic_Tag

#define INVALID_CHANNEL_NUM       -1
#define BASIC_CHANNEL_NUM         0

#define GPQESE_MAX_CAPDU_SIZE 65546
#define GPQESE_MAX_RAPDU_SIZE 65539

#define nfc_dev_node        "/dev/nq-nci"
#define ESE_SET_PWR         _IOW(0xE9, 0x02, unsigned int)
#define ESE_GET_PWR         _IOR(0xE9, 0x03, unsigned int)

typedef enum
{
  GPQESE_CMD_GENERIC_TAG_GET_ATR         = 0x00,
  GPQESE_CMD_GENERIC_TAG_INTERFACE_RESET = 0x01,
  GPQESE_CMD_GENERIC_TAG_HARD_RESET      = 0x02,
  GPQESE_CMD_GENERIC_TAG_IFS             = 0x03,
} GPQESE_Cmd_Generic_Tag;

static const char *teeName = NULL;
static const TEEC_UUID uuidEseGP = {
  0x32552B22, 0x89FE, 0x42B4,
  { 0x8A, 0x45, 0xA0, 0xC4, 0xE2, 0xDB, 0x03, 0x26 }
};

/* Scatter-Gather Buffer */
struct EseSgBuffer {
  union {
    uint8_t *base;
    const uint8_t *c_base;
  };
  uint32_t len;
};

int nativeeSEGetState();
int32_t gpqese_generic(GPQESE_Cmd_Generic_Tag reqType, struct EseSgBuffer *rx,
                           TEEC_Result *result);
uint32_t gpqese_transceive(struct EseSgBuffer *tx, struct EseSgBuffer *rx,
                           TEEC_Result *result);
uint32_t gpqese_transceive_common(struct EseSgBuffer *tx, struct EseSgBuffer *rx,
                           TEEC_Result *result, bool isRawTransmit, uint32_t channelNum);
TEEC_Result gpqese_open();
TEEC_Result gpqese_close();
