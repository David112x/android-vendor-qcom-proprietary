/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef FILE_CAPTURE_STREAM_CONFIG_H_
#define FILE_CAPTURE_STREAM_CONFIG_H_

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "audio_input_flag.h"
#include "audio_input_flags.h"
#include "deserializable.h"
#include "logger.h"
#include "platform_dependencies.h"
#include "xml_config.h"

namespace qti_hal_test
{
class FileCaptureStreamConfig : public Deserializable
{
public:
    FileCaptureStreamConfig();
    explicit FileCaptureStreamConfig(XmlConfig & xml_config);
    ~FileCaptureStreamConfig();

    FileCaptureStreamConfig(FileCaptureStreamConfig const & rhs);
    FileCaptureStreamConfig & operator=(FileCaptureStreamConfig const & rhs) = delete;
    FileCaptureStreamConfig(FileCaptureStreamConfig && rhs) noexcept = delete;
    FileCaptureStreamConfig & operator=(FileCaptureStreamConfig && rhs) noexcept = delete;

    void Deserialize(XmlConfig & xml_config) override;

    uint32_t            sample_rate_;
    uint16_t            channels_;
    uint16_t            bit_width_;
    audio_devices_t     input_device_;
    audio_input_flags_t audio_input_flags_;
    std::string         address_;
    audio_source_t      audio_source_;
    uint32_t            duration_;

    bool                override_audio_input_flags_; // TO DO: Defunct. Remove

    std::chrono::seconds    condition_timeout_;
};
}

#endif // FILE_CAPTURE_STREAM_CONFIG_H_