/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef WAVIO_FORMAT_CHUNK_H_
#define WAVIO_FORMAT_CHUNK_H_

#include <cassert>
#include <iostream>
#include <cstdint>

#include "chunk.h"
#include "wav_format.h"
#include "read_stream.h"

namespace qti_hal_test
{
class FormatChunk : public Chunk
{
public:
    FormatChunk();
    ~FormatChunk();

    uint32_t SizeInRiffHeader() const;

    bool Deserialize(ReadStream* const read_stream,
        uint32_t ckSize,
        uint32_t* total_bytes_read);
    void Serialize(char * const target);

    void Initialize(WAVEFORMATEX const & format);

    uint16_t GetwFormatTag() const { return wFormatTag_; }
    uint16_t GetnChannels() const { return nChannels_; }
    uint32_t GetnSamplesPerSec() const { return nSamplesPerSec_; }
    uint32_t GetnAvgBytesPerSec() const { return nAvgBytesPerSec_; }
    uint16_t GetnBlockAlign() const { return nBlockAlign_; }
    uint16_t GetwBitsPerSample() const { return wBitsPerSample_; }
    uint16_t GetcbSize() const { return cbSize_; }

private:
    bool ExtractFormat(WAVEFORMATEX const & format);
    uint16_t wFormatTag_;
    uint16_t nChannels_;
    uint32_t nSamplesPerSec_;
    uint32_t nAvgBytesPerSec_;
    uint16_t nBlockAlign_;
    uint16_t wBitsPerSample_;
    uint16_t cbSize_;

    uint32_t extended_byte_size_;
    char* extended_bytes;
};
}

#endif // WAVIO_FORMAT_CHUNK_H_