/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "file_playback_stream_mock.h"

namespace qti_hal_test
{
FilePlaybackStreamMock::FilePlaybackStreamMock(int stream_id,
                                               FilePlaybackStreamConfig const & config,
                                               std::shared_ptr<Source> source) :
    FilePlaybackStreamMock(stream_id, config, source, nullptr, nullptr)

{
    LOG_STREAM_ENTRY(stream_id_);
}

FilePlaybackStreamMock::FilePlaybackStreamMock(int stream_id,
                                               FilePlaybackStreamConfig const & config,
                                               std::shared_ptr<Source> source,
                                               std::function<void(std::exception_ptr & controller_exception)> controller) :
    FilePlaybackStreamMock(stream_id, config, source, controller, nullptr)
{
    LOG_STREAM_ENTRY(stream_id_);
}

FilePlaybackStreamMock::FilePlaybackStreamMock(int stream_id,
                                               FilePlaybackStreamConfig const & config,
                                               std::shared_ptr<Source> source,
                                               std::function<void(std::exception_ptr & controller_exception)> controller,
                                               std::shared_ptr<SharedBufferControl> shared_buffer_control) :
    HALStream { stream_id },
    config_{ config },
    source_{ source },
    controller_{ std::move(controller) },
    shared_buffer_control_{ shared_buffer_control }
{
    LOG_STREAM_ENTRY(stream_id_);
}

FilePlaybackStreamMock::~FilePlaybackStreamMock()
{
    LOG_STREAM_ENTRY(stream_id_);
}

void FilePlaybackStreamMock::Start()
{
    LOG_STREAM_ENTRY(stream_id_);
}
}