/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "file_playback_stream_config.h"

namespace qti_hal_test
{
FilePlaybackStreamConfig::FilePlaybackStreamConfig() : // TO OD: use delegating constructor  or set POD types in header.
    sample_rate_{ 0 },
    channels_{ 0 },
    bit_width_{ 0 },
    kvpairs_{},
    vol_level_{ 0.01f },
    output_device_{ AUDIO_DEVICE_NONE },
    audio_format_{ AUDIO_FORMAT_INVALID },
    address_{ "file_playback_stream" },
    audio_output_flags_{ AUDIO_OUTPUT_FLAG_NONE },
    aptx_decoder_bluetooth_address_{"00:00:00:00:00:00" },
    condition_timeout_{ 60 }
{
    LOG_ENTRY;
}

FilePlaybackStreamConfig::FilePlaybackStreamConfig(XmlConfig & xml_config) :
    FilePlaybackStreamConfig{}
{
    LOG_ENTRY;
    Deserialize(xml_config);
}

FilePlaybackStreamConfig::~FilePlaybackStreamConfig()
{
    LOG_ENTRY;
}

FilePlaybackStreamConfig::FilePlaybackStreamConfig(FilePlaybackStreamConfig const & rhs) :
    sample_rate_{ rhs.sample_rate_ },
    channels_{ rhs.channels_ },
    bit_width_{ rhs.bit_width_ },
    kvpairs_{rhs.kvpairs_},
    vol_level_{ rhs.vol_level_ },
    output_device_{ rhs.output_device_ },
    audio_format_{ rhs.audio_format_ },
    address_{ rhs.address_ },
    audio_output_flags_{ rhs.audio_output_flags_ },
    aptx_decoder_bluetooth_address_{ rhs.aptx_decoder_bluetooth_address_ },
    condition_timeout_{ rhs.condition_timeout_ }
{
    LOG_ENTRY;
}

void FilePlaybackStreamConfig::Deserialize(XmlConfig & xml_config)
{
    LOG_ENTRY;
    xml_config.MoveToElement("FilePlaybackStreamConfig");

    sample_rate_ = xml_config.GetElementTextAsUint32_t("SampleRate");  // TO DO: pass opt flag stating if not finding element is error.
    channels_    = xml_config.GetElementTextAsUint16_t("Channels");
    bit_width_   = xml_config.GetElementTextAsUint16_t("BitWidth");

    if (xml_config.MoveToElement("Metadata"))
    {
        Metadata metadata(xml_config);
        kvpairs_ = metadata.GetPairs();
    }
    // TO DO: This does not cause an error if the tag is missing, but subsequently induces
    // a parse error.
    vol_level_ = xml_config.GetElementTextAsFloat("VolumeLevel");

    output_device_ = static_cast<audio_devices_t>(xml_config.GetElementTextAsUint32_t("OutputDevice", 16)); // TO DO: Check for narrowing; use brace initialization.
    audio_format_  = static_cast<audio_format_t>(xml_config.GetElementTextAsUint32_t("AudioFormat", 16)); // TO DO: Check for narrowing; use brace initialization.

    address_ = xml_config.GetElementText("Address");

    if (xml_config.MoveToElement("AudioOutputFlags"))
    {
        std::vector<AudioOutputFlag> audio_flags;
        xml_config.GetElementAsVector("AudioOutputFlags", "AudioOutputFlag", audio_flags);

        AudioOutputFlags audio_output_flags(audio_flags);
        audio_output_flags_ = audio_output_flags.ConvertToaudio_output_flags_t();
    }

    if (xml_config.MoveToElement("APTXDecoderBluetoothAddress"))
    {
        aptx_decoder_bluetooth_address_ = xml_config.GetElementText("APTXDecoderBluetoothAddress");
    }

    if (xml_config.MoveToElement("ConditionTimeout"))
    {
        uint32_t temp = xml_config.GetElementTextAsUint32_t("ConditionTimeout");
        condition_timeout_ = std::chrono::seconds{ temp }; // TO DO: Define GetElementAsChronoSeconds in XmlConfig class.
    }
}
}