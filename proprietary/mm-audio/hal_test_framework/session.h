/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef SESSION_H_
#define SESSION_H_

#include <stdlib.h>

#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "logger.h"
#include "shared_buffer_control.h"
#include "stream.h"
#include "xml_config.h"

namespace qti_hal_test
{
class Session
{
public:
    Session();
    ~Session();

    Session(Session const & rhs) = delete;
    Session & operator=(Session const & rhs) = delete;

    Session(Session && rhs) noexcept = delete;
    Session & operator=(Session && rhs) noexcept = delete;

    std::shared_ptr<SharedBufferControl> GetSharedBufferControl();

    void AddStream(std::unique_ptr<Stream> stream);

    void Start();

private:

    static const size_t max_num_streams_ { 32 };
    // TO DO: There is an issue with using a std::vector of exception pointers.
    // Need to understand why it is like this.

    std::array<std::exception_ptr, max_num_streams_> stream_exceptions_;
    std::vector<std::unique_ptr<Stream>> streams_;
    std::vector<std::thread> stream_threads_;

    std::shared_ptr<SharedBufferControl> shared_buffer_control_;
    bool allocated_shared_buffer_control_;
};
}

#endif // SESSION_H_