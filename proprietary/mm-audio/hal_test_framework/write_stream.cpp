/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "write_stream.h"

namespace qti_hal_test
{
WriteStream::WriteStream() :
total_num_bytes_written_{ 0 },
is_closed_{ false }
{
    LOG_ENTRY;
}

WriteStream::~WriteStream()
{
    LOG_ENTRY;
    if (!is_closed_)
    {
        out_stream_.close();
        is_closed_ = true;
    }
}

void WriteStream::Open(std::string const & file_name)
{
    LOG_ENTRY;
    out_stream_.open(file_name);
    if (!out_stream_.good())
    {
        std::ostringstream error_msg;
        error_msg  << "WriteStream::Open: Could not open file " << file_name << std::endl;
        throw std::ios::failure{ error_msg.str() };
    }
}

void WriteStream::Seek(uint32_t offset)
{
    out_stream_.seekp(offset, out_stream_.cur);
    if (!out_stream_.good())
    {
        throw std::ios::failure{ "WriteStream::Seek: out_stream_.seekp failed" };
    }
}

void WriteStream::SeekAbs(uint32_t offset)
{
    out_stream_.seekp(offset, out_stream_.beg);
    if (!out_stream_.good())
    {
        throw std::ios::failure{ "WriteStream::Seek: out_stream_.seekp failed" };
    }
}

void WriteStream::Write(char const * const buffer,
    uint64_t num_bytes_to_write,
    uint64_t & num_bytes_written)
{
    total_num_bytes_written_ += num_bytes_to_write;

    out_stream_.write(buffer, num_bytes_to_write);

    num_bytes_written = num_bytes_to_write;

    if (!out_stream_.good())
    {
        throw std::ios::failure{ "WriteStream::Write: out_stream_.write failed" };
    }
}

void WriteStream::Close() noexcept
{
    LOG_ENTRY;
    if (!is_closed_)
    {
        out_stream_.close();
        is_closed_ = true;
    }
}
}
