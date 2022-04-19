/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef FILE_PLAYBPACK_STREAM_MOCK_H_
#define FILE_PLAYBPACK_STREAM_MOCK_H_

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
class FilePlaybackStreamMock : public HALStream
{
public:
    FilePlaybackStreamMock() = delete;

    FilePlaybackStreamMock(int stream_id,
                       FilePlaybackStreamConfig const & config,
                       std::shared_ptr<Source> source);  // TO DO: Use shared pointer until you can compile with GCC version > 5.0

    FilePlaybackStreamMock(int stream_id,
                       FilePlaybackStreamConfig const & config,
                       std::shared_ptr<Source> source,
                       std::function<void(std::exception_ptr & controller_exception)> controller);

    FilePlaybackStreamMock(int stream_id,
                       FilePlaybackStreamConfig const & config,
                       std::shared_ptr<Source> source,
                       std::function<void(std::exception_ptr & controller_exception)> controller,
                       std::shared_ptr<SharedBufferControl> shared_buffer_control);

    ~FilePlaybackStreamMock();

    FilePlaybackStreamMock(FilePlaybackStreamMock const & rhs) = delete;
    FilePlaybackStreamMock & operator=(FilePlaybackStreamMock const & rhs) = delete;
    FilePlaybackStreamMock(FilePlaybackStreamMock && rhs) noexcept = delete;
    FilePlaybackStreamMock & operator=(FilePlaybackStreamMock && rhs) noexcept = delete;

    void Start() override;

private:
    FilePlaybackStreamConfig                config_;

    std::shared_ptr<Source>                 source_; // TO DO: Use shared pointer until you can compile with GCC version > 5.0
    std::function<void(std::exception_ptr & controller_exception)> controller_;
    std::shared_ptr<SharedBufferControl>    shared_buffer_control_;
};
}

#endif // FILE_PLAYBPACK_STREAM_MOCK_H_