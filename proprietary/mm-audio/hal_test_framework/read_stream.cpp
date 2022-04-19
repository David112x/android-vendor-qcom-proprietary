/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "read_stream.h"

namespace qti_hal_test
{
ReadStream::ReadStream() :
    total_num_bytes_read_{ 0 },
    is_closed_{ false }
{
    LOG_ENTRY;
}

ReadStream::~ReadStream()
{
    LOG_ENTRY;
    if (!is_closed_)
    {
        in_stream_.close();
        is_closed_ = true;
    }
}

void ReadStream::Open(std::string file_name)
{
    LOG_ENTRY;
    in_stream_.open(file_name, std::ifstream::binary | std::ifstream::ate);

    if (!in_stream_.good())
    {
        std::ostringstream error_msg;
        error_msg << std::endl << "Failed to open file:\"" << file_name << "\"";
        throw std::ios::failure{ error_msg.str() };
    }

#if 0
    int size = in_stream_.tellg();
    std::streamoff size = in_stream_.tellg();

    if (!in_stream_.good())
    {
        std::ostringstream error_msg;
        error_msg << std::endl << "Failed to open file:\"" << file_name << "\"";
        throw std::ios::failure{ error_msg.str() };
    }
#endif

    in_stream_.seekg(in_stream_.beg);

    if (!in_stream_.good())
    {
        std::ostringstream error_msg;
        error_msg << std::endl << "Failed to open file:\"" << file_name << "\"";
        throw std::ios::failure{ error_msg.str() };
    }
}

void ReadStream::Read(char* const buffer,
                      uint64_t num_bytes_to_read,
                      uint64_t & num_bytes_read,
                      bool & eof)
{
    in_stream_.read(buffer, num_bytes_to_read);

    if (in_stream_.eof())
    {
        eof = true;
        in_stream_.clear();
    }
    else if (!in_stream_.good())
    {
        throw std::ios::failure{ "Failed to read file" };
    }

    num_bytes_read = static_cast<uint64_t>(in_stream_.gcount());  // TO DO: Narrowing cast - fix.

    total_num_bytes_read_ += num_bytes_read;
}

void ReadStream::Seek(uint32_t offset)
{
    in_stream_.seekg(offset, in_stream_.cur);
    if (!in_stream_.good())
    {
        throw std::ios::failure{ "Failed to seek relative to current position in file" };
    }
}

void ReadStream::SeekAbs(uint32_t offset)
{
    in_stream_.seekg(offset, in_stream_.beg);
    if (!in_stream_.good())
    {
        throw std::ios::failure{ "Failed to seek from beginning of file" };
    }
}

void ReadStream::Close() noexcept
{
    LOG_ENTRY;
    if (!is_closed_)
    {
        in_stream_.close();
        is_closed_ = true;
    }
}
}