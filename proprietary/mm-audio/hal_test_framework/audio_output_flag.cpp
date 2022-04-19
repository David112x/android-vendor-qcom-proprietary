/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "audio_output_flag.h"

namespace qti_hal_test
{
AudioOutputFlag::AudioOutputFlag(XmlConfig & xml_config)
{
    LOG_ENTRY;
    Deserialize(xml_config);
}

AudioOutputFlag::~AudioOutputFlag()
{
    LOG_ENTRY;
}
void AudioOutputFlag::Deserialize(XmlConfig & xml_config)
{
    LOG_ENTRY;

    flag_name_ = xml_config.GetElementText("AudioOutputFlag");
}
}