/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef SINK_H_
#define SINK_H_

#include <cstdint>

#include "logger.h"

namespace qti_hal_test
{
class Sink
{
public:
    Sink();
    virtual ~Sink();

    Sink(Sink const & rhs) = delete;
    Sink & operator=(Sink const & rhs) = delete;

    Sink(Sink && rhs) noexcept = delete;
    Sink & operator=(Sink && rhs) noexcept = delete;

    virtual void Write(char const * const sample_bytes,
                       uint64_t num_bytes_to_write) = 0;
};
}

#endif // SINK_H_