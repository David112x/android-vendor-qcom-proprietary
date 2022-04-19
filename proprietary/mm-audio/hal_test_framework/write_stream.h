/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef WAVIO_WRITE_STREAM_H_
#define WAVIO_WRITE_STREAM_H_

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "logger.h"

namespace qti_hal_test
{
class WriteStream
{
public:
    WriteStream();
    ~WriteStream();

    WriteStream(WriteStream const & rhs) = delete;
    WriteStream & operator=(WriteStream const & rhs) = delete;

    WriteStream(WriteStream && rhs) noexcept = delete;
    WriteStream & operator=(WriteStream && rhs) noexcept = delete;

    void Open(std::string const & file_name);
    void Seek(uint32_t offset); // TO DO: Factor out
    void SeekAbs(uint32_t offset);

    void Write(char const * const buffer,
        uint64_t num_bytes_to_write,
        uint64_t & num_bytes_written);

    void Close() noexcept;

private:
    std::ofstream out_stream_;

    uint64_t total_num_bytes_written_;

    bool is_closed_;
};
}

#endif // WAVIO_WRITE_STREAM_H_