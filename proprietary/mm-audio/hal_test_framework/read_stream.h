/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef READ_STREAM_H_
#define READ_STREAM_H_

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "logger.h"

namespace qti_hal_test
{
class ReadStream
{
public:
    ReadStream();
    ~ReadStream();

    ReadStream(ReadStream const & rhs) = delete;
    ReadStream & operator=(ReadStream const & rhs) = delete;

    ReadStream(ReadStream && rhs) noexcept = delete;
    ReadStream & operator=(ReadStream && rhs) noexcept = delete;

    void Open(std::string file_name);

    // TO DO: return bool indicating EOF
    void Read(char * const buffer,
              uint64_t num_bytes_to_read,
              uint64_t & num_bytes_read,
              bool & eof);
    void Seek(uint32_t offset); // TO DO: Factor out
    void SeekAbs(uint32_t offset); // TO DO: Factor out
    void Close() noexcept;

private:
    std::ifstream in_stream_;

    uint32_t total_num_bytes_read_;

    bool is_closed_;
};
}

#endif // READ_STREAM_H_