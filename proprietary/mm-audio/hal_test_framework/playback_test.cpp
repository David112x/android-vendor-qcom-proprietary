/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "playback_test.h"

namespace qti_hal_test
{
PlaybackTest::PlaybackTest(XmlConfig & xml_config, std::string const & log_file_directory) :
    HALTest{ log_file_directory }
{
    Deserialize(xml_config);

    FilePlaybackStreamFactory file_playback_stream_factory;

    file_playback_stream_ = file_playback_stream_factory.GetInstance(xml_config);
    assert(file_playback_stream_);
}

PlaybackTest::~PlaybackTest()
{

}

void PlaybackTest::Execute()
{
    file_playback_stream_->Start();
}

void PlaybackTest::Deserialize(XmlConfig & xml_config)
{
    if (!xml_config.IsMatch("PlaybackTest")) // TO DO: Here we're simply validating we're looking at
    {                                        //  the right element.  No need to move first.
        throw std::runtime_error{ "Expected element PlaybackTest" };
    }
}
}