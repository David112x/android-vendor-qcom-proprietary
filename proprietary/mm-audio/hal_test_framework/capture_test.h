/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef CAPTURE_TEST_H_
#define CAPTURE_TEST_H_

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>

#include "deserializable.h"
#include "file_capture_stream.h"
#include "file_capture_stream_factory.h"
#include "hal_test.h"
#include "xml_config.h"

namespace qti_hal_test
{
class CaptureTest : public HALTest, Deserializable
{
public:
    CaptureTest() = delete;
    CaptureTest(XmlConfig & xml_config, std::string const & log_file_directory);

    ~CaptureTest();

    void Execute() override;

private:
    void Deserialize(XmlConfig & xml_config) override;

    std::unique_ptr<FileCaptureStream> file_capture_stream_;
};
}

#endif // CAPTURE_TEST_H_