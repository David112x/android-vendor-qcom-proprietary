/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef SESSION_TEST_H_
#define SESSION_TEST_H_

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include "deserializable.h"
#include "hal_test.h"
#include "logger.h"
#include "session.h"
#include "session_factory.h"
#include "xml_config.h"

namespace qti_hal_test
{
class SessionTest : public HALTest, Deserializable
{
public:
    SessionTest() = delete;
    explicit SessionTest(XmlConfig & xml_config, std::string const & log_file_directory);

    ~SessionTest();

    SessionTest(SessionTest const & rhs) = delete;
    SessionTest & operator=(SessionTest const & rhs) = delete;
    SessionTest(SessionTest && rhs) noexcept = delete;
    SessionTest & operator=(SessionTest && rhs) noexcept = delete;

    void Execute() override;

private:
    void Deserialize(XmlConfig & xml_config) override;

    std::unique_ptr<Session> session_;
};
}

#endif // SESSION_TEST_H_