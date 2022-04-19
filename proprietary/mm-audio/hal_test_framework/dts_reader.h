/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef DTS_READER_H_
#define DTS_READER_H_

#include <cstdint>
#include <cstdio>
#include <cstring> // For strncmp
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "logger.h"
//#include "read_stream.h"
#include "source.h"

namespace qti_hal_test
{
class DTSReader : public Source
{
public:
    DTSReader();
    explicit DTSReader(std::string const & file_name);
    ~DTSReader();

    void Open(std::string const & file_name);

    DTSReader(DTSReader const & rhs) = delete;
    DTSReader & operator=(DTSReader const & rhs) = delete;

    DTSReader(DTSReader && rhs) noexcept = delete;
    DTSReader & operator=(DTSReader && rhs) noexcept = delete;

    void Read(char * const sample_bytes,
              uint64_t num_bytes_to_read,
              uint64_t & num_bytes_read,
              bool & eof) override;

    void Close();

private:
    void ParseFileHeader();

    /* convert big-endian to little-endian */
    uint64_t convert_BE_to_LE(uint64_t in) const;

    //ReadStream read_stream_;

    FILE * file_stream_;
};
}

#endif // DTS_READER_H_