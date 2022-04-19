/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef FILE_PLAYBACK_STREAM_CONFIG_H_
#define FILE_PLAYBACK_STREAM_CONFIG_H_

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "audio_output_flag.h"
#include "audio_output_flags.h"
#include "deserializable.h"
#include "logger.h"
#include "metadata.h"
#include "platform_dependencies.h"
#include "xml_config.h"

namespace qti_hal_test
{
class FilePlaybackStreamConfig : public Deserializable
{
public:
    FilePlaybackStreamConfig();
    explicit FilePlaybackStreamConfig(XmlConfig & xml_config);
    ~FilePlaybackStreamConfig();

    FilePlaybackStreamConfig(FilePlaybackStreamConfig const & rhs);
    FilePlaybackStreamConfig & operator=(FilePlaybackStreamConfig const & rhs) = delete;
    FilePlaybackStreamConfig(FilePlaybackStreamConfig && rhs) noexcept = delete;
    FilePlaybackStreamConfig & operator=(FilePlaybackStreamConfig && rhs) noexcept = delete;

    void Deserialize(XmlConfig & xml_config) override;

    uint32_t                sample_rate_;
    uint16_t                channels_;
    uint16_t                bit_width_;
    std::string             kvpairs_;
    float                   vol_level_;
    audio_devices_t         output_device_; // TO DO: Check for narrowing.
    audio_format_t          audio_format_;
    std::string             address_;
    audio_output_flags_t    audio_output_flags_;
    std::string             aptx_decoder_bluetooth_address_;
    std::chrono::seconds    condition_timeout_;
};
}

#endif // FILE_PLAYBACK_STREAM_CONFIG_H_