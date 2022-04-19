/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef CAPTURE_BUFFERER_H_
#define CAPTURE_BUFFERER_H_

#include <cassert>
#include <chrono>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <thread>

#include "logger.h"
#include "platform_dependencies.h"
#include "sink.h"
#include "stream_logger.h"

namespace qti_hal_test
{
class CaptureBufferer
{
public:
    CaptureBufferer() = delete;
    CaptureBufferer(int stream_id,
                    qahw_stream_handle_t * const in_stream_handle,
                    std::shared_ptr<Sink> sink,
                    uint32_t duration,
                    std::chrono::seconds condition_timeout,
                    std::shared_ptr<StreamLogger> stream_logger);

    ~CaptureBufferer();

    CaptureBufferer(CaptureBufferer const & rhs) = delete;
    CaptureBufferer & operator=(CaptureBufferer const & rhs) = delete;

    CaptureBufferer(CaptureBufferer && rhs) noexcept;
    CaptureBufferer & operator=(CaptureBufferer && rhs) noexcept = delete;

    void operator()(std::exception_ptr & stream_exception);

private:
    size_t GetBufferSize() const;

    void BufferSamples();

    int stream_id_{ -1 };

    qahw_stream_handle_t * in_stream_handle_;

    std::shared_ptr<Sink> sink_;

    uint32_t duration_;

    std::unique_ptr<char[]> buffer_;
    size_t                  buffer_size_;

    std::chrono::seconds    condition_timeout_;

    std::shared_ptr<StreamLogger> stream_logger_;
};
}
#endif // CAPTURE_BUFFERER_H_
