/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "unused_chunk.h"

namespace qti_hal_test
{
UnusedChunk::UnusedChunk()
{}

UnusedChunk::~UnusedChunk()
{}

bool UnusedChunk::Skip(ReadStream* const read_stream,
    uint32_t ckSize,
    uint32_t* total_bytes_read)
{
    read_stream->Seek(ckSize);

    *total_bytes_read += ckSize;

    return true;
}
}