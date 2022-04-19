/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef WAVIO_RIFF_WAVE_SEMI_CHUNK_H_
#define WAVIO_RIFF_WAVE_SEMI_CHUNK_H_

#include "chunk.h"
#include "read_stream.h"

namespace qti_hal_test
{
class RiffWaveSemiChunk : public Chunk
{
public:
    RiffWaveSemiChunk();
    ~RiffWaveSemiChunk();

    uint32_t SizeInRiffHeader() const;
    void Initialize(uint32_t ckSize);

    bool Deserialize(ReadStream* const read_stream,
        uint32_t ckSize,
        uint32_t* total_bytes_read);
    void Serialize(char * const target);
private:
    uint32_t WAVEID_;
};
}

#endif // WAVIO_RIFF_WAVE_SEMI_CHUNK_H_