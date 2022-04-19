/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef WAVIO_UNUSED_CHUNK_H_
#define WAVIO_UNUSED_CHUNK_H_

#include "chunk.h"
#include "read_stream.h"

namespace qti_hal_test
{
class UnusedChunk : public Chunk
{
public:
    UnusedChunk();
    ~UnusedChunk();

    bool Skip(ReadStream* const read_stream,
        uint32_t ckSize,
        uint32_t* total_bytes_read);
};
}
#endif // WAVIO_UNUSED_CHUNK_H_