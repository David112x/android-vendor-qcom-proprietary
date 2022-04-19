/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

uint8_t b_unpackb
(
    uint8_t* payload,
    uint32_t pos,
    uint8_t bits
);


uint16_t b_unpackw
(
    uint8_t* payload,
    uint32_t pos,
    uint32_t bitsLen
);

void b_packb
(
    uint8_t srcByte,
    uint8_t* payload,
    uint32_t pos,
    uint32_t len
);


void b_packw
(
    uint16_t srcByte,
    uint8_t* payload,
    uint32_t pos,
    uint32_t len
);
