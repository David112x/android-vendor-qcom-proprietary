/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "file_playback_stream_mock_2.h"

namespace qti_hal_test
{
FilePlaybackStreamMock2::FilePlaybackStreamMock2(int stream_id,
                                               FilePlaybackStreamConfig const & config,
                                               std::shared_ptr<Source> source) :
    FilePlaybackStreamMock2(stream_id, config, source, nullptr, nullptr)

{
    LOG_STREAM_ENTRY(stream_id_);
}

FilePlaybackStreamMock2::FilePlaybackStreamMock2(int stream_id,
                                               FilePlaybackStreamConfig const & config,
                                               std::shared_ptr<Source> source,
                                               std::function<void(std::exception_ptr & controller_exception)> controller) :
    FilePlaybackStreamMock2(stream_id, config, source, controller, nullptr)
{
    LOG_STREAM_ENTRY(stream_id_);
}

FilePlaybackStreamMock2::FilePlaybackStreamMock2(int stream_id,
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

FilePlaybackStreamMock2::~FilePlaybackStreamMock2()
{
    LOG_STREAM_ENTRY(stream_id_);
}

void FilePlaybackStreamMock2::Start()
{
    LOG_STREAM_ENTRY(stream_id_);
}
}