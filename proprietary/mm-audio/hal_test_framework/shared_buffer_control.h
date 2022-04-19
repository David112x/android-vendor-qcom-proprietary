/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef SHARED_BUFFER_CONTROL_H_
#define SHARED_BUFFER_CONTROL_H_

#include <cstdio>
#include <condition_variable>
#include <mutex>

#include "logger.h"
#include "platform_dependencies.h"

namespace qti_hal_test
{
class SharedBufferControl
{
public:
    SharedBufferControl();
    ~SharedBufferControl();

    SharedBufferControl(SharedBufferControl const & rhs) = delete;
    SharedBufferControl & operator=(SharedBufferControl const & rhs) = delete;
    SharedBufferControl(SharedBufferControl && rhs) noexcept = delete;
    SharedBufferControl & operator=(SharedBufferControl && rhs) noexcept = delete;

    bool WriteReady() const;
    void SetWriteReady();
    void ResetWriteReady();

    bool DrainReady() const;
    void SetDrainReady();
    void ResetDrainReady();

    std::timed_mutex write_mutex_;
    std::mutex write_wait_mutex_;
    std::condition_variable write_condition_;
    bool write_ready_;

    std::timed_mutex drain_mutex_;
    std::mutex drain_wait_mutex_;
    std::condition_variable drain_condition_;
    bool drain_ready_;
};
}

#endif // SHARED_BUFFER_CONTROL_H_