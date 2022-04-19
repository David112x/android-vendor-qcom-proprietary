/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef SESSION_FACTORY_H_
#define SESSION_FACTORY_H_

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include "file_capture_stream.h"
#include "file_capture_stream_factory.h"
#include "file_playback_stream.h"
#include "file_playback_stream_mock.h"
#include "file_playback_stream_mock_2.h"
#include "file_playback_stream_factory.h"
#include "hal_stream.h"
#include "logger.h"
#include "session.h"
#include "shared_buffer_control.h"
#include "xml_config.h"

namespace qti_hal_test
{
class SessionFactory
{
public:
    SessionFactory();
    ~SessionFactory();

    SessionFactory(SessionFactory const & rhs) = delete;
    SessionFactory & operator=(SessionFactory const & rhs) = delete;

    SessionFactory(SessionFactory && rhs) noexcept = delete;
    SessionFactory & operator=(SessionFactory && rhs) noexcept = delete;

    std::unique_ptr<Session> GetInstance(XmlConfig & xml_config);
};
}
#endif // SESSION_FACTORY_H_