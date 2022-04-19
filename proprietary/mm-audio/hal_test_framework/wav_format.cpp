/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "wav_format.h"

namespace qti_hal_test
{
void DisplayWAVEFORMATEX(WAVEFORMATEX const & format)
{
    std::cout << "wFormatTag     : " << format.wFormatTag << std::endl;
    std::cout << "nChannels      : " << format.nChannels << std::endl;
    std::cout << "nSamplesPerSec : " << format.nSamplesPerSec << std::endl;
    std::cout << "nAvgBytesPerSec: " << format.nAvgBytesPerSec << std::endl;
    std::cout << "nBlockAlign    : " << format.nBlockAlign << std::endl;
    std::cout << "wBitsPerSample : " << format.wBitsPerSample << std::endl;
    std::cout << "cbSize         : " << format.cbSize << std::endl;
}

void CreateWAVEFORMATEX(uint32_t nSamplesPerSec,
    uint16_t nChannels,
    uint16_t wBitsPerSample,
    WAVEFORMATEX* format)
{
    format->wFormatTag = 1;
    format->nChannels = nChannels;
    format->nSamplesPerSec = nSamplesPerSec;
    format->wBitsPerSample = wBitsPerSample;
    format->nBlockAlign = format->nChannels * (format->wBitsPerSample / 8);
    format->nAvgBytesPerSec = format->nBlockAlign * format->nSamplesPerSec;

    format->cbSize = 0;
}
}