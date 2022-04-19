/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "hal_test_factory.h"

namespace qti_hal_test
{
HALTestFactory::HALTestFactory()
{
    LOG_ENTRY;
}

HALTestFactory::~HALTestFactory()
{
    LOG_ENTRY;
}

std::unique_ptr<HALTest> HALTestFactory::GetInstance(XmlConfig & xml_config)
{
    LOG_ENTRY;

    // Deserialize HALTest element.  Recall that semantically HALTestFactory is a constructor.
    // Locate the reader on the element identifying/containing the test to be instantiated.
    std::string log_file_directory;
    Deserialize(xml_config, log_file_directory); // Exceptions thrown by Deserialize must
                                                 // be allowed to escape since a exception
                                                 // indicates that the log file path is not
                                                 // available.These errors cannot be
                                                 // processed by HALTestErrorSerializer,
                                                 // which depends on the file path.
                                                 // TO DO: Derive an exception type and
                                                 // filter at call site.
    std::unique_ptr<HALTest> hal_test;

    try // TO DO: Find a better way to switch: Generic and/or map?
    {
        xml_config.MoveToNextElement(); // Note that the pair MoveToNextElement() and
                                        // IsMatch() are repeated below -- combine.
        if (xml_config.IsMatch("PlaybackTest")) // TO DO: Consolidate the two calls
        {                                       // on xml_config.
            hal_test = std::make_unique<PlaybackTest>(xml_config, log_file_directory);
        }
        else if (xml_config.IsMatch("CaptureTest"))
        {
            hal_test = std::make_unique<CaptureTest>(xml_config, log_file_directory);
        }
		else if (xml_config.IsMatch("SessionTest"))
        {
            hal_test = std::make_unique<SessionTest>(xml_config, log_file_directory);
        }
        else
        {
            std::ostringstream error_msg;
            // TO DO: add the row-col error from IsMatch to the error message.
            error_msg << "Test " << xml_config.GetName() << " not found";
            throw std::runtime_error{ error_msg.str() };
        }
    }
    catch (std::exception const & e) // This needs to go away; see above.
    {
        HALTestResultSerializer::Serialize(log_file_directory, false, e.what());
        throw;
    }

    return hal_test;
}

void HALTestFactory::Deserialize(XmlConfig & xml_config, std::string & log_file_directory)
{
    LOG_ENTRY;

    xml_config.MoveToNextElement(); // We're at the root of the tree so calling
                                    // MoveToNextElement() is an attemptto ensure we don't
                                    // have a nested HALTest element.
    if (!xml_config.IsMatch("HALTest")) // TO DO: Consolidate the two calls on xml_config.
    {
        throw std::runtime_error{ "Expected element HALTest" };
    }

    log_file_directory = xml_config.GetElementText("LogFileDirectory");
}
}
