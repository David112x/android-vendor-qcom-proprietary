/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "file_reader.h"

namespace qti_hal_test
{
FileReader::FileReader()
{
    LOG_ENTRY;
}

FileReader::FileReader(std::string const & file_name)
{
    LOG_ENTRY;
    read_stream_.Open(file_name);
}

FileReader::~FileReader()
{
    LOG_ENTRY;
    Close();
}

void FileReader::Open(std::string const & file_name)
{
    LOG_ENTRY;
    read_stream_.Open(file_name);
}

// TO DO: return bool indicating EOF
void FileReader::Read(char * const sample_bytes,
                      uint64_t num_bytes_to_read,
                      uint64_t & num_bytes_read,
                      bool & eof)
{
    //LOG_ENTRY;
    read_stream_.Read(sample_bytes, num_bytes_to_read, num_bytes_read, eof);
}

void FileReader::Close() noexcept
{
    LOG_ENTRY;
    read_stream_.Close();
}
}
