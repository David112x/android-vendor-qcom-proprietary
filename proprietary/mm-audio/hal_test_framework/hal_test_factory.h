/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef HAL_TEST_FACTORY_H_
#define HAL_TEST_FACTORY_H_

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "capture_test.h"
#include "hal_test.h"
#include "hal_test_result_serializer.h"
#include "logger.h"
#include "playback_test.h"
#include "session_test.h"
#include "xml_config.h"

namespace qti_hal_test
{
class HALTestFactory
{
public:
    HALTestFactory();
    ~HALTestFactory();

    std::unique_ptr<HALTest> GetInstance(XmlConfig & xml_config);

private:
    void Deserialize(XmlConfig & xml_config, std::string & log_file_directory);

    HALTestResultSerializer result_serializer_;
};
}
#endif // HAL_TEST_FACTORY_H_