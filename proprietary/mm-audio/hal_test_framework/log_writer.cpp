/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "log_writer.h"

namespace qti_hal_test
{

std::mutex LogWriter::stream_mutex_;

LogWriter::LogWriter()
{
    LOG_ENTRY;
}

LogWriter::~LogWriter()
{
    LOG_ENTRY;
}

void LogWriter::Write(std::shared_ptr<LogMessage> log_message) const
{
    std::lock_guard<std::mutex> lock(stream_mutex_);
    if (log_message)
    {
        std::cout << log_message->ToString() << std::endl;
    }
    else
    {
        std::cout << "nullptr" << std::endl;
    }
}
}