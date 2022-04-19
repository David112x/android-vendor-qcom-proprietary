/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef FILE_READER_H_
#define FILE_READER_H_

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

#include "logger.h"
#include "read_stream.h"
#include "source.h"

namespace qti_hal_test
{
class FileReader : public Source
{
public:
    FileReader();
    explicit FileReader(std::string const & file_name);

    ~FileReader();

    void Open(std::string const & file_name);

    // TO DO: return bool indicating EOF
    void Read(char * const sample_bytes,
              uint64_t num_bytes_to_read,
              uint64_t & num_bytes_read,
              bool & eof) override;

    void Close() noexcept;

private:
    ReadStream read_stream_;
};
}

#endif // FILE_READER_H_