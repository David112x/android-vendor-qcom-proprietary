/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef WAVIO_DATA_SEMI_CHUNK_H_
#define WAVIO_DATA_SEMI_CHUNK_H_

#include "chunk.h"

namespace qti_hal_test
{
class DataSemiChunk : public Chunk
{
public:
    DataSemiChunk();
    ~DataSemiChunk();

    uint32_t SizeInRiffHeader() const;
    void Initialize(uint32_t ckSize);

    bool Deserialize(uint32_t ckSize);
    void Serialize(char * const target);
};
}

#endif // WAVIO_DATA_SEMI_CHUNK_H_