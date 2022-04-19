/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef FILE_PLAYBPACK_STREAM_H_
#define FILE_PLAYBPACK_STREAM_H_

#include <stdio.h>  // TO DO: REMOVE.  This is included while developing BT suppport only.
#include <string>
#include <regex>

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
#include "proxy_reader.h"
#include "qahw_module.h"
#include "shared_buffer_control.h"
#include "source.h"
#include "stream_logger.h"

namespace qti_hal_test
{
class FilePlaybackStream : public HALStream
{
public:
    FilePlaybackStream() = delete;

    FilePlaybackStream(int stream_id,
                       FilePlaybackStreamConfig const & config,
                       std::shared_ptr<Source> source); // TO DO: Use shared pointer until you can
                                                        // compile with GCC version > 5.0
    FilePlaybackStream(int stream_id,
                       FilePlaybackStreamConfig const & config,
                       std::shared_ptr<Source> source,
                       std::function<void(std::exception_ptr & controller_exception)> controller);

    FilePlaybackStream(int stream_id,
                       FilePlaybackStreamConfig const & config,
                       std::shared_ptr<Source> source,
                       std::function<void(std::exception_ptr & controller_exception)> controller,
                       std::shared_ptr<SharedBufferControl> shared_buffer_control);

    ~FilePlaybackStream();

    FilePlaybackStream(FilePlaybackStream const & rhs) = delete;
    FilePlaybackStream & operator=(FilePlaybackStream const & rhs) = delete;
    FilePlaybackStream(FilePlaybackStream && rhs) noexcept = delete;
    FilePlaybackStream & operator=(FilePlaybackStream && rhs) noexcept = delete;

    void Start() override;

private:
    void InitializeAudioOutputFlags();
    void Open();
    void InitializeAudioConfig(audio_config_t & audio_config);
    void SetMetadata();
    void Connect();
    void SetAPTXDecoderBluetoothAddress();
    qahw_aptx_dec_param ParseBD_ADDR() const;
    void Flow();
    void SetVolume();
    void RegisterAsyncCallback(std::shared_ptr<SharedBufferControl> shared_buffer_control);

    static int AsyncCallback(qahw_stream_callback_event_t event,
                             void * param,
                             void * cookie);
    void Close();
    void Disconnect();

    FilePlaybackStreamConfig                config_;
    QAHWModule                              qahw_module_;
    std::shared_ptr<Source>                 source_; // TO DO: Use shared pointer until you can
                                                     // compile with GCC version > 5.0
    using ControllerFunction = std::function<void(std::exception_ptr & controller_exception)>;
    ControllerFunction controller_;

    std::shared_ptr<SharedBufferControl>    shared_buffer_control_;

    qahw_stream_handle_t *                  out_stream_handle_;
    audio_output_flags_t                    flags_;
};
}

#endif // FILE_PLAYBPACK_STREAM_H_