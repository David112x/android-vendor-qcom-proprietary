/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef HAL_TEST_H_
#define HAL_TEST_H_

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "hal_test_result_serializer.h"
#include "logger.h"

// TO DO: Define a custom exception class here that is specific to HALTests. Sim with XMLDeserializer.
namespace qti_hal_test
{
class HALTest
{
public:
    HALTest() = delete;
    explicit HALTest(std::string const & log_file_directory);
    virtual ~HALTest();

    virtual void Execute() = 0;

    void SerializeResult(bool result, std::string const & message) const;

protected:
    std::string log_file_directory_;
};
}
#endif // HAL_TEST_H_
