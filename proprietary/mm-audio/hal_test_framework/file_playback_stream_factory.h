/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef FILE_PLAYBACK_STREAM_FACTORY_H_
#define FILE_PLAYBACK_STREAM_FACTORY_H_

#include <cassert>
#include <memory>
#include <stdexcept>

#include "dts_reader.h"
#include "file_playback_stream.h"
#include "file_playback_stream_config.h"
#include "file_reader.h"
#include "logger.h"
#include "shared_buffer_control.h"
#include "source.h"
#include "wav_reader.h"
#include "xml_config.h"

namespace qti_hal_test
{
class FilePlaybackStreamFactory
{
public:
    FilePlaybackStreamFactory();
    ~FilePlaybackStreamFactory();

    std::unique_ptr<FilePlaybackStream> GetInstance(XmlConfig & xml_config);
    std::unique_ptr<FilePlaybackStream> GetInstance(int stream_id, XmlConfig & xml_config);
    std::unique_ptr<FilePlaybackStream> GetInstance(int stream_id,
                                                    std::shared_ptr<SharedBufferControl> shared_buffer_control,
                                                    XmlConfig & xml_config);

    void Deserialize(XmlConfig & xml_config,
                     std::string & file_name,
                     FilePlaybackStreamConfig & file_playback_stream_config);
};
}
#endif // FILE_PLAYBACK_STREAM_FACTORY_H_