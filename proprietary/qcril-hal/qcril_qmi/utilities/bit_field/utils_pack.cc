/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <stdint.h>
#include "utilities/utils_common.h"


#ifndef MAX
#define  MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

///////////////////////////// Unpacking part ///////////////////////////////////////

uint8_t b_unpackb
(
    uint8_t* payload,
    uint32_t pos,
    uint8_t bitsLen
)
{
    if (bitsLen > 8) return 0;

    payload += (pos / 8);
    uint8_t bitsStart = pos % 8;
    uint8_t bitsLeft = 8 - bitsStart; // the bits left in current bytes, it is always > 0
    // clean up the left side value in current byte "Left""Extract part""Right"(
    // both "Left" and "Right" part could be empty) to "0s""Extract part""Right"
    uint8_t tmp_val = *payload & ((1 << bitsLeft) - 1);
    // right shift to eliminate the "Right" part value
    uint8_t value = tmp_val >> MAX(0, ((int)bitsLeft - (int)bitsLen));

    // handle the remaining bits in the next byte of payload if there is any
    if (bitsLen > bitsLeft) {
       uint8_t remainBits = bitsLen - bitsLeft; // it is a value always < 8
       uint8_t remainValue = (*(payload + 1) >> (8 - remainBits)) & ((1 << remainBits) - 1);
       value = (value << remainBits) + remainValue;
    }

    return value;
}

uint16_t b_unpackw
(
    uint8_t* payload,
    uint32_t pos,
    uint32_t bitsLen
)
{
    payload += (pos / 8);
    uint8_t bitsStart = pos % 8;
    uint8_t bitsLeft = 8 - bitsStart; // the bits left in current bytes, it is always > 0

    // clean up the left side value in current byte "Left""Extract part""Right"(
    // both "Left" and "Right" part could be empty) to "0s""Extract part""Right"
    uint8_t tmp_val = *payload & ((1 << bitsLeft) - 1);
    // right shift to eliminate the "Right" part value
    uint16_t value = tmp_val >> MAX(0, ((int)bitsLeft - (int)bitsLen));

    // handle the remaining bits in the next bytes of payload if there is any
    if (bitsLen > bitsLeft) {
        uint32_t remainBits = bitsLen - bitsLeft;
        // keep reading whole types if remaining bits is greater byte width 8
        while (remainBits >= 8) {
            value = (value << 8) | *(++payload);
            remainBits -= 8;
        }
        // remaining bits is less than 8, starting from pos 0 in a new byte
        if (remainBits) {
            uint8_t remainValue = *(++payload) >> (8 - remainBits);
            value = (value << remainBits) + remainValue;
        }
    }

    return value;
}
#if 0
void unpackFromPayload
(
    uint8_t* payload,
    uint32_t pos,
    uint32_t bitsLen,
    uint8_t* targetBuf,
    int32_t targetLen
)
{
    // reset target buf as 0
    memset(targetBuf, 0, targetLen);
    uint32_t remain_bits = bitsLen % 8;
    uint32_t total_bytes = remain_bits ? (bitsLen/8+1): (bitsLen/8);
    int index;

    if (!total_bytes) return;

    // The flow is
    // 1) Get the highest num of bits (which is a remainder: "total bits" mod 8)
    // 2) Get each bytes for the remaining if there is any
    if (!IS_BIG_ENDIAN()) {
        index = total_bytes - 1;
        if (remain_bits > 0) {
            targetBuf[index--] = b_unpackb(payload, pos, remain_bits);
            pos += remain_bits;
        }
        while (index > 0) {
            targetBuf[index--] = b_unpackb(payload, pos, 8);
            pos += 8;
        }
    } else {
        index = targetLen - total_bytes;
        if (index < 0) return;
        if (remain_bits > 0) {
            targetBuf[index++] = b_unpackb(payload, pos, remain_bits);
            pos += remain_bits;
        }
        while (index < targetLen) {
            targetBuf[index++] = b_unpackb(payload, pos, 8);
            pos += 8;
        }
    }
}

uint16_t b_unpackw
(
    uint8_t* payload,
    uint32_t pos,
    uint32_t bitsLen
)
{
    uint16_t value = 0;
    unpackFromPayload(payload, pos, bitsLen, (uint8_t*)&value, sizeof(value));
    return value;
}

uint32_t b_unpackd
(
    uint8_t* payload,
    uint32_t pos,
    uint32_t bitsLen
)
{
    uint32_t value = 0;
    unpackFromPayload(payload, pos, bitsLen, (uint8_t*)&value, sizeof(value));
    return value;
}

uint64_t unpackBitsToUint64
(
    uint8_t* payload,
    uint32_t pos,
    uint32_t bitsLen
)
{
    uint64_t value = 0;
    unpackFromPayload(payload, pos, bitsLen, (uint8_t*)&value, sizeof(value));
    return value;
}
#endif

///////////////////////////// Packing part ///////////////////////////////////////
void b_packb
(
    uint8_t srcByte,
    uint8_t* payload,
    uint32_t pos,
    uint32_t len
)
{
    // force srcByte a len bit value
    if (len > 8) return;
    srcByte &= ((1 << len) - 1);

    payload += (pos / 8);
    uint8_t bitsStart = pos % 8;
    uint8_t bitsLeft = 8 - bitsStart; // the bits left in current bytes, it is always > 0

    // assign the first part value in current byte in payload
    uint8_t part1Bits = (len > bitsLeft) ? bitsLeft : len;
    uint8_t part1Value = srcByte >> (len - part1Bits);
    *payload &= ((0xff << bitsLeft) | ((1 << (bitsLeft - part1Bits)) - 1));
    *payload = *payload | (part1Value << (bitsLeft - part1Bits)); // part1Bits always <= bitsLeft

    // assign the second part value if there is any
    if (len > bitsLeft) {
        uint8_t part2Bits = len - part1Bits;
        uint8_t part2Value = srcByte & ((1 << part2Bits) - 1);
        payload++;
        *payload &= ((1 << (8 - part2Bits)) - 1);
        *payload |= (part2Value << (8 - part2Bits));
    }
    return;
}

// this function will pad the highest bits as 0 if len is greater than 16
void b_packw
(
    uint16_t srcByte,
    uint8_t* payload,
    uint32_t pos,
    uint32_t len
)
{
    // force srcByte a len bit value
    srcByte &= ((1 << len) - 1);

    if (len <= 8) {
        b_packb((uint8_t)srcByte, payload, pos, len);
    } else {
        // the position of last bit
        uint8_t lastPos = (pos + len - 1) % 8;
        // the position of last byte
        payload += (pos + len - 1) / 8;
        // fill the last byte
        b_packb(srcByte & ((1 << (lastPos + 1)) - 1), payload--, 0, lastPos + 1);
        srcByte >>= (lastPos + 1);

        // fill the whole byte directly
        for (len -= (lastPos + 1); len >= 8; len -= 8) {
            *(payload--) = (uint8_t)srcByte;
            srcByte >>= 8;
        }

        // fill the remain part if there is any (highest bits at early pos)
        if (len > 0) {
            b_packb((uint8_t)srcByte, payload, pos, len);
        }

    }
}
