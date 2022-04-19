/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "file_playback_stream.h"

namespace qti_hal_test
{
// TO DO: Consider eliminating this constructor and pass in NullController from the factory
FilePlaybackStream::FilePlaybackStream(int stream_id,
                                       FilePlaybackStreamConfig const & config,
                                       std::shared_ptr<Source> source) :
    FilePlaybackStream(stream_id, config, source, nullptr, nullptr)

{
    LOG_STREAM_ENTRY(stream_id_);
}

FilePlaybackStream::FilePlaybackStream(int stream_id,
                                       FilePlaybackStreamConfig const & config,
                                       std::shared_ptr<Source> source,
                                       ControllerFunction controller) :
    FilePlaybackStream(stream_id, config, source, controller, nullptr)
{
    LOG_STREAM_ENTRY(stream_id_);
}

FilePlaybackStream::FilePlaybackStream(int stream_id,
                                       FilePlaybackStreamConfig const & config,
                                       std::shared_ptr<Source> source,
                                       ControllerFunction controller,
                                       std::shared_ptr<SharedBufferControl> shared_buffer_control) :
    HALStream { stream_id },
    config_{ config },
    source_{ source },
    out_stream_handle_{ nullptr },
    flags_{ AUDIO_OUTPUT_FLAG_NONE },
    controller_{ std::move(controller) },
    shared_buffer_control_{ shared_buffer_control }
{
    LOG_STREAM_ENTRY(stream_id_);
}

FilePlaybackStream::~FilePlaybackStream()
{
    LOG_STREAM_ENTRY(stream_id_);
}

void FilePlaybackStream::Start() // TO DO: Refactor the implementation of HALStream subclasses
{                                // to base.
    LOG_STREAM_ENTRY(stream_id_);

    if (config_.output_device_ & AUDIO_DEVICE_OUT_BLUETOOTH_A2DP)
    {
        Connect();
    }

    if (config_.audio_format_ == AUDIO_FORMAT_APTX)
    {
        SetAPTXDecoderBluetoothAddress();
    }

    // Implement timeout here.  Watchdog class will wait time interval and then throw.
    // However, you must ensure that if Open() succeeds, Close() is called.
    // Wrapping these will wrap all calls to QCHAL.
    // watchdog_.reset();
    Open();

    SetMetadata();

    // watchdog_.reset();
    Flow();
    // watchdog_.reset();
    Close();
    // watchdog_.disable();


    if (config_.output_device_ & AUDIO_DEVICE_OUT_BLUETOOTH_A2DP)
    {
        Disconnect();
    }
}

void FilePlaybackStream::InitializeAudioOutputFlags() // TO DO: Wrapper call may be superfluous now.
{
    LOG_STREAM_ENTRY(stream_id_);

    flags_ = config_.audio_output_flags_;
}

void FilePlaybackStream::Open()
{
    LOG_STREAM_ENTRY(stream_id_);

    InitializeAudioOutputFlags();

    audio_io_handle_t io_handle = AudioIOHandleAllocator::GetHandle();

    audio_config_t audio_config;

    InitializeAudioConfig(audio_config);

    stream_logger_->API("qahw_open_output_stream");
    int status = qahw_open_output_stream(*qahw_module_.GetHandle(),
                                         io_handle,
                                         config_.output_device_,
                                         flags_,
                                         &audio_config,
                                         &out_stream_handle_,
                                         config_.address_.c_str());
    if (status != 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_open_output_stream failed, returned " << status;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }

    if (out_stream_handle_ == nullptr)
    {                                       // TO DO: Consider: SUTError, SUTAPI ?
        LogMessage message = stream_logger_->ErrorLogMessage("qahw_open_output_stream failed," \
                                                             "out_stream_handle_ is null.");
        stream_logger_->Log(message);
        throw std::runtime_error{ message.ToString() };
    }
}

void FilePlaybackStream::InitializeAudioConfig(audio_config_t & audio_config)
{
    LOG_STREAM_ENTRY(stream_id_);
    memset(&audio_config, 0, sizeof(audio_config_t));

    audio_config.sample_rate = config_.sample_rate_;
    audio_config.channel_mask = audio_channel_out_mask_from_count(config_.channels_);
    audio_config.format = config_.audio_format_;

    audio_config.offload_info.version = AUDIO_OFFLOAD_INFO_VERSION_CURRENT;
    audio_config.offload_info.size = sizeof(audio_offload_info_t);
    audio_config.offload_info.sample_rate = audio_config.sample_rate;
    audio_config.offload_info.channel_mask = audio_config.channel_mask;
    audio_config.offload_info.format = audio_config.format;
    audio_config.offload_info.bit_width = config_.bit_width_;
}

void FilePlaybackStream::SetMetadata()
{
    LOG_STREAM_ENTRY(stream_id_);
    if (config_.kvpairs_.size() > 0)
    {
        stream_logger_->API("qahw_out_set_parameters");
        int status = qahw_out_set_parameters(out_stream_handle_, config_.kvpairs_.c_str());
        if (status != 0)
        {
            std::ostringstream error_msg;
            error_msg << "qahw_out_set_parameters failed, returned " << status;
            stream_logger_->Error(error_msg.str());
            throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
        }
    }
}

void FilePlaybackStream::Connect()
{
    LOG_STREAM_ENTRY(stream_id_);
    char param[100] = { 0 };
    uint32_t device = AUDIO_DEVICE_OUT_BLUETOOTH_A2DP;

    snprintf(param, sizeof(param), "%s=%d", "connect", device); // TO DO: Replace snprintf with 
                                                                // Modern C++ equivalent.
    stream_logger_->API("qahw_set_parameters(AUDIO_DEVICE_OUT_BLUETOOTH_A2DP, connect)");
    int status = qahw_set_parameters(*qahw_module_.GetHandle(), param);
    if (status != 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_set_parameters failed, returned " << status;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }
}

void FilePlaybackStream::SetAPTXDecoderBluetoothAddress()
{
    LOG_STREAM_ENTRY(stream_id_);
    qahw_aptx_dec_param aptx_dec_param = ParseBD_ADDR();

    qahw_param_payload payload;
    payload.aptx_params = aptx_dec_param;

    stream_logger_->API("qahw_set_param_data(QAHW_PARAM_APTX_DEC)");
    int status = qahw_set_param_data(*qahw_module_.GetHandle(), QAHW_PARAM_APTX_DEC, &payload);
    if (status != 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_set_param_data failed, returned " << status;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }
}

// TO DO: Move to XML parser, or to FilePlaybackStreamConfig.
qahw_aptx_dec_param FilePlaybackStream::ParseBD_ADDR() const
{
    LOG_STREAM_ENTRY(stream_id_);

    // Extract the component strings
    std::smatch address_byte_matches;

    bool found = std::regex_match(config_.aptx_decoder_bluetooth_address_,
                                  address_byte_matches,
                                  std::regex("(.*):(.*):(.*):(.*):(.*):(.*)")); // TO DO: (/d/d) or something

    const unsigned int num_BD_ADDR_bytes = 6;

    if (!found || address_byte_matches.size() != num_BD_ADDR_bytes + 1)
    {
        throw std::runtime_error{ "Failed to parse BD_ADDR" };
    }

    // Convert strings to hexadecimal unsigned integers.
    unsigned long address_bytes[num_BD_ADDR_bytes];

    for (int i = 0; i < num_BD_ADDR_bytes; ++i)
    {
        address_bytes[i] = std::stoul(address_byte_matches.str(i+1), nullptr, 16);
    }

    // Convert address_bytes to BD_ADDR components
    qahw_aptx_dec_param aptx_dec_param;
    
    aptx_dec_param.bt_addr.nap = (address_bytes[0] << 8) | address_bytes[1];
    aptx_dec_param.bt_addr.uap = address_bytes[2];
    aptx_dec_param.bt_addr.lap = (address_bytes[3] << 16) | (address_bytes[4] << 8) | address_bytes[5];

    return aptx_dec_param;
}

void FilePlaybackStream::Flow()
{
    LOG_STREAM_ENTRY(stream_id_);

    SetVolume();

    bool is_offload{ (flags_ & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) != 0 };

    if (is_offload)
    {
        RegisterAsyncCallback(shared_buffer_control_);
    }

    std::thread proxy_reader_thread;
    std::exception_ptr proxy_reader_exception;
    bool using_proxy_reader = false;
    if (config_.output_device_ & AUDIO_DEVICE_OUT_PROXY)
    {
        using_proxy_reader = true;
        ProxyReader proxy_reader(stream_id_, stream_logger_);
        proxy_reader_thread = std::thread{ std::move(proxy_reader),
                                   std::ref(proxy_reader_exception) };
    }

    std::thread controller_thread;
    std::exception_ptr controller_exception;
    bool using_controller = false;
    if (controller_)
    {
        using_controller = true;
        // moving controller_ to the controller_thread nulls the local copy of controller_.
        controller_thread = std::thread{ std::move(controller_),
                                         std::ref(controller_exception) };
    }

    PlaybackBufferer playback_bufferer(stream_id_,
                                       out_stream_handle_,
                                       std::move(source_),
                                       is_offload,
                                       config_.condition_timeout_,
                                       stream_logger_);
    std::exception_ptr playback_bufferer_exception;
    std::thread playback_bufferer_thread{ std::move(playback_bufferer),
                                          std::ref(playback_bufferer_exception),
                                          shared_buffer_control_ };

    playback_bufferer_thread.join();

    if (using_controller)
    {
        controller_thread.join();
    }

    if (using_proxy_reader)
    {
        proxy_reader_thread.join();
    }

    if (controller_exception)
    {
        std::rethrow_exception(controller_exception);
    }

    if (proxy_reader_exception)
    {
        std::rethrow_exception(proxy_reader_exception);
    }

    if (playback_bufferer_exception)
    {
        std::rethrow_exception(playback_bufferer_exception);
    }
}

void FilePlaybackStream::SetVolume()
{
    LOG_STREAM_ENTRY(stream_id_);
    stream_logger_->API("qahw_out_set_volume");
    int status = qahw_out_set_volume(out_stream_handle_,
        config_.vol_level_,
        config_.vol_level_);
    if (status < 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_out_set_volume failed, returned " << status;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }
}

void FilePlaybackStream::RegisterAsyncCallback(std::shared_ptr<SharedBufferControl> shared_buffer_control)
{
    LOG_STREAM_ENTRY(stream_id_);
    if (flags_ & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD)
    {
        stream_logger_->API("qahw_out_set_callback");
        int status = qahw_out_set_callback(out_stream_handle_,
                                           AsyncCallback,
                                           shared_buffer_control.get()); // TO DO: Reevaluate.
        if (status < 0)
        {
            std::ostringstream error_msg;
            error_msg << "qahw_out_set_callback failed, returned " << status;
            stream_logger_->Error(error_msg.str());
            throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
        }
    }
}

int FilePlaybackStream::AsyncCallback(qahw_stream_callback_event_t event,
                                      void * param,
                                      void * cookie)
{
    // TO DO: We're creating a raw ptr that used to be a shared ptr.  Maybe weak_ptr is
    // better, or precondition that stipulates no lifetime altering operations may be
    // performed. Consider whether you could for the scope of this method create a unique
    // ptr?
    // std::shared_ptr<SharedBufferControl> shared_buffer_control =
    //  std:shared_ptr<SharedBufferControl>(static_cast<SharedBufferControl *>(cookie));
    SharedBufferControl * const shared_buffer_control = static_cast<SharedBufferControl * const>(cookie);

    assert(shared_buffer_control);

    int status = 0;

    switch (event)
    {
    case QAHW_STREAM_CBK_EVENT_WRITE_READY:
        //std::cout << "received event - QAHW_STREAM_CBK_EVENT_WRITE_READY\n" << std::endl;
        {
            std::lock_guard<std::mutex> write_guard(shared_buffer_control->write_wait_mutex_);
            shared_buffer_control->SetWriteReady();
        }
        shared_buffer_control->write_condition_.notify_all();
        break;

    case QAHW_STREAM_CBK_EVENT_DRAIN_READY:
        //std::cout << "received event - QAHW_STREAM_CBK_EVENT_DRAIN_READY\n" << std::endl;
        {
            std::lock_guard<std::mutex> drain_guard(shared_buffer_control->drain_wait_mutex_);
            shared_buffer_control->SetDrainReady();
        }
        shared_buffer_control->drain_condition_.notify_all();
        break;

    case QAHW_STREAM_CBK_EVENT_ERROR:
        status = -1;
        break;

    case QAHW_STREAM_CBK_EVENT_ADSP:
        status = -2;
        break;

    default:
#if 0
        std::ostringstream error_msg;
        error_msg << "AsyncCallback received an event it did not recognize: " << event;
        stream_logger_->Error(error_msg.str());  // Needs a static method or nonmember overload
#else                                            // or (better) see if the callback can be a member.
                                                 // Or, pass the logger as part of cookie.
        std::cout << "AsyncCallback received an event it did not recognize: " << event << std::endl;
#endif
        status = -3;
        break;
    }

    return status;
}

void FilePlaybackStream::Close()
{
    LOG_STREAM_ENTRY(stream_id_);

    stream_logger_->API("qahw_close_output_stream");
    int status = qahw_close_output_stream(out_stream_handle_);
    if (status != 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_close_output_stream failed, returned " << status;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }
}

void FilePlaybackStream::Disconnect()
{
    LOG_STREAM_ENTRY(stream_id_);
    char param[100] = { 0 };
    uint32_t device = AUDIO_DEVICE_OUT_BLUETOOTH_A2DP;

    snprintf(param, sizeof(param), "%s=%d", "disconnect", device); // TO DO: Replace snprintf with 
                                                                // Modern C++ equivalent.
    stream_logger_->API("qahw_set_parameters(AUDIO_DEVICE_OUT_BLUETOOTH_A2DP, disconnect)");
    int status = qahw_set_parameters(*qahw_module_.GetHandle(), param);
    if (status != 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_set_parameters failed, returned " << status;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }
}
}