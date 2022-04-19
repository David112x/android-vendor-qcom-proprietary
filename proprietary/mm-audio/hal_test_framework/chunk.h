/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef WAVIO_CHUNK_H_
#define WAVIO_CHUNK_H_

#include <cstring>  // For memcpy with C99

#include <cstdint>

#include "read_stream.h"

namespace qti_hal_test
{
class Chunk
{
public:
    Chunk();
    explicit Chunk(uint32_t ckId);
    virtual ~Chunk();

    bool Deserialize(ReadStream* const read_stream, uint32_t* total_bytes_read);
    virtual void Serialize(char * const target);

    virtual uint32_t SizeInRiffHeader() const;

   uint32_t get_ckID() const { return ckID_; }
   uint32_t get_ckSize() const { return ckSize_; }

protected:
    void SerializeFourCCAndSize(char ** current);
    void LoadTarget(char ** current, void* source, size_t size);

    uint16_t Deserializeuint16_tField(char const ** current) const;
    uint32_t Deserializeuint32_tField(char const ** current) const;

    uint16_t ConvertcharArrayToLittleEndianuint16_t(char const * const buffer) const;
    uint32_t ConvertcharArrayToLittleEndianuint32_t(char const * const buffer) const;

    uint32_t ckID_;  // Utilize the naming convention in the RIFF spec.
    uint32_t ckSize_;
};
}

#endif // WAVIO_WAV_READER_H_