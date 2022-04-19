/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "hal_test_result_serializer.h"

namespace qti_hal_test
{
HALTestResultSerializer::HALTestResultSerializer()
{
    LOG_ENTRY;
}

HALTestResultSerializer::~HALTestResultSerializer()
{
    LOG_ENTRY;
}

void HALTestResultSerializer::Serialize(std::string const & log_file_directory,
                                        bool result,
                                        std::string const & message)
{
    LOG_FUNCTION;

    std::ofstream result_file;
    std::string log_file_{ log_file_directory };
    log_file_.append("/HALTestResult.xml");

    result_file.open(log_file_); // TO DO: Parameterize log path
    if (!result_file.good())
    {
        throw std::runtime_error{ "Failed to open XML result log." };
    }

    std::string tab{ "  " };

    result_file << "<?xml version=\"1.0\"?>" << std::endl
                << "<HALTest>" << std::endl
                << tab << "<Result>";

    if (result)
    {
        result_file << "pass";
    }
    else
    {
        result_file << "fail";
    }

    result_file << "</Result>" << std::endl;

    if (!result)
    {
        result_file << tab << "<Error>" << message << "</Error>" << std::endl;
    }

    result_file << "</HALTest>" << std::endl;

    //  TO DO: Check fstream state here.
    result_file.close();
}
}

