/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef AUDIO_INPUT_FLAGS_H_
#define AUDIO_INPUT_FLAGS_H_

#include <cstdint>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "audio_input_flag.h"
#include "logger.h"
#include "platform_dependencies.h"
#include "xml_config.h"

namespace qti_hal_test
{
class AudioInputFlags
{
public:
    AudioInputFlags() = delete;

    // Pass by value is intentional.
    AudioInputFlags(std::vector<AudioInputFlag> audio_flags);

    ~AudioInputFlags();

    AudioInputFlags(AudioInputFlags const & rhs) = delete;
    AudioInputFlags & operator=(AudioInputFlags const & rhs) = delete;

    AudioInputFlags(AudioInputFlags && rhs) noexcept = delete;
    AudioInputFlags & operator=(AudioInputFlags && rhs) noexcept = delete;

    audio_input_flags_t ConvertToaudio_input_flags_t() const;

private:
    std::vector<AudioInputFlag> audio_flags_;

    // use uint64_t in place of audio_output_flags_t since it's difficult to OR together enums.
    const std::map<std::string, uint64_t> audio_input_flag_LUT_
    {
        { std::string{ "NONE" },        AUDIO_INPUT_FLAG_NONE },
        { std::string{ "FAST" },        AUDIO_INPUT_FLAG_FAST },
        { std::string{ "HW_HOTWORD" },  AUDIO_INPUT_FLAG_HW_HOTWORD },
        { std::string{ "RAW" },         AUDIO_INPUT_FLAG_RAW },
        { std::string{ "SYNC" },        AUDIO_INPUT_FLAG_SYNC }
    };
};
}

#endif // AUDIO_INPUT_FLAGS_H_



