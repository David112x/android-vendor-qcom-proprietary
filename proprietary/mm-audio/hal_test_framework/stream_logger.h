/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef STREAM_LOGGER_H_
#define STREAM_LOGGER_H_

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>

#include "logger.h"
#include "log_message.h"
#include "log_writer.h"
#include "thread_safe_queue.h"

namespace qti_hal_test
{
class StreamLogger  // Perhaps "StreamLogWriter" is more consistent within the implementation.
{
public:
    StreamLogger(int stream_id);
    StreamLogger(int stream_id, std::shared_ptr<LogWriter> log_writer);
    ~StreamLogger();

    StreamLogger(StreamLogger const & rhs) = delete;
    StreamLogger & operator=(StreamLogger const & rhs) = delete;

    StreamLogger(StreamLogger && rhs) noexcept = delete;
    StreamLogger & operator=(StreamLogger && rhs) noexcept = delete;

    void API(std::string && api_name) const;
    void Enter(std::string && name) const;
    void Exit(std::string && name) const;

    void Error(std::string && message) const;
    void Warning(std::string && message) const;
    void Info(std::string && message) const;

    void FrameworkError(std::string && message) const;
    void FrameworkWarning(std::string && message) const;
    void FrameworkInfo(std::string && message) const;

    LogMessage ErrorLogMessage(std::string && message) const;

    void Log(LogMessage message) const;

private:
    int stream_id_;
    std::chrono::steady_clock::time_point initial_time_;
    std::shared_ptr<LogWriter> log_writer_;

#define USE_THREAD_SAFE_QUEUE
#ifdef USE_THREAD_SAFE_QUEUE

    // TO DO: Modify to thread-safe priority queue, dequeuing on timestamp order.
    mutable ThreadSafeQueue<LogMessage> messages_;
#else
    mutable std::queue<LogMessage> messages_;
#endif
};
}
#endif // STREAM_LOGGER_H_