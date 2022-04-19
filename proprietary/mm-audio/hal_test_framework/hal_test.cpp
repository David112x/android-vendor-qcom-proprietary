/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "hal_test.h"

namespace qti_hal_test
{
HALTest::HALTest(std::string const & log_file_directory):
    log_file_directory_{ log_file_directory }
{
    LOG_ENTRY;
}

HALTest::~HALTest()
{
    LOG_ENTRY;
}

void HALTest::SerializeResult(bool result, std::string const & message) const
{
    LOG_ENTRY;
    HALTestResultSerializer::Serialize(log_file_directory_, result, message);
}
}