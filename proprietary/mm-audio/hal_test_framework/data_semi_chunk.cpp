/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "data_semi_chunk.h"

namespace qti_hal_test
{
DataSemiChunk::DataSemiChunk() :
    Chunk(0x61746164)  // "data"
{}

DataSemiChunk::~DataSemiChunk()
{}

uint32_t DataSemiChunk::SizeInRiffHeader() const
{
    return Chunk::SizeInRiffHeader();
}

void DataSemiChunk::Initialize(uint32_t ckSize)
{
    ckSize_ = ckSize;
}

bool DataSemiChunk::Deserialize(uint32_t ckSize)
{
    ckSize_ = ckSize;
    return true;
}

void DataSemiChunk::Serialize(char * const target)
{
    char * current = target;

    Chunk::SerializeFourCCAndSize(&current);
}
}