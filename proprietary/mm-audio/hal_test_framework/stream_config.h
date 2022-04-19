/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef STREAM_CONFIG_H_
#define STREAM_CONFIG_H_

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "deserializable.h"
#include "logger.h"
#include "xml_config.h"

namespace qti_hal_test
{
class StreamConfig : public Deserializable
{
public:
    StreamConfig();
    explicit StreamConfig(XmlConfig & xml_config);
    // TO DO: Investigate move construction?

    ~StreamConfig();

    void Deserialize(XmlConfig & xml_config) override;
};
}

#endif // STREAM_CONFIG_H_