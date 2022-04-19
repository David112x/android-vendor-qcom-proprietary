/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef HAL_TEST_RESULT_SERIALIZER_H_
#define HAL_TEST_RESULT_SERIALIZER_H_

#include <fstream>
#include <stdexcept>
#include <string>

#include "logger.h"

namespace qti_hal_test
{
class HALTestResultSerializer
{
public:
    HALTestResultSerializer();
    ~HALTestResultSerializer();

    static void Serialize(std::string const & log_file_directory,
                          bool result,
                          std::string const & message);
};
}
#endif // HAL_TEST_RESULT_SERIALIZER_H_
