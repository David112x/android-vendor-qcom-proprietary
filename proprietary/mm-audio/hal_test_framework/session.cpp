/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "session.h"

namespace qti_hal_test
{
Session::Session() :
    allocated_shared_buffer_control_{false}
{
    LOG_ENTRY;
}

Session::~Session()
{
    LOG_ENTRY;
}

std::shared_ptr<SharedBufferControl> Session::GetSharedBufferControl()
{
    LOG_ENTRY;

    if (!allocated_shared_buffer_control_)
    {
        shared_buffer_control_ = std::make_shared<SharedBufferControl>();
        allocated_shared_buffer_control_ = true;
    }

    return shared_buffer_control_;   // TO DO: Confirm this doesn't introduce a security
}                                    // issue.

void Session::AddStream(std::unique_ptr<Stream> stream)
{
    LOG_ENTRY;

    streams_.emplace_back(std::move(stream));

    assert(streams_.size() > 0);
}

void Session::Start() // This name is misleading
{
    LOG_ENTRY;

    if (streams_.size() > max_num_streams_)
    {
        std::ostringstream error_msg;
        error_msg << "Maximum of " << max_num_streams_ << " currently supported";
        throw std::runtime_error{ error_msg.str() };
    }

    for (size_t i = 0; i < streams_.size(); ++i)
    {
        // TO DO: Check dereferece of unique_ptr.
        stream_threads_.emplace_back(std::thread{ std::move(*streams_[i]),
                                                  std::ref(stream_exceptions_[i]) });
    }

    for (size_t i = 0; i < streams_.size(); ++i)
    {
        stream_threads_[i].join();
    }

    // TO DO: Implement a thread-safe queue so that the first failure is logged, not the
    // one with the lowest index in the array.
    for (size_t i = 0; i < std::min(streams_.size(),max_num_streams_); ++i)
    {
        if (stream_exceptions_[i])
        {
            std::rethrow_exception(stream_exceptions_[i]);
        }
    }
}
}
