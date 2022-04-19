/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef AUDIO_OUTPUT_FLAG_H_
#define AUDIO_OUTPUT_FLAG_H_

#include <stdexcept>
#include <string>

#include "deserializable.h"
#include "logger.h"
#include "xml_config.h"

namespace qti_hal_test
{
class AudioOutputFlag : public Deserializable
{
public:
    AudioOutputFlag() = delete;
    explicit AudioOutputFlag(XmlConfig & xml_config);
    ~AudioOutputFlag();

    AudioOutputFlag(AudioOutputFlag const & rhs) = default;
    AudioOutputFlag & operator=(AudioOutputFlag const & rhs) = delete;

    AudioOutputFlag(AudioOutputFlag && rhs) noexcept = default;
    AudioOutputFlag & operator=(AudioOutputFlag && rhs) noexcept = delete;


    std::string flag_name_;
private:

    void Deserialize(XmlConfig & xml_config) override;
};
}

#endif // AUDIO_OUTPUT_FLAG_H_