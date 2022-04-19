/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "file_capture_stream_config.h"

namespace qti_hal_test
{
FileCaptureStreamConfig::FileCaptureStreamConfig() : // TO OD: use delegating constructor or set POD types in header.
    sample_rate_{ 0 },
    channels_{ 0 },
    bit_width_{ 0 },
    input_device_{ AUDIO_DEVICE_NONE },
    audio_input_flags_{ AUDIO_INPUT_FLAG_NONE },
    address_{ "file_capture_stream" },
    audio_source_{ AUDIO_SOURCE_DEFAULT },
    duration_{ 0 },
    override_audio_input_flags_{ false },
    condition_timeout_{ 60 }
{
    LOG_ENTRY;
}

// Postcondition: all data members are initialized.
FileCaptureStreamConfig::FileCaptureStreamConfig(XmlConfig & xml_config) :
    FileCaptureStreamConfig{} // Might be better to initialize the POD types in the class declaration
{
    LOG_ENTRY;
    Deserialize(xml_config);
}

FileCaptureStreamConfig::~FileCaptureStreamConfig()
{
    LOG_ENTRY;
}

FileCaptureStreamConfig::FileCaptureStreamConfig(FileCaptureStreamConfig const & rhs) :
    sample_rate_{ rhs.sample_rate_ },
    channels_{ rhs.channels_ },
    bit_width_{ rhs.bit_width_ },
    input_device_{ rhs.input_device_ },
    audio_input_flags_{ rhs.audio_input_flags_ },
    address_{ rhs.address_ },
    audio_source_{ rhs.audio_source_ },
    duration_{ rhs.duration_ },
    override_audio_input_flags_{ rhs.override_audio_input_flags_ },
    condition_timeout_{ rhs.condition_timeout_ }
{
    LOG_ENTRY;
}

void FileCaptureStreamConfig::Deserialize(XmlConfig & xml_config)
{
    LOG_ENTRY;
    xml_config.MoveToElement("FileCaptureStreamConfig");

    sample_rate_ = xml_config.GetElementTextAsUint32_t("SampleRate");  // TO DO: pass opt flag stating if not finding element is error.
    channels_ = xml_config.GetElementTextAsUint16_t("Channels");
    bit_width_ = xml_config.GetElementTextAsUint16_t("BitWidth");
    input_device_ = static_cast<audio_devices_t>(xml_config.GetElementTextAsUint32_t("InputDevice", 16)); // TO DO: Check for narrowing; use brace initialization.
    address_ = xml_config.GetElementText("Address");                                                     // A better way will be to check for leading "0x"
    audio_source_ = static_cast<audio_source_t>(xml_config.GetElementTextAsUint32_t("AudioSource", 16)); // TO DO: Check for narrowing; use brace initialization.

    if (xml_config.MoveToElement("AudioInputFlags"))
    {
        std::vector<AudioInputFlag> audio_flags;
        xml_config.GetElementAsVector("AudioInputFlags", "AudioInputFlag", audio_flags);

        AudioInputFlags audio_input_flags(audio_flags);
        audio_input_flags_ = audio_input_flags.ConvertToaudio_input_flags_t();

        override_audio_input_flags_ = true;
    }

    duration_ = xml_config.GetElementTextAsInt32_t("Duration");

    if (xml_config.MoveToElement("ConditionTimeout"))
    {
        uint32_t temp = xml_config.GetElementTextAsUint32_t("ConditionTimeout");
        condition_timeout_ = std::chrono::seconds{ temp }; // TO DO: Define GetElementAsChronoSeconds in XmlConfig class.
    }
}
}