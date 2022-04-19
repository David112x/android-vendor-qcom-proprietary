
/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "proxy_reader.h"

namespace qti_hal_test
{
ProxyReader::ProxyReader(int stream_id, std::shared_ptr<StreamLogger> stream_logger):
    stream_id_{ stream_id },
    stream_logger_{ stream_logger }
{
    LOG_STREAM_ENTRY(stream_id_);

    stream_logger_->Info("DEBUG: ProxyReader");
}

ProxyReader::~ProxyReader()
{
}

ProxyReader::ProxyReader(ProxyReader && rhs) noexcept :
    stream_id_ { rhs.stream_id_},
    stream_logger_{ std::move(rhs.stream_logger_) } // std::move is faster
{
}

void ProxyReader::operator()(std::exception_ptr & stream_exception)
{
    LOG_STREAM_ENTRY(stream_id_);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
}
