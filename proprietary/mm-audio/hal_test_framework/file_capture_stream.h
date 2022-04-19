/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef FILE_CAPTURE_STREAM_H_
#define FILE_CAPTURE_STREAM_H_

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <thread>

#include "audio_io_handle_allocator.h"
#include "capture_bufferer.h"
#include "controller.h"
#include "file_capture_stream_config.h"
#include "hal_stream.h"
#include "logger.h"
#include "null_controller.h"
#include "platform_dependencies.h"
#include "qahw_module.h"
#include "sink.h"
#include "stream_logger.h"

namespace qti_hal_test
{
class FileCaptureStream : public HALStream
{
public:
    FileCaptureStream() = delete;

    FileCaptureStream(int stream_id,
                      FileCaptureStreamConfig const & config,
                      std::shared_ptr<Sink> sink); // TO DO: Use shared pointer until you can compile with GCC version > 5.0

    FileCaptureStream(int stream_id,
                      FileCaptureStreamConfig const & config,
                      std::shared_ptr<Sink> sink,
                      std::function<void(std::exception_ptr & controller_exception)> controller);

    ~FileCaptureStream();

    FileCaptureStream(FileCaptureStream const & rhs) = delete;
    FileCaptureStream & operator=(FileCaptureStream const & rhs) = delete;
    FileCaptureStream(FileCaptureStream && rhs) noexcept = delete;
    FileCaptureStream & operator=(FileCaptureStream && rhs) noexcept = delete;

    void Start() override;

private:
    void InitializeAudioInputFlags();
    void Open();
    void InitializeAudioConfig(audio_config_t & audio_config);
    void Flow();
    void Close();

    FileCaptureStreamConfig    config_;
    QAHWModule                 qahw_module_;
    std::shared_ptr<Sink>      sink_;           // TO DO: Use shared pointer until you can compile with GCC version > 5.0

    using ControllerFunction = std::function<void(std::exception_ptr & controller_exception)>;
    ControllerFunction controller_;

    qahw_stream_handle_t *     in_stream_handle_;
    audio_input_flags_t        flags_;
};
}

#endif // FILE_CAPTURE_STREAM_H_