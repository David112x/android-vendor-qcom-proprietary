/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef WAVFORMAT_H_
#define WAVFORMAT_H_

#include <cstdint>
#include <iostream>

namespace qti_hal_test
{
// Modified from LA_UM_0_0\vendor\qcom\proprietary\mm-parser-noship\ASFParserLib\inc\wmftypes.h
typedef struct tWAVEFORMATEX
{
    uint16_t    wFormatTag;         // format type
    uint16_t    nChannels;          // number of channels (i.e. mono, stereo...)
    uint32_t    nSamplesPerSec;     // sample rate
    uint32_t    nAvgBytesPerSec;    // for buffer estimation
    uint16_t    nBlockAlign;        // block size of data
    uint16_t    wBitsPerSample;     // number of bits per sample of mono data
    uint16_t    cbSize;             // the count in bytes of the size of

} WAVEFORMATEX;

#if 0 // WAVEFORMATEXTENSIBLE is not currently supported
typedef struct tWAVEFORMATEXTENSIBLE
{
    WAVEFORMATEX Format;             // The traditional wave file header

    union
    {
        int16_t wValidBitsPerSample; // bits of precision
        int16_t wSamplesPerBlock;    // valid if wBitsPerSample==0
        int16_t wReserved;           // If neither applies, set to zero
    } Samples;

    int32_t    dwChannelMask;        // which channels are present in stream

    char SubFormat[16];              // specialization
} WAVEFORMATEXTENSIBLE;
#endif

void DisplayWAVEFORMATEX(WAVEFORMATEX const & format);
void CreateWAVEFORMATEX(uint32_t nSamplesPerSec,    // TO DO: Rename
    uint16_t nChannels,
    uint16_t wBitsPerSample,
    WAVEFORMATEX* format);
}

#endif // WAVFORMAT_H_
