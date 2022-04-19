/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "stream_config.h"

namespace qti_hal_test
{
StreamConfig::StreamConfig()
{
    LOG_ENTRY;
}

// Postcondition: all data members are initialized.
StreamConfig::StreamConfig(XmlConfig & xml_config)
{
    LOG_ENTRY;
    Deserialize(xml_config);
}

StreamConfig::~StreamConfig()
{
    LOG_ENTRY;
}

void StreamConfig::Deserialize(XmlConfig & xml_config)
{
    LOG_ENTRY;
    xml_config.MoveToElement("StreamConfig");
}
}