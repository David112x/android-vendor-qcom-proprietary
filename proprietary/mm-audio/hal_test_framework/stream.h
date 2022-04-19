/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef STREAM_H_
#define STREAM_H_

#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

#include "hal_stream.h"
#include "logger.h"
#include "xml_config.h"

namespace qti_hal_test
{
class Stream
{
public:
    Stream() = delete;
    explicit Stream(int stream_id,
                    std::unique_ptr<HALStream> hal_stream);
    ~Stream();

    Stream(Stream const & rhs) = delete;
    Stream & operator=(Stream const & rhs) = delete;

    Stream(Stream && rhs) noexcept;
    Stream & operator=(Stream && rhs) noexcept = delete;

    void Start();

    void operator()(std::exception_ptr & stream_exception);

private:
    int stream_id_{ -1 };
    std::unique_ptr<HALStream> hal_stream_;
};
}

#endif // STREAM_H_