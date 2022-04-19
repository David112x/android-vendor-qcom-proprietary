/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "file_playback_stream_factory.h"

namespace qti_hal_test
{
FilePlaybackStreamFactory::FilePlaybackStreamFactory()
{
    LOG_ENTRY;
}
FilePlaybackStreamFactory::~FilePlaybackStreamFactory()
{
    LOG_ENTRY;
}

std::unique_ptr<FilePlaybackStream> FilePlaybackStreamFactory::GetInstance(XmlConfig & xml_config)
{
    LOG_ENTRY;

    return GetInstance(0,
                       std::make_shared<SharedBufferControl>(),
                       xml_config);
}

std::unique_ptr<FilePlaybackStream> FilePlaybackStreamFactory::GetInstance(int stream_id,
                                                                           XmlConfig & xml_config)
{
    LOG_ENTRY;

    return GetInstance(stream_id,
                       std::make_shared<SharedBufferControl>(),
                       xml_config);
}

std::unique_ptr<FilePlaybackStream> FilePlaybackStreamFactory::GetInstance(int stream_id,
                                                                           std::shared_ptr<SharedBufferControl> shared_buffer_control,
                                                                           XmlConfig & xml_config)
{
    LOG_ENTRY;

    std::string file_name;

    FilePlaybackStreamConfig file_playback_stream_config;

    Deserialize(xml_config, file_name, file_playback_stream_config);

    std::shared_ptr<Source> source; // TO DO: Use shared pointer until you can compile with GCC
                                    // version > 5.0

    switch(file_playback_stream_config.audio_format_)
    {
        case AUDIO_FORMAT_PCM:
        case AUDIO_FORMAT_PCM_16_BIT:
        case AUDIO_FORMAT_PCM_8_BIT:
        case AUDIO_FORMAT_PCM_32_BIT:
        case AUDIO_FORMAT_PCM_8_24_BIT:
        case AUDIO_FORMAT_PCM_FLOAT:
        case AUDIO_FORMAT_PCM_24_BIT_PACKED:
            source = std::make_shared<WavReader>(file_name);
            break;

        case AUDIO_FORMAT_DTS:
        case AUDIO_FORMAT_DTS_HD:
            source = std::make_shared<DTSReader>(file_name);
            break;

        default:
            source = std::make_shared<FileReader>(file_name);
            break;
    }

    return std::make_unique<FilePlaybackStream>(stream_id,
                                                file_playback_stream_config,
                                                source,
                                                nullptr,
                                                shared_buffer_control); // FilePlaybackStream takes
                                                                        // ownership.
}

void FilePlaybackStreamFactory::Deserialize(XmlConfig & xml_config,
                                            std::string & file_name,
                                            FilePlaybackStreamConfig & file_playback_stream_config)
{
    LOG_ENTRY;

    xml_config.MoveToElement("FilePlaybackStream"); // Moving here will cause an exception to be
                                                    // thrown if we're looking for the wrong
    file_name = xml_config.GetElementText("FileName");  // "FileName" element.

    file_playback_stream_config.Deserialize(xml_config);
}
}