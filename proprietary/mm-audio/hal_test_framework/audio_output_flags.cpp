/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "audio_output_flags.h"

namespace qti_hal_test
{

AudioOutputFlags::AudioOutputFlags(std::vector<AudioOutputFlag> audio_flags) :
    audio_flags_{ audio_flags }
{
    LOG_ENTRY;
}

AudioOutputFlags::~AudioOutputFlags()
{
    LOG_ENTRY;
}

audio_output_flags_t AudioOutputFlags::ConvertToaudio_output_flags_t() const
{
    LOG_ENTRY;

    audio_output_flags_t audio_output_flags{ AUDIO_OUTPUT_FLAG_NONE };

    uint64_t tmp{ 0 };

    for (auto audio_flag : audio_flags_)
    {
        std::string tmp2 = audio_flag.flag_name_.substr(0, 2);

        if (audio_flag.flag_name_.substr(0, 2).compare("0x") == 0)
        {
            try
            {
                int64_t value = std::stoll(audio_flag.flag_name_, nullptr, 16);

                tmp |= value;
            }
            catch (std::logic_error const & e) // stoi can throw either invalid_argument or out_of_range.
            {
                std::ostringstream error_msg;
                error_msg << "Failed to convert value of hex flag \"" << audio_flag.flag_name_ << "\"";
                throw std::runtime_error{ error_msg.str() };
            }
        }
        else
        {
            try
            {
                tmp |= audio_output_flag_LUT_.at(audio_flag.flag_name_);
            }
            catch (std::out_of_range e)
            {
                std::ostringstream error_msg;
                error_msg << "Audio output flag \"" << audio_flag.flag_name_ << "\" not recognized";
                throw std::runtime_error{ error_msg.str() };
            }
        }
    }

    audio_output_flags = static_cast<audio_output_flags_t>(tmp);

    return audio_output_flags;
}
}