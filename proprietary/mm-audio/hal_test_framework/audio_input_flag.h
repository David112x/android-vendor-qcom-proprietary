/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef AUDIO_INPUT_FLAG_H_
#define AUDIO_INPUT_FLAG_H_

#include <stdexcept>
#include <string>

#include "deserializable.h"
#include "logger.h"
#include "xml_config.h"

namespace qti_hal_test
{
class AudioInputFlag : public Deserializable
{
public:
    AudioInputFlag() = delete;
    explicit AudioInputFlag(XmlConfig & xml_config);
    ~AudioInputFlag();

    AudioInputFlag(AudioInputFlag const & rhs) = default;
    AudioInputFlag & operator=(AudioInputFlag const & rhs) = delete;

    AudioInputFlag(AudioInputFlag && rhs) noexcept = default;
    AudioInputFlag & operator=(AudioInputFlag && rhs) noexcept = delete;


    std::string flag_name_;
private:

    void Deserialize(XmlConfig & xml_config) override;
};
}

#endif // AUDIO_INPUT_FLAG_H_