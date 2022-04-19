/***************************************************************************
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ***************************************************************************/
#ifndef __EZTUNE_INTERNAL_TYPES_H__
#define __EZTUNE_INTERNAL_TYPES_H__

#include "eztune_items_diag.h"

#define WAIT_TIME_MILLISECONDS 2000

typedef struct {
    //event_id is of EztuneNotify type and will be casted
    //appropriately. event_id is defined as uint32_t for cleaner
    //header seperation & inclusion
    uint32_t event_id;
    uint32_t size;
    void *data;
} ProcessThreadMessage;

typedef struct {
    uint32_t type;
    uint32_t payload_size;
    void *payload_ptr;
} InterfaceThreadMessage;

#if defined(__cplusplus)
extern "C" {
#endif

const uint32_t kEztuneScaledWidth = 320;
const uint32_t kEztuneScaledHeight = 240;
const uint32_t kEztuneScaledLumaSize = (320 * 240);
const uint32_t kEztuneScaledSize = (320 * 240 * 3) >> 1;
const uint8_t kEztuneMinFps = 15;

#if defined(__cplusplus)
}
#endif

#endif /* __EZTUNE_INTERNAL_TYPES_H__ */
