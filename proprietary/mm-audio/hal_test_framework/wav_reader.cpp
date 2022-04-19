/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "wav_reader.h"

namespace qti_hal_test
{
WavReader::WavReader() :
    data_chunk_offset_( 0 ),
    data_chunk_size_( 0 )
{
    LOG_ENTRY;
}

WavReader::WavReader(std::string const & file_name) :
    data_chunk_offset_(0),
    data_chunk_size_(0)
{
    LOG_ENTRY;

    WAVEFORMATEX format;
    Open(file_name.c_str(), &format);
}

WavReader::~WavReader()
{
    LOG_ENTRY;
    try
    {
        Close();
    }
    catch (std::exception const & e)
    {
        std::cout << "Could not close WavReader" << std::endl;
    }
}

void WavReader::Open(std::string const & file_name)
{
    LOG_ENTRY;

    WAVEFORMATEX format;
    Open(file_name.c_str(), &format);
}

void WavReader::Open(std::string const & file_name, WAVEFORMATEX* const format)
{
    LOG_ENTRY;

    Open(file_name.c_str(), format);
}

void WavReader::Open(char const * const file_name, WAVEFORMATEX* const format)
{
    LOG_ENTRY;

    read_stream_.Open(file_name);

    try
    {
        DeserializeChunks(format);

        read_stream_.SeekAbs(data_chunk_offset_);
    }
    catch(std::exception const & e)
    {
        read_stream_.Close();
        throw;
    }

    InitializeFormat(format);
}

void WavReader::GetFormatParameters(std::string const & file_name,
                                    uint32_t & sample_rate,
                                    uint16_t & num_channels,
                                    uint16_t & bits_per_sample)
{
    WAVEFORMATEX format;

    Open(file_name.c_str(), &format);

    sample_rate = format.nSamplesPerSec;
    num_channels = format.nChannels;
    bits_per_sample = format.wBitsPerSample;

    Close();
}

void WavReader::Read(char* const sample_bytes,
                     uint64_t num_bytes_to_read,
                     uint64_t & num_bytes_read,
                     bool & eof)
{
    read_stream_.Read(sample_bytes, num_bytes_to_read, num_bytes_read, eof);
}

void WavReader::Close()
{
    LOG_ENTRY;

    read_stream_.Close();
}

bool WavReader::DeserializeChunks(WAVEFORMATEX* const format)
{
    // Assign the parameter to a temp for now until I can determine how to push a command to ignore
    // the associated "unused parameter" compiler warning, or refactor.
    WAVEFORMATEX* const tmp_format = format;

    Chunk current_chunk;

    State current = kInit;

    bool chunksRemaining = true;
    while (chunksRemaining)
    {
        State next = kError;
        switch (current)
        {
        case kInit:

            if (!current_chunk.Deserialize(&read_stream_, &data_chunk_offset_))
            {
                std::cout << "WavReader::DeserializeChunks: kInit current_chunk parse error" << std::endl;
                return false;
            }

            if (current_chunk.get_ckID() == riff_wave_semi_chunk_.get_ckID())
            {
                next = kRiff;
            }
            else
            {
                next = kError;
            }

            break;

        case kRiff:

            if (!riff_wave_semi_chunk_.Deserialize(&read_stream_, current_chunk.get_ckSize(), &data_chunk_offset_))
            {
                std::cout << "WavReader::DeserializeChunks: kRiff riff_wave_semi_chunk_ parse error" << std::endl;
                return false;
            }

            if (!current_chunk.Deserialize(&read_stream_, &data_chunk_offset_))
            {
                std::cout << "WavReader::DeserializeChunks: kRiff current_chunk parse error" << std::endl;
                return false;
            }

            if (current_chunk.get_ckID() == format_chunk_.get_ckID())
            {
                next = kFmt;
            }
            else
            {
                next = kError;
            }

            break;

        case kFmt:

            if (!format_chunk_.Deserialize(&read_stream_, current_chunk.get_ckSize(), &data_chunk_offset_))
            {
                std::cout << "WavReader::DeserializeChunks: kFmt format_chunk_ parse error" << std::endl;
                return false;
            }

            if (!current_chunk.Deserialize(&read_stream_, &data_chunk_offset_))
            {
                std::cout << "WavReader::DeserializeChunks: kFmt current_chunk parse error" << std::endl;
                return false;
            }

            if (current_chunk.get_ckID() == data_semi_chunk_.get_ckID())
            {
                next = kData;
            }
            else
            {
                next = kUnrecognized;
            }

            break;

        case kData:

            if (!data_semi_chunk_.Deserialize(current_chunk.get_ckSize()))
            {
                std::cout << "WavReader::DeserializeChunks: kData data_semi_chunk_ parse error" << std::endl;
                return false;
            }


            next = kDone;

            break;

        case kUnrecognized:

            if (!unused_chunk_.Skip(&read_stream_, current_chunk.get_ckSize(), &data_chunk_offset_))
            {
                std::cout << "WavReader::DeserializeChunks: kUnrecognized unused_chunk_ parse error" << std::endl;
                return false;
            }

            if (!current_chunk.Deserialize(&read_stream_, &data_chunk_offset_))
            {
                std::cout << "WavReader::DeserializeChunks: kUnrecognized current_chunk parse error" << std::endl;
                return false;
            }

            if (current_chunk.get_ckID() == data_semi_chunk_.get_ckID())
            {
                next = kData;
            }
            else
            {
                next = kUnrecognized;
            }

            break;

        case kDone:
            chunksRemaining = false;
            break;

        case kError:
            std::cout << "WavReader::DeserializeChunks: kError parse error" << std::endl;
            return false;
        }

        current = next;
    }

    return true;
}

void WavReader::InitializeFormat(WAVEFORMATEX* const format)
{
    format->wFormatTag = format_chunk_.GetwFormatTag();
    format->nChannels = format_chunk_.GetnChannels();
    format->nSamplesPerSec = format_chunk_.GetnSamplesPerSec();
    format->nAvgBytesPerSec = format_chunk_.GetnAvgBytesPerSec();
    format->nBlockAlign = format_chunk_.GetnBlockAlign();
    format->wBitsPerSample = format_chunk_.GetwBitsPerSample();
    format->cbSize = format_chunk_.GetcbSize();
}
}
