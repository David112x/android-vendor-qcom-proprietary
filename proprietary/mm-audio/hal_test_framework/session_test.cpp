/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "session_test.h"

namespace qti_hal_test
{
SessionTest::SessionTest(XmlConfig & xml_config, std::string const & log_file_directory) :
    HALTest{ log_file_directory } // TO DO:  Compiler will not catch your mistake if you do
{                                 // not explicitly call this constructor if there is a
    LOG_ENTRY;                    // default constructor defined. You added one and
    Deserialize(xml_config);      // introduced this bug!

    SessionFactory session_factory;
    session_ = session_factory.GetInstance(xml_config);
}

SessionTest::~SessionTest()
{
    LOG_ENTRY;
}

void SessionTest::Execute()
{
    LOG_ENTRY;
    session_->Start();
}

void SessionTest::Deserialize(XmlConfig & xml_config)
{
    LOG_ENTRY;
    if (!xml_config.IsMatch("SessionTest")) // TO DO: Here we're simply validating we're
    {                                       // looking at the right element.  No need to
                                            // move first.
        throw std::runtime_error{ "Expected element SessionTest" };
    }
}
}