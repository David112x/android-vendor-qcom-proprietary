/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef LOG_WRITER_H_
#define LOG_WRITER_H_

#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>

#include "logger.h"
#include "log_message.h"

namespace qti_hal_test
{

class LogWriter
{
public:
    LogWriter();
    ~LogWriter();

    LogWriter(LogWriter const & rhs) = delete;
    LogWriter & operator=(LogWriter const & rhs) = delete;

    LogWriter(LogWriter && rhs) noexcept = delete;
    LogWriter & operator=(LogWriter && rhs) noexcept = delete;

    void Write(std::shared_ptr<LogMessage> log_message) const;

private:
    static std::mutex stream_mutex_;
};
}

#endif // LOG_WRITER_H_