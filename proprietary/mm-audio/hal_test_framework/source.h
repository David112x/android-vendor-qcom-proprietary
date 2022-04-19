/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef SOURCE_H_
#define SOURCE_H_

#include <cstdint>

#include "logger.h"

namespace qti_hal_test
{
class Source
{
public:
    Source();

    virtual ~Source();

    Source(Source const & rhs) = delete;
    Source & operator=(Source const & rhs) = delete;

    Source(Source && rhs) noexcept = delete;
    Source & operator=(Source && rhs) noexcept = delete;

    // TO DO: return bool indicating EOF
    virtual void Read(char * const sample_bytes,
                      uint64_t num_bytes_to_read,
                      uint64_t & num_bytes_read,
                      bool & eof) = 0;
};
}

#endif // SOURCE_H_