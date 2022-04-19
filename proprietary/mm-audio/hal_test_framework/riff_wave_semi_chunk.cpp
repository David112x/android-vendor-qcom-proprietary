/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "riff_wave_semi_chunk.h"

namespace qti_hal_test
{
RiffWaveSemiChunk::RiffWaveSemiChunk() :
Chunk( 0x46464952 ), // "RIFF"
WAVEID_( 0x45564157 ) // "WAVE"
{
}

RiffWaveSemiChunk::~RiffWaveSemiChunk()
{
}

uint32_t RiffWaveSemiChunk::SizeInRiffHeader() const
{
    return Chunk::SizeInRiffHeader() + sizeof(WAVEID_);
}

void RiffWaveSemiChunk::Initialize(uint32_t ckSize)
{
    ckSize_ = sizeof(WAVEID_) + ckSize;
}

bool RiffWaveSemiChunk::Deserialize(ReadStream* const read_stream,
                                    uint32_t ckSize,
                                    uint32_t* total_bytes_read)
{
    uint64_t num_bytes_read = 0;

    const size_t buffer_size = sizeof(uint32_t);
    char buffer[buffer_size];

    ckSize_ = ckSize;

    bool eof = false;

    read_stream->Read(buffer, buffer_size, num_bytes_read, eof);

    if (eof)   // WAVEID
    {
        return false;
    }

    if (num_bytes_read != buffer_size)
    {
        return false;
    }

    *total_bytes_read += buffer_size;

    uint32_t riff_form_type = ConvertcharArrayToLittleEndianuint32_t(buffer);
    if (riff_form_type != 0x45564157)
    {
        return false;   // Not a WAVE file
    }

    return true;
}

void RiffWaveSemiChunk::Serialize(char * const target)
{
    char * current = target;

    Chunk::SerializeFourCCAndSize(&current);

    LoadTarget(&current, &WAVEID_, sizeof(WAVEID_));
}

}