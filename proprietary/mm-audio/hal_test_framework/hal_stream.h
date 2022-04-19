/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef HAL_STREAM_H_
#define HAL_STREAM_H_

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

#include "logger.h"
#include "log_writer.h"
#include "stream_logger.h"

namespace qti_hal_test
{
class HALStream
{
public:
    explicit HALStream(int stream_id);
    virtual ~HALStream();

    HALStream(HALStream const & rhs) = delete;
    HALStream & operator=(HALStream const & rhs) = delete;

    HALStream(HALStream && rhs) noexcept = delete;
    HALStream & operator=(HALStream && rhs) noexcept = delete;

    virtual void Start() = 0;

protected:
    int stream_id_{ -1 };  // TO DO: This used to be declared const but since we want to
                     // be able to move construct a stream, we should reset it
                     // to an invalid value?

    std::shared_ptr<StreamLogger> stream_logger_;
};
}

#endif // HAL_STREAM_H_