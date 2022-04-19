/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef FILE_PLAYBPACK_STREAM_MOCK_2_H_
#define FILE_PLAYBPACK_STREAM_MOCK_2_H_

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <thread>

#include "audio_io_handle_allocator.h"
#include "controller.h"
#include "file_playback_stream_config.h"
#include "hal_stream.h"
#include "logger.h"
#include "null_controller.h"
#include "platform_dependencies.h"
#include "playback_bufferer.h"
#include "qahw_module.h"
#include "shared_buffer_control.h"
#include "source.h"

namespace qti_hal_test
{
class FilePlaybackStreamMock2 : public HALStream
{
public:
    FilePlaybackStreamMock2() = delete;

    FilePlaybackStreamMock2(int stream_id,
                       FilePlaybackStreamConfig const & config,
                       std::shared_ptr<Source> source);  // TO DO: Use shared pointer until you can compile with GCC version > 5.0

    FilePlaybackStreamMock2(int stream_id,
                       FilePlaybackStreamConfig const & config,
                       std::shared_ptr<Source> source,
                       std::function<void(std::exception_ptr & controller_exception)> controller);

    FilePlaybackStreamMock2(int stream_id,
                       FilePlaybackStreamConfig const & config,
                       std::shared_ptr<Source> source,
                       std::function<void(std::exception_ptr & controller_exception)> controller,
                       std::shared_ptr<SharedBufferControl> shared_buffer_control);

    ~FilePlaybackStreamMock2();

    FilePlaybackStreamMock2(FilePlaybackStreamMock2 const & rhs) = delete;
    FilePlaybackStreamMock2 & operator=(FilePlaybackStreamMock2 const & rhs) = delete;
    FilePlaybackStreamMock2(FilePlaybackStreamMock2 && rhs) noexcept = delete;
    FilePlaybackStreamMock2 & operator=(FilePlaybackStreamMock2 && rhs) noexcept = delete;

    void Start() override;

private:
    FilePlaybackStreamConfig                config_;

    std::shared_ptr<Source>                 source_; // TO DO: Use shared pointer until you can compile with GCC version > 5.0
    std::function<void(std::exception_ptr & controller_exception)> controller_;
    std::shared_ptr<SharedBufferControl>    shared_buffer_control_;
};
}

#endif // FILE_PLAYBPACK_STREAM_MOCK_2_H_