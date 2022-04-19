/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "chunk.h"

namespace qti_hal_test
{
Chunk::Chunk():
    ckID_( 0 ),
    ckSize_ ( 0 )
{
}

Chunk::Chunk(uint32_t ckId) :
    ckID_( ckId ),
    ckSize_( 0 )
{
}

Chunk::~Chunk()
{
}

bool Chunk::Deserialize(ReadStream * const read_stream, uint32_t * total_bytes_read)
{
    uint64_t num_bytes_read = 0;

    const size_t buffer_size = sizeof(uint32_t);
    char buffer[buffer_size];

    bool eof = false;

    read_stream->Read(buffer, buffer_size, num_bytes_read, eof);

    if (eof)
    {
        return false;
    }

    if (num_bytes_read != buffer_size)
    {
        return false;
    }

    *total_bytes_read += buffer_size;

    ckID_ = ConvertcharArrayToLittleEndianuint32_t(buffer);

    read_stream->Read(buffer, buffer_size, num_bytes_read, eof);

    if (eof)
    {
        return false;
    }

    if (num_bytes_read != buffer_size)
    {
        return false;
    }

    *total_bytes_read += buffer_size;

    ckSize_ = ConvertcharArrayToLittleEndianuint32_t(buffer);

    return true;
}

void Chunk::Serialize(char * const target)
{
    // Assign to avoid compile error regarding unused parameters
    //char * const temp = target;
}

uint32_t Chunk::SizeInRiffHeader() const
{
    return sizeof(ckID_)+sizeof(ckSize_);
}

void Chunk::SerializeFourCCAndSize(char ** current)
{
    memcpy(*current, &ckID_, sizeof(ckID_));

#if 0
    std::cout << "SerializeFourCCAndSize wrote ckID_  (" << sizeof(ckID_) << ") bytes";
    for (size_t i = 0; i < sizeof(ckID_); ++i)
    {
        std::cout << (*current)[i];
    }
    std::cout << std::endl;
#endif

    *current += sizeof(ckID_);

    memcpy(*current, &ckSize_, sizeof(ckSize_));

#if 0
    std::cout << "SerializeFourCCAndSize wrote ckSize_ (" << sizeof(ckSize_) << ") bytes";
    for (size_t i = 0; i < sizeof(ckSize_); ++i)
    {
        std::cout << (*current)[i];
    }
    std::cout << std::endl;
#endif

    *current += sizeof(ckSize_);
}

void Chunk::LoadTarget(char ** current, void* source, size_t size)
{
    memcpy(*current, source, size);
    *current += size;
}

uint16_t Chunk::Deserializeuint16_tField(char const ** current) const
{
    uint16_t field = ConvertcharArrayToLittleEndianuint16_t(*current);
    *current += sizeof(uint16_t);
    return field;
}

uint32_t Chunk::Deserializeuint32_tField(char const ** current) const
{
    uint32_t field = ConvertcharArrayToLittleEndianuint32_t(*current);
    *current += sizeof(uint32_t);
    return field;
}

uint16_t Chunk::ConvertcharArrayToLittleEndianuint16_t(char const * const buffer) const
{
    return static_cast<int16_t>((buffer[0]) | (buffer[1] << 8));
}

uint32_t Chunk::ConvertcharArrayToLittleEndianuint32_t(char const * const buffer) const
{
    return static_cast<int32_t>((buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24));
}
}
