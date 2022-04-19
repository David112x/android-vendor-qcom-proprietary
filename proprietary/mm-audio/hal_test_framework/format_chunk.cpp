/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "format_chunk.h"

namespace qti_hal_test
{
FormatChunk::FormatChunk() :
    Chunk(0x20746D66), // "fmt "
    wFormatTag_( 0 ),
    nChannels_( 0 ),
    nSamplesPerSec_( 0 ),
    nAvgBytesPerSec_( 0 ),
    nBlockAlign_( 0 ),
    wBitsPerSample_( 0 ),
    cbSize_( 0 )
{}

FormatChunk::~FormatChunk()
{}

uint32_t FormatChunk::SizeInRiffHeader() const
{
    return Chunk::SizeInRiffHeader() + ckSize_;
}

bool FormatChunk::Deserialize(ReadStream* const read_stream,
    uint32_t ckSize,
    uint32_t* total_bytes_read)
{
    ckSize_ = ckSize;
    const size_t buffer_size = ckSize_;
    char* buffer = new char[buffer_size];

    bool eof = false;
    uint64_t num_bytes_read = 0;

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

    char const * current = buffer;

    wFormatTag_ = Deserializeuint16_tField(&current);
    nChannels_ = Deserializeuint16_tField(&current);
    nSamplesPerSec_ = Deserializeuint32_tField(&current);
    nAvgBytesPerSec_ = Deserializeuint32_tField(&current);
    nBlockAlign_ = Deserializeuint16_tField(&current);
    wBitsPerSample_ = Deserializeuint16_tField(&current);
    cbSize_ = 0;        //  Force to zero.
    // Parameter is WAVEFORMATEX but for now only
    // supporting WAVFORMAT

    delete[] buffer;
    buffer = nullptr;

    return true;
}


void FormatChunk::Serialize(char * const target)
{
    char * current = target;

    Chunk::SerializeFourCCAndSize(&current);

    LoadTarget(&current , &wFormatTag_, sizeof(wFormatTag_));
    LoadTarget(&current , &nChannels_, sizeof(nChannels_));
    LoadTarget(&current , &nSamplesPerSec_, sizeof(nSamplesPerSec_));
    LoadTarget(&current , &nAvgBytesPerSec_, sizeof(nAvgBytesPerSec_));
    LoadTarget(&current , &nBlockAlign_, sizeof(nBlockAlign_));
    LoadTarget(&current , &wBitsPerSample_, sizeof(wBitsPerSample_));
    LoadTarget(&current , &cbSize_, sizeof(cbSize_));
#if 0
    LoadTarget(&current , &wValidBitsPerSample_, sizeof(wValidBitsPerSample_));
    LoadTarget(&current , &dwChannelMask_, sizeof(dwChannelMask_));
    LoadTarget(&current , &SubFormat_, sizeof(SubFormat_));
#endif
}

void FormatChunk::Initialize(WAVEFORMATEX const & format)
{
    ExtractFormat(format);

    ckSize_ = sizeof(wFormatTag_)+
        sizeof(nChannels_)+
        sizeof(nSamplesPerSec_)+
        sizeof(nAvgBytesPerSec_)+
        sizeof(nBlockAlign_)+
        sizeof(wBitsPerSample_)+
        sizeof(cbSize_)+
        cbSize_;
}

bool FormatChunk::ExtractFormat(WAVEFORMATEX const & format)
{
    wFormatTag_ = format.wFormatTag;
    nChannels_ = format.nChannels;
    nSamplesPerSec_ = format.nSamplesPerSec;
    nAvgBytesPerSec_ = format.nAvgBytesPerSec;
    nBlockAlign_ = format.nBlockAlign;
    wBitsPerSample_ = format.wBitsPerSample;
    cbSize_ = format.cbSize;

#if 0
    ValidBitsPerSample_ = format.Samples.wValidBitsPerSample;
    wChannelMask_ = format.dwChannelMask;
    SubFormat_ = format.SubFormat;

    extendedByteSize = cbSize - (sizeof(wValidBitsPerSample)+sizeof(dwChannelMask) +sizeof(SubFormat));
    if (extendedByteSize > 0)
    {
        return false;
    }
#endif
    return true;
}
}
