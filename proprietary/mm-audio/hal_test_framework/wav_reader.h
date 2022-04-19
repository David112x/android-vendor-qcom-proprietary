/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef WAVIO_WAV_READER_H_
#define WAVIO_WAV_READER_H_

#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "chunk.h"
#include "data_semi_chunk.h"
#include "format_chunk.h"
#include "logger.h"
#include "read_stream.h"
#include "source.h"
#include "riff_wave_semi_chunk.h"
#include "unused_chunk.h"
#include "wav_format.h"

namespace qti_hal_test
{
class WavReader : public Source
{
public:
    WavReader();
    explicit WavReader(std::string const & file_name);
    ~WavReader();

    // NOTE: For now we're only supporting WAVEFORMAT instead of WAVEFORMATEX.
    void Open(std::string const & file_name);
    void Open(std::string const & file_name, WAVEFORMATEX* const format);
    void Open(char const * const file_name, WAVEFORMATEX* const format);

    WavReader(WavReader const & rhs) = delete;
    WavReader & operator=(WavReader const & rhs) = delete;

    WavReader(WavReader && rhs) noexcept = delete;
    WavReader & operator=(WavReader && rhs) noexcept = delete;

    void Read(char * const sample_bytes,
              uint64_t num_bytes_to_read,
              uint64_t & num_bytes_read,
              bool & eof) override;

    void Close();

private:
    enum State { kInit, kRiff, kFmt, kData, kUnrecognized, kDone, kError };

    void GetFormatParameters(std::string const & file_name,
                             uint32_t & sample_rate,
                             uint16_t & num_channels,
                             uint16_t & bits_per_sample);

    bool DeserializeChunks(WAVEFORMATEX* const format);
    void InitializeFormat(WAVEFORMATEX* const format);

    uint32_t data_chunk_offset_;
    uint32_t data_chunk_size_;

    ReadStream read_stream_;

    RiffWaveSemiChunk riff_wave_semi_chunk_;
    FormatChunk       format_chunk_;
    DataSemiChunk     data_semi_chunk_;
    UnusedChunk       unused_chunk_;
};
}

#endif // WAVIO_WAV_READER_H_