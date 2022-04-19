 /**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "command_line_parser.h"
#include "hal_test.h"
#include "hal_test_factory.h"
#include "logger.h"
#include "xml_config.h"

using namespace qti_hal_test;

void Run(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    LOG_FUNCTION;

    int status = 0;

    try
    {
        Run(argc, argv);
    }
    catch (std::exception const & e)
    {
        std::cout << e.what() << std::endl;
        status = 1;
    }

    return status;
}

void Run(int argc, char* argv[])
{
    LOG_FUNCTION;

    CommandLineParser command_line_parser(argc, argv);

    std::string xml_config_file_path;
    if (!command_line_parser.Find("-x", xml_config_file_path))
    {
        throw std::runtime_error{ "-x xml_config_file_path required" };
    }

    XmlConfig xml_config(xml_config_file_path);

    HALTestFactory hal_test_factory;

    std::unique_ptr<HALTest> hal_test{ hal_test_factory.GetInstance(xml_config) };

    try
    {
        hal_test->Execute();
    }
    catch (std::exception const & e)
    {
        hal_test->SerializeResult(false, e.what());
        throw;
    }

    hal_test->SerializeResult(true, "");
}
