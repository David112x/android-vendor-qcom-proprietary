/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "wav_writer.h"

namespace qti_hal_test
{
WavWriter::WavWriter() :
total_data_bytes_written_{ 0 },
num_parity_bytes_written_{ 0 }
{
    LOG_ENTRY;
}

WavWriter::WavWriter(std::string const & file_name,
    uint32_t sample_rate,
    uint16_t num_channels,
    uint16_t bits_per_sample) :
total_data_bytes_written_{ 0 },
num_parity_bytes_written_{ 0 }
{
    LOG_ENTRY;

    CreateWAVEFORMATEX(sample_rate,
        num_channels,
        bits_per_sample,
        &format_);

    ///DisplayWAVEFORMATEX(format_);

    Open(file_name, format_);
}

void WavWriter::Open(std::string const & file_name,
    uint32_t sample_rate,
    uint16_t num_channels,
    uint16_t bits_per_sample)
{
    LOG_ENTRY;

    CreateWAVEFORMATEX(sample_rate,
        num_channels,
        bits_per_sample,
        &format_);

    //DisplayWAVEFORMATEX(format_);

    Open(file_name, format_);
}

WavWriter::~WavWriter()
{
    LOG_ENTRY;

    try
    {
        Close();
    }
    catch (std::exception const & e)
    {
        // std::cout << "Could not close WavWriter" << std::endl;
    }
}

// Strategy:  Open the file, compute the size of a generalization of the RIFF,
// format, and data headers.  We're not really writing separate chunks.
// We're writing a byte by byte map of the three chunks and appending the
// data bytes to the end of the file.   Once the generalized header is written,
// seek the file pointer out by the size of the generalized header.Then write
// all the data bytes.  Finally, seek back to the beginning, update the sizes
// in the generalized header, and write it to the file in the space we left
// open for it.

// If Open succeeds, the client must call Close to complete the file writing.
void WavWriter::Open(std::string const & file_name, WAVEFORMATEX const & format)
{
    LOG_ENTRY;

    write_stream_.Open(file_name);

    format_chunk_.Initialize(format);

    uint32_t riff_header_size = RiffHeaderSize();

    write_stream_.Seek(riff_header_size);
}

void WavWriter::Write(char const * const sample_bytes,
                      uint64_t num_bytes_to_write)
{
    //LOG_ENTRY;

    uint64_t num_bytes_written = 0;

    write_stream_.Write(sample_bytes, num_bytes_to_write, num_bytes_written);

    total_data_bytes_written_ += num_bytes_written;
}

void WavWriter::Close()
{
    LOG_ENTRY;

    try
    {
        AdjustDataChunkByteParity();

        SerializeChunks();
    }
    catch (std::exception const & e)
    {
        write_stream_.Close();
        throw;
    }

    write_stream_.Close();
}

uint32_t WavWriter::RiffHeaderSize() const
{
    return riff_wave_semi_chunk_.SizeInRiffHeader() +
        format_chunk_.SizeInRiffHeader() +
        data_semi_chunk_.SizeInRiffHeader();
}

// RIFF spec requires that the number of data bytes in the file be even.
// Furthermore, this parity byte, if written is not included in the data chunk size, but
// is included in the RIFF chunk size.
void WavWriter::AdjustDataChunkByteParity()
{
    // Fix-up byte parity
    if (total_data_bytes_written_ % 2 == 1)
    {
        char parity_byte = 0;
        uint64_t num_bytes_written = 0;

        write_stream_.Write(&parity_byte, 1, num_bytes_written);

        num_parity_bytes_written_ = 1;
    }
}

void WavWriter::SetChunkSizes()
{
    data_semi_chunk_.Initialize(total_data_bytes_written_);

    // Update RIFF chunk to include the size of all subchunks
    riff_wave_semi_chunk_.Initialize(data_semi_chunk_.SizeInRiffHeader() +
                                     format_chunk_.SizeInRiffHeader() +
                                     total_data_bytes_written_ +
                                     num_parity_bytes_written_);
#if 0
    std::cout << "SetChunkSizes: total_data_bytes_written_: "
        << num_parity_bytes_written_
        << " total_data_bytes_written_ "
        << total_data_bytes_written_
        << std::endl;
#endif
}

void WavWriter::SerializeChunks()
{
    write_stream_.SeekAbs(0);

    SetChunkSizes();

    SerializeChunk(&riff_wave_semi_chunk_);

    SerializeChunk(&format_chunk_);

    SerializeChunk(&data_semi_chunk_);
}

void WavWriter::SerializeChunk(Chunk * const chunk)
{
    char* chunk_buffer = new char[chunk->SizeInRiffHeader()];

    chunk->Serialize(chunk_buffer);

    uint64_t num_bytes_written = 0;

    try
    {
        write_stream_.Write(chunk_buffer, chunk->SizeInRiffHeader(), num_bytes_written);
    }
    catch (std::exception const & e)
    {
        delete[] chunk_buffer;
        chunk_buffer = nullptr;

        throw;
    }

    delete[] chunk_buffer;
    chunk_buffer = nullptr;
}
}
