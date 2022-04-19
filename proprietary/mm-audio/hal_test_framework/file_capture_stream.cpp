/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "file_capture_stream.h"

namespace qti_hal_test
{
FileCaptureStream::FileCaptureStream(int stream_id,
                                     FileCaptureStreamConfig const & config,
                                     std::shared_ptr<Sink> sink) :
   FileCaptureStream(stream_id, config, sink, nullptr)
{
    LOG_STREAM_ENTRY(stream_id_);
}

FileCaptureStream::FileCaptureStream(int stream_id,
                                     FileCaptureStreamConfig const & config,
                                     std::shared_ptr<Sink> sink,
                                     std::function<void(std::exception_ptr&controller_exception)> controller) :
    HALStream{ stream_id },
    config_{ config },
    sink_{ sink },
    controller_{ std::move(controller) },
    in_stream_handle_{ nullptr },
    flags_{ AUDIO_INPUT_FLAG_NONE }
{
    LOG_STREAM_ENTRY(stream_id_);
}

FileCaptureStream::~FileCaptureStream()
{
    LOG_STREAM_ENTRY(stream_id_);
}

void FileCaptureStream::Start()
{
    LOG_STREAM_ENTRY(stream_id_);

    InitializeAudioInputFlags();
    Open();
    Flow();
    Close();
}

void FileCaptureStream::InitializeAudioInputFlags()
{
    LOG_STREAM_ENTRY(stream_id_);

    flags_ = config_.audio_input_flags_;
}

void FileCaptureStream::InitializeAudioConfig(audio_config_t & audio_config)
{
    LOG_STREAM_ENTRY(stream_id_);
    memset(&audio_config, 0, sizeof(audio_config_t));

    audio_config.sample_rate = config_.sample_rate_;
    audio_config.channel_mask = AUDIO_CHANNEL_IN_STEREO;  // TO DO: Coordinate wtih num_channels_ use get_audio_channel_mask
    audio_config.format = AUDIO_FORMAT_PCM_16_BIT; // TO DO: Coordinate wtih bits_per_sample_
}

void FileCaptureStream::Open()
{
    LOG_STREAM_ENTRY(stream_id_);

    audio_io_handle_t io_handle = AudioIOHandleAllocator::GetHandle();

    audio_config_t audio_config;

    InitializeAudioConfig(audio_config);

    stream_logger_->API("qahw_open_input_stream");
    int status = qahw_open_input_stream(*qahw_module_.GetHandle(),
                                        io_handle,
                                        config_.input_device_,
                                        &audio_config,
                                        &in_stream_handle_,
                                        flags_,
                                        config_.address_.c_str(),
                                        config_.audio_source_);
    if (status != 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_open_input_stream failed, returned " << status;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }

    if (in_stream_handle_ == nullptr)
    {
        LogMessage message = stream_logger_->ErrorLogMessage("qahw_input_output_stream failed, out_stream_handle_ is null.");
        stream_logger_->Log(message);
        throw std::runtime_error{ message.ToString() };
    }
}

void FileCaptureStream::Flow()
{
    LOG_STREAM_ENTRY(stream_id_);

    assert(in_stream_handle_);
    assert(sink_);

    CaptureBufferer capture_bufferer(stream_id_,
                                     in_stream_handle_,
                                     std::move(sink_),
                                     config_.duration_,
                                     config_.condition_timeout_,
                                     stream_logger_);

    std::exception_ptr capture_bufferer_exception;

    std::exception_ptr controller_exception;

    std::thread controller_thread;

    std::thread capture_bufferer_thread{ std::move(capture_bufferer),
                                         std::ref(capture_bufferer_exception) };

    bool using_controller = false;
    if (controller_)
    {
        using_controller = true;
        controller_thread = std::thread{ std::move(controller_),             // moving controller_ to the controller_thread
                                         std::ref(controller_exception) };   // nulls the local copy of controller_
    }

    capture_bufferer_thread.join();

    if (using_controller)
    {
        controller_thread.join();

        if (controller_exception)
        {
            std::rethrow_exception(controller_exception);
        }
    }

    if (capture_bufferer_exception)
    {
        std::rethrow_exception(capture_bufferer_exception);
    }
}

void FileCaptureStream::Close()  // TO DO: Factor standby call to its own method.
{
    LOG_STREAM_ENTRY(stream_id_);

    stream_logger_->API("qahw_in_standby");
    int status = qahw_in_standby(in_stream_handle_); // TO DO: Null the handle - its a pointer.
    if (status != 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_in_standby failed, returned " << status;
        stream_logger_->Error(error_msg.str());

        stream_logger_->API("qahw_close_input_stream" );
        status = qahw_close_input_stream(in_stream_handle_);
        if (status != 0)
        {
            error_msg << std::endl << "qahw_close_input_stream failed, returned: " << status;
            stream_logger_->Error(error_msg.str());
            throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
        }

        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }

    stream_logger_->API("qahw_close_input_stream");
    status = qahw_close_input_stream(in_stream_handle_);
    if (status != 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_close_input_stream failed, returned " << status;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }
}
}