/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "audio_input_flag.h"

namespace qti_hal_test
{
AudioInputFlag::AudioInputFlag(XmlConfig & xml_config)
{
    LOG_ENTRY;
    Deserialize(xml_config);
}

AudioInputFlag::~AudioInputFlag()
{
    LOG_ENTRY;
}
void AudioInputFlag::Deserialize(XmlConfig & xml_config)
{
    LOG_ENTRY;

    flag_name_ = xml_config.GetElementText("AudioInputFlag");
}
}