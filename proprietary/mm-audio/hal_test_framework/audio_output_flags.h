/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef AUDIO_OUTPUT_FLAGS_H_
#define AUDIO_OUTPUT_FLAGS_H_

#include <cstdint>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "audio_output_flag.h"
#include "logger.h"
#include "platform_dependencies.h"
#include "xml_config.h"

namespace qti_hal_test
{
class AudioOutputFlags
{
public:
    AudioOutputFlags() = delete;

    // Pass by value is intentional.
    AudioOutputFlags(std::vector<AudioOutputFlag> audio_flags);

    ~AudioOutputFlags();

    AudioOutputFlags(AudioOutputFlags const & rhs) = delete;
    AudioOutputFlags & operator=(AudioOutputFlags const & rhs) = delete;

    AudioOutputFlags(AudioOutputFlags && rhs) noexcept = delete;
    AudioOutputFlags & operator=(AudioOutputFlags && rhs) noexcept = delete;

    audio_output_flags_t ConvertToaudio_output_flags_t() const;

private:
    std::vector<AudioOutputFlag> audio_flags_;

    // use uint64_t in place of audio_output_flags_t since it's difficult to OR together enums.
    const std::map<std::string, uint64_t> audio_output_flag_LUT_
    {
        { std::string{ "NONE" },                 AUDIO_OUTPUT_FLAG_NONE },
        { std::string{ "DIRECT" },               AUDIO_OUTPUT_FLAG_DIRECT },
        { std::string{ "PRIMARY" },              AUDIO_OUTPUT_FLAG_PRIMARY },
        { std::string{ "FAST" },                 AUDIO_OUTPUT_FLAG_FAST },
        { std::string{ "DEEP_BUFFER" },          AUDIO_OUTPUT_FLAG_DEEP_BUFFER },
        { std::string{ "COMPRESS_OFFLOAD" },     AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD },
        { std::string{ "NON_BLOCKING" },         AUDIO_OUTPUT_FLAG_NON_BLOCKING },
        { std::string{ "HW_AV_SYNC" },           AUDIO_OUTPUT_FLAG_HW_AV_SYNC },
        { std::string{ "TTS" },                  AUDIO_OUTPUT_FLAG_TTS },
        { std::string{ "RAW" },                  AUDIO_OUTPUT_FLAG_RAW },
        { std::string{ "SYNC" },                 AUDIO_OUTPUT_FLAG_SYNC },
        { std::string{ "IEC958_NONAUDIO" },      AUDIO_OUTPUT_FLAG_IEC958_NONAUDIO },
        { std::string{ "VOIP_RX" },              AUDIO_OUTPUT_FLAG_VOIP_RX },
        { std::string{ "COMPRESS_PASSTHROUGH" }, AUDIO_OUTPUT_FLAG_COMPRESS_PASSTHROUGH },
        { std::string{ "DIRECT_PCM" },           AUDIO_OUTPUT_FLAG_DIRECT_PCM },
        { std::string{ "MAIN" },                 AUDIO_OUTPUT_FLAG_MAIN },
        { std::string{ "ASSOCIATED" },           AUDIO_OUTPUT_FLAG_ASSOCIATED },
        { std::string{ "TIMESTAMP" },            AUDIO_OUTPUT_FLAG_TIMESTAMP }
    };
};
}

#endif // AUDIO_OUTPUT_FLAGS_H_