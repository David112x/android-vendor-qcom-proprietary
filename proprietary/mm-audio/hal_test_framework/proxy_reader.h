/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef PROXY_READER_H_
#define PROXY_READER_H_

#include <cassert>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include "logger.h"
#include "platform_dependencies.h"
#include "source.h"
#include "stream_logger.h"

namespace qti_hal_test
{
class ProxyReader
{
public:

    ProxyReader() = delete;
    ProxyReader(int stream_id, std::shared_ptr<StreamLogger> stream_logger);

    ~ProxyReader();

    ProxyReader(ProxyReader const & rhs) = delete;
    ProxyReader & operator=(ProxyReader const & rhs) = delete;

    ProxyReader(ProxyReader && rhs) noexcept;
    ProxyReader & operator=(ProxyReader && rhs) noexcept = delete;

    void operator()(std::exception_ptr & stream_exception);


private:
    int stream_id_{ -1 };

    std::shared_ptr<StreamLogger> stream_logger_;
};
}

#endif // PROXY_READER_H_