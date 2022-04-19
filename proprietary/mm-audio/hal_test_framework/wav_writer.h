/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef WAVIO_WAV_WRITER_H_
#define WAVIO_WAV_WRITER_H_

#include <cstdint>
#include <stdexcept>
#include <string>

#include "data_semi_chunk.h"
#include "format_chunk.h"
#include "logger.h"
#include "riff_wave_semi_chunk.h"
#include "sink.h"
#include "wav_format.h"
#include "write_stream.h"

namespace qti_hal_test
{

class WavWriter : public Sink
{
public:
    WavWriter();
    WavWriter(std::string const & file_name,
              uint32_t sample_rate,
              uint16_t num_channels,
              uint16_t bits_per_sample);

    ~WavWriter();

    WavWriter(WavWriter const & rhs) = delete;
    WavWriter & operator=(WavWriter const & rhs) = delete;

    WavWriter(WavWriter && rhs) noexcept = delete;
    WavWriter & operator=(WavWriter && rhs) noexcept = delete;

    void Open(std::string const & file_name, WAVEFORMATEX const & format);

    void Open(std::string const & file_name,
              uint32_t sample_rate,
              uint16_t num_channels,
              uint16_t bits_per_sample);

    void Write(char const * const sample_bytes,
               uint64_t num_bytes_to_write) override;

    void Close();

private:
    uint32_t RiffHeaderSize() const;
    void AdjustDataChunkByteParity();

    void SetChunkSizes();
    void SerializeChunks();
    void SerializeChunk(Chunk * const chunk);

    WAVEFORMATEX format_;

    RiffWaveSemiChunk riff_wave_semi_chunk_;
    FormatChunk       format_chunk_;
    DataSemiChunk     data_semi_chunk_;

    uint64_t total_data_bytes_written_;
    uint64_t num_parity_bytes_written_;

    WriteStream write_stream_;
};
}

#endif // WAVIO_WAV_WRITER_H_