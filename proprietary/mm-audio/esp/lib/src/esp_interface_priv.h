/*============================================================================
* Copyright (c) 2018 Qualcomm Technologies, Inc.                       *
* All Rights Reserved.                                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc.                 *
* ===========================================================================*/
#ifndef _ESP_INTERFACE_PRIV_H_
#define _ESP_INTERFACE_PRIV_H_

/**
 * @file esp_interface_priv.h
 *
 * This header file contains private definitions for the ESP
   algorithm interface.
 */

#include <stdint.h>
#include <stdio.h>

#include "esp_api.h"
#include "mutex_cond.h"
#include "task_queue.h"

#ifdef FEATURE_ESP
#include "./VAD_Features/VAD_class.h"
#include "./DA_Metrics/FrameEnergyComputing.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_RELEASE_VERSION     0x10000000  // 1.0_0.0.0

//Allocating for 80ms data at 16k sampling rate for circular buffer
#define CIRC_BUF_SIZE (ESPIN_FRAME_SIZE*4)

/* Align an offset to multiple of 8 (useful for aligning byte offsets
    to 64-bit boundary */
#define ALIGN8(o)         (((o)+7)&(~7))

typedef struct esp_lib_t {
    int16 pInCirPCMBuf[CIRC_BUF_SIZE];
    int16 pInbuf[ESPIN_FRAME_SIZE];
    int16 readIndex;
    int16 writeIndex;
    uint16 unReadSamples;
    bool GVAD;
    int64 currentFrameEnergy;
    uint32 frameNum;
    int16 frameSize;
    VADClass* vadObjPtr;
    FrameEnergyClass* frameEnergyComputeObjPtr;
    esp_energy_levels_t enrgyLevels;
#if defined(__arm__)
    task_queue_obj *pTaskQueue;
#endif
    DECLARE_MUTEX(readMtx);
    DECLARE_COND_VAR(readCond);
} esp_lib_t;

#if defined(__arm__)
static void esp_process_thread_fn(void *handle);
#endif

void esp_process_thread(void *handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //FEATURE_ESP

#endif /* _ESP_INTERFACE_PRIV_H_ */
