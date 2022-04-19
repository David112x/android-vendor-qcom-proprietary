/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "capture_test.h"

namespace qti_hal_test
{
CaptureTest::CaptureTest(XmlConfig & xml_config, std::string const & log_file_directory) :
    HALTest{ log_file_directory }
{
    Deserialize(xml_config);

    FileCaptureStreamFactory file_capture_stream_factory;

    file_capture_stream_ = file_capture_stream_factory.GetInstance(xml_config);

    assert(file_capture_stream_);
}

CaptureTest::~CaptureTest()
{

}

void CaptureTest::Execute()
{
    file_capture_stream_->Start();
}

void CaptureTest::Deserialize(XmlConfig & xml_config)
{
    if (!xml_config.IsMatch("CaptureTest")) // TO DO: Here we're simply validating we're looking at the right
    {                                       // element.  No need to move first.
        throw std::runtime_error{ "Expected element CaptureTest" };
    }
}
}
