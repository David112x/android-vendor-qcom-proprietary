/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef PLAYBACK_TEST_H_
#define PLAYBACK_TEST_H_

#include <cassert>
#include <memory>
#include <string>
#include <stdexcept>

#include "deserializable.h"
#include "file_playback_stream.h"
#include "file_playback_stream_factory.h"
#include "hal_test.h"
#include "xml_config.h"

namespace qti_hal_test
{
class PlaybackTest : public HALTest, Deserializable
{
public:
    PlaybackTest() = delete;
    PlaybackTest(XmlConfig & xml_config, std::string const & log_file_directory);

    ~PlaybackTest();

    void Execute() override;

private:
    void Deserialize(XmlConfig & xml_config) override;

    std::unique_ptr<FilePlaybackStream> file_playback_stream_;
};
}

#endif // PLAYBACK_TEST_H_